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
#include "jsleak/ref_chain_stage.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {

namespace {

/**
 * 按「最短引用链」聚合一批节点，返回聚合信息列表，并按累计 retained size 降序排列。
 * predicate 决定节点是否入选；exclude_chains 命中的引用链直接跳过。
 * top_k 有值时只保留累计 retained size 最大的前 K 条（两遍扫描：第一遍统计
 * 大小选出 top K，第二遍才收集 nodeIDs，避免为非 top 链保存大量节点 ID）；
 * top_k 为空时单遍聚合全部引用链。
 */
template <typename Pred> std::vector<RefChainInfo> collectRefChains(const HeapSnapshot& heap,
    Pred&& predicate,
    const std::optional<size_t>& topK,
    const std::unordered_set<std::string>& excludeChains)
{
    std::vector<RefChainInfo> result;

    // 关键路径耗时 trace
    StageTimer timer("ref_chain");
    timer.Stage("start node_count=" + std::to_string(heap.nodeCount) +
                " top_k=" + (topK ? std::to_string(*topK) : "all"));

    // exclude 语义：排除「目标节点名」在 exclude_chains 中的链。
    // 原实现按完整链字符串精确匹配；当前唯一用例为 {"SyntheticRoot"}（仅根节点，
    // 其链字符串 == 目标节点简单名），故按简单名匹配等价且无需展开扩展名。
    auto isExcluded = [&excludeChains, &heap](uint32_t uNi) {
        return !excludeChains.empty() &&
               excludeChains.find(heap.GetNodeName(uNi)) != excludeChains.end();
    };

    if (!topK.has_value()) {
        // 单遍：按引用链 128-bit hash 聚合（避免整条链字符串常驻）
        std::unordered_map<RefChainKey, RefChainInfo, RefChainKeyHash> chainMap;
        for (int ni = 0; ni < heap.nodeCount; ++ni) {
            const uint32_t uNi = static_cast<uint32_t>(ni);
            if (!predicate(uNi)) {
                continue;
            }
            const RefChainKey h = heap.RefChainHash(uNi);  // 不可达返回 {0,0}
            if (h == RefChainKey{} || isExcluded(uNi)) {
                continue;
            }
            auto& info = chainMap[h];
            if (info.frames.empty()) {
                info.frames = heap.FormatRefChainFrames(uNi);
            }
            info.nodeIds.push_back(heap.GetNodeId(uNi));
            info.totalRetainedSize += static_cast<uint64_t>(heap.GetRetainedSize(uNi));
        }
        timer.Tick("pass1", "entries=" + std::to_string(chainMap.size()));

        result.reserve(chainMap.size());
        for (auto& kv : chainMap) {
            result.emplace_back(std::move(kv.second));
        }
    } else {
        const size_t k = *topK;

        // 第一遍：统计每条引用链（按 hash）的累计 retained size
        std::unordered_map<RefChainKey, uint64_t, RefChainKeyHash> sizeMap;
        for (int ni = 0; ni < heap.nodeCount; ++ni) {
            const uint32_t uNi = static_cast<uint32_t>(ni);
            if (!predicate(uNi)) {
                continue;
            }
            const RefChainKey h = heap.RefChainHash(uNi);
            if (h == RefChainKey{} || isExcluded(uNi)) {
                continue;
            }
            sizeMap[h] += static_cast<uint64_t>(heap.GetRetainedSize(uNi));
        }
        timer.Tick("pass1", "entries=" + std::to_string(sizeMap.size()));

        // 用最小堆选出累计 retained size 最大的前 K 条（堆顶最小）
        using Pair = std::pair<RefChainKey, uint64_t>;  // (key, retained_size)
        auto cmp = [](const Pair& a, const Pair& b) { return a.second > b.second; };
        std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> minHeap(cmp);
        for (const auto& entry : sizeMap) {
            if (minHeap.size() < k) {
                minHeap.push(entry);
            } else if (!minHeap.empty() && entry.second > minHeap.top().second) {
                minHeap.pop();
                minHeap.push(entry);
            }
        }

        std::vector<Pair> topPairs;
        topPairs.reserve(minHeap.size());
        while (!minHeap.empty()) {
            topPairs.push_back(minHeap.top());
            minHeap.pop();
        }
        if (topPairs.empty()) {
            return result;
        }
        timer.Tick("topK", "K=" + std::to_string(topPairs.size()));

        // 第二遍：只收集 top K 引用链的节点 ID
        std::unordered_map<RefChainKey, size_t, RefChainKeyHash> hashToIndex;
        hashToIndex.reserve(topPairs.size());
        result.reserve(topPairs.size());
        for (const auto& p : topPairs) {
            RefChainInfo info;
            info.totalRetainedSize = p.second;
            result.emplace_back(std::move(info));
            hashToIndex[p.first] = result.size() - 1;
        }
        for (int ni = 0; ni < heap.nodeCount; ++ni) {
            const uint32_t uNi = static_cast<uint32_t>(ni);
            if (!predicate(uNi)) {
                continue;
            }
            const RefChainKey h = heap.RefChainHash(uNi);
            if (h == RefChainKey{} || isExcluded(uNi)) {
                continue;
            }
            auto it = hashToIndex.find(h);
            if (it != hashToIndex.end()) {
                auto& info = result[it->second];
                if (info.frames.empty()) {
                    info.frames = heap.FormatRefChainFrames(uNi);
                }
                info.nodeIds.push_back(heap.GetNodeId(uNi));
            }
        }
        timer.Tick("pass2");
    }

    std::sort(result.begin(), result.end(), [](const RefChainInfo& a, const RefChainInfo& b) {
        return a.totalRetainedSize > b.totalRetainedSize;
    });
    return result;
}

} // namespace

RefChainStage::RefChainStage(std::string mode,
    const char* previewId,
    const char* jsonKey,
    std::optional<std::string> targetClass,
    std::optional<size_t> topK,
    std::unordered_set<std::string> excludeChains)
    : mode_(std::move(mode)),
      previewId(previewId),
      jsonKey(jsonKey),
      targetClass(std::move(targetClass)),
      topK(topK),
      excludeChains(std::move(excludeChains)) {}

const std::string& RefChainStage::Mode() const
{
    return mode_;
}

const char* RefChainStage::PreviewId() const
{
    return previewId;
}

const char* RefChainStage::JsonKey() const
{
    return jsonKey;
}

void RefChainStage::Run(const HeapSnapshot& heap)
{
    refChainInfos = collectRefChains(
        heap,
        [this, &heap](uint32_t uNi) {
            if (!targetClass) {
                return true;
            }
            return heap.GetNodeName(uNi) == *targetClass;
        },
        topK,
        excludeChains);

    if (targetClass) {
        size_t nodeCount = 0;
        for (const auto& chain : refChainInfos) {
            nodeCount += chain.nodeIds.size();
        }
        log::Logok("  Analysis completed. Found ", nodeCount, " ", *targetClass,
            " nodes across ", refChainInfos.size(), " reference chains.");
    } else {
        log::Logok("  Analysis completed. Found ", refChainInfos.size(),
            " top reference chains.");
    }
}

json RefChainStage::ToJson() const
{
    return RefChainsToJson(refChainInfos);
}

} // namespace mem_analyzer