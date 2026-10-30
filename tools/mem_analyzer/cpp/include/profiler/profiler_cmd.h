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

namespace mem_analyzer {

// Options parsed by the main CliParser for the `profiler` subcommand.
// Keeping the options in the overall CLI architecture means
// `mem-analyzer profiler --help` renders like any other subcommand
// (trace/tshandle/jsleak) instead of a separate help layout.
constexpr int DEFAULT_CAPTURE_DURATION_S = 120;

struct ProfilerCliOptions {
    std::string processName;        // positional <bundle>  target bundle name (required unless --list-apps)
    int duration = DEFAULT_CAPTURE_DURATION_S;              // -t            capture duration in seconds (default 120)
    bool durationSpecified = false; // whether -t was explicitly given on the command line
    std::string outputDir = "out";  // -d/--out-dir  output directory

    // Capture type: exactly one category.
    // Valid values: nativeheap/nh/native, snapshot/snap, globalhandle/global/gh,
    //               allhandle/handle/ah, localhandle/local/lh, nativeleaks/nl,
    //               fd, dma, thread, ashmem
    std::string category = "nativeheap";

    bool flagReport = false;        // -o/--auto-open  generate HTML report and open in browser
    bool flagListApps = false;     // --list-apps     list apps and exit

    // Deobfuscation: forwarded to the report subcommand so captured data is
    // restored before analysis (heapsnapshot via source map/nameCache, htrace via .so).
    std::string sourcemapDir;       // --sourcemap source map archive directory
    std::string nameObfuscation;    // --namecache nameCache archive directory
    std::string soDir;              // --so-dir native .so symbol directory (trace/tshandle only)
    std::string connectKey;         // --device value: device serial number
    bool connectKeyProvided = false;   // whether --device was given explicitly
};

// Run the profiler subcommand with already-parsed options. Performs
// environment checks (hdc + device), loads the config, processes the
// template, and executes the hdc command.
// Returns the process exit code.
int RunProfilerCmd(const ProfilerCliOptions& opts);

} // namespace mem_analyzer
