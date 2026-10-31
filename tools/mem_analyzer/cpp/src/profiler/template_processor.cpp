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
#include "profiler/template_processor.h"
#include "profiler/string_utils.hpp"
#include "utils/file_utils.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>

namespace mem_analyzer {

/// Extract countdown duration from content or opts.
static long long ExtractTemplateDuration(const std::string& content,
    const ProfilerOptions& opts)
{
    if (opts.t.has_value()) {
        return opts.t.value();
    }
    std::regex durRe(R"(\$\(t:([^)]*)\))");
    std::smatch durMatch;
    if (std::regex_search(content, durMatch, durRe)) {
        try {
            return std::stoll(durMatch[1].str());
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

/// Replace $(p:...)/$(t:...)/$(ts:...) placeholders in template content.
static std::string ReplaceTemplatePlaceholders(const std::string& content,
    const ProfilerOptions& opts, std::string& processName)
{
    std::regex re(R"(\$\((p|t|ts):([^)]*)\))");
    std::string out;
    out.reserve(content.size());
    std::sregex_iterator it(content.cbegin(), content.cend(), re);
    std::sregex_iterator end;
    std::string::const_iterator lastEnd = content.cbegin();
    for (; it != end; ++it) {
        const std::smatch& match = *it;
        const std::string letter = match[1].str();
        const std::string def = match[2].str();
        std::string replacement;
        if (letter == "p") {
            replacement = opts.p.has_value() ? opts.p.value() : def;
            processName = replacement;
        } else if (letter == "t") {
            replacement = opts.t.has_value() ? std::to_string(opts.t.value()) : def;
        } else if (letter == "ts") {
            replacement = CurrentTimestamp();
        }
        out.append(lastEnd, match[0].first);
        out.append(replacement);
        lastEnd = match[0].second;
    }
    out.append(lastEnd, content.cend());
    return out;
}

/// Split content into command (first line) and device config (rest).
static void SplitCommandAndConfig(const std::string& out, CaptureCommand& result)
{
    const auto firstNl = out.find('\n');
    if (firstNl == std::string::npos) {
        result.command = out;
        result.deviceConfigContent.clear();
    } else {
        result.command = out.substr(0, firstNl);
        result.deviceConfigContent = out.substr(firstNl + 1);
    }
}

/// Extract the hiprofiler_cmd -o <remote_path> argument from command line.
static std::string ExtractRemoteOutputPath(const std::string& command)
{
    std::istringstream iss(command);
    std::string tok;
    bool waitingForValue = false;
    while (iss >> tok) {
        if (waitingForValue) {
            if (!tok.empty() && tok[0] != '-') {
                return tok;
            }
            return {};
        }
        if (tok == "-o") {
            waitingForValue = true;
        }
    }
    return {};
}

CaptureCommand ProcessTemplate(const std::string& configFilePath, const ProfilerOptions& opts)
{
    CaptureCommand result;

    std::ifstream ifs(PathFromUtf8(configFilePath), std::ios::binary);
    if (!ifs.is_open()) {
        result.ok = false;
        result.error = "failed to open config file: " + configFilePath;
        return result;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string content = ss.str();
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

    result.duration = ExtractTemplateDuration(content, opts);

    std::string out = ReplaceTemplatePlaceholders(content, opts, result.processName);
    result.content = out;

    SplitCommandAndConfig(out, result);

    result.remoteOutputPath = ExtractRemoteOutputPath(result.command);

    result.ok = true;
    return result;
}

} // namespace mem_analyzer
