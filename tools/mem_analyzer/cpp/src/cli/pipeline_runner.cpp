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

static bool check_path_length(const std::string& file_path, const std::string& output_dir,
                              const std::string& source_name) {
#if defined(_WIN32)
    constexpr size_t path_limit = 260;
    constexpr const char* platform_name = "Windows MAX_PATH";
#elif defined(__APPLE__)
    constexpr size_t path_limit = 1024;
    constexpr const char* platform_name = "macOS PATH_MAX";
#else
    constexpr size_t path_limit = 4096;
    constexpr const char* platform_name = "Linux PATH_MAX";
#endif
    auto abs_out = platform::absolute_path(output_dir);
    auto abs_in = platform::absolute_path(file_path);
    size_t out_file_len = abs_out.size() + 1 + source_name.size() + 5;

    if (out_file_len >= path_limit) {
        log::loge("Error: Output file path exceeds ", platform_name, " limit (", path_limit, " chars).\n"
                  "  Output file: ", abs_out, "\\", source_name, ".html (", out_file_len, " chars)\n"
                  "  Please move the input file or output directory to a shorter path.");
        return false;
    }
    if (abs_in.size() >= path_limit) {
        log::loge("Error: Input path exceeds ", platform_name, " limit (", abs_in.size(), " chars).\n"
                  "  Input file: ", abs_in, " (", abs_in.size(), " chars)\n"
                  "  Please move the input file to a shorter path.");
        return false;
    }
    return true;
}

bool process_file(const std::string& file_path, AnalysisModule& module,
                  const RunContext& base_ctx) {
    log::logi("Processing file: ", file_path);
    log::logi("Using parser: ", module.name());

    // 派生当前文件的运行参数：输出目录嵌套一层 source stem（与 zip/tshandle 行为一致）。
    // 不再改写任何全局状态。
    RunContext ctx = base_ctx;
    auto source_name = platform::stem(file_path);
    ctx.output.out_dir = platform::join_path(ctx.output.out_dir, source_name);
    log::logd("output_dir: ", ctx.output.out_dir, " (parse_only=", ctx.output.parse_only,
              ", format=", (ctx.output.format.empty() ? "<default>" : ctx.output.format), ")");

    if (!check_path_length(file_path, ctx.output.out_dir, source_name)) {
        return false;
    }

    ModuleRequest req;
    req.file_path = file_path;
    req.parse_only = ctx.output.parse_only;
    req.format = ctx.output.format;

    const std::string out_file = module.run(req, ctx);
    if (out_file.empty()) {
        return false;
    }

    // Auto-open result file if --auto-open is set
    if (ctx.output.auto_open && platform::file_exists(out_file)) {
        platform::open_with_default_app(out_file);
    }
    return true;
}

} // namespace mem_analyzer
