/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "profiler/process_utils.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <vector>

#include "utils/file_utils.h"
#include "utils/platform.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>   // fork, exec, read, usleep
#include <poll.h>     // poll
#include <csignal>    // kill, SIGTERM, SIGKILL
#include <cerrno>     // errno, EINTR
#include <sys/wait.h> // WIFEXITED, WEXITSTATUS
#endif

namespace fs = std::filesystem;

namespace mem_analyzer {
namespace proc {

namespace {
[[maybe_unused]] constexpr size_t QUOTED_STRING_MIN_LEN = 2;
[[maybe_unused]] constexpr int EXIT_COMMAND_NOT_FOUND = 127;
[[maybe_unused]] constexpr int SHELL_REDIRECT_SUFFIX_LEN = 5;
[[maybe_unused]] constexpr int IDLE_POLL_INTERVAL_MS = 10;
[[maybe_unused]] constexpr int BOUNDED_POLL_INTERVAL_MS = 20;
[[maybe_unused]] constexpr int KILL_GRACE_RETRIES = 20;
[[maybe_unused]] constexpr int KILL_GRACE_SLEEP_US = 100000;
[[maybe_unused]] constexpr size_t SHELL_QUOTE_RESERVE = 16;
// CommandLineToArgvW 2N 规则：紧跟在 `"` 前的 N 个反斜杠需写成 2N 个，
// 解析端才会还原成 N 个。此即 ShellQuote Windows 分支里的反斜杠翻倍因子。
[[maybe_unused]] constexpr int WIN_ARGV_BACKSLASH_DOUBLING = 2;
} // namespace

#if defined(_WIN32)
namespace {

// Program token of a command line: everything up to the first space. Empty when the
// program itself is quoted — then we leave the line to the shell fallback below.
std::string ProgramToken(const std::string& cmd)
{
    if (cmd.empty() || cmd.front() == '"') {
        return {};
    }
    const size_t sp = cmd.find(' ');
    return cmd.substr(0, sp == std::string::npos ? cmd.size() : sp);
}

// Resolve `prog` the way cmd.exe would: PATH order, and within each directory the PATHEXT
// extensions (cmd does NOT match an extensionless file, unlike a POSIX shell). This is
// required because CreateProcess only ever appends `.exe`: with a `.bat`/`.cmd` shim
// earlier on PATH and a real `.exe` further down, a naive direct spawn would silently run
// a DIFFERENT program than the shell — and than the one the user means. Pure filesystem
// lookups, so it cannot recurse into capture_command(). Empty when nothing matches.
/**
 * Parse PATHEXT environment variable into a list of extensions.
 */
static std::vector<std::string> ParsePathExtensions()
{
    std::vector<std::string> exts;
    if (const char* extEnv = std::getenv("PATHEXT"); extEnv != nullptr) {
        const std::string all(extEnv);
        size_t b = 0;
        while (b <= all.size()) {
            const size_t e = all.find(';', b);
            const std::string one = all.substr(b, e == std::string::npos ? std::string::npos : e - b);
            if (!one.empty()) {
                exts.push_back(one);
            }
            if (e == std::string::npos) {
                break;
            }
            b = e + 1;
        }
    }
    if (exts.empty()) {
        exts = {".COM", ".EXE", ".BAT", ".CMD"};
    }
    return exts;
}

std::string ResolveProgram(const std::string& prog)
{
    if (prog.empty()) {
        return {};
    }
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv == nullptr) {
        return {};
    }

    auto exts = ParsePathExtensions();

    const std::string dirs(pathEnv);
    size_t b = 0;
    while (b <= dirs.size()) {
        const size_t e = dirs.find(';', b);
        std::string dir = dirs.substr(b, e == std::string::npos ? std::string::npos : e - b);
        b = (e == std::string::npos) ? dirs.size() + 1 : e + 1;
        if (dir.size() >= QUOTED_STRING_MIN_LEN && dir.front() == '"' && dir.back() == '"') {
            dir = dir.substr(1, dir.size() - QUOTED_STRING_MIN_LEN);
        }
        if (dir.empty()) {
            continue;
        }
        std::string base = dir;
        if (base.back() != '\\' && base.back() != '/') {
            base += '\\';
        }
        base += prog;
        for (const auto& ext : exts) {
            const std::string cand = base + ext;
            std::error_code ec;
            if (fs::is_regular_file(PathFromUtf8(cand), ec) && !ec) {
                return cand;
            }
        }
    }
    return {};
}

// A `.bat` / `.cmd` shim can only be started by cmd.exe.
bool IsScriptShim(const std::string& path)
{
    auto endsWithCi = [&path](const char* suffix) {
        const size_t n = std::strlen(suffix);
        if (path.size() < n) {
            return false;
        }
        for (size_t i = 0; i < n; ++i) {
            const unsigned char a = static_cast<unsigned char>(path[path.size() - n + i]);
            const unsigned char c = static_cast<unsigned char>(suffix[i]);
            if (std::tolower(a) != std::tolower(c)) {
                return false;
            }
        }
        return true;
    };
    return endsWithCi(".bat") || endsWithCi(".cmd");
}

void DrainAvailable(HANDLE hRead, std::string& output)
{
    for (;;) {
        DWORD avail = 0;
        if (!PeekNamedPipe(hRead, nullptr, 0, nullptr, &avail, nullptr)) {
            break;
        }
        if (avail == 0) {
            break;
        }
        char buf[4096];
        DWORD n = 0;
        if (!ReadFile(hRead, buf, static_cast<DWORD>(std::min<size_t>(avail, sizeof(buf))),
            &n, nullptr) || n == 0) {
            break;
        }
        output.append(buf, n);
    }
}

// Create an inheritable pipe for the child's stdout.
bool CreateChildPipeWindows(HANDLE& hRead, HANDLE& hWrite)
{
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return false;
    }
    if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }
    return true;
}

// Run a real executable DIRECTLY, without a shell. `cmd.exe /c` re-tokenizes the line
// with cmd's own rules, which do NOT match the MSVCRT escaping shell_quote() produces:
// cmd.exe has no `\"` escape (its `"` merely toggles quoting), so a quoted value such
// as `a" & calc` escapes the quotes and runs as a separate command, and `%VAR%` would
// be expanded even inside quotes. A direct CreateProcess parses the line exactly the
// way shell_quote() assumes. (`run_command_streaming()` in profiler_cmd.cpp already
// spawns this way for the same reason.)
// The program is resolved first and its absolute path is substituted, so the direct
// spawn cannot pick a different file than the shell would (see resolve_program()).
// A `.bat`/`.cmd` shim (or an unresolvable program) is intentionally NOT run: doing so
// would require cmd.exe, whose re-tokenization reopens the injection hole above.
// stderr is redirected into the pipe via si.hStdError, so callers' trailing " 2>&1" is
// redundant — and without a shell it would reach the program as a literal argument, so
// it must be dropped from the direct form.
std::string ResolveSpawnLineWindows(const std::string& cmd)
{
    std::string direct = cmd;
    if (direct.size() >= SHELL_REDIRECT_SUFFIX_LEN &&
        direct.compare(direct.size() - SHELL_REDIRECT_SUFFIX_LEN, SHELL_REDIRECT_SUFFIX_LEN, " 2>&1") == 0) {
        direct.resize(direct.size() - SHELL_REDIRECT_SUFFIX_LEN);
    }
    const std::string tok = ProgramToken(direct);
    const std::string prog = ResolveProgram(tok);
    if (!prog.empty() && !IsScriptShim(prog)) {
        return prog + direct.substr(tok.size());
    }
    return {};
}

// Unbounded: drain until EOF, then wait for the process.
void ReadUnboundedWindows(HANDLE hRead, PROCESS_INFORMATION pi, CapturedCmd& cap)
{
    std::string output;
    for (;;) {
        DWORD avail = 0;
        if (!PeekNamedPipe(hRead, nullptr, 0, nullptr, &avail, nullptr)) {
            break;
        }
        if (avail == 0) {
            if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
                break;
            }
            Sleep(IDLE_POLL_INTERVAL_MS);
            continue;
        }
        char buf[4096];
        DWORD n = 0;
        if (!ReadFile(hRead, buf, static_cast<DWORD>(std::min<size_t>(avail, sizeof(buf))),
            &n, nullptr) || n == 0) {
            break;
        }
        output.append(buf, n);
    }
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    cap.output = std::move(output);
    cap.exitCode = static_cast<int>(code);
}

// Bounded read.
void ReadBoundedWindows(HANDLE hRead, PROCESS_INFORMATION pi, long long timeoutMs, CapturedCmd& cap)
{
    std::string output;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeoutMs);
    bool timedOut = false;
    for (;;) {
        DrainAvailable(hRead, output);
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
            break;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            timedOut = true;
            break;
        }
        Sleep(BOUNDED_POLL_INTERVAL_MS);
    }
    DrainAvailable(hRead, output);
    CloseHandle(hRead);

    if (timedOut) {
        cap.timedOut = true;
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, INFINITE);
        cap.exitCode = -1;
    } else {
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        cap.exitCode = static_cast<int>(code);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    cap.output = std::move(output);
}

CapturedCmd CaptureCommandWindows(const std::string& cmd, long long timeoutMs)
{
    CapturedCmd cap;
    HANDLE hRead = nullptr;
    HANDLE hWrite = nullptr;
    if (!CreateChildPipeWindows(hRead, hWrite)) {
        return cap;
    }

    STARTUPINFOW si{ sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    // Route stderr into the same pipe so shell/diagnostic messages cannot
    // leak onto the user's terminal (callers no longer rely on 2>&1 to
    // suppress them).
    si.hStdError = hWrite;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};

    const std::string resolvedLine = ResolveSpawnLineWindows(cmd);
    bool started = false;
    if (!resolvedLine.empty()) {
        std::wstring wresolved = WidenPath(resolvedLine);
        started = CreateProcessW(nullptr, wresolved.data(), nullptr,
            nullptr, TRUE, 0, nullptr, nullptr, &si, &pi) != 0;
    }

    if (!started) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return cap;
    }
    CloseHandle(hWrite);
    cap.spawned = true;

    if (timeoutMs <= 0) {
        ReadUnboundedWindows(hRead, pi, cap);
        return cap;
    }
    ReadBoundedWindows(hRead, pi, timeoutMs, cap);
    return cap;
}

} // namespace
#endif

#if !defined(_WIN32)
// Strip trailing " 2>&1" (redundant: SpawnPiped already dup2's stderr to the pipe).
static std::string StripRedirect(const std::string& cmd)
{
    constexpr const char* redirectSuffix = " 2>&1";
    constexpr size_t redirectLen = 5;
    if (cmd.size() > redirectLen && cmd.compare(cmd.size() - redirectLen, redirectLen, redirectSuffix) == 0) {
        return cmd.substr(0, cmd.size() - redirectLen);
    }
    return cmd;
}

// Parse one shell-quoted token starting at position i, advancing i past the token.
// Handles ShellQuote's '\'' escape sequence for embedded single quotes.
static std::string ParseNextToken(const std::string& s, size_t& i)
{
    std::string token;
    while (i < s.size() && s[i] != ' ' && s[i] != '\t') {
        if (s[i] == '\'') {
            ++i;
            while (i < s.size() && s[i] != '\'') {
                token += s[i++];
            }
            if (i < s.size()) {
                ++i;
            }
        } else if (s[i] == '\\' && i + 1 < s.size()) {
            ++i;
            token += s[i++];
        } else {
            token += s[i++];
        }
    }
    return token;
}

// Split a shell-quoted command string into argv, reversing ShellQuote escaping.
static std::vector<std::string> ShellSplitArgs(const std::string& cmd)
{
    std::string s = StripRedirect(cmd);
    std::vector<std::string> args;
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
            ++i;
        }
        if (i >= s.size()) {
            break;
        }
        args.push_back(ParseNextToken(s, i));
    }
    return args;
}

// Common fork + pipe + execvp: stdout+stderr merged into pipe.
// Used by capture_command, run_command_streaming, and spawn_hdc_worker.
SpawnedProc SpawnPiped(const std::string& cmd)
{
    int fds[2];
    if (pipe(fds) != 0) {
        return {};
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return {};
    }
    if (pid == 0) {
        dup2(fds[1], STDOUT_FILENO);
        dup2(fds[1], STDERR_FILENO);
        close(fds[0]);
        close(fds[1]);
        auto argStrings = ShellSplitArgs(cmd);
        if (argStrings.empty()) {
            _exit(EXIT_COMMAND_NOT_FOUND);
        }
        std::vector<char*> argv;
        argv.reserve(argStrings.size() + 1);
        for (auto& a : argStrings) {
            argv.push_back(a.data());
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(EXIT_COMMAND_NOT_FOUND);
    }
    close(fds[1]);
    return {pid, fds[0]};
}

namespace {

// Give the child a brief chance to exit, then force-kill it.
void KillChildPosix(pid_t pid, CapturedCmd& cap)
{
    cap.timedOut = true;
    kill(pid, SIGTERM);
    int st = 0;
    for (int i = 0; i < KILL_GRACE_RETRIES; ++i) {
        if (waitpid(pid, &st, WNOHANG) == pid) {
            break;
        }
        usleep(KILL_GRACE_SLEEP_US);
    }
    if (waitpid(pid, &st, WNOHANG) != pid) {
        kill(pid, SIGKILL);
        waitpid(pid, &st, 0);
    }
    cap.exitCode = -1;
}

void ReadUnboundedPosix(const SpawnedProc& sp, CapturedCmd& cap)
{
    std::string output;
    char buf[4096];
    ssize_t n = 0;
    while ((n = read(sp.readFd, buf, sizeof(buf))) > 0) {
        output.append(buf, static_cast<size_t>(n));
    }
    close(sp.readFd);
    sp.readFd = -1;
    int st = 0;
    waitpid(sp.pid, &st, 0);
    cap.output = std::move(output);
    cap.exitCode = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

// Bounded read: poll for readability until EOF or the deadline passes.
void ReadBoundedPosix(const SpawnedProc& sp, long long timeoutMs, CapturedCmd& cap)
{
    std::string output;
    char buf[4096];
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeoutMs);
    bool timedOut = false;
    for (;;) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            timedOut = true;
            break;
        }
        const long long remainMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - now).count();
        struct pollfd pfd{ sp.readFd, POLLIN, 0 };
        const int pr = poll(&pfd, 1, static_cast<int>(remainMs));
        if (pr == 0) {
            timedOut = true;
            break;
        }
        if (pr < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (pfd.revents & (POLLIN | POLLHUP)) {
            const ssize_t n = read(sp.readFd, buf, sizeof(buf));
            if (n > 0) {
                output.append(buf, static_cast<size_t>(n));
            } else {
                break;  // EOF
            }
        } else {
            break;
        }
    }
    close(sp.readFd);
    sp.readFd = -1;
    if (timedOut) {
        KillChildPosix(sp.pid, cap);
    } else {
        int st = 0;
        waitpid(sp.pid, &st, 0);
        cap.exitCode = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    }
    cap.output = std::move(output);
}

// POSIX: spawn via fork+pipe+execl (shared helper); bounded read can
// terminate the child on timeout (unlike pclose which blocks).
CapturedCmd CaptureCommandPosix(const std::string& cmd, long long timeoutMs)
{
    CapturedCmd cap;
    auto sp = SpawnPiped(cmd);
    if (!sp.Ok()) {
        return cap;
    }
    cap.spawned = true;

    if (timeoutMs <= 0) {
        ReadUnboundedPosix(sp, cap);
        return cap;
    }
    ReadBoundedPosix(sp, timeoutMs, cap);
    return cap;
}

} // namespace
#endif

CapturedCmd CaptureCommand(const std::string& cmd, long long timeoutMs)
{
#if defined(_WIN32)
    return CaptureCommandWindows(cmd, timeoutMs);
#else
    return CaptureCommandPosix(cmd, timeoutMs);
#endif
}

std::string ShellQuote(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + SHELL_QUOTE_RESERVE);
#if defined(_WIN32)
    // CommandLineToArgvW tokenization (the rules CreateProcessW applies to
    // lpCommandLine): a backslash is special ONLY when it immediately precedes a
    // `"`. N backslashes + `"` -> N/2 backslashes + a quote toggle (N even), or
    // (N-1)/2 backslashes + a literal `"` that does NOT toggle (N odd). So a bare
    // `\"` emits a literal `"` instead of closing the quote. To embed a literal
    // `"` inside a quoted argument, double the run of backslashes preceding it,
    // then emit `\"`; trailing backslashes before the closing `"` must also be
    // doubled. Escaping only `"` (the former behaviour) left paths ending in `\`
    // (e.g. `C:\dir\`) with an effectively unclosed quote, which merged following
    // arguments.
    out += '"';
    size_t backslashes = 0;
    for (char c : s) {
        if (c == '\\') {
            ++backslashes;
        } else if (c == '"') {
            out.append(backslashes * WIN_ARGV_BACKSLASH_DOUBLING, '\\');
            out += "\\\"";
            backslashes = 0;
        } else {
            out.append(backslashes, '\\');
            out += c;
            backslashes = 0;
        }
    }
    out.append(backslashes * WIN_ARGV_BACKSLASH_DOUBLING, '\\');
    out += '"';
#else
    out += '\'';
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += '\'';
#endif
    return out;
}

std::string ShellQuoteDevice(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + SHELL_QUOTE_RESERVE);
    out += '\'';
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += '\'';
    return out;
}

CapturedCmd FindInPath(const std::string& name)
{
#if defined(_WIN32)
    return CaptureCommand("where " + ShellQuote(name) + " 2>&1");
#else
    // Direct PATH search — avoids shell builtins (command -v) that can't
    // be used with execvp (SpawnPiped no longer invokes /bin/sh).
    CapturedCmd cap;
    cap.spawned = true;
    cap.exitCode = 1;
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv == nullptr) {
        return cap;
    }
    std::string pathStr(pathEnv);
    size_t start = 0;
    while (start <= pathStr.size()) {
        size_t end = pathStr.find(':', start);
        if (end == std::string::npos) {
            end = pathStr.size();
        }
        std::string dir = pathStr.substr(start, end - start);
        start = end + 1;
        if (dir.empty()) {
            continue;
        }
        fs::path candidate = fs::path(dir) / name;
        std::error_code ec;
        if (fs::exists(candidate, ec) && !fs::is_directory(candidate, ec)) {
            cap.output = candidate.string() + "\n";
            cap.exitCode = 0;
            break;
        }
    }
    return cap;
#endif
}

std::string GetExecutableDir()
{
    return platform::GetExecutableDir();
}

} // namespace proc
} // namespace mem_analyzer
