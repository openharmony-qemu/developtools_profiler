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

ModuleCapability JsleakModule::capability() const {
    // parse 侧全空 = 该域明确不支持 parse-only（`-p` 会被拒绝）
    ModuleCapability cap;
    cap.analysis.formats = {"html", "json"};
    cap.analysis.default_format = "html";
    return cap;
}

std::string JsleakModule::run(const ModuleRequest& req, const RunContext& ctx) const {
    FormatKeyword kw = resolve_format_keyword(name(), capability(), req.format, req.parse_only);
    if (!kw.error.empty()) {
        log::loge("Error: ", kw.error);
        return {};
    }
    log::logd("Resolved format: ", kw.keyword, " (parse_only=", req.parse_only, ")");

    const std::string& out_dir = ctx.output.out_dir;
    try {
        // 对比模式：-b/--baseline 非空时，req.file_path 为目标快照 B。
        // 与单文件路径共用 parse → analyze → write 骨架与格式路由。
        if (!ctx.snapshot.compare.baseline_file.empty()) {
            std::string error;
            if (!jsleak_compare_inputs_valid(ctx.snapshot.compare.baseline_file,
                                             req.file_path, error)) {
                log::loge("Error: ", error);
                return {};
            }
            StageTimer timer("process_file");
            timer.stage("Parsing baseline");
            HeapSnapshot baseline = jsleak_parse(ctx.snapshot.compare.baseline_file, ctx);
            timer.tick("parse_baseline");
            timer.stage("Parsing target");
            HeapSnapshot target = jsleak_parse(req.file_path, ctx);
            timer.tick("parse_target");

            timer.stage("Analyzing comparison");
            json report = jsleak_compare_analyze(
                baseline, target, ctx.snapshot.compare.baseline_file, req.file_path,
                ctx.snapshot.top_n, ctx.snapshot.mode);
            timer.tick("analysis");

            timer.stage("Writing output");
            std::string out = kw.keyword == "json"
                       ? jsleak_write_compare_json(report, out_dir)
                       : jsleak_write_compare_html(report, out_dir);
            timer.tick("write");
            timer.done();
            return out;
        }

        log::logi("  Parsing...");
        StageTimer timer("process_file");
        timer.stage("Parsing");
        HeapSnapshot heap = jsleak_parse(req.file_path, ctx);
        timer.tick("parse");

        log::logi("  Analyzing...");
        timer.stage("Analyzing");
        json data = jsleak_analyze(heap, req.file_path, ctx);
        timer.tick("analysis");

        timer.stage("Writing output");
        std::string out = kw.keyword == "json"
                   ? jsleak_write_analysis_json(data, req.file_path, ctx, out_dir)
                   : jsleak_write_analysis_html(data, req.file_path, ctx, out_dir);
        timer.tick("write");
        timer.done();
        return out;
    } catch (const std::exception& e) {
        log::loge("Error: ", e.what());
        return {};
    }
}

} // namespace mem_analyzer
