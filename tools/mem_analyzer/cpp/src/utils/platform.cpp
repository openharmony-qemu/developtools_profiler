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
#include "utils/platform.hpp"
#include "utils/file_utils.hpp"
#include "utils/process_spinner.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/temp_dir.hpp"
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

static std::string get_base_path() {
    // Base path is the directory containing the executable.
    // This makes the release package portable — just keep third_party/ next to the exe.
    return platform::get_executable_dir();
}

Platform current_platform() {
#if defined(_WIN32)
    return Platform::Windows;
#elif defined(__APPLE__)
    return Platform::Mac;
#else
    return Platform::Linux;
#endif
}

namespace platform {

std::string get_executable_dir() {
    // Resolved once: the executable path does not change during a run.
    static const std::string dir = []() -> std::string {
#if defined(_WIN32)
        wchar_t buf[MAX_PATH];
        DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) return {};
        return path_to_utf8(fs::path(buf).parent_path());
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buf(size, '\0');
        if (_NSGetExecutablePath(&buf[0], &size) != 0) return {};
        return fs::path(buf).parent_path().string();
#else
        // Linux: /proc/self/exe works reliably
        std::error_code ec;
        auto p = fs::read_symlink("/proc/self/exe", ec);
        if (ec) return path_to_utf8(fs::current_path());
        return path_to_utf8(p.parent_path());
#endif
    }();
    return dir;
}

std::string get_trace_streamer_path() {
    auto base = path_from_utf8(get_base_path()) / "third_party" / "bin" / "trace_streamer_binary";
#if defined(_WIN32)
    auto p = path_to_utf8(base / "trace_streamer_windows.exe");
    return p;
#elif defined(__APPLE__)
    return path_to_utf8(base / "trace_streamer_mac");
#else
    return path_to_utf8(base / "trace_streamer_linux_static");
#endif
}

std::string get_rawheap_translator_path() {
    auto base = path_from_utf8(get_base_path()) / "third_party" / "bin" / "rawheaptranslator";
#if defined(_WIN32)
    return path_to_utf8(base / "windows-x64" / "rawheap_translator.exe");
#elif defined(__APPLE__)
    return path_to_utf8(base / "mac-arm64" / "rawheap_translator");
#else
    return path_to_utf8(base / "linux-x64" / "rawheap_translator");
#endif
}

bool file_exists(const std::string& path) {
#if defined(_WIN32)
    std::wstring wpath = widen_path(path);
    DWORD attr = GetFileAttributesW(wpath.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    return exists && !ec && fs::is_regular_file(path, ec);
#endif
}

bool directory_exists(const std::string& path) {
#if defined(_WIN32)
    std::wstring wpath = widen_path(path);
    DWORD attr = GetFileAttributesW(wpath.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    return exists && !ec && fs::is_directory(path, ec);
#endif
}

bool create_directories(const std::string& path) {
#if defined(_WIN32)
    std::wstring wpath = widen_path(path);
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

bool create_directory(const std::string& path) {
#if defined(_WIN32)
    std::wstring wpath = widen_path(path);
    return CreateDirectoryW(wpath.c_str(), nullptr) ||
           GetLastError() == ERROR_ALREADY_EXISTS;
#else
    std::error_code ec;
    return fs::create_directory(path, ec);
#endif
}

std::string dirname(const std::string& path) {
    return path_to_utf8(path_from_utf8(path).parent_path());
}

std::string basename(const std::string& path) {
    return path_to_utf8(path_from_utf8(path).filename());
}

std::string stem(const std::string& path) {
    return path_to_utf8(path_from_utf8(path).stem());
}

std::string join_path(const std::string& a, const std::string& b) {
    return path_to_utf8(path_from_utf8(a) / b);
}

std::string filename(const std::string& path) {
    return path_to_utf8(path_from_utf8(path).filename());
}

std::string absolute_path(const std::string& path) {
    std::error_code ec;
    auto abs_p = fs::absolute(path_from_utf8(path), ec);
    return ec ? path : path_to_utf8(abs_p);
}

bool remove_all(const std::string& path) {
#if defined(_WIN32)
    std::wstring wpath = widen_path(path);
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
static std::string resolve_path_arg(const std::string& arg) {
    if (arg.empty() || arg[0] == '-') return arg;  // flag, not a path
    auto p = path_from_utf8(arg);
    if (!p.is_relative()) return arg;              // already absolute
    std::error_code ec;
    auto ap = fs::absolute(p, ec);
    return ec ? arg : path_to_utf8(ap.lexically_normal());
}

int run_process(const std::string& exe,
                const std::vector<std::string>& args,
                int output_fd) {
    // On macOS the child is started from inside its own directory
    // ("cd <exe_dir> && DYLD_LIBRARY_PATH=."), so relative path args must be
    // made absolute or they would resolve against the wrong directory. On
    // other platforms this is a harmless no-op (relative == absolute here).
    std::vector<std::string> abs_args;
    abs_args.reserve(args.size());
    for (const auto& a : args) {
        abs_args.push_back(resolve_path_arg(a));
    }
    bool redirect = output_fd >= 0;
#if defined(_WIN32)
    std::wstring wexe = widen_path(exe);
    std::wstring cmdline = wexe;
    for (const auto& a : abs_args) {
        cmdline += L" \"" + widen_path(a) + L"\"";
    }

    HANDLE hOut = nullptr;
    if (redirect) {
        if (output_fd == 1) {
            hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        } else if (output_fd == 2) {
            hOut = GetStdHandle(STD_ERROR_HANDLE);
        } else {
            hOut = reinterpret_cast<HANDLE>(_get_osfhandle(output_fd));
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
    return (int)exitCode;
#else
    // POSIX（macOS / Linux）：fork + execv，参数数组不经 shell 解析，
    // 文件路径中的 `"` / `$()` / 反引号 / `;` 等元字符不再被解释（消除命令注入）。
    std::string program = exe;
#if defined(__APPLE__)
    auto exe_path = path_from_utf8(exe);
    const std::string exe_dir = path_to_utf8(exe_path.parent_path());
#endif

    std::vector<std::string> argv_store;
    argv_store.reserve(abs_args.size() + 1);
    argv_store.push_back(program);
    for (const auto& a : abs_args) argv_store.push_back(a);

    std::vector<char*> argv;
    argv.reserve(argv_store.size() + 1);
    for (auto& s : argv_store) argv.push_back(const_cast<char*>(s.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程
#if defined(__APPLE__)
        // 复刻原 "cd <exe_dir> && DYLD_LIBRARY_PATH=." 语义：trace_streamer 依赖同目录 dylib
        chdir(exe_dir.c_str());
        setenv("DYLD_LIBRARY_PATH", ".", 1);
#endif
        if (redirect) {
            dup2(output_fd, STDOUT_FILENO);
            dup2(output_fd, STDERR_FILENO);
        }
        execv(program.c_str(), argv.data());
        _exit(127);
    }
    if (pid < 0) {
        throw std::runtime_error("Failed to launch process: " + exe);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
#endif
}

int run_process_with_spinner(
    const std::string& exe,
    const std::vector<std::string>& args,
    const std::string& description)
{
    TempDir tmp("spinner", true);
    std::string out_name = platform::stem(exe) + "_output.txt";
    std::string out_file = platform::join_path(tmp.path(), out_name);

#if defined(_WIN32)
    std::wstring wout = widen_path(out_file);
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE h = CreateFileW(wout.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                           &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h), 0);
#else
    int fd = ::open(out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
#endif

    ProcessSpinner spinner(description);
    int ret = platform::run_process(exe, args, fd);
    spinner.done(ret == 0);

#if defined(_WIN32)
    _close(fd);  // also closes the underlying HANDLE
#else
    ::close(fd);
#endif

    if (ret != 0) {
        std::ifstream f(path_from_utf8(out_file), std::ios::binary | std::ios::ate);
        if (f) {
            auto size = f.tellg();
            if (size > 0 && size < 1024 * 1024) {
                f.seekg(0);
                std::string buf(static_cast<size_t>(size), '\0');
                f.read(&buf[0], size);
                log::logw("  --- " + description + " output ---", "\n", buf,
                          "\n  -----------------------------");
            }
        }
    }

    return ret;
}

std::string sanitize_utf8(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = (unsigned char)input[i];
        // Check if this is a valid UTF-8 start byte
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
            if (i + 1 >= input.size()) { result += '?'; i++; continue; }
            unsigned char c2 = (unsigned char)input[i+1];
            if ((c2 & 0xC0) != 0x80) { result += '?'; i++; continue; }
            // Check for overlong: 0xC0 0x80 = U+0000 (overlong)
            if ((c & 0x1E) == 0) { result += '?'; i++; continue; }
            result += input[i]; result += input[i+1];
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // Three-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
            if (i + 2 >= input.size()) { result += '?'; i++; continue; }
            unsigned char c2 = (unsigned char)input[i+1];
            unsigned char c3 = (unsigned char)input[i+2];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) { result += '?'; i++; continue; }
            // Check for surrogates (U+D800-U+DFFF)
            int cp = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (cp >= 0xD800 && cp <= 0xDFFF) { result += '?'; i++; continue; }
            result += input[i]; result += input[i+1]; result += input[i+2];
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // Four-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            if (i + 3 >= input.size()) { result += '?'; i++; continue; }
            unsigned char c2 = (unsigned char)input[i+1];
            unsigned char c3 = (unsigned char)input[i+2];
            unsigned char c4 = (unsigned char)input[i+3];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) {
                result += '?'; i++; continue;
            }
            int cp = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
            if (cp > 0x10FFFF) { result += '?'; i++; continue; }
            result += input[i]; result += input[i+1]; result += input[i+2]; result += input[i+3];
            i += 4;
        } else {
            // Invalid start byte (10xxxxxx or 11111xxx)
            result += '?';
            i++;
        }
    }
    return result;
}

sqlite3* open_sqlite3_db(const std::string& db_path) {
    sqlite3* db = nullptr;
#if defined(_WIN32)
    auto wdb_path = widen_path(db_path);
    if (sqlite3_open16(wdb_path.c_str(), &db) != SQLITE_OK) {
#else
    if (sqlite3_open_v2(db_path.c_str(),
                         &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
#endif
        std::string err = db ? sqlite3_errmsg(db) : "unknown";
        if (db) sqlite3_close(db);
        throw std::runtime_error("Cannot open database: " + db_path + " (" + err + ")");
    }

    sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_TRUNCATE, nullptr, nullptr);

    sqlite3_stmt* chk = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT count(*) FROM sqlite_master WHERE type='table'", -1, &chk, nullptr) == SQLITE_OK) {
        if (sqlite3_step(chk) == SQLITE_ROW && sqlite3_column_int(chk, 0) == 0) {
            sqlite3_finalize(chk);
            sqlite3_close(db);
            throw std::runtime_error("Database file is empty (no tables): " + db_path +
                " — likely path mismatch caused sqlite3 to create a new empty DB");
        }
        sqlite3_finalize(chk);
    }
    return db;
}

void open_with_default_app(const std::string& file_path) {
#if defined(_WIN32)
    // ShellExecuteW 直接传路径，不经 shell，避免 `cmd /c start` 命令注入。
    auto wpath = path_from_utf8(file_path);
    HINSTANCE rc = ShellExecuteW(nullptr, L"open", wpath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(rc) <= 32) {
        log::loge("Error: Unable to launch the default browser to open ", file_path);
    }
#else
    // fork + exec：参数数组不经 shell 解析，file_path 中的元字符不会被解释。
    pid_t pid = fork();
    if (pid == 0) {
#if defined(__APPLE__)
        execl("/usr/bin/open", "open", file_path.c_str(), (char*)nullptr);
#else
        execl("/usr/bin/xdg-open", "xdg-open", file_path.c_str(), (char*)nullptr);
#endif
        _exit(127);
    }
    if (pid < 0) {
        log::loge("Error: Unable to launch the default browser to open ", file_path);
        return;
    }
    int status = 0;
    waitpid(pid, &status, 0);
#endif
}

} // namespace platform
} // namespace mem_analyzer
