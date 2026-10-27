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
#if !defined(_WIN32)
#include <sys/types.h>
#endif

namespace mem_analyzer {
namespace proc {

// Result of running a command and capturing its output. Profiler-only helper:
// kept out of utils/platform to avoid touching shared code.
struct CapturedCmd {
    std::string output;
    int exit_code = -1;
    bool spawned = false;
    bool timed_out = false;  // true when `timeout_ms` expired before completion
};

// Run a command and capture all stdout. The caller embeds `2>&1` to merge
// stderr. When `timeout_ms > 0` the command is bounded by that many
// milliseconds: on expiry the child is terminated and `timed_out` is set.
CapturedCmd capture_command(const std::string& cmd, long long timeout_ms = 0);

// Wrap `s` as a single shell argument for the host shell.
// POSIX  -> '...' with each ' escaped as '\''
// Windows -> "..." with each " escaped as \"
std::string shell_quote(const std::string& s);

// Wrap `s` as a single shell argument for the device-side shell (POSIX),
// regardless of the host platform. Reached through `hdc shell`, where the
// host-side Windows/cmd quoting rules do not apply.
std::string shell_quote_device(const std::string& s);

// Look up `name` on PATH. POSIX: `command -v`; Windows: `where`.
CapturedCmd find_in_path(const std::string& name);

// Directory containing the running executable, or empty on failure.
std::string get_executable_dir();

#if !defined(_WIN32)
// Result of spawn_piped: child pid + pipe read fd (stdout+stderr merged).
struct SpawnedProc {
    pid_t pid = -1;
    int read_fd = -1;
    bool ok() const { return pid > 0; }
};

// POSIX only: fork + execl("/bin/sh","-c",cmd) with stdout+stderr piped.
// On success returns {pid>0, read_fd}; on failure {-1,-1} (nothing to close).
// Caller owns read_fd (close after consuming) and must waitpid(pid).
SpawnedProc spawn_piped(const std::string& cmd);
#endif

} // namespace proc
} // namespace mem_analyzer
