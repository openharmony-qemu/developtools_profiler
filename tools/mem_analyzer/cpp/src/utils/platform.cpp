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
#include "utils/platform.h"
#include "utils/file_utils.h"
#include "utils/process_spinner.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/temp_dir.h"
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <cstdint>
#endif
#endif

#include <sqlite3.h>

namespace fs = std::filesystem;

namespace mem_analyzer {

static std::string GetBasePath()
{
    // Base path is the directory containing the executable.
    // This makes the release package portable — just keep third_party/ next to the exe.
    return platform::GetExecutableDir();
}

Platform CurrentPlatform()
{
#if defined(_WIN32)
    return Platform::WINDOWS;
#elif defined(__APPLE__)
    return Platform::MAC;
#else
    return Platform::LINUX;
#endif
}

namespace platform {

namespace {
[[maybe_unused]] constexpr int STDERR_FD = 2;
[[maybe_unused]] constexpr int EXIT_COMMAND_NOT_FOUND = 127;
constexpr long long BYTES_PER_KB = 1024;
constexpr int UTF8_TWO_BYTE_LEN = 2;
constexpr int UTF8_THREE_BYTE_LEN = 3;
constexpr int UTF8_MAX_BYTES = 4;
} // namespace

std::string GetExecutableDir()
{
    // Resolved once: the executable path does not change during a run.
    static const std::string dir = []() -> std::string {
#if defined(_WIN32)
        wchar_t buf[MAX_PATH];
        DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) {
            return {};
        }
        return PathToUtf8(fs::path(buf).parent_path());
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buf(size, '\0');
        if (_NSGetExecutablePath(&buf[0], &size) != 0) {
            return {};
        }
        return fs::path(buf).parent_path().string();
#else
        // Linux: /proc/self/exe works reliably
        std::error_code ec;
        auto p = fs::read_symlink("/proc/self/exe", ec);
        if (ec) {
            return PathToUtf8(fs::current_path());
        }
        return PathToUtf8(p.parent_path());
#endif
    }();
    return dir;
}

std::vector<std::string> HelperArchSubdirs() {
#if defined(_WIN32)
    return {"windows-x64"};
#elif defined(__APPLE__)
    // 按编译期目标架构优先；另一个架构作为兜底（打包错位时仍能启动）。
#  if defined(__aarch64__) || defined(__arm64__)
    return {"mac-arm64", "mac-x64"};
#  else
    return {"mac-x64", "mac-arm64"};
#  endif
#else
    return {"linux-x64"};
#endif
}

#if defined(__APPLE__)
namespace {
// 读 thin Mach-O 头判断架构：1 = arm64/arm，2 = x86_64/i386，0 = 非 thin Mach-O 或无法识别。
int MachoArchOf(const std::string& path) {
    std::ifstream in(PathFromUtf8(path), std::ios::binary);
    if (!in) {
        return 0;
    }
    unsigned char hdr[8] = {0};
    in.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
    if (!in) {
        return 0;
    }
    const uint32_t leMagic = (static_cast<uint32_t>(hdr[3]) << 24) | (static_cast<uint32_t>(hdr[2]) << 16) |
        (static_cast<uint32_t>(hdr[1]) << 8) | hdr[0];
    // 非 thin Mach-O（含 universal/fat）不做判断，交给系统加载器。
    if (leMagic != 0xFEEDFACF && leMagic != 0xFEEDFACE)  {
        return 0;
    }
    const uint32_t cpu = (static_cast<uint32_t>(hdr[7]) << 24) | (static_cast<uint32_t>(hdr[6]) << 16) |
        (static_cast<uint32_t>(hdr[5]) << 8) | hdr[4];
    if (cpu == 0x0100000C || cpu == 12) {
        return 1;  // arm64 / arm
    }
    if (cpu == 0x01000007 || cpu == 7) {
        return 2;   // x86_64 / i386
    }
    return 0;
}

int NativeMachoArch() {
#  if defined(__aarch64__) || defined(__arm64__)
    return 1;
#  else
    return 2;
#  endif
}
}  // namespace
#endif

HelperBinary ResolveHelperBinary(const std::string& family, const std::vector<std::string>& subdirs,
    const std::string& exeName) {
    HelperBinary result;
    const fs::path base = PathFromUtf8(GetBasePath()) / "third_party" / "bin" / family;
    for (size_t i = 0; i < subdirs.size(); ++i) {
        const fs::path candidate = subdirs[i].empty() ? (base / exeName) : (base / subdirs[i] / exeName);
        const std::string candidateStr = PathToUtf8(candidate);
        result.candidates.push_back(candidateStr);
        if (result.path.empty() && FileExists(candidateStr)) {
            result.path = candidateStr;
            if (i > 0) {
                log::Logw("Warning: preferred helper binary missing, using fallback: ", candidateStr);
            }
        }
    }
#if defined(__APPLE__)
    if (!result.path.empty()) {
        const int arch = MachoArchOf(result.path);
        if (arch != 0 && arch != NativeMachoArch()) {
            log::Logw("Warning: ", result.path, " is not a native ", (NativeMachoArch() == 1 ? "arm64" : "x86_64"),
                " binary; it may require Rosetta 2 to run");
        }
    }
#endif
    return result;
}

HelperBinary GetTraceStreamerHelper() {
#if defined(_WIN32)
    return ResolveHelperBinary("trace_streamer_binary", {""}, "trace_streamer_windows.exe");
#elif defined(__APPLE__)
    return ResolveHelperBinary("trace_streamer_binary", {""}, "trace_streamer_mac");
#else
    return ResolveHelperBinary("trace_streamer_binary", {""}, "trace_streamer_linux_static");
#endif
}

HelperBinary GetRawheapTranslatorHelper() {
#if defined(_WIN32)
    return ResolveHelperBinary("rawheaptranslator", HelperArchSubdirs(), "rawheap_translator.exe");
#else
    return ResolveHelperBinary("rawheaptranslator", HelperArchSubdirs(), "rawheap_translator");
#endif
}

std::string GetTraceStreamerPath() { return GetTraceStreamerHelper().path; }

std::string GetRawheapTranslatorPath() { return GetRawheapTranslatorHelper().path; }

bool FileExists(const std::string& path)
{
#if defined(_WIN32)
    std::wstring wpath = WidenPath(path);
    DWORD attr = GetFileAttributesW(wpath.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    return exists && !ec && fs::is_regular_file(path, ec);
#endif
}

bool DirectoryExists(const std::string& path)
{
#if defined(_WIN32)
    std::wstring wpath = WidenPath(path);
    DWORD attr = GetFileAttributesW(wpath.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    return exists && !ec && fs::is_directory(path, ec);
#endif
}

bool CreateDirectories(const std::string& path)
{
#if defined(_WIN32)
    std::wstring wpath = WidenPath(path);
    // Create directories recursively
    size_t pos = 0;
    while (true) {
        pos = wpath.find(L'\\', pos + 1);
        if (pos == std::wstring::npos) {
            return CreateDirectoryW(wpath.c_str(), nullptr) ||
                   GetLastError() == ERROR_ALREADY_EXISTS;
        }
        std::wstring sub = wpath.substr(0, pos);
        if (sub.size() > 0 && sub[sub.size() - 1] != L':') {
            CreateDirectoryW(sub.c_str(), nullptr);
        }
    }
#else
    std::error_code ec;
    return fs::create_directories(path, ec);
#endif
}

bool CreateDirectory(const std::string& path)
{
#if defined(_WIN32)
    std::wstring wpath = WidenPath(path);
    return CreateDirectoryW(wpath.c_str(), nullptr) ||
           GetLastError() == ERROR_ALREADY_EXISTS;
#else
    std::error_code ec;
    return fs::create_directory(path, ec);
#endif
}

std::string Dirname(const std::string& path)
{
    return PathToUtf8(PathFromUtf8(path).parent_path());
}

std::string Basename(const std::string& path)
{
    return PathToUtf8(PathFromUtf8(path).filename());
}

std::string Stem(const std::string& path)
{
    return PathToUtf8(PathFromUtf8(path).stem());
}

std::string JoinPath(const std::string& a, const std::string& b)
{
    return PathToUtf8(PathFromUtf8(a) / b);
}

std::string Filename(const std::string& path)
{
    return PathToUtf8(PathFromUtf8(path).filename());
}

std::string AbsolutePath(const std::string& path)
{
    std::error_code ec;
    auto absP = fs::absolute(PathFromUtf8(path), ec);
    return ec ? path : PathToUtf8(absP);
}

bool RemoveAll(const std::string& path)
{
#if defined(_WIN32)
    std::wstring wpath = WidenPath(path);
    std::error_code ec;
    return fs::remove_all(fs::path(wpath), ec) > 0;
#else
    std::error_code ec;
    return fs::remove_all(path, ec) > 0;
#endif
}

// Resolve a single child-process argument if it denotes a filesystem path:
// make it absolute against the caller's CWD. Non-path args (CLI flags with a
// leading '-') and already-absolute paths are returned unchanged.
static std::string ResolvePathArg(const std::string& arg)
{
    if (arg.empty() || arg[0] == '-') {
        return arg; // flag, not a path
    }
    auto p = PathFromUtf8(arg);
    if (!p.is_relative()) { // already absolute
        return arg;
    }
    std::error_code ec;
    auto ap = fs::absolute(p, ec);
    return ec ? arg : PathToUtf8(ap.lexically_normal());
}

#if defined(_WIN32)
/// Windows process launch via CreateProcessW.
static int RunProcessWindows(const std::string& exe,
    const std::vector<std::string>& absArgs, bool redirect, int outputFd)
{
    std::wstring wexe = WidenPath(exe);
    std::wstring cmdline = wexe;
    for (const auto& a : absArgs) {
        cmdline += L" \"" + WidenPath(a) + L"\"";
    }

    HANDLE hOut = nullptr;
    if (redirect) {
        if (outputFd == 1) {
            hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        } else if (outputFd == STDERR_FD) {
            hOut = GetStdHandle(STD_ERROR_HANDLE);
        } else {
            hOut = reinterpret_cast<HANDLE>(_get_osfhandle(outputFd));
        }
    }

    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    if (redirect) {
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hOut;
        si.hStdError  = hOut;
    }

    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(wexe.c_str(), cmdline.data(),
                        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        throw std::runtime_error("Failed to launch process: " + exe +
            " (CreateProcessW error " + std::to_string(GetLastError()) + ")");
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(exitCode);
}
#else
/// POSIX process launch via fork + execv.
// POSIX（macOS / Linux）：fork + execv，参数数组不经 shell 解析，
// 文件路径中的 `"` / `$()` / 反引号 / `;` 等元字符不再被解释（消除命令注入）。
static int RunProcessPosix(const std::string& exe,
    const std::vector<std::string>& absArgs, bool redirect, int outputFd)
{
    std::string program = exe;
#if defined(__APPLE__)
    auto exePath = PathFromUtf8(exe);
    const std::string exeDir = PathToUtf8(exePath.parent_path());
#endif

    std::vector<std::string> argvStore;
    argvStore.reserve(absArgs.size() + 1);
    argvStore.push_back(program);
    for (const auto& a : absArgs) {
        argvStore.push_back(a);
    }

    std::vector<char*> argv;
    argv.reserve(argvStore.size() + 1);
    for (auto& s : argvStore) {
        argv.push_back(const_cast<char*>(s.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程
#if defined(__APPLE__)
        // 复刻原 "cd <exe_dir> && DYLD_LIBRARY_PATH=." 语义：trace_streamer 依赖同目录 dylib
        chdir(exeDir.c_str());
        setenv("DYLD_LIBRARY_PATH", ".", 1);
#endif
        if (redirect) {
            dup2(outputFd, STDOUT_FILENO);
            dup2(outputFd, STDERR_FILENO);
        }
        execv(program.c_str(), argv.data());
        _exit(EXIT_COMMAND_NOT_FOUND);
    }
    if (pid < 0) {
        throw std::runtime_error("Failed to launch process: " + exe);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}
#endif

int RunProcess(const std::string& exe, const std::vector<std::string>& args, int outputFd)
{
    // On macOS the child is started from inside its own directory
    // ("cd <exe_dir> && DYLD_LIBRARY_PATH=."), so relative path args must be
    // made absolute or they would resolve against the wrong directory. On
    // other platforms this is a harmless no-op (relative == absolute here).
    std::vector<std::string> absArgs;
    absArgs.reserve(args.size());
    for (const auto& a : args) {
        absArgs.push_back(ResolvePathArg(a));
    }
    bool redirect = outputFd >= 0;
#if defined(_WIN32)
    return RunProcessWindows(exe, absArgs, redirect, outputFd);
#else
    return RunProcessPosix(exe, absArgs, redirect, outputFd);
#endif
}

int RunProcessWithSpinner(const std::string& exe, const std::vector<std::string>& args, const std::string& description)
{
    TempDir tmp("spinner", true);
    std::string outName = platform::Stem(exe) + "_output.txt";
    std::string outFile = platform::JoinPath(tmp.Path(), outName);

#if defined(_WIN32)
    std::wstring wout = WidenPath(outFile);
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE h = CreateFileW(wout.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h), 0);
#else
    int fd = ::open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
#endif

    ProcessSpinner spinner(description);
    int ret = platform::RunProcess(exe, args, fd);
    spinner.Done(ret == 0);

#if defined(_WIN32)
    _close(fd);  // also closes the underlying HANDLE
#else
    ::close(fd);
#endif

    if (ret != 0) {
        std::ifstream f(PathFromUtf8(outFile), std::ios::binary | std::ios::ate);
        if (f) {
            auto size = f.tellg();
            if (size > 0 && size < BYTES_PER_KB * BYTES_PER_KB) {
                f.seekg(0);
                std::string buf(static_cast<size_t>(size), '\0');
                f.read(&buf[0], size);
                log::Logw("  --- " + description + " output ---", "\n", buf,
                          "\n  -----------------------------");
            }
        }
    }

    return ret;
}

/// Decode a 2-byte UTF-8 sequence. Returns new index (i+1 on error, i+2 on success).
static size_t DecodeTwoByteUtf8(const std::string& input, size_t i, std::string& result)
{
    if (i + 1 >= input.size()) {
        result += '?';
        return i + 1;
    }
    unsigned char c = (unsigned char)input[i];
    unsigned char c2 = (unsigned char)input[i+1];
    if ((c2 & 0xC0) != 0x80) {
        result += '?';
        return i + 1;
    }
    // Check for overlong: 0xC0 0x80 = U+0000 (overlong)
    if ((c & 0x1E) == 0) {
        result += '?';
        return i + 1;
    }
    result.append(input, i, UTF8_TWO_BYTE_LEN);
    return i + UTF8_TWO_BYTE_LEN;
}

/// Decode a 3-byte UTF-8 sequence. Returns new index (i+1 on error, i+3 on success).
static size_t DecodeThreeByteUtf8(const std::string& input, size_t i, std::string& result)
{
    if (i + UTF8_THREE_BYTE_LEN - 1 >= input.size()) {
        result += '?';
        return i + 1;
    }
    unsigned char c = (unsigned char)input[i];
    unsigned char c2 = (unsigned char)input[i+1];
    unsigned char c3 = (unsigned char)input[i+2];
    if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
        result += '?';
        return i + 1;
    }
    // Check for surrogates (U+D800-U+DFFF)
    int cp = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    if (cp >= 0xD800 && cp <= 0xDFFF) {
        result += '?';
        return i + 1;
    }
    result.append(input, i, UTF8_THREE_BYTE_LEN);
    return i + UTF8_THREE_BYTE_LEN;
}

/// Decode a 4-byte UTF-8 sequence. Returns new index (i+1 on error, i+4 on success).
static size_t DecodeFourByteUtf8(const std::string& input, size_t i, std::string& result)
{
    if (i + UTF8_MAX_BYTES - 1 >= input.size()) {
        result += '?';
        return i + 1;
    }
    unsigned char c = (unsigned char)input[i];
    unsigned char c2 = (unsigned char)input[i+1];
    unsigned char c3 = (unsigned char)input[i+2];
    unsigned char c4 = (unsigned char)input[i+3];
    if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) {
        result += '?';
        return i + 1;
    }
    int cp = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    if (cp > 0x10FFFF) {
        result += '?';
        return i + 1;
    }
    result.append(input, i, UTF8_MAX_BYTES);
    return i + UTF8_MAX_BYTES;
}

std::string SanitizeUtf8(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = (unsigned char)input[i];
        if (c <= 0x7F) {
            // Single byte (0xxxxxxx) - always valid
            if (c == 0) {
                // Null byte is not valid in UTF-8
                result += '?';
            } else {
                result += input[i];
            }
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // Two-byte sequence (110xxxxx 10xxxxxx)
            i = DecodeTwoByteUtf8(input, i, result);
        } else if ((c & 0xF0) == 0xE0) {
            // Three-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
            i = DecodeThreeByteUtf8(input, i, result);
        } else if ((c & 0xF8) == 0xF0) {
            // Four-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            i = DecodeFourByteUtf8(input, i, result);
        } else {
            // Invalid start byte (10xxxxxx or 11111xxx)
            result += '?';
            i++;
        }
    }
    return result;
}

sqlite3* OpenSqlite3Db(const std::string& dbPath)
{
    sqlite3* db = nullptr;
#if defined(_WIN32)
    auto wdbPath = WidenPath(dbPath);
    if (sqlite3_open16(wdbPath.c_str(), &db) != SQLITE_OK) {
#else
    if (sqlite3_open_v2(dbPath.c_str(),
        &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
#endif
        std::string err = db ? sqlite3_errmsg(db) : "unknown";
        if (db) {
            sqlite3_close(db);
        }
        throw std::runtime_error("Cannot open database: " + dbPath + " (" + err + ")");
    }

    sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_TRUNCATE, nullptr, nullptr);

    sqlite3_stmt* chk = nullptr;
    if (sqlite3_prepare_v2(db,
        "SELECT count(*) FROM sqlite_master WHERE type='table'",
        -1, &chk, nullptr) == SQLITE_OK) {
        if (sqlite3_step(chk) == SQLITE_ROW && sqlite3_column_int(chk, 0) == 0) {
            sqlite3_finalize(chk);
            sqlite3_close(db);
            throw std::runtime_error("Database file is empty (no tables): " + dbPath +
                " — likely path mismatch caused sqlite3 to create a new empty DB");
        }
        sqlite3_finalize(chk);
    }
    return db;
}

void OpenWithDefaultApp(const std::string& filePath)
{
#if defined(_WIN32)
    // ShellExecuteW 直接传路径，不经 shell，避免 `cmd /c start` 命令注入。
    auto wpath = PathFromUtf8(filePath);
    HINSTANCE rc = ShellExecuteW(nullptr, L"open", wpath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(rc) <= 32) {
        log::Loge("Error: Unable to launch the default browser to open ", filePath);
    }
#else
    // fork + exec：参数数组不经 shell 解析，file_path 中的元字符不会被解释。
    pid_t pid = fork();
    if (pid == 0) {
#if defined(__APPLE__)
        execl("/usr/bin/open", "open", filePath.c_str(), static_cast<char*>(nullptr));
#else
        execl("/usr/bin/xdg-open", "xdg-open", filePath.c_str(), static_cast<char*>(nullptr));
#endif
        _exit(EXIT_COMMAND_NOT_FOUND);
    }
    if (pid < 0) {
        log::Loge("Error: Unable to launch the default browser to open ", filePath);
        return;
    }
    int status = 0;
    waitpid(pid, &status, 0);
#endif
}
} // namespace platform
} // namespace mem_analyzer
