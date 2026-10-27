#pragma once
#include <vector>
#include <cstdint>
#include "shared/heapsnapshot/heap_snapshot.hpp"

namespace mem_analyzer {

struct RetainedSizeResult {
    std::vector<double> retained_sizes;     // retainedSizes[nodeIndex]
    std::vector<uint32_t> dominators;        // dominators[nodeIndex] = idom nodeIndex
    std::vector<uint32_t> post_order;        // reachable nodes in post-order
    std::vector<uint8_t> reachable;          // 1 if reachable
    uint32_t node_count = 0;
    uint32_t reachable_count = 0;
    double total_self_size = 0;
    double reachable_self_size = 0;
    double root_retained_size = 0;
};

/**
 * Compute dominator-based retained sizes.
 * Algorithm: Lengauer-Tarjan dominator + post-order prefix sum.
 * exclude_weak_edges: if true, skip 'weak' edges (matching Chrome devtools).
 * exclude_shortcut_edges: if true, skip 'shortcut' edges from non-root nodes (matching Chrome devtools).
 */
RetainedSizeResult compute_node_sizes(const HeapSnapshot& snapshot,
                                         bool exclude_weak_edges = true,
                                         bool exclude_shortcut_edges = true);

/// One step in the shortest reference chain from GC root to a node.
struct PathEdgeRecord {
    uint32_t parent_node_index;      // parent in shortest path (UINT32_MAX = root)
    uint32_t edge_field_start;       // edge field start index in edges array (UINT32_MAX = root)
};

/// Result of compute_shortest_paths.
struct ShortestPathsResult {
    std::vector<PathEdgeRecord> path_edges;  // per nodeIndex
    uint32_t reachable_count;
};

/**
 * BFS from GC roots to annotate every node with its shortest-path incoming edge.
 * The resulting path_edges array allows O(path_length) backtracking from any
 * reachable node to a GC root.
 *
 * exclude_weak_edges: if true, weak edges are not traversed during BFS.
 */
ShortestPathsResult compute_shortest_paths(const HeapSnapshot& snapshot, bool exclude_weak_edges = true, bool exclude_shortcut_edges = true, bool exclude_hidden_edges = true);

} // namespace mem_analyzer
