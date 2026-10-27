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
#include <memory>

#include "shared/htrace_model/htrace_data.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

namespace mem_analyzer {

// Key for O(1) child lookup in flame/icicle trie (int-based, no string memory)
struct FlameKey {
    int symbol_id = 0;
    int file_id = 0;
    bool file_id_null = false;
    int depth = 0;

    bool operator==(const FlameKey& o) const {
        return symbol_id == o.symbol_id && file_id == o.file_id &&
               file_id_null == o.file_id_null && depth == o.depth;
    }
};

struct FlameKeyHash {
    size_t operator()(const FlameKey& k) const {
        size_t h = std::hash<int>{}(k.symbol_id);
        h ^= std::hash<int>{}(k.file_id) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<bool>{}(k.file_id_null) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(k.depth) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct FlameNode {
    int symbol_id = 0;
    int file_id = 0;
    bool file_id_null = false;
    uint64_t vaddr = 0;
    bool vaddr_null = false;
    int build_id = 0;
    bool build_id_null = true;
    int depth = 0;           // relative depth: negative for flame (up), positive for icicle (down)
    long long leak_size = 0;
    long long existing_count = 0;   // accumulated existing count of allocations passing through this node
    std::vector<FlameNode> children;
    // O(1) child lookup keyed by (symbol_id, file_id, file_id_null, depth)
    std::unordered_map<FlameKey, size_t, FlameKeyHash> child_map;

    // Serialization: resolves int IDs to strings via data_dict at output time
    json to_flame_json(const std::unordered_map<int, std::string>& data_dict) const;
    json to_icicle_json(const std::unordered_map<int, std::string>& data_dict) const;
};

FlameNode build_flame_graph(
    const std::string& target_symbol,
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, int>& depth_per_chain,
    const std::unordered_map<int, std::string>& data_dict
);

FlameNode build_icicle_graph(
    const std::string& target_symbol,
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, int>& depth_per_chain,
    const std::unordered_map<int, std::string>& data_dict
);

// 整栈合并火焰图（无根符号，所有栈整帧序列合并到一个 trie）；用于 other
FlameNode build_merged_icicle(
    const std::vector<CompressedCallStack>& call_stacks,
    const std::vector<int>& chain_ids,
    const std::unordered_map<int, std::string>& data_dict
);

// Helper: resolve int IDs to strings (used by FlameNode serialization and HTML formatter)
inline const std::string& resolve_str(int id, const std::unordered_map<int, std::string>& data_dict) {
    static const std::string unknown = "unknown";
    auto it = data_dict.find(id);
    return (it != data_dict.end()) ? it->second : unknown;
}

inline std::string resolve_offset_str(uint64_t vaddr, bool vaddr_null) {
    if (vaddr_null) return "";
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)vaddr);
    return buf;
}

} // namespace mem_analyzer
