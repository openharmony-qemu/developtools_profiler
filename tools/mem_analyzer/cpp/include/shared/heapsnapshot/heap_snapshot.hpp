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
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace mem_analyzer {

// Forward declarations
struct RetainedSizeResult;

/// 引用链聚合的分组 key：128-bit 指纹（lo/hi 两个 64-bit lane）。
struct RefChainKey {
    uint64_t lo = 0;
    uint64_t hi = 0;
    bool operator==(const RefChainKey& o) const {
        return lo == o.lo && hi == o.hi;
    }
};

/// 引用链聚合 key 的桶哈希（把 128-bit 混合成单个 size_t bucket 索引）。
struct RefChainKeyHash {
    std::size_t operator()(const RefChainKey& k) const
    {
        uint64_t x = k.lo ^ (k.hi * 0x9E3779B97F4A7C15ULL);
        x ^= x >> 30;
        x *= 0xBF58476D1CE4E5B9ULL;
        x ^= x >> 27;
        x *= 0x94D049BB133111EBULL;
        x ^= x >> 31;
        return static_cast<std::size_t>(x);
    }
};

/**
 * V8 Heap Snapshot parser.
 * Parses .heapsnapshot files (JSON format) without loading the full file.
 * Uses chunked reading + manual numeric array parsing for large arrays.
 */
class HeapSnapshot {
public:
    // once_flag 不可拷贝/移动，故用 unique_ptr 持有，保持 HeapSnapshot 可移动
    //（from_file 需按值返回）。
    HeapSnapshot() : extNamesOnce_(std::make_unique<std::once_flag>()),
        chainHashOnce_(std::make_unique<std::once_flag>()) {}

    /// Node field offsets (discovered from meta.node_fields)
    int nodeTypeOffset = -1;        // type field index within node record
    int nodeNameOffset = -1;        // name field index (string table index)
    int nodeIdOffset = -1;          // id field index
    int nodeSelfSizeOffset = -1;   // self_size field index
    int nodeNativeSizeOffset = -1; // native_size field index (V8 heap snapshots)
    int nodeEdgeCountOffset = -1;  // edge_count field index
    int nodeFieldsCount = 0;        // total fields per node
    int edgeFieldsCount = 0;        // total fields per edge
    int edgeTypeOffset = -1;        // type field index within edge record
    int edgeToNodeOffset = -1;     // to_node field index
    int edgeNameOrIndexOffset = -1; // name_or_index field index (the remaining edge field)
    int edgeWeakType = -1;          // numeric type value for "weak" edges, or -1
    int edgeShortcutType = -1;       // numeric type value for "shortcut" edges, or -1
    int edgeHiddenType = -1;         // numeric type value for "hidden" edges, or -1
    int edgeElementType = -1;        // numeric type value for "element" edges, or -1
    int edgeInternalType = -1;       // numeric type value for "internal" edges, or -1

    // Raw data
    std::vector<uint32_t> nodes;               // flat node field array
    std::vector<uint32_t> edges;               // flat edge field array
    std::vector<uint32_t> firstEdgePointers;  // CSR: for node i, edges start at first_edge_pointers[i]
    std::vector<std::string> strings;           // string table
    std::vector<std::string> nodeTypeNames;    // type names from meta.node_types (e.g. "object", "string")

    int nodeCount = 0;
    int edgeCount = 0;

    /// nodeId → nodeIndex lookup
    std::unordered_map<uint32_t, uint32_t> nodeIdToIdx;

    /// SyntheticRoot 节点索引缓存（from_file 阶段一次性定位），
    /// 使 get_total_heap_size 由 O(N) 扫描降为 O(1)。UINT32_MAX = 未找到。
    uint32_t syntheticRootIndex = UINT32_MAX;

    // ========================================================================
    //  Computed results (populated automatically during from_file)
    //  Populated during from_file.
    // ========================================================================

    /// Retained sizes per node index: self_size + dominated descendants' retained sizes
    /// (self_size only; native_size is NOT added — see node_size_calculator.cpp).
    std::vector<double> retainedSizes;

    /// Immediate dominator per node index (UINT32_MAX = unreachable).
    std::vector<uint32_t> dominators;

    /// Shortest-path parent node index (UINT32_MAX = root or unreachable).
    std::vector<uint32_t> pathParentIndex;

    /// Shortest-path incoming edge field start (UINT32_MAX = root).
    std::vector<uint32_t> pathEdgeStart;

    /// 1 if reachable from GC root.
    std::vector<uint8_t> reachable;

    /// Get retained size for a node (dominator-based; self_size only, native_size excluded).
    double GetRetainedSize(uint32_t nodeIndex) const
    {
        if (nodeIndex < retainedSizes.size()) {
            return retainedSizes[nodeIndex];
        }
        return 0.0;
    }

    /// Get the shallow/self size stored on a node.
    uint32_t GetNodeSelfSize(uint32_t nodeIndex) const
    {
        if (nodeSelfSizeOffset < 0 || nodeIndex >= static_cast<uint32_t>(nodeCount)) {
            return 0;
        }
        return nodes[static_cast<size_t>(nodeIndex) * static_cast<size_t>(nodeFieldsCount) +
                     static_cast<size_t>(nodeSelfSizeOffset)];
    }

    /// Get numeric id for a node.
    uint32_t GetNodeId(uint32_t nodeIndex) const
    {
        if (nodeIdOffset < 0 || nodeIndex >= static_cast<uint32_t>(nodeCount)) {
            return UINT32_MAX;
        }
        return nodes[static_cast<size_t>(nodeIndex) * static_cast<size_t>(nodeFieldsCount) +
                     static_cast<size_t>(nodeIdOffset)];
    }

    /// Format shortest reference chain as "RootNode -> NodeA -> TargetNode"
    /// matching TS's getSimpleRefChain() format.
    std::string FormatRefChain(uint32_t nodeIndex) const;

    /// Return the shortest reference chain frame names (root -> target) as a list.
    /// Shares the same path collection as format_ref_chain, but emits structured
    /// frame names instead of a joined " -> " string.
    std::vector<std::string> FormatRefChainFrames(uint32_t nodeIndex) const;

    /// 计算最短引用链的 128-bit 稳定标识（对 target->root 的扩展名序列做双路
    /// FNV hash），不可达返回 {0,0}。作为引用链聚合的分组 key。
    /// 注意：必须 hash 扩展名（名字/类型）序列，而非 node_index（具体对象）序列，
    /// 否则会把「走同一类型路径的多个对象」错误拆成多条单节点链。
    RefChainKey RefChainHash(uint32_t nodeIndex) const;

    /// Result of getNodeById
    struct NodeView {
        uint32_t nodeIndex;
        uint32_t id;
        std::string name;
        uint32_t selfSize;
    };

    /// Lookup a node by its numeric id
    NodeView GetNodeById(uint32_t id) const;

    /// Get node name from strings table
    std::string GetNodeName(uint32_t nodeIndex) const;

    // Get ArkInternalHash value from node's edge (returns 0 if not found)
    uint32_t GetNodeInternalHash(uint32_t nodeIndex) const;

    /// Build HeapSnapshot from a .heapsnapshot file
    static HeapSnapshot FromFile(const std::string& path);

    /// Get type name for node at node_index (e.g. "object", "string", "number")
    std::string GetNodeTypeName(uint32_t nodeIndex) const;

    /// Number of outgoing edges for a given node
    uint32_t GetNodeEdgeCount(uint32_t nodeIndex) const;

    // 获取节点的扩展名称
    const std::string& GetExtendedNodeName(uint32_t nodeIndex) const;

    /// Per-edge info exposed via for_each_edge callback
    struct EdgeInfo {
        uint32_t type;              // numeric type index (matches edge_types meta)
        uint32_t nameOrIndex;     // edge name (string table index) or numeric index
        uint32_t toNode;           // target node index (edges[to_node_off] / node_fields_count)
        uint32_t toNodeId;        // target node id (decoded from to_node)
    };

    /// Iterate outgoing edges of node at node_index.
    /// Callback receives (const EdgeInfo&, size_t local_edge_index).
    template<typename Fn>
    void ForEachEdge(uint32_t nodeIndex, Fn&& fn) const
    {
        size_t begin = firstEdgePointers[nodeIndex];
        size_t end = firstEdgePointers[static_cast<size_t>(nodeIndex) + 1];
        const size_t edgesSize = edges.size();
        const size_t nodesSize = nodes.size();
        size_t localIdx = 0;
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(edgeFieldsCount)) {
            if (ptr + static_cast<size_t>(edgeFieldsCount) > edgesSize) {
                break;
            }
            EdgeInfo edge;
            edge.type = edges[ptr + static_cast<size_t>(edgeTypeOffset)];
            edge.nameOrIndex = edges[ptr + static_cast<size_t>(edgeNameOrIndexOffset)];
            edge.toNode = edges[ptr + static_cast<size_t>(edgeToNodeOffset)]
                           / static_cast<uint32_t>(nodeFieldsCount);
            if (edge.toNode < static_cast<uint32_t>(nodeCount) &&
                edge.toNode * static_cast<uint32_t>(nodeFieldsCount) + static_cast<size_t>(nodeIdOffset) < nodesSize) {
                edge.toNodeId = nodes[edge.toNode * static_cast<uint32_t>(nodeFieldsCount) +
                                        static_cast<size_t>(nodeIdOffset)];
                fn(edge, localIdx);
            }
            localIdx++;
        }
    }

    uint64_t GetTotalHeapSize() const
    {
        if (syntheticRootIndex == UINT32_MAX) {
            return 0;
        }
        return static_cast<uint64_t>(GetRetainedSize(syntheticRootIndex));
    }

private:
    // 获取 JSObject 扩展名
    std::string GetJsObjectExtName(uint32_t nodeIndex) const;

    // 获取 String 扩展名
    std::string GetStringExtName(uint32_t nodeIndex) const;

    // 获取 stateMgmt.js#observedClass 全局对象的扩展名（沿原型链提取 ArkTS 类名）
    std::string GetObservedClassExtName(uint32_t nodeIndex) const;

    // 沿 hclass → Proto 交替上溯，提取 stateMgmt 包装对象背后被包装的类名。
    // skip_markers 中的名字视为「仍是包装层」，跳过并继续上溯。返回空 = 未命中。
    std::string ResolveStateMgmtWrappedClass(uint32_t nodeIndex, const std::vector<std::string>& skipMarkers) const;

    // 获取 stateMgmt.js#JSObject 的扩展名（同 observedClass 的套路，展开被包装的 ArkTS 类）
    std::string GetStateMgmtJsObjectExtName(uint32_t nodeIndex, const std::string& nodeName) const;

    // declarative_frontend/stateMgmt.js 对象扩展名的统一入口
    // （observedClass 展开 / JSObject 展开 / 其余去前缀）
    std::string GetStateMgmtExtName(uint32_t nodeIndex, const std::string& nodeName) const;

    // 获取 SourceTextModule 扩展名
    std::string GetSourceTextModuleExtName(uint32_t nodeIndex) const;

    // 计算单个节点的扩展名（不查缓存、不递归调用 get_extended_node_name）。
    // 由 get_extended_node_name 在预构建阶段串行调用。
    std::string ComputeExtendedName(uint32_t nodeIndex) const;

    // 一次性预构建的扩展名表（按 node_index 索引）。并行阶段只读、无锁。
    // once_flag 用 unique_ptr 持有（once_flag 不可拷贝/移动）。
    mutable std::unique_ptr<std::once_flag> extNamesOnce_;
    mutable std::vector<std::string> extNames_;

    // 一次性预构建的「正向链 hash」表（按 node_index 索引，存 root→node 的扩展名序列
    // 的 FNV 指纹）。ref_chain_hash 直接查表返回 O(1)；方向为正向才能前缀增量递推。
    mutable std::unique_ptr<std::once_flag> chainHashOnce_;
    mutable std::vector<RefChainKey> chainHash_;
    mutable std::vector<uint8_t> chainHashComputed_;
};

} // namespace mem_analyzer
