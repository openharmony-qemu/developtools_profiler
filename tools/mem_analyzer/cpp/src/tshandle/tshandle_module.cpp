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
#include "tshandle/tshandle_module.hpp"

#include "core/format_resolver.hpp"
#include "tshandle/formatters/tshandle_analysis_json_formatter.hpp"
#include "tshandle/formatters/tshandle_formatter.hpp"
#include "tshandle/formatters/tshandle_html_formatter.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "tshandle/zip_parser.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"

#include <iostream>

namespace mem_analyzer {

ModuleCapability TshandleModule::capability() const
{
    ModuleCapability cap;
    cap.parse.formats = {"json"};
    cap.parse.defaultFormat = "json";
    cap.analysis.formats = {"json", "html"};
    cap.analysis.defaultFormat = "html";
    return cap;
}

std::string TshandleModule::run(const ModuleRequest& req, const RunContext& ctx) const
{
    FormatKeyword kw = ResolveFormatKeyword(name(), capability(), req.format, req.parseOnly);
    if (!kw.error.empty()) {
        log::Loge("Error: ", kw.error);
        return {};
    }
    log::Logd("Resolved format: ", kw.keyword, " (parse_only=", req.parseOnly, ")");

    const std::string& outDir = ctx.output.outDir;
    try {
        log::Logi("  Parsing...");
        StageTimer timer("process_file");
        timer.stage("Parsing");
        TshandleParseResult parsed = TshandleParse(req.filePath, ctx);
        timer.tick("parse");

        if (!req.parseOnly) {
            log::Logi("  Analyzing...");
            timer.stage("Analyzing");
            TshandleAnalysisOutput analysis = TshandleAnalyze(parsed, ctx);
            timer.tick("analysis");

            timer.stage("Writing output");
            std::string out = kw.keyword == "json"
                       ? TshandleWriteAnalysisJson(analysis, req.filePath, ctx, outDir)
                       : TshandleWriteAnalysisHtml(analysis, req.filePath, ctx, outDir);
            timer.tick("write");
            timer.done();
            return out;
        }

        // parse-only：产物仍含 ArkTS 类（见头文件约定）
        log::LogiMinor("  Parse-only mode: skipping analysis.");
        timer.stage("Writing output");
        std::string out = TshandleWriteParseJson(parsed, req.filePath, ctx, outDir);
        timer.tick("write");
        timer.done();
        return out;
    } catch (const std::exception& e)
    {
        log::Loge("Error: ", e.what());
        return {};
    }
}

} // namespace mem_analyzer
