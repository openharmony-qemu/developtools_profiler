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
#define CRT_SECURE_NO_WARNINGS
#include "shared/heapsnapshot/node_size_calculator.h"
#include "utils/timer.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <functional>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

// ============================================================================
//  node_size_calculator.cpp
//  Implements dominator-based retained size computation and shortest-path
//  BFS for V8 heap snapshots.
//  Dominator algorithm: Lengauer-Tarjan (1979), O(N log N) near-linear.
//    - DFS numbering on the forward graph (filtered by isEssentialEdge + page-flag)
//    - Semi-dominator computation via reverse CSR (filtered same as DFS)
//    - DSU with path compression for O(α(N)) amortized eval/link
//    - Finalization pass to resolve implicit dominators
//    - Retained size = post-order prefix sum up the dominator tree
//  Shortest paths: layered BFS from GC roots with priority queues.
//    - High-priority roots first, then low-priority fallback
//    - Postpone queue for less-preferable edges/nodes
//    - Edge filtering matching isMeaningfulEdge + isBlockListedEdge
// ============================================================================

namespace mem_analyzer {

namespace {

// Sentinel values
constexpr uint32_t UNREACHABLE = 0xFFFFFFFF;  // dominator for unreachable nodes
constexpr uint32_t ROOT = 0;                  // GC root node index (always 0)
constexpr int EMPTY_SLOT = -1;                // Cooper algorithm sentinel (unused in LT)
constexpr uint32_t MAP_DESC_SKIP_THRESHOLD = 2;       // map descriptor edge index skip threshold
constexpr uint32_t MAP_DESC_FIELDS_PER_ENTRY = 3;      // fields per map descriptor entry
constexpr uint32_t MIN_DFN_FOR_SEMI_DOM = 2;           // skip root (dfn 0) and first real node (dfn 1)

// --------------------------------------------------------------------------
//  Low-level array access helpers
// --------------------------------------------------------------------------

/**
 * Decode the target node index from an edge field pointer.
 * V8 stores to_node as (nodeIndex * nodeFieldsCount), so we divide.
 */
inline uint32_t TargetIndexOf(const std::vector<uint32_t>& edges, size_t ptr, int toNodeOff, int nodeFieldsCount)
{
    return edges[ptr + static_cast<size_t>(toNodeOff)] / static_cast<uint32_t>(nodeFieldsCount);
}

/**
 * Read self_size for a node from the flat nodes array.
 * node_index * node_fields_count + self_size_off
 */
inline uint32_t SelfSizeOf(const std::vector<uint32_t>& nodes, uint32_t nodeIndex, int nodeFieldsCount, int selfSizeOff)
{
    return nodes[static_cast<size_t>(nodeIndex) * static_cast<size_t>(nodeFieldsCount) +
                 static_cast<size_t>(selfSizeOff)];
}

// ============================================================================
//  DomContext — bundles all snapshot data for dominator computation.
//  Instead of passing 20+ parameters through every function, we pack
//  all read-only references and field offsets into a single struct.
//  The helper methods (get_node_name, etc.) encapsulate the raw array
//  indexing logic, making the algorithmic code cleaner.
// ============================================================================
struct DomContext {
    // ---- Raw data arrays (read-only references) ----
    const std::vector<uint32_t>& nodes;               // flat node field array
    const std::vector<uint32_t>& edges;               // flat edge field array
    const std::vector<std::string>& strings;          // string table
    const std::vector<std::string>& nodeTypeNames;  // e.g. ["object", "string", "code", ...]
    const std::vector<uint32_t>& firstEdgePointers; // CSR: node i's edges start at first_edge_pointers[i]

    // ---- Field counts and offsets (discovered from snapshot metadata) ----
    int nodeFieldsCount;        // number of fields per node record
    int edgeFieldsCount;        // number of fields per edge record
    int selfSizeOff;            // offset of self_size within node record
    int nativeSizeOff;          // offset of native_size within node record
    int edgeTypeOff;            // offset of edge type within edge record
    int edgeNameOff;            // offset of edge name_or_index within edge record
    int toNodeOff;              // offset of to_node within edge record
    int nameOff;                 // offset of name (string table index) within node record
    int nodeTypeOff;            // offset of type within node record

    // ---- Edge type numeric values (-1 if not present in this snapshot) ----
    int weakType;                // numeric value for "weak" edges
    int shortcutType;            // numeric value for "shortcut" edges
    int elementType;             // numeric value for "element" edges

    // ---- Total node count ----
    uint32_t nodeCount;

    /**
     * Look up a node's name from the string table.
     */
    std::string GetNodeName(uint32_t idx) const
    {
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(nodeFieldsCount);
        uint32_t ni = nodes[base + static_cast<size_t>(nameOff)];
        return (ni < strings.size()) ? strings[ni] : "";
    }
};

// ============================================================================
//  Step 0: compute page-object flags
//  Marks nodes that are reachable from the "window" (i.e., page-owned objects).
//  This mirrors the reference implementation's flagReachableNodesFromWindow.
//  Purpose: prevent debugger-injected references from dominating page objects.
//  Edges from non-page-owned nodes to page-owned nodes are skipped during
//  dominator computation.
//  Algorithm: BFS/DFS from root's children, skipping:
//    - shortcut edges (always from root)
//    - element edges to (Document DOM trees) synthetic node
//    - weak edges (in the DFS phase)
// ============================================================================
static void ComputePageFlags(DomContext& ctx, std::vector<uint32_t>& pageFlags, StageTimer& timer)
{
    constexpr uint32_t pageObjectFlag = 1;  // single bit flag
    pageFlags.assign(ctx.nodeCount, 0);

    // Use a stack-based DFS (not recursive) to avoid stack overflow on large heaps
    std::vector<uint32_t> stack(ctx.nodeCount, 0);
    uint32_t sp = 0;

    // ---- Seed: root's children ----
    // We iterate root's outgoing edges and seed the DFS with all non-excluded children.
    // This matches the reference implementation's flagReachableNodesFromWindow
    // which starts from root's children (not root itself).
    size_t rBegin = ctx.firstEdgePointers[ROOT];
    size_t rEnd = ctx.firstEdgePointers[ROOT + 1];
    for (size_t ptr = rBegin; ptr < rEnd; ptr += static_cast<size_t>(ctx.edgeFieldsCount)) {
        int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edgeTypeOff)]);

        // Skip shortcut edges from root (they point to synthetic GC internals)
        if (ctx.shortcutType >= 0 && t == ctx.shortcutType) {
            continue;
        }

        // Skip element edges to (Document DOM trees) — this synthetic node
        // is not a real page object and should not propagate page flags.
        if (ctx.elementType >= 0 && t == ctx.elementType) {
            uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (child < ctx.nodeCount && ctx.GetNodeName(child) == "(Document DOM trees)") {
                continue;
            }
        }

        uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
        if (child < ctx.nodeCount && !(pageFlags[child] & pageObjectFlag)) {
            pageFlags[child] |= pageObjectFlag;
            stack[sp++] = child;
        }
    }

    // ---- DFS from flagged children ----
    // Follow all non-weak edges to propagate the page flag transitively.
    // Weak edges are excluded because weak references don't indicate page ownership.
    while (sp > 0) {
        uint32_t cur = stack[--sp];
        size_t begin = ctx.firstEdgePointers[cur];
        size_t end = ctx.firstEdgePointers[static_cast<size_t>(cur) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edgeFieldsCount)) {
            int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edgeTypeOff)]);
            if (ctx.weakType >= 0 && t == ctx.weakType) {
                continue;
            }
            uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (child >= ctx.nodeCount || (pageFlags[child] & pageObjectFlag)) {
                continue;
            }
            pageFlags[child] |= pageObjectFlag;
            stack[sp++] = child;
        }
    }
    timer.Tick("page_flags");
}

// ============================================================================
//  Step 1: DFS post-order traversal
//  Builds the post-order index used by the retained-size prefix sum.
//  Uses is_traversable filter: isEssentialEdge + page-flag exclusion.
//  This matches the reference implementation's buildPostOrderIndex.
//  The result is normalized so that ALL nodes have a post-order index:
//    - Reachable non-root nodes: indices 0 .. reachable_count-2
//    - Unreachable nodes: indices reachable_count-1 .. node_count-2
//    - Root: index node_count-1 (highest, matching rootPostOrderedIndex)
// ============================================================================
struct PostOrderResult {
    std::vector<uint8_t> visited;              // 1 if node was reached in DFS
    std::vector<uint32_t> postOrder;          // post_order[po_index] = node_index
    std::vector<int> nodeIndex2PostOrder;  // reverse mapping: node_index -> po_index
    uint32_t reachableCount;                  // number of nodes reached from root
    uint32_t rootPo;                          // post-order index of root (always node_count-1)
};

constexpr uint32_t PAGE_OBJECT_FLAG = 1;

/// Edge filter: combines isEssentialEdge (weak + non-root shortcut) with page-flag exclusion.
static bool IsTraversableEdge(const DomContext& ctx,
    const std::vector<uint32_t>& pageFlags,
    uint32_t sourceNode, size_t ptr)
{
    int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edgeTypeOff)]);
    // isEssentialEdge: weak edges are always excluded;
    // shortcut edges are excluded from non-root nodes
    if (ctx.weakType >= 0 && t == ctx.weakType) {
        return false;
    }
    if (ctx.shortcutType >= 0 && t == ctx.shortcutType && sourceNode != ROOT) {
        return false;
    }
    // Page-flag exclusion: skip edges from non-page-owned to page-owned nodes.
    uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
    if (sourceNode != ROOT && child < ctx.nodeCount &&
        (pageFlags[child] & PAGE_OBJECT_FLAG) &&
        !(pageFlags[sourceNode] & PAGE_OBJECT_FLAG)) {
        return false;
    }
    return true;
}

/// Normalize post-order: assign indices to ALL nodes (reachable, unreachable, root).
static void NormalizePostOrder(PostOrderResult& po,
    const DomContext& ctx, uint32_t reachableCount)
{
    // ---- Normalize: assign post-order indices to ALL nodes ----
    // The LT algorithm and retained-size propagation require every node to have
    // a valid post-order index.  Unreachable nodes are placed after reachable
    // ones, and root is placed at the very end (highest index).
    std::vector<uint32_t> newPo(ctx.nodeCount, 0);
    uint32_t newPoIdx = 0;

    // 1) Reachable non-root nodes (keep existing relative order)
    for (uint32_t i = 0; i < reachableCount; i++) {
        uint32_t ni = po.postOrder[i];
        if (ni != ROOT) {
            newPo[newPoIdx] = ni;
            po.nodeIndex2PostOrder[ni] = static_cast<int>(newPoIdx);
            newPoIdx++;
        }
    }

    // 2) Unreachable nodes (not visited, not root)
    for (uint32_t ni = 0; ni < ctx.nodeCount; ni++) {
        if (!po.visited[ni] && ni != ROOT) {
            newPo[newPoIdx] = ni;
            po.nodeIndex2PostOrder[ni] = static_cast<int>(newPoIdx);
            newPoIdx++;
        }
    }

    // 3) Root at the VERY END (highest post-order index)
    po.nodeIndex2PostOrder[ROOT] = static_cast<int>(newPoIdx);
    newPo[newPoIdx] = ROOT;
    newPoIdx++;

    po.postOrder = std::move(newPo);
    po.reachableCount = reachableCount;
    po.rootPo = ctx.nodeCount - 1;
}

static PostOrderResult ComputeDfsPostOrder(DomContext& ctx, const std::vector<uint32_t>& pageFlags, StageTimer& timer)
{
    PostOrderResult po;
    po.visited.assign(ctx.nodeCount, 0);
    po.postOrder.assign(ctx.nodeCount, 0);
    po.nodeIndex2PostOrder.assign(ctx.nodeCount, -1);

    // ---- Iterative DFS using explicit stack ----
    // s_node[sp] = current node, s_cur[sp] = current edge cursor for that node.
    // This avoids recursion which would overflow the stack on large heaps (3M+ nodes).
    std::vector<uint32_t> sNode(ctx.nodeCount, 0);
    std::vector<uint32_t> sCur(ctx.nodeCount, 0);
    uint32_t sp = 0;
    uint32_t reachableCount = 0;

    po.visited[ROOT] = 1;
    sNode[sp] = ROOT;
    sCur[sp] = ctx.firstEdgePointers[ROOT];
    sp++;

    while (sp > 0) {
        uint32_t i = sp - 1;
        uint32_t node = sNode[i];
        uint32_t end = ctx.firstEdgePointers[static_cast<size_t>(node) + 1];
        bool descended = false;

        // Try to find the next unvisited child via a traversable edge
        while (sCur[i] < end) {
            size_t ptr = sCur[i];
            sCur[i] += static_cast<uint32_t>(ctx.edgeFieldsCount);
            if (!IsTraversableEdge(ctx, pageFlags, node, ptr)) {
                continue;
            }

            uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (child < ctx.nodeCount && !po.visited[child]) {
                po.visited[child] = 1;
                sNode[sp] = child;
                sCur[sp] = ctx.firstEdgePointers[child];
                sp++;
                descended = true;
                break;
            }
        }

        // If no unvisited child was found, this node is done — emit post-order
        if (!descended) {
            po.nodeIndex2PostOrder[node] = static_cast<int>(reachableCount);
            po.postOrder[reachableCount] = node;
            reachableCount++;
            sp--;
        }
    }

    NormalizePostOrder(po, ctx, reachableCount);
    timer.Tick("dfs_po", std::to_string(reachableCount) + " reachable");
    return po;
}

// ============================================================================
//  Step 2: build reverse CSR (predecessors / referrers)
//  Constructs a Compressed Sparse Row (CSR) representation of the reverse
//  edge graph: for each node, lists all predecessor nodes that have an edge
//  pointing to it, along with the edge field pointer for filtering.
//  The reverse CSR includes ALL edges (no filtering).  Filtering is applied
//  dynamically during the semi-dominator loop, matching the reference
//  implementation's forEachReferrer callback.
//  Source order is reversed (N-1 first, 0 last) to match the reference
//  implementation's --retainingEdgeCount pre-decrement fill pattern.
// ============================================================================
struct ReverseCSR {
    std::vector<uint32_t> revStart;     // CSR row pointers: rev_start[node] = start index
    std::vector<uint32_t> revAdj;       // compact predecessor list
    std::vector<uint32_t> revEdgePtr;  // edge field pointer for each predecessor
    uint32_t revTotal;                  // total number of reverse edges
};

static ReverseCSR BuildReverseCsr(DomContext& ctx, StageTimer& timer)
{
    // ---- Pass 1: count predecessors per node ----
    std::vector<uint32_t> revCount(ctx.nodeCount, 0);
    for (uint32_t src = 0; src < ctx.nodeCount; src++) {
        size_t begin = ctx.firstEdgePointers[src];
        size_t end = ctx.firstEdgePointers[static_cast<size_t>(src) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edgeFieldsCount)) {
            uint32_t t = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (t < ctx.nodeCount) {
                revCount[t]++;
            }
        }
    }

    // ---- Pass 2: compute CSR row pointers (prefix sum) ----
    std::vector<uint32_t> revStart(ctx.nodeCount + 1, 0);
    for (uint32_t i = 0; i < ctx.nodeCount; i++) {
        revStart[i + 1] = revStart[i] + revCount[i];
    }
    uint32_t revTotal = revStart[ctx.nodeCount];
    std::vector<uint32_t> revAdj(revTotal, 0);
    std::vector<uint32_t> revEdgePtr(revTotal, 0);
    std::vector<uint32_t> revFill(ctx.nodeCount, 0);

    // ---- Pass 3: fill with reversed source order ----
    // Iterating sources in reverse order (N-1 down to 0) and filling forward
    // produces the same slot order as the reference implementation's
    // --retainingEdgeCount pre-decrement pattern.
    for (uint32_t src = ctx.nodeCount; src > 0;) {
        src--;
        size_t begin = ctx.firstEdgePointers[src];
        size_t end = ctx.firstEdgePointers[static_cast<size_t>(src) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edgeFieldsCount)) {
            uint32_t t = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (t >= ctx.nodeCount) {
                continue;
            }
            size_t slot = revStart[t] + revFill[t]++;
            revAdj[slot] = src;
            revEdgePtr[slot] = static_cast<uint32_t>(ptr);
        }
    }
    timer.Tick("rev_csr", std::to_string(revTotal) + " edges");
    return {std::move(revStart), std::move(revAdj), std::move(revEdgePtr), revTotal};
}

// ============================================================================
//  Step 3: Lengauer-Tarjan dominator algorithm
//  Reference: "A Fast Algorithm for Finding Dominators in a Flowgraph"
//  by Thomas Lengauer and Robert Endre Tarjan, ACM TOPLAS 1979.
//  The algorithm has three sub-steps:
//    3a) DFS to number nodes (1-based dfn), building the spanning tree
//    3b) Compute semi-dominators in reverse dfn order, using DSU for eval
//    3c) Finalize immediate dominators in forward dfn order
//  Edge filtering for the semi-dominator pass matches the TS reference:
//    - isEssentialEdge (weak + non-root shortcut)
//    - self-reference
//    - page-flag exclusion (non-page-owned -> page-owned)
// ============================================================================
static std::vector<uint32_t> computeLtDominators(
    DomContext& ctx,
    const std::vector<uint32_t>& pageFlags,
    const ReverseCSR& rev,
    StageTimer& timer)
{
    constexpr uint32_t pageObjectFlag = 1;

    // ---- Edge filter for the LT DFS (same as post-order DFS) ----
    auto isTraversable = [&ctx, &pageFlags](uint32_t sourceNode, size_t ptr) -> bool {
        int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edgeTypeOff)]);
        if (ctx.weakType >= 0 && t == ctx.weakType) {
            return false;
        }
        if (ctx.shortcutType >= 0 && t == ctx.shortcutType && sourceNode != ROOT) {
            return false;
        }
        uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
        if (sourceNode != ROOT && child < ctx.nodeCount &&
            (pageFlags[child] & pageObjectFlag) &&
            !(pageFlags[sourceNode] & pageObjectFlag)) {
            return false;
        }
        return true;
    };

    // ---- LT working arrays (1-indexed: index 0 = invalid) ----
    // lt_dfn[v]        : DFS number of node v (0 = unvisited)
    // lt_vertex[dfn]   : node index for a given DFS number
    // lt_parent[v]     : parent of v in the DFS spanning tree
    // lt_semi[v]       : semi-dominator of v (stored as DFS number)
    // lt_idom[v]       : immediate dominator of v (stored as node index)
    // lt_ancestor[v]   : DSU ancestor pointer (0 = root of DSU tree)
    // lt_label[v]      : DSU label: node with minimal semi on the path to ancestor
    // lt_bucket_head[v]: head of linked list of nodes whose semi-dominator is v
    std::vector<uint32_t> ltDfn(ctx.nodeCount, 0);
    std::vector<uint32_t> ltVertex(ctx.nodeCount + 1, 0);
    std::vector<uint32_t> ltParent(ctx.nodeCount, ROOT);
    std::vector<int> ltSemi(ctx.nodeCount, 0);
    std::vector<uint32_t> ltIdom(ctx.nodeCount, ROOT);
    std::vector<uint32_t> ltAncestor(ctx.nodeCount, 0);
    std::vector<uint32_t> ltLabel(ctx.nodeCount, 0);
    std::vector<uint32_t> ltBucketHead(ctx.nodeCount, 0);
    std::vector<uint32_t> ltBucketNext(ctx.nodeCount, 0);

    // ---- 3a) Iterative DFS to number nodes ----
    // Uses the same is_traversable filter as the post-order DFS.
    // Assigns DFS numbers starting from 1 (root = 1).
    std::vector<uint32_t> ltDfsStack(ctx.nodeCount);
    std::vector<uint32_t> ltDfsChildIdx(ctx.nodeCount);
    uint32_t ltDfsSp = 0;
    uint32_t ltDfnCount = 0;

    ltDfn[ROOT] = 1;
    ltVertex[1] = ROOT;
    ltSemi[ROOT] = 1;        // semi-dominator of root is itself
    ltLabel[ROOT] = ROOT;    // DSU label: root points to itself
    ltDfnCount = 1;

    ltDfsStack[0] = ROOT;
    ltDfsChildIdx[0] = ctx.firstEdgePointers[ROOT];
    ltDfsSp = 1;

    while (ltDfsSp > 0) {
        uint32_t top = ltDfsSp - 1;
        uint32_t node = ltDfsStack[top];
        size_t end = ctx.firstEdgePointers[static_cast<size_t>(node) + 1];
        bool descended = false;

        while (ltDfsChildIdx[top] < end) {
            size_t ptr = ltDfsChildIdx[top];
            ltDfsChildIdx[top] += static_cast<uint32_t>(ctx.edgeFieldsCount);
            if (!isTraversable(node, ptr)) {
                continue;
            }

            uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (child < ctx.nodeCount && ltDfn[child] == 0) {
                // First time visiting this child — assign DFS number and tree edge
                ltDfnCount++;
                ltDfn[child] = ltDfnCount;
                ltVertex[ltDfnCount] = child;
                ltParent[child] = node;
                ltSemi[child] = static_cast<int>(ltDfnCount);  // initial semi = own dfn
                ltLabel[child] = child;                           // DSU label = self

                ltDfsStack[ltDfsSp] = child;
                ltDfsChildIdx[ltDfsSp] = ctx.firstEdgePointers[child];
                ltDfsSp++;
                descended = true;
                break;
            }
        }

        if (!descended) {
            ltDfsSp--;
        }
    }

    // ---- DSU (Disjoint Set Union) helpers for the LT algorithm ----
    // These implement the LINK and EVAL operations with path compression.
    // EVAL(v) returns the node on the path from v to the DSU root
    // that has the minimal semi-dominator.

    // compress(v): iterative path compression with semi-dominator update.
    // Replaced the recursive call with an explicit path buffer to avoid stack
    // overflow on deep dominator-tree chains (large heap / rawheap dumps),
    // consistent with the iterative DFS used elsewhere in this file.
    // After compression, lt_label[v] holds the node with minimal semi
    // on the path from v to the DSU root.
    std::vector<uint32_t> ltCompressPath(ctx.nodeCount);
    std::function<void(uint32_t)> ltCompress;
    ltCompress = [&ltAncestor, &ltCompressPath, &ltSemi, &ltLabel](uint32_t v) {
        // 1) Walk up the ancestor chain until reaching a node whose ancestor
        //    is a DSU root (lt_ancestor[lt_ancestor[cur]] == 0), recording
        //    the path for the post-order update below.
        uint32_t pathLen = 0;
        uint32_t cur = v;
        while (ltAncestor[ltAncestor[cur]] != 0) {
            ltCompressPath[pathLen++] = cur;
            cur = ltAncestor[cur];
        }
        // 2) Replay the path in reverse so deeper nodes are finalized first,
        //    mirroring the recursive version's post-return updates.
        while (pathLen > 0) {
            uint32_t u = ltCompressPath[--pathLen];
            uint32_t a = ltAncestor[u];
            if (ltSemi[ltLabel[a]] < ltSemi[ltLabel[u]]) {
                ltLabel[u] = ltLabel[a];
            }
            ltAncestor[u] = ltAncestor[a];
        }
    };

    // eval(v): returns the node with minimal semi-dominator on the path
    // from v to the root of its DSU tree.
    auto ltEval = [&ltAncestor, &ltLabel, &ltSemi, &ltCompress](uint32_t v) -> uint32_t {
        if (ltAncestor[v] == 0) { // v is a DSU root
            return ltLabel[v];
        }
        ltCompress(v);
        // After compression, lt_ancestor[v] is the DSU root.
        // Compare semi of label[ancestor[v]] with label[v].
        if (ltSemi[ltLabel[ltAncestor[v]]] < ltSemi[ltLabel[v]]) {
            return ltLabel[ltAncestor[v]];
        }
        return ltLabel[v];
    };

    // link(v, w): makes v the parent of w in the DSU forest.
    auto ltLink = [&ltAncestor](uint32_t v, uint32_t w) {
        ltAncestor[w] = v;
    };

    // ---- 3b) Compute semi-dominators in reverse dfn order ----
    // Process nodes from highest dfn down to 2 (skip root).
    // For each node w, examine all its predecessors (from reverse CSR).
    // The semi-dominator of w is the minimum of:
    //   eval(pred).semi  for all essential predecessors pred of w
    for (uint32_t i = ltDfnCount; i >= MIN_DFN_FOR_SEMI_DOM; i--) {
        uint32_t w = ltVertex[i];  // current node (by descending dfn)
        int wSemi = ltSemi[w];    // current best semi-dominator

        size_t rBegin = rev.revStart[w];
        size_t rEnd = rev.revStart[static_cast<size_t>(w) + 1];
        for (size_t r = rBegin; r < rEnd; r++) {
            uint32_t pred = rev.revAdj[r];
            if (ltDfn[pred] == 0) { // predecessor not in DFS tree
                continue;
            }

            // ---- Edge filtering (matching the reference implementation) ----
            uint32_t edgePtr = rev.revEdgePtr[r];
            int et = static_cast<int>(ctx.edges[edgePtr + static_cast<size_t>(ctx.edgeTypeOff)]);

            // isEssentialEdge: exclude weak edges and non-root shortcut edges
            bool essential = !((ctx.weakType >= 0 && et == ctx.weakType) ||
                               (ctx.shortcutType >= 0 && et == ctx.shortcutType && pred != ROOT));
            if (!essential) {
                continue;
            }

            // Self-reference: a node cannot dominate itself through a self-loop
            if (pred == w) {
                continue;
            }

            // Page-flag exclusion: skip edges from non-page-owned to page-owned nodes
            if (pred != ROOT &&
                (pageFlags[w] & pageObjectFlag) &&
                !(pageFlags[pred] & pageObjectFlag)) {
                continue;
            }

            // Evaluate the predecessor: find the node with minimal semi-dominator
            // on the path from pred to the DSU root.
            uint32_t uEval = ltEval(pred);
            int uSemi = ltSemi[uEval];
            if (uSemi < wSemi) {
                wSemi = uSemi;
            }
        }

        ltSemi[w] = wSemi;

        // ---- Bucket processing ----
        // Add w to the bucket of its semi-dominator node.
        // Later, when processing the parent of w, we resolve the dominators
        // of all nodes in the parent's bucket.
        uint32_t semiNode = ltVertex[static_cast<size_t>(wSemi)];
        ltBucketNext[w] = ltBucketHead[semiNode];
        ltBucketHead[semiNode] = w;

        // Link w into the DSU forest as a child of its DFS parent
        ltLink(ltParent[w], w);

        // Process the bucket of parent[w]: for each node v in the bucket,
        // compute its immediate dominator using eval.
        uint32_t p = ltParent[w];
        uint32_t b = ltBucketHead[p];
        while (b != 0) {
            uint32_t v = b;
            b = ltBucketNext[v];
            uint32_t u = ltEval(v);
            // If semi(u) < semi(v), then u is a better candidate for idom(v);
            // otherwise the parent p is the idom.
            ltIdom[v] = (ltSemi[u] < ltSemi[v]) ? u : p;
        }
        ltBucketHead[p] = 0;
    }

    // ---- 3c) Finalize immediate dominators in forward dfn order ----
    // For nodes where idom[w] != vertex[semi[w]], the true idom is the
    // idom of the current idom (implicit dominator resolution).
    for (uint32_t i = 2; i <= ltDfnCount; i++) {
        uint32_t w = ltVertex[i];
        if (ltIdom[w] != ltVertex[static_cast<size_t>(ltSemi[w])]) {
            ltIdom[w] = ltIdom[ltIdom[w]];
        }
    }
    ltIdom[ROOT] = ROOT;  // root dominates itself

    // ---- Materialize dominators indexed by nodeIndex ----
    // Nodes not reached in the DFS (lt_dfn == 0) are marked UNREACHABLE.
    std::vector<uint32_t> dominators(ctx.nodeCount, UNREACHABLE);
    for (uint32_t i = 0; i < ctx.nodeCount; i++) {
        if (ltDfn[i] != 0) {
            dominators[i] = ltIdom[i];
        }
    }
    dominators[ROOT] = ROOT;
    timer.Tick("lt_dominator", std::to_string(ltDfnCount) + " dfn");
    return dominators;
}

// ============================================================================
//  Step 4: retained size via post-order prefix sum
//  Retained size of a node = its self_size + sum of retained sizes of all
//  nodes it dominates.  This is computed by propagating sizes up the
//  dominator tree in ascending post-order (leaves first, root last).
//  Uses self_size only (not native_size), matching the reference
//  implementation's calculateRetainedSizesFromDominatorNodes.
// ============================================================================
static void ComputeRetainedSizes(
    DomContext& ctx,
    const PostOrderResult& po,
    const std::vector<uint32_t>& dominators,
    RetainedSizeResult& result)
{
    // ---- Initialize: each node's retained size starts as its self_size ----
    std::vector<double> retainedSizes(ctx.nodeCount, 0.0);
    double totalSelfSize = 0.0;

    for (uint32_t i = 0; i < ctx.nodeCount; i++) {
        double s = static_cast<double>(SelfSizeOf(ctx.nodes, i, ctx.nodeFieldsCount, ctx.selfSizeOff));
        retainedSizes[i] = s;
        totalSelfSize += s;
    }

    // ---- Compute reachable self_size for statistics ----
    double reachableSelfSize = 0.0;
    for (uint32_t i = 0; i < po.reachableCount; i++) {
        reachableSelfSize += static_cast<double>(
            SelfSizeOf(ctx.nodes, po.postOrder[i], ctx.nodeFieldsCount, ctx.selfSizeOff));
    }

    // ---- Propagate retained sizes up the dominator tree ----
    // Iterate in ascending post-order (leaves first).
    // For each node, add its retained size to its immediate dominator.
    // Since we process in post-order, a node's retained size is fully
    // computed (all descendants already added) before it's propagated.
    for (uint32_t i = 0; i < po.rootPo; i++) {
        uint32_t nodeIndex = po.postOrder[i];
        uint32_t domNodeIndex = dominators[nodeIndex];
        if (domNodeIndex != UNREACHABLE) {
            retainedSizes[domNodeIndex] += retainedSizes[nodeIndex];
        }
    }

    // ---- Populate result ----
    result.retainedSizes = std::move(retainedSizes);
    result.dominators = dominators;
    result.postOrder.assign(po.postOrder.begin(), po.postOrder.begin() + ctx.nodeCount);
    result.reachable = po.visited;
    result.reachableCount = po.reachableCount;
    result.totalSelfSize = totalSelfSize;
    result.reachableSelfSize = reachableSelfSize;
    result.rootRetainedSize = result.retainedSizes[ROOT];
}

} // anonymous namespace

// ============================================================================
//  Public API: compute_node_sizes
//  Orchestrates the four steps of dominator-based retained size computation:
//    0. page_flags (marks page-owned objects)
//    1. DFS post-order (for retained size propagation)
//    2. Reverse CSR (for predecessor iteration)
//    3. LT dominator (computes the dominator tree)
//    4. Retained size propagation (post-order prefix sum)
// ============================================================================

RetainedSizeResult ComputeNodeSizes(const HeapSnapshot& snapshot, bool excludeWeakEdges, bool excludeShortcutEdges)
{
    StageTimer timer("NodeSizes");

    // ---- Build the context struct from snapshot data ----
    DomContext ctx{
        snapshot.nodes,
        snapshot.edges,
        snapshot.strings,
        snapshot.nodeTypeNames,
        snapshot.firstEdgePointers,
        snapshot.nodeFieldsCount,
        snapshot.edgeFieldsCount,
        snapshot.nodeSelfSizeOffset,
        snapshot.nodeNativeSizeOffset,
        snapshot.edgeTypeOffset,
        snapshot.edgeNameOrIndexOffset,
        snapshot.edgeToNodeOffset,
        snapshot.nodeNameOffset,
        snapshot.nodeTypeOffset,
        excludeWeakEdges ? snapshot.edgeWeakType : -1,
        excludeShortcutEdges ? snapshot.edgeShortcutType : -1,
        snapshot.edgeElementType,
        static_cast<uint32_t>(snapshot.nodeCount),
    };

    if (ctx.nodeCount == 0) {
        return {};
    }

    RetainedSizeResult result;
    result.nodeCount = ctx.nodeCount;

    // ---- Execute the pipeline ----
    std::vector<uint32_t> pageFlags;
    ComputePageFlags(ctx, pageFlags, timer);

    auto po = ComputeDfsPostOrder(ctx, pageFlags, timer);

    auto rev = BuildReverseCsr(ctx, timer);

    auto dominators = computeLtDominators(ctx, pageFlags, rev, timer);

    ComputeRetainedSizes(ctx, po, dominators, result);

    timer.Done();
    return result;
}

// ============================================================================
//  Shortest paths BFS — context and helpers
//  Computes the shortest reference chain from GC roots to every reachable
//  node.  Uses a layered BFS with priority queues, matching the reference
//  implementation's annotateShortestPaths.
//  Key design decisions:
//    - Layer-by-layer processing with LIFO pop within each layer
//    - 'queued' flag prevents nodes from being targeted by multiple edges
//    - Postpone queue for low-priority edges/nodes (grey lists)
//    - Fallback to low-priority roots when normal queue is exhausted
// ============================================================================

namespace {

// 查表越界时返回的空字符串（const std::string& 返回路径需要稳定的引用，
// 不能返回临时对象 ""，否则悬垂）。
const std::string EMPTY_STRING;

/**
 * Context struct for the BFS, mirroring DomContext for the dominator pipeline.
 */
struct BfsContext {
    const std::vector<uint32_t>& edges;
    const std::vector<uint32_t>& nodes;
    const std::vector<std::string>& strings;
    const std::vector<std::string>& nodeTypeNames;
    const std::vector<uint32_t>& firstEdgePointers;
    int edgeFieldsCount;
    int edgeTypeOff;
    int edgeNameOff;
    int toNodeOff;
    int nodeFieldsCount;
    int nodeTypeOff;
    int nameOff;
    int idOff;            // offset of node id within node record
    int weakType;
    int shortcutType;
    int hiddenType;
    int internalType;
    int elementType;
    uint32_t nodeCount;

    /**
     * Look up an edge's name from the string table.
     */
    const std::string& GetEdgeName(size_t ptr) const
    {
        uint32_t nameIdx = edges[ptr + static_cast<size_t>(edgeNameOff)];
        return (nameIdx < strings.size()) ? strings[nameIdx] : EMPTY_STRING;
    }

    /**
     * Look up a node's name from the string table.
     */
    const std::string& GetNodeName(uint32_t idx) const
    {
        if (idx >= nodeCount) {
            return EMPTY_STRING;
        }
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(nodeFieldsCount);
        uint32_t ni = nodes[base + static_cast<size_t>(nameOff)];
        return (ni < strings.size()) ? strings[ni] : EMPTY_STRING;
    }

    /**
     * Look up a node's type name (e.g. "object", "string", "code").
     */
    const std::string& GetNodeType(uint32_t idx) const
    {
        if (idx >= nodeCount || nodeTypeNames.empty()) {
            return EMPTY_STRING;
        }
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(nodeFieldsCount);
        uint32_t ti = nodes[base + static_cast<size_t>(nodeTypeOff)];
        return (ti < nodeTypeNames.size()) ? nodeTypeNames[ti] : EMPTY_STRING;
    }
};

// ---- Block lists (matching the reference implementation's Config) ----

/**
 * Edge name block list: edges with these names are never traversed.
 */
static bool IsEdgeNameBlocked(const std::string& name)
{
    return name == "feedback_cell" ||
           name == "part of key -> value pair in ephemeron table";
}

/**
 * Node name block list: nodes with these names are skipped entirely.
 */
static bool IsNodeNameBlocked(const std::string& name)
{
    return name == "system / PropertyCell";
}

// ---- Grey lists (low priority) ----
// Edges/nodes matching these are deferred to the postpone queue.

static const std::unordered_set<std::string> edgeNameGreyList = {
    "alternate", "firstEffect", "lastEffect"
};

static const std::unordered_set<std::string> nodeNameGreyList = {
    "InternalNode", "Pending activities", "StyleEngine",
    "(GC roots)", "(Internalized strings)", "(External strings)",
    "(Read-only roots)", "(Strong roots)", "(Smi roots)",
    "(Bootstrapper)", "(Stack roots)", "(Relocatable)", "(Debugger)",
    "(Compilation cache)", "(Handle scope)", "(Builtins)",
    "(Global handles)", "(Eternal handles)", "(Thread manager)",
    "(Extensions)", "(Code flusher)", "(Startup object cache)",
    "(Read-only object cache)", "(Weak collections)", "(Wrapper tracing)",
    "(Write barrier)", "(Retain maps)", "(Unknown)", "<Synthetic>",
};

/**
 * Check if a node is a "C++ roots" synthetic node (low priority).
 */
static bool IsCppRootsNode(const BfsContext& ctx, uint32_t idx)
{
    return ctx.GetNodeName(idx) == "C++ roots" && ctx.GetNodeType(idx) == "synthetic";
}

/**
 * Collect root nodes (high-priority + low-priority).
 * Matches the reference implementation's getRootNodeList with prioritize:true.
 * High-priority roots are processed first; low-priority roots are only used
 * as a fallback when the normal queue is empty.
 * Root classification:
 *   High-priority:
 *     - Node 0 (SyntheticRoot): always
 *     - id == 0 or id == 1
 *     - type == "native" && name starts with "Window"
 *     - type == "synthetic" && name == "(GC roots)"
 *   Low-priority:
 *     - type == "synthetic" && name is Blink roots
 *     - (synthetic || native) && name == "Pending activities"
 */
static void CollectShortestPathRoots(
    const BfsContext& ctx,
    std::vector<uint32_t>& highPri,
    std::vector<uint32_t>& lowPri)
{
    // Node 0 (SyntheticRoot) is always a high-priority root
    highPri.push_back(0);

    for (uint32_t i = 1; i < ctx.nodeCount; i++) {
        uint32_t nid = ctx.nodes[i * static_cast<size_t>(ctx.nodeFieldsCount) +
                               static_cast<size_t>(ctx.idOff)];

        std::string typeName = ctx.GetNodeType(i);
        std::string nodeName = ctx.GetNodeName(i);

        // High-priority root check (excludeBlinkRoot=true, excludePendingActivity=true)
        if (nid == 0 || nid == 1) {
            highPri.push_back(i);
        } else if (typeName == "native" && nodeName.find("Window") == 0) {
            highPri.push_back(i);
        } else if (typeName == "synthetic" && nodeName == "(GC roots)") {
            highPri.push_back(i);
        } else if (typeName == "synthetic" && (nodeName == "Blink cross-thread roots" || nodeName == "Blink roots")) {
            // Low-priority root check: Blink cross-thread roots, Blink roots, Pending activities
            lowPri.push_back(i);
        } else if ((typeName == "synthetic" || typeName == "native") &&
                    nodeName == "Pending activities") {
            lowPri.push_back(i);
        }
    }
}

static bool ShouldSkipWeakMapEdge(const BfsContext& ctx, size_t ptr, uint32_t child)
{
    // Edge name block list + WeakMap key edge check
    // WeakMap edges: keys are weakly referenced; skip the forward direction
    // where the edge name contains "@<key_id>) ->" and key_id == target.id
    const std::string& en = ctx.GetEdgeName(ptr);
    if (IsEdgeNameBlocked(en)) {
        return true;
    }
    if (en.find("WeakMap") != std::string::npos) {
        static const std::regex keyRe(R"(@(\d+)\) ->)");
        std::smatch match;
        if (std::regex_search(en, match, keyRe)) {
            uint32_t keyId = 0xFFFFFFFFu;
            try {
                keyId = static_cast<uint32_t>(std::stoul(match[1].str()));
            } catch (const std::exception&) {
                // 越界/非法 key id：不当作 WeakMap key 匹配
            }
            uint32_t childId = ctx.nodes[static_cast<size_t>(child) *
                static_cast<size_t>(ctx.nodeFieldsCount) +
                static_cast<size_t>(ctx.idOff)];
            if (keyId == childId) {
                return true;
            }
        }
    }
    return false;
}

static bool ShouldSkipTargetNode(const BfsContext& ctx, uint32_t child)
{
    // ---- isMeaningfulNode check (applied to the TARGET node) ----
    const std::string& childName = ctx.GetNodeName(child);
    if (childName == "system / NativeContext" ||
        childName == "system / SourcePositionTableWithFrameCache" ||
        childName == "(map descriptors)") {
        return true;
    }
    if (ctx.GetNodeType(child) == "code") {
        return true;
    }
    if (IsNodeNameBlocked(childName)) {
        return true;
    }
    return false;
}

static bool IsLowPriorityEdge(const BfsContext& ctx, size_t ptr, uint32_t child)
{
    // ---- Priority checks (isLessPreferableEdge / isLessPreferableNode) ----
    bool isLowPriority = false;

    // Edge name in grey list → defer
    const std::string& en = ctx.GetEdgeName(ptr);
    if (edgeNameGreyList.count(en)) {
        isLowPriority = true;
    }

    // Node name in grey list → defer
    if (!isLowPriority) {
        const std::string& childName = ctx.GetNodeName(child);
        if (nodeNameGreyList.count(childName)) {
            isLowPriority = true;
        }
    }

    // C++ roots synthetic node → defer
    if (!isLowPriority && IsCppRootsNode(ctx, child)) {
        isLowPriority = true;
    }

    return isLowPriority;
}

enum class BfsEdgeAction {
    Skip,
    EnqueueNormal,
    EnqueueDeferred,
};

static bool IsInternalCodeEdge(const BfsContext& ctx, size_t ptr, int t)
{
    return ctx.internalType >= 0 && t == ctx.internalType && ctx.GetEdgeName(ptr) == "code";
}

static BfsEdgeAction ClassifyBfsEdge(const BfsContext& ctx, uint32_t cur,
    size_t ptr, uint32_t child, bool alreadyQueued)
{
    int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edgeTypeOff)]);

    // ---- Basic edge-type exclusions ----
    if (ctx.weakType >= 0 && t == ctx.weakType) {
        return BfsEdgeAction::Skip;
    }
    if (ctx.shortcutType >= 0 && t == ctx.shortcutType && cur != 0) {
        return BfsEdgeAction::Skip;
    }
    if (ctx.hiddenType >= 0 && t == ctx.hiddenType) {
        return BfsEdgeAction::Skip;
    }

    // ---- isMeaningfulEdge checks (matching the reference implementation) ----
    if (cur == child) { // self-reference
        return BfsEdgeAction::Skip;
    }

    // Edge name block list + WeakMap key edge check
    if (ShouldSkipWeakMapEdge(ctx, ptr, child)) {
        return BfsEdgeAction::Skip;
    }

    // Already queued — prevents multi-parent targeting
    if (alreadyQueued) {
        return BfsEdgeAction::Skip;
    }

    // Internal edge named "code" — V8 internal, not meaningful for leak analysis
    if (IsInternalCodeEdge(ctx, ptr, t)) {
        return BfsEdgeAction::Skip;
    }

    // Element edge to (map descriptors): only specific indices are meaningful.
    // In V8, (map descriptors) are fixed-length arrays; only elements at
    // index < 2 or index % 3 != 1 hold representative references.
    if (ctx.elementType >= 0 && t == ctx.elementType) {
        const std::string& childName = ctx.GetNodeName(child);
        if (childName == "(map descriptors)") {
            uint32_t index = ctx.edges[ptr + static_cast<size_t>(ctx.edgeNameOff)];
            if (index >= MAP_DESC_SKIP_THRESHOLD || index % MAP_DESC_FIELDS_PER_ENTRY == 1) {
                return BfsEdgeAction::Skip;
            }
        }
    }

    // ---- isMeaningfulNode check (applied to the TARGET node) ----
    if (ShouldSkipTargetNode(ctx, child)) {
        return BfsEdgeAction::Skip;
    }

    // ---- Priority checks (isLessPreferableEdge / isLessPreferableNode) ----
    bool isLowPriority = IsLowPriorityEdge(ctx, ptr, child);

    if (isLowPriority) {
        return BfsEdgeAction::EnqueueDeferred;
    }
    return BfsEdgeAction::EnqueueNormal;
}

// ============================================================================
//  Run layered BFS from roots
//  Implements the reference implementation's annotateShortestPaths:
//    1. Layer-by-layer processing with LIFO pop within each layer
//    2. Edge filtering: weak, shortcut, hidden, self-ref, blocklists,
//       WeakMap key edges, internal "code", element (map descriptors),
//       isMeaningfulNode checks
//    3. Postpone queue for low-priority edges/nodes (grey lists)
//    4. Fallback to postpone queue when normal queue is empty
//    5. Fallback to low-priority roots when both are empty
// ============================================================================
struct BfsLayerState {
    std::vector<uint8_t>& queued;
    std::vector<std::pair<uint32_t, size_t>>& postponeQueue;
    std::vector<uint32_t>& nextQueue;
    ShortestPathsResult& result;
};

static void ProcessCurrentLayer(BfsContext& ctx, std::vector<uint32_t>& curQueue,
    BfsLayerState& state)
{
    constexpr uint32_t invalid = 0xFFFFFFFF;
    while (!curQueue.empty()) {
        uint32_t cur = curQueue.back();
        curQueue.pop_back();
        size_t begin = ctx.firstEdgePointers[cur];
        size_t end = ctx.firstEdgePointers[static_cast<size_t>(cur) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edgeFieldsCount)) {
            uint32_t child = ctx.edges[ptr + static_cast<size_t>(ctx.toNodeOff)] /
                            static_cast<uint32_t>(ctx.nodeFieldsCount);
            if (child >= ctx.nodeCount) {
                continue;
            }
            BfsEdgeAction action = ClassifyBfsEdge(ctx, cur, ptr, child, state.queued[child] != 0);
            if (action == BfsEdgeAction::Skip) {
                continue;
            }
            // ---- Enqueue ----
            if (action == BfsEdgeAction::EnqueueDeferred) {
                // Deferred: add to postpone queue (queued but no path edge yet)
                state.postponeQueue.emplace_back(cur, ptr);
                state.queued[child] = 1;
            } else {
                // Normal priority: assign path edge and add to next layer
                state.result.pathEdges[child] = {cur, static_cast<uint32_t>(ptr)};
                state.nextQueue.push_back(child);
                state.queued[child] = 1;
            }
        }
    }
}

static void RunLayeredBfs(
    BfsContext& ctx,
    std::vector<uint32_t>& highPriRoots,
    std::vector<uint32_t>& lowPriRoots,
    ShortestPathsResult& result)
{
    constexpr uint32_t invalid = 0xFFFFFFFF;

    // 'queued' is set immediately when a target is enqueued, matching the
    // reference implementation's queued[toNode.nodeIndex] = 1.
    // This is separate from hasPathEdge — nodes in the postpone queue have
    // queued[] = 1 but NOT hasPathEdge yet.
    std::vector<uint8_t> queued(ctx.nodeCount, 0);
    std::vector<uint32_t> curQueue;

    // Enqueue high-priority roots in node-index order.
    // pop_back() then processes the highest-index root first (LIFO),
    // matching the reference implementation's curQueue.pop().
    for (uint32_t idx : highPriRoots) {
        result.pathEdges[idx] = {idx, invalid};  // root points to itself
        curQueue.push_back(idx);
        queued[idx] = 1;
        result.reachableCount++;
    }

    // Postpone queue: (parent_node_index, edge_field_start)
    std::vector<std::pair<uint32_t, size_t>> postponeQueue;

    while (!curQueue.empty()) {
        std::vector<uint32_t> nextQueue;

        // ---- Process current layer (LIFO) ----
        BfsLayerState state{queued, postponeQueue, nextQueue, result};
        ProcessCurrentLayer(ctx, curQueue, state);

        // ---- Fallback 1: process postponed queue ----
        // When no normal-priority nodes were found in this layer,
        // promote the postponed edges to the next layer.
        while (nextQueue.empty() && !postponeQueue.empty()) {
            auto [parent, ptr] = postponeQueue.back();
            postponeQueue.pop_back();
            uint32_t child = TargetIndexOf(ctx.edges, ptr, ctx.toNodeOff, ctx.nodeFieldsCount);
            if (result.pathEdges[child].parentNodeIndex == invalid) {
                result.pathEdges[child] = {parent, static_cast<uint32_t>(ptr)};
                nextQueue.push_back(child);
            }
        }

        // ---- Fallback 2: process low-priority roots ----
        // When both normal and postponed queues are empty,
        // promote low-priority roots to the next layer.
        while (nextQueue.empty() && !lowPriRoots.empty()) {
            uint32_t idx = lowPriRoots.back();
            lowPriRoots.pop_back();
            if (!queued[idx]) {
                result.pathEdges[idx] = {idx, invalid};
                nextQueue.push_back(idx);
                queued[idx] = 1;
                result.reachableCount++;
            }
        }

        curQueue = std::move(nextQueue);
    }
}

} // anonymous namespace

// ============================================================================
//  Public API: compute_shortest_paths
// ============================================================================

ShortestPathsResult ComputeShortestPaths(const HeapSnapshot& snapshot,
    bool excludeWeakEdges,
    bool excludeShortcutEdges,
    bool excludeHiddenEdges)
{
    StageTimer timer("ShortPaths");

    BfsContext ctx{
        snapshot.edges,
        snapshot.nodes,
        snapshot.strings,
        snapshot.nodeTypeNames,
        snapshot.firstEdgePointers,
        snapshot.edgeFieldsCount,
        snapshot.edgeTypeOffset,
        snapshot.edgeNameOrIndexOffset,
        snapshot.edgeToNodeOffset,
        snapshot.nodeFieldsCount,
        snapshot.nodeTypeOffset,
        snapshot.nodeNameOffset,
        snapshot.nodeIdOffset,
        excludeWeakEdges ? snapshot.edgeWeakType : -1,
        excludeShortcutEdges ? snapshot.edgeShortcutType : -1,
        excludeHiddenEdges ? snapshot.edgeHiddenType : -1,
        snapshot.edgeInternalType,
        snapshot.edgeElementType,
        static_cast<uint32_t>(snapshot.nodeCount),
    };

    constexpr uint32_t invalid = 0xFFFFFFFF;
    ShortestPathsResult result;
    // 与 compute_node_sizes 对称：空快照直接返回，避免后面 collect_shortest_path_roots
    // 把节点 0 当根、对空 vector 越界写。
    if (ctx.nodeCount == 0) {
        return result;
    }
    result.pathEdges.resize(ctx.nodeCount, {invalid, invalid});

    std::vector<uint32_t> highPriRoots;
    std::vector<uint32_t> lowPriRoots;
    CollectShortestPathRoots(ctx, highPriRoots, lowPriRoots);

    RunLayeredBfs(ctx, highPriRoots, lowPriRoots, result);

    timer.Tick("bfs", std::to_string(result.reachableCount) + " reachable");
    timer.Done();
    return result;
}

} // namespace mem_analyzer