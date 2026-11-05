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
#include <optional>

namespace mem_analyzer {

// User-supplied options for the profiler subcommand.
struct ProfilerOptions {
    std::optional<std::string> p;  // process name
    std::optional<long long>    t;  // duration in seconds
    std::optional<std::string> o;  // output directory
    std::string configOption;     // e.g. "-g", "-l", "-s", "-n"
};

// The device-side capture command resolved from a config template: the
// command line (first line, placeholders replaced) plus the device config
// content, duration, target process and remote output path needed to
// launch the capture.
struct CaptureCommand {
    bool ok = false;
    std::string error;

    // The command part (first line) after placeholder replacement.
    std::string command;

    // The config content (everything after the first line) for hiprofiler commands.
    std::string deviceConfigContent;

    // The full file content after placeholder replacement.
    std::string content;

    long long duration = 0;

    // The resolved target process name.
    std::string processName;

    // Device-side result file path extracted from the -o argument.
    std::string remoteOutputPath;
};

// Read the file at `config_file_path` and replace $(p:default) / $(t:default)
// / $(ts:) placeholders: p/t come from `opts`, ts expands to the local time
// stamp in YYYYMMDD_HHMMSS format.
CaptureCommand ProcessTemplate(const std::string& configFilePath, const ProfilerOptions& opts);

} // namespace mem_analyzer
