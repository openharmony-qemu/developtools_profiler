#pragma once
#include <string>
#include <vector>

namespace mem_analyzer {

// Parsed result of `hdc list targets`.
struct HdcTargets {
    std::vector<std::string> keys;   // usable device ids (device serial numbers), \r\n trimmed
    size_t unparsed_lines = 0;       // not [Empty]/[Fail], not id-like, and not hdc's own
                                     // chatter: an unknown shape. hdc chatter is skipped
                                     // (not counted) so it cannot disable the single-device
                                     // pin; any other unrecognized entry makes the count
                                     // uncertain, which the gate refuses to guess about
    bool saw_fail = false;           // output contains a line starting with "[Fail]"
    bool spawn_failed = false;       // hdc could not be launched (not installed / not on
                                     // PATH) — in that case no listing was attempted and
                                     // `keys`/`unparsed_lines` are left empty on purpose
    std::string raw_output;          // raw hdc output
};

// ---- Global target state (written by the gate, read by business commands) ----
// The gate must resolve the target before any business command is built:
//   - target pinned (explicit --device, or the single attached device) -> commands carry -t
//   - otherwise (0 devices / no unambiguous device) -> untargeted command (no -t)
// The gate must call one of these on EVERY path it returns true from; a missed call
// makes the builders report the contract violation instead of failing silently.
void hdc_mark_target_resolved();
void hdc_set_connect_key(const std::string& key);
const std::string& hdc_connect_key();

// ---- Parsing / classification ----
// Uses a bare `hdc list targets` (never -v, never -t).
HdcTargets hdc_list_targets();
// True when the output contains a line starting with "[Fail]".
bool hdc_is_device_failure(const std::string& output);
// Keeps only the "[Fail]" lines of `output` (drops hdc internal log noise).
std::string hdc_failure_lines(const std::string& output);
// Keeps only the lines the parser could NOT classify (not [Empty]/[Fail], not hdc's own
// chatter, not an id-like token), joined by '\n' and capped at `max_lines`. This is what
// makes a rejected/ambiguous verdict diagnosable: the user sees the offending text, and a
// new hdc output shape shows up in the message instead of staying invisible.
std::string hdc_unparsed_lines(const std::string& output, size_t max_lines = 3);

// ---- Command construction ----
// Explicit target, never reads/writes the global state; used by the gate probe.
std::string hdc_raw_with_target(const std::string& target, const std::string& subcmd_args);
std::string hdc_shell(const std::string& device_cmd);
std::string hdc_file_send(const std::string& local, const std::string& remote);
std::string hdc_file_recv(const std::string& remote, const std::string& local);

} // namespace mem_analyzer
