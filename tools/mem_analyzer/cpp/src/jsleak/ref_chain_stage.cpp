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

/// 按「最短引用链」聚合一批节点，返回聚合信息列表，并按累计 retained size 降序排列。
///
/// predicate 决定节点是否入选；exclude_chains 命中的引用链直接跳过。
/// top_k 有值时只保留累计 retained size 最大的前 K 条（两遍扫描：第一遍统计
/// 大小选出 top K，第二遍才收集 nodeIDs，避免为非 top 链保存大量节点 ID）；
/// top_k 为空时单遍聚合全部引用链。
template <typename Pred>
std::vector<RefChainInfo>
collect_ref_chains(const HeapSnapshot& heap,
                   Pred&& predicate,
                   const std::optional<size_t>& top_k,
                   const std::unordered_set<std::string>& exclude_chains) {
    std::vector<RefChainInfo> result;

    // 关键路径耗时 trace
    StageTimer timer("ref_chain");
    timer.stage("start node_count=" + std::to_string(heap.node_count) +
                " top_k=" + (top_k ? std::to_string(*top_k) : "all"));

    // exclude 语义：排除「目标节点名」在 exclude_chains 中的链。
    // 原实现按完整链字符串精确匹配；当前唯一用例为 {"SyntheticRoot"}（仅根节点，
    // 其链字符串 == 目标节点简单名），故按简单名匹配等价且无需展开扩展名。
    auto is_excluded = [&](uint32_t u_ni) {
        return !exclude_chains.empty() &&
               exclude_chains.find(heap.get_node_name(u_ni)) != exclude_chains.end();
    };

    if (!top_k.has_value()) {
        // 单遍：按引用链 128-bit hash 聚合（避免整条链字符串常驻）
        std::unordered_map<RefChainKey, RefChainInfo, RefChainKeyHash> chain_map;
        for (int ni = 0; ni < heap.node_count; ++ni) {
            const uint32_t u_ni = static_cast<uint32_t>(ni);
            if (!predicate(u_ni)) continue;
            const RefChainKey h = heap.ref_chain_hash(u_ni);  // 不可达返回 {0,0}
            if (h == RefChainKey{} || is_excluded(u_ni)) continue;
            auto& info = chain_map[h];
            if (info.frames.empty()) {
                info.frames = heap.format_ref_chain_frames(u_ni);
            }
            info.node_ids.push_back(heap.get_node_id(u_ni));
            info.total_retained_size += static_cast<uint64_t>(heap.get_retained_size(u_ni));
        }
        timer.tick("pass1", "entries=" + std::to_string(chain_map.size()));

        result.reserve(chain_map.size());
        for (auto& kv : chain_map) {
            result.emplace_back(std::move(kv.second));
        }
    } else {
        const size_t K = *top_k;

        // 第一遍：统计每条引用链（按 hash）的累计 retained size
        std::unordered_map<RefChainKey, uint64_t, RefChainKeyHash> size_map;
        for (int ni = 0; ni < heap.node_count; ++ni) {
            const uint32_t u_ni = static_cast<uint32_t>(ni);
            if (!predicate(u_ni)) continue;
            const RefChainKey h = heap.ref_chain_hash(u_ni);
            if (h == RefChainKey{} || is_excluded(u_ni)) continue;
            size_map[h] += static_cast<uint64_t>(heap.get_retained_size(u_ni));
        }
        timer.tick("pass1", "entries=" + std::to_string(size_map.size()));

        // 用最小堆选出累计 retained size 最大的前 K 条（堆顶最小）
        using Pair = std::pair<RefChainKey, uint64_t>;  // (key, retained_size)
        auto cmp = [](const Pair& a, const Pair& b) { return a.second > b.second; };
        std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> min_heap(cmp);
        for (const auto& entry : size_map) {
            if (min_heap.size() < K) {
                min_heap.push(entry);
            } else if (!min_heap.empty() && entry.second > min_heap.top().second) {
                min_heap.pop();
                min_heap.push(entry);
            }
        }

        std::vector<Pair> top_pairs;
        top_pairs.reserve(min_heap.size());
        while (!min_heap.empty()) {
            top_pairs.push_back(min_heap.top());
            min_heap.pop();
        }
        if (top_pairs.empty()) return result;
        timer.tick("topK", "K=" + std::to_string(top_pairs.size()));

        // 第二遍：只收集 top K 引用链的节点 ID
        std::unordered_map<RefChainKey, size_t, RefChainKeyHash> hash_to_index;
        hash_to_index.reserve(top_pairs.size());
        result.reserve(top_pairs.size());
        for (const auto& p : top_pairs) {
            RefChainInfo info;
            info.total_retained_size = p.second;
            result.emplace_back(std::move(info));
            hash_to_index[p.first] = result.size() - 1;
        }
        for (int ni = 0; ni < heap.node_count; ++ni) {
            const uint32_t u_ni = static_cast<uint32_t>(ni);
            if (!predicate(u_ni)) continue;
            const RefChainKey h = heap.ref_chain_hash(u_ni);
            if (h == RefChainKey{} || is_excluded(u_ni)) continue;
            auto it = hash_to_index.find(h);
            if (it != hash_to_index.end()) {
                auto& info = result[it->second];
                if (info.frames.empty()) {
                    info.frames = heap.format_ref_chain_frames(u_ni);
                }
                info.node_ids.push_back(heap.get_node_id(u_ni));
            }
        }
        timer.tick("pass2");
    }

    std::sort(result.begin(), result.end(), [](const RefChainInfo& a, const RefChainInfo& b) {
        return a.total_retained_size > b.total_retained_size;
    });
    return result;
}

} // namespace

RefChainStage::RefChainStage(std::string mode,
                             const char* preview_id,
                             const char* json_key,
                             std::optional<std::string> target_class,
                             std::optional<size_t> top_k,
                             std::unordered_set<std::string> exclude_chains)
    : mode_(std::move(mode)),
      preview_id_(preview_id),
      json_key_(json_key),
      target_class_(std::move(target_class)),
      top_k_(top_k),
      exclude_chains_(std::move(exclude_chains)) {}

const std::string& RefChainStage::mode() const {
    return mode_;
}

const char* RefChainStage::preview_id() const {
    return preview_id_;
}

const char* RefChainStage::json_key() const {
    return json_key_;
}

void RefChainStage::run(const HeapSnapshot& heap) {
    ref_chain_infos_ = collect_ref_chains(
        heap,
        [&](uint32_t u_ni) {
            if (!target_class_) return true;
            return heap.get_node_name(u_ni) == *target_class_;
        },
        top_k_,
        exclude_chains_);

    if (target_class_) {
        size_t node_count = 0;
        for (const auto& chain : ref_chain_infos_) {
            node_count += chain.node_ids.size();
        }
        log::logok("  Analysis completed. Found ", node_count, " ", *target_class_,
                   " nodes across ", ref_chain_infos_.size(), " reference chains.");
    } else {
        log::logok("  Analysis completed. Found ", ref_chain_infos_.size(),
                   " top reference chains.");
    }
}

json RefChainStage::to_json() const {
    return ref_chains_to_json(ref_chain_infos_);
}

} // namespace mem_analyzer