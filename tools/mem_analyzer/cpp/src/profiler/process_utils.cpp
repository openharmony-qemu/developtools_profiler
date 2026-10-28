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
#include "profiler/process_utils.hpp"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <filesystem>

#include "utils/file_utils.hpp"
#include "utils/platform.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>   // fork, exec, read, usleep
#include <poll.h>     // poll
#include <signal.h>   // kill, SIGTERM, SIGKILL
#include <errno.h>    // errno, EINTR
#include <sys/wait.h> // WIFEXITED, WEXITSTATUS
#endif

namespace fs = std::filesystem;

namespace mem_analyzer {
namespace proc {

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
std::string ResolveProgram(const std::string& prog)
{
    if (prog.empty()) {
        return {};
    }
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv == nullptr) {
        return {};
    }

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

    const std::string dirs(pathEnv);
    size_t b = 0;
    while (b <= dirs.size()) {
        const size_t e = dirs.find(';', b);
        std::string dir = dirs.substr(b, e == std::string::npos ? std::string::npos : e - b);
        b = (e == std::string::npos) ? dirs.size() + 1 : e + 1;
        if (dir.size() >= 2 && dir.front() == '"' && dir.back() == '"') {
            dir = dir.substr(1, dir.size() - 2);
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

} // namespace
#endif

#if !defined(_WIN32)
// Common fork + pipe + execl("/bin/sh","-c",cmd): stdout+stderr merged into pipe.
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
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char*)nullptr);
        _exit(127);
    }
    close(fds[1]);
    return {pid, fds[0]};
}
#endif

CapturedCmd CaptureCommand(const std::string& cmd, long long timeoutMs)
{
    CapturedCmd cap;
#if defined(_WIN32)
    // Create an inheritable pipe for the child's stdout.
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE hRead = nullptr, hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return cap;
    }
    if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
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
    // Run a real executable DIRECTLY, without a shell. `cmd.exe /c` re-tokenizes the line
    // with cmd's own rules, which do NOT match the MSVCRT escaping shell_quote() produces:
    // cmd.exe has no `\"` escape (its `"` merely toggles quoting), so a quoted value such
    // as `a" & calc` escapes the quotes and runs as a separate command, and `%VAR%` would
    // be expanded even inside quotes. A direct CreateProcess parses the line exactly the
    // way shell_quote() assumes. (`run_command_streaming()` in profiler_cmd.cpp already
    // spawns this way for the same reason.)
    //
    // The program is resolved first and its absolute path is substituted, so the direct
    // spawn cannot pick a different file than the shell would (see resolve_program()).
    // A `.bat`/`.cmd` shim (or an unresolvable program) is intentionally NOT run: doing so
    // would require cmd.exe, whose re-tokenization reopens the injection hole above.
    //
    // stderr is redirected into the pipe via si.hStdError, so callers' trailing " 2>&1" is
    // redundant — and without a shell it would reach the program as a literal argument, so
    // it must be dropped from the direct form.
    std::string direct = cmd;
    if (direct.size() >= 5 && direct.compare(direct.size() - 5, 5, " 2>&1") == 0) {
        direct.resize(direct.size() - 5);
    }
    const std::string tok = ProgramToken(direct);
    const std::string prog = ResolveProgram(tok);
    std::string resolvedLine;
    if (!prog.empty() && !IsScriptShim(prog)) {
        resolvedLine = prog + direct.substr(tok.size());
    }

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

    std::string output;
    auto drainAvailable = [&]() {
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
            if (!ReadFile(hRead, buf, (DWORD)std::min<size_t>(avail, sizeof(buf)),
                &n, nullptr) || n == 0) {
                break;
            }
            output.append(buf, n);
        }
    };

    if (timeoutMs <= 0) {
        // Unbounded: drain until EOF, then wait for the process.
        for (;;) {
            DWORD avail = 0;
            if (!PeekNamedPipe(hRead, nullptr, 0, nullptr, &avail, nullptr)) {
                break;
            }
            if (avail == 0) {
                if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
                    break;
                }
                Sleep(10);
                continue;
            }
            char buf[4096];
            DWORD n = 0;
            if (!ReadFile(hRead, buf, (DWORD)std::min<size_t>(avail, sizeof(buf)),
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
        return cap;
    }

    // Bounded read.
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeoutMs);
    bool timedOut = false;
    for (;;) {
        drainAvailable();
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
            break;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            timedOut = true;
            break;
        }
        Sleep(20);
    }
    drainAvailable();
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
    return cap;
#else
    // POSIX: spawn via fork+pipe+execl (shared helper); bounded read can
    // terminate the child on timeout (unlike pclose which blocks).
    auto sp = SpawnPiped(cmd);
    if (!sp.ok()) {
        return cap;
    }
    cap.spawned = true;

    std::string output;
    char buf[4096];
    if (timeoutMs <= 0) {
        ssize_t n = 0;
        while ((n = read(sp.readFd, buf, sizeof(buf))) > 0) {
            output.append(buf, static_cast<size_t>(n));
        }
        close(sp.readFd);
        int st = 0;
        waitpid(sp.pid, &st, 0);
        cap.output = std::move(output);
        cap.exitCode = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
        return cap;
    }

    // Bounded read: poll for readability until EOF or the deadline passes.
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

    if (timedOut) {
        cap.timedOut = true;
        // Give the child a brief chance to exit, then force-kill it.
        kill(sp.pid, SIGTERM);
        int st = 0;
        for (int i = 0; i < 20; ++i) {
            if (waitpid(sp.pid, &st, WNOHANG) == sp.pid) {
                break;
            }
            usleep(100000);
        }
        if (waitpid(sp.pid, &st, WNOHANG) != sp.pid) {
            kill(sp.pid, SIGKILL);
            waitpid(sp.pid, &st, 0);
        }
        cap.exitCode = -1;
    } else {
        int st = 0;
        waitpid(sp.pid, &st, 0);
        cap.exitCode = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    }
    cap.output = std::move(output);
    return cap;
#endif
}

std::string ShellQuote(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 16);
#if defined(_WIN32)
    out += '"';
    for (char c : s) {
        if (c == '"') {
            out += "\\\"";
        }
        else {
            out += c;
        }
    }
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
    out.reserve(s.size() + 16);
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
    return CaptureCommand("command -v " + ShellQuote(name) + " 2>&1");
#endif
}

std::string GetExecutableDir()
{
    return platform::GetExecutableDir();
}

} // namespace proc
} // namespace mem_analyzer
