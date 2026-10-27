#include "tshandle/formatters/tshandle_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include "tshandle/tshandle_processor.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {
namespace {

// Round double to 2 decimal places for JSON output
static double round2(double v) {
    return std::round(v * 100.0) / 100.0;
}

json build_summary(const std::vector<ResolvedCallStack>& call_stacks,
                  const std::vector<ArkTsClass>& arkts_classes) {
    struct Totals {
        long long count = 0;
        long long memory = 0;
        long long shallow = 0;
        long long retained = 0;
    };
    std::unordered_map<HtraceEventType, Totals> by_type = {
        {HtraceEventType::ARK_GLOBAL_HANDLE, {}},
        {HtraceEventType::ARK_LOCAL_HANDLE, {}}
    };

    for (const auto& stack : call_stacks) {
        const auto found = by_type.find(stack.type);
        if (found == by_type.end()) continue;
        ++found->second.count;
        found->second.memory += stack.existing_size;
    }

    for (const auto& cls : arkts_classes) {
        for (const auto& instance : cls.instances) {
            std::unordered_set<HtraceEventType> types;
            for (const auto& ref : instance.native_ref) {
                types.insert(ref.handle_type);
            }
            for (const auto& type : types) {
                const auto found = by_type.find(type);
                if (found == by_type.end()) continue;
                found->second.shallow += instance.shallow_size;
                found->second.retained += instance.retained_size;
            }
        }
    }

    json handles = json::array();
    for (auto htype : {HtraceEventType::ARK_GLOBAL_HANDLE, HtraceEventType::ARK_LOCAL_HANDLE}) {
        const auto& total = by_type.at(htype);
        if (total.count == 0 && total.memory == 0
            && total.shallow == 0 && total.retained == 0) continue;
        handles.push_back({
            {"handleType", htrace_event_type_name(htype)},
            {"totalCount", total.count},
            {"totalMemory", total.memory},
            {"totalShallowSize", total.shallow},
            {"totalRetainedSize", total.retained}
        });
    }
    return {{"arkTsHandle", handles}};
}

// Convert ResolvedCallStack to JSON for output
static json call_stack_to_json(const ResolvedCallStack& stack) {
    json j;
    j["id"] = stack.id;
    j["type"] = htrace_event_type_name(stack.type);
    j["existingSize"] = round2(stack.related_size);
    j["nativeRefObjSize"] = stack.existing_size;
    json addrs = json::array();
    if (stack.existing_addrs) {
        for (const auto& addr : *stack.existing_addrs) {
            addrs.push_back(addr);
        }
    }
    j["existingAddrs"] = addrs;
    json frames = json::array();
    for (const auto& f : stack.frames) {
        frames.push_back({
            {"depth", f.depth},
            {"symbol", f.symbol},
            {"buildId", f.build_id},
            {"lib", f.lib},
            {"libOffset", f.lib_offset}
        });
    }
    j["frames"] = frames;
    return j;
}

// Convert TraceMeta to JSON
static json trace_meta_to_json(const TraceMeta& meta) {
    return {
        {"startTimeMs", meta.start_time_ms},
        {"endTimeMs", meta.end_time_ms},
        {"duration", meta.duration_ns},
        {"processName", meta.process_name}
    };
}

// Convert ArkTsClass vector to JSON
static json arkts_classes_to_json(const std::vector<ArkTsClass>& classes) {
    // 1. Merge buckets by clean JSON name
    std::unordered_map<std::string, ArkTsClass> merged_map;
    std::vector<std::string> order; // Preserve insertion order
    
    for (const auto& cls : classes) {
        std::string json_name = cls.name;
        if (json_name.find("JSSharedObject(") == 0) {
            json_name = "JSSharedObject";
        } else if (json_name.find("JSObject(") == 0) {
            json_name = "JSObject";
        } else if (json_name.find("string(size:") == 0) {
            json_name = "(unknown)";
        }
        
        auto it = merged_map.find(json_name);
        if (it == merged_map.end()) {
            ArkTsClass new_cls;
            new_cls.name = json_name;
            new_cls.shallow_size = cls.shallow_size;
            new_cls.retained_size = cls.retained_size;
            new_cls.instances = cls.instances;
            merged_map[json_name] = std::move(new_cls);
            order.push_back(json_name);
        } else {
            it->second.shallow_size += cls.shallow_size;
            it->second.retained_size += cls.retained_size;
            it->second.instances.insert(it->second.instances.end(), cls.instances.begin(), cls.instances.end());
        }
    }

    // Sort the keys alphabetically to perfectly match the legacy std::map output order
    std::sort(order.begin(), order.end());

    // 2. Output to JSON
    json arr = json::array();
    for (const auto& key : order) {
        const auto& cls = merged_map[key];
        json cj;
        cj["name"] = cls.name;
        cj["shallowSize"] = cls.shallow_size;
        cj["retainedSize"] = cls.retained_size;
        json instances = json::array();
        for (const auto& inst : cls.instances) {
            json ij;
            ij["id"] = inst.id;
            ij["shallowSize"] = inst.shallow_size;
            ij["retainedSize"] = inst.retained_size;
            json refs = json::array();
            // 排序：先按 handleType（global handle 优先），再按 callStackId 从大到小
            auto sorted_refs = inst.native_ref;
            std::sort(sorted_refs.begin(), sorted_refs.end(),
                [](const NativeRefEntry& a, const NativeRefEntry& b) {
                    if (a.handle_type != b.handle_type) {
                        return a.handle_type < b.handle_type;
                    }
                    return a.call_stack_id > b.call_stack_id;
                });
            for (const auto& ref : sorted_refs) {
                json rj;
                rj["handleType"] = htrace_event_type_name(ref.handle_type);
                rj["callStackId"] = ref.call_stack_id;
                json raddrs = json::array();
                for (const auto& addr : ref.addrs) {
                    raddrs.push_back(addr);
                }
                rj["addrs"] = raddrs;
                refs.push_back(rj);
            }
            ij["nativeRef"] = refs;
            instances.push_back(ij);
        }
        cj["instances"] = instances;
        arr.push_back(cj);
    }
    return arr;
}

} // namespace

std::string tshandle_write_parse_json(const TshandleParseResult& parsed, const std::string& file_path,
                                      const RunContext& ctx, const std::string& out_dir) {
    StageTimer timer("JSON write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".json");
    platform::create_directories(out_dir);

    JsonStreamWriter writer(out_path);

    json meta = build_output_meta_base(ctx);
    meta["sourceFileInfo"] = {{"zipFile", file_path}};

    json content;
    content["summary"] = build_summary(parsed.call_stacks(), parsed.arkts_classes);

    if (!parsed.arkts_classes.empty()) {
        content["arkTsClasses"] = arkts_classes_to_json(parsed.arkts_classes);
    } else {
        content["arkTsClasses"] = json::array();
    }

    // memoryTraceInfo
    json call_stacks_json = json::array();
    for (const auto& cs : parsed.call_stacks()) {
        call_stacks_json.push_back(call_stack_to_json(cs));
    }
    content["memoryTraceInfo"] = json::object();
    content["memoryTraceInfo"]["callStacks"] = call_stacks_json;

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
