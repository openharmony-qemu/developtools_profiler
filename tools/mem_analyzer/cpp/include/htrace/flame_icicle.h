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

#include "shared/htrace_model/htrace_data.h"
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

namespace mem_analyzer {

// Key for O(1) child lookup in flame/icicle trie (int-based, no string memory)
struct FlameKey {
    int symbolId = 0;
    int fileId = 0;
    bool fileIdNull = false;
    int depth = 0;

    bool operator==(const FlameKey& o) const
    {
        return symbolId == o.symbolId && fileId == o.fileId &&
               fileIdNull == o.fileIdNull && depth == o.depth;
    }
};

constexpr int HASH_COMBINE_LSHIFT = 6;
constexpr int HASH_COMBINE_RSHIFT = 2;

struct FlameKeyHash {
    size_t operator()(const FlameKey& k) const
    {
        size_t h = std::hash<int>{}(k.symbolId);
        h ^= std::hash<int>{}(k.fileId) + 0x9e3779b9 + (h << HASH_COMBINE_LSHIFT) + (h >> HASH_COMBINE_RSHIFT);
        h ^= std::hash<bool>{}(k.fileIdNull) + 0x9e3779b9 + (h << HASH_COMBINE_LSHIFT) + (h >> HASH_COMBINE_RSHIFT);
        h ^= std::hash<int>{}(k.depth) + 0x9e3779b9 + (h << HASH_COMBINE_LSHIFT) + (h >> HASH_COMBINE_RSHIFT);
        return h;
    }
};

struct FlameNode {
    int symbolId = 0;
    int fileId = 0;
    bool fileIdNull = false;
    uint64_t vaddr = 0;
    bool vaddrNull = false;
    int buildId = 0;
    bool buildIdNull = true;
    int depth = 0;           // relative depth: negative for flame (up), positive for icicle (down)
    long long leakSize = 0;
    long long existingCount = 0;   // accumulated existing count of allocations passing through this node
    std::vector<FlameNode> children;
    // O(1) child lookup keyed by (symbol_id, file_id, file_id_null, depth)
    std::unordered_map<FlameKey, size_t, FlameKeyHash> childMap;

    // Serialization: resolves int IDs to strings via data_dict at output time
    json ToFlameJson(const std::unordered_map<int, std::string>& dataDict) const;
    json ToIcicleJson(const std::unordered_map<int, std::string>& dataDict) const;
};

FlameNode BuildFlameGraph(
    const std::string& targetSymbol,
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, int>& depthPerChain,
    const std::unordered_map<int, std::string>& dataDict
    );

FlameNode BuildIcicleGraph(
    const std::string& targetSymbol,
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, int>& depthPerChain,
    const std::unordered_map<int, std::string>& dataDict
    );

// 整栈合并火焰图（无根符号，所有栈整帧序列合并到一个 trie）；用于 other
FlameNode BuildMergedIcicle(
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, std::string>& dataDict
    );

// Helper: resolve int IDs to strings (used by FlameNode serialization and HTML formatter)
inline const std::string& ResolveStr(int id, const std::unordered_map<int, std::string>& dataDict)
{
    static const std::string unknown = "unknown";
    auto it = dataDict.find(id);
    return (it != dataDict.end()) ? it->second : unknown;
}

inline std::string ResolveOffsetStr(uint64_t vaddr, bool vaddrNull)
{
    if (vaddrNull) {
        return "";
    }
    char buf[32];
    static_cast<void>(snprintf(buf, sizeof(buf), "0x%llx", static_cast<unsigned long long>(vaddr)));
    return buf;
}

} // namespace mem_analyzer
