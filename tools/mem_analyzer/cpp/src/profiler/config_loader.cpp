#include "profiler/config_loader.h"
#include "profiler/process_utils.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <system_error>
#include <vector>

#include "utils/platform.hpp"
#include "utils/file_utils.hpp"

namespace mem_analyzer {

namespace fs = std::filesystem;

// IMPORTANT: nowhere in this file do we call fs::absolute() / fs::current_path()
// — both invoke getcwd(), which fails ("Operation not permitted") in some
// sandboxes and throws filesystem_error. All path resolution uses lexical
// operations (lexically_normal / parent_path) only; CWD-relative paths resolve
// against the process CWD at file-access time.

// Default config file path (relative to CWD).
std::string default_config_path() {
    return "config/profiler/profiler_cmd_config.json";
}

std::string resolve_config_path() {
    // 固定目录：可执行文件同级的 config/profiler/profiler_cmd_config.json
    const std::string rel = default_config_path();
    const std::string exe_dir = proc::get_executable_dir();
    if (exe_dir.empty()) return rel;
    return platform::join_path(exe_dir, rel);
}

// Load and parse profiler_cmd_config.json. Resolves option_path_map paths
// relative to the json file's directory (lexical normalization only, no
// fs::absolute() to avoid getcwd()). Returns false and sets `err` on any
// open / parse / structural error rather than throwing.
bool load_cmd_config(const std::string& config_path, CmdConfig& out, std::string& err) {
    // Use config_path as-is (lexical operations only; no fs::absolute — see
    // getcwd caveat). Relative paths resolve against CWD at file-access time.
    const fs::path config_fp = path_from_utf8(config_path);

    // Open the file. We use a non-throwing open so we can produce a friendly
    // error message instead of letting an exception escape.
    std::ifstream stream(config_fp);
    if (!stream.is_open()) {
        err = "failed to open config file: " + path_to_utf8(config_fp);
        return false;
    }

    // Parse the JSON. parse_error is the only json exception type we expect
    // here; any other exception is left to propagate (should not happen for
    // well-formed input read from a stream).
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::parse_error& e) {
        err = "config file JSON parse failed: " + std::string(e.what());
        return false;
    }

    // `config_dir`: lexically-normalized parent of the json file (may be
    // relative — no fs::absolute, see getcwd caveat).
    out.config_dir = path_to_utf8(config_fp.parent_path().lexically_normal());

    // `option_path_map`: array of { "option": "-n", "path": "xxx.txt" }.
    // The `path` is interpreted relative to config_dir and lexically
    // normalized. Missing key => empty list (not an error).
    out.option_path_map.clear();
    if (j.contains("option_path_map")) {
        if (!j["option_path_map"].is_array()) {
            err = "the 'option_path_map' field in the config file is not an array";
            return false;
        }
        for (const auto& elem : j["option_path_map"]) {
            if (!elem.is_object()) {
                err = "an element of 'option_path_map' in the config file is not an object";
                return false;
            }
            OptionPath op;
            op.option = elem.value("option", std::string{});
            const std::string rel_path = elem.value("path", std::string{});
            // Resolve relative to config_dir; lexically_normal collapses any
            // redundant components (e.g. "." / "..").
            op.path = path_to_utf8(
                (path_from_utf8(out.config_dir) / path_from_utf8(rel_path)).lexically_normal());
            // Optional device-side async result directory (snapshot flow). Read
            // verbatim — it is a device path, not resolved against config_dir.
            op.result_dir = elem.value("result_dir", std::string{});
            // Optional flag: requires app to be in fresh start state (e.g., -l).
            op.require_fresh_start = elem.value("require_fresh_start", false);
            // Optional minimum device API version (0 = no check).
            op.min_api_version = elem.value("min_api_version", 0);
            out.option_path_map.push_back(std::move(op));
        }
    }

    return true;
}

} // namespace mem_analyzer
