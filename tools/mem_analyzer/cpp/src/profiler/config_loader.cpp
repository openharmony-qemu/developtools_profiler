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
std::string DefaultConfigPath()
{
    return "config/profiler/profiler_cmd_config.json";
}

std::string ResolveConfigPath()
{
    // 固定目录：可执行文件同级的 config/profiler/profiler_cmd_config.json
    const std::string rel = DefaultConfigPath();
    const std::string exeDir = proc::GetExecutableDir();
    if (exeDir.empty()) {
        return rel;
    }
    return platform::JoinPath(exeDir, rel);
}

// Load and parse profiler_cmd_config.json. Resolves option_path_map paths
// relative to the json file's directory (lexical normalization only, no
// fs::absolute() to avoid getcwd()). Returns false and sets `err` on any
// open / parse / structural error rather than throwing.
bool LoadCmdConfig(const std::string& configPath, CmdConfig& out, std::string& err)
{
    // Use config_path as-is (lexical operations only; no fs::absolute — see
    // getcwd caveat). Relative paths resolve against CWD at file-access time.
    const fs::path configFp = PathFromUtf8(configPath);

    // Open the file. We use a non-throwing open so we can produce a friendly
    // error message instead of letting an exception escape.
    std::ifstream stream(configFp);
    if (!stream.is_open()) {
        err = "failed to open config file: " + PathToUtf8(configFp);
        return false;
    }

    // Parse the JSON. parse_error is the only json exception type we expect
    // here; any other exception is left to propagate (should not happen for
    // well-formed input read from a stream).
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::parse_error& e)
    {
        err = "config file JSON parse failed: " + std::string(e.what());
        return false;
    }

    // `config_dir`: lexically-normalized parent of the json file (may be
    // relative — no fs::absolute, see getcwd caveat).
    out.configDir = PathToUtf8(configFp.parent_path().lexically_normal());

    // `option_path_map`: array of { "option": "-n", "path": "xxx.txt" }.
    // The `path` is interpreted relative to config_dir and lexically
    // normalized. Missing key => empty list (not an error).
    out.optionPathMap.clear();
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
            const std::string relPath = elem.value("path", std::string{});
            // Resolve relative to config_dir; lexically_normal collapses any
            // redundant components (e.g. "." / "..").
            op.path = PathToUtf8(
                (PathFromUtf8(out.configDir) / PathFromUtf8(relPath)).lexically_normal());
            // Optional device-side async result directory (snapshot flow). Read
            // verbatim — it is a device path, not resolved against config_dir.
            op.resultDir = elem.value("result_dir", std::string{});
            // Optional flag: requires app to be in fresh start state (e.g., -l).
            op.requireFreshStart = elem.value("require_fresh_start", false);
            // Optional minimum device API version (0 = no check).
            op.minApiVersion = elem.value("min_api_version", 0);
            out.optionPathMap.push_back(std::move(op));
        }
    }

    return true;
}

} // namespace mem_analyzer
