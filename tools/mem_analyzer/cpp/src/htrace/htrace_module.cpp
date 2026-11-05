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
#include "htrace/htrace_module.h"

#include "core/format_resolver.h"
#include "htrace/formatters/htrace_analysis_json_formatter.h"
#include "htrace/formatters/htrace_html_formatter.h"
#include "htrace/formatters/htrace_parse_json_formatter.h"
#include "htrace/htrace_analyzer.h"
#include "htrace/leak_filter_analyzer.h"
#include "htrace/htrace_parser.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/timer.h"

#include <iostream>

namespace mem_analyzer {

ModuleCapability HtraceModule::Capability() const
{
    ModuleCapability cap;
    cap.parse.formats = {"json"};
    cap.parse.defaultFormat = "json";
    cap.analysis.formats = {"json", "html"};
    cap.analysis.defaultFormat = "html";
    return cap;
}

std::string HtraceModule::Run(const ModuleRequest& req, const RunContext& ctx) const
{
    FormatKeyword kw = ResolveFormatKeyword(Name(), Capability(), req.format, req.parseOnly);
    if (!kw.error.empty()) {
        log::Loge("Error: ", kw.error);
        return {};
    }
    log::Logd("Resolved format: ", kw.keyword, " (parse_only=", req.parseOnly, ")");

    const std::string& outDir = ctx.output.outDir;
    try {
        log::Logi("  Parsing...");
        StageTimer timer("process_file");
        timer.Stage("Parsing");
        TraceDB db = HtraceParse(req.filePath, ctx);
        timer.Tick("parse");

        // -l: L1 泄漏地址过滤（parse 后，解析/分析模式均执行）
        if (!ctx.htrace.leakAddrs.empty()) {
            ApplyLeakAddrFilter(db, ctx.htrace.leakAddrs);
        }

        if (!req.parseOnly) {
            log::Logi("  Analyzing...");
            timer.Stage("Analyzing");
            HtraceAnalysisOutput full = ctx.htrace.leakAddrs.empty() ?
                HtraceAnalyze(db, ctx)
                : HtraceLeakAnalyze(db, ctx);
            timer.Tick("analysis");

            timer.Stage("Writing output");
            std::string out = kw.keyword == "json" ?
                       HtraceWriteAnalysisJson(full, req.filePath, ctx, outDir)
                       : HtraceWriteAnalysisHtml(full, req.filePath, ctx, outDir);
            timer.Tick("write");
            timer.Done();
            return out;
        }

        log::LogiMinor("  Parse-only mode: skipping analysis.");
        timer.Stage("Writing output");
        std::string out = HtraceWriteParseJson(db, req.filePath, ctx, outDir);
        timer.Tick("write");
        timer.Done();
        return out;
    } catch (const std::exception& e) {
        log::Loge("Error: ", e.what());
        return {};
    }
}

} // namespace mem_analyzer
