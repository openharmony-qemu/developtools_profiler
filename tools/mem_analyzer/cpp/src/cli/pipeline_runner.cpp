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
#include "cli/pipeline_runner.hpp"

#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

#include <iostream>

namespace mem_analyzer {

static bool CheckPathLength(const std::string& filePath, const std::string& outputDir,
                              const std::string& sourceName) {
#if defined(_WIN32)
    constexpr size_t pathLimit = 260;
    constexpr const char* platformName = "Windows MAX_PATH";
#elif defined(__APPLE__)
    constexpr size_t pathLimit = 1024;
    constexpr const char* platformName = "macOS PATH_MAX";
#else
    constexpr size_t pathLimit = 4096;
    constexpr const char* platformName = "Linux PATH_MAX";
#endif
    auto absOut = platform::AbsolutePath(outputDir);
    auto absIn = platform::AbsolutePath(filePath);
    size_t outFileLen = absOut.size() + 1 + sourceName.size() + 5;

    if (outFileLen >= pathLimit) {
        log::Loge("Error: Output file path exceeds ", platformName, " limit (", pathLimit, " chars).\n"
                  "  Output file: ", absOut, "\\", sourceName, ".html (", outFileLen, " chars)\n"
                  "  Please move the input file or output directory to a shorter path.");
        return false;
    }
    if (absIn.size() >= pathLimit) {
        log::Loge("Error: Input path exceeds ", platformName, " limit (", absIn.size(), " chars).\n"
                  "  Input file: ", absIn, " (", absIn.size(), " chars)\n"
                  "  Please move the input file to a shorter path.");
        return false;
    }
    return true;
}

bool ProcessFile(const std::string& filePath, AnalysisModule& module,
                  const RunContext& baseCtx) {
    log::Logi("Processing file: ", filePath);
    log::Logi("Using parser: ", module.name());

    // 派生当前文件的运行参数：输出目录嵌套一层 source stem（与 zip/tshandle 行为一致）。
    // 不再改写任何全局状态。
    RunContext ctx = baseCtx;
    auto sourceName = platform::stem(filePath);
    ctx.output.outDir = platform::JoinPath(ctx.output.outDir, sourceName);
    log::Logd("output_dir: ", ctx.output.outDir, " (parse_only=", ctx.output.parseOnly,
              ", format=", (ctx.output.format.empty() ? "<default>" : ctx.output.format), ")");

    if (!CheckPathLength(filePath, ctx.output.outDir, sourceName)) {
        return false;
    }

    ModuleRequest req;
    req.filePath = filePath;
    req.parseOnly = ctx.output.parseOnly;
    req.format = ctx.output.format;

    const std::string outFile = module.run(req, ctx);
    if (outFile.empty()) {
        return false;
    }

    // Auto-open result file if --auto-open is set
    if (ctx.output.autoOpen && platform::FileExists(outFile)) {
        platform::OpenWithDefaultApp(outFile);
    }
    return true;
}

} // namespace mem_analyzer
