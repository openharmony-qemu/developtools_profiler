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
