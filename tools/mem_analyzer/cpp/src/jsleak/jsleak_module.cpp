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
#include "jsleak/jsleak_module.hpp"

#include "core/format_resolver.hpp"
#include "jsleak/formatters/snapshot_compare_formatter.hpp"
#include "jsleak/formatters/snapshot_single_html_formatter.hpp"
#include "jsleak/formatters/snapshot_single_json_formatter.hpp"
#include "jsleak/snapshot_analyzer.hpp"
#include "jsleak/snapshot_compare.hpp"
#include "jsleak/snapshot_parser.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"

#include <iostream>

namespace mem_analyzer {

ModuleCapability JsleakModule::capability() const
{
    // parse 侧全空 = 该域明确不支持 parse-only（`-p` 会被拒绝）
    ModuleCapability cap;
    cap.analysis.formats = {"html", "json"};
    cap.analysis.defaultFormat = "html";
    return cap;
}

std::string JsleakModule::run(const ModuleRequest& req, const RunContext& ctx) const
{
    FormatKeyword kw = ResolveFormatKeyword(name(), capability(), req.format, req.parseOnly);
    if (!kw.error.empty()) {
        log::Loge("Error: ", kw.error);
        return {};
    }
    log::Logd("Resolved format: ", kw.keyword, " (parse_only=", req.parseOnly, ")");

    const std::string& outDir = ctx.output.outDir;
    try {
        // 对比模式：-b/--baseline 非空时，req.file_path 为目标快照 B。
        // 与单文件路径共用 parse → analyze → write 骨架与格式路由。
        if (!ctx.snapshot.compare.baselineFile.empty()) {
            std::string error;
            if (!JsleakCompareInputsValid(ctx.snapshot.compare.baselineFile,
                req.filePath, error)) {
                log::Loge("Error: ", error);
                return {};
            }
            StageTimer timer("process_file");
            timer.Stage("Parsing baseline");
            HeapSnapshot baseline = JsleakParse(ctx.snapshot.compare.baselineFile, ctx);
            timer.Tick("parse_baseline");
            timer.Stage("Parsing target");
            HeapSnapshot target = JsleakParse(req.filePath, ctx);
            timer.Tick("parse_target");

            timer.Stage("Analyzing comparison");
            json report = JsleakCompareAnalyze(
                baseline, target, ctx.snapshot.compare.baselineFile, req.filePath,
                ctx.snapshot.topN, ctx.snapshot.mode);
            timer.Tick("analysis");

            timer.Stage("Writing output");
            std::string out = kw.keyword == "json" ?
                       JsleakWriteCompareJson(report, outDir)
                       : JsleakWriteCompareHtml(report, outDir);
            timer.Tick("write");
            timer.Done();
            return out;
        }

        log::Logi("  Parsing...");
        StageTimer timer("process_file");
        timer.Stage("Parsing");
        HeapSnapshot heap = JsleakParse(req.filePath, ctx);
        timer.Tick("parse");

        log::Logi("  Analyzing...");
        timer.Stage("Analyzing");
        json data = JsleakAnalyze(heap, req.filePath, ctx);
        timer.Tick("analysis");

        timer.Stage("Writing output");
        std::string out = kw.keyword == "json" ?
                   JsleakWriteAnalysisJson(data, req.filePath, ctx, outDir)
                   : JsleakWriteAnalysisHtml(data, req.filePath, ctx, outDir);
        timer.Tick("write");
        timer.Done();
        return out;
    } catch (const std::exception& e)
    {
        log::Loge("Error: ", e.what());
        return {};
    }
}

} // namespace mem_analyzer
