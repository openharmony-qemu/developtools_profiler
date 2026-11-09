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
#pragma once
#include <string>
#include <vector>

struct sqlite3;

namespace mem_analyzer {

enum class Platform { WINDOWS, LINUX, MAC };

Platform CurrentPlatform();

namespace platform {

/**
 * Directory containing the running executable (UTF-8).
 * Used to locate bundled resources (third_party/, config/) so the release
 * package stays portable. Returns an empty string if it cannot be determined.
 */
std::string GetExecutableDir();

std::string GetTraceStreamerPath();
std::string GetRawheapTranslatorPath();

/**
 * 第三方辅助可执行文件（trace_streamer / rawheap_translator）的解析结果。
 * path 为空表示所有候选路径都不存在；candidates 按优先级排列（用于错误信息）。
 */
struct HelperBinary {
    std::string path;
    std::vector<std::string> candidates;
};

/**
 * 当前平台的辅助二进制目录候选。
 * macOS 按编译期目标架构优先（arm64 → mac-arm64，其他 → mac-x64），另一个作为兜底；
 * 其余平台只有一个固定目录。
 */
std::vector<std::string> HelperArchSubdirs();

/**
 * 在 third_party/bin/<family>/<subdir>/<exe> 中按候选顺序找第一个存在的文件。
 * 命中的不是首个候选时会打一条 fallback 警告；macOS 上若 Mach-O 架构与本进程不一致，
 * 会打 warning 提示可能需要 Rosetta 2。所有候选路径都会保留在返回值里。
 */
HelperBinary ResolveHelperBinary(const std::string& family, const std::vector<std::string>& subdirs,
    const std::string& exeName);

/** 带候选列表的辅助二进制解析（trace_streamer / rawheap_translator）。 */
HelperBinary GetTraceStreamerHelper();
HelperBinary GetRawheapTranslatorHelper();
bool FileExists(const std::string& path);
bool DirectoryExists(const std::string& path);
bool CreateDirectories(const std::string& path);
bool CreateDirectory(const std::string& path);
std::string Dirname(const std::string& path);
std::string Basename(const std::string& path);
std::string Stem(const std::string& path);
std::string Filename(const std::string& path);
std::string JoinPath(const std::string& a, const std::string& b);
std::string AbsolutePath(const std::string& path);
bool RemoveAll(const std::string& path);

constexpr int OUTPUT_FD_INHERIT = -1;

/**
 * Run a process.
 * @param output_fd  OUTPUT_FD_INHERIT → inherit parent stdout/stderr
 *                   STDOUT_FILENO      → redirect to stdout
 *                   STDERR_FILENO      → redirect to stderr
 *                   other fd           → redirect to given file descriptor
 */
int RunProcess(const std::string& exe, const std::vector<std::string>& args, int outputFd = OUTPUT_FD_INHERIT);

/**
 * Run a process with a spinner indicator.
 * Shows "Running <description>... ⠋ (elapsed)" while the process runs,
 * replaced by "done" or "failed" on completion. Non-TTY output is plain text.
 * On failure, captured subprocess output is printed to stderr (up to 1MB).
 */
int RunProcessWithSpinner(const std::string& exe, const std::vector<std::string>& args, const std::string& description);

/** Sanitize UTF-8: replace invalid byte sequences with '?'. */
std::string SanitizeUtf8(const std::string& input);

/**
 * Open a SQLite3 database with cross-platform path handling.
 * On Windows, uses sqlite3_open16 (UTF-16) for proper Unicode path support.
 * On macOS/Linux, uses sqlite3_open (UTF-8) directly.
 * Throws std::runtime_error on failure.
 */
sqlite3* OpenSqlite3Db(const std::string& dbPath);

/** Open a file with the system default application. */
void OpenWithDefaultApp(const std::string& filePath);

} // namespace platform
} // namespace mem_analyzer
