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
#include "profiler/profiler_cmd.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "profiler/config_loader.h"
#include "profiler/hdc_runner.h"
#include "profiler/hdc_target.h"
#include "profiler/process_utils.hpp"
#include "profiler/string_utils.hpp"
#include "profiler/template_processor.h"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {

namespace {

constexpr int SHELL_REDIRECT_SUFFIX_LEN = 5;
constexpr int STREAM_POLL_INTERVAL_MS = 20;

// Run a command, streaming its stdout/stderr to the console in real time
// (instead of capturing it all and printing once at the end).
// On Windows, uses CreateProcessW directly (not cmd.exe /c) to avoid
// the cmd.exe quote-stripping bug that corrupts multi-quoted command lines.
// Stderr is merged into stdout via STARTUPINFO hStdError = hStdOutput pipe.
// The caller must NOT append "2>&1" to the command (it is handled internally).
// Returns the exit code, or -1 if the command could not be launched.
int RunCommandStreaming(const std::string& cmd)
{
#if defined(_WIN32)
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE hRead = nullptr;
    HANDLE hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return -1;
    }
    if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return -1;
    }

    STARTUPINFOW si{ sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};

    // Run the command directly via CreateProcessW — no cmd.exe /c shell.
    // This avoids cmd.exe's first/last-quote stripping behavior which
    // corrupts command lines with multiple quoted arguments.
    // Strip any trailing "2>&1" that the caller may have appended (it's
    // redundant since STARTUPINFO already merges stderr into stdout).
    std::string cleanCmd = cmd;
    if (cleanCmd.size() >= SHELL_REDIRECT_SUFFIX_LEN && cleanCmd.substr(cleanCmd.size() - SHELL_REDIRECT_SUFFIX_LEN) == " 2>&1") {
        cleanCmd.resize(cleanCmd.size() - SHELL_REDIRECT_SUFFIX_LEN);
    }
    std::string cmdline = cleanCmd;
    std::wstring wcmdline = WidenPath(cmdline);
    if (!CreateProcessW(nullptr, wcmdline.data(),
        nullptr, nullptr, TRUE, 0, nullptr, nullptr,
        &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return -1;
    }
    CloseHandle(hWrite);

    auto drainAvailable = [&hRead]() {
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
                &n, nullptr) || n == 0) break;
            std::cout.write(buf, n);
            std::cout.flush();
        }
    };

    for (;;) {
        drainAvailable();
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
            break;
        }
        Sleep(STREAM_POLL_INTERVAL_MS);
    }
    drainAvailable();
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(code);
#else
    auto sp = proc::SpawnPiped(cmd);
    if (!sp.Ok()) {
        return -1;
    }
    char buf[4096];
    ssize_t n = 0;
    while ((n = read(sp.readFd, buf, sizeof(buf))) > 0) {
        std::cout.write(buf, n);
        std::cout.flush();
    }
    close(sp.readFd);
    int st = 0;
    waitpid(sp.pid, &st, 0);
    if (WIFEXITED(st)) {
        return WEXITSTATUS(st);
    }
    return -1;
#endif
}

// Pre-flight environment checks for the `profiler` subcommand.
bool CheckProfilerEnv()
{
    // 1. hdc binary available?
    {
        auto cap = proc::FindInPath("hdc");
        if (!cap.spawned || cap.exitCode != 0 || cap.output.empty()) {
            log::Loge("Environment check failed: hdc command-line tool not found.\n\n"
                      "Suggested fix:\n"
                      "  1. Install the HarmonyOS SDK toolkit.\n"
                      "  2. Add the hdc directory to your PATH.\n"
                      "  3. Verify: run `hdc version`.");
            return false;
        }
    }

    // 2. at least one device connected?
    {
        auto cap = proc::CaptureCommand("hdc list targets 2>&1");
        if (!cap.spawned) {
            log::Loge("Environment check failed: unable to launch hdc list targets.");
            return false;
        }
        bool hasDevice = false;
        if (!cap.output.empty() && cap.output.find("[Empty]") == std::string::npos) {
            std::string line;
            std::istringstream iss(cap.output);
            while (std::getline(iss, line)) {
                const auto f = line.find_first_not_of(" \t\r\n");
                if (f != std::string::npos) {
                    hasDevice = true;
                    break;
                }
            }
        }
        if (!hasDevice) {
            log::Loge("Environment check failed: no connected device detected.\n\n"
                      "Suggested fix:\n"
                      "  1. Connect your HarmonyOS device via USB.\n"
                      "  2. Enable USB debugging in Developer options.\n"
                      "  3. Verify: run `hdc list targets`.");
            return false;
        }
    }

    return true;
}

// ---- Target device gate (--device) ----
// Runs before ANY hdc command (including `--list-apps`) and decides the target once:
//   >=2 devices listed       : reject, listing the candidates (id count decides, no
//                              complete parse needed, and a "[Fail]" line does not
//                              veto the count — see the (3) branch below)
//   exactly 1 device, clean listing : pin its id internally, so every command
//                              carries -t
//   1 candidate, untrustworthy listing : reject and ask for --device (we would
//                              otherwise have to guess, and "run untargeted" means
//                              hdc picks the device)
//   0 devices / hdc failure  : leave commands untargeted (no -t) and stay quiet,
//                              which must stay byte-identical to the pre-change
//                              behaviour; with unrecognized output present a
//                              warning is printed, as the count is then not certain

constexpr const char* HDC_MISSING_MSG =
    "Environment check failed: hdc command-line tool not found.";
constexpr int EXIT_DIR_CREATE_FAILED = 2;
constexpr const char* FALLBACK_NOT_MATCH =
    "[Fail]Not match target founded, check connect-key please";
constexpr const char* FALLBACK_NEED_KEY =
    "[Fail]ExecuteCommand need connect-key? please confirm a device by help info";
constexpr const char* EMPTY_KEY_MSG =
    "Error: --device requires a non-empty value (device serial number).";
constexpr const char* AMBIGUOUS_LIST_MSG =
    "Error: cannot determine the target device: 'hdc list targets' returned an entry this "
    "tool cannot recognize, so another device may be attached as well.\n"
    "Name the device explicitly with --device to continue.";
constexpr size_t MAX_PROBE_KEY_LEN = 256;   // longer values skip the probe command
constexpr size_t MULTI_DEVICE_THRESHOLD = 2;

// Surrounding whitespace is invisible in a shell and would otherwise make an
// otherwise correct id miss the exact match against `hdc list targets` (`hdc -t`
// would not match it either), so the gate compares trimmed values.
std::string TrimWs(const std::string& s)
{
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) {
        return {};
    }
    const auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

// Probe command: explicit target (never touches global state); the device side runs
// the harmless no-op ':'.
static std::string HdcProbe(const std::string& target)
{
    auto cap = proc::CaptureCommand(
        HdcRawWithTarget(target, "shell " + proc::ShellQuote(":")) + " 2>&1");
    return cap.spawned ? cap.output : std::string();
}

// Candidate devices + rejected-target note + a copy-paste example.
static void AppendTargetHints(std::string& out, const std::vector<std::string>& keys, const std::string& rejectedKey)
{
    if (!keys.empty()) {
        out += "Available targets (" + std::to_string(keys.size()) + "):\n";
        for (const auto& k : keys) {
            out += "  " + k + "\n";
        }
    }
    if (!rejectedKey.empty()) {
        out += "Note: '" + rejectedKey +
               "' is not a usable target now (it may be Offline, unauthorized, or disconnected).\n";
    }
    if (!keys.empty()) {
        out += "Example:\n  mem-analyzer profiler --device " + keys.front() +
               " <bundle> -c nativeheap -t 30\n";
    }
}

// Append the lines the parser could not classify, indented, so a rejected/ambiguous verdict
// says exactly what blocked it. Keeping the offending text in the message is also how a new
// hdc output shape becomes visible instead of turning into an unexplained error.
static void AppendUnparsedLines(std::string& out, const HdcTargets& t)
{
    const std::string lines = HdcUnparsedLines(t.rawOutput);
    if (lines.empty()) {
        return;
    }
    out += "Unrecognized 'hdc list targets' line(s):\n";
    std::istringstream iss(lines);
    std::string line;
    while (std::getline(iss, line)) {
        out += "  " + line + "\n";
    }
}

// hdc text + candidates + hints + example (the hdc line itself is left unchanged).
// NOTE: only hdc output may be passed in here — the text is filtered down to its
// "[Fail]" lines, which would swallow a message produced by this tool.
static std::string DecorateDeviceError(const std::string& hdcText,
    const std::vector<std::string>& keys,
    const std::string& rejectedKey)
{
    // Keep only the "[Fail]" lines: drops hdc internal log noise such as
    // "[F][2026-09-18 18:10:10] Send isDead channelId:...".
    std::string out = HdcFailureLines(hdcText);
    if (out.empty() || out.back() != '\n') {
        out += '\n';
    }
    AppendTargetHints(out, keys, rejectedKey);
    return out;
}

// Returns true when we may continue; on false, msg holds the full error message.
static bool ResolveDeviceTarget(const std::string& rawKey, bool keyProvided, std::string& msg)
{
    // Both branches below compare against `hdc list targets` verbatim, so normalize the
    // user's value once here. A whitespace-only value thereby becomes empty and lands in
    // branch (1), which is the intended reading of `--device " "`.
    const std::string key = TrimWs(rawKey);
    // (1) explicit empty value: `--device ""` is a parameter error, not "no device
    //     selected". hdc's own reaction is surfaced when it reports one; otherwise this
    //     tool's wording is used. Either way the candidate list is appended, so a user
    //     with several devices can see the ids to choose from.
    if (keyProvided && key.empty()) {
        auto t = HdcListTargets();
        std::string text = HdcProbe("");
        if (HdcIsDeviceFailure(text)) {
            msg = DecorateDeviceError(text, t.keys, "");
            return false;
        }
        // Our own wording must skip decorate_device_error(): that one filters the text
        // down to hdc's "[Fail]" lines, which would drop this message entirely.
        msg = std::string(EMPTY_KEY_MSG) + "\n";
        AppendTargetHints(msg, t.keys, "");   // keys is empty when hdc is missing
        return false;
    }
    // (2) explicit target: it must match a target hdc currently lists, so a key that
    //     names an unplugged / unauthorized device is caught here rather than later.
    if (keyProvided) {
        auto t = HdcListTargets();
        if (t.spawnFailed) {
            msg = HDC_MISSING_MSG;
            return false;
        }
        bool found = false;
        for (const auto& k : t.keys) {
            if (k == key) {
                found = true;
                break;
            }
        }
        if (!found) {
            // fail-open: an incomplete parse means the candidate list may be wrong, so
            // let hdc decide instead of rejecting a possibly valid target locally.
            if (t.unparsedLines > 0) {
                HdcSetConnectKey(key);
                return true;
            }
            std::string text;
            if (key.size() <= MAX_PROBE_KEY_LEN) {
                text = HdcProbe(key);
            }
            if (!HdcIsDeviceFailure(text)) {
                text = FALLBACK_NOT_MATCH;
            }
            msg = DecorateDeviceError(text, t.keys, key);
            return false;
        }
        HdcSetConnectKey(key);   // matched a listed target
        return true;
    }
    // (3) not given: silently probe the device count.
    // The multi-device verdict needs no complete parse: two id-shaped keys already prove
    // that at least two devices are attached, and a further line we could not classify
    // cannot take that back. Requiring `unparsed_lines == 0` here used to let any hdc
    // chatter line (banner / dated log) drop us into the untargeted path, where every
    // command fails and the user is shown a misleading "could not parse version".
    // For the same reason a "[Fail]" line in the listing does NOT veto this verdict: two
    // parsed ids are stronger evidence than one failed line, and letting "[Fail]" win
    // would send untargeted commands to a two-device setup — the exact hazard the gate
    // exists to prevent.
    auto t = HdcListTargets();
    if (!t.spawnFailed && t.keys.size() >= MULTI_DEVICE_THRESHOLD) {
        std::string text = HdcProbe("");
        if (!HdcIsDeviceFailure(text)) {
            text = FALLBACK_NEED_KEY;
        }
        msg = DecorateDeviceError(text, t.keys, "");
        return false;
    }
    // One candidate, but the listing is not trustworthy (an unrecognized entry, or hdc
    // reported a failure while still listing a device): a second device could be hiding
    // in it. Both "pin the only id" and "run untargeted" would be guesses, and the latter
    // hands the device choice back to hdc, so ask the user to name the device instead.
    // Note this must come *after* the >=2 verdict above, which stays the more specific
    // message when the count already proves multiple devices.
    if (!t.spawnFailed && t.keys.size() == 1 && (t.unparsedLines > 0 || t.sawFail)) {
        msg = std::string(AMBIGUOUS_LIST_MSG) + "\n";
        AppendUnparsedLines(msg, t);
        AppendTargetHints(msg, t.keys, "");   // our wording: skip decorate_device_error()
        return false;
    }
    // Exactly one usable target: pin its id. Every business command then carries
    // `-t <id>`, which buys two things: unplugging this device mid-capture makes hdc
    // answer "[Fail]Not match" instead of silently routing the next command to a
    // newly attached device, and the pinned key is what arms the early device-loss
    // detection (note_device_failure() ignores "[Fail]" while no key is pinned).
    // (hdc's own chatter is skipped by the parser, so it does not block the pin.)
    if (!t.spawnFailed && !t.sawFail && t.unparsedLines == 0 && t.keys.size() == 1) {
        HdcSetConnectKey(t.keys[0]);
        return true;
    }
    // 0 devices, a failed spawn or "[Fail]": stay untargeted, i.e. keep the pre-change
    // command form (no -t) and let the existing paths report the problem. When the output
    // held entries we could not classify we are not actually sure the device count is 0,
    // so say so instead of silently emitting untargeted commands (this must not fire on a
    // clean 0-device listing, whose output has to stay byte-identical to the baseline).
    if (!t.spawnFailed && t.keys.empty() && t.unparsedLines > 0) {
        std::string warn = "Warning: 'hdc list targets' returned output this tool cannot recognize; continuing"
            " WITHOUT a pinned target (no -t), so hdc picks the device.\n";
        AppendUnparsedLines(warn, t);
        log::Logw(warn);
    }
    HdcMarkTargetResolved();   // must mark here too, or the builders report a missed gate
    return true;
}

// Check that the device's API version meets the minimum required.
bool CheckApiVersion(int minApiVersion)
{
    if (minApiVersion <= 0) {
        return true;
    }

    auto cap = proc::CaptureCommand(HdcShell("param get const.ohos.apiversion") + " 2>&1");
    if (!cap.spawned) {
        log::Loge("API version check failed: unable to launch hdc shell.");
        return false;
    }
    // A pinned target surfaces the hdc device-level failure verbatim, instead of
    // misreporting it as "could not parse version". Untargeted runs (no key pinned:
    // 0 devices, noisy parse, hdc failure) keep the original branch and wording.
    if (!HdcConnectKey().empty() && HdcIsDeviceFailure(cap.output)) {
        log::Loge(HdcFailureLines(cap.output));
        return false;
    }

    std::string trimmed = cap.output;
    const auto first = trimmed.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        log::Loge("API version check failed: empty response from device.");
        return false;
    }
    std::string numStr;
    for (size_t i = first; i < trimmed.size(); ++i) {
        if (trimmed[i] >= '0' && trimmed[i] <= '9') {
            numStr += trimmed[i];
        } else {
            break;
        }
    }
    if (numStr.empty()) {
        log::Loge("API version check failed: could not parse version.");
        return false;
    }

    int version = 0;
    try {
        version = std::stoi(numStr);
    } catch (...) {
        log::Loge("API version check failed: invalid version number.");
        return false;
    }

    if (version < minApiVersion) {
        log::Loge("API version check failed: device API version is ", version,
            ", but this capture type requires >= ", minApiVersion, ".");
        return false;
    }

    return true;
}

// Describes one profiler capture type: its CLI flag and the metadata derived
// from it (ArkTS-vs-native, zip-vs-o-file output, snapshot file prefix, and
// the report subcommand). Keeping these in one table avoids several scattered
// if-else chains that must stay in sync.
struct CaptureTypeInfo {
    const char* flag;            // CLI flag, e.g. "-s"
    bool requireApp;            // target must be an installed app (ArkTS types)
    bool waitForZip;           // result arrives as a hiprofiler_data_*.zip
    const char* snapshotPrefix; // local snapshot file prefix: "smaps" or "fds"
    const char* reportSubcmd;   // analysis subcommand used by --report
    const char* reportExtra;    // extra args for the report subcommand
};

const CaptureTypeInfo CAPTURE_TYPES[] = {
    {"snapshot",     true,  false, "smaps", "jsleak",   ""},
    {"nativeheap",   false, false, "smaps", "trace",    " -c nativeheap"},
    {"nativeleaks",  false, false, "smaps", "trace",    " -c nativeheap"},
    {"globalhandle", true,  true,  "smaps", "tshandle", ""},
    {"allhandle",    true,  true,  "smaps", "tshandle", ""},
    {"localhandle",  true,  true,  "smaps", "tshandle", ""},
    {"fd",           false, false, "fds",   "trace",    " -c fd"},
    {"dma",          false, false, "smaps", "trace",    " -c dma"},
    {"thread",       false, false, "smaps", "trace",    " -c thread"},
    {"ashmem",       false, false, "smaps", "trace",    " -c ashmem"},
};
constexpr size_t CAPTURE_TYPE_COUNT = sizeof(CAPTURE_TYPES) / sizeof(CAPTURE_TYPES[0]);

// Resolve a user-facing category string to a CaptureTypeInfo entry.
// Supports full names and short aliases:
//   nativeheap → nh, native
//   snapshot   → snap, sn
//   globalhandle → global, gh
//   allhandle    → handle, ah
//   localhandle  → local, lh
//   fd, dma, thread, ashmem → (no alias, already short)
// Returns nullptr and sets err_msg on unrecognized input.
const CaptureTypeInfo* ResolveCategory(const std::string& cat, std::string& errMsg)
{
    struct CategoryMapping {
        const char* name;
        const char* alias1;
        const char* alias2;
    };
    static const CategoryMapping mappings[] = {
        {"nativeheap",  "nh",     "native"},
        {"nativeleaks", "nl",     nullptr},
        {"snapshot",    "sn",     "snap"},
        {"globalhandle", "global", "gh"},
        {"allhandle",    "handle", "ah"},
        {"localhandle",  "local",  "lh"},
        {"fd",          nullptr,  nullptr},
        {"dma",         nullptr,  nullptr},
        {"thread",      nullptr,  nullptr},
        {"ashmem",      nullptr,  nullptr},
    };

    const char* resolved = nullptr;
    for (const auto& m : mappings) {
        if (cat == m.name || (m.alias1 && cat == m.alias1) || (m.alias2 && cat == m.alias2)) {
            resolved = m.name;
            break;
        }
    }
    if (!resolved) {
        errMsg = "invalid category '" + cat + "'. Valid: nativeheap/nh, nativeleaks/nl, snapshot/snap, global/gh, local/lh, fd, dma, thread, ashmem";
        return nullptr;
    }
    for (size_t i = 0; i < CAPTURE_TYPE_COUNT; ++i) {
        if (CAPTURE_TYPES[i].flag == resolved) {
            return &CAPTURE_TYPES[i];
        }
    }
    errMsg = "internal error: category '" + cat + "' resolved to unknown flag '" + resolved + "'";
    return nullptr;
}

// Query the device for installed apps via `bm dump -a -l`, parse the output
// into {package_name, app_name} pairs. Supports multiple output formats:
//   JSON:    [{"bundleName":"pkg","appName":"App"},...]
//   key-val: bundleName: pkg \n  name: App
//   simple:  pkg:App
struct AppEntry {
    std::string package;
    std::string appName;
};

// Result of an app query: carries the "hdc device-level failure" flag so callers can tell
// "there really is no app" apart from "device communication failed" (the latter must NOT
// fall back to the system-process path).
struct AppQueryResult {
    std::vector<AppEntry> apps;
    bool deviceFailed = false;   // set only when a target is pinned (see hdc_connect_key())
    std::string deviceErr;
};

static AppQueryResult QueryDeviceApps()
{
    AppQueryResult out;
    std::vector<AppEntry>& apps = out.apps;
    auto cap = proc::CaptureCommand(HdcShell("bm dump -a -l") + " 2>&1");
    // A pinned target surfaces the hdc device-level failure verbatim, instead of
    // misreading it as "no apps / system process". Untargeted runs are untouched.
    if (!HdcConnectKey().empty() && HdcIsDeviceFailure(cap.output)) {
        out.deviceFailed = true;
        out.deviceErr = HdcFailureLines(cap.output);
        return out;
    }
    if (!cap.spawned || cap.exitCode != 0) {
        return out;
    }

    const std::string& output = cap.output;

    // Try JSON format first: [{...}, ...]
    auto jsonStart = output.find('[');
    auto jsonEnd = output.rfind(']');
    if (jsonStart != std::string::npos && jsonEnd != std::string::npos &&
        jsonEnd > jsonStart) {
        std::string jsonStr = output.substr(jsonStart, jsonEnd - jsonStart + 1);
        // Simple JSON parser for the expected structure without nlohmann dep.
        size_t pos = 0;
        while ((pos = jsonStr.find("\"bundleName\"", pos)) != std::string::npos) {
            AppEntry entry;
            // Extract bundleName value
            auto colon = jsonStr.find(':', pos);
            if (colon == std::string::npos) {
                break;
            }
            auto valStart = jsonStr.find('"', colon + 1);
            if (valStart == std::string::npos) {
                break;
            }
            auto valEnd = jsonStr.find('"', valStart + 1);
            if (valEnd == std::string::npos) {
                break;
            }
            entry.package = jsonStr.substr(valStart + 1, valEnd - valStart - 1);

            // Extract app name (may be "label", "appName", or "name")
            auto extractLabel = [&jsonStr, &valEnd](const std::string& key) -> std::string {
                auto keyPos = jsonStr.find("\"" + key + "\"", valEnd);
                if (keyPos == std::string::npos) {
                    return "";
                }
                auto colon = jsonStr.find(':', keyPos);
                if (colon == std::string::npos) {
                    return "";
                }
                auto val = jsonStr.find('"', colon + 1);
                if (val == std::string::npos) {
                    return "";
                }
                auto end = jsonStr.find('"', val + 1);
                if (end == std::string::npos) {
                    return "";
                }
                return jsonStr.substr(val + 1, end - val - 1);
            };
            entry.appName = extractLabel("label");
            if (entry.appName.empty()) {
                entry.appName = extractLabel("appName");
            }
            if (entry.appName.empty()) {
                entry.appName = extractLabel("name");
            }
            if (!entry.package.empty()) {
                apps.push_back(std::move(entry));
            }
            pos = valEnd + 1;
        }
        if (!apps.empty()) {
            return out;
        }
    }

    // Fallback: line-by-line key-value parsing.
    AppEntry current;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        // Trim
        auto f = line.find_first_not_of(" \t\r\n");
        if (f == std::string::npos) {
            // Empty line: flush current entry if it has a package name
            if (!current.package.empty()) {
                apps.push_back(std::move(current));
                current = AppEntry{};
            }
            continue;
        }
        line = line.substr(f);

        // Try "key: value" or "key=value" format
        auto sep = line.find(':');
        if (sep == std::string::npos || sep == 0 || sep >= line.size() - 1) {
            // Check for "pkg:AppName" format (no space after colon)
            // Line itself might be a package name
            if (line.find('.') != std::string::npos && line.find(' ') == std::string::npos) {
                auto colon2 = line.find(':');
                if (colon2 != std::string::npos) {
                    // Format: com.example.app:App Name
                    current.package = line.substr(0, colon2);
                    current.appName = line.substr(colon2 + 1);
                    apps.push_back(std::move(current));
                    current = AppEntry{};
                } else {
                    // Just a package name line
                    current.package = line;
                }
            }
            continue;
        }
        std::string key = line.substr(0, sep);
        // Trim key
        while (!key.empty() && key.back() == ' ') {
            key.pop_back();
        }

        auto valStart = line.find_first_not_of(": =", sep);
        std::string val = (valStart != std::string::npos) ? line.substr(valStart) : "";

        if (key == "bundleName") {
            if (!current.package.empty()) {
                apps.push_back(std::move(current));
            }
            current.package = val;
        } else if (key == "name" || key == "appName") {
            current.appName = val;
        }
    }
    // Flush last entry
    if (!current.package.empty()) {
        apps.push_back(std::move(current));
    }
    return out;
}

// Resolve user input (partial package/app name) to an exact package name.
// For ArkTS capture types (require_app=true, -s/-g/-l) the target must be an
// installed app from `bm dump -a -l`. Native types (require_app=false,
// -n/-f/-d) can also analyze system processes, so a non-app input is kept
// verbatim instead of being rejected.
// Returns true and sets resolved_pkg on success (including the verbatim
// system-process fallback), and sets is_app to whether the resolved target is
// an installed app. Prints diagnostic info and returns false on unresolvable
// input.
static bool ResolveProcessName(const std::string& input, bool requireApp, std::string& resolvedPkg, bool& isApp)
{
    // Any target resolved from the installed-app list is an app; the verbatim
    // system-process fallbacks below override this to false.
    isApp = true;
    auto q = QueryDeviceApps();
    // A pinned target reports the device-level failure here, instead of misreading it as
    // a system process and carrying on with a wrong premise.
    if (q.deviceFailed) {
        log::Loge(q.deviceErr);
        return false;
    }
    const auto& apps = q.apps;
    if (apps.empty()) {
        if (!requireApp) {
            resolvedPkg = input;
            isApp = false;
            log::Logi("Resolved: ", input, " -> package: ", resolvedPkg,
                " (unable to query apps; treated as a system process)");
            return true;
        }
        log::Loge("Error: unable to query device apps via 'bm dump -a -l'.\n"
            "Please ensure a device is connected and try again.");
        return false;
    }

    // Exact match first (case-insensitive on package name)
    for (const auto& a : apps) {
        if (ContainsCi(a.package, input) && a.package.size() == input.size()) {
            // If multiple exact matches on different fields, still prefer package exact
            resolvedPkg = a.package;
            log::Logi("Resolved: ", input, " -> package: ", resolvedPkg,
                (!a.appName.empty() ? (" (" + a.appName + ")") : std::string()));
            return true;
        }
    }
    for (const auto& a : apps) {
        if (ContainsCi(a.appName, input) && a.appName.size() == input.size()) {
            resolvedPkg = a.package;
            log::Logi("Resolved: ", input, " -> package: ", resolvedPkg,
                (!a.appName.empty() ? (" (" + a.appName + ")") : std::string()));
            return true;
        }
    }

    // Fuzzy match: substring in app name (label) only.
    // Package names require exact match to avoid false positives.
    std::vector<const AppEntry*> matches;
    for (const auto& a : apps) {
        if (!a.appName.empty() && ContainsCi(a.appName, input)) {
            matches.push_back(&a);
        }
    }

    if (matches.empty()) {
        if (!requireApp) {
            // Native capture types can also analyze system processes that are
            // not installed apps (e.g. system services / daemons).
            resolvedPkg = input;
            isApp = false;
            log::Logi("Resolved: ", input, " -> package: ", resolvedPkg,
                " (not an installed app; treated as a system process)");
            return true;
        }
        log::Loge("Error: no app matches '", input, "'.\n"
            "Use 'mem-analyzer prof --list-apps' to see available apps,\n"
            "or provide an exact package name or a partial app name.");
        return false;
    }

    if (matches.size() == 1) {
        resolvedPkg = matches[0]->package;
        log::Logi("Resolved: ", input, " -> package: ", resolvedPkg,
            (!matches[0]->appName.empty() ? (" (" + matches[0]->appName + ")") : std::string()));
        return true;
    }

    // Multiple matches: list them
    std::string msg = "Error: '" + input + "' matches " + std::to_string(matches.size()) + " apps:\n";
    for (const auto* m : matches) {
        msg += "  " + m->package;
        if (!m->appName.empty()) {
            msg += " (" + m->appName + ")";
        }
        msg += "\n";
    }
    msg += "Please provide a more specific name.";
    log::Loge(msg);
    return false;
}

// Make `name` usable as a single path component: for system processes the
// resolved target is the user's verbatim input (see resolve_process_name), and
// platform::join_path does not normalize the path, so a separator would
// silently add directory levels (or `..` escape the output dir). Characters
// that are illegal in a path component on Windows are replaced as well and a
// trailing space/dot is dropped, so a session directory is valid on every
// platform the tool ships for.
static std::string AsPathSegment(const std::string& name)
{
    static const std::string illegal = "/\\:*?\"<>|";
    std::string out;
    out.reserve(name.size());
    for (const char c : name) {
        out += (illegal.find(c) != std::string::npos) ? '_' : c;
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '.')) {
        out.pop_back();
    }
    return out;
}

// Resolve the mem-analyzer binary path: prefer the directory of the
// current executable, fall back to the bare name on PATH.
std::string ResolveMemAnalyzerBinary()
{
#if defined(_WIN32)
    constexpr const char* binaryName = "mem-analyzer.exe";
#else
    constexpr const char* binaryName = "mem-analyzer";
#endif
    const std::string exeDir = proc::GetExecutableDir();
    return exeDir.empty() ? binaryName :
                           platform::JoinPath(exeDir, binaryName);
}

// Undo the nesting the analysis subcommands apply to their output: move
// everything a report run produced from <dir>/<stem>/ up into <dir> and drop
// the directory once nothing is left in it. Used when a report is generated
// into a session directory, where that extra level is redundant. `stem` is the
// input file stem.
void FlattenReportDir(const std::string& dir, const std::string& stem)
{
    const std::string nestedDir = platform::JoinPath(dir, stem);
    std::error_code ec;
    if (!fs::is_directory(PathFromUtf8(nestedDir), ec) || ec) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(PathFromUtf8(nestedDir), ec)) {
        if (ec) {
            break;
        }
        std::error_code typeEc;
        if (!entry.is_regular_file(typeEc) || typeEc) {
            continue;
        }
        const std::string dest =
            platform::JoinPath(dir, PathToUtf8(entry.path().filename()));
        std::error_code moveEc;
        fs::rename(entry.path(), PathFromUtf8(dest), moveEc);
        if (moveEc) {
            log::Loge("Report: unable to move ", PathToUtf8(entry.path()),
                " into ", dir, " (", moveEc.message(), ")");
        }
    }

    // Fails while a subdirectory (or an unmovable file) is still there.
    std::error_code rmEc;
    fs::remove(PathFromUtf8(nestedDir), rmEc);
    if (rmEc) {
        log::Logw("Warning: ", nestedDir, " was kept (", rmEc.message(), ")");
    }
}

constexpr int EXIT_PROFILER_FAILED = 2;
constexpr int EXIT_EXPORT_FAILED = 3;
constexpr int EXIT_CONFIG_FAILED = 2;
constexpr int EXIT_API_CHECK_FAILED = 2;

} // namespace

int RunProfilerCmd(const ProfilerCliOptions& opt)
{
    // 1. Determine config path and load config.
    std::string configPath = ResolveConfigPath();
    CmdConfig cfg;
    std::string err;
    if (!LoadCmdConfig(configPath, cfg, err)) {
        log::Loge("Failed to load config: ", err);
        return 2;
    }

    // 2. The options were already parsed by the main CliParser (see main.cpp),
    // so --help/validation are handled by the shared CLI architecture.

    // 3. Local copies of the parsed options.
    std::string processName = opt.processName;
    int durationVal = opt.duration;
    std::string outputDirVal = opt.outputDir;
    std::string category = opt.category;
    bool flagReport = opt.flagReport;
    bool flagListApps = opt.flagListApps;

    // 3.4 Target device gate: decides the target before ANY hdc command (including
    //     --list-apps) — see resolve_device_target(). It prints nothing on the
    //     untargeted paths, which keeps 0-device output byte-identical.
    {
        std::string gateMsg;
        if (!ResolveDeviceTarget(opt.connectKey, opt.connectKeyProvided, gateMsg)) {
            log::Loge(gateMsg);
            return 2;
        }
    }

    // 3.45 Echo the target once, right after resolution: printed whenever a device
    //      actually drives the run — an explicit `--device` or the only attached device
    //      the gate pinned. This is the only place it is printed, so it leads every
    //      downstream path (including the ones that abort on a later check) and stays
    //      off the untargeted paths, which either failed the gate above or never
    //      resolved a key. The capture transcript is per target process otherwise, and
    //      a user running two devices side by side cannot tell the two consoles apart.
    //      This is purely diagnostic and intentionally independent of the output
    //      directory layer, which is added only for an explicit `--device` (see the
    //      session_path block below): an auto-pinned run still echoes the id it picked.
    if (!HdcConnectKey().empty()) {
        log::Logi("Device: ", HdcConnectKey());
    }

    // 3.5 Handle --list-apps early (before capture type validation).
    if (flagListApps) {
        auto q = QueryDeviceApps();
        if (q.deviceFailed) {
            log::Loge(q.deviceErr);
            return 2;
        }
        const auto& apps = q.apps;
        if (apps.empty()) {
            log::Loge("No apps found or unable to query device.");
            return 2;
        }
        std::cout << "Installed apps (" << apps.size() << "):\n";
        for (const auto& a : apps) {
            std::cout << "  " << a.package;
            if (!a.appName.empty()) {
                std::cout << "  (" << a.appName << ")";
            }
            std::cout << "\n";
        }
        return 0;
    }

    // 4. Resolve category string to capture type.
    std::string errMsg;
    const CaptureTypeInfo* capture = ResolveCategory(category, errMsg);
    if (!capture) {
        log::Loge("Error:", errMsg);
        return 2;
    }
    if (processName.empty()) {
        log::Loge("Error: <bundle> is required for capture.");
        return 2;
    }
    const std::string configOption = capture->flag;

    // 4. Environment check.
    if (!CheckProfilerEnv()) {
        return 2;
    }

    // 4.5 Resolve process name: support partial matching of package name or app name.
    //     ArkTS types (-s/-g/-l) require an installed app; native types
    //     (-n/-f/-d) may also target system processes not in the app list.
    bool isApp = true;  // whether the resolved target is an installed app
    {
        std::string resolved;
        const bool requireApp = capture->requireApp;
        if (!ResolveProcessName(processName, requireApp, resolved, isApp)) {
            return 2;
        }
        processName = resolved;
    }

    // 5. Resolve config option to its template file path.
    std::string configFile;
    std::string resultDir;
    bool requireFreshStart = false;
    int minApiVersion = 0;
    for (const auto& op : cfg.optionPathMap) {
        if (op.option == configOption) {
            configFile = op.path;
            resultDir = op.resultDir;
            requireFreshStart = op.requireFreshStart;
            minApiVersion = op.minApiVersion;
            break;
        }
    }
    if (configFile.empty()) {
        log::Loge("Unknown config option: ", configOption);
        return 2;
    }

    // 6. API version check.
    if (!CheckApiVersion(minApiVersion)) {
        return EXIT_API_CHECK_FAILED;
    }

    // 7. Process template.
    ProfilerOptions popts;
    popts.p = processName;
    if (durationVal > 0) {
        popts.t = durationVal;
    } else {
        // Default to 120s when -t is not given, instead of finishing immediately.
        popts.t = 120;
    }
    popts.o = outputDirVal;
    popts.configOption = configOption;

    CaptureCommand cc = ProcessTemplate(configFile, popts);
    if (!cc.ok) {
        log::Loge("Failed to process config file:", cc.error);
        return EXIT_CONFIG_FAILED;
    }

    // Result output: one capture owns one session directory,
    //   <output_dir>/[<device-id>/]<capture type>-[session-]<target>-<timestamp>/
    // e.g. out/nativeheap-com.example.app-20260911_211900/, so the capture type
    // and target are visible in the path and every capture type follows the
    // same layout instead of a directory named after the device-side result
    // file (which is the same for most hiprofiler types). The device-side file
    // keeps its own name; the directory timestamp is the host time from
    // current_timestamp(), unrelated to any timestamp the device puts in the
    // file name.
    // The session is nested under the device id — exactly the value `hdc -t` receives —
    // only when the user named the device with `--device`. An auto-pinned single device
    // deliberately does NOT add that layer: the user never asked for a per-device
    // partition, so `-d <dir>` (or the default `out`) keeps the flat shape it had before
    // the feature existed. When `--device` IS given, captures aimed at different devices
    // can never share one folder and a report stays attributable to its device. The key
    // is used as a single path component, so characters that are illegal in one are
    // replaced (see as_path_segment). The untargeted legacy paths (0 devices, an
    // unclassifiable line shape, hdc missing or failing) stay flat as well, and those
    // abort before any directory is created.
    // snapshot with an explicit -t keeps monitoring for the whole duration
    // (start/end + peak-triggered snapshots) instead of exporting once. Both
    // modes share the capture type, so the continuous one carries a `session`
    // segment to tell them apart in the directory name alone.
    const bool continuousSnapshot =
        (configOption == "snapshot") && opt.durationSpecified;
    const std::string sessionName =
        configOption + (continuousSnapshot ? "-session" : "") + "-" +
        AsPathSegment(processName) + "-" + CurrentTimestamp();
    const std::string deviceSegment =
        (opt.connectKeyProvided && !HdcConnectKey().empty()) ?
            AsPathSegment(HdcConnectKey()) :
            std::string();
    const std::string sessionPath =
        deviceSegment.empty() ? sessionName :
                               platform::JoinPath(deviceSegment, sessionName);
    const std::string outputDir = platform::JoinPath(outputDirVal, sessionPath);
    // create_directories also reports false for an existing directory.
    if (!platform::CreateDirectories(outputDir) && !platform::DirectoryExists(outputDir)) {
        log::Loge("Error: cannot create the output directory: ", outputDir);
        return EXIT_DIR_CREATE_FAILED;
    }

    // 8. Run hdc. The target was already echoed right after the gate (step 3.45).
    HdcRunOptions runOpts;
    runOpts.content = cc.command;
    runOpts.duration = cc.duration;
    runOpts.outputDir = outputDir;
    runOpts.remoteOutputPath = cc.remoteOutputPath;
    runOpts.processName = cc.processName;
    runOpts.resultDir = resultDir;
    runOpts.requireFreshStart = requireFreshStart;
    runOpts.waitForZip = capture->waitForZip;
    runOpts.pullNativeleaks = (configOption == "nativeleaks");
    runOpts.deviceConfigContent = cc.deviceConfigContent;
    runOpts.smapsPrefix = capture->snapshotPrefix;
    runOpts.isApp = isApp;
    runOpts.continuousSnapshot = continuousSnapshot;
    RunResult rr = RunHdc(runOpts);
    if (!rr.ok) {
        // Name the pinned device when there is one: rr.error is hdc text and does not always
        // mention the target, and by now the `Device:` echo is well above this line.
        std::string err = "hdc execution failed: " + rr.error;
        if (!HdcConnectKey().empty()) {
            err += " [device: " + HdcConnectKey() + "]";
        }
        log::Loge(err);
        if (!rr.output.empty() && !rr.outputStreamed) {
            std::cout << rr.output;
        }
        return EXIT_PROFILER_FAILED;
    }

    // 9. Report results.
    // `output_streamed` means the transcript was already printed to the console
    // in real time (async `hiprofiler_cmd start`), so don't print it again.
    if (!rr.output.empty() && !rr.outputStreamed) {
        std::cout << rr.output;
        if (rr.output.back() != '\n') {
            std::cout << '\n';
        }
    }
    log::Logi("Elapsed: ", rr.elapsed, "s");

    if (rr.snapshotAttempted) {
        log::Logi("Snapshot status: ", rr.snapshotStatus);
        for (const auto& f : rr.snapshotPulledFiles) {
            log::Logok("Snapshot file exported: ", f);
        }
    } else if (rr.pullAttempted) {
        if (rr.pullOk) {
            log::Logok("Result file exported: ", rr.pulledFile);
        } else {
            // `hdc file recv` exits 0 even when the remote file is missing,
            // so the captured output holds the real reason (e.g. "[Fail]Error
            // opening file: ..."); prefer it over the exit code.
            if (!rr.pullOutput.empty()) {
                log::Loge("Result file export failed:\n", rr.pullOutput);
            } else {
                log::Loge("Result file export failed: (exit code ", rr.pullExitCode, ")");
            }
            return EXIT_EXPORT_FAILED;
        }
    }

    // Summarize the per-process snapshot files exported during the capture
    // (hidumper --mem-smaps for -n/-g/-l/-d, lsof for -f). The paths were
    // already printed at the moment of export; this consolidates them.
    if (!rr.smapsExportedFiles.empty()) {
        const char* label = (std::string(capture->snapshotPrefix) == "fds") ?
                                "FD snapshot exported" :
                                "Smaps exported";
        for (const auto& f : rr.smapsExportedFiles) {
            log::Logi(label, ": ", f);
        }
    }

    // 10. Generate HTML analysis report if --report is set.
    if (flagReport) {
        // Continuous snapshot sessions (snapshot -t) produce start + peak + end
        // snapshots. With -o, generate a single comparison report (start=baseline,
        // end=target) via jsleak -b, instead of per-file reports.
        if (continuousSnapshot && !rr.snapshotPulledFiles.empty()) {
            std::string baselineFile;
            std::string targetFile;
            for (const auto& f : rr.snapshotPulledFiles) {
                const std::string stem = platform::Stem(f);
                if (stem.empty()) {
                    continue;
                }
                const std::string startSfx = "-start";
                const std::string endSfx = "-end";
                if (baselineFile.empty() &&
                    stem.size() >= startSfx.size() &&
                    stem.compare(stem.size() - startSfx.size(),
                        startSfx.size(), startSfx) == 0) {
                    baselineFile = f;
                } else if (targetFile.empty() &&
                    stem.size() >= endSfx.size() &&
                    stem.compare(stem.size() - endSfx.size(),
                        endSfx.size(), endSfx) == 0) {
                    targetFile = f;
                }
            }

            // Fallback: if either was not located by role suffix, use file
            // modification timestamps — earliest = start (baseline),
            // latest = end (target).
            if (baselineFile.empty() || targetFile.empty()) {
                log::Logi("Role suffix not found on all snapshots; "
                    "falling back to file modification timestamps.");
                fs::file_time_type minTime = fs::file_time_type::max();
                fs::file_time_type maxTime = fs::file_time_type::min();
                std::string earliest;
                std::string latest;
                for (const auto& f : rr.snapshotPulledFiles) {
                    std::error_code ec;
                    const auto t = fs::last_write_time(PathFromUtf8(f), ec);
                    if (ec) {
                        continue;
                    }
                    if (t < minTime) {
                        minTime = t;
                        earliest = f;
                    }
                    if (t > maxTime) {
                        maxTime = t;
                        latest = f;
                    }
                }
                if (baselineFile.empty()) {
                    baselineFile = earliest;
                }
                if (targetFile.empty()) {
                    targetFile = latest;
                }
            }

            if (baselineFile.empty() || targetFile.empty() ||
                baselineFile == targetFile) {
                log::Loge("Comparison report: could not locate start/end snapshots ",
                    "(baseline=", baselineFile.empty() ? "<missing>" : baselineFile,
                    ", target=", targetFile.empty() ? "<missing>" : targetFile, ").");
                return rr.exitCode;
            }

            log::Logi("Generating comparison report: baseline=start, target=end");

            const std::string binary = ResolveMemAnalyzerBinary();

            // jsleak -b <baseline> <target> -d <output_dir>
            std::string cmd = proc::ShellQuote(binary) +
                " jsleak -b " + proc::ShellQuote(baselineFile) +
                " " + proc::ShellQuote(targetFile) +
                " -d " + proc::ShellQuote(outputDir);
            // Forward ArkTS deobfuscation options (same as per-file path).
            if (!opt.sourcemapDir.empty()) {
                cmd += " --sourcemap " + proc::ShellQuote(opt.sourcemapDir);
            }
            if (!opt.nameObfuscation.empty()) {
                cmd += " --namecache " + proc::ShellQuote(opt.nameObfuscation);
            }

            log::Logd("Command: ", cmd);
            int rc = RunCommandStreaming(cmd + " 2>&1");
            if (rc < 0) {
                log::Loge("Comparison report generation failed: unable to launch ", binary);
            } else if (rc != 0) {
                log::Loge("Comparison report generation failed: exited with code ", rc);
            } else {
                // The jsleak compare subcommand nests its output under
                // <output_dir>/<target-stem>/, writing
                // <baseline-stem>-vs-<target-stem>.html there. Lift it up.
                const std::string targetStem = platform::Stem(targetFile);
                const std::string baselineStem = platform::Stem(baselineFile);
                if (!targetStem.empty()) {
                    FlattenReportDir(outputDir, targetStem);
                }
                const std::string htmlName =
                    (baselineStem.empty() ? "baseline" : baselineStem) + "-vs-" +
                    (targetStem.empty() ? "target" : targetStem) + ".html";
                const std::string htmlPath = platform::JoinPath(outputDir, htmlName);
                log::Logok("Comparison report generated: ", htmlPath);
                if (platform::FileExists(htmlPath)) {
                    platform::OpenWithDefaultApp(htmlPath);
                } else {
                    log::Loge("Comparison report HTML not found at: ", htmlPath,
                        " (the capture may have contained no data).");
                }
            }
            return rr.exitCode;
        }

        // Collect the file(s) to analyze.
        std::vector<std::string> reportFiles;
        if (rr.snapshotAttempted) {
            reportFiles = rr.snapshotPulledFiles;
        } else if (rr.pullAttempted && rr.pullOk && !rr.pulledFile.empty()) {
            reportFiles.push_back(rr.pulledFile);
        }

        if (reportFiles.empty()) {
            log::Loge("Report: no captured file available to analyze.");
        } else {
            // Determine the subcommand and extra args from the capture type.
            const std::string subcmd = capture->reportSubcmd;
            const std::string extraArgs = capture->reportExtra;

            const std::string binary = ResolveMemAnalyzerBinary();

            for (const auto& f : reportFiles) {
                // Every capture file of a session lives directly in the session
                // directory, so analyzing it there keeps the report inside the
                // session tree. The analysis subcommand writes it one level
                // deeper, under <dir>/<stem>/, which is undone below.
                // nativeleaks: feed the orphan-snapshot file to `trace -l`
                // alongside the .htrace so the L1/L2/L3 leak filtering runs.
                std::string leaksArg;
                if (configOption == "nativeleaks") {
                    if (!rr.pulledLeaksFile.empty()) {
                        leaksArg = " -l " + proc::ShellQuote(rr.pulledLeaksFile);
                    } else {
                        // leaks 文件未获取（超时/设备掉线）：明确告知用户本次
                        // 未做 -l 泄漏过滤，仅输出普通 nativeheap 报告，避免
                        // 静默降级让用户误以为泄漏过滤已运行。
                        log::Logw("Warning: nativeleaks orphan-snapshot file was not captured; "
                                  "running trace WITHOUT -l leak filtering (plain nativeheap "
                                  "analysis only). Re-run the capture if leak filtering is needed.");
                    }
                }
                std::string cmd = proc::ShellQuote(binary) +
                    " " + subcmd + extraArgs + leaksArg +
                    " " + proc::ShellQuote(f) +
                    " -d " + proc::ShellQuote(outputDir);
                // 转发反混淆选项给 jsleak 报告子命令，还原混淆的符号/路径后再分析
                if (!opt.sourcemapDir.empty()) {
                    cmd += " --sourcemap " + proc::ShellQuote(opt.sourcemapDir);
                }
                if (!opt.nameObfuscation.empty()) {
                    cmd += " --namecache " + proc::ShellQuote(opt.nameObfuscation);
                }
                // native .so 符号反解只对 trace/tshandle 分析有意义（jsleak 走 ArkTS 反混淆）
                if (!opt.soDir.empty() && (subcmd == "trace" || subcmd == "tshandle")) {
                    cmd += " --so-dir " + proc::ShellQuote(opt.soDir);
                }

                log::Logd("Generating report: ", cmd);
                int rc = RunCommandStreaming(cmd + " 2>&1");
                if (rc < 0) {
                    log::Loge("Report generation failed: unable to launch ", binary);
                } else if (rc != 0) {
                    log::Loge("Report generation failed: exited with code ", rc);
                } else {
                    // The analysis subcommand nests its output under
                    // <output_dir>/<source_name>/, where source_name is the input
                    // file stem. That level is redundant here: the capture file
                    // already lives in its own session directory, so lift
                    // everything the run produced next to it and report the
                    // final location (the subprocess printed the old one).
                    std::string base = platform::Stem(f);
                    if (base.empty()) {
                        base = "output";
                    }
                    FlattenReportDir(outputDir, base);

                    const std::string htmlPath = platform::JoinPath(outputDir, base + ".html");
                    log::Logok("Report generated: ", htmlPath);
                    if (platform::FileExists(htmlPath)) {
                        // Open the generated HTML in the default browser.
                        platform::OpenWithDefaultApp(htmlPath);
                    } else {
                        log::Loge("Report HTML not found at: ", htmlPath,
                            " (the capture may have contained no data).");
                    }
                }
            }
        }
    }

    return rr.exitCode;
}

} // namespace mem_analyzer
