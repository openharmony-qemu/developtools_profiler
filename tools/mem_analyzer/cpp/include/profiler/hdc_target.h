#pragma once
#include <string>
#include <vector>

namespace mem_analyzer {

// Parsed result of `hdc list targets`.
struct HdcTargets {
    std::vector<std::string> keys;   // usable device ids (device serial numbers), \r\n trimmed
    size_t unparsedLines = 0;       // not [Empty]/[Fail], not id-like, and not hdc's own
                                     // chatter: an unknown shape. hdc chatter is skipped
                                     // (not counted) so it cannot disable the single-device
                                     // pin; any other unrecognized entry makes the count
                                     // uncertain, which the gate refuses to guess about
    bool sawFail = false;           // output contains a line starting with "[Fail]"
    bool spawnFailed = false;       // hdc could not be launched (not installed / not on
                                     // PATH) — in that case no listing was attempted and
                                     // `keys`/`unparsed_lines` are left empty on purpose
    std::string rawOutput;          // raw hdc output
};

// ---- Global target state (written by the gate, read by business commands) ----
// The gate must resolve the target before any business command is built:
//   - target pinned (explicit --device, or the single attached device) -> commands carry -t
//   - otherwise (0 devices / no unambiguous device) -> untargeted command (no -t)
// The gate must call one of these on EVERY path it returns true from; a missed call
// makes the builders report the contract violation instead of failing silently.
void HdcMarkTargetResolved();
void HdcSetConnectKey(const std::string& key);
const std::string& HdcConnectKey();

// ---- Parsing / classification ----
// Uses a bare `hdc list targets` (never -v, never -t).
HdcTargets HdcListTargets();
// True when the output contains a line starting with "[Fail]".
bool HdcIsDeviceFailure(const std::string& output);
// Keeps only the "[Fail]" lines of `output` (drops hdc internal log noise).
std::string HdcFailureLines(const std::string& output);
// Keeps only the lines the parser could NOT classify (not [Empty]/[Fail], not hdc's own
// chatter, not an id-like token), joined by '\n' and capped at `max_lines`. This is what
// makes a rejected/ambiguous verdict diagnosable: the user sees the offending text, and a
// new hdc output shape shows up in the message instead of staying invisible.
std::string HdcUnparsedLines(const std::string& output, size_t maxLines = 3);

// ---- Command construction ----
// Explicit target, never reads/writes the global state; used by the gate probe.
std::string HdcRawWithTarget(const std::string& target, const std::string& subcmdArgs);
std::string HdcShell(const std::string& deviceCmd);
std::string HdcFileSend(const std::string& local, const std::string& remote);
std::string HdcFileRecv(const std::string& remote, const std::string& local);

} // namespace mem_analyzer
