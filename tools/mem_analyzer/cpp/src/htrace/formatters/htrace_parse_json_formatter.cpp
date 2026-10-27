#include "htrace/formatters/htrace_parse_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/json_writer.hpp"
#include "htrace/formatters/htrace_json_formatter.hpp"
#include "htrace/htrace_category.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <cstdio>
#include <iostream>
#include <variant>

namespace mem_analyzer {
namespace {

json build_summary(const TraceDB& db) {
    const auto& ts = db.meta.type_sizes;

    auto get_type_size = [&](const char* name) -> long long {
        auto et = htrace_event_type_from_name(name);
        if (et == HtraceEventType::UNKNOWN) return 0;
        auto it = ts.find(et);
        return it != ts.end() ? it->second : 0;
    };

    json summary = htrace_build_summary_common(db.meta.mode, db.meta.duration_ns,
                                               db.meta.start_time_ms, db.meta.end_time_ms,
                                               db.meta.process_name);

    // Parse mode: output all resource types（summary_entries 表驱动，顺序=历史字节序）
    static const char* const kParseSummaryOrder[] = {
        "nativeheap", "mmap", "ashmem", "dma", "fd", "thread"
    };
    for (const char* cat : kParseSummaryOrder) {
        for (const auto& e : htrace_category(cat).summary_entries) {
            long long v = 0;
            for (const char* tn : e.type_names) v += get_type_size(tn);
            if (v > 0) summary[e.json_key] = std::to_string(v);
        }
    }

    return summary;
}

} // namespace

std::string htrace_write_parse_json(const TraceDB& db, const std::string& file_path,
                                    const RunContext& ctx, const std::string& out_dir) {
    StageTimer timer("JSON write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".json");
    platform::create_directories(out_dir);

    JsonStreamWriter writer(out_path);

    const auto& stacks = std::get<CompressedCallStackList>(db.stacks);
    json summary = build_summary(db);

    writer.begin_object();

    json meta = htrace_build_meta(file_path, ctx);
    writer.write_key("meta");
    writer.write_raw(meta.dump(4));

    writer.write_key("content");
    writer.begin_object();

    // summary (small, raw dump)
    writer.write_key("summary");
    writer.begin_object();
    writer.write_key("htrace");
    writer.write_raw(summary.dump(4));
    writer.end_object();

    // callStacks — streaming, one at a time
    writer.write_key("callStacks");
    writer.begin_array();
    for (const auto& cs : stacks.stacks) {
        writer.begin_object();
        writer.write_key_value("id", std::to_string(cs.id));
        writer.write_key_value("type", htrace_event_type_name(cs.type));
        if (cs.type == HtraceEventType::FILE_PAGE_MSG) {
            std::string filename = "unknown";
            auto fit = stacks.dict.find(cs.sub_type_id);
            if (fit != stacks.dict.end()) filename = fit->second;
            filename = normalize_filepage_filename(filename, db.meta.mode);
            writer.write_key_value("fileName", filename);
        }
        writer.write_key_value("existingSize", std::to_string(cs.existing_size));
        writer.write_key_value("existingCount", std::to_string(cs.existing_count));

        writer.write_key("frames");
        writer.begin_array();
        for (const auto& f : cs.frames) {
            htrace_write_callstack_frame(writer, f, stacks.dict);
        }
        writer.end_array();

        writer.end_object();
    }
    writer.end_array();

    writer.end_object();
    writer.end_object();

    writer.close();

    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer