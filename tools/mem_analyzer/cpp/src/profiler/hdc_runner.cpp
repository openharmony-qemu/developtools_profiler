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
#include <cstdio>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <csignal>
#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#else
#include <sys/wait.h>
#include <unistd.h>   // fork, pipe, dup2, execl, fdopen, close
#endif

#include "profiler/hdc_runner.h"
#include "profiler/hdc_target.h"
#include "profiler/process_utils.hpp"
#include "profiler/string_utils.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {

using proc::capture_command;
using proc::shell_quote;
using proc::CapturedCmd;

namespace fs = std::filesystem;
using clock_type = std::chrono::steady_clock;

namespace {

// Ctrl+C (SIGINT) flag: the signal handler only sets this flag; the capture
// loop polls it so we can stop hiprofiler cleanly and save the data instead
// of the process dying abruptly.
volatile std::sig_atomic_t g_interrupt_requested = 0;

extern "C" void handle_sigint(int) {
    g_interrupt_requested = 1;
}

// Pinned target (explicit --device, or the single attached device auto-pinned by the
// gate): on an hdc device-level failure, flag it and surface the hdc text once, so
// polling loops can bail out early instead of spinning until timeout. Untargeted runs
// (no key pinned: 0 devices, noisy parse, hdc failure) never trigger this.
std::atomic<bool> g_hdc_device_failed{false};

bool note_device_failure(const std::string& output) {
    if (hdc_connect_key().empty()) return false;
    if (!hdc_is_device_failure(output)) return false;
    if (!g_hdc_device_failed.exchange(true)) log::loge(hdc_failure_lines(output));
    return true;
}

// Pinned target: has the target device failed at the hdc level?
//
// Why every wait/poll loop must ask this: the flag is only ever set by
// note_device_failure(), i.e. by an hdc device-level error (a "[Fail]" line),
// and it stays set for the rest of the run. Without this check a device that
// disappears mid-capture makes the loops idle out their full timeouts, keep
// issuing hdc round-trips to an unreachable device, and let callers misread the
// empty results as "process not running" / "no file produced". Asking lets each
// loop end promptly and fall through to the normal teardown, which still stops
// hiprofiler and keeps the data captured so far.
//
// What must NOT count as device loss: a stalled connection (cap.timed_out), an
// empty output, or any text without a "[Fail]" line. Those are normal in-flight
// conditions — e.g. a second hdc connection that is briefly busy because the
// foregrounded capture holds it — so treating them as device loss would abort a
// healthy capture.
//
// Scope: only runs with a pinned target can set the flag (note_device_failure()
// returns early while no key is pinned), so 0-device runs behave exactly as before.
// A 1-device run IS pinned by the gate, so it gains this early-stop behaviour on
// purpose — that is what the auto-pin is for (see resolve_device_target()).
bool device_lost() { return g_hdc_device_failed.load(); }

// Whole seconds elapsed since `start`.
long long elapsed_since(clock_type::time_point start) {
    return static_cast<long long>(
        std::chrono::duration_cast<std::chrono::seconds>(
            clock_type::now() - start)
            .count());
}

// Sleep in 100ms ticks for at most `ticks` iterations, returning as soon as
// `stop()` becomes true. Waiting loops use this so they react to completion /
// early-abort conditions promptly instead of sleeping out a full 5s period.
void sleep_ticks(int ticks, const std::function<bool()>& stop) {
    for (int i = 0; i < ticks; ++i) {
        if (stop()) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// The device-side `hiprofiler_cmd start` prints "Use 'hiprofiler_cmd stop' to
// stop profiling" once the session is up. Our users never run stop manually —
// they wait for the countdown to expire or interrupt with the platform's
// terminate key — so replace that line with an accurate hint when displaying
// it on the console. (The raw transcript is still saved to the log file.)
// The terminate key differs by platform: macOS uses Control+C, while
// Windows/Linux use Ctrl+C.
std::string console_start_line(const std::string& line) {
    if (line.find("Use 'hiprofiler_cmd stop'") != std::string::npos) {
        const char* key = nullptr;
        switch (current_platform()) {
            case Platform::Mac:     key = "Control+C"; break;
            case Platform::Windows: key = "Ctrl+C";    break;
            default:                key = "Ctrl+C";    break;  // Linux
        }
        return std::string("Capture stops when the countdown ends, or press ")
               + key + " to stop early.\n";
    }
    return line;
}

// Process memory/fd metrics parsed from `hidumper --mem <pid>` (pss / native
// heap / arkts heap / dma(graph) / anon page other / .so) and `lsof -p <pid>`
// (fd count).
struct ProcessMetrics {
    bool valid = false;             // at least one metric was parsed
    long long pss_kb = 0;           // "Total" row Pss (already includes SwapPss)
    long long native_heap_kb = 0;   // "native heap" row Pss + SwapPss
    long long arkts_heap_kb = 0;    // "ark ts heap" row Pss + SwapPss
    long long dma_graph_kb = 0;     // "Graph" row Pss + SwapPss
    long long anon_page_other_kb = 0;   // "AnonPage other" row Pss + SwapPss
    long long so_kb = 0;                // ".so" row Pss + SwapPss
    long long gl_kb = 0;                // "GL" row Pss + SwapPss
    long long hap_kb = 0;               // ".hap" row Pss + SwapPss
    long long arkweb_pa_kb = 0;         // "arkweb-pa heap" row Pss + SwapPss
    long long file_page_other_kb = 0;   // "FilePage other" row Pss + SwapPss
    int fd_count = -1;              // lsof data rows (excluding header); -1 = unknown
};

// Parse the `hidumper --mem <pid>` memory table. Each data row is
//     [label] Pss SharedClean SharedDirty PrivateClean PrivateDirty Swap SwapPss ...
// (the "native heap" row carries three extra heap columns after SwapPss). A
// category row's first numeric column is its plain Pss, which excludes swap;
// hidumper adds SwapPss only to the "Total" row's Pss (totalAllPss). To keep
// every shown category consistent with the process-total convention
// (pss + swapPss), the whole numeric row is read and SwapPss (7th column,
// index 6) is added to each category value.
// Separator rows (all dashes) are skipped because their label starts with '-';
// header rows ("Pss"/"Size" etc.) match none of the target labels. Only exact
// full-label matches are accepted so lookalike rows ("native heap:" subtitle,
// "arkts-static heap") are ignored.
namespace {

// Declarative spec for the hidumper --mem rows we care about. `label` holds
// the exact row label split into whitespace tokens ("native heap",
// "ark ts heap", ...); `field` is the ProcessMetrics member the row fills;
// `pss_only` marks rows whose first numeric column already includes SwapPss
// (only the "Total" row). Adding a new displayed category is one table row.
struct MemRowSpec {
    std::vector<std::string> label;
    long long ProcessMetrics::*field;
    bool pss_only;
};

const MemRowSpec kMemRows[] = {
    {{"Total"},             &ProcessMetrics::pss_kb,             true},
    {{"native", "heap"},    &ProcessMetrics::native_heap_kb,     false},
    {{"ark", "ts", "heap"}, &ProcessMetrics::arkts_heap_kb,      false},
    {{"Graph"},             &ProcessMetrics::dma_graph_kb,       false},
    {{"AnonPage", "other"}, &ProcessMetrics::anon_page_other_kb, false},
    {{".so"},               &ProcessMetrics::so_kb,              false},
    {{"GL"},                &ProcessMetrics::gl_kb,              false},
    {{".hap"},              &ProcessMetrics::hap_kb,             false},
    {{"arkweb-pa", "heap"}, &ProcessMetrics::arkweb_pa_kb,       false},
    {{"FilePage", "other"}, &ProcessMetrics::file_page_other_kb, false},
};

}  // namespace

ProcessMetrics parse_hidumper_mem(const std::string& output) {
    ProcessMetrics m;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream lss(line);
        std::string first;
        if (!(lss >> first)) continue;                 // blank line
        if (!first.empty() && first[0] == '-') continue;  // separator row
        // Match the row label against the spec table (first token + the rest).
        const MemRowSpec* spec = nullptr;
        for (const auto& s : kMemRows) {
            if (s.label[0] != first) continue;
            bool matched = true;
            for (size_t i = 1; i < s.label.size(); ++i) {
                std::string tok;
                if (!(lss >> tok) || tok != s.label[i]) {
                    matched = false;   // e.g. "native heap:" -> "heap:" != "heap"
                    break;
                }
            }
            if (matched) {
                spec = &s;
                break;
            }
        }
        if (!spec) continue;
        // Collect the remaining numeric columns of the row. Non-numeric tokens
        // are skipped; strtoll avoids std::stoll's exceptions per token.
        std::vector<long long> nums;
        std::string v;
        while (lss >> v) {
            char* end = nullptr;
            long long val = std::strtoll(v.c_str(), &end, 10);
            if (end != v.c_str() && *end == '\0') nums.push_back(val);
        }
        if (nums.empty()) continue;
        const long long pss = nums[0];
        const long long swap_pss = nums.size() > 6 ? nums[6] : 0;
        m.*(spec->field) = spec->pss_only ? pss : pss + swap_pss;
        m.valid = true;
    }
    return m;
}

// Count the data rows of a `lsof -p <pid>` output: every non-empty line that
// does not start with the "COMMAND" header column counts as one open fd.
int count_lsof_fds(const std::string& output) {
    int count = 0;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        if (line.find("COMMAND") == 0) continue;  // header row
        ++count;
    }
    return count;
}

// Sample the live memory/fd metrics of one process: `hidumper --mem <pid>`
// (Pss / native heap / arkts heap / dma / ...) and `lsof -p <pid>` (fd count).
// An empty pid, or queries that yield nothing, produce an invalid sample.
ProcessMetrics sample_process_metrics(const std::string& pid,
                                      long long timeout_ms = 8000) {
    ProcessMetrics metrics;
    if (pid.empty()) return metrics;
    CapturedCmd mem_cap = capture_command(
        hdc_shell("hidumper --mem " + proc::shell_quote_device(pid)) + " 2>&1", timeout_ms);
    // Pinned target: on a device-level failure the sample would merely come out
    // empty; note_device_failure() flags it so the caller aborts the countdown
    // instead of polling a device that is no longer reachable. This helper is a
    // plain function (not a loop), so it returns an invalid sample rather than
    // breaking out.
    if (note_device_failure(mem_cap.output)) return metrics;
    metrics = parse_hidumper_mem(mem_cap.output);
    CapturedCmd lsof_cap = capture_command(
        hdc_shell("lsof -p " + proc::shell_quote_device(pid)) + " 2>&1", timeout_ms);
    // Same gate as above: a device-level failure must stop the polling here too,
    // otherwise fd_count would silently be reported as 0.
    if (note_device_failure(lsof_cap.output)) return metrics;
    metrics.fd_count = count_lsof_fds(lsof_cap.output);
    return metrics;
}

// Detect permission-related failures from a device command's output. On a
// non-root phone, hidumper refuses to export memory info for release-signed
// (non-tunable) apps and prints something like
//   "[Permission Check Failed] The --mem-jsheap option needs a
//    debug-signed application. ..."
// On a hit, returns a short block of error text starting at the matched line
// (up to 8 lines); otherwise returns an empty string, so callers can fail
// fast before a capture / poll loop runs to completion.
std::string detect_permission_error(const std::string& output) {
    static const char* kMarkers[] = {
        "permission check failed",
        "debug-signed application",
        "permission denied",
        "operation not permitted",
        "no permission",
        "not permitted",
        "access denied",
    };
    std::istringstream iss(output);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(iss, line)) lines.push_back(line);

    for (size_t i = 0; i < lines.size(); ++i) {
        bool hit = false;
        for (const char* m : kMarkers) {
            if (contains_ci(lines[i], m)) { hit = true; break; }
        }
        if (!hit) continue;
        // Collect an error block starting at the matched line: up to 8 lines,
        // stopping at the first empty line.
        std::string block;
        for (size_t j = i; j < lines.size() && j < i + 8; ++j) {
            if (lines[j].empty()) break;
            std::string blk = lines[j];
            // Strip trailing \r (hdc pty output may be \r\n) and whitespace.
            while (!blk.empty() &&
                   (blk.back() == '\r' || blk.back() == ' ' || blk.back() == '\t')) {
                blk.pop_back();
            }
            block += blk;
            block += '\n';
        }
        while (!block.empty() &&
               (block.back() == '\n' || block.back() == ' ' || block.back() == '\t')) {
            block.pop_back();
        }
        if (!block.empty()) return block;
    }
    return "";
}

// Export a per-process snapshot file to `output_dir` via `hdc shell`:
// `lsof -p <pid>` when `prefix` is "fds", otherwise `hidumper --mem-smaps <pid>`.
// The captured output is written to <output_dir>/<prefix>_<timestamp>.txt
// (binary mode). Returns the local path on success, or an empty string on
// failure (unable to launch hdc / empty output / cannot open the file).
std::string export_process_file(const std::string& pid,
                                const std::string& output_dir,
                                const std::string& prefix,
                                std::string* permission_error = nullptr) {
    const std::string device_cmd =
        (prefix == "fds") ? ("lsof -p " + proc::shell_quote_device(pid)) : ("hidumper --mem-smaps " + proc::shell_quote_device(pid));
    // Timeout so a stalled device query cannot leave the export thread (and
    // thus the join at stop time) blocked forever.
    CapturedCmd cap = capture_command(
        hdc_shell(device_cmd) + " 2>&1", 10000);
    if (note_device_failure(cap.output)) return "";
    if (!cap.spawned || cap.output.empty()) return "";
    // Permission probe: on a non-root phone, hidumper refuses to export memory
    // info for release-signed (non-tunable) apps and prints a permission
    // failure. Carry the result back to the caller so the capture aborts
    // right at the start instead of running the full duration.
    if (permission_error) {
        *permission_error = detect_permission_error(cap.output);
    }
    const std::string local_path = path_to_utf8(
        (path_from_utf8(output_dir) /
         (prefix + "_" + current_timestamp() + ".txt"))
            .lexically_normal());
    // UTF-8 path -> wide path on Windows (path_from_utf8 is a no-op elsewhere).
    std::ofstream ofs(path_from_utf8(local_path), std::ios::binary);
    if (!ofs.is_open()) return "";
    ofs << cap.output;
    return local_path;
}

// Local path a `hdc file recv <remote> <output_dir>` writes to:
// <output_dir>/<basename(remote)>, lexically normalized.
std::string local_recv_path(const std::string& output_dir,
                            const std::string& remote) {
    return path_to_utf8(
        (path_from_utf8(output_dir) / path_from_utf8(remote).filename())
            .lexically_normal());
}

// Mark an exported snapshot with the role it was captured for by inserting
// `-<role>` before the extension:
//   hidumper-jsheap-123-123-1788535089196.rawheap
//     -> hidumper-jsheap-123-123-1788535089196-start.rawheap
// The role has to live in the file name because every snapshot of a continuous
// session is pulled flat into the session directory (no role subdirectories).
// An empty `role` leaves the path untouched; if the rename fails the original
// path is kept so the file is still reported and analyzed.
std::string mark_snapshot_role(const std::string& path, const std::string& role) {
    if (role.empty()) return path;
    const fs::path p = path_from_utf8(path);
    const fs::path renamed =
        p.parent_path() / (path_to_utf8(p.stem()) + "-" + role + path_to_utf8(p.extension()));
    std::error_code ec;
    fs::rename(p, renamed, ec);
    if (ec) {
        log::logw("Warning: unable to tag the snapshot with its role (", role, "): ", path, " (", ec.message(), ")");
        return path;
    }
    return path_to_utf8(renamed);
}

// Outcome of a `hdc file recv` invocation.
struct PullOutcome {
    bool ok = false;
    int exit_code = 0;
    std::string output;
};

// Pull a file from the device: `hdc file recv <remote> <local_dir>` (stderr
// merged). Returns the captured output and the hdc exit code.
PullOutcome pull_file(const std::string& remote, const std::string& local_dir) {
    std::string cmd = hdc_file_recv(remote, local_dir) + " 2>&1";
    CapturedCmd cap = capture_command(cmd);
    PullOutcome po;
    if (note_device_failure(cap.output)) {
        po.exit_code = cap.exit_code;
        po.output = cap.output;
        return po;
    }
    if (!cap.spawned) {
        po.output = "unable to launch hdc file recv";
        return po;
    }
    po.exit_code = cap.exit_code;
    po.output = cap.output;
    // `hdc file recv` is unreliable on BOTH sides: it exits 0 even when the
    // remote file does not exist (printing "[Fail]Error opening file: ..." to
    // stderr), and it can exit nonzero even after a fully successful transfer
    // (e.g. some hdc builds return a nonzero code after FileTransfer
    // finishes). So neither the exit code alone nor the transfer banner
    // decides success — a failure marker in the output, or a missing/empty
    // local result file, marks the pull as failed.
    const bool failed_msg = po.output.find("[Fail]") != std::string::npos ||
                            po.output.find("Error opening file") != std::string::npos ||
                            po.output.find("no such file") != std::string::npos ||
                            po.output.find("No such file") != std::string::npos;
    bool file_ok = false;
    {
        const std::string local = local_recv_path(local_dir, remote);
        const fs::path local_p = path_from_utf8(local);
        std::error_code ec;
        if (fs::exists(local_p, ec) && !ec) {
            std::error_code ec2;
            const auto size = fs::file_size(local_p, ec2);
            file_ok = !ec2 && size > 0;
        }
    }
    po.ok = !failed_msg && file_ok;
    return po;
}

// Extract the millisecond-epoch timestamp from a hiprofiler_data_*.zip
// filename. The naming convention is: hiprofiler_data_<ms>.zip
// Returns seconds since epoch (ms / 1000), or -1 on parse failure.
long long parse_hiprofiler_data_ts(const std::string& filename) {
    // Expected: "hiprofiler_data_1785766871434.zip"
    constexpr const char* prefix = "hiprofiler_data_";
    const auto plen = std::strlen(prefix);
    if (filename.size() <= plen + 4) return -1;  // need at least "X.zip"
    if (filename.rfind(prefix, 0) != 0) return -1;
    if (filename.substr(filename.size() - 4) != ".zip") return -1;

    // Extract the numeric part between prefix and ".zip".
    std::string numstr = filename.substr(plen, filename.size() - plen - 4);
    if (numstr.empty()) return -1;

    // Validate all digits.
    for (char c : numstr) {
        if (c < '0' || c > '9') return -1;
    }

    try {
        long long ms = std::stoll(numstr);
        return ms / 1000;  // convert milliseconds -> seconds
    } catch (...) {
        return -1;
    }
}

// List all hiprofiler_data_*.zip files in `dir` via plain `ls`. Returns a
// map of { filename -> embedded-timestamp-seconds }. The embedded timestamp
// (extracted from the filename) is the actual time when hiprofiler generated
// the zip on the device, so it is a reliable indicator that hiprofiler has
// finished writing the file.
std::map<std::string, long long> list_hiprofiler_zips(const std::string& dir) {
    std::map<std::string, long long> result;
    std::string cmd = hdc_shell("ls " + proc::shell_quote_device(dir)) + " 2>&1";
    CapturedCmd cap = capture_command(cmd);
    if (note_device_failure(cap.output)) return result;
    if (!cap.spawned) return result;

    std::istringstream iss(cap.output);
    std::string line;
    while (std::getline(iss, line)) {
        const auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        const auto last = line.find_last_not_of(" \t\r\n");
        std::string name = line.substr(first, last - first + 1);

        // Filter: must start with "hiprofiler_data_" and end with ".zip".
        if (name.rfind("hiprofiler_data_", 0) != 0) continue;
        if (name.size() < 4 || name.substr(name.size() - 4) != ".zip") continue;

        const long long ts = parse_hiprofiler_data_ts(name);
        if (ts < 0) continue;
        result[name] = ts;
    }
    return result;
}

// Build the full device path for a hiprofiler zip file inside `dir`.
std::string make_device_path(const std::string& dir, const std::string& name) {
    std::string full = dir;
    if (!full.empty() && full.back() != '/') full += "/";
    full += name;
    return full;
}

// List the regular files in a device directory with their sizes (bytes), via
// `hdc shell 'ls -l <dir>'`. Returns a name->size map; empty on error /
// nonexistent / empty dir. Directory entries, the "total N" header and blank
// lines are skipped. Filenames are assumed to contain no spaces (true for
// heap-snapshot / hiprofiler files); the whole remainder after the time field
// is taken as the name. Only files whose name starts with `name_prefix` (and,
// when `name_suffix` is non-empty, also ends with it) are kept.
//
// Two date/time layouts are supported, since the device may emit either:
//   traditional "Aug  2 20:53"     -> 3 fields (month, day, time)
//   ISO         "2026-08-02 20:53" -> 2 fields (date, time)
// A '-' in the first field after `size` signals the ISO form; otherwise the
// traditional 3-field form follows. Only the 5 leading fields (perm, links,
// owner, group, size) are structurally required.
std::map<std::string, long long> list_dir_file_sizes(const std::string& dir,
                                                     const std::string& name_prefix,
                                                     const std::string& name_suffix = "") {
    std::map<std::string, long long> files;
    std::string cmd = hdc_shell("ls -l " + proc::shell_quote_device(dir)) + " 2>&1";
    CapturedCmd cap = capture_command(cmd);
    if (note_device_failure(cap.output)) return files;
    if (!cap.spawned) return files;
    std::istringstream iss(cap.output);
    std::string line;
    while (std::getline(iss, line)) {
        // Expected line (one of):
        //   "-rw-r--r-- 1 root root 56789 Aug  2 20:53 name"
        //   "-rw-r--r-- 1 root root 56789 2026-08-02 20:53 name"
        std::istringstream lss(line);
        std::string perm;
        lss >> perm;
        if (perm.empty() || perm[0] != '-') continue;  // skip dirs / "total" / blank
        std::string links, owner, group, size_str;
        if (!(lss >> links >> owner >> group >> size_str)) continue;
        long long size = 0;
        try {
            size = std::stoll(size_str);
        } catch (...) {
            continue;
        }
        // Skip the date/time fields (ISO = 2, traditional = 3; see header).
        std::string date_or_mon;
        if (!(lss >> date_or_mon)) continue;
        std::string discard;
        if (date_or_mon.find('-') != std::string::npos) {
            lss >> discard;              // time (HH:MM)
        } else {
            lss >> discard >> discard;   // day, time
        }
        std::string name;
        std::getline(lss, name);
        const auto first = name.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        const auto last = name.find_last_not_of(" \t\r\n");
        name = name.substr(first, last - first + 1);
        // Only track files matching the expected name pattern; ignore
        // unrelated files that may exist in the same device directory.
        if (name.rfind(name_prefix, 0) != 0) continue;
        if (!name_suffix.empty() &&
            (name.size() < name_suffix.size() ||
             name.compare(name.size() - name_suffix.size(), name_suffix.size(),
                          name_suffix) != 0)) {
            continue;
        }
        files[name] = size;
    }
    return files;
}

// After the (near-instant) snapshot command returns, poll `result_dir` for
// newly-generated (or size-changed) heap files until their sizes stabilize,
// then pull each via `hdc file recv` into `target_dir`, the session directory
// of the capture that produced it. `role` tags the pulled files with their
// position in a continuous session (start / peak / end); it is empty for a
// one-shot export and for captures that don't distinguish roles.
// `baseline` is the file->size map captured before the command ran; a file is a "candidate" when
// it is absent from the baseline or its size differs. A candidate is deemed
// complete when its size is unchanged across two consecutive polls (and > 0).
// Polls every 2s; gives up after `timeout_s`. Only complete files are pulled
// (per "pull only after completion"). The local paths of the pulled files are
// appended to `files` and a human-readable outcome is written to `status`.
void poll_and_pull_snapshot(const std::string& result_dir,
                            const std::string& target_dir,
                            const std::map<std::string, long long>& baseline,
                            long long timeout_s,
                            std::vector<std::string>& files,
                            std::string& status,
                            const std::string& role = "") {
    const int interval_s = 2;
    auto start = clock_type::now();

    std::map<std::string, long long> prev_sizes;  // candidates' sizes from prior poll
    std::map<std::string, long long> stable;      // candidates confirmed complete
    int poll_num = 0;

    while (stable.empty()) {
        if (device_lost()) break;   // pinned target: device-level failure -> stop polling now
        ++poll_num;
        const long long elapsed_s = elapsed_since(start);
        if (elapsed_s >= timeout_s) break;

        const auto current = list_dir_file_sizes(result_dir, "hidumper-jsheap");
        // Candidates: new files or files whose size changed vs baseline.
        std::map<std::string, long long> candidates;
        for (const auto& kv : current) {
            auto it = baseline.find(kv.first);
            if (it == baseline.end() || it->second != kv.second) {
                candidates[kv.first] = kv.second;
            }
        }

        if (candidates.empty()) {
            std::cout << "[Snapshot check] #" << poll_num << " (" << elapsed_s
                      << "s): no new files\n"
                      << std::flush;
            prev_sizes.clear();
        } else {
            bool all_stable = true;
            for (const auto& kv : candidates) {
                auto pit = prev_sizes.find(kv.first);
                const bool s = (pit != prev_sizes.end() && pit->second == kv.second &&
                                kv.second > 0);
                if (!s) all_stable = false;
                std::cout << "[Snapshot check] #" << poll_num << " (" << elapsed_s
                          << "s): " << kv.first << " " << (kv.second / 1024) << "KB"
                          << (s ? " stable" : " generating") << "\n"
                          << std::flush;
            }
            if (all_stable) {
                stable = candidates;
                break;
            }
            prev_sizes = candidates;
        }

        // Sleep `interval_s` (clamped to the remaining timeout), in 100ms ticks
        // so we don't overshoot the deadline by much.
        const long long remain = timeout_s - elapsed_s;
        const int sleep_s = static_cast<int>(std::min<long long>(interval_s, remain));
        sleep_ticks(sleep_s * 10, []() { return false; });
    }

    if (stable.empty()) {
        status = "Timeout: snapshot file generation not detected as complete (" + result_dir + ")";
        return;
    }

    // Pull the completed snapshot file(s) into the capture's own directory so
    // results don't land flat in the top-level output dir (mirrors the analysis
    // subcommands).
    std::error_code ec_mkdir;
    fs::create_directories(path_from_utf8(target_dir), ec_mkdir);

    // Pull each completed file (remote path with a trailing slash).
    std::string dir = result_dir;
    if (!dir.empty() && dir.back() != '/') dir += "/";
    const size_t pulled_before = files.size();
    for (const auto& kv : stable) {
        const std::string remote = dir + kv.first;
        PullOutcome po = pull_file(remote, target_dir);
        if (po.ok) {
            files.push_back(mark_snapshot_role(local_recv_path(target_dir, remote), role));
        } else {
            log::loge("Snapshot file export failed: ", remote, " (exit code ", po.exit_code, ")\n", po.output);
        }
    }
    status = files.size() > pulled_before ? "complete" : "snapshot file export failed";
}

// ── Automated app launch / fresh-restart via the `aa` tool ─────────────

// Parse the entry ability name of an app from `bm dump -n <package>` output.
// Tries the FA-model "mainAbility" and Stage-model "mainElementName" JSON
// fields. `bm dump` output can contain multiple occurrences of a key where the
// top-level one is empty (e.g. `"mainAbility": ""` appears first, then the real
// `"mainAbility": "<name>"` in a module section), so we scan for the first
// NON-empty value rather than stopping at the first match.
// Returns an empty string when the output is unavailable or unparsable.
std::string get_device_main_ability(const std::string& package_name) {
    std::string cmd = hdc_shell("bm dump -n " + proc::shell_quote_device(package_name)) + " 2>&1";
    CapturedCmd cap = capture_command(cmd, 15000);
    if (note_device_failure(cap.output)) return "";
    if (!cap.spawned) return "";

    // Return the first non-empty string value for `"<key>": "<value>"`,
    // scanning every occurrence of the key in the output.
    auto extract_value = [](const std::string& text, const std::string& key) -> std::string {
        const std::string needle = "\"" + key + "\"";
        std::string::size_type pos = 0;
        while ((pos = text.find(needle, pos)) != std::string::npos) {
            pos += needle.size();
            const auto colon = text.find(':', pos);
            if (colon == std::string::npos) break;
            const auto vstart = text.find('"', colon + 1);
            if (vstart == std::string::npos) break;
            const auto vend = text.find('"', vstart + 1);
            if (vend == std::string::npos) break;
            std::string value = text.substr(vstart + 1, vend - vstart - 1);
            if (!value.empty()) return value;
            pos = vend + 1;  // empty value; keep scanning for a non-empty one
        }
        return "";
    };

    std::string ability = extract_value(cap.output, "mainAbility");
    if (ability.empty()) ability = extract_value(cap.output, "mainElementName");
    return ability;
}

// Launch an app on the device via `aa start -a <ability> -b <package>`.
// The entry ability is resolved from `bm dump -n`; common defaults
// ("EntryAbility" / "MainAbility") are tried as fallbacks. Returns true when
// the device reports a successful start.
bool start_device_app(const std::string& package_name) {
    std::vector<std::string> abilities;
    const std::string resolved = get_device_main_ability(package_name);
    if (!resolved.empty()) {
        abilities.push_back(resolved);
    } else {
        log::logw("Warning: unable to resolve the entry ability of '", package_name, "' via 'bm dump -n'; trying common defaults.");
    }
    abilities.push_back("EntryAbility");
    abilities.push_back("MainAbility");

    for (const auto& ability : abilities) {
        const std::string device_cmd = "aa start -a " + proc::shell_quote_device(ability)
                                     + " -b " + proc::shell_quote_device(package_name);
        CapturedCmd cap = capture_command(
            hdc_shell(device_cmd) + " 2>&1", 15000);
        if (!cap.spawned) return false;
        const bool ok = cap.output.find("start ability successfully") != std::string::npos &&
                        cap.output.find("error: failed") == std::string::npos;
        std::cout << "  " << device_cmd << "\n" << cap.output << std::flush;
        if (ok) return true;
    }
    return false;
}

// Force-stop an app on the device via `aa force-stop <package>`.
// Harmless when the app is not running; device output is printed if any.
void force_stop_device_app(const std::string& package_name) {
    const std::string device_cmd = "aa force-stop " + proc::shell_quote_device(package_name);
    CapturedCmd cap = capture_command(
        hdc_shell(device_cmd) + " 2>&1", 15000);
    if (cap.spawned && !cap.output.empty()) {
        std::cout << "  " << device_cmd << "\n" << cap.output << std::flush;
    }
}

// Launch an app and wait (polling `pidof`) until its process is up.
// Returns true when the process appears within the wait budget (~30s).
bool launch_device_app_and_wait(const std::string& package_name) {
    if (!start_device_app(package_name)) return false;
    for (int i = 0; i < 60; ++i) {  // 500ms ticks
        if (!get_process_pid_fast(package_name, 3000).empty()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return false;
}

// Device-side path where the hiprofiler config file is pushed via hdc file send.
constexpr const char* kDeviceConfigPath = "/data/local/tmp/memory_analyzer_hiprofiler.config";

// Device directory where hiprofiler writes its result zip files.
constexpr const char* kHiprofilerZipDir = "/data/local/tmp";

// Detect async "hiprofiler_cmd start" mode: `start` captures on the device and
// we must explicitly run `hiprofiler_cmd stop` when the countdown expires to
// finish the capture. Returns true when the command contains a
// `hiprofiler_cmd ... start` token sequence.
bool detect_async_start_mode(const std::string& content) {
    std::istringstream iss(content);
    std::string tok;
    bool seen_hiprofiler_cmd = false;
    while (iss >> tok) {
        if (seen_hiprofiler_cmd && tok == "start") return true;
        if (tok == "hiprofiler_cmd") seen_hiprofiler_cmd = true;
    }
    return false;
}

// Push the hiprofiler device-config content to the device via a local temp
// file + `hdc file send`, BEFORE executing the command (avoids Windows
// cmd.exe's ~8191-char command-line length limit for long configs). On success
// `local_config_file` holds the temp path (used later for the log entry and
// cleanup). Returns false and sets result.ok=false with a description on any
// failure.
bool push_device_config(const std::string& device_config_content,
                        std::string& local_config_file, RunResult& result) {
    if (device_config_content.empty()) return true;
    local_config_file = path_to_utf8(
        fs::temp_directory_path() /
        ("mem_analyzer_config_" + current_timestamp() + ".cfg"));
    std::ofstream cfg_ofs(path_from_utf8(local_config_file), std::ios::binary);
    if (!cfg_ofs.is_open()) {
        result.ok = false;
        result.error = "failed to create local temp config file";
        return false;
    }
    cfg_ofs << device_config_content;
    cfg_ofs.close();

    std::string send_cmd = hdc_file_send(local_config_file, kDeviceConfigPath) + " 2>&1";
    CapturedCmd send_cap = capture_command(send_cmd);
    if (!send_cap.spawned || send_cap.exit_code != 0) {
        result.ok = false;
        result.error = "failed to push config file to device (exit code " +
                       std::to_string(send_cap.exit_code) + ")";
        if (!send_cap.output.empty()) {
            result.error += ": " + send_cap.output;
        }
        std::error_code ec;
        fs::remove(path_from_utf8(local_config_file), ec);
        return false;
    }
    return true;
}

// Run `hiprofiler_cmd stop` on the device and report progress. Can take a
// while (stopping sessions, flushing the data file), so it runs in a
// background thread with a periodic wait-time hint — otherwise the user may
// think the tool is stuck.
void stop_hiprofiler() {
    std::atomic<bool> stop_done{false};
    CapturedCmd stop_cap;
    std::thread stop_worker([&]() {
        std::string stop_cmd = hdc_shell("hiprofiler_cmd stop") + " 2>&1";
        stop_cap = capture_command(stop_cmd);
        stop_done = true;
    });

    const auto stop_start = clock_type::now();
    while (!stop_done.load()) {
        const long long wait = elapsed_since(stop_start);
        // Only show progress once the wait becomes non-trivial.
        if (wait >= 5) {
            std::cout << "Waiting for hiprofiler to stop... (" << wait << "s)\n"
                      << std::flush;
        }
        // Sleep up to 5s, waking every 100ms so we react to completion.
        sleep_ticks(50, [&]() { return stop_done.load(); });
    }
    stop_worker.join();

    if (!stop_cap.output.empty()) {
        std::cout << stop_cap.output;
        if (stop_cap.output.back() != '\n') std::cout << '\n';
    }
    std::cout << std::flush;
}

// After `hdc shell` returns, hiprofiler may still be writing the result zip
// asynchronously for a few seconds. Poll `dir` for up to `timeout_s` seconds
// for a NEW hiprofiler_data_*.zip (absent from `baseline`) whose size is
// stable across two consecutive samples; the embedded filename timestamp is
// the authoritative indicator that hiprofiler finished writing it. Returns
// the full device path of the detected zip, or an empty string when none was
// found (the caller then falls back to the -o file).
std::string wait_for_hiprofiler_zip(const std::map<std::string, long long>& baseline,
                                    const std::string& dir, long long timeout_s) {
    std::cout << "Waiting for hiprofiler_data zip (up to " << timeout_s << "s)...\n"
              << std::flush;

    long long prev_size = -1;
    int stable_count = 0;
    auto poll_start = clock_type::now();

    for (int i = 0; i < timeout_s; ++i) {
        if (device_lost()) break;   // pinned target: device-level failure -> stop polling now
        const long long elapsed_poll = elapsed_since(poll_start);
        if (elapsed_poll >= timeout_s) break;

        // Re-scan the directory for any hiprofiler_data_*.zip files.
        const auto current_zips = list_hiprofiler_zips(dir);

        std::string candidate;
        long long candidate_ts = -1;
        // Look for a NEW file not in baseline; among new files pick the one
        // with the largest embedded timestamp (most likely produced by this
        // run).
        for (const auto& kv : current_zips) {
            if (baseline.count(kv.first)) continue;
            if (kv.second > candidate_ts) {
                candidate = kv.first;
                candidate_ts = kv.second;
            }
        }

        if (candidate.empty()) {
            prev_size = -1;
            stable_count = 0;
            std::cout << "[hiprofiler wait] #" << i
                      << " (" << elapsed_poll << "s): no new hiprofiler_data zip yet\n"
                      << std::flush;
        } else {
            const auto sizes = list_dir_file_sizes(dir, "hiprofiler_data_", ".zip");
            long long cur_size = 0;
            auto sit = sizes.find(candidate);
            if (sit != sizes.end()) cur_size = sit->second;

            if (cur_size > 0 && cur_size == prev_size) {
                ++stable_count;
            } else {
                stable_count = (cur_size > 0) ? 1 : 0;
            }
            prev_size = cur_size;

            std::cout << "[hiprofiler wait] #" << i
                      << " (" << elapsed_poll << "s): new zip candidate " << candidate
                      << " size=" << (cur_size / 1024) << "KB"
                      << (stable_count >= 2 ? " stable"
                          : (stable_count == 1 ? " stabilizing" : " generating"))
                      << "\n" << std::flush;

            if (stable_count >= 2) {
                std::cout << "Detected new hiprofiler_data zip after " << i
                          << " polls: " << candidate
                          << " (will prefer this over -o file)\n" << std::flush;
                return make_device_path(dir, candidate);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return "";
}

// nativeleaks: after the .htrace is pulled, hiprofiler may still be writing the
// Native Leaks orphan-snapshot file (memleak-native-orphansnapshot-<pid>.txt)
// asynchronously. The filename is keyed by the target pid, so we poll that ONE
// file (constructed from `pid`) for up to `timeout_s` seconds until its size is
// stable across two consecutive samples. This sidesteps a directory baseline
// entirely: a re-run that overwrites the same-pid file, and stale files of
// other pids, are both handled correctly because we only ever look at the exact
// file this run produced. Returns the full device path, or empty string when
// the file never appeared (or `pid` is empty).
std::string wait_for_nativeleaks_file(const std::string& pid,
                                      const std::string& dir, long long timeout_s) {
    if (pid.empty()) return "";
    const std::string filename = "memleak-native-orphansnapshot-" + pid + ".txt";
    std::cout << "Waiting for nativeleaks orphan-snapshot file (" << filename
              << ", up to " << timeout_s << "s)...\n" << std::flush;

    long long prev_size = -1;
    int stable_count = 0;
    auto poll_start = clock_type::now();

    for (int i = 0; i < timeout_s; ++i) {
        if (device_lost()) break;
        const long long elapsed_poll = elapsed_since(poll_start);
        if (elapsed_poll >= timeout_s) break;

        // Pass the full filename as the prefix so list_dir_file_sizes matches
        // only that one file (rfind(prefix, 0) == 0), ignoring other pids'.
        const auto current = list_dir_file_sizes(dir, filename);
        long long cur_size = 0;
        const auto it = current.find(filename);
        const bool found = (it != current.end());
        if (found) cur_size = it->second;

        if (!found || cur_size <= 0) {
            prev_size = -1;
            stable_count = 0;
            std::cout << "[nativeleaks wait] #" << i << " (" << elapsed_poll
                      << "s): " << filename << " not ready yet\n" << std::flush;
        } else {
            if (cur_size == prev_size) {
                ++stable_count;
            } else {
                stable_count = 1;
            }
            prev_size = cur_size;

            std::cout << "[nativeleaks wait] #" << i << " (" << elapsed_poll << "s): "
                      << filename << " size=" << (cur_size / 1024) << "KB"
                      << (stable_count >= 2 ? " stable"
                          : (stable_count == 1 ? " stabilizing" : " generating"))
                      << "\n" << std::flush;

            if (stable_count >= 2) {
                std::cout << "Detected nativeleaks orphan-snapshot file after " << i
                          << " polls: " << filename << "\n" << std::flush;
                return make_device_path(dir, filename);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return "";
}

// Record an exported per-process snapshot file (hidumper --mem-smaps / lsof)
// in `result` and report it on the console.
void record_exported_file(const std::string& path, const std::string& smaps_prefix,
                          RunResult& result) {
    result.smaps_exported_files.push_back(path);
    log::logok(smaps_prefix == "fds" ? "FD snapshot exported" : "Smaps exported", ": ", path);
}

// Export an end-of-capture smaps/lsof snapshot of the target process so it can
// be diffed against the start-of-capture export. Called only when the capture
// actually ran (not aborted for lack of permission / failed to start).
void export_end_snapshot(const std::string& pid, const std::string& output_dir,
                         const std::string& smaps_prefix, RunResult& result) {
    if (pid.empty()) {
        log::logw("Warning: target process exited; skipping end-of-capture ", smaps_prefix, " export");
        return;
    }
    const std::string path = export_process_file(pid, output_dir, smaps_prefix);
    if (!path.empty()) {
        record_exported_file(path, smaps_prefix, result);
    } else {
        log::logw("Warning: ", smaps_prefix, " export failed at capture end");
    }
}

// Snapshot flow: the hidumper command returns almost instantly. If its output
// clearly reports insufficient permission (non-root phone + release-signed
// app; the device prints "[Permission Check Failed] ..."), or the command
// exits with a nonzero code, fail immediately instead of idling through the
// 120s poll timeout. Returns true when the snapshot command has been judged
// failed and written into `result`; callers should skip polling then.
bool handle_snapshot_command_failure(RunResult& result) {
    const std::string perr = detect_permission_error(result.output);
    const bool cmd_failed = result.exit_code != 0;
    if (perr.empty() && !cmd_failed) return false;
    result.ok = false;
    result.snapshot_attempted = true;
    if (!perr.empty()) {
        result.error =
            "permission check failed: the target app cannot be tuned on this "
            "device (release-signed app on a non-root phone)";
        result.snapshot_status = "permission check failed: " + perr;
    } else {
        result.error = "snapshot export command failed (exit code " +
                       std::to_string(result.exit_code) + ")";
        result.snapshot_status = result.error;
    }
    return true;
}

// Shared state between the worker / pid checker / fresh-restart / export
// threads and the main (progress) thread inside run_hdc. Heap-allocated so the
// threads can be safely detached when a permission failure aborts the capture
// early: the main flow returns while the threads keep running, and the shared
// state stays alive until the last thread releases it.
struct CaptureState {
    // Command transcript + spawn status.
    std::string output;                 // guarded by mtx
    std::mutex mtx;
    std::atomic<int> exit_code{0};
    std::atomic<bool> done{false};
    std::atomic<bool> spawn_failed{false};

    // Background pid-checker state.
    std::mutex pid_mtx;
    std::string latest_pid;             // guarded by pid_mtx
    ProcessMetrics latest_metrics;      // guarded by pid_mtx
    std::atomic<bool> pid_changed{false};
    std::atomic<bool> stop_pid_check{false};

    // Fresh-restart automation state.
    std::atomic<bool> stop_fresh_restart{false};
    std::atomic<bool> fresh_restart_triggered{false};
    std::atomic<bool> fresh_restart_active{false};

    // Permission probe: set by the first smaps/fds export. The device-side
    // reason is guarded by mtx.
    std::atomic<bool> permission_failed{false};
    std::string permission_error_msg;   // guarded by mtx
};

} // namespace

// Poll whether the target process is running on the device.
// Primary: uses `pidof <name>` — the same command the device-side template
// (snapshot_hidumper_cmd.txt) uses. This is reliable on all HarmonyOS versions.
// Fallback: `ps -ef` for systems where `pidof` is unavailable.
// Returns the PID if found, empty string otherwise.
std::string get_process_pid(const std::string& process_name) {
    if (process_name.empty()) return "";

    // 1) Primary: pidof (same as device-side template, fast & reliable)
    {
        std::string cmd = hdc_shell("pidof " + proc::shell_quote_device(process_name)) + " 2>&1";
        CapturedCmd cap = capture_command(cmd, 5000);
        // Pinned target: an hdc [Fail] means the DEVICE failed
        // (unplugged / unauthorized), not "process not running". Surface the hdc
        // text once and bail out instead of misparsing it as a missing process.
        if (note_device_failure(cap.output)) return "";
        if (cap.spawned && !cap.timed_out) {
            std::istringstream iss(cap.output);
            std::string pid;
            iss >> pid;
            if (!pid.empty() && pid.find_first_not_of("0123456789") == std::string::npos) {
                return pid;
            }
        }
    }

    // 2) Fallback: ps -ef (Linux-style, may not be available on all devices)
    {
        std::string cmd = hdc_shell("ps -ef") + " 2>&1";
        CapturedCmd cap = capture_command(cmd);
        if (note_device_failure(cap.output)) return "";
        if (!cap.spawned) return "";

        std::string suffix_pid;
        std::istringstream iss(cap.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            if (line.find("UID") == 0) continue;

            std::istringstream lss(line);
            std::string pid, tok;
            for (int col = 0; col < 7; ++col) {
                if (!(lss >> tok)) { pid.clear(); break; }
                if (col == 1) pid = tok;
            }
            if (pid.empty()) continue;
            if (pid.find_first_not_of("0123456789") != std::string::npos) continue;
            std::string cmd_field;
            std::getline(lss, cmd_field);
            const auto cmd_start = cmd_field.find_first_not_of(" \t");
            if (cmd_start == std::string::npos) continue;
            cmd_field = cmd_field.substr(cmd_start);

            if (cmd_field == process_name) return pid;

            if (suffix_pid.empty() &&
                cmd_field.size() > process_name.size() &&
                cmd_field.compare(0, process_name.size(), process_name) == 0 &&
                cmd_field[process_name.size()] == ':') {
                suffix_pid = pid;
            }
        }
        return suffix_pid;
    }
}

std::string get_process_pid_fast(const std::string& process_name, long long timeout_ms,
                                 bool* reliable) {
    if (reliable) *reliable = false;
    if (process_name.empty()) return "";
    // Use `pidof` — a very lightweight command that outputs only the PID
    // (a few bytes) or nothing. This is drastically faster than `ps -ef`
    // (which outputs the entire process list, often several KB) and is
    // specifically designed for use in a background thread while another
    // `hdc shell` (hiprofiler_cmd) is running.
    //
    // pidof returns space-separated PIDs (e.g. "12345 12346") on success,
    // or nothing if the process isn't running. We take the first PID.
    std::string cmd = hdc_shell("pidof " + proc::shell_quote_device(process_name)) + " 2>&1";
    CapturedCmd cap = capture_command(cmd, timeout_ms);
    if (!cap.spawned || cap.timed_out) return "";  // unreliable (stalled hdc)
    // Pinned target: a device-level failure is reported once and
    // stops the background checker. `reliable` deliberately stays false so the
    // countdown keeps the last known pid instead of flipping to "not running"
    // (which would be a wrong conclusion: the device is gone, not the process).
    if (note_device_failure(cap.output)) return "";

    // Parse the first whitespace-separated token and validate it's a number.
    std::istringstream iss(cap.output);
    std::string pid;
    iss >> pid;
    if (!pid.empty() && pid.find_first_not_of("0123456789") == std::string::npos) {
        if (reliable) *reliable = true;
        return pid;
    }
    // Empty output means pidof ran cleanly and the process is not running
    // (reliable). Any other output (e.g. "sh: pidof: not found", garbled
    // text) means the query itself failed — mark it unreliable so the caller
    // can keep the previously known PID instead of misreporting.
    const bool blank =
        cap.output.empty() ||
        cap.output.find_first_not_of(" \t\r\n") == std::string::npos;
    if (reliable) *reliable = blank;
    return "";
}

namespace {

// ── capture setup ────────────────────────────────────────────────────────────

// Everything that must happen before the capture threads start: trap Ctrl+C,
// create the output directory, push the device config (hiprofiler) and build
// the final `hdc shell ...` command. Returns false (with `result` describing
// the error) when the capture cannot proceed.
struct CaptureSetup {
    std::string command;             // full "hdc shell <cmd> 2>&1"
    std::string local_config_file;   // temp config to remove on exit
    bool async_start_mode = false;   // "hiprofiler_cmd start" (needs stop)
};

bool setup_capture(const HdcRunOptions& opts, RunResult& result, CaptureSetup& setup) {
    // Trap Ctrl+C so the profiler can stop hiprofiler and save data instead
    // of being killed abruptly. The handler just sets a flag that the capture
    // loop polls.
    g_interrupt_requested = 0;
    std::signal(SIGINT, handle_sigint);

    // Create the output directory (and parents) if missing; ignore existing.
    std::error_code ec;
    fs::create_directories(path_from_utf8(opts.output_dir), ec);

    // If there's device config content (hiprofiler commands), write it to a
    // local temp file and push it to the device via `hdc file send` BEFORE
    // executing the command (avoids Windows cmd.exe's ~8191-character
    // command-line length limit for long hiprofiler configs).
    if (!push_device_config(opts.device_config_content, setup.local_config_file, result)) {
        return false;
    }

    // Shell-quoted arg preserves newlines/backslashes/heredoc for the device
    // shell; 2>&1 merges stderr into stdout.
    setup.command = hdc_shell(opts.content) + " 2>&1";

    // Detect async "hiprofiler_cmd start" mode. With `start`, hiprofiler
    // starts capturing on the device and we must explicitly run
    // `hiprofiler_cmd stop` when the countdown expires to finish the capture.
    // On the device, `hiprofiler_cmd start` does NOT return by itself — it
    // stays attached to the shell until `stop` runs — so we run it in the
    // FOREGROUND and let the worker block until stop releases it (a second
    // hdc connection running `stop` causes the start command to exit). Running
    // it in the foreground also makes its output — including startup errors
    // like "Failed to start profiling session" — stream through hdc to us
    // instead of being lost.
    setup.async_start_mode = detect_async_start_mode(opts.content);
    return true;
}

// ── pre-capture: baselines + initial process state ───────────────────────────

// State gathered before the worker occupies the hdc connection: device result
// dir baseline (snapshot mode), hiprofiler zip baseline (-g/-l) and the
// initial pid / metrics / running status of the target process.
struct PreCaptureState {
    bool was_running_at_start = false;
    std::string initial_pid;
    ProcessMetrics initial_metrics;
    std::map<std::string, long long> result_baseline;          // snapshot mode
    std::map<std::string, long long> hiprofiler_zip_baseline;  // -g / -l
};

// Snapshot flow: the target process must be running before we attempt the
// capture, and we capture the device result dir BEFORE the command runs so we
// can later identify the files this run generated (new / size-changed).
// Returns false when the capture must abort (e.g. snapshot target not
// running); the error is written into `result`.
bool prepare_capture_start(const HdcRunOptions& opts, bool snapshot_mode,
                           PreCaptureState& pre, RunResult& result) {
    if (snapshot_mode) {
        if (!opts.process_name.empty()) {
            const std::string pid = get_process_pid(opts.process_name);
            if (pid.empty()) {
                result.ok = false;
                result.error = "target process '" + opts.process_name +
                               "' is not running; please start the app first";
                return false;
            }
            log::logi("Target process '", opts.process_name, "' is running, pid:", pid);
            pre.initial_pid = pid;
        }
        pre.result_baseline = list_dir_file_sizes(opts.result_dir, "hidumper-jsheap");
        return true;
    }

    // hiprofiler flow baseline (only for -g / -l via wait_for_zip): capture
    // the pre-existing hiprofiler_data_*.zip files in /data/local/tmp BEFORE
    // the command runs. After hdc shell returns hiprofiler may still be writing
    // the zip asynchronously, so we detect "hiprofiler has finished" by polling
    // for a NEW zip file that was not present at startup (the embedded
    // timestamp inside the filename is the authoritative indicator of
    // hiprofiler completion). Other capture types (-n/-f/-d) don't produce a
    // hiprofiler_data_*.zip, so they skip this and just pull the -o file.
    if (!opts.remote_output_path.empty() && opts.wait_for_zip) {
        pre.hiprofiler_zip_baseline = list_hiprofiler_zips(kHiprofilerZipDir);
    }

    // hiprofiler mode: do the initial process check BEFORE starting the worker
    // thread. Once the worker's `hdc shell 'hiprofiler_cmd ...'` is running, it
    // may occupy the hdc connection and serialize other `hdc shell` calls.
    //
    // We check the process status once here (when hdc is still free). During
    // the countdown, a background thread periodically tries to refresh the pid
    // — if hdc is free, the pid updates in real-time; if hdc serializes, the
    // background thread blocks harmlessly and the countdown uses the last known
    // pid. Either way, the main countdown thread never blocks.
    if (opts.duration <= 0 || opts.process_name.empty()) return true;

    // Automated app-state management (all capture types except -s):
    //   - app not running, no fresh-start need (-n/-g/-f/-d): launch now
    //     via `aa start` so it is up when the countdown begins.
    //   - fresh-start (-l), app running: force-stop + relaunch is deferred
    //     until the capture session is up (the hook library must load at
    //     app startup to be traced).
    //   - fresh-start (-l), app not running: launch is deferred the same way.
    //   - app running, no fresh-start need: left untouched.
    const std::string running_pid = get_process_pid(opts.process_name);
    const bool was_running = !running_pid.empty();
    if (was_running) {
        log::logi("Target process '", opts.process_name, "' is running, pid:", running_pid);
    } else {
        log::logw("Target process '", opts.process_name, "' is not running.");
    }

    if (!was_running && !opts.require_fresh_start) {
        if (opts.is_app) {
            log::logi("Auto-launching '", opts.process_name, "' via aa start...");
            if (launch_device_app_and_wait(opts.process_name)) {
                log::logi("'", opts.process_name, "' is running.");
            } else {
                log::logw("Warning: failed to auto-launch '", opts.process_name, "'; please start it manually.");
            }
        } else {
            // System process: never auto-launch; keep the original hint.
            log::logi("Please start it as soon as possible.");
        }
    } else if (opts.require_fresh_start) {
        log::logi("This capture type requires a FRESH START: the app will be force-stopped "
                  "and relaunched automatically once the capture session is ready.");
    }

    pre.initial_pid = get_process_pid(opts.process_name);
    pre.was_running_at_start = !pre.initial_pid.empty();
    if (!was_running && pre.was_running_at_start) {
        log::logi("Target process '", opts.process_name, "' is now running, pid:", pre.initial_pid);
    }
    // Collect an initial sample of the process metrics while hdc is still
    // free (before the worker occupies the connection), so the first
    // countdown tick can show them immediately.
    if (pre.was_running_at_start) {
        CapturedCmd mem_cap = capture_command(
            hdc_shell("hidumper --mem " + proc::shell_quote_device(pre.initial_pid)) + " 2>&1");
        pre.initial_metrics = parse_hidumper_mem(mem_cap.output);
        CapturedCmd lsof_cap = capture_command(
            hdc_shell("lsof -p " + proc::shell_quote_device(pre.initial_pid)) + " 2>&1");
        pre.initial_metrics.fd_count = count_lsof_fds(lsof_cap.output);
    }
    std::cout << std::flush;
    return true;
}

// ── background threads ───────────────────────────────────────────────────────

// Worker thread: runs the hdc command and streams its output into the shared
// `output` buffer as it arrives (instead of capturing it all once at the
// end), so the countdown loop can surface the device-side `hiprofiler_cmd
// start` transcript in real time. In async start mode the foregrounded
// command stays attached until `stop` runs, so this thread only finishes
// after the countdown's stop releases it.
std::thread spawn_worker(const std::shared_ptr<CaptureState>& state,
                         const std::string& cmd, bool async_start_mode) {
    return std::thread([state, cmd, async_start_mode]() {
        // 逐行消费子进程输出：写入共享缓冲，async 模式下即时回显到控制台。
        auto consume_stream = [&](FILE* fp) {
            char buf[4096];
            while (fgets(buf, sizeof(buf), fp) != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(state->mtx);
                    state->output.append(buf);
                }
                // Async start mode: stream the `hiprofiler_cmd start` transcript
                // to the console line-by-line, the moment it arrives. The device
                // may delay part of the output (e.g. a startup error ~1s in), so
                // batching would make errors feel missing — print immediately,
                // exactly like running the command in a terminal window.
                if (async_start_mode) {
                    std::cout << console_start_line(buf) << std::flush;
                }
            }
        };
#if defined(_WIN32)
        // 无 shell 执行：CreateProcessW 直接解析命令行（MSVCRT 规则，不经 cmd.exe），
        // 宽字符避免中文进程名的 ANSI 乱码；管道合并 stderr 到 stdout，
        // 消除 cmd.exe 对 `&|<>^` 等元字符的解释（主机命令注入）。
        SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        HANDLE h_read = nullptr, h_write = nullptr;
        if (!CreatePipe(&h_read, &h_write, &sa, 0)) {
            state->spawn_failed = true; state->done = true; return;
        }
        if (!SetHandleInformation(h_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
            CloseHandle(h_read); CloseHandle(h_write);
            state->spawn_failed = true; state->done = true; return;
        }
        STARTUPINFOW si{ sizeof(STARTUPINFOW) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = h_write;
        si.hStdError = h_write;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION pi{};

        // 去掉尾部 " 2>&1"：stderr 已由上面的 STARTF_USESTDHANDLES 合并进 stdout。
        std::string clean_cmd = cmd;
        if (clean_cmd.size() >= 5 && clean_cmd.substr(clean_cmd.size() - 5) == " 2>&1") {
            clean_cmd.resize(clean_cmd.size() - 5);
        }
        std::wstring wcmdline = widen_path(clean_cmd);
        if (!CreateProcessW(nullptr, wcmdline.data(), nullptr, nullptr, TRUE, 0,
                            nullptr, nullptr, &si, &pi)) {
            CloseHandle(h_read); CloseHandle(h_write);
            state->spawn_failed = true; state->done = true; return;
        }
        CloseHandle(h_write);

        // 把管道读端包装成 FILE*，保持 fgets 逐行读取的原语义。
        int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h_read), _O_RDONLY);
        if (fd == -1) {
            CloseHandle(h_read);
            state->spawn_failed = true; state->done = true; return;
        }
        FILE* fp = _fdopen(fd, "r");
        if (!fp) {
            _close(fd);
            state->spawn_failed = true; state->done = true; return;
        }

        consume_stream(fp);
        fclose(fp);  // 关闭 fd，进而关闭底层 h_read 句柄

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        state->exit_code = static_cast<int>(code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        auto sp = proc::spawn_piped(cmd);
        if (!sp.ok()) {
            state->spawn_failed = true; state->done = true; return;
        }
        FILE* fp = fdopen(sp.read_fd, "r");
        if (!fp) {
            close(sp.read_fd);
            state->spawn_failed = true; state->done = true; return;
        }
        consume_stream(fp);
        fclose(fp);
        int st = 0;
        waitpid(sp.pid, &st, 0);
        state->exit_code = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
#endif
        state->done = true;
    });
}

// Background process checker: periodically calls get_process_pid_fast (pidof)
// in a separate thread so the main countdown thread never blocks on hdc.
// Updates shared state (latest_pid, pid_changed) that the countdown loop reads
// without blocking. If hdc serializes shell commands, the background call may
// block for the entire capture — that's fine, the countdown just uses the last
// known pid. The thread is joined after worker.join() (when hdc is freed), so
// any blocked call completes quickly at that point.
std::thread spawn_pid_checker(const std::shared_ptr<CaptureState>& state,
                              const std::string& process_name,
                              const std::string& initial_pid,
                              const ProcessMetrics& initial_metrics,
                              bool ignore_done) {
    // Seed the shared pid/metrics state with the pre-countdown sample so the
    // first countdown tick can display it immediately (before the background
    // checker's first hdc round-trip lands).
    state->latest_pid = initial_pid;
    state->latest_metrics = initial_metrics;
    return std::thread([state, process_name, ignore_done]() {
        while (!state->stop_pid_check.load() &&
               !device_lost() &&
               (!state->done.load() || ignore_done)) {
            // Use pidof (get_process_pid_fast), NOT ps -ef: the full ps
            // output stalls (no EOF) on a second hdc connection while the
            // foregrounded hiprofiler_cmd capture is running, leaving the
            // countdown stuck on "not running" even after the app starts.
            // A per-call timeout lets the check recover from a stalled
            // hdc session and retry on the next tick.
            // A stalled/failed pidof query returns "" with reliable=false:
            // keep the previously known PID instead of flipping the
            // countdown to "not running" for one tick (the hdc connection
            // is busy with the foregrounded capture, so concurrent
            // queries can briefly fail).
            bool reliable = false;
            std::string pid = get_process_pid_fast(process_name, 5000, &reliable);
            // Publish the pid FIRST so the countdown always has a fresh
            // process status; the (slower) metrics below never delay it.
            {
                std::lock_guard<std::mutex> lock(state->pid_mtx);
                if (!pid.empty()) {
                    if (pid != state->latest_pid) {
                        state->pid_changed = true;
                    }
                    state->latest_pid = pid;
                } else if (reliable || state->latest_pid.empty()) {
                    // pidof confirmed the process is gone, or we never
                    // knew a PID: publish the empty result.
                    if (!state->latest_pid.empty()) {
                        state->pid_changed = true;
                    }
                    state->latest_pid.clear();
                }
                // Unreliable query with a known PID: keep the old value.
            }
            ProcessMetrics metrics;
            if (!pid.empty()) {
                // Gather live memory/fd metrics for the countdown display.
                // Ran outside the lock because hdc calls may block.
                metrics = sample_process_metrics(pid);
            }
            {
                std::lock_guard<std::mutex> lock(state->pid_mtx);
                // Only publish a sample that actually parsed. A stalled
                // hidumper/lsof call (the hdc connection is busy with the
                // foregrounded capture) yields an invalid sample; dropping
                // it keeps the last good metrics so the countdown keeps
                // showing the PSS line instead of going blank for a few
                // ticks.
                if (metrics.valid) {
                    state->latest_metrics = metrics;
                }
            }
            // Sleep 5s between checks, in 100ms ticks so we stop promptly.
            sleep_ticks(50, [state, ignore_done]() {
                return state->stop_pid_check.load() ||
                       device_lost() ||
                       (state->done.load() && !ignore_done);
            });
        }
    });
}

// Fresh-start automation thread (-l): once the hiprofiler session is up,
// force-stop and relaunch the target app so its startup allocations are
// captured (nativehook with startup_mode hooks the process at startup).
// Runs in a background thread with a bounded wait so a stalled device query
// cannot freeze the countdown.
std::thread spawn_fresh_restart(const std::shared_ptr<CaptureState>& state,
                                const std::string& process_name) {
    // The countdown must not nag about a manual fresh start — the automation
    // below handles it.
    state->fresh_restart_triggered = true;
    return std::thread([state, process_name]() {
        // Wait for the capture session to come up (the start command prints
        // "Use 'hiprofiler_cmd stop' ..." when ready), or abort if the start
        // fails or the countdown ends first.
        // Also abort as soon as the target device is gone (pinned target): the
        // readiness marker can never arrive from a lost device, so waiting out the
        // full 60s would only delay the teardown and print a misleading "session did
        // not become ready" warning. Untargeted runs never set the flag.
        bool session_up = false;
        for (int i = 0; i < 120 && !state->stop_fresh_restart.load() && !device_lost(); ++i) {
            bool marker = false;
            bool failed = false;
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                marker = state->output.find("Use 'hiprofiler_cmd stop'") != std::string::npos;
                failed = state->output.find("Failed to start profiling session") != std::string::npos ||
                         state->output.find("MakeCreateRequest failed") != std::string::npos;
            }
            if (marker) { session_up = true; break; }
            if (failed) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (!session_up) {
            log::logw("Warning: capture session did not become ready; please restart '", process_name, "' manually if needed.");
            return;
        }
        state->fresh_restart_active = true;
        log::logi("Capture session ready; force-stopping and relaunching '",
                  process_name, "' for a fresh start...");
        force_stop_device_app(process_name);
        // Wait for the process to actually exit before relaunching.
        for (int i = 0; i < 20; ++i) {  // up to 10s
            if (get_process_pid_fast(process_name, 3000).empty()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (launch_device_app_and_wait(process_name)) {
            log::logi("'", process_name, "' relaunched (fresh start).");
        } else {
            log::logw("Warning: failed to relaunch '", process_name, "'; please restart it manually.");
        }
        state->fresh_restart_active = false;
    });
}

// ── countdown display helpers ────────────────────────────────────────────────

// Did the device-side `hiprofiler_cmd start` report a failed session? Its
// exit code is useless (it returns 0 even on failure), so detect it from the
// transcript instead.
bool capture_start_failed(const std::shared_ptr<CaptureState>& state) {
    std::lock_guard<std::mutex> lock(state->mtx);
    return state->output.find("Failed to start profiling session") != std::string::npos ||
           state->output.find("MakeCreateRequest failed") != std::string::npos;
}

// Print the live memory/fd metrics line for the countdown. kB values are
// shown as MB with 2 decimals.
void print_metrics_line(const ProcessMetrics& metrics) {
    auto fmt_mb = [](long long kb) -> std::string {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", kb / 1024.0);
        return buf;
    };
    // Two lines, indented under the "Remaining ...s" line: the core
    // categories, then the secondary ones plus fd count, so a single
    // countdown tick stays readable on a normal terminal width.
    std::cout << "    PSS: " << fmt_mb(metrics.pss_kb)
              << " MB | Native heap: " << fmt_mb(metrics.native_heap_kb)
              << " MB | ArkTS heap: " << fmt_mb(metrics.arkts_heap_kb)
              << " MB | DMA(ION): " << fmt_mb(metrics.dma_graph_kb)
              << " MB | AnonPage other: " << fmt_mb(metrics.anon_page_other_kb)
              << " MB | GL: " << fmt_mb(metrics.gl_kb)
              << " MB\n"
              << "    ArkWeb-pa heap: " << fmt_mb(metrics.arkweb_pa_kb)
              << " MB | .so: " << fmt_mb(metrics.so_kb)
              << " MB | .hap: " << fmt_mb(metrics.hap_kb)
              << " MB | FilePage other: " << fmt_mb(metrics.file_page_other_kb)
              << " MB | FD: "
              << (metrics.fd_count >= 0 ? std::to_string(metrics.fd_count) : "-")
              << "\n"
              << std::flush;
}

// Export a start-of-capture smaps/lsof snapshot once the process is running
// (immediately, or after a fresh start is detected via pid_changed). The
// device query runs in a background thread so the countdown is never frozen.
// The thread outlives this function, so it captures output_dir / smaps_prefix
// / pid by value; it only references `result` (alive until the caller joins
// the thread) and the heap-allocated `state`.
void maybe_start_snapshot_export(const std::shared_ptr<CaptureState>& state,
                                 const std::string& output_dir,
                                 const std::string& smaps_prefix,
                                 bool require_fresh_start,
                                 const std::string& pid,
                                 bool& start_exported,
                                 std::thread& start_export_thread,
                                 RunResult& result) {
    if (start_exported) return;
    if (require_fresh_start && !state->pid_changed.load()) return;
    start_exported = true;
    const std::string pid_copy = pid;
    start_export_thread = std::thread([state, pid_copy, output_dir, smaps_prefix, &result]() {
        std::string perr;
        const std::string path =
            export_process_file(pid_copy, output_dir, smaps_prefix, &perr);
        if (!perr.empty()) {
            // First export already lacks permission: record the device-side
            // reason; once the countdown loop sees permission_failed it aborts
            // the whole capture.
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                state->permission_error_msg = perr;
            }
            state->permission_failed = true;
        } else if (!path.empty()) {
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                result.smaps_exported_files.push_back(path);
            }
            log::logok(smaps_prefix == "fds" ? "FD snapshot exported" : "Smaps exported",
                       ": ", path);
        } else {
            log::logw("Warning: failed to export ", smaps_prefix, " at capture start");
        }
    });
}

// ── main wait loop ───────────────────────────────────────────────────────────

// Wait while showing progress.
//   snapshot: the hidumper command is near-instant, so there is no countdown —
//     just wait for it; the result-dir polling below is where the user-visible
//     waiting happens. `-t` caps that polling wait.
//   hiprofiler: print a "Remaining Xs, target process ..." countdown every 5s.
//     Process status comes from the background pid_checker thread
//     (non-blocking read of shared state).
void wait_for_capture(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
                      clock_type::time_point start, const PreCaptureState& pre,
                      bool async_start_mode, bool snapshot_mode,
                      bool& start_exported, std::thread& start_export_thread,
                      RunResult& result) {
    if (snapshot_mode) {
        log::logi("Running snapshot export command...");
        while (!state->done.load() && !g_interrupt_requested &&
               !device_lost()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return;
    }
    if (opts.duration <= 0) return;

    int round_no = 0;  // monotonically increasing per-tick sequence number
    while (true) {
        const long long remain = opts.duration - elapsed_since(start);
        if (remain <= 0) break;
        if (g_interrupt_requested) {
            log::logw("Interrupted (Ctrl+C); stopping hiprofiler and saving data...");
            break;
        }
        // Pinned target: the target device failed at the hdc level (e.g. unplugged
        // mid-capture). Stop the countdown now instead of idling until the timeout —
        // note_device_failure() already printed the hdc text once. Untargeted runs
        // never set this flag, so their behaviour is unchanged.
        if (device_lost()) {
            // Name the device: the failure belongs to one target, and by the time the line
            // is read the `Device:` echo is far above (or the log of a second console is
            // interleaved with this one). device_lost() is only ever true while a key is
            // pinned (note_device_failure() ignores "[Fail]" otherwise), so the key is
            // non-empty here and the wording always reads correctly.
            log::logw("Device failure detected on ", hdc_connect_key(), "; stopping hiprofiler and saving data...");
            break;
        }
        // Sync mode: the hiprofiler_cmd runs until it finishes on its own
        // (done drives the loop). Async start mode: the command stays
        // attached until we send stop, so the countdown is time-driven.
        if (!async_start_mode && state->done.load()) break;
        // If `start` failed to spawn, exited on its own (a start that fails
        // exits immediately; a successful one stays attached until stop), or
        // its transcript reports failure, abort the countdown instead of
        // waiting out the full duration.
        if (async_start_mode &&
            (state->spawn_failed || (state->done.load() && capture_start_failed(state)))) {
            break;
        }
        // The first smaps/lsof export found a permission problem: abort the
        // capture immediately instead of idling through the countdown (a
        // release app on a non-root phone cannot be tuned; further captures
        // would only produce meaningless data).
        if (state->permission_failed.load()) {
            std::string perr;
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                perr = state->permission_error_msg;
            }
            log::loge("\n", perr, "\nThe target app cannot be tuned on this device (release-signed app on a non-root phone); aborting the capture.");
            break;
        }
        if (!opts.process_name.empty()) {
            // Read the latest known pid (non-blocking).
            std::string pid;
            {
                std::lock_guard<std::mutex> lock(state->pid_mtx);
                pid = state->latest_pid;
            }
            const bool running = !pid.empty();

            if (running && opts.require_fresh_start && pre.was_running_at_start &&
                !state->pid_changed.load() && !state->fresh_restart_triggered.load()) {
                std::cout << "[" << ++round_no << "] Remaining " << remain << "s, "
                          << "WARNING: target process '" << opts.process_name << "' is already running!"
                          << " pid:" << pid << "\n"
                          << "         This capture type requires a FRESH START.\n"
                          << "         Please force-stop the app and restart it NOW.\n"
                          << "\n"
                          << std::flush;
            } else {
                std::cout << "[" << ++round_no << "] Remaining " << remain << "s, "
                          << (running ? "target process is running pid:" + pid
                                      : (state->fresh_restart_active.load()
                                             ? "target process is being relaunched by the tool"
                                             : "target process not running, please start it as soon as possible"))
                          << "\n"
                          << std::flush;
                if (running) {
                    // Show live memory/fd metrics gathered by the pid checker
                    // (non-blocking read of shared state).
                    ProcessMetrics metrics;
                    {
                        std::lock_guard<std::mutex> lock(state->pid_mtx);
                        metrics = state->latest_metrics;
                    }
                    if (metrics.valid) {
                        print_metrics_line(metrics);
                    }
                    maybe_start_snapshot_export(state, opts.output_dir, opts.smaps_prefix,
                                                opts.require_fresh_start, pid,
                                                start_exported, start_export_thread, result);
                }
            }
        } else {
            std::cout << "[" << ++round_no << "] Remaining " << remain << "s\n" << std::flush;
        }
        // Sleep up to 5s, waking every 100ms to stop promptly when hdc
        // finishes mid-interval, when the start command fails, when the
        // first smaps/lsof export reports a permission failure, or when the
        // target device fails at the hdc level (pinned target).
        sleep_ticks(50, [&]() {
            return (state->done.load() && (!async_start_mode || capture_start_failed(state))) ||
                   state->permission_failed.load() ||
                   device_lost();
        });
    }
}

// ── shutdown: stop hiprofiler, finalize wait, join/detach threads ───────────

// After the countdown: signal the background threads, join the start-export
// thread, export the end-of-capture smaps/lsof snapshot, then stop hiprofiler
// (async mode) and wait for the worker to finish before joining it. The end
// snapshot runs BEFORE `hiprofiler_cmd stop`: stop flushes buffers and
// finalizes the session, and its device-side cleanup would delay the export
// and skew the sampled state. A permission failure aborts the capture early
// instead — the background threads are detached and finish on their own (they
// only reference the heap-allocated shared state, so this is safe).
void stop_and_join_threads(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
                           clock_type::time_point start, bool async_start_mode,
                           std::thread& worker, std::thread& pid_checker,
                           std::thread& fresh_restart_thread,
                           std::thread& start_export_thread, RunResult& result) {
    // Signal the background pid checker and fresh-start automation to stop.
    state->stop_pid_check = true;
    state->stop_fresh_restart = true;

    // The start-of-capture snapshot export runs concurrently with the countdown
    // (see above); join it here so the exported files list is complete and no
    // thread outlives run_hdc's locals. It finishes within a couple of seconds
    // (a single hdc query), so this never meaningfully delays the stop.
    if (start_export_thread.joinable()) {
        start_export_thread.join();
    }

    // Run `hiprofiler_cmd stop` on the device. In async start mode the
    // foregrounded `hiprofiler_cmd start` stays attached to the worker (blocked
    // inside the worker pipe) until the stop releases it, so stop must run BEFORE joining
    // the worker — joining first would deadlock. `hiprofiler_cmd stop` also
    // finishes the capture and writes out the result file.
    //
    // The end-of-capture smaps/lsof snapshot must be exported BEFORE stop runs:
    // stop flushes buffers and finalizes the session, and the device-side
    // cleanup it triggers would both delay the export and skew the sampled
    // state (the capture session is no longer the one the user profiled).
    // Skipped on a permission failure (the capture was already aborted).
    auto export_end_snapshot_before_stop = [&]() {
        if (state->permission_failed.load()) return;
        std::string pid;
        {
            std::lock_guard<std::mutex> lock(state->pid_mtx);
            pid = state->latest_pid;
        }
        export_end_snapshot(pid, opts.output_dir, opts.smaps_prefix, result);
    };
    if (async_start_mode) {
        if (!state->spawn_failed) {
            if (g_interrupt_requested) {
                // Ctrl+C: SIGINT killed the local hdc child, but the
                // device-side session keeps capturing — it must still be
                // stopped and saved.
                log::logw("Interrupted; stopping hiprofiler and saving data...");
                export_end_snapshot_before_stop();
                stop_hiprofiler();
            } else if (capture_start_failed(state)) {
                // The device reported a start failure (its exit code is
                // useless — it returns 0 even on failure); the error
                // transcript was already streamed above. Nothing to stop.
                log::loge("`hiprofiler_cmd start` failed (see the error above); skipping stop.");
            } else {
                // Normal completion: start is still attached, stop releases it
                // and finalizes the capture.
                log::logi("Capture duration finished; stopping hiprofiler...");
                export_end_snapshot_before_stop();
                stop_hiprofiler();
            }
        }

        // The start command has now been released (or exited on its own);
        // join the worker and then the pid checker. worker.join() frees the
        // hdc connection, so any blocked pid-checker call completes promptly
        // after. A permission failure aborts the capture early instead; the
        // background threads are detached and finish on their own (they only
        // reference the heap-allocated shared state, so this is safe).
        if (state->permission_failed.load()) {
            worker.detach();
            if (pid_checker.joinable()) pid_checker.detach();
            if (fresh_restart_thread.joinable()) fresh_restart_thread.detach();
        } else {
            worker.join();
            if (pid_checker.joinable()) pid_checker.join();
            if (fresh_restart_thread.joinable()) fresh_restart_thread.join();
        }
        return;
    }

    // After the capture countdown expires, hiprofiler_cmd on the device
    // typically continues running for a while to do post-capture cleanup
    // (StopSession / DestroySession, flush buffers, write the zip file,
    // etc.). The `done` flag is still false at this point, and
    // worker.join() below would block silently — which looks like a hang
    // to the user. So we keep printing a "finalizing" line every 5s with
    // the total elapsed time until the worker thread actually finishes.
    //
    // Snapshot mode is unaffected: its loop above waits on `done` directly,
    // so when we reach here `done` is already true and this block is skipped.
    // A permission failure is also unaffected: the capture was already aborted,
    // so there is nothing to finalize and we must not idle until the
    // device-side command runs out.
    if (!state->done.load() && !g_interrupt_requested &&
        !state->permission_failed.load()) {
        log::logi("Capture duration finished; hiprofiler is finalizing output "
                  "(stopping session, flushing data)...");
        int finalize_wait = 0;
        while (!state->done.load() && !g_interrupt_requested) {
            const long long total = elapsed_since(start);
            std::cout << "Finalizing... total " << total << "s elapsed "
                      << "(post-capture wait: " << finalize_wait << "s)\n"
                      << std::flush;
            sleep_ticks(50, [&]() {
                return state->done.load() || g_interrupt_requested;
            });
            finalize_wait += 5;
        }
        log::logi("hiprofiler post-capture processing complete.");
    }

    // worker.join() frees the hdc connection, so any blocked pid-checker call
    // completes promptly after. On a permission failure the worker's `hdc
    // shell` may still be running the full capture duration on the device, so
    // joining would idle-block here — detach it (and the other background
    // threads) instead; they only reference the heap-allocated shared state
    // and terminate when the process exits.
    if (state->permission_failed.load()) {
        worker.detach();
        if (pid_checker.joinable()) pid_checker.detach();
        if (fresh_restart_thread.joinable()) fresh_restart_thread.detach();
    } else {
        worker.join();
        if (pid_checker.joinable()) pid_checker.join();
        if (fresh_restart_thread.joinable()) fresh_restart_thread.join();
    }
}

// ── result assembly: end snapshot, result-file pull, cleanup ─────────────────

// Assemble the RunResult: copy the shared transcript, mark success/failure
// (including a permission abort), pull the result file(s) and remove the local
// temp config file. The end-of-capture smaps/lsof snapshot is exported in
// stop_and_join_threads, before `hiprofiler_cmd stop` runs.
void finalize_results(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
                      clock_type::time_point start, bool snapshot_mode,
                      bool async_start_mode,
                      const std::map<std::string, long long>& result_baseline,
                      const std::map<std::string, long long>& hiprofiler_zip_baseline,
                      const std::string& nativeleaks_pid,
                      const std::string& local_config_file, RunResult& result) {
    result.elapsed = elapsed_since(start);

    // fork/pipe failed to spawn hdc — report without writing a log.
    if (state->spawn_failed) {
        result.ok = false;
        result.error = "unable to launch hdc command";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(state->mtx);
        result.output = state->output;
    }
    result.exit_code = state->exit_code.load();
    result.ok = true;
    // Async start: the transcript was already streamed to the console in real
    // time, so tell the caller not to print it a second time (it stays in the
    // log file).
    result.output_streamed = async_start_mode && !state->spawn_failed;

    // The first smaps/lsof export found a permission problem: the capture was
    // already aborted in the countdown loop. Mark this run as failed and skip
    // the end-of-capture export and the result-file pull.
    const bool permission_abort = !snapshot_mode && state->permission_failed.load();
    if (permission_abort) {
        result.ok = false;
        result.error =
            "permission check failed: the target app cannot be tuned on this "
            "device (release-signed app on a non-root phone)";
    }

    // Export result file(s). Two mutually exclusive flows:
    //   snapshot: poll result_dir until files' sizes stabilize, then pull.
    //     (hiprofiler's remote_output_path is empty here.) `-t` caps the wait;
    //     default 120s. Recompute elapsed to cover polling + pull.
    //   hiprofiler: the result file is ready when the command finishes, so
    //     pull remote_output_path immediately.
    if (snapshot_mode) {
        // The hidumper command returns almost instantly. If its output clearly
        // reports insufficient permission (non-root phone + release-signed
        // app; "[Permission Check Failed] ..."), or the command exits with a
        // nonzero code, fail now instead of idling through the poll timeout.
        if (!handle_snapshot_command_failure(result)) {
            const long long poll_timeout = (opts.duration > 0) ? opts.duration : 120;
            result.snapshot_attempted = true;
            poll_and_pull_snapshot(opts.result_dir, opts.output_dir, result_baseline,
                                   poll_timeout, result.snapshot_pulled_files,
                                   result.snapshot_status);
            result.elapsed = elapsed_since(start);
        }
    } else if (result.ok && !opts.remote_output_path.empty()) {
        // hiprofiler export flow.
        //
        // wait_for_zip (only -g / -l): The hiprofiler_cmd process on the
        // device may continue writing the hiprofiler_data_*.zip file for a
        // few seconds AFTER `hdc shell` has returned. We poll /data/local/tmp
        // for up to 3 seconds for a NEW zip (not in the pre-command baseline)
        // whose size is stable across two consecutive samples. If found, we
        // prefer the zip over the -o file; otherwise we fall back to the -o
        // file.
        //
        // !wait_for_zip (-n / -f / -d): these capture types don't produce a
        // hiprofiler_data_*.zip, so we just pull the -o file directly.
        std::string file_to_pull = opts.remote_output_path;

        if (opts.wait_for_zip) {
            const long long hiprofiler_wait_timeout_s = 3;
            const std::string new_zip_path = wait_for_hiprofiler_zip(
                hiprofiler_zip_baseline, kHiprofilerZipDir, hiprofiler_wait_timeout_s);
            if (!new_zip_path.empty()) {
                file_to_pull = new_zip_path;
            } else {
                std::string msg = "No new hiprofiler_data zip detected within " +
                                  std::to_string(hiprofiler_wait_timeout_s) +
                                  "s; falling back to -o file: " + opts.remote_output_path;
                // Same chain as the "Device failure detected" line below: wait_for_hiprofiler_zip()
                // stops early on device_lost(), so an unplugged device shows up HERE as well, and
                // "no new zip" is meaningless without knowing which device. Conditional because
                // this path can also be reached untargeted (see the gate's Legacy branch).
                if (!hdc_connect_key().empty()) {
                    msg += " [device: " + hdc_connect_key() + "]";
                }
                log::loge(msg);
            }
        }

        result.pull_attempted = true;
        PullOutcome po = pull_file(file_to_pull, opts.output_dir);
        result.pull_ok = po.ok;
        result.pull_exit_code = po.exit_code;
        result.pull_output = po.output;
        result.pulled_file = local_recv_path(opts.output_dir, file_to_pull);

        // nativeleaks: also pull the orphan-snapshot file written alongside
        // the .htrace. It may appear a few seconds after hiprofiler_cmd
        // returns, so poll for it like wait_for_hiprofiler_zip, then pull it
        // into the session dir so `trace -l <leaks> <htrace>` can use both.
        if (opts.pull_nativeleaks && result.pull_ok) {
            // leaks 文件名按 pid 命名（memleak-native-orphansnapshot-<pid>.txt）。
            // nativeleaks_pid 是采集前 pid；空（进程采集前未运行、被 aa start
            // 拉起）则实时取，此时进程必在运行（hiprofiler_cmd 刚结束）。
            std::string pid = nativeleaks_pid;
            if (pid.empty()) pid = get_process_pid(opts.process_name);
            const long long leaks_wait_timeout_s = 10;
            const std::string leaks_remote = wait_for_nativeleaks_file(
                pid, kHiprofilerZipDir, leaks_wait_timeout_s);
            if (!leaks_remote.empty()) {
                PullOutcome lpo = pull_file(leaks_remote, opts.output_dir);
                if (lpo.ok) {
                    result.pulled_leaks_file = local_recv_path(opts.output_dir, leaks_remote);
                    log::logok("Nativeleaks file exported: ", result.pulled_leaks_file);
                } else {
                    log::loge("Nativeleaks file export failed: ", leaks_remote,
                              " (exit code ", lpo.exit_code, ")\n", lpo.output);
                }
            } else {
                std::string msg = "No new nativeleaks orphan-snapshot file detected within " +
                    std::to_string(leaks_wait_timeout_s) + "s";
                if (!hdc_connect_key().empty()) msg += " [device: " + hdc_connect_key() + "]";
                log::loge(msg);
            }
        }
    }

    // Clean up the local temp config file if it was created.
    if (!local_config_file.empty()) {
        std::error_code ec_cleanup;
        fs::remove(path_from_utf8(local_config_file), ec_cleanup);
    }
}

// ── continuous snapshot session (-c snapshot with an explicit -t) ────────────

// Memory sampling interval, matching the other capture types' countdown.
constexpr int kSnapshotSampleIntervalS = 5;
// Number of consecutive samples above the recorded ArkTS-heap peak that
// trigger a snapshot export.
constexpr int kSnapshotPeakTrigger = 10;
// How far (in KB) the ArkTS heap must exceed the recorded peak before a sample
// counts as above it, so that small fluctuations don't trigger an export.
constexpr long long kSnapshotPeakGrowthKb = 50 * 1024;
// Upper bound for one device-side snapshot command.
constexpr long long kSnapshotCommandTimeoutMs = 120000;
// Upper bound for waiting until a generated snapshot file stops growing.
constexpr long long kSnapshotPollTimeoutS = 120;

// Export one snapshot: re-baseline the device result dir, run the snapshot
// command, then wait for the generated file(s) to stabilize and pull them into
// the session directory, tagged with `role` (start / peak / end). Returns true
// when at least one file was pulled (its local path(s) appended to `files`).
// `err` is set on a fatal failure (permission / command failure); `status`
// always carries a human-readable outcome and `output` the device-side command
// transcript.
bool export_snapshot_once(const HdcRunOptions& opts,
                          const std::string& command,
                          const std::string& role,
                          std::vector<std::string>& files,
                          std::string& status,
                          std::string& output,
                          std::string& err) {
    files.clear();
    status.clear();
    err.clear();
    // Re-baseline right before the command so this call only picks up the
    // file(s) its own command generates.
    const auto baseline = list_dir_file_sizes(opts.result_dir, "hidumper-jsheap");
    CapturedCmd cap = capture_command(command, kSnapshotCommandTimeoutMs);
    output = cap.output;
    const std::string perr = detect_permission_error(cap.output);
    if (!perr.empty()) {
        status = "permission check failed: " + perr;
        err = "permission check failed: the target app cannot be tuned on this "
              "device (release-signed app on a non-root phone)";
        return false;
    }
    if (cap.timed_out) {
        err = "snapshot export command timed out";
        status = err;
        return false;
    }
    if (cap.exit_code != 0) {
        err = "snapshot export command failed (exit code " +
              std::to_string(cap.exit_code) + ")";
        status = err;
        return false;
    }
    poll_and_pull_snapshot(opts.result_dir, opts.output_dir, baseline,
                           kSnapshotPollTimeoutS, files, status, role);
    return !files.empty();
}

// Delete a snapshot's local artifacts. Used to keep only one peak snapshot on
// disk.
void remove_snapshot_artifacts(const std::vector<std::string>& files) {
    for (const auto& f : files) {
        std::error_code ec;
        fs::remove(path_from_utf8(f), ec);
        if (ec) {
            log::logw("Warning: unable to remove the previous peak snapshot ", f, " (", ec.message(), ")");
            continue;
        }
        log::logi("Removed the previous peak snapshot: ", f);
    }
}

// `-c snapshot` with an explicit `-t`: keep monitoring for `duration` seconds
// like the other capture types (same periodic memory line) and export a
// snapshot at the start, at the end, and whenever the ArkTS heap stays at least
// kSnapshotPeakGrowthKb above its recorded peak for kSnapshotPeakTrigger
// consecutive samples. Ctrl+C ends the session and still exports the final
// snapshot.
//
// Artifacts: the start snapshot is exported once at the very beginning and is
// never overwritten nor removed afterwards (a target restart doesn't re-export
// it either), and the recorded peak never decreases, so the single peak
// snapshot that is kept is always the one holding the session's highest ArkTS
// heap.
void run_continuous_snapshot(const HdcRunOptions& opts,
                             const std::string& command,
                             const PreCaptureState& pre,
                             clock_type::time_point start,
                             RunResult& result) {
    result.snapshot_attempted = true;

    std::string pid = pre.initial_pid;
    std::vector<std::string> start_files, peak_files, end_files;
    std::string status, output, err;
    std::string transcript;

    // The caller already created this session's directory
    // (<output_dir>/snapshot-<bundle>-<timestamp>/). Every snapshot of the
    // session is pulled flat into it and tagged with its role by a file-name
    // suffix (...-start.rawheap / -peak.rawheap / -end.rawheap) instead of a
    // role subdirectory: the layout stays shallow while each file remains
    // self-describing (a plain device timestamp doesn't tell which export it is).

    // The recorded peak (the "previous max" the trigger compares against) is
    // seeded from the first valid sample of the monitoring loop below, i.e.
    // after the start-of-session export. It is deliberately not sampled before
    // that export: hidumper-jsheap runs a GC while dumping, so a pre-export
    // reading (still holding the garbage the dump just collected) is
    // systematically higher than every sample the trigger later compares
    // against, which would put the peak out of reach for the whole session.
    // Until it is seeded the trigger stays idle, so the baseline and the
    // compared samples always come from the same (post-GC) regime.
    ProcessMetrics metrics;
    long long arkts_peak_kb = 0;
    bool peak_seeded = false;

    // ── start-of-session snapshot ───────────────────────────────────────────
    log::logi("Exporting the start-of-session snapshot...");
    if (!export_snapshot_once(opts, command, "start", start_files, status, output, err)) {
        result.ok = false;
        result.error = err.empty() ? status : err;
        result.snapshot_status = status.empty() ? err : status;
        result.output = output;
        return;
    }
    transcript += output;
    log::logok("Snapshot exported: ", start_files.front(), " (start)");

    // ── monitoring loop ─────────────────────────────────────────────────────
    int round_no = 0;
    int samples_above_peak = 0;
    while (true) {
        const long long remain = opts.duration - elapsed_since(start);
        if (remain <= 0) break;
        if (g_interrupt_requested) {
            log::logw("Interrupted (Ctrl+C); finishing the capture...");
            break;
        }
        // Pinned target: the device failed at the hdc level (measured while sampling
        // the metrics below). Stop sampling and fall through to the teardown, which
        // still exports what has been captured so far; note_device_failure() has
        // already printed the hdc text once. Untargeted runs never set the flag, so
        // their behaviour is unchanged.
        if (device_lost()) break;

        metrics = sample_process_metrics(pid);
        if (!metrics.valid) {
            // The target may have exited or been restarted: resolve it again.
            // A restart deliberately keeps the recorded peak (it is the maximum
            // of the whole session) and the streak: the trigger and the kept
            // peak snapshot keep referring to the highest value the session has
            // seen, even across process instances.
            const std::string fresh_pid = get_process_pid(opts.process_name);
            if (fresh_pid != pid) {
                pid = fresh_pid;
                metrics = sample_process_metrics(pid);
            }
        }
        std::cout << "[" << ++round_no << "] Remaining " << remain << "s, "
                  << (pid.empty()
                          ? "target process not running, please start it as soon as possible"
                          : "target process is running pid:" + pid)
                  << "\n" << std::flush;
        if (metrics.valid) print_metrics_line(metrics);

        // Seed the recorded peak with the first valid sample of the loop (see
        // the note above): this is the session's baseline ArkTS heap. A sample
        // that parsed other rows but not the "ark ts heap" one leaves
        // arkts_heap_kb at 0, which must not become the baseline (it would turn
        // the growth gate into a plain absolute threshold).
        if (metrics.valid && !peak_seeded && metrics.arkts_heap_kb > 0) {
            arkts_peak_kb = metrics.arkts_heap_kb;
            peak_seeded = true;
        }

        // ArkTS heap at least kSnapshotPeakGrowthKb above the recorded peak
        // for kSnapshotPeakTrigger consecutive samples: export a snapshot and
        // lift the peak to it.
        if (metrics.valid && peak_seeded) {
            if (metrics.arkts_heap_kb > arkts_peak_kb + kSnapshotPeakGrowthKb) {
                ++samples_above_peak;
                if (samples_above_peak >= kSnapshotPeakTrigger) {
                    log::logi("ArkTS heap stayed above the recorded peak by more than ",
                              (kSnapshotPeakGrowthKb / 1024), " MB for ",
                              samples_above_peak,
                              " consecutive samples; exporting a snapshot...");
                    // The peak is lifted as soon as the export is decided, so
                    // that later samples are compared against this new value
                    // even if the export itself fails.
                    arkts_peak_kb = metrics.arkts_heap_kb;
                    std::vector<std::string> new_peak;
                    std::string peak_status, peak_output, peak_err;
                    if (export_snapshot_once(opts, command, "peak", new_peak, peak_status,
                                             peak_output, peak_err)) {
                        // Only one peak snapshot is kept: the new export is
                        // complete, so the previous one can be dropped now.
                        remove_snapshot_artifacts(peak_files);
                        peak_files = new_peak;
                        transcript += peak_output;
                        log::logok("Snapshot exported: ", peak_files.front(), " (peak)");
                    } else {
                        log::logw("Warning: snapshot export failed: ", (peak_err.empty() ? peak_status : peak_err));
                    }
                    samples_above_peak = 0;
                }
            } else {
                samples_above_peak = 0;
            }
        }

        // Also wake up when the device is lost, so the check above runs at the
        // next tick instead of sleeping out the remaining interval (pinned target).
        sleep_ticks(kSnapshotSampleIntervalS * 10,
                    []() { return g_interrupt_requested != 0 || device_lost(); });
    }

    // ── end-of-session snapshot ─────────────────────────────────────────────
    // Pinned target: once the device is gone this export is certain to fail — it
    // would spend one more hdc round-trip (up to kSnapshotCommandTimeoutMs) just to
    // print a warning. Skip it, keep the snapshots exported above, and record the
    // device failure as the reason the session ended. Untargeted runs never set the
    // flag, so their behaviour and output are unchanged.
    if (device_lost()) {
        result.snapshot_status = "device failure detected: end-of-session snapshot skipped";
    } else {
        log::logi("Exporting the end-of-session snapshot...");
        if (export_snapshot_once(opts, command, "end", end_files, status, output, err)) {
            transcript += output;
            log::logok("Snapshot exported: ", end_files.front(), " (end)");
            result.snapshot_status = "complete";
        } else {
            const std::string reason = err.empty() ? status : err;
            log::logw("Warning: end-of-session snapshot export failed: ", reason);
            result.snapshot_status = "complete (end-of-session snapshot failed: " + reason + ")";
        }
    }

    // Kept artifacts: the start snapshot, the latest peak snapshot and the end
    // snapshot.
    for (const auto& f : start_files) result.snapshot_pulled_files.push_back(f);
    for (const auto& f : peak_files) result.snapshot_pulled_files.push_back(f);
    for (const auto& f : end_files) result.snapshot_pulled_files.push_back(f);
    result.output = transcript;
    result.ok = true;
}

} // namespace

RunResult run_hdc(const HdcRunOptions& opts) {
    RunResult result;
    const bool snapshot_mode = !opts.result_dir.empty();

    CaptureSetup setup;
    if (!setup_capture(opts, result, setup)) {
        return result;
    }

    auto state = std::make_shared<CaptureState>();
    const auto start = clock_type::now();

    PreCaptureState pre;
    if (!prepare_capture_start(opts, snapshot_mode, pre, result)) {
        return result;
    }

    // Continuous snapshot session (`-c snapshot` with an explicit `-t`): the
    // snapshot command is re-run at the start, at the end, and on ArkTS-heap
    // peak growth, while a periodic memory line is shown. It manages its own
    // process sampling, so the hiprofiler worker / pid-checker / fresh-restart
    // threads and the finalize step below do not apply.
    if (opts.continuous_snapshot) {
        run_continuous_snapshot(opts, setup.command, pre, start, result);
        result.elapsed = elapsed_since(start);
        // The finalize step below (which normally removes the temp config) is
        // skipped here, so clean up explicitly.
        if (!setup.local_config_file.empty()) {
            std::error_code ec;
            fs::remove(path_from_utf8(setup.local_config_file), ec);
        }
        return result;
    }

    // Worker / pid checker / fresh-restart automation threads.
    std::thread worker = spawn_worker(state, setup.command, setup.async_start_mode);

    std::thread pid_checker;
    if (!snapshot_mode && opts.duration > 0 && !opts.process_name.empty()) {
        pid_checker = spawn_pid_checker(state, opts.process_name, pre.initial_pid,
                                        pre.initial_metrics, setup.async_start_mode);
    }

    std::thread fresh_restart_thread;
    if (!snapshot_mode && opts.duration > 0 && !opts.process_name.empty() &&
        opts.require_fresh_start && opts.is_app) {
        fresh_restart_thread = spawn_fresh_restart(state, opts.process_name);
    }

    // Wait while showing progress: a snapshot wait or a "Remaining Xs"
    // countdown. The start-of-capture snapshot export is triggered inside.
    bool start_exported = false;
    std::thread start_export_thread;
    wait_for_capture(opts, state, start, pre, setup.async_start_mode, snapshot_mode,
                     start_exported, start_export_thread, result);

    // Signal the background threads, stop hiprofiler (async mode), wait out the
    // post-capture finalization and join/detach the threads.
    stop_and_join_threads(opts, state, start, setup.async_start_mode, worker,
                          pid_checker, fresh_restart_thread, start_export_thread,
                          result);

    result.elapsed = elapsed_since(start);

    // Assemble the result, export the end-of-capture snapshot, pull the result
    // file(s) and clean up the temp config.
    finalize_results(opts, state, start, snapshot_mode, setup.async_start_mode,
                     pre.result_baseline, pre.hiprofiler_zip_baseline,
                     pre.initial_pid,
                     setup.local_config_file, result);

    return result;
}

} // namespace mem_analyzer
