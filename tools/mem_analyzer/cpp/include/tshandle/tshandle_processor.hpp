#pragma once
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "shared/htrace_model/htrace_data.hpp"
using json = nlohmann::ordered_json;

/**
 * tshandle 数据结构 — ARK handle 事件（type 17=globalhandle, 18=localhandle）。
 * Frame/CallStack/Meta 已统一到 htrace_data.hpp 中的 ResolvedStackFrame / ResolvedCallStack / TraceMeta。
 */
namespace mem_analyzer {

// ── ArkTS class analysis types (shared with arkts_analyzer.hpp) ──

struct NativeRefEntry {
    HtraceEventType handle_type; // globalhandle or localhandle
    long long call_stack_id;     // maps to CallStack::id
    std::vector<std::string> addrs;
};

struct ArkTsInstance {
    uint32_t id;                 // heapsnapshot node id
    long long shallow_size;
    long long retained_size;
    std::vector<NativeRefEntry> native_ref;
};

struct ArkTsClass {
    std::string name;
    long long shallow_size;
    long long retained_size;
    std::vector<ArkTsInstance> instances;
};

struct AnomalyCounts {
    int multi = 0;
    int no_node = 0;
    int missing_node = 0;
    int invalid_addr = 0;
    int duplicate = 0;
    int addr_file_invalid = 0;
};

// ── Parse result for tshandle pipeline ──

/**
 * Complete parse result for tshandle (zip) processing pipeline.
 * Wraps TraceDB from DB extraction plus ArkTS analysis data from rawheap.
 * `tshandle_parse()` 的强类型返回值，直接传给 `tshandle_analyze()`。
 */
struct TshandleParseResult {
    TraceDB db;
    std::string parsed_at;    // timestamp for report display
    // ArkTS classes (filled when rawheap is available)
    std::vector<ArkTsClass> arkts_classes;
    AnomalyCounts anomaly_counts;
    std::vector<std::string> anomaly_lines;

    // Convenience accessors — tshandle always uses ResolvedCallStackList
    std::vector<ResolvedCallStack>& call_stacks() {
        return std::get<ResolvedCallStackList>(db.stacks).stacks;
    }
    const std::vector<ResolvedCallStack>& call_stacks() const {
        return std::get<ResolvedCallStackList>(db.stacks).stacks;
    }
};

/**
 * 处理 trace_streamer 生成的 SQLite DB，提取 ARK handle 调用栈。
 * 返回 TraceDB，与 process_htrace_db 一致。
 */
TraceDB process_tshandle_db(const std::string& db_path);

} // namespace mem_analyzer
