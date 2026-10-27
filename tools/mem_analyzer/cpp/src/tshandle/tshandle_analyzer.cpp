#include "tshandle/tshandle_analyzer.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {
namespace {

bool call_stack_is_app(const ResolvedCallStack& stack) {
    for (const auto& frame : stack.frames) {
        if (HtraceConfigMgr::instance().is_app_symbol(frame.lib, true)) {
            return true;
        }
    }
    return false;
}

/// 构建一个 section，只纳入 id 在 valid_stack_ids 中的调用栈。
/// object.existing_size = 该对象这些调用链分摊到的 retained size（链 size 之和），
/// total_size 为各对象 size 之和，percentage 相对该 total_size 计算。
/// 对同一对象，应用作用域链 size + 系统作用域链 size = 该对象全部链 size 之和（拆分前后守恒）。
TshandleTypeSection build_section_with_stacks(
    HtraceEventType handle_type,
    const std::vector<ArkTsClass>& arkts_classes,
    const std::unordered_map<long long, const ResolvedCallStack*>& stacks_by_id,
    const std::unordered_set<long long>& valid_stack_ids,
    int top_n)
{
    TshandleTypeSection section;
    for (long long id : valid_stack_ids) {
        const auto it = stacks_by_id.find(id);
        if (it != stacks_by_id.end()) {
            section.has_data = true;
            section.total_call_stack_existing_size += it->second->related_size;
        }
    }

    std::vector<TshandleTopObject> objects;
    for (const auto& cls : arkts_classes) {
        if (cls.name.empty()) {
            continue;
        }
        TshandleTopObject object;
        object.object_name = cls.name;
        object.type = handle_type;
        std::vector<long long> call_stack_refs;
        std::unordered_set<long long> seen_call_stack_ids;
        std::unordered_map<long long, double> chain_size_by_call_stack;
        std::unordered_map<long long, long long> chain_count_by_call_stack;

        for (const auto& instance : cls.instances) {
            int total_addrs = 0;
            for (const auto& ref : instance.native_ref) {
                if (ref.handle_type == handle_type) {
                    total_addrs += static_cast<int>(ref.addrs.size());
                }
            }
            if (total_addrs == 0) {
                continue;
            }
            bool instance_has_ref = false;
            for (const auto& ref : instance.native_ref) {
                if (ref.handle_type != handle_type||
                    ref.call_stack_id == 0 ||
                    valid_stack_ids.find(ref.call_stack_id) == valid_stack_ids.end()) {
                    continue;
                }
                long long id = ref.call_stack_id;
                instance_has_ref = true;
                if (seen_call_stack_ids.insert(id).second) {
                    call_stack_refs.push_back(id);
                }
                if (instance.retained_size > 0) {
                    chain_size_by_call_stack[id] +=
                        (static_cast<double>(instance.retained_size) * ref.addrs.size() / total_addrs);
                }
                chain_count_by_call_stack[id] += static_cast<long long>(ref.addrs.size());
            }
            if (instance_has_ref) {
                ++object.count;
            }
        }

        double object_scope_size = 0.0;
        for (const auto& [id, sz] : chain_size_by_call_stack) {
            object_scope_size += sz;
        }

        std::stable_sort(call_stack_refs.begin(), call_stack_refs.end());
        for (const auto& id : call_stack_refs) {
            const auto stack_it = stacks_by_id.find(id);
            if (stack_it == stacks_by_id.end()) {
                continue;
            }
            const ResolvedCallStack* stack = stack_it->second;
            if (stack->type != handle_type || stack->frames.empty()) {
                continue;
            }
            const auto cs_it = chain_size_by_call_stack.find(id);
            const double cs_size = (cs_it != chain_size_by_call_stack.end()) ? cs_it->second : 0.0;
            const auto count_it = chain_count_by_call_stack.find(id);
            const long long cs_count = (count_it != chain_count_by_call_stack.end()) ? count_it->second : 0LL;
            if (cs_size <= 0.0 && cs_count == 0) {
                continue;
            }
            object.call_chains.push_back({stack->id, stack->frames, cs_size, cs_count, 0.0});
        }
        if (object.call_chains.empty()) {
            continue;
        }
        object.existing_size = object_scope_size;
        objects.push_back(std::move(object));
    }

    section.total_objects = static_cast<long long>(objects.size());
    std::stable_sort(objects.begin(), objects.end(), [](const auto& left, const auto& right) {
        return left.existing_size > right.existing_size;
    });

    double total_size = 0.0;
    for (const auto& object : objects) {
        total_size += object.existing_size;
        section.total_count += object.count;
    }
    section.total_size = total_size;

    if (top_n > 0 && objects.size() > static_cast<std::size_t>(top_n)) {
        objects.resize(static_cast<std::size_t>(top_n));
    }

    for (std::size_t index = 0; index < objects.size(); ++index) {
        auto& object = objects[index];
        object.rank = static_cast<int>(index + 1);
        std::stable_sort(object.call_chains.begin(), object.call_chains.end(),
            [](const auto& left, const auto& right) {
                return left.existing_size > right.existing_size;
            });
    }
    section.top_objects = std::move(objects);
    return section;
}

void compute_section_percentages(TshandleTypeSection& section, double denominator) {
    for (auto& object : section.top_objects) {
        object.percentage = (denominator > 0.0) ? (object.existing_size / denominator * 100.0) : 0.0;
        for (auto& chain : object.call_chains) {
            chain.percentage = (denominator > 0.0) ? (chain.existing_size / denominator * 100.0) : 0.0;
        }
    }
}

json frame_to_json(const ResolvedStackFrame& frame) {
    return {{"depth", frame.depth}, {"symbol", frame.symbol}, {"buildId", frame.build_id},
            {"lib", frame.lib}, {"libOffset", frame.lib_offset}};
}

} // namespace

json TshandleCallChain::to_json() const {
    json frame_json = json::array();
    for (const auto& frame : frames) {
        frame_json.push_back(frame_to_json(frame));
    }
    return {{"callStackId", call_stack_id},
            {"existingSize", existing_size},
            {"count", count},
            {"percentage", percentage},
            {"frames", frame_json}};
}

json TshandleTopObject::to_json() const {
    json chain_json = json::array();
    for (const auto& chain : call_chains) {
        chain_json.push_back(chain.to_json());
    }
    return {{"rank", rank},
            {"objectName", object_name},
            {"existingSize", existing_size},
            {"count", count},
            {"percentage", percentage},
            {"type", htrace_event_type_name(type)},
            {"callChains", chain_json}};
}

json TshandleTypeSection::to_json() const {
    json object_json = json::array();
    for (const auto& object : top_objects) {
        object_json.push_back(object.to_json());
    }
    return {{"hasData", has_data},
            {"totalObjects", total_objects},
            {"totalSize", total_size},
            {"totalCount", total_count},
            {"totalCallStackExistingSize", total_call_stack_existing_size},
            {"topObjects", object_json}};
}

json TshandleAnalysisOutput::to_json() const {
    return {
        {"global", {{"app", global.app.to_json()}, {"sys", global.sys.to_json()}}},
        {"local", local.to_json()}
    };
}

TshandleAnalysisOutput analyze_tshandle_data(
    const std::vector<ResolvedCallStack>& call_stacks,
    const std::vector<ArkTsClass>& arkts_classes,
    int top_n)
{
    auto& cfg = HtraceConfigMgr::instance();
    if (!cfg.is_loaded()) {
        std::string err;
        cfg.load_default(err);
    }

    std::unordered_map<long long, const ResolvedCallStack*> stacks_by_id;
    for (const auto& stack : call_stacks) {
        if (stack.id != 0) {
            stacks_by_id[stack.id] = &stack;
        }
    }

    TshandleAnalysisOutput output;

    // GlobalHandle：按调用链符号作用域拆分为应用 / 系统两个 section。
    std::unordered_set<long long> global_app_ids;
    std::unordered_set<long long> global_sys_ids;
    for (const auto& [id, stack] : stacks_by_id) {
        if (stack->type != HtraceEventType::ARK_GLOBAL_HANDLE || stack->frames.empty()) {
            continue;
        }
        if (call_stack_is_app(*stack)) {
            global_app_ids.insert(id);
        } else {
            global_sys_ids.insert(id);
        }
    }
    output.global.app = build_section_with_stacks(
        HtraceEventType::ARK_GLOBAL_HANDLE, arkts_classes, stacks_by_id, global_app_ids, top_n);
    output.global.sys = build_section_with_stacks(
        HtraceEventType::ARK_GLOBAL_HANDLE, arkts_classes, stacks_by_id, global_sys_ids, top_n);
    const double global_total = output.global.app.total_size + output.global.sys.total_size;
    compute_section_percentages(output.global.app, global_total);
    compute_section_percentages(output.global.sys, global_total);

    // LocalHandle：不拆分，所有调用链合成单个 section。
    std::unordered_set<long long> local_ids;
    for (const auto& [id, stack] : stacks_by_id) {
        if (stack->type == HtraceEventType::ARK_LOCAL_HANDLE && !stack->frames.empty()) {
            local_ids.insert(id);
        }
    }
    output.local = build_section_with_stacks(
        HtraceEventType::ARK_LOCAL_HANDLE, arkts_classes, stacks_by_id, local_ids, top_n);
    compute_section_percentages(output.local, output.local.total_size);

    return output;
}

TshandleAnalysisOutput tshandle_analyze(TshandleParseResult& parse_result, const RunContext& ctx) {
    auto analysis = analyze_tshandle_data(parse_result.call_stacks(), parse_result.arkts_classes,
                                          ctx.tshandle.top_n);

    // Carry parse-level data needed by formatters
    analysis.trace_meta = std::move(parse_result.db.meta);
    analysis.parsed_at = std::move(parse_result.parsed_at);
    analysis.call_stacks = std::move(parse_result.call_stacks());
    analysis.arkts_classes = std::move(parse_result.arkts_classes);

    log::logok("  Analysis completed. Global(app): ", analysis.global.app.total_objects, " objects, ",
              analysis.global.app.total_count, " instances | Global(sys): ",
              analysis.global.sys.total_objects, " objects, ",
              analysis.global.sys.total_count, " instances | Local: ",
              analysis.local.total_objects, " objects, ",
              analysis.local.total_count, " instances.");

    return analysis;
}

} // namespace mem_analyzer
