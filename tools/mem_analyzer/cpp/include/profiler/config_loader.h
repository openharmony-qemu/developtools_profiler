#pragma once

#include <string>
#include <vector>

namespace mem_analyzer {

struct OptionPath {
    std::string option;
    std::string path;
    std::string result_dir;
    bool require_fresh_start = false;
    int min_api_version = 0;
};

struct CmdConfig {
    std::vector<OptionPath> option_path_map;

    // Config file directory (where profiler_cmd_config.json lives).
    // Populated by resolve_config_path(); used to resolve relative
    // template paths.
    std::string config_dir;
};

// Resolve the path to profiler_cmd_config.json.
// Priority: MEM_ANALYZER_CONFIG env var > CWD-relative > exe-relative.
std::string resolve_config_path();

// Load and parse a CmdConfig from the JSON file at `config_path`.
// Populates config_dir so template paths can be resolved.
bool load_cmd_config(const std::string& config_path, CmdConfig& cfg, std::string& error);

} // namespace mem_analyzer
