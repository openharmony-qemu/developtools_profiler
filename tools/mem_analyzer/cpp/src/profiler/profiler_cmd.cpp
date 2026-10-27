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

// Run a command, streaming its stdout/stderr to the console in real time
// (instead of capturing it all and printing once at the end).
// On Windows, uses CreateProcessW directly (not cmd.exe /c) to avoid
// the cmd.exe quote-stripping bug that corrupts multi-quoted command lines.
// Stderr is merged into stdout via STARTUPINFO hStdError = hStdOutput pipe.
// The caller must NOT append "2>&1" to the command (it is handled internally).
// Returns the exit code, or -1 if the command could not be launched.
int run_command_streaming(const std::string& cmd) {
#if defined(_WIN32)
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE h_read = nullptr, h_write = nullptr;
    if (!CreatePipe(&h_read, &h_write, &sa, 0)) return -1;
    if (!SetHandleInformation(h_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        CloseHandle(h_read); CloseHandle(h_write); return -1;
    }

    STARTUPINFOW si{ sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = h_write;
    si.hStdError = h_write;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};

    // Run the command directly via CreateProcessW — no cmd.exe /c shell.
    // This avoids cmd.exe's first/last-quote stripping behavior which
    // corrupts command lines with multiple quoted arguments.
    // Strip any trailing "2>&1" that the caller may have appended (it's
    // redundant since STARTUPINFO already merges stderr into stdout).
    std::string clean_cmd = cmd;
    if (clean_cmd.size() >= 5 && clean_cmd.substr(clean_cmd.size() - 5) == " 2>&1") {
        clean_cmd.resize(clean_cmd.size() - 5);
    }
    std::string cmdline = clean_cmd;
    std::wstring wcmdline = widen_path(cmdline);
    if (!CreateProcessW(nullptr, wcmdline.data(),
                        nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                        &si, &pi)) {
        CloseHandle(h_read); CloseHandle(h_write);
        return -1;
    }
    CloseHandle(h_write);

    auto drain_available = [&]() {
        for (;;) {
            DWORD avail = 0;
            if (!PeekNamedPipe(h_read, nullptr, 0, nullptr, &avail, nullptr)) break;
            if (avail == 0) break;
            char buf[4096];
            DWORD n = 0;
            if (!ReadFile(h_read, buf, (DWORD)std::min<size_t>(avail, sizeof(buf)),
                          &n, nullptr) || n == 0) break;
            std::cout.write(buf, n);
            std::cout.flush();
        }
    };

    for (;;) {
        drain_available();
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) break;
        Sleep(20);
    }
    drain_available();
    CloseHandle(h_read);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(code);
#else
    auto sp = proc::spawn_piped(cmd);
    if (!sp.ok()) return -1;
    char buf[4096];
    ssize_t n = 0;
    while ((n = read(sp.read_fd, buf, sizeof(buf))) > 0) {
        std::cout.write(buf, n);
        std::cout.flush();
    }
    close(sp.read_fd);
    int st = 0;
    waitpid(sp.pid, &st, 0);
    if (WIFEXITED(st)) return WEXITSTATUS(st);
    return -1;
#endif
}

// Pre-flight environment checks for the `profiler` subcommand.
bool check_profiler_env() {
    // 1. hdc binary available?
    {
        auto cap = proc::find_in_path("hdc");
        if (!cap.spawned || cap.exit_code != 0 || cap.output.empty()) {
            log::loge("Environment check failed: hdc command-line tool not found.\n\n"
                      "Suggested fix:\n"
                      "  1. Install the HarmonyOS SDK toolkit.\n"
                      "  2. Add the hdc directory to your PATH.\n"
                      "  3. Verify: run `hdc version`.");
            return false;
        }
    }

    // 2. at least one device connected?
    {
        auto cap = proc::capture_command("hdc list targets 2>&1");
        if (!cap.spawned) {
            log::loge("Environment check failed: unable to launch hdc list targets.");
            return false;
        }
        bool has_device = false;
        if (!cap.output.empty() && cap.output.find("[Empty]") == std::string::npos) {
            std::string line;
            std::istringstream iss(cap.output);
            while (std::getline(iss, line)) {
                const auto f = line.find_first_not_of(" \t\r\n");
                if (f != std::string::npos) {
                    has_device = true;
                    break;
                }
            }
        }
        if (!has_device) {
            log::loge("Environment check failed: no connected device detected.\n\n"
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

constexpr const char* kHdcMissingMsg =
    "Environment check failed: hdc command-line tool not found.";
constexpr const char* kFallbackNotMatch =
    "[Fail]Not match target founded, check connect-key please";
constexpr const char* kFallbackNeedKey =
    "[Fail]ExecuteCommand need connect-key? please confirm a device by help info";
constexpr const char* kEmptyKeyMsg =
    "Error: --device requires a non-empty value (device serial number).";
constexpr const char* kAmbiguousListMsg =
    "Error: cannot determine the target device: 'hdc list targets' returned an entry this "
    "tool cannot recognize, so another device may be attached as well.\n"
    "Name the device explicitly with --device to continue.";
constexpr size_t kMaxProbeKeyLen = 256;   // longer values skip the probe command

// Surrounding whitespace is invisible in a shell and would otherwise make an
// otherwise correct id miss the exact match against `hdc list targets` (`hdc -t`
// would not match it either), so the gate compares trimmed values.
std::string trim_ws(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return {};
    const auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

// Probe command: explicit target (never touches global state); the device side runs
// the harmless no-op ':'.
static std::string hdc_probe(const std::string& target) {
    auto cap = proc::capture_command(
        hdc_raw_with_target(target, "shell " + proc::shell_quote(":")) + " 2>&1");
    return cap.spawned ? cap.output : std::string();
}

// Candidate devices + rejected-target note + a copy-paste example.
static void append_target_hints(std::string& out, const std::vector<std::string>& keys,
                                const std::string& rejected_key) {
    if (!keys.empty()) {
        out += "Available targets (" + std::to_string(keys.size()) + "):\n";
        for (const auto& k : keys) out += "  " + k + "\n";
    }
    if (!rejected_key.empty()) {
        out += "Note: '" + rejected_key +
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
static void append_unparsed_lines(std::string& out, const HdcTargets& t) {
    const std::string lines = hdc_unparsed_lines(t.raw_output);
    if (lines.empty()) return;
    out += "Unrecognized 'hdc list targets' line(s):\n";
    std::istringstream iss(lines);
    std::string line;
    while (std::getline(iss, line)) out += "  " + line + "\n";
}

// hdc text + candidates + hints + example (the hdc line itself is left unchanged).
// NOTE: only hdc output may be passed in here — the text is filtered down to its
// "[Fail]" lines, which would swallow a message produced by this tool.
static std::string decorate_device_error(const std::string& hdc_text,
                                        const std::vector<std::string>& keys,
                                        const std::string& rejected_key) {
    // Keep only the "[Fail]" lines: drops hdc internal log noise such as
    // "[F][2026-09-18 18:10:10] Send isDead channelId:...".
    std::string out = hdc_failure_lines(hdc_text);
    if (out.empty() || out.back() != '\n') out += '\n';
    append_target_hints(out, keys, rejected_key);
    return out;
}

// Returns true when we may continue; on false, msg holds the full error message.
static bool resolve_device_target(const std::string& raw_key, bool key_provided,
                                  std::string& msg) {
    // Both branches below compare against `hdc list targets` verbatim, so normalize the
    // user's value once here. A whitespace-only value thereby becomes empty and lands in
    // branch (1), which is the intended reading of `--device " "`.
    const std::string key = trim_ws(raw_key);
    // (1) explicit empty value: `--device ""` is a parameter error, not "no device
    //     selected". hdc's own reaction is surfaced when it reports one; otherwise this
    //     tool's wording is used. Either way the candidate list is appended, so a user
    //     with several devices can see the ids to choose from.
    if (key_provided && key.empty()) {
        auto t = hdc_list_targets();
        std::string text = hdc_probe("");
        if (hdc_is_device_failure(text)) {
            msg = decorate_device_error(text, t.keys, "");
            return false;
        }
        // Our own wording must skip decorate_device_error(): that one filters the text
        // down to hdc's "[Fail]" lines, which would drop this message entirely.
        msg = std::string(kEmptyKeyMsg) + "\n";
        append_target_hints(msg, t.keys, "");   // keys is empty when hdc is missing
        return false;
    }
    // (2) explicit target: it must match a target hdc currently lists, so a key that
    //     names an unplugged / unauthorized device is caught here rather than later.
    if (key_provided) {
        auto t = hdc_list_targets();
        if (t.spawn_failed) { msg = kHdcMissingMsg; return false; }
        bool found = false;
        for (const auto& k : t.keys) { if (k == key) { found = true; break; } }
        if (!found) {
            // fail-open: an incomplete parse means the candidate list may be wrong, so
            // let hdc decide instead of rejecting a possibly valid target locally.
            if (t.unparsed_lines > 0) {
                hdc_set_connect_key(key);
                return true;
            }
            std::string text;
            if (key.size() <= kMaxProbeKeyLen) text = hdc_probe(key);
            if (!hdc_is_device_failure(text)) text = kFallbackNotMatch;
            msg = decorate_device_error(text, t.keys, key);
            return false;
        }
        hdc_set_connect_key(key);   // matched a listed target
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
    auto t = hdc_list_targets();
    if (!t.spawn_failed && t.keys.size() >= 2) {
        std::string text = hdc_probe("");
        if (!hdc_is_device_failure(text)) text = kFallbackNeedKey;
        msg = decorate_device_error(text, t.keys, "");
        return false;
    }
    // One candidate, but the listing is not trustworthy (an unrecognized entry, or hdc
    // reported a failure while still listing a device): a second device could be hiding
    // in it. Both "pin the only id" and "run untargeted" would be guesses, and the latter
    // hands the device choice back to hdc, so ask the user to name the device instead.
    // Note this must come *after* the >=2 verdict above, which stays the more specific
    // message when the count already proves multiple devices.
    if (!t.spawn_failed && t.keys.size() == 1 && (t.unparsed_lines > 0 || t.saw_fail)) {
        msg = std::string(kAmbiguousListMsg) + "\n";
        append_unparsed_lines(msg, t);
        append_target_hints(msg, t.keys, "");   // our wording: skip decorate_device_error()
        return false;
    }
    // Exactly one usable target: pin its id. Every business command then carries
    // `-t <id>`, which buys two things: unplugging this device mid-capture makes hdc
    // answer "[Fail]Not match" instead of silently routing the next command to a
    // newly attached device, and the pinned key is what arms the early device-loss
    // detection (note_device_failure() ignores "[Fail]" while no key is pinned).
    // (hdc's own chatter is skipped by the parser, so it does not block the pin.)
    if (!t.spawn_failed && !t.saw_fail && t.unparsed_lines == 0 && t.keys.size() == 1) {
        hdc_set_connect_key(t.keys[0]);
        return true;
    }
    // 0 devices, a failed spawn or "[Fail]": stay untargeted, i.e. keep the pre-change
    // command form (no -t) and let the existing paths report the problem. When the output
    // held entries we could not classify we are not actually sure the device count is 0,
    // so say so instead of silently emitting untargeted commands (this must not fire on a
    // clean 0-device listing, whose output has to stay byte-identical to the baseline).
    if (!t.spawn_failed && t.keys.empty() && t.unparsed_lines > 0) {
        std::string warn = "Warning: 'hdc list targets' returned output this tool cannot recognize; continuing"
            " WITHOUT a pinned target (no -t), so hdc picks the device.\n";
        append_unparsed_lines(warn, t);
        log::logw(warn);
    }
    hdc_mark_target_resolved();   // must mark here too, or the builders report a missed gate
    return true;
}

// Check that the device's API version meets the minimum required.
bool check_api_version(int min_api_version) {
    if (min_api_version <= 0) return true;

    auto cap = proc::capture_command(hdc_shell("param get const.ohos.apiversion") + " 2>&1");
    if (!cap.spawned) {
        log::loge("API version check failed: unable to launch hdc shell.");
        return false;
    }
    // A pinned target surfaces the hdc device-level failure verbatim, instead of
    // misreporting it as "could not parse version". Untargeted runs (no key pinned:
    // 0 devices, noisy parse, hdc failure) keep the original branch and wording.
    if (!hdc_connect_key().empty() && hdc_is_device_failure(cap.output)) {
        log::loge(hdc_failure_lines(cap.output));
        return false;
    }

    std::string trimmed = cap.output;
    const auto first = trimmed.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        log::loge("API version check failed: empty response from device.");
        return false;
    }
    std::string num_str;
    for (size_t i = first; i < trimmed.size(); ++i) {
        if (trimmed[i] >= '0' && trimmed[i] <= '9') {
            num_str += trimmed[i];
        } else {
            break;
        }
    }
    if (num_str.empty()) {
        log::loge("API version check failed: could not parse version.");
        return false;
    }

    int version = 0;
    try {
        version = std::stoi(num_str);
    } catch (...) {
        log::loge("API version check failed: invalid version number.");
        return false;
    }

    if (version < min_api_version) {
        log::loge("API version check failed: device API version is ", version,
                  ", but this capture type requires >= ", min_api_version, ".");
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
    bool require_app;            // target must be an installed app (ArkTS types)
    bool wait_for_zip;           // result arrives as a hiprofiler_data_*.zip
    const char* snapshot_prefix; // local snapshot file prefix: "smaps" or "fds"
    const char* report_subcmd;   // analysis subcommand used by --report
    const char* report_extra;    // extra args for the report subcommand
};

const CaptureTypeInfo kCaptureTypes[] = {
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
constexpr size_t kCaptureTypeCount = sizeof(kCaptureTypes) / sizeof(kCaptureTypes[0]);

// Resolve a user-facing category string to a CaptureTypeInfo entry.
// Supports full names and short aliases:
//   nativeheap → nh, native
//   snapshot   → snap, sn
//   globalhandle → global, gh
//   allhandle    → handle, ah
//   localhandle  → local, lh
//   fd, dma, thread, ashmem → (no alias, already short)
// Returns nullptr and sets err_msg on unrecognized input.
const CaptureTypeInfo* resolve_category(const std::string& cat, std::string& err_msg) {
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
        err_msg = "invalid category '" + cat + "'. Valid: nativeheap/nh, nativeleaks/nl, snapshot/snap, global/gh, local/lh, fd, dma, thread, ashmem";
        return nullptr;
    }
    for (size_t i = 0; i < kCaptureTypeCount; ++i) {
        if (kCaptureTypes[i].flag == resolved) return &kCaptureTypes[i];
    }
    err_msg = "internal error: category '" + cat + "' resolved to unknown flag '" + resolved + "'";
    return nullptr;
}

// Query the device for installed apps via `bm dump -a -l`, parse the output
// into {package_name, app_name} pairs. Supports multiple output formats:
//   JSON:    [{"bundleName":"pkg","appName":"App"},...]
//   key-val: bundleName: pkg \n  name: App
//   simple:  pkg:App
struct AppEntry {
    std::string package;
    std::string app_name;
};

// Result of an app query: carries the "hdc device-level failure" flag so callers can tell
// "there really is no app" apart from "device communication failed" (the latter must NOT
// fall back to the system-process path).
struct AppQueryResult {
    std::vector<AppEntry> apps;
    bool device_failed = false;   // set only when a target is pinned (see hdc_connect_key())
    std::string device_err;
};

static AppQueryResult query_device_apps() {
    AppQueryResult out;
    std::vector<AppEntry>& apps = out.apps;
    auto cap = proc::capture_command(hdc_shell("bm dump -a -l") + " 2>&1");
    // A pinned target surfaces the hdc device-level failure verbatim, instead of
    // misreading it as "no apps / system process". Untargeted runs are untouched.
    if (!hdc_connect_key().empty() && hdc_is_device_failure(cap.output)) {
        out.device_failed = true;
        out.device_err = hdc_failure_lines(cap.output);
        return out;
    }
    if (!cap.spawned || cap.exit_code != 0) return out;

    const std::string& output = cap.output;

    // Try JSON format first: [{...}, ...]
    auto json_start = output.find('[');
    auto json_end = output.rfind(']');
    if (json_start != std::string::npos && json_end != std::string::npos &&
        json_end > json_start) {
        std::string json_str = output.substr(json_start, json_end - json_start + 1);
        // Simple JSON parser for the expected structure without nlohmann dep.
        size_t pos = 0;
        while ((pos = json_str.find("\"bundleName\"", pos)) != std::string::npos) {
            AppEntry entry;
            // Extract bundleName value
            auto colon = json_str.find(':', pos);
            if (colon == std::string::npos) break;
            auto val_start = json_str.find('"', colon + 1);
            if (val_start == std::string::npos) break;
            auto val_end = json_str.find('"', val_start + 1);
            if (val_end == std::string::npos) break;
            entry.package = json_str.substr(val_start + 1, val_end - val_start - 1);

            // Extract app name (may be "label", "appName", or "name")
            auto extract_label = [&](const std::string& key) -> std::string {
                auto key_pos = json_str.find("\"" + key + "\"", val_end);
                if (key_pos == std::string::npos) return "";
                auto colon = json_str.find(':', key_pos);
                if (colon == std::string::npos) return "";
                auto val = json_str.find('"', colon + 1);
                if (val == std::string::npos) return "";
                auto end = json_str.find('"', val + 1);
                if (end == std::string::npos) return "";
                return json_str.substr(val + 1, end - val - 1);
            };
            entry.app_name = extract_label("label");
            if (entry.app_name.empty()) entry.app_name = extract_label("appName");
            if (entry.app_name.empty()) entry.app_name = extract_label("name");
            if (!entry.package.empty()) {
                apps.push_back(std::move(entry));
            }
            pos = val_end + 1;
        }
        if (!apps.empty()) return out;
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
                    current.app_name = line.substr(colon2 + 1);
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
        while (!key.empty() && key.back() == ' ') key.pop_back();

        auto val_start = line.find_first_not_of(": =", sep);
        std::string val = (val_start != std::string::npos) ? line.substr(val_start) : "";

        if (key == "bundleName") {
            if (!current.package.empty()) {
                apps.push_back(std::move(current));
            }
            current.package = val;
        } else if (key == "name" || key == "appName") {
            current.app_name = val;
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
static bool resolve_process_name(const std::string& input, bool require_app,
                                 std::string& resolved_pkg, bool& is_app) {
    // Any target resolved from the installed-app list is an app; the verbatim
    // system-process fallbacks below override this to false.
    is_app = true;
    auto q = query_device_apps();
    // A pinned target reports the device-level failure here, instead of misreading it as
    // a system process and carrying on with a wrong premise.
    if (q.device_failed) {
        log::loge(q.device_err);
        return false;
    }
    const auto& apps = q.apps;
    if (apps.empty()) {
        if (!require_app) {
            resolved_pkg = input;
            is_app = false;
            log::logi("Resolved: ", input, " -> package: ", resolved_pkg,
                      " (unable to query apps; treated as a system process)");
            return true;
        }
        log::loge("Error: unable to query device apps via 'bm dump -a -l'.\n"
                  "Please ensure a device is connected and try again.");
        return false;
    }

    // Exact match first (case-insensitive on package name)
    for (const auto& a : apps) {
        if (contains_ci(a.package, input) && a.package.size() == input.size()) {
            // If multiple exact matches on different fields, still prefer package exact
            resolved_pkg = a.package;
            log::logi("Resolved: ", input, " -> package: ", resolved_pkg,
                      (!a.app_name.empty() ? (" (" + a.app_name + ")") : std::string()));
            return true;
        }
    }
    for (const auto& a : apps) {
        if (contains_ci(a.app_name, input) && a.app_name.size() == input.size()) {
            resolved_pkg = a.package;
            log::logi("Resolved: ", input, " -> package: ", resolved_pkg,
                      (!a.app_name.empty() ? (" (" + a.app_name + ")") : std::string()));
            return true;
        }
    }

    // Fuzzy match: substring in app name (label) only.
    // Package names require exact match to avoid false positives.
    std::vector<const AppEntry*> matches;
    for (const auto& a : apps) {
        if (!a.app_name.empty() && contains_ci(a.app_name, input)) {
            matches.push_back(&a);
        }
    }

    if (matches.empty()) {
        if (!require_app) {
            // Native capture types can also analyze system processes that are
            // not installed apps (e.g. system services / daemons).
            resolved_pkg = input;
            is_app = false;
            log::logi("Resolved: ", input, " -> package: ", resolved_pkg,
                      " (not an installed app; treated as a system process)");
            return true;
        }
        log::loge("Error: no app matches '", input, "'.\n"
                  "Use 'mem-analyzer prof --list-apps' to see available apps,\n"
                  "or provide an exact package name or a partial app name.");
        return false;
    }

    if (matches.size() == 1) {
        resolved_pkg = matches[0]->package;
        log::logi("Resolved: ", input, " -> package: ", resolved_pkg,
                  (!matches[0]->app_name.empty() ? (" (" + matches[0]->app_name + ")") : std::string()));
        return true;
    }

    // Multiple matches: list them
    std::string msg = "Error: '" + input + "' matches " + std::to_string(matches.size()) + " apps:\n";
    for (const auto* m : matches) {
        msg += "  " + m->package;
        if (!m->app_name.empty()) msg += " (" + m->app_name + ")";
        msg += "\n";
    }
    msg += "Please provide a more specific name.";
    log::loge(msg);
    return false;
}

// Make `name` usable as a single path component: for system processes the
// resolved target is the user's verbatim input (see resolve_process_name), and
// platform::join_path does not normalize the path, so a separator would
// silently add directory levels (or `..` escape the output dir). Characters
// that are illegal in a path component on Windows are replaced as well and a
// trailing space/dot is dropped, so a session directory is valid on every
// platform the tool ships for.
static std::string as_path_segment(const std::string& name) {
    static const std::string kIllegal = "/\\:*?\"<>|";
    std::string out;
    out.reserve(name.size());
    for (const char c : name) {
        out += (kIllegal.find(c) != std::string::npos) ? '_' : c;
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '.')) {
        out.pop_back();
    }
    return out;
}

// Resolve the mem-analyzer binary path: prefer the directory of the
// current executable, fall back to the bare name on PATH.
std::string resolve_mem_analyzer_binary() {
#if defined(_WIN32)
    constexpr const char* kBinaryName = "mem-analyzer.exe";
#else
    constexpr const char* kBinaryName = "mem-analyzer";
#endif
    const std::string exe_dir = proc::get_executable_dir();
    return exe_dir.empty() ? kBinaryName
                           : platform::join_path(exe_dir, kBinaryName);
}

// Undo the nesting the analysis subcommands apply to their output: move
// everything a report run produced from <dir>/<stem>/ up into <dir> and drop
// the directory once nothing is left in it. Used when a report is generated
// into a session directory, where that extra level is redundant. `stem` is the
// input file stem.
void flatten_report_dir(const std::string& dir, const std::string& stem) {
    const std::string nested_dir = platform::join_path(dir, stem);
    std::error_code ec;
    if (!fs::is_directory(path_from_utf8(nested_dir), ec) || ec) return;

    for (const auto& entry : fs::directory_iterator(path_from_utf8(nested_dir), ec)) {
        if (ec) break;
        std::error_code type_ec;
        if (!entry.is_regular_file(type_ec) || type_ec) continue;
        const std::string dest =
            platform::join_path(dir, path_to_utf8(entry.path().filename()));
        std::error_code move_ec;
        fs::rename(entry.path(), path_from_utf8(dest), move_ec);
        if (move_ec) {
            log::loge("Report: unable to move ", path_to_utf8(entry.path()),
                      " into ", dir, " (", move_ec.message(), ")");
        }
    }

    // Fails while a subdirectory (or an unmovable file) is still there.
    std::error_code rm_ec;
    fs::remove(path_from_utf8(nested_dir), rm_ec);
    if (rm_ec) {
        log::logw("Warning: ", nested_dir, " was kept (", rm_ec.message(), ")");
    }
}

} // namespace

int run_profiler_cmd(const ProfilerCliOptions& opt) {
    // 1. Determine config path and load config.
    std::string config_path = resolve_config_path();
    CmdConfig cfg;
    std::string err;
    if (!load_cmd_config(config_path, cfg, err)) {
        log::loge("Failed to load config: ", err);
        return 2;
    }

    // 2. The options were already parsed by the main CliParser (see main.cpp),
    // so --help/validation are handled by the shared CLI architecture.

    // 3. Local copies of the parsed options.
    std::string process_name = opt.process_name;
    int duration_val = opt.duration;
    std::string output_dir_val = opt.output_dir;
    std::string category = opt.category;
    bool flag_report = opt.flag_report, flag_list_apps = opt.flag_list_apps;

    // 3.4 Target device gate: decides the target before ANY hdc command (including
    //     --list-apps) — see resolve_device_target(). It prints nothing on the
    //     untargeted paths, which keeps 0-device output byte-identical.
    {
        std::string gate_msg;
        if (!resolve_device_target(opt.connect_key, opt.connect_key_provided, gate_msg)) {
            log::loge(gate_msg);
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
    if (!hdc_connect_key().empty()) {
        log::logi("Device: ", hdc_connect_key());
    }

    // 3.5 Handle --list-apps early (before capture type validation).
    if (flag_list_apps) {
        auto q = query_device_apps();
        if (q.device_failed) { log::loge(q.device_err); return 2; }
        const auto& apps = q.apps;
        if (apps.empty()) {
            log::loge("No apps found or unable to query device.");
            return 2;
        }
        std::cout << "Installed apps (" << apps.size() << "):\n";
        for (const auto& a : apps) {
            std::cout << "  " << a.package;
            if (!a.app_name.empty()) std::cout << "  (" << a.app_name << ")";
            std::cout << "\n";
        }
        return 0;
    }

    // 4. Resolve category string to capture type.
    std::string err_msg;
    const CaptureTypeInfo* capture = resolve_category(category, err_msg);
    if (!capture) {
        log::loge("Error:", err_msg);
        return 2;
    }
    if (process_name.empty()) {
        log::loge("Error: <bundle> is required for capture.");
        return 2;
    }
    const std::string config_option = capture->flag;

    // 4. Environment check.
    if (!check_profiler_env()) {
        return 2;
    }

    // 4.5 Resolve process name: support partial matching of package name or app name.
    //     ArkTS types (-s/-g/-l) require an installed app; native types
    //     (-n/-f/-d) may also target system processes not in the app list.
    bool is_app = true;  // whether the resolved target is an installed app
    {
        std::string resolved;
        const bool require_app = capture->require_app;
        if (!resolve_process_name(process_name, require_app, resolved, is_app)) {
            return 2;
        }
        process_name = resolved;
    }

    // 5. Resolve config option to its template file path.
    std::string config_file;
    std::string result_dir;
    bool require_fresh_start = false;
    int min_api_version = 0;
    for (const auto& op : cfg.option_path_map) {
        if (op.option == config_option) {
            config_file = op.path;
            result_dir = op.result_dir;
            require_fresh_start = op.require_fresh_start;
            min_api_version = op.min_api_version;
            break;
        }
    }
    if (config_file.empty()) {
        log::loge("Unknown config option: ", config_option);
        return 2;
    }

    // 6. API version check.
    if (!check_api_version(min_api_version)) {
        return 2;
    }

    // 7. Process template.
    ProfilerOptions popts;
    popts.p = process_name;
    if (duration_val > 0) {
        popts.t = duration_val;
    } else {
        // Default to 120s when -t is not given, instead of finishing immediately.
        popts.t = 120;
    }
    popts.o = output_dir_val;
    popts.config_option = config_option;

    CaptureCommand cc = process_template(config_file, popts);
    if (!cc.ok) {
        log::loge("Failed to process config file:", cc.error);
        return 2;
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
    const bool continuous_snapshot =
        (config_option == "snapshot") && opt.duration_specified;
    const std::string session_name =
        config_option + (continuous_snapshot ? "-session" : "") + "-" +
        as_path_segment(process_name) + "-" + current_timestamp();
    const std::string device_segment =
        (opt.connect_key_provided && !hdc_connect_key().empty())
            ? as_path_segment(hdc_connect_key())
            : std::string();
    const std::string session_path =
        device_segment.empty() ? session_name
                               : platform::join_path(device_segment, session_name);
    const std::string output_dir = platform::join_path(output_dir_val, session_path);
    // create_directories also reports false for an existing directory.
    if (!platform::create_directories(output_dir) && !platform::directory_exists(output_dir)) {
        log::loge("Error: cannot create the output directory: ", output_dir);
        return 2;
    }

    // 8. Run hdc. The target was already echoed right after the gate (step 3.45).
    HdcRunOptions run_opts;
    run_opts.content = cc.command;
    run_opts.duration = cc.duration;
    run_opts.output_dir = output_dir;
    run_opts.remote_output_path = cc.remote_output_path;
    run_opts.process_name = cc.process_name;
    run_opts.result_dir = result_dir;
    run_opts.require_fresh_start = require_fresh_start;
    run_opts.wait_for_zip = capture->wait_for_zip;
    run_opts.pull_nativeleaks = (config_option == "nativeleaks");
    run_opts.device_config_content = cc.device_config_content;
    run_opts.smaps_prefix = capture->snapshot_prefix;
    run_opts.is_app = is_app;
    run_opts.continuous_snapshot = continuous_snapshot;
    RunResult rr = run_hdc(run_opts);
    if (!rr.ok) {
        // Name the pinned device when there is one: rr.error is hdc text and does not always
        // mention the target, and by now the `Device:` echo is well above this line.
        std::string err = "hdc execution failed: " + rr.error;
        if (!hdc_connect_key().empty()) err += " [device: " + hdc_connect_key() + "]";
        log::loge(err);
        if (!rr.output.empty() && !rr.output_streamed) std::cout << rr.output;
        return 2;
    }

    // 9. Report results.
    // `output_streamed` means the transcript was already printed to the console
    // in real time (async `hiprofiler_cmd start`), so don't print it again.
    if (!rr.output.empty() && !rr.output_streamed) {
        std::cout << rr.output;
        if (rr.output.back() != '\n') std::cout << '\n';
    }
    log::logi("Elapsed: ", rr.elapsed, "s");

    if (rr.snapshot_attempted) {
        log::logi("Snapshot status: ", rr.snapshot_status);
        for (const auto& f : rr.snapshot_pulled_files) {
            log::logok("Snapshot file exported: ", f);
        }
    } else if (rr.pull_attempted) {
        if (rr.pull_ok) {
            log::logok("Result file exported: ", rr.pulled_file);
        } else {
            // `hdc file recv` exits 0 even when the remote file is missing,
            // so the captured output holds the real reason (e.g. "[Fail]Error
            // opening file: ..."); prefer it over the exit code.
            if (!rr.pull_output.empty()) {
                log::loge("Result file export failed:\n", rr.pull_output);
            } else {
                log::loge("Result file export failed: (exit code ", rr.pull_exit_code, ")");
            }
            return 3;
        }
    }

    // Summarize the per-process snapshot files exported during the capture
    // (hidumper --mem-smaps for -n/-g/-l/-d, lsof for -f). The paths were
    // already printed at the moment of export; this consolidates them.
    if (!rr.smaps_exported_files.empty()) {
        const char* label = (std::string(capture->snapshot_prefix) == "fds")
                                ? "FD snapshot exported"
                                : "Smaps exported";
        for (const auto& f : rr.smaps_exported_files) {
            log::logi(label, ": ", f);
        }
    }

    // 10. Generate HTML analysis report if --report is set.
    if (flag_report) {
        // Continuous snapshot sessions (snapshot -t) produce start + peak + end
        // snapshots. With -o, generate a single comparison report (start=baseline,
        // end=target) via jsleak -b, instead of per-file reports.
        if (continuous_snapshot && !rr.snapshot_pulled_files.empty()) {
            std::string baseline_file, target_file;
            for (const auto& f : rr.snapshot_pulled_files) {
                const std::string stem = platform::stem(f);
                if (stem.empty()) continue;
                const std::string start_sfx = "-start";
                const std::string end_sfx = "-end";
                if (baseline_file.empty() &&
                    stem.size() >= start_sfx.size() &&
                    stem.compare(stem.size() - start_sfx.size(),
                                 start_sfx.size(), start_sfx) == 0) {
                    baseline_file = f;
                } else if (target_file.empty() &&
                           stem.size() >= end_sfx.size() &&
                           stem.compare(stem.size() - end_sfx.size(),
                                        end_sfx.size(), end_sfx) == 0) {
                    target_file = f;
                }
            }

            // Fallback: if either was not located by role suffix, use file
            // modification timestamps — earliest = start (baseline),
            // latest = end (target).
            if (baseline_file.empty() || target_file.empty()) {
                log::logi("Role suffix not found on all snapshots; "
                          "falling back to file modification timestamps.");
                fs::file_time_type min_time = fs::file_time_type::max();
                fs::file_time_type max_time = fs::file_time_type::min();
                std::string earliest, latest;
                for (const auto& f : rr.snapshot_pulled_files) {
                    std::error_code ec;
                    const auto t = fs::last_write_time(path_from_utf8(f), ec);
                    if (ec) continue;
                    if (t < min_time) { min_time = t; earliest = f; }
                    if (t > max_time) { max_time = t; latest = f; }
                }
                if (baseline_file.empty()) baseline_file = earliest;
                if (target_file.empty()) target_file = latest;
            }

            if (baseline_file.empty() || target_file.empty() ||
                baseline_file == target_file) {
                log::loge("Comparison report: could not locate start/end snapshots ",
                          "(baseline=", baseline_file.empty() ? "<missing>" : baseline_file,
                          ", target=", target_file.empty() ? "<missing>" : target_file, ").");
                return rr.exit_code;
            }

            log::logi("Generating comparison report: baseline=start, target=end");

            const std::string binary = resolve_mem_analyzer_binary();

            // jsleak -b <baseline> <target> -d <output_dir>
            std::string cmd = proc::shell_quote(binary) +
                " jsleak -b " + proc::shell_quote(baseline_file) +
                " " + proc::shell_quote(target_file) +
                " -d " + proc::shell_quote(output_dir);
            // Forward ArkTS deobfuscation options (same as per-file path).
            if (!opt.sourcemap_dir.empty()) {
                cmd += " --sourcemap " + proc::shell_quote(opt.sourcemap_dir);
            }
            if (!opt.name_obfuscation.empty()) {
                cmd += " --namecache " + proc::shell_quote(opt.name_obfuscation);
            }

            log::logd("Command: ", cmd);
            int rc = run_command_streaming(cmd + " 2>&1");
            if (rc < 0) {
                log::loge("Comparison report generation failed: unable to launch ", binary);
            } else if (rc != 0) {
                log::loge("Comparison report generation failed: exited with code ", rc);
            } else {
                // The jsleak compare subcommand nests its output under
                // <output_dir>/<target-stem>/, writing
                // <baseline-stem>-vs-<target-stem>.html there. Lift it up.
                const std::string target_stem = platform::stem(target_file);
                const std::string baseline_stem = platform::stem(baseline_file);
                if (!target_stem.empty()) {
                    flatten_report_dir(output_dir, target_stem);
                }
                const std::string html_name =
                    (baseline_stem.empty() ? "baseline" : baseline_stem) + "-vs-" +
                    (target_stem.empty() ? "target" : target_stem) + ".html";
                const std::string html_path = platform::join_path(output_dir, html_name);
                log::logok("Comparison report generated: ", html_path);
                if (platform::file_exists(html_path)) {
                    platform::open_with_default_app(html_path);
                } else {
                    log::loge("Comparison report HTML not found at: ", html_path,
                              " (the capture may have contained no data).");
                }
            }
            return rr.exit_code;
        }

        // Collect the file(s) to analyze.
        std::vector<std::string> report_files;
        if (rr.snapshot_attempted) {
            report_files = rr.snapshot_pulled_files;
        } else if (rr.pull_attempted && rr.pull_ok && !rr.pulled_file.empty()) {
            report_files.push_back(rr.pulled_file);
        }

        if (report_files.empty()) {
            log::loge("Report: no captured file available to analyze.");
        } else {
            // Determine the subcommand and extra args from the capture type.
            const std::string subcmd = capture->report_subcmd;
            const std::string extra_args = capture->report_extra;

            const std::string binary = resolve_mem_analyzer_binary();

            for (const auto& f : report_files) {
                // Every capture file of a session lives directly in the session
                // directory, so analyzing it there keeps the report inside the
                // session tree. The analysis subcommand writes it one level
                // deeper, under <dir>/<stem>/, which is undone below.
                // nativeleaks: feed the orphan-snapshot file to `trace -l`
                // alongside the .htrace so the L1/L2/L3 leak filtering runs.
                std::string leaks_arg;
                if (config_option == "nativeleaks") {
                    if (!rr.pulled_leaks_file.empty()) {
                        leaks_arg = " -l " + proc::shell_quote(rr.pulled_leaks_file);
                    } else {
                        // leaks 文件未获取（超时/设备掉线）：明确告知用户本次
                        // 未做 -l 泄漏过滤，仅输出普通 nativeheap 报告，避免
                        // 静默降级让用户误以为泄漏过滤已运行。
                        log::logw("Warning: nativeleaks orphan-snapshot file was not captured; "
                                  "running trace WITHOUT -l leak filtering (plain nativeheap "
                                  "analysis only). Re-run the capture if leak filtering is needed.");
                    }
                }
                std::string cmd = proc::shell_quote(binary) +
                    " " + subcmd + extra_args + leaks_arg +
                    " " + proc::shell_quote(f) +
                    " -d " + proc::shell_quote(output_dir);
                // 转发反混淆选项给 jsleak 报告子命令，还原混淆的符号/路径后再分析
                if (!opt.sourcemap_dir.empty()) {
                    cmd += " --sourcemap " + proc::shell_quote(opt.sourcemap_dir);
                }
                if (!opt.name_obfuscation.empty()) {
                    cmd += " --namecache " + proc::shell_quote(opt.name_obfuscation);
                }
                // native .so 符号反解只对 trace/tshandle 分析有意义（jsleak 走 ArkTS 反混淆）
                if (!opt.so_dir.empty() && (subcmd == "trace" || subcmd == "tshandle")) {
                    cmd += " --so-dir " + proc::shell_quote(opt.so_dir);
                }

                log::logd("Generating report: ", cmd);
                int rc = run_command_streaming(cmd + " 2>&1");
                if (rc < 0) {
                    log::loge("Report generation failed: unable to launch ", binary);
                } else if (rc != 0) {
                    log::loge("Report generation failed: exited with code ", rc);
                } else {
                    // The analysis subcommand nests its output under
                    // <output_dir>/<source_name>/, where source_name is the input
                    // file stem. That level is redundant here: the capture file
                    // already lives in its own session directory, so lift
                    // everything the run produced next to it and report the
                    // final location (the subprocess printed the old one).
                    std::string base = platform::stem(f);
                    if (base.empty()) base = "output";
                    flatten_report_dir(output_dir, base);

                    const std::string html_path = platform::join_path(output_dir, base + ".html");
                    log::logok("Report generated: ", html_path);
                    if (platform::file_exists(html_path)) {
                        // Open the generated HTML in the default browser.
                        platform::open_with_default_app(html_path);
                    } else {
                        log::loge("Report HTML not found at: ", html_path,
                                  " (the capture may have contained no data).");
                    }
                }
            }
        }
    }

    return rr.exit_code;
}

} // namespace mem_analyzer
