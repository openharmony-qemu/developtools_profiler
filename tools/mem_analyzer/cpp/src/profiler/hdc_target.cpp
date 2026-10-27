#include "profiler/hdc_target.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "profiler/process_utils.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {
namespace {

std::string g_connect_key;        // device id (serial number); empty => no -t is added
bool g_target_resolved = false;   // whether the gate has already resolved the target

// Contract check for the three command builders: the gate must have resolved the
// target before any business command is built. A missed gate is the dangerous case
// here, because an untargeted command silently reaches whichever device hdc picks.
// The check is compiled in unconditionally (there is no NDEBUG variant), and it does
// not abort: it reports the violation once and falls back to the untargeted command,
// so a programming mistake cannot cut short a user's running capture.
void check_target_resolved(const char* fn) {
    if (g_target_resolved) return;
    static bool warned = false;
    if (!warned) {
        warned = true;
        log::loge("[internal] ", fn, "() called before target resolution; continuing without -t");
    }
    // Mark resolved so later builders skip this check.
    g_target_resolved = true;
}

constexpr size_t kMaxKeyLen = 64;  // max length of a usable device id

// Usable device id shape: a device serial number is alnum; '.' ':' '_' '-' are tolerated
// as well, so an unusual id is still handed to hdc (which owns the final verdict) instead
// of being rejected here.
bool looks_like_key(const std::string& tok) {
    if (tok.empty() || tok.size() > kMaxKeyLen) return false;
    for (unsigned char c : tok) {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == ':' || c == '-')) return false;
    }
    return true;
}

// hdc also writes its own chatter to the stream we capture with `2>&1`:
//   "* daemon started successfully *"                     (server-start banner)
//   "[F][2026-09-18 18:10:10] Send isDead channelId:..."   (hdc's dated log line)
// Neither can be a device id (ids start with an alphanumeric character), so they are
// skipped instead of being counted as unparsable output. This matters: an unparsable
// line makes the gate give up on the multi-device verdict, and here it would also
// disable the single-device pin — i.e. hdc's own noise would silently switch off the
// protection against talking to the wrong device.
bool is_hdc_chatter(const std::string& s) {
    return s[0] == '*' || s[0] == '[';   // "[Empty]"/"[Fail]" are handled by the caller
}

std::string trimmed(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return {};
    const auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

} // namespace

// ---- Global target state ----

void hdc_mark_target_resolved() { g_target_resolved = true; }

void hdc_set_connect_key(const std::string& key) {
    g_connect_key = key;
    g_target_resolved = true;
}

const std::string& hdc_connect_key() { return g_connect_key; }

// ---- Parsing / classification ----

HdcTargets hdc_list_targets() {
    HdcTargets t;
    // hdc must exist before its output can be read as a device listing. When it does not,
    // the host shell answers "… is not recognized as an internal or external command,
    // operable program or batch file.", and that text is NOT a listing: its continuation
    // line even satisfies the device-id rule ("operable"), which would turn a missing hdc
    // into a bogus one-device verdict. Detect it up front and report a spawn failure, so
    // the gate keeps its "hdc missing" behaviour instead of reasoning about that text.
    // (Localized shells answer in their own language, so this must not be text-matched.)
    {
        auto probe = proc::find_in_path("hdc");
        if (!probe.spawned || probe.exit_code != 0 || probe.output.empty()) {
            t.spawn_failed = true;
            return t;
        }
    }
    // Must use `hdc list targets` WITHOUT -v:
    //   -v additionally prints persistent UART ports (COM1/COM2/COM3, always "Ready"),
    //   whose shape passes this module's device-id rule, so they would be mistaken for
    //   devices and force a bogus "multiple devices" verdict, breaking the 0/1-device
    //   compatibility contract.
    // Listing targets must be global: the literal below never carries -t.
    auto cap = proc::capture_command("hdc list targets 2>&1");
    if (!cap.spawned) {
        t.spawn_failed = true;
        return t;
    }
    t.raw_output = cap.output;

    std::istringstream iss(cap.output);
    std::string line;
    while (std::getline(iss, line)) {
        const std::string s = trimmed(line);
        if (s.empty()) continue;
        if (s.rfind("[Empty]", 0) == 0) continue;
        if (s.rfind("[Fail]", 0) == 0) {
            t.saw_fail = true;
            continue;
        }
        if (is_hdc_chatter(s)) continue;   // hdc's own banner / log lines, never a device
        // Observed format: the whole line is the device id. Taking the first token also
        // tolerates a possible two-column "id state" layout.
        const std::string tok = s.substr(0, s.find_first_of(" \t"));
        if (looks_like_key(tok)) {
            // hdc has been seen listing the same id twice. A duplicate is not a second
            // device, and counting it as one would reject a single-device run and then
            // print the very same id twice as "Available targets (2)".
            if (std::find(t.keys.begin(), t.keys.end(), tok) == t.keys.end()) {
                t.keys.push_back(tok);
            }
        } else {
            ++t.unparsed_lines;   // unknown shape: the count is no longer certain (see §8)
        }
    }
    return t;
}

bool hdc_is_device_failure(const std::string& output) {
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (trimmed(line).rfind("[Fail]", 0) == 0) return true;
    }
    return false;
}

std::string hdc_failure_lines(const std::string& output) {
    std::string out;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (trimmed(line).rfind("[Fail]", 0) != 0) continue;  // drop hdc internal log lines
        out += line;
        out += '\n';
    }
    return out;
}

std::string hdc_unparsed_lines(const std::string& output, size_t max_lines) {
    std::string out;
    size_t kept = 0;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        const std::string s = trimmed(line);
        if (s.empty()) continue;
        if (s.rfind("[Empty]", 0) == 0) continue;   // same classification as hdc_list_targets()
        if (s.rfind("[Fail]", 0) == 0) continue;
        if (is_hdc_chatter(s)) continue;
        if (looks_like_key(s.substr(0, s.find_first_of(" \t")))) continue;
        if (kept == max_lines) {   // cap: keep the message readable on a large listing
            out += "...\n";
            break;
        }
        out += s;
        out += '\n';
        ++kept;
    }
    if (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

// ---- Command construction ----

std::string hdc_raw_with_target(const std::string& target, const std::string& subcmd_args) {
    std::string cmd = "hdc";
    if (!target.empty()) {
        // A usable id (coming from `hdc list targets` candidates) is emitted bare: this
        // matches hdc's own usage and avoids per-platform quoting differences (on Windows
        // cmd.exe a single quote is not a quoting character).
        // A non-conforming value can only come from the gate probe, and is quoted there
        // to prevent shell injection.
        cmd += " -t " + (looks_like_key(target) ? target : proc::shell_quote(target));
    }
    cmd += " " + subcmd_args;
    log::logd("[hdc-trace] ", cmd);
    return cmd;
}

std::string hdc_shell(const std::string& device_cmd) {
    check_target_resolved("hdc_shell");
    // With no target this is byte-identical to the original form:
    // "hdc shell " + shell_quote(device_cmd)
    return hdc_raw_with_target(g_connect_key, "shell " + proc::shell_quote(device_cmd));
}

std::string hdc_file_send(const std::string& local, const std::string& remote) {
    check_target_resolved("hdc_file_send");
    return hdc_raw_with_target(
        g_connect_key,
        "file send " + proc::shell_quote(local) + " " + proc::shell_quote(remote));
}

std::string hdc_file_recv(const std::string& remote, const std::string& local) {
    check_target_resolved("hdc_file_recv");
    return hdc_raw_with_target(
        g_connect_key,
        "file recv " + proc::shell_quote(remote) + " " + proc::shell_quote(local));
}

} // namespace mem_analyzer
