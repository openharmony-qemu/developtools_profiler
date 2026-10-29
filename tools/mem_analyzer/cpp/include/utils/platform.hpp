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
bool FileExists(const std::string& path);
bool DirectoryExists(const std::string& path);
bool CreateDirectories(const std::string& path);
bool CreateDirectory(const std::string& path);
std::string dirname(const std::string& path);
std::string Basename(const std::string& path);
std::string Stem(const std::string& path);
std::string filename(const std::string& path);
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
