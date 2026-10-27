#pragma once

#include <string>
#include <vector>

namespace mem_analyzer {

struct OptionPath {
    std::string option;
    std::string path;
    std::string resultDir;
    bool requireFreshStart = false;
    int minApiVersion = 0;
};

struct CmdConfig {
    std::vector<OptionPath> optionPathMap;

    // Config file directory (where profiler_cmd_config.json lives).
    // Populated by resolve_config_path(); used to resolve relative
    // template paths.
    std::string configDir;
};

// Resolve the path to profiler_cmd_config.json.
// Priority: MEM_ANALYZER_CONFIG env var > CWD-relative > exe-relative.
std::string ResolveConfigPath();

// Load and parse a CmdConfig from the JSON file at `config_path`.
// Populates config_dir so template paths can be resolved.
bool LoadCmdConfig(const std::string& ConfigPath, CmdConfig& cfg, std::string& error);

} // namespace mem_analyzer
