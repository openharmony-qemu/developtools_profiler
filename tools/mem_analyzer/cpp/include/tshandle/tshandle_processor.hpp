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
    HtraceEventType handleType; // globalhandle or localhandle
    long long callStackId;     // maps to CallStack::id
    std::vector<std::string> addrs;
};

struct ArkTsInstance {
    uint32_t id;                 // heapsnapshot node id
    long long shallowSize;
    long long retainedSize;
    std::vector<NativeRefEntry> nativeRef;
};

struct ArkTsClass {
    std::string name;
    long long shallowSize;
    long long retainedSize;
    std::vector<ArkTsInstance> instances;
};

struct AnomalyCounts {
    int multi = 0;
    int noNode = 0;
    int missingNode = 0;
    int invalidAddr = 0;
    int duplicate = 0;
    int addrFileInvalid = 0;
};

// ── Parse result for tshandle pipeline ──

/**
 * Complete parse result for tshandle (zip) processing pipeline.
 * Wraps TraceDB from DB extraction plus ArkTS analysis data from rawheap.
 * `tshandle_parse()` 的强类型返回值，直接传给 `tshandle_analyze()`。
 */
struct TshandleParseResult {
    TraceDB db;
    std::string parsedAt;    // timestamp for report display
    // ArkTS classes (filled when rawheap is available)
    std::vector<ArkTsClass> arktsClasses;
    AnomalyCounts anomalyCounts;
    std::vector<std::string> anomalyLines;

    // Convenience accessors — tshandle always uses ResolvedCallStackList
    std::vector<ResolvedCallStack>& CallStacks()
    {
        return std::get<ResolvedCallStackList>(db.stacks).stacks;
    }
    const std::vector<ResolvedCallStack>& CallStacks() const
    {
        return std::get<ResolvedCallStackList>(db.stacks).stacks;
    }
};

/**
 * 处理 trace_streamer 生成的 SQLite DB，提取 ARK handle 调用栈。
 * 返回 TraceDB，与 process_htrace_db 一致。
 */
TraceDB ProcessTshandleDb(const std::string& dbPath);

} // namespace mem_analyzer
