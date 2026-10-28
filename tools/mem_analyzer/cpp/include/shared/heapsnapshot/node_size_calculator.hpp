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
#include <vector>
#include <cstdint>
#include "shared/heapsnapshot/heap_snapshot.hpp"

namespace mem_analyzer {

struct RetainedSizeResult {
    std::vector<double> retainedSizes;     // retainedSizes[nodeIndex]
    std::vector<uint32_t> dominators;        // dominators[nodeIndex] = idom nodeIndex
    std::vector<uint32_t> postOrder;        // reachable nodes in post-order
    std::vector<uint8_t> reachable;          // 1 if reachable
    uint32_t nodeCount = 0;
    uint32_t reachableCount = 0;
    double totalSelfSize = 0;
    double reachableSelfSize = 0;
    double rootRetainedSize = 0;
};

/**
 * Compute dominator-based retained sizes.
 * Algorithm: Lengauer-Tarjan dominator + post-order prefix sum.
 * exclude_weak_edges: if true, skip 'weak' edges (matching Chrome devtools).
 * exclude_shortcut_edges: if true, skip 'shortcut' edges from non-root nodes (matching Chrome devtools).
 */
RetainedSizeResult ComputeNodeSizes(const HeapSnapshot& snapshot,
    bool excludeWeakEdges = true,
    bool excludeShortcutEdges = true);

/// One step in the shortest reference chain from GC root to a node.
struct PathEdgeRecord {
    uint32_t parentNodeIndex;      // parent in shortest path (UINT32_MAX = root)
    uint32_t edgeFieldStart;       // edge field start index in edges array (UINT32_MAX = root)
};

/// Result of compute_shortest_paths.
struct ShortestPathsResult {
    std::vector<PathEdgeRecord> pathEdges;  // per nodeIndex
    uint32_t reachableCount;
};

/**
 * BFS from GC roots to annotate every node with its shortest-path incoming edge.
 * The resulting path_edges array allows O(path_length) backtracking from any
 * reachable node to a GC root.
 *
 * exclude_weak_edges: if true, weak edges are not traversed during BFS.
 */
ShortestPathsResult ComputeShortestPaths(const HeapSnapshot& snapshot, bool excludeWeakEdges = true,
    bool excludeShortcutEdges = true, bool excludeHiddenEdges = true);

} // namespace mem_analyzer
