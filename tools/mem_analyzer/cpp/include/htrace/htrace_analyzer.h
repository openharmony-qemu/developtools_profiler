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
#include <unordered_map>
#include <set>
#include <map>
#include <cstdint>
#include <memory>
#include <variant>

#include "shared/htrace_model/htrace_data.h"
#include "core/run_context.h"
#include "config.h"
#include "htrace/flame_icicle.h"

namespace mem_analyzer {

struct TopSymbolInfo {
    uint64_t uid = 0;
    std::string symbol;
    std::vector<std::string> libs;
    std::string libOffset;
    std::string type;
    long long totalLeakSize = 0;
    long long totalExistingCount = 0;  // sum of existing_count over all chains containing this symbol
    bool isApp = false;
    int buildId = 0;
    bool buildIdNull = true;
    FlameNode flameGraph;
    FlameNode icicleGraph;
    std::vector<CompressedCallStack> callChains;
};

// "other" 符号：-l 分析模式下非 top N 覆盖的栈合并占位符（top_symbols 尾项）
inline constexpr const char* OTHER_SYMBOL = "Others";

struct AlgorithmStats {
    int orgSetSize = 0;
    int filteredSetSize = 0;
    int finalSetSize = 0;
    int equivalentGroups = 0;
    int upApproxRemoved = 0;
    int downApproxRemoved = 0;
    int representationAdded = 0;
};

// mmap-filepage 分析单元：按文件（sub_type_id）聚合，不做 symbol 分析（旁路）。
struct FilepageInfo {
    std::string filename;        // data_dict[sub_type_id] 原样
    long long size = 0;          // Σ existing_size
    long long count = 0;         // Σ existing_count
    double percentage = 0.0;     // size / mmap 总大小（anon + filepage）
    std::vector<CompressedCallStack> callStacks;  // 该文件所有调用栈（完整帧，自包含供冰柱图 + 展开 callStacks）
};

// N 轨行模型：一行 = 符号分析单元 | 文件页单元（未来可扩展第 3 种行）
using HtraceRow = std::variant<TopSymbolInfo, FilepageInfo>;

// 一轨 = 一组同类行；key 是写器路由键（"" = 单轨顶层数组；非空 = 多轨嵌套对象的键）
struct HtraceSection {
    std::string key;    // 写器路由键（mmapAnons / mmapFilepages / "" 等）
    std::string label;  // 展示名（HTML 二级 tab 等，未来供写器消费）
    std::vector<HtraceRow> rows;
};

// 一个 scope 的 N 轨聚合结果（轨数运行时可变，不再硬编码 2 轨 / bool 标志）
struct ScopeResult {
    std::vector<HtraceSection> sections;

    bool Empty() const
    {
        for (const auto& s : sections) {
            if (!s.rows.empty()) {
                return false;
            }
        }
        return true;
    }

    // 只统计符号行（filepage 不计入 top-symbols 计数，保持日志与旧行为字节一致）
    size_t SymbolCount() const
    {
        size_t n = 0;
        for (const auto& s : sections) {
            for (const auto& r : s.rows) {
                if (std::holds_alternative<TopSymbolInfo>(r)) {
                    ++n;
                }
            }
        }
        return n;
    }
};

struct TopSymbols {
    std::string mode; // app-only, system-only, app-and-system, mixed
    ScopeResult app;
    ScopeResult system;
    ScopeResult mixed;
};

struct HtraceAnalysisOutput {
    // Analysis output
    long long totalLeakSize = 0;
    TopSymbols topSymbols;
    AlgorithmStats stats;

    // Fields extracted from parse result (needed by formatters)
    std::unordered_map<int, std::string> dataDict;  // for flame/icicle symbol resolution
    std::string mode;                                 // "statistic" or "detail"
    std::string processName;
    std::unordered_map<HtraceEventType, long long> typeSizes;
    TraceMeta traceMeta;

    json ToJson() const;
};

struct AnalysisConfig {
    long long lowThreshold = 1024;
    // 近似判定的相对差值阈值 = min(被取代者 total * appro_rel_ratio/100, appro_rel_cap)
    int approRelRatio = 10;                     // 相对比例（%）：允许 diff 最多为被取代者的 10%
    long long approRelCap = 10LL * 1024 * 1024; // 相对阈值封顶（10MB）
    int approRatioThreshold = 30;               // 贡献比例下限（%）：共同链值需占 counterpart 的 30%
    // 孤儿栈兜底显著度门槛 = min(总泄漏量 * orphan_rel_ratio/100, orphan_rel_cap)
    int orphanRelRatio = 1;                           // 相对比例（%）
    long long orphanRelCap = 10LL * 1024 * 1024;      // 阈值封顶（10MB）
    int topN = 10;
    // 以下两项为运行参数（原先由全局 Config 单例提供，现由调用方显式传入）
    Scope scope = Scope::APP_AND_SYSTEM;    // 分析范围
    std::string category = "nativeheap";  // 资源类型
    // ── -l 分析模式扩展 ──
    bool leakMode = false;            // -l: 跳过 is_app 符号过滤（L3 已分栈，整组符号定性）+ 收集 other
    bool useForceScope = false;     // 用 force_scope 覆盖 config.scope
    Scope forceScope = Scope::APP_AND_SYSTEM;
};

HtraceAnalysisOutput AnalyzeCallStacks(
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict,
    AnalysisConfig config = AnalysisConfig{},
    const std::string& mode = ""
    );

/**
 * Htrace 分析步骤：对解析出的压缩调用栈跑泄漏分析，并把解析期的元信息一并带出。
 * 强类型入参/出参：不再经过 ProcessData / std::any。
 * 会消费 `db` 中的字典与元信息。
 */
HtraceAnalysisOutput HtraceAnalyze(TraceDB& db, const RunContext& ctx);

} // namespace mem_analyzer
