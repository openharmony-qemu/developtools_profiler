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
#include "htrace/flame_icicle.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cstdio>

namespace mem_analyzer {

json FlameNode::to_flame_json(const std::unordered_map<int, std::string>& data_dict) const {
    json j = {
        {"depth", depth},
        {"size", leak_size},
        {"count", existing_count},
        {"name", resolve_str(symbol_id, data_dict)},
        {"lib", file_id_null ? "unknown" : resolve_str(file_id, data_dict)},
        {"libOffset", resolve_offset_str(vaddr, vaddr_null)},
        {"buildId", (!build_id_null && build_id != 0)
            ? json(data_dict.at(build_id)) : ""}
    };
    if (!children.empty()) {
        json arr = json::array();
        for (const auto& c : children) {
            arr.push_back(c.to_flame_json(data_dict));
        }
        j["callers"] = arr;
    }
    return j;
}

json FlameNode::to_icicle_json(const std::unordered_map<int, std::string>& data_dict) const {
    json j = {
        {"depth", depth},
        {"size", leak_size},
        {"count", existing_count},
        {"name", resolve_str(symbol_id, data_dict)},
        {"lib", file_id_null ? "unknown" : resolve_str(file_id, data_dict)},
        {"libOffset", resolve_offset_str(vaddr, vaddr_null)},
        {"buildId", (!build_id_null && build_id != 0)
            ? json(data_dict.at(build_id)) : ""}
    };
    if (!children.empty()) {
        json arr = json::array();
        for (const auto& c : children) {
            arr.push_back(c.to_icicle_json(data_dict));
        }
        j["callees"] = arr;
    }
    return j;
}

// Insert a path into a prefix trie (O(1) child lookup via int-based key)
// Both leak size and existing count of each chain are accumulated onto
// every node along the path.
static void insert_path(FlameNode& root, std::vector<FlameNode>& path, long long leak, long long count) {
    root.leak_size += leak;
    root.existing_count += count;
    FlameNode* current = &root;

    for (auto& node : path) {
        FlameKey key{node.symbol_id, node.file_id, node.file_id_null, node.depth};

        auto it = current->child_map.find(key);
        if (it != current->child_map.end()) {
            size_t idx = it->second;
            current->children[idx].leak_size += leak;
            current->children[idx].existing_count += count;
            current = &current->children[idx];
        } else {
            size_t idx = current->children.size();
            current->child_map[key] = idx;
            FlameNode new_node = node;
            new_node.leak_size = leak;
            new_node.existing_count = count;
            current->children.push_back(std::move(new_node));
            current = &current->children.back();
        }
    }
}

// 递归清理FlameNode的child_map（构建完成后不再需要，释放冗余内存）
static void clear_child_maps(FlameNode& node) {
    node.child_map.clear();
    for (auto& child : node.children) {
        clear_child_maps(child);
    }
}

FlameNode build_flame_graph(
    const std::string& /*target_symbol*/,
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, int>& depth_per_chain,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};

    for (int ci : chain_ids) {
        if (ci < 0 || ci >= (int)call_stacks.size()) continue;
        auto& chain = call_stacks[ci];
        auto depth_it = depth_per_chain.find(ci);
        if (depth_it == depth_per_chain.end()) continue;
        int target_depth = depth_it->second;
        long long leak = chain.existing_size;
        if (leak <= 0) continue;

        // Collect frames above target_depth (store int IDs, no string resolution)
        std::vector<FlameNode> path;
        for (auto& frame : chain.frames) {
            int fd = frame.depth;
            if (fd >= target_depth) continue;
            path.push_back({frame.symbol_id, frame.file_id, frame.file_id_null,
                           frame.vaddr, frame.vaddr_null, frame.build_id, frame.build_id_null,
                           fd - target_depth, 0, 0, {}});
        }
        std::reverse(path.begin(), path.end());
        insert_path(root, path, leak, chain.existing_count);
    }
    clear_child_maps(root);  // 构建完成，释放child_map冗余内存
    return root;
}

FlameNode build_icicle_graph(
    const std::string& /*target_symbol*/,
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, int>& depth_per_chain,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};

    for (int ci : chain_ids) {
        if (ci < 0 || ci >= (int)call_stacks.size()) continue;
        auto& chain = call_stacks[ci];
        auto depth_it = depth_per_chain.find(ci);
        if (depth_it == depth_per_chain.end()) continue;
        int target_depth = depth_it->second;
        long long leak = chain.existing_size;
        if (leak <= 0) continue;

        std::vector<FlameNode> path;
        for (auto& frame : chain.frames) {
            int fd = frame.depth;
            if (fd <= target_depth) continue;
            path.push_back({frame.symbol_id, frame.file_id, frame.file_id_null,
                           frame.vaddr, frame.vaddr_null, frame.build_id, frame.build_id_null,
                           fd - target_depth, 0, 0, {}});
        }
        insert_path(root, path, leak, chain.existing_count);
    }
    clear_child_maps(root);  // 构建完成，释放child_map冗余内存
    return root;
}

FlameNode build_merged_icicle(
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};
    for (int ci : chain_ids) {
        if (ci < 0 || ci >= (int)call_stacks.size()) continue;
        auto& chain = call_stacks[ci];
        long long leak = chain.existing_size;
        if (leak <= 0) continue;
        // 整栈帧序列（caller→callee 正序），insert 成 root→caller→callee；
        // icicle 渲染时 root 在顶、caller 在上、callee 在下，与原 flame 栈方向一致
        std::vector<FlameNode> path;
        path.reserve(chain.frames.size());
        for (auto it = chain.frames.begin(); it != chain.frames.end(); ++it) {
            path.push_back({it->symbol_id, it->file_id, it->file_id_null,
                            it->vaddr, it->vaddr_null, it->build_id, it->build_id_null,
                            it->depth, 0, 0, {}});
        }
        insert_path(root, path, leak, chain.existing_count);
    }
    clear_child_maps(root);
    return root;
}

} // namespace mem_analyzer
