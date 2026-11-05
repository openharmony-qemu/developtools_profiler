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

namespace mem_analyzer {

// Result of running the hdc command.
struct RunResult {
    bool ok = false;
    std::string error;

    int exitCode = 0;       // hdc command exit code
    std::string output;      // captured stdout + stderr (merged)
    long long elapsed = 0;   // actual elapsed seconds

    // True when the transcript was already streamed to the console in real
    // time (async `hiprofiler_cmd start`); the caller must not print `output`
    // a second time (it is still kept for the log).
    bool outputStreamed = false;

    // Result of pulling the device-side result file via `hdc file recv`
    // (only attempted when remote_output_path was provided and the command
    // spawned successfully).
    bool pullAttempted = false;
    bool pullOk = false;
    int pullExitCode = 0;
    std::string pullOutput;  // captured stdout + stderr of `hdc file recv`
    std::string pulledFile;  // local path of the received file
    // nativeleaks: the Native Leaks orphan-snapshot file (memleak-native-
    // orphansnapshot-<pid>.txt) pulled in addition to the .htrace, so the
    // two files can be fed to `trace -l <leaks> <htrace>` together.
    std::string pulledLeaksFile;

    // Snapshot (result_dir) flow: the device directory is polled after the
    // (near-instant) command returns, until newly-generated files' sizes
    // stabilize, then each is pulled via `hdc file recv`.
    bool snapshotAttempted = false;
    std::vector<std::string> snapshotPulledFiles;  // local paths of received files
    std::string snapshotStatus;  // human-readable outcome

    // Per-process snapshot files (hidumper --mem-smaps / lsof) exported during
    // the capture, so the start-of-capture export can be diffed against the
    // end-of-capture one.
    std::vector<std::string> smapsExportedFiles;
};

// Options for `run_hdc`. Two result-export flows, selected by which field is
// non-empty:
//   - hiprofiler_cmd flow (`remote_output_path`): the command runs for
//     `duration` seconds (countdown shown); when it finishes, the result file
//     is pulled via `hdc file recv`.
//   - snapshot flow (`result_dir`): the command is near-instant; after it
//     returns, `result_dir` is polled until files stabilize, then each is
//     pulled via `hdc file recv`.
// `device_config_content`, when non-empty, is pushed to the device via
// `hdc file send` before the command is executed.
struct HdcRunOptions {
    std::string content;               // device command to run under `hdc shell`
    long long duration = 0;            // capture duration (seconds); 0 = until done
    std::string outputDir;            // local dir for exported files
    std::string remoteOutputPath;    // device-side result file to pull
    std::string processName;          // target process/package name
    std::string resultDir;            // device dir polled after a snapshot command
    bool requireFreshStart = false;  // capture type needs a fresh app start
    bool waitForZip = false;         // result arrives as a hiprofiler_data_*.zip
    bool pullNativeleaks = false;     // nativeleaks: also pull memleak-native-orphansnapshot-*.txt
    std::string deviceConfigContent; // config pushed to the device first
    std::string smapsPrefix;          // "smaps" or "fds" snapshot file prefix
    bool isApp = false;               // whether the target is an installed app

    // Continuous snapshot session (`-c snapshot` with an explicit `-t`): run
    // for `duration` seconds like the other capture types, exporting a snapshot
    // at the start, at the end, and whenever the ArkTS heap keeps growing above
    // its recorded peak. Unset keeps the original behavior (export once and
    // finish).
    bool continuousSnapshot = false;
};

// Poll whether the target process is running on the device via
// `hdc shell 'ps -ef'`. Parses the output and matches the CMD column
// precisely against the process name. Returns the PID if found, or an
// empty string if not running.
std::string GetProcessPid(const std::string& processName);

// Lightweight process check via `hdc shell 'pidof <process_name>'`, bounded
// by `timeout_ms` so a stalled hdc session cannot block the caller forever.
// When `reliable` is non-null it reports whether the query actually succeeded
// (true even when the process is simply not running) — a failed/stalled
// query sets it to false so callers can avoid misreporting a process as gone.
std::string GetProcessPidFast(const std::string& processName, long long timeoutMs, bool* reliable = nullptr);

RunResult RunHdc(const HdcRunOptions& opts);

} // namespace mem_analyzer
