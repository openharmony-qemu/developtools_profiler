#include "tshandle/formatters/tshandle_analysis_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/json_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include "tshandle/formatters/tshandle_call_graph.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace mem_analyzer {
namespace {

double round2(double v) {
    return std::round(v * 100.0) / 100.0;
}

double round1(double v) {
    return std::round(v * 10.0) / 10.0;
}

std::string format_duration(long long duration_ns) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3)
        << (static_cast<double>(duration_ns) / 1e9);
    return oss.str();
}

json trace_meta_to_json(const TshandleAnalysisOutput& analysis) {
    struct Totals {
        long long count = 0;
        long long memory = 0;
        long long shallow = 0;
        long long retained = 0;
    };
    Totals global_totals, local_totals;

    for (const auto& stack : analysis.call_stacks) {
        if (stack.type == HtraceEventType::ARK_GLOBAL_HANDLE) {
            ++global_totals.count;
            global_totals.memory += stack.existing_size;
        } else if (stack.type == HtraceEventType::ARK_LOCAL_HANDLE) {
            ++local_totals.count;
            local_totals.memory += stack.existing_size;
        }
    }

    for (const auto& cls : analysis.arkts_classes) {
        for (const auto& instance : cls.instances) {
            std::unordered_set<HtraceEventType> types;
            for (const auto& ref : instance.native_ref) {
                types.insert(ref.handle_type);
            }
            for (const auto& type : types) {
                if (type == HtraceEventType::ARK_GLOBAL_HANDLE) {
                    global_totals.shallow += instance.shallow_size;
                    global_totals.retained += instance.retained_size;
                } else if (type == HtraceEventType::ARK_LOCAL_HANDLE) {
                    local_totals.shallow += instance.shallow_size;
                    local_totals.retained += instance.retained_size;
                }
            }
        }
    }

    auto make_obj = [](const Totals& t) -> json {
        return {
            {"totalCount", t.count},
            {"totalMemory", t.memory},
            {"totalShallowSize", t.shallow},
            {"totalRetainedSize", t.retained}
        };
    };

    const auto& meta = analysis.trace_meta;
    return {
        {"isStatistics", (meta.mode == "detail") ? "0" : "1"},
        {"duration", format_duration(meta.duration_ns)},
        {"timestampFrom", meta.start_time_ms > 0 ? format_readable_timestamp(meta.start_time_ms) : "0"},
        {"timestampTo", meta.end_time_ms > 0 ? format_readable_timestamp(meta.end_time_ms) : "0"},
        {"processName", meta.process_name},
        {"global", make_obj(global_totals)},
        {"local", make_obj(local_totals)}
    };
}

json render_node_to_json(const TshandleRenderNode& node) {
    json children = json::array();
    for (const auto& child : node.children) {
        children.push_back(render_node_to_json(child));
    }
    return {
        {"depth", node.depth},
        {"shallowSize", node.value},
        {"retainedSize", node.total},
        {"count", node.count},
        {"name", node.symbol},
        {"lib", node.lib},
        {"libOffset", node.lib_offset},
        {"buildId", node.build_id},
        {"callees", std::move(children)}
    };
}

json section_with_trees_to_json(const TshandleTypeSection& section) {
    json object_json = json::array();
    for (const auto& object : section.top_objects) {
        auto tree = build_tshandle_downstream_graph(
            object.object_name, object.call_chains, object.existing_size);
        json obj = {
            {"rank", object.rank},
            {"existingSize", object.existing_size},
            {"existingCount", object.count},
            {"percentage", round2(object.percentage)},
            {"name", tree.symbol}
        };
        json icicle_arr = json::array();
        for (const auto& child : tree.children) {
            icicle_arr.push_back(render_node_to_json(child));
        }
        obj["icicle"] = std::move(icicle_arr);
        object_json.push_back(std::move(obj));
    }
    return {
        {"totalObjects", section.total_objects},
        {"totalSize", round1(section.total_size)},
        {"totalCount", section.total_count},
        {"totalCallStackExistingSize", round1(section.total_call_stack_existing_size)},
        {"topArkTsObjects", object_json}
    };
}

} // namespace

std::string tshandle_write_analysis_json(const TshandleAnalysisOutput& analysis,
                                        const std::string& file_path, const RunContext& ctx,
                                        const std::string& out_dir) {
    StageTimer timer("JSON write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".json");
    platform::create_directories(out_dir);

    JsonStreamWriter writer(out_path);

    json meta = build_output_meta_base(ctx);
    std::string filename = platform::filename(file_path);
    auto dot_pos = filename.rfind('.');
    std::string file_type = (dot_pos != std::string::npos) ? filename.substr(dot_pos + 1) : "";
    meta["sourceFileInfo"] = {{"name", file_path}, {"fileType", file_type}};

    json content;
    content["summary"] = {{"tshandle", trace_meta_to_json(analysis)}};
    content["global"] = {
        {"app", section_with_trees_to_json(analysis.global.app)},
        {"system", section_with_trees_to_json(analysis.global.sys)}
    };
    content["local"] = section_with_trees_to_json(analysis.local);

    writer.begin_object();
    writer.write_key("meta");
    writer.write_raw(meta.dump(4));
    writer.write_key("content");
    writer.write_raw(content.dump(4));
    writer.end_object();

    writer.close();

    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer
