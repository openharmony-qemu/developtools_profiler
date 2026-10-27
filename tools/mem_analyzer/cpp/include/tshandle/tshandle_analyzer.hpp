#pragma once
#include <string>
#include <vector>
#include "core/run_context.hpp"
#include "tshandle/tshandle_processor.hpp"

namespace mem_analyzer {

struct TshandleCallChain {
    long long call_stack_id;
    std::vector<ResolvedStackFrame> frames;
    double existing_size = 0.0;
    long long count = 0;
    double percentage = 0.0;

    json to_json() const;
};

struct TshandleTopObject {
    int rank = 0;
    std::string object_name;
    double existing_size = 0.0;
    long long count = 0;
    double percentage = 0.0;
    HtraceEventType type = HtraceEventType::UNKNOWN;
    std::vector<TshandleCallChain> call_chains;

    json to_json() const;
};

struct TshandleTypeSection {
    bool has_data = false;
    long long total_objects = 0;
    double total_size = 0.0;
    long long total_count = 0;
    double total_call_stack_existing_size = 0.0;
    std::vector<TshandleTopObject> top_objects;

    json to_json() const;
};

/// 一个 handle 类型按调用链符号作用域（应用 / 系统）拆分后的两个 section。
struct TshandleHandleSections {
    TshandleTypeSection app;
    TshandleTypeSection sys;
};

struct TshandleAnalysisOutput {
    // Parse data carried forward for formatters
    TraceMeta trace_meta;
    std::string parsed_at;
    std::vector<ResolvedCallStack> call_stacks;
    std::vector<ArkTsClass> arkts_classes;

    // GlobalHandle 按调用链拆分为应用 / 系统两个 section；LocalHandle 不拆分。
    TshandleHandleSections global;
    TshandleTypeSection local;

    json to_json() const;
};

TshandleAnalysisOutput analyze_tshandle_data(
    const std::vector<ResolvedCallStack>& call_stacks,
    const std::vector<ArkTsClass>& arkts_classes,
    int top_n);

/// tshandle 分析步骤（global / local 两个 handle 类型的聚合）。
/// 强类型入参/出参：不再经过 ProcessData / std::any。
/// 会消费 `parse_result`（其 call_stacks 与 arkts_classes 被移出）。
TshandleAnalysisOutput tshandle_analyze(TshandleParseResult& parse_result, const RunContext& ctx);

} // namespace mem_analyzer
