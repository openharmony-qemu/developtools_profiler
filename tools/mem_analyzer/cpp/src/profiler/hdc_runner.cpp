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

using proc::CaptureCommand;
using proc::ShellQuote;
using proc::CapturedCmd;

namespace fs = std::filesystem;
using clockType = std::chrono::steady_clock;

namespace {

// Ctrl+C (SIGINT) flag: the signal handler only sets this flag; the capture
// loop polls it so we can stop hiprofiler cleanly and save the data instead
// of the process dying abruptly.
volatile std::sig_atomic_t gInterruptRequested = 0;

extern "C" void HandleSigint(int)
{
    gInterruptRequested = 1;
}

// Pinned target (explicit --device, or the single attached device auto-pinned by the
// gate): on an hdc device-level failure, flag it and surface the hdc text once, so
// polling loops can bail out early instead of spinning until timeout. Untargeted runs
// (no key pinned: 0 devices, noisy parse, hdc failure) never trigger this.
std::atomic<bool> gHdcDeviceFailed{false};

bool NoteDeviceFailure(const std::string& output)
{
    if (HdcConnectKey().empty()) {
        return false;
    }
    if (!HdcIsDeviceFailure(output)) {
        return false;
    }
    if (!gHdcDeviceFailed.exchange(true)) {
        log::Loge(HdcFailureLines(output));
    }
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
bool DeviceLost() { return gHdcDeviceFailed.load(); }

// Whole seconds elapsed since `start`.
long long ElapsedSince(clockType::time_point start)
{
    return static_cast<long long>(
        std::chrono::duration_cast<std::chrono::seconds>(
            clockType::now() - start)
            .count());
}

// Sleep in 100ms ticks for at most `ticks` iterations, returning as soon as
// `stop()` becomes true. Waiting loops use this so they react to completion /
// early-abort conditions promptly instead of sleeping out a full 5s period.
void SleepTicks(int ticks, const std::function<bool()>& stop)
{
    for (int i = 0; i < ticks; ++i) {
        if (stop()) {
            return;
        }
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
std::string ConsoleStartLine(const std::string& line)
{
    if (line.find("Use 'hiprofiler_cmd stop'") != std::string::npos) {
        const char* key = nullptr;
        switch (CurrentPlatform()) {
            case Platform::Mac:     key = "Control+C"; break;
            case Platform::Windows: key = "Ctrl+C";    break;
            default:                key = "Ctrl+C";    break;  // Linux
        }
        return std::string("Capture stops when the countdown ends, or press ") +
               key + " to stop early.\n";
    }
    return line;
}

// Process memory/fd metrics parsed from `hidumper --mem <pid>` (pss / native
// heap / arkts heap / dma(graph) / anon page other / .so) and `lsof -p <pid>`
// (fd count).
struct ProcessMetrics {
    bool valid = false;             // at least one metric was parsed
    long long pssKb = 0;           // "Total" row Pss (already includes SwapPss)
    long long nativeHeapKb = 0;   // "native heap" row Pss + SwapPss
    long long arktsHeapKb = 0;    // "ark ts heap" row Pss + SwapPss
    long long dmaGraphKb = 0;     // "Graph" row Pss + SwapPss
    long long anonPageOtherKb = 0;   // "AnonPage other" row Pss + SwapPss
    long long soKb = 0;                // ".so" row Pss + SwapPss
    long long glKb = 0;                // "GL" row Pss + SwapPss
    long long hapKb = 0;               // ".hap" row Pss + SwapPss
    long long arkwebPaKb = 0;         // "arkweb-pa heap" row Pss + SwapPss
    long long filePageOtherKb = 0;   // "FilePage other" row Pss + SwapPss
    int fdCount = -1;              // lsof data rows (excluding header); -1 = unknown
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
    bool pssOnly;
};

const MemRowSpec kMemRows[] = {
    {{"Total"},             &ProcessMetrics::pssKb,             true},
    {{"native", "heap"},    &ProcessMetrics::nativeHeapKb,     false},
    {{"ark", "ts", "heap"}, &ProcessMetrics::arktsHeapKb,      false},
    {{"Graph"},             &ProcessMetrics::dmaGraphKb,       false},
    {{"AnonPage", "other"}, &ProcessMetrics::anonPageOtherKb, false},
    {{".so"},               &ProcessMetrics::soKb,              false},
    {{"GL"},                &ProcessMetrics::glKb,              false},
    {{".hap"},              &ProcessMetrics::hapKb,             false},
    {{"arkweb-pa", "heap"}, &ProcessMetrics::arkwebPaKb,       false},
    {{"FilePage", "other"}, &ProcessMetrics::filePageOtherKb, false},
};

}  // namespace

ProcessMetrics ParseHidumperMem(const std::string& output)
{
    ProcessMetrics m;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream lss(line);
        std::string first;
        if (!(lss >> first)) {
            continue;
        }// blank line
        if (!first.empty() && first[0] == '-') {
            continue; // separator row
        }
        // Match the row label against the spec table (first token + the rest).
        const MemRowSpec* spec = nullptr;
        for (const auto& s : kMemRows) {
            if (s.label[0] != first) {
                continue;
            }
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
        if (!spec) {
            continue;
        }
        // Collect the remaining numeric columns of the row. Non-numeric tokens
        // are skipped; strtoll avoids std::stoll's exceptions per token.
        std::vector<long long> nums;
        std::string v;
        while (lss >> v) {
            char* end = nullptr;
            long long val = std::strtoll(v.c_str(), &end, 10);
            if (end != v.c_str() && *end == '\0') {
                nums.push_back(val);
            }
        }
        if (nums.empty()) {
            continue;
        }
        const long long pss = nums[0];
        const long long swapPss = nums.size() > 6 ? nums[6] : 0;
        m.*(spec->field) = spec->pssOnly ? pss : pss + swapPss;
        m.valid = true;
    }
    return m;
}

// Count the data rows of a `lsof -p <pid>` output: every non-empty line that
// does not start with the "COMMAND" header column counts as one open fd.
int CountLsofFds(const std::string& output)
{
    int count = 0;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.find("COMMAND") == 0) {
            continue; // header row
        }
        ++count;
    }
    return count;
}

// Sample the live memory/fd metrics of one process: `hidumper --mem <pid>`
// (Pss / native heap / arkts heap / dma / ...) and `lsof -p <pid>` (fd count).
// An empty pid, or queries that yield nothing, produce an invalid sample.
ProcessMetrics SampleProcessMetrics(const std::string& pid, long long timeoutMs = 8000)
{
    ProcessMetrics metrics;
    if (pid.empty()) {
        return metrics;
    }
    CapturedCmd memCap = CaptureCommand(
        HdcShell("hidumper --mem " + proc::ShellQuoteDevice(pid)) + " 2>&1", timeoutMs);
    // Pinned target: on a device-level failure the sample would merely come out
    // empty; note_device_failure() flags it so the caller aborts the countdown
    // instead of polling a device that is no longer reachable. This helper is a
    // plain function (not a loop), so it returns an invalid sample rather than
    // breaking out.
    if (NoteDeviceFailure(memCap.output)) {
        return metrics;
    }
    metrics = ParseHidumperMem(memCap.output);
    CapturedCmd lsofCap = CaptureCommand(
        HdcShell("lsof -p " + proc::ShellQuoteDevice(pid)) + " 2>&1", timeoutMs);
    // Same gate as above: a device-level failure must stop the polling here too,
    // otherwise fd_count would silently be reported as 0.
    if (NoteDeviceFailure(lsofCap.output)) {
        return metrics;
    }
    metrics.fdCount = CountLsofFds(lsofCap.output);
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
std::string DetectPermissionError(const std::string& output)
{
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
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        bool hit = false;
        for (const char* m : kMarkers) {
            if (ContainsCi(lines[i], m)) { hit = true; break; }
        }
        if (!hit) {
            continue;
        }
        // Collect an error block starting at the matched line: up to 8 lines,
        // stopping at the first empty line.
        std::string block;
        for (size_t j = i; j < lines.size() && j < i + 8; ++j) {
            if (lines[j].empty()) {
                break;
            }
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
        if (!block.empty()) {
            return block;
        }
    }
    return "";
}

// Export a per-process snapshot file to `output_dir` via `hdc shell`:
// `lsof -p <pid>` when `prefix` is "fds", otherwise `hidumper --mem-smaps <pid>`.
// The captured output is written to <output_dir>/<prefix>_<timestamp>.txt
// (binary mode). Returns the local path on success, or an empty string on
// failure (unable to launch hdc / empty output / cannot open the file).
std::string ExportProcessFile(const std::string& pid,
    const std::string& outputDir,
    const std::string& prefix,
    std::string* permissionError = nullptr)
{
    const std::string deviceCmd =
        (prefix == "fds") ? ("lsof -p " + proc::ShellQuoteDevice(pid)) : ("hidumper --mem-smaps " + proc::ShellQuoteDevice(pid));
    // Timeout so a stalled device query cannot leave the export thread (and
    // thus the join at stop time) blocked forever.
    CapturedCmd cap = CaptureCommand(
        HdcShell(deviceCmd) + " 2>&1", 10000);
    if (NoteDeviceFailure(cap.output)) {
        return "";
    }
    if (!cap.spawned || cap.output.empty()) {
        return "";
    }
    // Permission probe: on a non-root phone, hidumper refuses to export memory
    // info for release-signed (non-tunable) apps and prints a permission
    // failure. Carry the result back to the caller so the capture aborts
    // right at the start instead of running the full duration.
    if (permissionError) {
        *permissionError = DetectPermissionError(cap.output);
    }
    const std::string localPath = PathToUtf8(
        (PathFromUtf8(outputDir) /
         (prefix + "_" + CurrentTimestamp() + ".txt"))
            .lexically_normal());
    // UTF-8 path -> wide path on Windows (path_from_utf8 is a no-op elsewhere).
    std::ofstream ofs(PathFromUtf8(localPath), std::ios::binary);
    if (!ofs.is_open()) {
        return "";
    }
    ofs << cap.output;
    return localPath;
}

// Local path a `hdc file recv <remote> <output_dir>` writes to:
// <output_dir>/<basename(remote)>, lexically normalized.
std::string LocalRecvPath(const std::string& outputDir, const std::string& remote)
{
    return PathToUtf8(
        (PathFromUtf8(outputDir) / PathFromUtf8(remote).filename())
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
std::string MarkSnapshotRole(const std::string& path, const std::string& role)
{
    if (role.empty()) {
        return path;
    }
    const fs::path p = PathFromUtf8(path);
    const fs::path renamed =
        p.parent_path() / (PathToUtf8(p.stem()) + "-" + role + PathToUtf8(p.extension()));
    std::error_code ec;
    fs::rename(p, renamed, ec);
    if (ec) {
        log::Logw("Warning: unable to tag the snapshot with its role (", role, "): ", path, " (", ec.message(), ")");
        return path;
    }
    return PathToUtf8(renamed);
}

// Outcome of a `hdc file recv` invocation.
struct PullOutcome {
    bool ok = false;
    int exitCode = 0;
    std::string output;
};

// Pull a file from the device: `hdc file recv <remote> <local_dir>` (stderr
// merged). Returns the captured output and the hdc exit code.
PullOutcome PullFile(const std::string& remote, const std::string& localDir)
{
    std::string cmd = HdcFileRecv(remote, localDir) + " 2>&1";
    CapturedCmd cap = CaptureCommand(cmd);
    PullOutcome po;
    if (NoteDeviceFailure(cap.output)) {
        po.exitCode = cap.exitCode;
        po.output = cap.output;
        return po;
    }
    if (!cap.spawned) {
        po.output = "unable to launch hdc file recv";
        return po;
    }
    po.exitCode = cap.exitCode;
    po.output = cap.output;
    // `hdc file recv` is unreliable on BOTH sides: it exits 0 even when the
    // remote file does not exist (printing "[Fail]Error opening file: ..." to
    // stderr), and it can exit nonzero even after a fully successful transfer
    // (e.g. some hdc builds return a nonzero code after FileTransfer
    // finishes). So neither the exit code alone nor the transfer banner
    // decides success — a failure marker in the output, or a missing/empty
    // local result file, marks the pull as failed.
    const bool failedMsg = po.output.find("[Fail]") != std::string::npos ||
                            po.output.find("Error opening file") != std::string::npos ||
                            po.output.find("no such file") != std::string::npos ||
                            po.output.find("No such file") != std::string::npos;
    bool fileOk = false;
    {
        const std::string local = LocalRecvPath(localDir, remote);
        const fs::path localP = PathFromUtf8(local);
        std::error_code ec;
        if (fs::exists(localP, ec) && !ec) {
            std::error_code ec2;
            const auto size = fs::file_size(localP, ec2);
            fileOk = !ec2 && size > 0;
        }
    }
    po.ok = !failedMsg && fileOk;
    return po;
}

// Extract the millisecond-epoch timestamp from a hiprofiler_data_*.zip
// filename. The naming convention is: hiprofiler_data_<ms>.zip
// Returns seconds since epoch (ms / 1000), or -1 on parse failure.
long long ParseHiprofilerDataTs(const std::string& filename)
{
    // Expected: "hiprofiler_data_1785766871434.zip"
    constexpr const char* prefix = "hiprofiler_data_";
    const auto plen = std::strlen(prefix);
    if (filename.size() <= plen + 4) {
        return -1;
    }// need at least "X.zip"
    if (filename.rfind(prefix, 0) != 0) {
        return -1;
    }
    if (filename.substr(filename.size() - 4) != ".zip") {
        return -1;
    }

    // Extract the numeric part between prefix and ".zip".
    std::string numstr = filename.substr(plen, filename.size() - plen - 4);
    if (numstr.empty()) {
        return -1;
    }

    // Validate all digits.
    for (char c : numstr) {
        if (c < '0' || c > '9') {
            return -1;
        }
    }

    try {
        long long ms = std::stoll(numstr);
        return ms / 1000;  // convert milliseconds -> seconds
    } catch (...)
    {
        return -1;
    }
}

// List all hiprofiler_data_*.zip files in `dir` via plain `ls`. Returns a
// map of { filename -> embedded-timestamp-seconds }. The embedded timestamp
// (extracted from the filename) is the actual time when hiprofiler generated
// the zip on the device, so it is a reliable indicator that hiprofiler has
// finished writing the file.
std::map<std::string, long long> listHiprofilerZips(const std::string& dir)
{
    std::map<std::string, long long> result;
    std::string cmd = HdcShell("ls " + proc::ShellQuoteDevice(dir)) + " 2>&1";
    CapturedCmd cap = CaptureCommand(cmd);
    if (NoteDeviceFailure(cap.output)) {
        return result;
    }
    if (!cap.spawned) {
        return result;
    }

    std::istringstream iss(cap.output);
    std::string line;
    while (std::getline(iss, line)) {
        const auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            continue;
        }
        const auto last = line.find_last_not_of(" \t\r\n");
        std::string name = line.substr(first, last - first + 1);

        // Filter: must start with "hiprofiler_data_" and end with ".zip".
        if (name.rfind("hiprofiler_data_", 0) != 0) {
            continue;
        }
        if (name.size() < 4 || name.substr(name.size() - 4) != ".zip") {
            continue;
        }

        const long long ts = ParseHiprofilerDataTs(name);
        if (ts < 0) {
            continue;
        }
        result[name] = ts;
    }
    return result;
}

// Build the full device path for a hiprofiler zip file inside `dir`.
std::string MakeDevicePath(const std::string& dir, const std::string& name)
{
    std::string full = dir;
    if (!full.empty() && full.back() != '/') {
        full += "/";
    }
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
std::map<std::string, long long> listDirFileSizes(const std::string& dir,
    const std::string& namePrefix,
    const std::string& nameSuffix = "")
{
    std::map<std::string, long long> files;
    std::string cmd = HdcShell("ls -l " + proc::ShellQuoteDevice(dir)) + " 2>&1";
    CapturedCmd cap = CaptureCommand(cmd);
    if (NoteDeviceFailure(cap.output)) {
        return files;
    }
    if (!cap.spawned) {
        return files;
    }
    std::istringstream iss(cap.output);
    std::string line;
    while (std::getline(iss, line)) {
        // Expected line (one of):
        //   "-rw-r--r-- 1 root root 56789 Aug  2 20:53 name"
        //   "-rw-r--r-- 1 root root 56789 2026-08-02 20:53 name"
        std::istringstream lss(line);
        std::string perm;
        lss >> perm;
        if (perm.empty() || perm[0] != '-') {
            continue; // skip dirs / "total" / blank
        }
        std::string links, owner, group, sizeStr;
        if (!(lss >> links >> owner >> group >> sizeStr)) {
            continue;
        }
        long long size = 0;
        try {
            size = std::stoll(sizeStr);
        } catch (...)
        {
            continue;
        }
        // Skip the date/time fields (ISO = 2, traditional = 3; see header).
        std::string dateOrMon;
        if (!(lss >> dateOrMon)) {
            continue;
        }
        std::string discard;
        if (dateOrMon.find('-') != std::string::npos) {
            lss >> discard;              // time (HH:MM)
        } else {
            lss >> discard >> discard;   // day, time
        }
        std::string name;
        std::getline(lss, name);
        const auto first = name.find_first_not_of(" \t");
        if (first == std::string::npos) {
            continue;
        }
        const auto last = name.find_last_not_of(" \t\r\n");
        name = name.substr(first, last - first + 1);
        // Only track files matching the expected name pattern; ignore
        // unrelated files that may exist in the same device directory.
        if (name.rfind(namePrefix, 0) != 0) {
            continue;
        }
        if (!nameSuffix.empty() &&
            (name.size() < nameSuffix.size() ||
             name.compare(name.size() - nameSuffix.size(), nameSuffix.size(),
                          nameSuffix) != 0)) {
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
void pollAndPullSnapshot(const std::string& resultDir,
    const std::string& targetDir,
    const std::map<std::string, long long>& baseline,
    long long timeoutS,
    std::vector<std::string>& files,
    std::string& status,
    const std::string& role = "")
{
    const int intervalS = 2;
    auto start = clockType::now();

    std::map<std::string, long long> prevSizes;  // candidates' sizes from prior poll
    std::map<std::string, long long> stable;      // candidates confirmed complete
    int pollNum = 0;

    while (stable.empty()) {
        if (DeviceLost()) {
            break;
        }// pinned target: device-level failure -> stop polling now
        ++pollNum;
        const long long elapsedS = ElapsedSince(start);
        if (elapsedS >= timeoutS) {
            break;
        }

        const auto current = listDirFileSizes(resultDir, "hidumper-jsheap");
        // Candidates: new files or files whose size changed vs baseline.
        std::map<std::string, long long> candidates;
        for (const auto& kv : current) {
            auto it = baseline.find(kv.first);
            if (it == baseline.end() || it->second != kv.second) {
                candidates[kv.first] = kv.second;
            }
        }

        if (candidates.empty()) {
            std::cout << "[Snapshot check] #" << pollNum << " (" << elapsedS <<
                      "s): no new files\n" <<
                      std::flush;
            prevSizes.clear();
        } else {
            bool allStable = true;
            for (const auto& kv : candidates) {
                auto pit = prevSizes.find(kv.first);
                const bool s = (pit != prevSizes.end() && pit->second == kv.second &&
                                kv.second > 0);
                if (!s) {
                    allStable = false;
                }
                std::cout << "[Snapshot check] #" << pollNum << " (" << elapsedS <<
                          "s): " << kv.first << " " << (kv.second / 1024) << "KB" <<
                          (s ? " stable" : " generating") << "\n" <<
                          std::flush;
            }
            if (allStable) {
                stable = candidates;
                break;
            }
            prevSizes = candidates;
        }

        // Sleep `interval_s` (clamped to the remaining timeout), in 100ms ticks
        // so we don't overshoot the deadline by much.
        const long long remain = timeoutS - elapsedS;
        const int sleepS = static_cast<int>(std::min<long long>(intervalS, remain));
        SleepTicks(sleepS * 10, []() { return false; });
    }

    if (stable.empty()) {
        status = "Timeout: snapshot file generation not detected as complete (" + resultDir + ")";
        return;
    }

    // Pull the completed snapshot file(s) into the capture's own directory so
    // results don't land flat in the top-level output dir (mirrors the analysis
    // subcommands).
    std::error_code ecMkdir;
    fs::create_directories(PathFromUtf8(targetDir), ecMkdir);

    // Pull each completed file (remote path with a trailing slash).
    std::string dir = resultDir;
    if (!dir.empty() && dir.back() != '/') {
        dir += "/";
    }
    const size_t pulledBefore = files.size();
    for (const auto& kv : stable) {
        const std::string remote = dir + kv.first;
        PullOutcome po = PullFile(remote, targetDir);
        if (po.ok) {
            files.push_back(MarkSnapshotRole(LocalRecvPath(targetDir, remote), role));
        } else {
            log::Loge("Snapshot file export failed: ", remote, " (exit code ", po.exitCode, ")\n", po.output);
        }
    }
    status = files.size() > pulledBefore ? "complete" : "snapshot file export failed";
}

// ── Automated app launch / fresh-restart via the `aa` tool ─────────────

// Parse the entry ability name of an app from `bm dump -n <package>` output.
// Tries the FA-model "mainAbility" and Stage-model "mainElementName" JSON
// fields. `bm dump` output can contain multiple occurrences of a key where the
// top-level one is empty (e.g. `"mainAbility": ""` appears first, then the real
// `"mainAbility": "<name>"` in a module section), so we scan for the first
// NON-empty value rather than stopping at the first match.
// Returns an empty string when the output is unavailable or unparsable.
std::string GetDeviceMainAbility(const std::string& packageName)
{
    std::string cmd = HdcShell("bm dump -n " + proc::ShellQuoteDevice(packageName)) + " 2>&1";
    CapturedCmd cap = CaptureCommand(cmd, 15000);
    if (NoteDeviceFailure(cap.output)) {
        return "";
    }
    if (!cap.spawned) {
        return "";
    }

    // Return the first non-empty string value for `"<key>": "<value>"`,
    // scanning every occurrence of the key in the output.
    auto extractValue = [](const std::string& text, const std::string& key) -> std::string {
        const std::string needle = "\"" + key + "\"";
        std::string::size_type pos = 0;
        while ((pos = text.find(needle, pos)) != std::string::npos) {
            pos += needle.size();
            const auto colon = text.find(':', pos);
            if (colon == std::string::npos) {
                break;
            }
            const auto vstart = text.find('"', colon + 1);
            if (vstart == std::string::npos) {
                break;
            }
            const auto vend = text.find('"', vstart + 1);
            if (vend == std::string::npos) {
                break;
            }
            std::string value = text.substr(vstart + 1, vend - vstart - 1);
            if (!value.empty()) {
                return value;
            }
            pos = vend + 1;  // empty value; keep scanning for a non-empty one
        }
        return "";
    };

    std::string ability = extractValue(cap.output, "mainAbility");
    if (ability.empty()) {
        ability = extractValue(cap.output, "mainElementName");
    }
    return ability;
}

// Launch an app on the device via `aa start -a <ability> -b <package>`.
// The entry ability is resolved from `bm dump -n`; common defaults
// ("EntryAbility" / "MainAbility") are tried as fallbacks. Returns true when
// the device reports a successful start.
bool StartDeviceApp(const std::string& packageName)
{
    std::vector<std::string> abilities;
    const std::string resolved = GetDeviceMainAbility(packageName);
    if (!resolved.empty()) {
        abilities.push_back(resolved);
    } else {
        log::Logw("Warning: unable to resolve the entry ability of '", packageName, "' via 'bm dump -n'; trying common defaults.");
    }
    abilities.push_back("EntryAbility");
    abilities.push_back("MainAbility");

    for (const auto& ability : abilities) {
        const std::string deviceCmd = "aa start -a " + proc::ShellQuoteDevice(ability) +
                                     " -b " + proc::ShellQuoteDevice(packageName);
        CapturedCmd cap = CaptureCommand(
            HdcShell(deviceCmd) + " 2>&1", 15000);
        if (!cap.spawned) {
            return false;
        }
        const bool ok = cap.output.find("start ability successfully") != std::string::npos &&
                        cap.output.find("error: failed") == std::string::npos;
        std::cout << "  " << deviceCmd << "\n" << cap.output << std::flush;
        if (ok) {
            return true;
        }
    }
    return false;
}

// Force-stop an app on the device via `aa force-stop <package>`.
// Harmless when the app is not running; device output is printed if any.
void ForceStopDeviceApp(const std::string& packageName)
{
    const std::string deviceCmd = "aa force-stop " + proc::ShellQuoteDevice(packageName);
    CapturedCmd cap = CaptureCommand(
        HdcShell(deviceCmd) + " 2>&1", 15000);
    if (cap.spawned && !cap.output.empty()) {
        std::cout << "  " << deviceCmd << "\n" << cap.output << std::flush;
    }
}

// Launch an app and wait (polling `pidof`) until its process is up.
// Returns true when the process appears within the wait budget (~30s).
bool LaunchDeviceAppAndWait(const std::string& packageName)
{
    if (!StartDeviceApp(packageName)) {
        return false;
    }
    for (int i = 0; i < 60; ++i) {  // 500ms ticks
        if (!GetProcessPidFast(packageName, 3000).empty()) {
            return true;
        }
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
bool DetectAsyncStartMode(const std::string& content)
{
    std::istringstream iss(content);
    std::string tok;
    bool seenHiprofilerCmd = false;
    while (iss >> tok) {
        if (seenHiprofilerCmd && tok == "start") {
            return true;
        }
        if (tok == "hiprofiler_cmd") {
            seenHiprofilerCmd = true;
        }
    }
    return false;
}

// Push the hiprofiler device-config content to the device via a local temp
// file + `hdc file send`, BEFORE executing the command (avoids Windows
// cmd.exe's ~8191-char command-line length limit for long configs). On success
// `local_config_file` holds the temp path (used later for the log entry and
// cleanup). Returns false and sets result.ok=false with a description on any
// failure.
bool PushDeviceConfig(const std::string& deviceConfigContent, std::string& localConfigFile, RunResult& result)
{
    if (deviceConfigContent.empty()) {
        return true;
    }
    localConfigFile = PathToUtf8(
        fs::temp_directory_path() /
        ("mem_analyzer_config_" + CurrentTimestamp() + ".cfg"));
    std::ofstream cfgOfs(PathFromUtf8(localConfigFile), std::ios::binary);
    if (!cfgOfs.is_open()) {
        result.ok = false;
        result.error = "failed to create local temp config file";
        return false;
    }
    cfgOfs << deviceConfigContent;
    cfgOfs.close();

    std::string sendCmd = HdcFileSend(localConfigFile, kDeviceConfigPath) + " 2>&1";
    CapturedCmd sendCap = CaptureCommand(sendCmd);
    if (!sendCap.spawned || sendCap.exitCode != 0) {
        result.ok = false;
        result.error = "failed to push config file to device (exit code " +
                       std::to_string(sendCap.exitCode) + ")";
        if (!sendCap.output.empty()) {
            result.error += ": " + sendCap.output;
        }
        std::error_code ec;
        fs::remove(PathFromUtf8(localConfigFile), ec);
        return false;
    }
    return true;
}

// Run `hiprofiler_cmd stop` on the device and report progress. Can take a
// while (stopping sessions, flushing the data file), so it runs in a
// background thread with a periodic wait-time hint — otherwise the user may
// think the tool is stuck.
void StopHiprofiler()
{
    std::atomic<bool> stopDone{false};
    CapturedCmd stopCap;
    std::thread StopWorker([&]()
    {
        std::string stopCmd = HdcShell("hiprofiler_cmd stop") + " 2>&1";
        stopCap = CaptureCommand(stopCmd);
        stopDone = true;
    });

    const auto stopStart = clockType::now();
    while (!stopDone.load()) {
        const long long wait = ElapsedSince(stopStart);
        // Only show progress once the wait becomes non-trivial.
        if (wait >= 5) {
            std::cout << "Waiting for hiprofiler to stop... (" << wait << "s)\n" <<
                      std::flush;
        }
        // Sleep up to 5s, waking every 100ms so we react to completion.
        SleepTicks(50, [&]() { return stopDone.load(); });
    }
    StopWorker.join();

    if (!stopCap.output.empty()) {
        std::cout << stopCap.output;
        if (stopCap.output.back() != '\n') {
            std::cout << '\n';
        }
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
std::string WaitForHiprofilerZip(const std::map<std::string, long long>& baseline,
    const std::string& dir, long long timeoutS)
{
    std::cout << "Waiting for hiprofiler_data zip (up to " << timeoutS << "s)...\n" <<
              std::flush;

    long long prevSize = -1;
    int stableCount = 0;
    auto pollStart = clockType::now();

    for (int i = 0; i < timeoutS; ++i) {
        if (DeviceLost()) {
            break;
        }// pinned target: device-level failure -> stop polling now
        const long long elapsedPoll = ElapsedSince(pollStart);
        if (elapsedPoll >= timeoutS) {
            break;
        }

        // Re-scan the directory for any hiprofiler_data_*.zip files.
        const auto currentZips = listHiprofilerZips(dir);

        std::string candidate;
        long long candidateTs = -1;
        // Look for a NEW file not in baseline; among new files pick the one
        // with the largest embedded timestamp (most likely produced by this
        // run).
        for (const auto& kv : currentZips) {
            if (baseline.count(kv.first)) {
                continue;
            }
            if (kv.second > candidateTs) {
                candidate = kv.first;
                candidateTs = kv.second;
            }
        }

        if (candidate.empty()) {
            prevSize = -1;
            stableCount = 0;
            std::cout << "[hiprofiler wait] #" << i <<
                      " (" << elapsedPoll << "s): no new hiprofiler_data zip yet\n" <<
                      std::flush;
        } else {
            const auto sizes = listDirFileSizes(dir, "hiprofiler_data_", ".zip");
            long long curSize = 0;
            auto sit = sizes.find(candidate);
            if (sit != sizes.end()) {
                curSize = sit->second;
            }

            if (curSize > 0 && curSize == prevSize) {
                ++stableCount;
            } else {
                stableCount = (curSize > 0) ? 1 : 0;
            }
            prevSize = curSize;

            std::cout << "[hiprofiler wait] #" << i <<
                      " (" << elapsedPoll << "s): new zip candidate " << candidate <<
                      " size=" << (curSize / 1024) << "KB" <<
                      (stableCount >= 2 ? " stable" :
                          (stableCount == 1 ? " stabilizing" : " generating")) <<
                      "\n" << std::flush;

            if (stableCount >= 2) {
                std::cout << "Detected new hiprofiler_data zip after " << i <<
                          " polls: " << candidate <<
                          " (will prefer this over -o file)\n" << std::flush;
                return MakeDevicePath(dir, candidate);
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
std::string WaitForNativeleaksFile(const std::string& pid, const std::string& dir, long long timeoutS)
{
    if (pid.empty()) {
        return "";
    }
    const std::string filename = "memleak-native-orphansnapshot-" + pid + ".txt";
    std::cout << "Waiting for nativeleaks orphan-snapshot file (" << filename <<
              ", up to " << timeoutS << "s)...\n" << std::flush;

    long long prevSize = -1;
    int stableCount = 0;
    auto pollStart = clockType::now();

    for (int i = 0; i < timeoutS; ++i) {
        if (DeviceLost()) {
            break;
        }
        const long long elapsedPoll = ElapsedSince(pollStart);
        if (elapsedPoll >= timeoutS) {
            break;
        }

        // Pass the full filename as the prefix so list_dir_file_sizes matches
        // only that one file (rfind(prefix, 0) == 0), ignoring other pids'.
        const auto current = listDirFileSizes(dir, filename);
        long long curSize = 0;
        const auto it = current.find(filename);
        const bool found = (it != current.end());
        if (found) {
            curSize = it->second;
        }

        if (!found || curSize <= 0) {
            prevSize = -1;
            stableCount = 0;
            std::cout << "[nativeleaks wait] #" << i << " (" << elapsedPoll <<
                      "s): " << filename << " not ready yet\n" << std::flush;
        } else {
            if (curSize == prevSize) {
                ++stableCount;
            } else {
                stableCount = 1;
            }
            prevSize = curSize;

            std::cout << "[nativeleaks wait] #" << i << " (" << elapsedPoll << "s): " <<
                      filename << " size=" << (curSize / 1024) << "KB" <<
                      (stableCount >= 2 ? " stable" :
                          (stableCount == 1 ? " stabilizing" : " generating")) <<
                      "\n" << std::flush;

            if (stableCount >= 2) {
                std::cout << "Detected nativeleaks orphan-snapshot file after " << i <<
                          " polls: " << filename << "\n" << std::flush;
                return MakeDevicePath(dir, filename);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return "";
}

// Record an exported per-process snapshot file (hidumper --mem-smaps / lsof)
// in `result` and report it on the console.
void RecordExportedFile(const std::string& path, const std::string& smapsPrefix, RunResult& result)
{
    result.smapsExportedFiles.push_back(path);
    log::Logok(smapsPrefix == "fds" ? "FD snapshot exported" : "Smaps exported", ": ", path);
}

// Export an end-of-capture smaps/lsof snapshot of the target process so it can
// be diffed against the start-of-capture export. Called only when the capture
// actually ran (not aborted for lack of permission / failed to start).
void ExportEndSnapshot(const std::string& pid, const std::string& outputDir,
    const std::string& smapsPrefix, RunResult& result)
{
    if (pid.empty()) {
        log::Logw("Warning: target process exited; skipping end-of-capture ", smapsPrefix, " export");
        return;
    }
    const std::string path = ExportProcessFile(pid, outputDir, smapsPrefix);
    if (!path.empty()) {
        RecordExportedFile(path, smapsPrefix, result);
    } else {
        log::Logw("Warning: ", smapsPrefix, " export failed at capture end");
    }
}

// Snapshot flow: the hidumper command returns almost instantly. If its output
// clearly reports insufficient permission (non-root phone + release-signed
// app; the device prints "[Permission Check Failed] ..."), or the command
// exits with a nonzero code, fail immediately instead of idling through the
// 120s poll timeout. Returns true when the snapshot command has been judged
// failed and written into `result`; callers should skip polling then.
bool HandleSnapshotCommandFailure(RunResult& result)
{
    const std::string perr = DetectPermissionError(result.output);
    const bool cmdFailed = result.exitCode != 0;
    if (perr.empty() && !cmdFailed) {
        return false;
    }
    result.ok = false;
    result.snapshotAttempted = true;
    if (!perr.empty()) {
        result.error =
            "permission check failed: the target app cannot be tuned on this "
            "device (release-signed app on a non-root phone)";
        result.snapshotStatus = "permission check failed: " + perr;
    } else {
        result.error = "snapshot export command failed (exit code " +
                       std::to_string(result.exitCode) + ")";
        result.snapshotStatus = result.error;
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
    std::atomic<int> exitCode{0};
    std::atomic<bool> done{false};
    std::atomic<bool> spawnFailed{false};

    // Background pid-checker state.
    std::mutex pidMtx;
    std::string latestPid;             // guarded by pid_mtx
    ProcessMetrics latestMetrics;      // guarded by pid_mtx
    std::atomic<bool> pidChanged{false};
    std::atomic<bool> stopPidCheck{false};

    // Fresh-restart automation state.
    std::atomic<bool> stopFreshRestart{false};
    std::atomic<bool> freshRestartTriggered{false};
    std::atomic<bool> freshRestartActive{false};

    // Permission probe: set by the first smaps/fds export. The device-side
    // reason is guarded by mtx.
    std::atomic<bool> permissionFailed{false};
    std::string permissionErrorMsg;   // guarded by mtx
};

} // namespace

// Poll whether the target process is running on the device.
// Primary: uses `pidof <name>` — the same command the device-side template
// (snapshot_hidumper_cmd.txt) uses. This is reliable on all HarmonyOS versions.
// Fallback: `ps -ef` for systems where `pidof` is unavailable.
// Returns the PID if found, empty string otherwise.
std::string GetProcessPid(const std::string& processName)
{
    if (processName.empty()) {
        return "";
    }

    // 1) Primary: pidof (same as device-side template, fast & reliable)
    {
        std::string cmd = HdcShell("pidof " + proc::ShellQuoteDevice(processName)) + " 2>&1";
        CapturedCmd cap = CaptureCommand(cmd, 5000);
        // Pinned target: an hdc [Fail] means the DEVICE failed
        // (unplugged / unauthorized), not "process not running". Surface the hdc
        // text once and bail out instead of misparsing it as a missing process.
        if (NoteDeviceFailure(cap.output)) {
            return "";
        }
        if (cap.spawned && !cap.timedOut) {
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
        std::string cmd = HdcShell("ps -ef") + " 2>&1";
        CapturedCmd cap = CaptureCommand(cmd);
        if (NoteDeviceFailure(cap.output)) {
            return "";
        }
        if (!cap.spawned) {
            return "";
        }

        std::string suffixPid;
        std::istringstream iss(cap.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) {
                continue;
            }
            if (line.find("UID") == 0) {
                continue;
            }

            std::istringstream lss(line);
            std::string pid, tok;
            for (int col = 0; col < 7; ++col) {
                if (!(lss >> tok)) { pid.clear(); break; }
                if (col == 1) {
                    pid = tok;
                }
            }
            if (pid.empty()) {
                continue;
            }
            if (pid.find_first_not_of("0123456789") != std::string::npos) {
                continue;
            }
            std::string cmdField;
            std::getline(lss, cmdField);
            const auto cmdStart = cmdField.find_first_not_of(" \t");
            if (cmdStart == std::string::npos) {
                continue;
            }
            cmdField = cmdField.substr(cmdStart);

            if (cmdField == processName) {
                return pid;
            }

            if (suffixPid.empty() &&
                cmdField.size() > processName.size() &&
                cmdField.compare(0, processName.size(), processName) == 0 &&
                cmdField[processName.size()] == ':') {
                suffixPid = pid;
            }
        }
        return suffixPid;
    }
}

std::string GetProcessPidFast(const std::string& processName, long long timeoutMs, bool* reliable) {
    if (reliable) {
        *reliable = false;
    }
    if (processName.empty()) {
        return "";
    }
    // Use `pidof` — a very lightweight command that outputs only the PID
    // (a few bytes) or nothing. This is drastically faster than `ps -ef`
    // (which outputs the entire process list, often several KB) and is
    // specifically designed for use in a background thread while another
    // `hdc shell` (hiprofiler_cmd) is running.
    //
    // pidof returns space-separated PIDs (e.g. "12345 12346") on success,
    // or nothing if the process isn't running. We take the first PID.
    std::string cmd = HdcShell("pidof " + proc::ShellQuoteDevice(processName)) + " 2>&1";
    CapturedCmd cap = CaptureCommand(cmd, timeoutMs);
    if (!cap.spawned || cap.timedOut) {
        return ""; // unreliable (stalled hdc)
    }
    // Pinned target: a device-level failure is reported once and
    // stops the background checker. `reliable` deliberately stays false so the
    // countdown keeps the last known pid instead of flipping to "not running"
    // (which would be a wrong conclusion: the device is gone, not the process).
    if (NoteDeviceFailure(cap.output)) {
        return "";
    }

    // Parse the first whitespace-separated token and validate it's a number.
    std::istringstream iss(cap.output);
    std::string pid;
    iss >> pid;
    if (!pid.empty() && pid.find_first_not_of("0123456789") == std::string::npos) {
        if (reliable) {
            *reliable = true;
        }
        return pid;
    }
    // Empty output means pidof ran cleanly and the process is not running
    // (reliable). Any other output (e.g. "sh: pidof: not found", garbled
    // text) means the query itself failed — mark it unreliable so the caller
    // can keep the previously known PID instead of misreporting.
    const bool blank =
        cap.output.empty() ||
        cap.output.find_first_not_of(" \t\r\n") == std::string::npos;
    if (reliable) {
        *reliable = blank;
    }
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
    std::string localConfigFile;   // temp config to remove on exit
    bool asyncStartMode = false;   // "hiprofiler_cmd start" (needs stop)
};

bool SetupCapture(const HdcRunOptions& opts, RunResult& result, CaptureSetup& setup)
{
    // Trap Ctrl+C so the profiler can stop hiprofiler and save data instead
    // of being killed abruptly. The handler just sets a flag that the capture
    // loop polls.
    gInterruptRequested = 0;
    std::signal(SIGINT, HandleSigint);

    // Create the output directory (and parents) if missing; ignore existing.
    std::error_code ec;
    fs::create_directories(PathFromUtf8(opts.outputDir), ec);

    // If there's device config content (hiprofiler commands), write it to a
    // local temp file and push it to the device via `hdc file send` BEFORE
    // executing the command (avoids Windows cmd.exe's ~8191-character
    // command-line length limit for long hiprofiler configs).
    if (!PushDeviceConfig(opts.deviceConfigContent, setup.localConfigFile, result)) {
        return false;
    }

    // Shell-quoted arg preserves newlines/backslashes/heredoc for the device
    // shell; 2>&1 merges stderr into stdout.
    setup.command = HdcShell(opts.content) + " 2>&1";

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
    setup.asyncStartMode = DetectAsyncStartMode(opts.content);
    return true;
}

// ── pre-capture: baselines + initial process state ───────────────────────────

// State gathered before the worker occupies the hdc connection: device result
// dir baseline (snapshot mode), hiprofiler zip baseline (-g/-l) and the
// initial pid / metrics / running status of the target process.
struct PreCaptureState {
    bool wasRunningAtStart = false;
    std::string initialPid;
    ProcessMetrics initialMetrics;
    std::map<std::string, long long> resultBaseline;          // snapshot mode
    std::map<std::string, long long> hiprofilerZipBaseline;  // -g / -l
};

// Snapshot flow: the target process must be running before we attempt the
// capture, and we capture the device result dir BEFORE the command runs so we
// can later identify the files this run generated (new / size-changed).
// Returns false when the capture must abort (e.g. snapshot target not
// running); the error is written into `result`.
bool PrepareCaptureStart(const HdcRunOptions& opts, bool snapshotMode, PreCaptureState& pre, RunResult& result)
{
    if (snapshotMode) {
        if (!opts.processName.empty()) {
            const std::string pid = GetProcessPid(opts.processName);
            if (pid.empty()) {
                result.ok = false;
                result.error = "target process '" + opts.processName +
                               "' is not running; please start the app first";
                return false;
            }
            log::Logi("Target process '", opts.processName, "' is running, pid:", pid);
            pre.initialPid = pid;
        }
        pre.resultBaseline = listDirFileSizes(opts.resultDir, "hidumper-jsheap");
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
    if (!opts.remoteOutputPath.empty() && opts.waitForZip) {
        pre.hiprofilerZipBaseline = listHiprofilerZips(kHiprofilerZipDir);
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
    if (opts.duration <= 0 || opts.processName.empty()) {
        return true;
    }

    // Automated app-state management (all capture types except -s):
    //   - app not running, no fresh-start need (-n/-g/-f/-d): launch now
    //     via `aa start` so it is up when the countdown begins.
    //   - fresh-start (-l), app running: force-stop + relaunch is deferred
    //     until the capture session is up (the hook library must load at
    //     app startup to be traced).
    //   - fresh-start (-l), app not running: launch is deferred the same way.
    //   - app running, no fresh-start need: left untouched.
    const std::string runningPid = GetProcessPid(opts.processName);
    const bool wasRunning = !runningPid.empty();
    if (wasRunning) {
        log::Logi("Target process '", opts.processName, "' is running, pid:", runningPid);
    } else {
        log::Logw("Target process '", opts.processName, "' is not running.");
    }

    if (!wasRunning && !opts.requireFreshStart) {
        if (opts.isApp) {
            log::Logi("Auto-launching '", opts.processName, "' via aa start...");
            if (LaunchDeviceAppAndWait(opts.processName)) {
                log::Logi("'", opts.processName, "' is running.");
            } else {
                log::Logw("Warning: failed to auto-launch '", opts.processName, "'; please start it manually.");
            }
        } else {
            // System process: never auto-launch; keep the original hint.
            log::Logi("Please start it as soon as possible.");
        }
    } else if (opts.requireFreshStart) {
        log::Logi("This capture type requires a FRESH START: the app will be force-stopped "
                  "and relaunched automatically once the capture session is ready.");
    }

    pre.initialPid = GetProcessPid(opts.processName);
    pre.wasRunningAtStart = !pre.initialPid.empty();
    if (!wasRunning && pre.wasRunningAtStart) {
        log::Logi("Target process '", opts.processName, "' is now running, pid:", pre.initialPid);
    }
    // Collect an initial sample of the process metrics while hdc is still
    // free (before the worker occupies the connection), so the first
    // countdown tick can show them immediately.
    if (pre.wasRunningAtStart) {
        CapturedCmd memCap = CaptureCommand(
            HdcShell("hidumper --mem " + proc::ShellQuoteDevice(pre.initialPid)) + " 2>&1");
        pre.initialMetrics = ParseHidumperMem(memCap.output);
        CapturedCmd lsofCap = CaptureCommand(
            HdcShell("lsof -p " + proc::ShellQuoteDevice(pre.initialPid)) + " 2>&1");
        pre.initialMetrics.fdCount = CountLsofFds(lsofCap.output);
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
std::thread SpawnWorker(const std::shared_ptr<CaptureState>& state, const std::string& cmd, bool asyncStartMode)
{
    return std::thread([state, cmd, asyncStartMode]() {
        // 逐行消费子进程输出：写入共享缓冲，async 模式下即时回显到控制台。
        auto consumeStream = [&](FILE* fp) {
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
                if (asyncStartMode) {
                    std::cout << ConsoleStartLine(buf) << std::flush;
                }
            }
        };
#if defined(_WIN32)
        // 无 shell 执行：CreateProcessW 直接解析命令行（MSVCRT 规则，不经 cmd.exe），
        // 宽字符避免中文进程名的 ANSI 乱码；管道合并 stderr 到 stdout，
        // 消除 cmd.exe 对 `&|<>^` 等元字符的解释（主机命令注入）。
        SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        HANDLE hRead = nullptr, hWrite = nullptr;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            state->spawnFailed = true; state->done = true; return;
        }
        if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
            CloseHandle(hRead); CloseHandle(hWrite);
            state->spawnFailed = true; state->done = true; return;
        }
        STARTUPINFOW si{ sizeof(STARTUPINFOW) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION pi{};

        // 去掉尾部 " 2>&1"：stderr 已由上面的 STARTF_USESTDHANDLES 合并进 stdout。
        std::string cleanCmd = cmd;
        if (cleanCmd.size() >= 5 && cleanCmd.substr(cleanCmd.size() - 5) == " 2>&1") {
            cleanCmd.resize(cleanCmd.size() - 5);
        }
        std::wstring wcmdline = WidenPath(cleanCmd);
        if (!CreateProcessW(nullptr, wcmdline.data(), nullptr, nullptr, TRUE, 0,
                            nullptr, nullptr, &si, &pi)) {
            CloseHandle(hRead); CloseHandle(hWrite);
            state->spawnFailed = true; state->done = true; return;
        }
        CloseHandle(hWrite);

        // 把管道读端包装成 FILE*，保持 fgets 逐行读取的原语义。
        int fd = _open_osfhandle(reinterpret_cast<intptr_t>(hRead), _O_RDONLY);
        if (fd == -1) {
            CloseHandle(hRead);
            state->spawnFailed = true; state->done = true; return;
        }
        FILE* fp = _fdopen(fd, "r");
        if (!fp) {
            _close(fd);
            state->spawnFailed = true; state->done = true; return;
        }

        consumeStream(fp);
        fclose(fp);  // 关闭 fd，进而关闭底层 h_read 句柄

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        state->exitCode = static_cast<int>(code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        auto sp = proc::SpawnPiped(cmd);
        if (!sp.ok()) {
            state->spawnFailed = true; state->done = true; return;
        }
        FILE* fp = fdopen(sp.ReadFd, "r");
        if (!fp) {
            close(sp.ReadFd);
            state->spawnFailed = true; state->done = true; return;
        }
        consumeStream(fp);
        fclose(fp);
        int st = 0;
        waitpid(sp.pid, &st, 0);
        state->exitCode = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
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
std::thread SpawnPidChecker(const std::shared_ptr<CaptureState>& state,
    const std::string& processName,
    const std::string& initialPid,
    const ProcessMetrics& initialMetrics,
    bool ignoreDone)
{
    // Seed the shared pid/metrics state with the pre-countdown sample so the
    // first countdown tick can display it immediately (before the background
    // checker's first hdc round-trip lands).
    state->latestPid = initialPid;
    state->latestMetrics = initialMetrics;
    return std::thread([state, processName, ignoreDone]() {
        while (!state->stopPidCheck.load() &&
               !DeviceLost() &&
               (!state->done.load() || ignoreDone)) {
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
            std::string pid = GetProcessPidFast(processName, 5000, &reliable);
            // Publish the pid FIRST so the countdown always has a fresh
            // process status; the (slower) metrics below never delay it.
            {
                std::lock_guard<std::mutex> lock(state->pidMtx);
                if (!pid.empty()) {
                    if (pid != state->latestPid) {
                        state->pidChanged = true;
                    }
                    state->latestPid = pid;
                } else if (reliable || state->latestPid.empty()) {
                    // pidof confirmed the process is gone, or we never
                    // knew a PID: publish the empty result.
                    if (!state->latestPid.empty()) {
                        state->pidChanged = true;
                    }
                    state->latestPid.clear();
                }
                // Unreliable query with a known PID: keep the old value.
            }
            ProcessMetrics metrics;
            if (!pid.empty()) {
                // Gather live memory/fd metrics for the countdown display.
                // Ran outside the lock because hdc calls may block.
                metrics = SampleProcessMetrics(pid);
            }
            {
                std::lock_guard<std::mutex> lock(state->pidMtx);
                // Only publish a sample that actually parsed. A stalled
                // hidumper/lsof call (the hdc connection is busy with the
                // foregrounded capture) yields an invalid sample; dropping
                // it keeps the last good metrics so the countdown keeps
                // showing the PSS line instead of going blank for a few
                // ticks.
                if (metrics.valid) {
                    state->latestMetrics = metrics;
                }
            }
            // Sleep 5s between checks, in 100ms ticks so we stop promptly.
            SleepTicks(50, [state, ignoreDone]()
            {
                return state->stopPidCheck.load() ||
                       DeviceLost() ||
                       (state->done.load() && !ignoreDone);
            });
        }
    });
}

// Fresh-start automation thread (-l): once the hiprofiler session is up,
// force-stop and relaunch the target app so its startup allocations are
// captured (nativehook with startup_mode hooks the process at startup).
// Runs in a background thread with a bounded wait so a stalled device query
// cannot freeze the countdown.
std::thread SpawnFreshRestart(const std::shared_ptr<CaptureState>& state, const std::string& processName)
{
    // The countdown must not nag about a manual fresh start — the automation
    // below handles it.
    state->freshRestartTriggered = true;
    return std::thread([state, processName]() {
        // Wait for the capture session to come up (the start command prints
        // "Use 'hiprofiler_cmd stop' ..." when ready), or abort if the start
        // fails or the countdown ends first.
        // Also abort as soon as the target device is gone (pinned target): the
        // readiness marker can never arrive from a lost device, so waiting out the
        // full 60s would only delay the teardown and print a misleading "session did
        // not become ready" warning. Untargeted runs never set the flag.
        bool sessionUp = false;
        for (int i = 0; i < 120 && !state->stopFreshRestart.load() && !DeviceLost(); ++i) {
            bool marker = false;
            bool failed = false;
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                marker = state->output.find("Use 'hiprofiler_cmd stop'") != std::string::npos;
                failed = state->output.find("Failed to start profiling session") != std::string::npos ||
                         state->output.find("MakeCreateRequest failed") != std::string::npos;
            }
            if (marker) { sessionUp = true; break; }
            if (failed) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (!sessionUp) {
            log::Logw("Warning: capture session did not become ready; please restart '", processName, "' manually if needed.");
            return;
        }
        state->freshRestartActive = true;
        log::Logi("Capture session ready; force-stopping and relaunching '",
                  processName, "' for a fresh start...");
        ForceStopDeviceApp(processName);
        // Wait for the process to actually exit before relaunching.
        for (int i = 0; i < 20; ++i) {  // up to 10s
            if (GetProcessPidFast(processName, 3000).empty()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (LaunchDeviceAppAndWait(processName)) {
            log::Logi("'", processName, "' relaunched (fresh start).");
        } else {
            log::Logw("Warning: failed to relaunch '", processName, "'; please restart it manually.");
        }
        state->freshRestartActive = false;
    });
}

// ── countdown display helpers ────────────────────────────────────────────────

// Did the device-side `hiprofiler_cmd start` report a failed session? Its
// exit code is useless (it returns 0 even on failure), so detect it from the
// transcript instead.
bool CaptureStartFailed(const std::shared_ptr<CaptureState>& state)
{
    std::lock_guard<std::mutex> lock(state->mtx);
    return state->output.find("Failed to start profiling session") != std::string::npos ||
           state->output.find("MakeCreateRequest failed") != std::string::npos;
}

// Print the live memory/fd metrics line for the countdown. kB values are
// shown as MB with 2 decimals.
void PrintMetricsLine(const ProcessMetrics& metrics)
{
    auto fmtMb = [](long long kb) -> std::string {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", kb / 1024.0);
        return buf;
    };
    // Two lines, indented under the "Remaining ...s" line: the core
    // categories, then the secondary ones plus fd count, so a single
    // countdown tick stays readable on a normal terminal width.
    std::cout << "    PSS: " << fmtMb(metrics.pssKb) <<
              " MB | Native heap: " << fmtMb(metrics.nativeHeapKb) <<
              " MB | ArkTS heap: " << fmtMb(metrics.arktsHeapKb) <<
              " MB | DMA(ION): " << fmtMb(metrics.dmaGraphKb) <<
              " MB | AnonPage other: " << fmtMb(metrics.anonPageOtherKb) <<
              " MB | GL: " << fmtMb(metrics.glKb) <<
              " MB\n" <<
              "    ArkWeb-pa heap: " << fmtMb(metrics.arkwebPaKb) <<
              " MB | .so: " << fmtMb(metrics.soKb) <<
              " MB | .hap: " << fmtMb(metrics.hapKb) <<
              " MB | FilePage other: " << fmtMb(metrics.filePageOtherKb) <<
              " MB | FD: " <<
              (metrics.fdCount >= 0 ? std::to_string(metrics.fdCount) : "-") <<
              "\n" <<
              std::flush;
}

// Export a start-of-capture smaps/lsof snapshot once the process is running
// (immediately, or after a fresh start is detected via pid_changed). The
// device query runs in a background thread so the countdown is never frozen.
// The thread outlives this function, so it captures output_dir / smaps_prefix
// / pid by value; it only references `result` (alive until the caller joins
// the thread) and the heap-allocated `state`.
void maybeStartSnapshotExport(const std::shared_ptr<CaptureState>& state,
    const std::string& outputDir,
    const std::string& smapsPrefix,
    bool requireFreshStart,
    const std::string& pid,
    bool& startExported,
    std::thread& startExportThread,
    RunResult& result)
{
    if (startExported) {
        return;
    }
    if (requireFreshStart && !state->pidChanged.load()) {
        return;
    }
    startExported = true;
    const std::string pidCopy = pid;
    startExportThread = std::thread([state, pidCopy, outputDir, smapsPrefix, &result]()
    {
        std::string perr;
        const std::string path =
            ExportProcessFile(pidCopy, outputDir, smapsPrefix, &perr);
        if (!perr.empty()) {
            // First export already lacks permission: record the device-side
            // reason; once the countdown loop sees permission_failed it aborts
            // the whole capture.
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                state->permissionErrorMsg = perr;
            }
            state->permissionFailed = true;
        } else if (!path.empty()) {
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                result.smapsExportedFiles.push_back(path);
            }
            log::Logok(smapsPrefix == "fds" ? "FD snapshot exported" : "Smaps exported",
                       ": ", path);
        } else {
            log::Logw("Warning: failed to export ", smapsPrefix, " at capture start");
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
void WaitForCapture(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
    clockType::time_point start, const PreCaptureState& pre,
    bool asyncStartMode, bool snapshotMode,
    bool& startExported, std::thread& startExportThread,
    RunResult& result)
{
    if (snapshotMode) {
        log::Logi("Running snapshot export command...");
        while (!state->done.load() && !gInterruptRequested &&
               !DeviceLost()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return;
    }
    if (opts.duration <= 0) {
        return;
    }

    int roundNo = 0;  // monotonically increasing per-tick sequence number
    while (true) {
        const long long remain = opts.duration - ElapsedSince(start);
        if (remain <= 0) {
            break;
        }
        if (gInterruptRequested) {
            log::Logw("Interrupted (Ctrl+C); stopping hiprofiler and saving data...");
            break;
        }
        // Pinned target: the target device failed at the hdc level (e.g. unplugged
        // mid-capture). Stop the countdown now instead of idling until the timeout —
        // note_device_failure() already printed the hdc text once. Untargeted runs
        // never set this flag, so their behaviour is unchanged.
        if (DeviceLost()) {
            // Name the device: the failure belongs to one target, and by the time the line
            // is read the `Device:` echo is far above (or the log of a second console is
            // interleaved with this one). device_lost() is only ever true while a key is
            // pinned (note_device_failure() ignores "[Fail]" otherwise), so the key is
            // non-empty here and the wording always reads correctly.
            log::Logw("Device failure detected on ", HdcConnectKey(), "; stopping hiprofiler and saving data...");
            break;
        }
        // Sync mode: the hiprofiler_cmd runs until it finishes on its own
        // (done drives the loop). Async start mode: the command stays
        // attached until we send stop, so the countdown is time-driven.
        if (!asyncStartMode && state->done.load()) {
            break;
        }
        // If `start` failed to spawn, exited on its own (a start that fails
        // exits immediately; a successful one stays attached until stop), or
        // its transcript reports failure, abort the countdown instead of
        // waiting out the full duration.
        if (asyncStartMode &&
            (state->spawnFailed || (state->done.load() && CaptureStartFailed(state)))) {
            break;
        }
        // The first smaps/lsof export found a permission problem: abort the
        // capture immediately instead of idling through the countdown (a
        // release app on a non-root phone cannot be tuned; further captures
        // would only produce meaningless data).
        if (state->permissionFailed.load()) {
            std::string perr;
            {
                std::lock_guard<std::mutex> lock(state->mtx);
                perr = state->permissionErrorMsg;
            }
            log::Loge("\n", perr, "\nThe target app cannot be tuned on this device (release-signed app on a non-root phone); aborting the capture.");
            break;
        }
        if (!opts.processName.empty()) {
            // Read the latest known pid (non-blocking).
            std::string pid;
            {
                std::lock_guard<std::mutex> lock(state->pidMtx);
                pid = state->latestPid;
            }
            const bool running = !pid.empty();

            if (running && opts.requireFreshStart && pre.wasRunningAtStart &&
                !state->pidChanged.load() && !state->freshRestartTriggered.load()) {
                std::cout << "[" << ++roundNo << "] Remaining " << remain << "s, " <<
                          "WARNING: target process '" << opts.processName << "' is already running!" <<
                          " pid:" << pid << "\n" <<
                          "         This capture type requires a FRESH START.\n" <<
                          "         Please force-stop the app and restart it NOW.\n" <<
                          "\n" <<
                          std::flush;
            } else {
                std::cout << "[" << ++roundNo << "] Remaining " << remain << "s, " <<
                          (running ? "target process is running pid:" + pid :
                                      (state->freshRestartActive.load() ?
                                             "target process is being relaunched by the tool" :
                                             "target process not running, please start it as soon as possible")) <<
                          "\n" <<
                          std::flush;
                if (running) {
                    // Show live memory/fd metrics gathered by the pid checker
                    // (non-blocking read of shared state).
                    ProcessMetrics metrics;
                    {
                        std::lock_guard<std::mutex> lock(state->pidMtx);
                        metrics = state->latestMetrics;
                    }
                    if (metrics.valid) {
                        PrintMetricsLine(metrics);
                    }
                    maybeStartSnapshotExport(state, opts.outputDir, opts.smapsPrefix,
                                                opts.requireFreshStart, pid,
                                                startExported, startExportThread, result);
                }
            }
        } else {
            std::cout << "[" << ++roundNo << "] Remaining " << remain << "s\n" << std::flush;
        }
        // Sleep up to 5s, waking every 100ms to stop promptly when hdc
        // finishes mid-interval, when the start command fails, when the
        // first smaps/lsof export reports a permission failure, or when the
        // target device fails at the hdc level (pinned target).
        SleepTicks(50, [&]()
        {
            return (state->done.load() && (!asyncStartMode || CaptureStartFailed(state))) ||
                   state->permissionFailed.load() ||
                   DeviceLost();
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
void StopAndJoinThreads(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
    clockType::time_point start, bool asyncStartMode,
    std::thread& worker, std::thread& pidChecker,
    std::thread& freshRestartThread,
    std::thread& startExportThread, RunResult& result)
{
    // Signal the background pid checker and fresh-start automation to stop.
    state->stopPidCheck = true;
    state->stopFreshRestart = true;

    // The start-of-capture snapshot export runs concurrently with the countdown
    // (see above); join it here so the exported files list is complete and no
    // thread outlives run_hdc's locals. It finishes within a couple of seconds
    // (a single hdc query), so this never meaningfully delays the stop.
    if (startExportThread.joinable()) {
        startExportThread.join();
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
    auto exportEndSnapshotBeforeStop = [&]() {
        if (state->permissionFailed.load()) {
            return;
        }
        std::string pid;
        {
            std::lock_guard<std::mutex> lock(state->pidMtx);
            pid = state->latestPid;
        }
        ExportEndSnapshot(pid, opts.outputDir, opts.smapsPrefix, result);
    };
    if (asyncStartMode) {
        if (!state->spawnFailed) {
            if (gInterruptRequested) {
                // Ctrl+C: SIGINT killed the local hdc child, but the
                // device-side session keeps capturing — it must still be
                // stopped and saved.
                log::Logw("Interrupted; stopping hiprofiler and saving data...");
                exportEndSnapshotBeforeStop();
                StopHiprofiler();
            } else if (CaptureStartFailed(state)) {
                // The device reported a start failure (its exit code is
                // useless — it returns 0 even on failure); the error
                // transcript was already streamed above. Nothing to stop.
                log::Loge("`hiprofiler_cmd start` failed (see the error above); skipping stop.");
            } else {
                // Normal completion: start is still attached, stop releases it
                // and finalizes the capture.
                log::Logi("Capture duration finished; stopping hiprofiler...");
                exportEndSnapshotBeforeStop();
                StopHiprofiler();
            }
        }

        // The start command has now been released (or exited on its own);
        // join the worker and then the pid checker. worker.join() frees the
        // hdc connection, so any blocked pid-checker call completes promptly
        // after. A permission failure aborts the capture early instead; the
        // background threads are detached and finish on their own (they only
        // reference the heap-allocated shared state, so this is safe).
        if (state->permissionFailed.load()) {
            worker.detach();
            if (pidChecker.joinable()) {
                pidChecker.detach();
            }
            if (freshRestartThread.joinable()) {
                freshRestartThread.detach();
            }
        } else {
            worker.join();
            if (pidChecker.joinable()) {
                pidChecker.join();
            }
            if (freshRestartThread.joinable()) {
                freshRestartThread.join();
            }
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
    if (!state->done.load() && !gInterruptRequested &&
        !state->permissionFailed.load()) {
        log::Logi("Capture duration finished; hiprofiler is finalizing output "
                  "(stopping session, flushing data)...");
        int finalizeWait = 0;
        while (!state->done.load() && !gInterruptRequested) {
            const long long total = ElapsedSince(start);
            std::cout << "Finalizing... total " << total << "s elapsed " <<
                      "(post-capture wait: " << finalizeWait << "s)\n" <<
                      std::flush;
            SleepTicks(50, [&]()
            {
                return state->done.load() || gInterruptRequested;
            });
            finalizeWait += 5;
        }
        log::Logi("hiprofiler post-capture processing complete.");
    }

    // worker.join() frees the hdc connection, so any blocked pid-checker call
    // completes promptly after. On a permission failure the worker's `hdc
    // shell` may still be running the full capture duration on the device, so
    // joining would idle-block here — detach it (and the other background
    // threads) instead; they only reference the heap-allocated shared state
    // and terminate when the process exits.
    if (state->permissionFailed.load()) {
        worker.detach();
        if (pidChecker.joinable()) {
            pidChecker.detach();
        }
        if (freshRestartThread.joinable()) {
            freshRestartThread.detach();
        }
    } else {
        worker.join();
        if (pidChecker.joinable()) {
            pidChecker.join();
        }
        if (freshRestartThread.joinable()) {
            freshRestartThread.join();
        }
    }
}

// ── result assembly: end snapshot, result-file pull, cleanup ─────────────────

// Assemble the RunResult: copy the shared transcript, mark success/failure
// (including a permission abort), pull the result file(s) and remove the local
// temp config file. The end-of-capture smaps/lsof snapshot is exported in
// stop_and_join_threads, before `hiprofiler_cmd stop` runs.
void finalizeResults(const HdcRunOptions& opts, const std::shared_ptr<CaptureState>& state,
    clockType::time_point start, bool snapshotMode,
    bool asyncStartMode,
    const std::map<std::string, long long>& resultBaseline,
    const std::map<std::string, long long>& hiprofilerZipBaseline,
    const std::string& nativeleaksPid,
    const std::string& localConfigFile, RunResult& result)
{
    result.elapsed = ElapsedSince(start);

    // fork/pipe failed to spawn hdc — report without writing a log.
    if (state->spawnFailed) {
        result.ok = false;
        result.error = "unable to launch hdc command";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(state->mtx);
        result.output = state->output;
    }
    result.exitCode = state->exitCode.load();
    result.ok = true;
    // Async start: the transcript was already streamed to the console in real
    // time, so tell the caller not to print it a second time (it stays in the
    // log file).
    result.outputStreamed = asyncStartMode && !state->spawnFailed;

    // The first smaps/lsof export found a permission problem: the capture was
    // already aborted in the countdown loop. Mark this run as failed and skip
    // the end-of-capture export and the result-file pull.
    const bool permissionAbort = !snapshotMode && state->permissionFailed.load();
    if (permissionAbort) {
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
    if (snapshotMode) {
        // The hidumper command returns almost instantly. If its output clearly
        // reports insufficient permission (non-root phone + release-signed
        // app; "[Permission Check Failed] ..."), or the command exits with a
        // nonzero code, fail now instead of idling through the poll timeout.
        if (!HandleSnapshotCommandFailure(result)) {
            const long long pollTimeout = (opts.duration > 0) ? opts.duration : 120;
            result.snapshotAttempted = true;
            pollAndPullSnapshot(opts.resultDir, opts.outputDir, resultBaseline,
                                   pollTimeout, result.snapshotPulledFiles,
                                   result.snapshotStatus);
            result.elapsed = ElapsedSince(start);
        }
    } else if (result.ok && !opts.remoteOutputPath.empty()) {
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
        std::string fileToPull = opts.remoteOutputPath;

        if (opts.waitForZip) {
            const long long hiprofilerWaitTimeoutS = 3;
            const std::string newZipPath = WaitForHiprofilerZip(
                hiprofilerZipBaseline, kHiprofilerZipDir, hiprofilerWaitTimeoutS);
            if (!newZipPath.empty()) {
                fileToPull = newZipPath;
            } else {
                std::string msg = "No new hiprofiler_data zip detected within " +
                                  std::to_string(hiprofilerWaitTimeoutS) +
                                  "s; falling back to -o file: " + opts.remoteOutputPath;
                // Same chain as the "Device failure detected" line below: wait_for_hiprofiler_zip()
                // stops early on device_lost(), so an unplugged device shows up HERE as well, and
                // "no new zip" is meaningless without knowing which device. Conditional because
                // this path can also be reached untargeted (see the gate's Legacy branch).
                if (!HdcConnectKey().empty()) {
                    msg += " [device: " + HdcConnectKey() + "]";
                }
                log::Loge(msg);
            }
        }

        result.pullAttempted = true;
        PullOutcome po = PullFile(fileToPull, opts.outputDir);
        result.pullOk = po.ok;
        result.pullExitCode = po.exitCode;
        result.pullOutput = po.output;
        result.pulledFile = LocalRecvPath(opts.outputDir, fileToPull);

        // nativeleaks: also pull the orphan-snapshot file written alongside
        // the .htrace. It may appear a few seconds after hiprofiler_cmd
        // returns, so poll for it like wait_for_hiprofiler_zip, then pull it
        // into the session dir so `trace -l <leaks> <htrace>` can use both.
        if (opts.pullNativeleaks && result.pullOk) {
            // leaks 文件名按 pid 命名（memleak-native-orphansnapshot-<pid>.txt）。
            // nativeleaks_pid 是采集前 pid；空（进程采集前未运行、被 aa start
            // 拉起）则实时取，此时进程必在运行（hiprofiler_cmd 刚结束）。
            std::string pid = nativeleaksPid;
            if (pid.empty()) {
                pid = GetProcessPid(opts.processName);
            }
            const long long leaksWaitTimeoutS = 10;
            const std::string leaksRemote = WaitForNativeleaksFile(
                pid, kHiprofilerZipDir, leaksWaitTimeoutS);
            if (!leaksRemote.empty()) {
                PullOutcome lpo = PullFile(leaksRemote, opts.outputDir);
                if (lpo.ok) {
                    result.pulledLeaksFile = LocalRecvPath(opts.outputDir, leaksRemote);
                    log::Logok("Nativeleaks file exported: ", result.pulledLeaksFile);
                } else {
                    log::Loge("Nativeleaks file export failed: ", leaksRemote,
                              " (exit code ", lpo.exitCode, ")\n", lpo.output);
                }
            } else {
                std::string msg = "No new nativeleaks orphan-snapshot file detected within " +
                    std::to_string(leaksWaitTimeoutS) + "s";
                if (!HdcConnectKey().empty()) {
                    msg += " [device: " + HdcConnectKey() + "]";
                }
                log::Loge(msg);
            }
        }
    }

    // Clean up the local temp config file if it was created.
    if (!localConfigFile.empty()) {
        std::error_code ecCleanup;
        fs::remove(PathFromUtf8(localConfigFile), ecCleanup);
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
bool exportSnapshotOnce(const HdcRunOptions& opts,
    const std::string& command,
    const std::string& role,
    std::vector<std::string>& files,
    std::string& status,
    std::string& output,
    std::string& err)
{
    files.clear();
    status.clear();
    err.clear();
    // Re-baseline right before the command so this call only picks up the
    // file(s) its own command generates.
    const auto baseline = listDirFileSizes(opts.resultDir, "hidumper-jsheap");
    CapturedCmd cap = CaptureCommand(command, kSnapshotCommandTimeoutMs);
    output = cap.output;
    const std::string perr = DetectPermissionError(cap.output);
    if (!perr.empty()) {
        status = "permission check failed: " + perr;
        err = "permission check failed: the target app cannot be tuned on this "
              "device (release-signed app on a non-root phone)";
        return false;
    }
    if (cap.timedOut) {
        err = "snapshot export command timed out";
        status = err;
        return false;
    }
    if (cap.exitCode != 0) {
        err = "snapshot export command failed (exit code " +
              std::to_string(cap.exitCode) + ")";
        status = err;
        return false;
    }
    pollAndPullSnapshot(opts.resultDir, opts.outputDir, baseline,
                           kSnapshotPollTimeoutS, files, status, role);
    return !files.empty();
}

// Delete a snapshot's local artifacts. Used to keep only one peak snapshot on
// disk.
void RemoveSnapshotArtifacts(const std::vector<std::string>& files)
{
    for (const auto& f : files) {
        std::error_code ec;
        fs::remove(PathFromUtf8(f), ec);
        if (ec) {
            log::Logw("Warning: unable to remove the previous peak snapshot ", f, " (", ec.message(), ")");
            continue;
        }
        log::Logi("Removed the previous peak snapshot: ", f);
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
void RunContinuousSnapshot(const HdcRunOptions& opts,
    const std::string& command,
    const PreCaptureState& pre,
    clockType::time_point start,
    RunResult& result)
{
    result.snapshotAttempted = true;

    std::string pid = pre.initialPid;
    std::vector<std::string> startFiles, peakFiles, endFiles;
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
    long long arktsPeakKb = 0;
    bool peakSeeded = false;

    // ── start-of-session snapshot ───────────────────────────────────────────
    log::Logi("Exporting the start-of-session snapshot...");
    if (!exportSnapshotOnce(opts, command, "start", startFiles, status, output, err)) {
        result.ok = false;
        result.error = err.empty() ? status : err;
        result.snapshotStatus = status.empty() ? err : status;
        result.output = output;
        return;
    }
    transcript += output;
    log::Logok("Snapshot exported: ", startFiles.front(), " (start)");

    // ── monitoring loop ─────────────────────────────────────────────────────
    int roundNo = 0;
    int samplesAbovePeak = 0;
    while (true) {
        const long long remain = opts.duration - ElapsedSince(start);
        if (remain <= 0) {
            break;
        }
        if (gInterruptRequested) {
            log::Logw("Interrupted (Ctrl+C); finishing the capture...");
            break;
        }
        // Pinned target: the device failed at the hdc level (measured while sampling
        // the metrics below). Stop sampling and fall through to the teardown, which
        // still exports what has been captured so far; note_device_failure() has
        // already printed the hdc text once. Untargeted runs never set the flag, so
        // their behaviour is unchanged.
        if (DeviceLost()) {
            break;
        }

        metrics = SampleProcessMetrics(pid);
        if (!metrics.valid) {
            // The target may have exited or been restarted: resolve it again.
            // A restart deliberately keeps the recorded peak (it is the maximum
            // of the whole session) and the streak: the trigger and the kept
            // peak snapshot keep referring to the highest value the session has
            // seen, even across process instances.
            const std::string freshPid = GetProcessPid(opts.processName);
            if (freshPid != pid) {
                pid = freshPid;
                metrics = SampleProcessMetrics(pid);
            }
        }
        std::cout << "[" << ++roundNo << "] Remaining " << remain << "s, " <<
                  (pid.empty() ?
                          "target process not running, please start it as soon as possible" :
                          "target process is running pid:" + pid) <<
                  "\n" << std::flush;
        if (metrics.valid) {
            PrintMetricsLine(metrics);
        }

        // Seed the recorded peak with the first valid sample of the loop (see
        // the note above): this is the session's baseline ArkTS heap. A sample
        // that parsed other rows but not the "ark ts heap" one leaves
        // arkts_heap_kb at 0, which must not become the baseline (it would turn
        // the growth gate into a plain absolute threshold).
        if (metrics.valid && !peakSeeded && metrics.arktsHeapKb > 0) {
            arktsPeakKb = metrics.arktsHeapKb;
            peakSeeded = true;
        }

        // ArkTS heap at least kSnapshotPeakGrowthKb above the recorded peak
        // for kSnapshotPeakTrigger consecutive samples: export a snapshot and
        // lift the peak to it.
        if (metrics.valid && peakSeeded) {
            if (metrics.arktsHeapKb > arktsPeakKb + kSnapshotPeakGrowthKb) {
                ++samplesAbovePeak;
                if (samplesAbovePeak >= kSnapshotPeakTrigger) {
                    log::Logi("ArkTS heap stayed above the recorded peak by more than ",
                              (kSnapshotPeakGrowthKb / 1024), " MB for ",
                              samplesAbovePeak,
                              " consecutive samples; exporting a snapshot...");
                    // The peak is lifted as soon as the export is decided, so
                    // that later samples are compared against this new value
                    // even if the export itself fails.
                    arktsPeakKb = metrics.arktsHeapKb;
                    std::vector<std::string> newPeak;
                    std::string peakStatus, peakOutput, peakErr;
                    if (exportSnapshotOnce(opts, command, "peak", newPeak, peakStatus,
                                             peakOutput, peakErr)) {
                        // Only one peak snapshot is kept: the new export is
                        // complete, so the previous one can be dropped now.
                        RemoveSnapshotArtifacts(peakFiles);
                        peakFiles = newPeak;
                        transcript += peakOutput;
                        log::Logok("Snapshot exported: ", peakFiles.front(), " (peak)");
                    } else {
                        log::Logw("Warning: snapshot export failed: ", (peakErr.empty() ? peakStatus : peakErr));
                    }
                    samplesAbovePeak = 0;
                }
            } else {
                samplesAbovePeak = 0;
            }
        }

        // Also wake up when the device is lost, so the check above runs at the
        // next tick instead of sleeping out the remaining interval (pinned target).
        SleepTicks(kSnapshotSampleIntervalS * 10,
                    []() { return gInterruptRequested != 0 || DeviceLost(); });
    }

    // ── end-of-session snapshot ─────────────────────────────────────────────
    // Pinned target: once the device is gone this export is certain to fail — it
    // would spend one more hdc round-trip (up to kSnapshotCommandTimeoutMs) just to
    // print a warning. Skip it, keep the snapshots exported above, and record the
    // device failure as the reason the session ended. Untargeted runs never set the
    // flag, so their behaviour and output are unchanged.
    if (DeviceLost()) {
        result.snapshotStatus = "device failure detected: end-of-session snapshot skipped";
    } else {
        log::Logi("Exporting the end-of-session snapshot...");
        if (exportSnapshotOnce(opts, command, "end", endFiles, status, output, err)) {
            transcript += output;
            log::Logok("Snapshot exported: ", endFiles.front(), " (end)");
            result.snapshotStatus = "complete";
        } else {
            const std::string reason = err.empty() ? status : err;
            log::Logw("Warning: end-of-session snapshot export failed: ", reason);
            result.snapshotStatus = "complete (end-of-session snapshot failed: " + reason + ")";
        }
    }

    // Kept artifacts: the start snapshot, the latest peak snapshot and the end
    // snapshot.
    for (const auto& f : startFiles) {
        result.snapshotPulledFiles.push_back(f);
    }
    for (const auto& f : peakFiles) {
        result.snapshotPulledFiles.push_back(f);
    }
    for (const auto& f : endFiles) {
        result.snapshotPulledFiles.push_back(f);
    }
    result.output = transcript;
    result.ok = true;
}

} // namespace

RunResult RunHdc(const HdcRunOptions& opts)
{
    RunResult result;
    const bool snapshotMode = !opts.resultDir.empty();

    CaptureSetup setup;
    if (!SetupCapture(opts, result, setup)) {
        return result;
    }

    auto state = std::make_shared<CaptureState>();
    const auto start = clockType::now();

    PreCaptureState pre;
    if (!PrepareCaptureStart(opts, snapshotMode, pre, result)) {
        return result;
    }

    // Continuous snapshot session (`-c snapshot` with an explicit `-t`): the
    // snapshot command is re-run at the start, at the end, and on ArkTS-heap
    // peak growth, while a periodic memory line is shown. It manages its own
    // process sampling, so the hiprofiler worker / pid-checker / fresh-restart
    // threads and the finalize step below do not apply.
    if (opts.continuousSnapshot) {
        RunContinuousSnapshot(opts, setup.command, pre, start, result);
        result.elapsed = ElapsedSince(start);
        // The finalize step below (which normally removes the temp config) is
        // skipped here, so clean up explicitly.
        if (!setup.localConfigFile.empty()) {
            std::error_code ec;
            fs::remove(PathFromUtf8(setup.localConfigFile), ec);
        }
        return result;
    }

    // Worker / pid checker / fresh-restart automation threads.
    std::thread worker = SpawnWorker(state, setup.command, setup.asyncStartMode);

    std::thread pidChecker;
    if (!snapshotMode && opts.duration > 0 && !opts.processName.empty()) {
        pidChecker = SpawnPidChecker(state, opts.processName, pre.initialPid,
                                        pre.initialMetrics, setup.asyncStartMode);
    }

    std::thread freshRestartThread;
    if (!snapshotMode && opts.duration > 0 && !opts.processName.empty() &&
        opts.requireFreshStart && opts.isApp) {
        freshRestartThread = SpawnFreshRestart(state, opts.processName);
    }

    // Wait while showing progress: a snapshot wait or a "Remaining Xs"
    // countdown. The start-of-capture snapshot export is triggered inside.
    bool startExported = false;
    std::thread startExportThread;
    WaitForCapture(opts, state, start, pre, setup.asyncStartMode, snapshotMode,
                     startExported, startExportThread, result);

    // Signal the background threads, stop hiprofiler (async mode), wait out the
    // post-capture finalization and join/detach the threads.
    StopAndJoinThreads(opts, state, start, setup.asyncStartMode, worker,
                          pidChecker, freshRestartThread, startExportThread,
                          result);

    result.elapsed = ElapsedSince(start);

    // Assemble the result, export the end-of-capture snapshot, pull the result
    // file(s) and clean up the temp config.
    finalizeResults(opts, state, start, snapshotMode, setup.asyncStartMode,
                     pre.resultBaseline, pre.hiprofilerZipBaseline,
                     pre.initialPid,
                     setup.localConfigFile, result);

    return result;
}

} // namespace mem_analyzer
