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

json FlameNode::ToFlameJson(const std::unordered_map<int, std::string>& dataDict) const
{
    json j = {
        {"depth", depth},
        {"size", leakSize},
        {"count", existingCount},
        {"name", ResolveStr(symbolId, dataDict)},
        {"lib", fileIdNull ? "unknown" : ResolveStr(fileId, dataDict)},
        {"libOffset", ResolveOffsetStr(vaddr, vaddrNull)},
        {"buildId", (!buildIdNull && buildId != 0) ?
            json(dataDict.at(buildId)) : ""}
    };
    if (!children.empty()) {
        json arr = json::array();
        for (const auto& c : children) {
            arr.push_back(c.ToFlameJson(dataDict));
        }
        j["callers"] = arr;
    }
    return j;
}

json FlameNode::ToIcicleJson(const std::unordered_map<int, std::string>& dataDict) const
{
    json j = {
        {"depth", depth},
        {"size", leakSize},
        {"count", existingCount},
        {"name", ResolveStr(symbolId, dataDict)},
        {"lib", fileIdNull ? "unknown" : ResolveStr(fileId, dataDict)},
        {"libOffset", ResolveOffsetStr(vaddr, vaddrNull)},
        {"buildId", (!buildIdNull && buildId != 0) ?
            json(dataDict.at(buildId)) : ""}
    };
    if (!children.empty()) {
        json arr = json::array();
        for (const auto& c : children) {
            arr.push_back(c.ToIcicleJson(dataDict));
        }
        j["callees"] = arr;
    }
    return j;
}

// Insert a path into a prefix trie (O(1) child lookup via int-based key)
// Both leak size and existing count of each chain are accumulated onto
// every node along the path.
static void InsertPath(FlameNode& root, std::vector<FlameNode>& path, long long leak, long long count)
{
    root.leakSize += leak;
    root.existingCount += count;
    FlameNode* current = &root;

    for (auto& node : path) {
        FlameKey key{node.symbolId, node.fileId, node.fileIdNull, node.depth};

        auto it = current->childMap.find(key);
        if (it != current->childMap.end()) {
            size_t idx = it->second;
            current->children[idx].leakSize += leak;
            current->children[idx].existingCount += count;
            current = &current->children[idx];
        } else {
            size_t idx = current->children.size();
            current->childMap[key] = idx;
            FlameNode newNode = node;
            newNode.leakSize = leak;
            newNode.existingCount = count;
            current->children.push_back(std::move(newNode));
            current = &current->children.back();
        }
    }
}

// 递归清理FlameNode的child_map（构建完成后不再需要，释放冗余内存）
static void ClearChildMaps(FlameNode& node)
{
    node.childMap.clear();
    for (auto& child : node.children) {
        ClearChildMaps(child);
    }
}

FlameNode BuildFlameGraph(
    const std::string& /*target_symbol*/,
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, int>& depthPerChain,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};

    for (int ci : chainIds) {
        if (ci < 0 || ci >= static_cast<int>(callStacks.size())) {
            continue;
        }
        auto& chain = callStacks[ci];
        auto depthIt = depthPerChain.find(ci);
        if (depthIt == depthPerChain.end()) {
            continue;
        }
        int targetDepth = depthIt->second;
        long long leak = chain.existingSize;
        if (leak <= 0) {
            continue;
        }

        // Collect frames above target_depth (store int IDs, no string resolution)
        std::vector<FlameNode> path;
        for (auto& frame : chain.frames) {
            int fd = frame.depth;
            if (fd >= targetDepth) {
                continue;
            }
            path.push_back({frame.symbolId, frame.fileId, frame.fileIdNull,
                frame.vaddr, frame.vaddrNull, frame.buildId, frame.buildIdNull,
                fd - targetDepth, 0, 0, {}});
        }
        std::reverse(path.begin(), path.end());
        InsertPath(root, path, leak, chain.existingCount);
    }
    ClearChildMaps(root);  // 构建完成，释放child_map冗余内存
    return root;
}

FlameNode BuildIcicleGraph(
    const std::string& /*target_symbol*/,
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, int>& depthPerChain,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};

    for (int ci : chainIds) {
        if (ci < 0 || ci >= static_cast<int>(callStacks.size())) {
            continue;
        }
        auto& chain = callStacks[ci];
        auto depthIt = depthPerChain.find(ci);
        if (depthIt == depthPerChain.end()) {
            continue;
        }
        int targetDepth = depthIt->second;
        long long leak = chain.existingSize;
        if (leak <= 0) {
            continue;
        }

        std::vector<FlameNode> path;
        for (auto& frame : chain.frames) {
            int fd = frame.depth;
            if (fd <= targetDepth) {
                continue;
            }
            path.push_back({frame.symbolId, frame.fileId, frame.fileIdNull,
                frame.vaddr, frame.vaddrNull, frame.buildId, frame.buildIdNull,
                fd - targetDepth, 0, 0, {}});
        }
        InsertPath(root, path, leak, chain.existingCount);
    }
    ClearChildMaps(root);  // 构建完成，释放child_map冗余内存
    return root;
}

FlameNode BuildMergedIcicle(
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<int>& chainIds,
    const std::unordered_map<int, std::string>& /*data_dict*/)
{
    FlameNode root{};
    for (int ci : chainIds) {
        if (ci < 0 || ci >= static_cast<int>(callStacks.size())) {
            continue;
        }
        auto& chain = callStacks[ci];
        long long leak = chain.existingSize;
        if (leak <= 0) {
            continue;
        }
        // 整栈帧序列（caller→callee 正序），insert 成 root→caller→callee；
        // icicle 渲染时 root 在顶、caller 在上、callee 在下，与原 flame 栈方向一致
        std::vector<FlameNode> path;
        path.reserve(chain.frames.size());
        for (auto it = chain.frames.begin(); it != chain.frames.end(); ++it) {
            path.push_back({it->symbolId, it->fileId, it->fileIdNull,
                            it->vaddr, it->vaddrNull, it->buildId, it->buildIdNull,
                            it->depth, 0, 0, {}});
        }
        InsertPath(root, path, leak, chain.existingCount);
    }
    ClearChildMaps(root);
    return root;
}

} // namespace mem_analyzer
