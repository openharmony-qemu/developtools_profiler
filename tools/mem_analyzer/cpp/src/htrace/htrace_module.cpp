#include "htrace/htrace_module.hpp"

#include "core/format_resolver.hpp"
#include "htrace/formatters/htrace_analysis_json_formatter.hpp"
#include "htrace/formatters/htrace_html_formatter.hpp"
#include "htrace/formatters/htrace_parse_json_formatter.hpp"
#include "htrace/htrace_analyzer.hpp"
#include "htrace/leak_filter_analyzer.hpp"
#include "htrace/htrace_parser.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"

#include <iostream>

namespace mem_analyzer {

ModuleCapability HtraceModule::capability() const {
    ModuleCapability cap;
    cap.parse.formats = {"json"};
    cap.parse.default_format = "json";
    cap.analysis.formats = {"json", "html"};
    cap.analysis.default_format = "html";
    return cap;
}

std::string HtraceModule::run(const ModuleRequest& req, const RunContext& ctx) const {
    FormatKeyword kw = resolve_format_keyword(name(), capability(), req.format, req.parse_only);
    if (!kw.error.empty()) {
        log::loge("Error: ", kw.error);
        return {};
    }
    log::logd("Resolved format: ", kw.keyword, " (parse_only=", req.parse_only, ")");

    const std::string& out_dir = ctx.output.out_dir;
    try {
        log::logi("  Parsing...");
        StageTimer timer("process_file");
        timer.stage("Parsing");
        TraceDB db = htrace_parse(req.file_path, ctx);
        timer.tick("parse");

        // -l: L1 泄漏地址过滤（parse 后，解析/分析模式均执行）
        if (!ctx.htrace.leak_addrs.empty()) {
            apply_leak_addr_filter(db, ctx.htrace.leak_addrs);
        }

        if (!req.parse_only) {
            log::logi("  Analyzing...");
            timer.stage("Analyzing");
            HtraceAnalysisOutput full = ctx.htrace.leak_addrs.empty()
                ? htrace_analyze(db, ctx)
                : htrace_leak_analyze(db, ctx);
            timer.tick("analysis");

            timer.stage("Writing output");
            std::string out = kw.keyword == "json"
                       ? htrace_write_analysis_json(full, req.file_path, ctx, out_dir)
                       : htrace_write_analysis_html(full, req.file_path, ctx, out_dir);
            timer.tick("write");
            timer.done();
            return out;
        }

        log::logi_minor("  Parse-only mode: skipping analysis.");
        timer.stage("Writing output");
        std::string out = htrace_write_parse_json(db, req.file_path, ctx, out_dir);
        timer.tick("write");
        timer.done();
        return out;
    } catch (const std::exception& e) {
        log::loge("Error: ", e.what());
        return {};
    }
}

} // namespace mem_analyzer
