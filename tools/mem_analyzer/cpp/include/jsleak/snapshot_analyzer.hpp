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
#include "core/run_context.hpp"
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include "formatter/json/json_writer.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace mem_analyzer {

struct RefChainInfo {
    std::vector<uint32_t> node_ids;      // 命中该链的目标节点 id 集合（非帧级）
    uint64_t total_retained_size = 0;
    std::vector<std::string> frames;     // 链路径的帧名序列（根 -> 目标）
};

struct PropertyTagObjInfo {
    uint64_t total_retained_size = 0;
    size_t total_node_count = 0;
    std::string property_name;
    std::string concern_type;            // 技术名：命中的目标组件基类名（如 ViewPU），可空
    std::string concern_display_type;    // 展示类型：如 Component / 业务对象
    std::vector<uint32_t> hash_values;
    std::unordered_map<std::string, RefChainInfo> reference_chains;
};

// 共享序列化：属性标签阶段（map 形式 referenceChains）与引用链阶段（vector 形式）共用。
json ref_chain_to_json(const RefChainInfo& chain);
json ref_chains_to_json(const std::vector<RefChainInfo>& chains);
json ref_chains_to_json(const std::unordered_map<std::string, RefChainInfo>& chains);

/// 一个可独立开关的 jsleak 分析阶段。各阶段持有自己的状态与配置，实现
/// run（分析 + 统计打印）与 to_json（产出结果 json）。协调器统一按
/// mode/preview 过滤并调用，阶段本身不感知开关。
struct JsleakStage {
    virtual ~JsleakStage() = default;
    virtual const std::string& mode() const = 0;   // "property-tag" / "top-chain"；"all" 表示仅 -m all 运行
    virtual const char* preview_id() const = 0;    // 非空表示受预览开关管控
    virtual const char* json_key() const = 0;      // 输出到结果 json 的 key
    virtual void run(const HeapSnapshot& heap) = 0;
    virtual json to_json() const = 0;
};

/// jsleak 分析步骤：按 `mode` 与预览开关过滤各阶段，产出结果 json
/// （含 parsedAt / sourceFile / totalHeapSize 与各阶段结果）。
///
/// **注意（P4-d 有意保留）**：分析层仍**提前 JSON 化**——各阶段结果在分析阶段即写成
/// `json`，writer 只做重组。改掉它会动分析层与输出层的数据协议，不在本期范围内。
///
/// 强类型入参/出参：不再经过 ProcessData / std::any。
json jsleak_analyze(const HeapSnapshot& heap, const std::string& file_path, const RunContext& ctx);

} // namespace mem_analyzer
