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
#include "jsleak/snapshot_analyzer.hpp"
#include "jsleak/property_tag_stage.hpp"
#include "jsleak/ref_chain_stage.hpp"
#include "config_mgr/preview_features_mgr.hpp"
#include <chrono>
#include <memory>
#include <unordered_set>

namespace mem_analyzer {

// ===== 共享序列化 =====

json ref_chain_to_json(const RefChainInfo& chain) {
    nlohmann::ordered_json item;
    item["chain"] = chain.frames;         // 帧名数组（根 -> 目标）
    item["nodeIds"] = chain.node_ids;
    item["totalRetainedSize"] = chain.total_retained_size;
    return item;
}

/// 把「引用链 → 聚合信息」列表序列化为数组，元素含 chain/nodeIds/totalRetainedSize，
/// 并保持传入顺序。
json ref_chains_to_json(const std::vector<RefChainInfo>& chains) {
    nlohmann::ordered_json arr = nlohmann::ordered_json::array();
    for (const auto& chain : chains) {
        arr.push_back(ref_chain_to_json(chain));
    }
    return arr;
}

/// 同上，输入为以引用链名称为 key 的 map（propertyTagObjects 的 referenceChains）。
json ref_chains_to_json(const std::unordered_map<std::string, RefChainInfo>& chains) {
    nlohmann::ordered_json arr = nlohmann::ordered_json::array();
    for (const auto& [name, chain] : chains) {
        arr.push_back(ref_chain_to_json(chain));
    }
    return arr;
}

// ===== 分析器 =====

json jsleak_analyze(const HeapSnapshot& heap, const std::string& file_path, const RunContext& ctx) {
    const std::string& mode = ctx.snapshot.mode;

    // 阶段按「结果 json 输出顺序」组装。
    std::vector<std::unique_ptr<JsleakStage>> stages;
    stages.push_back(std::make_unique<PropertyTagStage>(ctx.snapshot.leak_list_file));

    // 预览阶段：mode 填 "all" 表示仅在 -m all 时运行，不与 property-tag 绑定。
    stages.push_back(std::make_unique<RefChainStage>(
        "all", "napi_wrap", "napiWrapRefChains",
        std::string("JSWrappedNapiObject"), std::nullopt,
        std::unordered_set<std::string>{}));

    // top_n 为负时按 size_t 溢出为极大值，等价于不限制条数（保持原行为）。
    stages.push_back(std::make_unique<RefChainStage>(
        "top-chain", nullptr, "topRefChains",
        std::nullopt, static_cast<size_t>(ctx.snapshot.top_n),
        std::unordered_set<std::string>{"SyntheticRoot"}));

    // json 由各阶段在分析完成后依次写入（快照数据本身即为 map 结构）
    nlohmann::ordered_json data;
    data["parsedAt"] = format_readable_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    data["sourceFile"] = file_path;
    data["totalHeapSize"] = heap.get_total_heap_size();

    for (auto& stage : stages) {
        // 唯一过滤点：① 预览特性未使能则跳过；② 分析模式不匹配则跳过。
        if (stage->preview_id() &&
            !PreviewFeaturesMgr::instance().enabled(stage->preview_id())) {
            continue;
        }
        if (mode != "all" && mode != stage->mode()) continue;
        stage->run(heap);
        data[stage->json_key()] = stage->to_json();
    }

    return data;
}

} // namespace mem_analyzer
