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

CaptureCommand ProcessTemplate(const std::string& configFilePath, const ProfilerOptions& opts)
{
    CaptureCommand result;

    // 1. Read the entire file at config_file_path into a std::string content.
    //    Use binary mode to preserve newlines exactly (the file has `\`
    //    line continuations and a heredoc).
    std::ifstream ifs(PathFromUtf8(configFilePath), std::ios::binary);
    if (!ifs.is_open()) {
        result.ok = false;
        result.error = "failed to open config file: " + configFilePath;
        return result;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string content = ss.str();

    // Normalize CRLF → LF before splitting. These config templates are plain
    // text that git may check out with CRLF on Windows. Without this, the first
    // line (`hiprofiler_cmd ...`) keeps a trailing \r, which rides into the
    // device command via `hdc shell` and is interpreted by the device shell as
    // an empty argument → hiprofiler_cmd reports "unknown param '\n'" and the
    // capture never starts. (This was masked before the P0 hardening by
    // `_popen`→`cmd.exe`, whose parser dropped the stray CR; `CreateProcessW`
    // passes it through verbatim, so we now normalize at the source.)
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

    // 2. Determine the countdown duration BEFORE replacement, from the ORIGINAL content.
    long long duration = 0;
    if (opts.t.has_value()) {
        duration = opts.t.value();
    } else {
        // Search the original content for the FIRST occurrence of `$(t:default)`.
        std::regex durRe(R"(\$\(t:([^)]*)\))");
        std::smatch durMatch;
        if (std::regex_search(content, durMatch, durRe)) {
            const std::string def = durMatch[1].str();
            try {
                duration = std::stoll(def);
            } catch (...)
            {
                // If it fails to parse, treat as 0.
                duration = 0;
            }
        }
        // If not found or unparseable: duration stays 0.
    }
    result.duration = duration;

    // 3. Replace placeholders using std::regex.
    //    Pattern matches `$(X:default)` where X is p, t or ts and default is any
    //    chars except `)`. `ts` expands to the local time stamp in
    //    YYYYMMDD_HHMMSS format (e.g. 20260809_232156) — this feeds the
    //    device-side output file name
    //    (mem_analyzer_hiprofiler_data_20260809_232156.htrace). (Note: `-o`
    //    is NOT a placeholder source — the CLI `-o` is the local receive dir,
    //    while the config's literal `-o` is the device-side result path
    //    extracted separately below.)
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
            // Capture the resolved process name so run_hdc can poll the
            // device for it during the countdown. All $(p:...) occurrences
            // resolve to the same value, so overwriting is harmless.
            result.processName = replacement;
        } else if (letter == "t") {
            replacement = opts.t.has_value() ? std::to_string(opts.t.value()) : def;
        } else if (letter == "ts") {
            replacement = CurrentTimestamp();
        }

        // Append the text between previous match end and this match start.
        out.append(lastEnd, match[0].first);
        // Append the replacement.
        out.append(replacement);

        lastEnd = match[0].second;
    }

    // Append any trailing text after the last match.
    out.append(lastEnd, content.cend());

    result.content = out;

    // 4. Split content into command (first line) and device config (rest).
    //    Hiprofiler commands have a multi-line format where the first line is
    //    the hiprofiler_cmd invocation and the remaining lines are the config
    //    content to be written to a file on the device. Snapshot commands are
    //    single-line (just the hidumper command).
    {
        const auto firstNl = out.find('\n');
        if (firstNl == std::string::npos) {
            result.command = out;
            result.deviceConfigContent.clear();
        } else {
            result.command = out.substr(0, firstNl);
            // Everything after the first newline is the config content.
            // Preserve the remaining content as-is.
            result.deviceConfigContent = out.substr(firstNl + 1);
        }
    }

    // 5. Extract the hiprofiler_cmd `-o <remote_path>` argument from the
    //    command line (first line). Only scan the command tokens, stop at any
    //    token starting with `-` after `-o`'s value has been consumed.
    {
        std::istringstream iss(result.command);
        std::string tok;
        bool waitingForValue = false;
        while (iss >> tok) {
            if (waitingForValue) {
                if (!tok.empty() && tok[0] != '-') {
                    result.remoteOutputPath = tok;
                }
                break;  // consumed the value (or found none); done
            }
            if (tok == "-o") {
                waitingForValue = true;
            }
        }
    }

    // 6. Set result.ok = true and return.
    result.ok = true;
    return result;
}

} // namespace mem_analyzer
