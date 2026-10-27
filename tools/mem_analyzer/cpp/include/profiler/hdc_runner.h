#pragma once

#include <string>
#include <vector>

namespace mem_analyzer {

// Result of running the hdc command.
struct RunResult {
    bool ok = false;
    std::string error;

    int exit_code = 0;       // hdc command exit code
    std::string output;      // captured stdout + stderr (merged)
    long long elapsed = 0;   // actual elapsed seconds

    // True when the transcript was already streamed to the console in real
    // time (async `hiprofiler_cmd start`); the caller must not print `output`
    // a second time (it is still kept for the log).
    bool output_streamed = false;

    // Result of pulling the device-side result file via `hdc file recv`
    // (only attempted when remote_output_path was provided and the command
    // spawned successfully).
    bool pull_attempted = false;
    bool pull_ok = false;
    int pull_exit_code = 0;
    std::string pull_output;  // captured stdout + stderr of `hdc file recv`
    std::string pulled_file;  // local path of the received file
    // nativeleaks: the Native Leaks orphan-snapshot file (memleak-native-
    // orphansnapshot-<pid>.txt) pulled in addition to the .htrace, so the
    // two files can be fed to `trace -l <leaks> <htrace>` together.
    std::string pulled_leaks_file;

    // Snapshot (result_dir) flow: the device directory is polled after the
    // (near-instant) command returns, until newly-generated files' sizes
    // stabilize, then each is pulled via `hdc file recv`.
    bool snapshot_attempted = false;
    std::vector<std::string> snapshot_pulled_files;  // local paths of received files
    std::string snapshot_status;  // human-readable outcome

    // Per-process snapshot files (hidumper --mem-smaps / lsof) exported during
    // the capture, so the start-of-capture export can be diffed against the
    // end-of-capture one.
    std::vector<std::string> smaps_exported_files;
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
    std::string output_dir;            // local dir for exported files
    std::string remote_output_path;    // device-side result file to pull
    std::string process_name;          // target process/package name
    std::string result_dir;            // device dir polled after a snapshot command
    bool require_fresh_start = false;  // capture type needs a fresh app start
    bool wait_for_zip = false;         // result arrives as a hiprofiler_data_*.zip
    bool pull_nativeleaks = false;     // nativeleaks: also pull memleak-native-orphansnapshot-*.txt
    std::string device_config_content; // config pushed to the device first
    std::string smaps_prefix;          // "smaps" or "fds" snapshot file prefix
    bool is_app = false;               // whether the target is an installed app

    // Continuous snapshot session (`-c snapshot` with an explicit `-t`): run
    // for `duration` seconds like the other capture types, exporting a snapshot
    // at the start, at the end, and whenever the ArkTS heap keeps growing above
    // its recorded peak. Unset keeps the original behavior (export once and
    // finish).
    bool continuous_snapshot = false;
};

// Poll whether the target process is running on the device via
// `hdc shell 'ps -ef'`. Parses the output and matches the CMD column
// precisely against the process name. Returns the PID if found, or an
// empty string if not running.
std::string get_process_pid(const std::string& process_name);

// Lightweight process check via `hdc shell 'pidof <process_name>'`, bounded
// by `timeout_ms` so a stalled hdc session cannot block the caller forever.
// When `reliable` is non-null it reports whether the query actually succeeded
// (true even when the process is simply not running) — a failed/stalled
// query sets it to false so callers can avoid misreporting a process as gone.
std::string get_process_pid_fast(const std::string& process_name,
                                 long long timeout_ms,
                                 bool* reliable = nullptr);

RunResult run_hdc(const HdcRunOptions& opts);

} // namespace mem_analyzer
