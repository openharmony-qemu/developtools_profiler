#define _CRT_SECURE_NO_WARNINGS
#include "shared/heapsnapshot/node_size_calculator.hpp"
#include "utils/timer.hpp"
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
//
//  Implements dominator-based retained size computation and shortest-path
//  BFS for V8 heap snapshots.
//
//  Dominator algorithm: Lengauer-Tarjan (1979), O(N log N) near-linear.
//    - DFS numbering on the forward graph (filtered by isEssentialEdge + page-flag)
//    - Semi-dominator computation via reverse CSR (filtered same as DFS)
//    - DSU with path compression for O(α(N)) amortized eval/link
//    - Finalization pass to resolve implicit dominators
//    - Retained size = post-order prefix sum up the dominator tree
//
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

// --------------------------------------------------------------------------
//  Low-level array access helpers
// --------------------------------------------------------------------------

/// Decode the target node index from an edge field pointer.
/// V8 stores to_node as (nodeIndex * nodeFieldsCount), so we divide.
inline uint32_t target_index_of(const std::vector<uint32_t>& edges, size_t ptr,
                                 int to_node_off, int node_fields_count) {
    return edges[ptr + static_cast<size_t>(to_node_off)] / static_cast<uint32_t>(node_fields_count);
}

/// Read self_size for a node from the flat nodes array.
/// node_index * node_fields_count + self_size_off
inline uint32_t self_size_of(const std::vector<uint32_t>& nodes, uint32_t node_index,
                              int node_fields_count, int self_size_off) {
    return nodes[static_cast<size_t>(node_index) * static_cast<size_t>(node_fields_count)
                 + static_cast<size_t>(self_size_off)];
}

// ============================================================================
//  DomContext — bundles all snapshot data for dominator computation.
//
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
    const std::vector<std::string>& node_type_names;  // e.g. ["object", "string", "code", ...]
    const std::vector<uint32_t>& first_edge_pointers; // CSR: node i's edges start at first_edge_pointers[i]

    // ---- Field counts and offsets (discovered from snapshot metadata) ----
    int node_fields_count;        // number of fields per node record
    int edge_fields_count;        // number of fields per edge record
    int self_size_off;            // offset of self_size within node record
    int native_size_off;          // offset of native_size within node record
    int edge_type_off;            // offset of edge type within edge record
    int edge_name_off;            // offset of edge name_or_index within edge record
    int to_node_off;              // offset of to_node within edge record
    int name_off;                 // offset of name (string table index) within node record
    int node_type_off;            // offset of type within node record

    // ---- Edge type numeric values (-1 if not present in this snapshot) ----
    int weak_type;                // numeric value for "weak" edges
    int shortcut_type;            // numeric value for "shortcut" edges
    int element_type;             // numeric value for "element" edges

    // ---- Total node count ----
    uint32_t node_count;

    /// Look up a node's name from the string table.
    std::string get_node_name(uint32_t idx) const {
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(node_fields_count);
        uint32_t ni = nodes[base + static_cast<size_t>(name_off)];
        return (ni < strings.size()) ? strings[ni] : "";
    }
};

// ============================================================================
//  Step 0: compute page-object flags
//
//  Marks nodes that are reachable from the "window" (i.e., page-owned objects).
//  This mirrors the reference implementation's flagReachableNodesFromWindow.
//
//  Purpose: prevent debugger-injected references from dominating page objects.
//  Edges from non-page-owned nodes to page-owned nodes are skipped during
//  dominator computation.
//
//  Algorithm: BFS/DFS from root's children, skipping:
//    - shortcut edges (always from root)
//    - element edges to (Document DOM trees) synthetic node
//    - weak edges (in the DFS phase)
// ============================================================================
static void compute_page_flags(DomContext& ctx, std::vector<uint32_t>& page_flags,
                                StageTimer& timer) {
    constexpr uint32_t PAGE_OBJECT_FLAG = 1;  // single bit flag
    page_flags.assign(ctx.node_count, 0);

    // Use a stack-based DFS (not recursive) to avoid stack overflow on large heaps
    std::vector<uint32_t> stack(ctx.node_count, 0);
    uint32_t sp = 0;

    // ---- Seed: root's children ----
    // We iterate root's outgoing edges and seed the DFS with all non-excluded children.
    // This matches the reference implementation's flagReachableNodesFromWindow
    // which starts from root's children (not root itself).
    size_t r_begin = ctx.first_edge_pointers[ROOT];
    size_t r_end = ctx.first_edge_pointers[ROOT + 1];
    for (size_t ptr = r_begin; ptr < r_end; ptr += static_cast<size_t>(ctx.edge_fields_count)) {
        int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edge_type_off)]);

        // Skip shortcut edges from root (they point to synthetic GC internals)
        if (ctx.shortcut_type >= 0 && t == ctx.shortcut_type) continue;

        // Skip element edges to (Document DOM trees) — this synthetic node
        // is not a real page object and should not propagate page flags.
        if (ctx.element_type >= 0 && t == ctx.element_type) {
            uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (child < ctx.node_count) {
                if (ctx.get_node_name(child) == "(Document DOM trees)") continue;
            }
        }

        uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
        if (child < ctx.node_count && !(page_flags[child] & PAGE_OBJECT_FLAG)) {
            page_flags[child] |= PAGE_OBJECT_FLAG;
            stack[sp++] = child;
        }
    }

    // ---- DFS from flagged children ----
    // Follow all non-weak edges to propagate the page flag transitively.
    // Weak edges are excluded because weak references don't indicate page ownership.
    while (sp > 0) {
        uint32_t cur = stack[--sp];
        size_t begin = ctx.first_edge_pointers[cur];
        size_t end = ctx.first_edge_pointers[static_cast<size_t>(cur) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edge_fields_count)) {
            int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edge_type_off)]);
            if (ctx.weak_type >= 0 && t == ctx.weak_type) continue;
            uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (child < ctx.node_count && !(page_flags[child] & PAGE_OBJECT_FLAG)) {
                page_flags[child] |= PAGE_OBJECT_FLAG;
                stack[sp++] = child;
            }
        }
    }
    timer.tick("page_flags");
}

// ============================================================================
//  Step 1: DFS post-order traversal
//
//  Builds the post-order index used by the retained-size prefix sum.
//  Uses is_traversable filter: isEssentialEdge + page-flag exclusion.
//  This matches the reference implementation's buildPostOrderIndex.
//
//  The result is normalized so that ALL nodes have a post-order index:
//    - Reachable non-root nodes: indices 0 .. reachable_count-2
//    - Unreachable nodes: indices reachable_count-1 .. node_count-2
//    - Root: index node_count-1 (highest, matching rootPostOrderedIndex)
// ============================================================================
struct PostOrderResult {
    std::vector<uint8_t> visited;              // 1 if node was reached in DFS
    std::vector<uint32_t> post_order;          // post_order[po_index] = node_index
    std::vector<int> node_index_2_post_order;  // reverse mapping: node_index -> po_index
    uint32_t reachable_count;                  // number of nodes reached from root
    uint32_t root_po;                          // post-order index of root (always node_count-1)
};

static PostOrderResult compute_dfs_post_order(DomContext& ctx,
                                               const std::vector<uint32_t>& page_flags,
                                               StageTimer& timer) {
    constexpr uint32_t PAGE_OBJECT_FLAG = 1;

    // ---- Edge filter for DFS reachability ----
    // Combines isEssentialEdge (weak + non-root shortcut) with page-flag exclusion.
    // This is identical to the filter used in the LT DFS (Step 3a).
    auto is_traversable = [&](uint32_t source_node, size_t ptr) -> bool {
        int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edge_type_off)]);

        // isEssentialEdge: weak edges are always excluded;
        // shortcut edges are excluded from non-root nodes
        if (ctx.weak_type >= 0 && t == ctx.weak_type) return false;
        if (ctx.shortcut_type >= 0 && t == ctx.shortcut_type && source_node != ROOT) return false;

        // Page-flag exclusion: skip edges from non-page-owned to page-owned nodes.
        // The debugger may inject additional references into page objects;
        // these should not affect the dominance relation.
        uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
        if (source_node != ROOT && child < ctx.node_count &&
            (page_flags[child] & PAGE_OBJECT_FLAG) &&
            !(page_flags[source_node] & PAGE_OBJECT_FLAG)) {
            return false;
        }
        return true;
    };

    PostOrderResult po;
    po.visited.assign(ctx.node_count, 0);
    po.post_order.assign(ctx.node_count, 0);
    po.node_index_2_post_order.assign(ctx.node_count, -1);

    // ---- Iterative DFS using explicit stack ----
    // s_node[sp] = current node, s_cur[sp] = current edge cursor for that node.
    // This avoids recursion which would overflow the stack on large heaps (3M+ nodes).
    std::vector<uint32_t> s_node(ctx.node_count, 0);
    std::vector<uint32_t> s_cur(ctx.node_count, 0);
    uint32_t sp = 0;
    uint32_t reachable_count = 0;

    po.visited[ROOT] = 1;
    s_node[sp] = ROOT;
    s_cur[sp] = ctx.first_edge_pointers[ROOT];
    sp++;

    while (sp > 0) {
        uint32_t i = sp - 1;
        uint32_t node = s_node[i];
        uint32_t end = ctx.first_edge_pointers[static_cast<size_t>(node) + 1];
        bool descended = false;

        // Try to find the next unvisited child via a traversable edge
        while (s_cur[i] < end) {
            size_t ptr = s_cur[i];
            s_cur[i] += static_cast<uint32_t>(ctx.edge_fields_count);
            if (!is_traversable(node, ptr)) continue;

            uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (child < ctx.node_count && !po.visited[child]) {
                po.visited[child] = 1;
                s_node[sp] = child;
                s_cur[sp] = ctx.first_edge_pointers[child];
                sp++;
                descended = true;
                break;
            }
        }

        // If no unvisited child was found, this node is done — emit post-order
        if (!descended) {
            po.node_index_2_post_order[node] = static_cast<int>(reachable_count);
            po.post_order[reachable_count] = node;
            reachable_count++;
            sp--;
        }
    }

    // ---- Normalize: assign post-order indices to ALL nodes ----
    // The LT algorithm and retained-size propagation require every node to have
    // a valid post-order index.  Unreachable nodes are placed after reachable
    // ones, and root is placed at the very end (highest index).
    {
        std::vector<uint32_t> new_po(ctx.node_count, 0);
        uint32_t new_po_idx = 0;

        // 1) Reachable non-root nodes (keep existing relative order)
        for (uint32_t i = 0; i < reachable_count; i++) {
            uint32_t ni = po.post_order[i];
            if (ni != ROOT) {
                new_po[new_po_idx] = ni;
                po.node_index_2_post_order[ni] = static_cast<int>(new_po_idx);
                new_po_idx++;
            }
        }

        // 2) Unreachable nodes (not visited, not root)
        for (uint32_t ni = 0; ni < ctx.node_count; ni++) {
            if (!po.visited[ni] && ni != ROOT) {
                new_po[new_po_idx] = ni;
                po.node_index_2_post_order[ni] = static_cast<int>(new_po_idx);
                new_po_idx++;
            }
        }

        // 3) Root at the VERY END (highest post-order index)
        po.node_index_2_post_order[ROOT] = static_cast<int>(new_po_idx);
        new_po[new_po_idx] = ROOT;
        new_po_idx++;

        po.post_order = std::move(new_po);
    }

    po.reachable_count = reachable_count;
    po.root_po = ctx.node_count - 1;  // root is always at the highest PO index
    timer.tick("dfs_po", std::to_string(reachable_count) + " reachable");
    return po;
}

// ============================================================================
//  Step 2: build reverse CSR (predecessors / referrers)
//
//  Constructs a Compressed Sparse Row (CSR) representation of the reverse
//  edge graph: for each node, lists all predecessor nodes that have an edge
//  pointing to it, along with the edge field pointer for filtering.
//
//  The reverse CSR includes ALL edges (no filtering).  Filtering is applied
//  dynamically during the semi-dominator loop, matching the reference
//  implementation's forEachReferrer callback.
//
//  Source order is reversed (N-1 first, 0 last) to match the reference
//  implementation's --retainingEdgeCount pre-decrement fill pattern.
// ============================================================================
struct ReverseCSR {
    std::vector<uint32_t> rev_start;     // CSR row pointers: rev_start[node] = start index
    std::vector<uint32_t> rev_adj;       // compact predecessor list
    std::vector<uint32_t> rev_edge_ptr;  // edge field pointer for each predecessor
    uint32_t rev_total;                  // total number of reverse edges
};

static ReverseCSR build_reverse_csr(DomContext& ctx, StageTimer& timer) {
    // ---- Pass 1: count predecessors per node ----
    std::vector<uint32_t> rev_count(ctx.node_count, 0);
    for (uint32_t src = 0; src < ctx.node_count; src++) {
        size_t begin = ctx.first_edge_pointers[src];
        size_t end = ctx.first_edge_pointers[static_cast<size_t>(src) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edge_fields_count)) {
            uint32_t t = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (t < ctx.node_count) rev_count[t]++;
        }
    }

    // ---- Pass 2: compute CSR row pointers (prefix sum) ----
    std::vector<uint32_t> rev_start(ctx.node_count + 1, 0);
    for (uint32_t i = 0; i < ctx.node_count; i++) {
        rev_start[i + 1] = rev_start[i] + rev_count[i];
    }
    uint32_t rev_total = rev_start[ctx.node_count];
    std::vector<uint32_t> rev_adj(rev_total, 0);
    std::vector<uint32_t> rev_edge_ptr(rev_total, 0);
    std::vector<uint32_t> rev_fill(ctx.node_count, 0);

    // ---- Pass 3: fill with reversed source order ----
    // Iterating sources in reverse order (N-1 down to 0) and filling forward
    // produces the same slot order as the reference implementation's
    // --retainingEdgeCount pre-decrement pattern.
    for (uint32_t src = ctx.node_count; src > 0; ) {
        src--;
        size_t begin = ctx.first_edge_pointers[src];
        size_t end = ctx.first_edge_pointers[static_cast<size_t>(src) + 1];
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edge_fields_count)) {
            uint32_t t = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (t >= ctx.node_count) continue;
            size_t slot = rev_start[t] + rev_fill[t]++;
            rev_adj[slot] = src;
            rev_edge_ptr[slot] = static_cast<uint32_t>(ptr);
        }
    }
    timer.tick("rev_csr", std::to_string(rev_total) + " edges");
    return {std::move(rev_start), std::move(rev_adj), std::move(rev_edge_ptr), rev_total};
}

// ============================================================================
//  Step 3: Lengauer-Tarjan dominator algorithm
//
//  Reference: "A Fast Algorithm for Finding Dominators in a Flowgraph"
//  by Thomas Lengauer and Robert Endre Tarjan, ACM TOPLAS 1979.
//
//  The algorithm has three sub-steps:
//    3a) DFS to number nodes (1-based dfn), building the spanning tree
//    3b) Compute semi-dominators in reverse dfn order, using DSU for eval
//    3c) Finalize immediate dominators in forward dfn order
//
//  Edge filtering for the semi-dominator pass matches the TS reference:
//    - isEssentialEdge (weak + non-root shortcut)
//    - self-reference
//    - page-flag exclusion (non-page-owned -> page-owned)
// ============================================================================
static std::vector<uint32_t> compute_lt_dominators(
    DomContext& ctx,
    const std::vector<uint32_t>& page_flags,
    const ReverseCSR& rev,
    StageTimer& timer)
{
    constexpr uint32_t PAGE_OBJECT_FLAG = 1;

    // ---- Edge filter for the LT DFS (same as post-order DFS) ----
    auto is_traversable = [&](uint32_t source_node, size_t ptr) -> bool {
        int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edge_type_off)]);
        if (ctx.weak_type >= 0 && t == ctx.weak_type) return false;
        if (ctx.shortcut_type >= 0 && t == ctx.shortcut_type && source_node != ROOT) return false;
        uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
        if (source_node != ROOT && child < ctx.node_count &&
            (page_flags[child] & PAGE_OBJECT_FLAG) &&
            !(page_flags[source_node] & PAGE_OBJECT_FLAG)) {
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
    std::vector<uint32_t> lt_dfn(ctx.node_count, 0);
    std::vector<uint32_t> lt_vertex(ctx.node_count + 1, 0);
    std::vector<uint32_t> lt_parent(ctx.node_count, ROOT);
    std::vector<int> lt_semi(ctx.node_count, 0);
    std::vector<uint32_t> lt_idom(ctx.node_count, ROOT);
    std::vector<uint32_t> lt_ancestor(ctx.node_count, 0);
    std::vector<uint32_t> lt_label(ctx.node_count, 0);
    std::vector<uint32_t> lt_bucket_head(ctx.node_count, 0);
    std::vector<uint32_t> lt_bucket_next(ctx.node_count, 0);

    // ---- 3a) Iterative DFS to number nodes ----
    // Uses the same is_traversable filter as the post-order DFS.
    // Assigns DFS numbers starting from 1 (root = 1).
    std::vector<uint32_t> lt_dfs_stack(ctx.node_count);
    std::vector<uint32_t> lt_dfs_child_idx(ctx.node_count);
    uint32_t lt_dfs_sp = 0;
    uint32_t lt_dfn_count = 0;

    lt_dfn[ROOT] = 1;
    lt_vertex[1] = ROOT;
    lt_semi[ROOT] = 1;        // semi-dominator of root is itself
    lt_label[ROOT] = ROOT;    // DSU label: root points to itself
    lt_dfn_count = 1;

    lt_dfs_stack[0] = ROOT;
    lt_dfs_child_idx[0] = ctx.first_edge_pointers[ROOT];
    lt_dfs_sp = 1;

    while (lt_dfs_sp > 0) {
        uint32_t top = lt_dfs_sp - 1;
        uint32_t node = lt_dfs_stack[top];
        size_t end = ctx.first_edge_pointers[static_cast<size_t>(node) + 1];
        bool descended = false;

        while (lt_dfs_child_idx[top] < end) {
            size_t ptr = lt_dfs_child_idx[top];
            lt_dfs_child_idx[top] += static_cast<uint32_t>(ctx.edge_fields_count);
            if (!is_traversable(node, ptr)) continue;

            uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (child < ctx.node_count && lt_dfn[child] == 0) {
                // First time visiting this child — assign DFS number and tree edge
                lt_dfn_count++;
                lt_dfn[child] = lt_dfn_count;
                lt_vertex[lt_dfn_count] = child;
                lt_parent[child] = node;
                lt_semi[child] = static_cast<int>(lt_dfn_count);  // initial semi = own dfn
                lt_label[child] = child;                           // DSU label = self

                lt_dfs_stack[lt_dfs_sp] = child;
                lt_dfs_child_idx[lt_dfs_sp] = ctx.first_edge_pointers[child];
                lt_dfs_sp++;
                descended = true;
                break;
            }
        }

        if (!descended) {
            lt_dfs_sp--;
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
    std::vector<uint32_t> lt_compress_path(ctx.node_count);
    std::function<void(uint32_t)> lt_compress;
    lt_compress = [&](uint32_t v) {
        // 1) Walk up the ancestor chain until reaching a node whose ancestor
        //    is a DSU root (lt_ancestor[lt_ancestor[cur]] == 0), recording
        //    the path for the post-order update below.
        uint32_t path_len = 0;
        uint32_t cur = v;
        while (lt_ancestor[lt_ancestor[cur]] != 0) {
            lt_compress_path[path_len++] = cur;
            cur = lt_ancestor[cur];
        }
        // 2) Replay the path in reverse so deeper nodes are finalized first,
        //    mirroring the recursive version's post-return updates.
        while (path_len > 0) {
            uint32_t u = lt_compress_path[--path_len];
            uint32_t a = lt_ancestor[u];
            if (lt_semi[lt_label[a]] < lt_semi[lt_label[u]]) {
                lt_label[u] = lt_label[a];
            }
            lt_ancestor[u] = lt_ancestor[a];
        }
    };

    // eval(v): returns the node with minimal semi-dominator on the path
    // from v to the root of its DSU tree.
    auto lt_eval = [&](uint32_t v) -> uint32_t {
        if (lt_ancestor[v] == 0) return lt_label[v];  // v is a DSU root
        lt_compress(v);
        // After compression, lt_ancestor[v] is the DSU root.
        // Compare semi of label[ancestor[v]] with label[v].
        if (lt_semi[lt_label[lt_ancestor[v]]] < lt_semi[lt_label[v]]) {
            return lt_label[lt_ancestor[v]];
        }
        return lt_label[v];
    };

    // link(v, w): makes v the parent of w in the DSU forest.
    auto lt_link = [&](uint32_t v, uint32_t w) {
        lt_ancestor[w] = v;
    };

    // ---- 3b) Compute semi-dominators in reverse dfn order ----
    // Process nodes from highest dfn down to 2 (skip root).
    // For each node w, examine all its predecessors (from reverse CSR).
    // The semi-dominator of w is the minimum of:
    //   eval(pred).semi  for all essential predecessors pred of w
    for (uint32_t i = lt_dfn_count; i >= 2; i--) {
        uint32_t w = lt_vertex[i];  // current node (by descending dfn)
        int w_semi = lt_semi[w];    // current best semi-dominator

        size_t r_begin = rev.rev_start[w];
        size_t r_end = rev.rev_start[static_cast<size_t>(w) + 1];
        for (size_t r = r_begin; r < r_end; r++) {
            uint32_t pred = rev.rev_adj[r];
            if (lt_dfn[pred] == 0) continue;  // predecessor not in DFS tree

            // ---- Edge filtering (matching the reference implementation) ----
            uint32_t edge_ptr = rev.rev_edge_ptr[r];
            int et = static_cast<int>(ctx.edges[edge_ptr + static_cast<size_t>(ctx.edge_type_off)]);

            // isEssentialEdge: exclude weak edges and non-root shortcut edges
            bool essential = !((ctx.weak_type >= 0 && et == ctx.weak_type) ||
                               (ctx.shortcut_type >= 0 && et == ctx.shortcut_type && pred != ROOT));
            if (!essential) continue;

            // Self-reference: a node cannot dominate itself through a self-loop
            if (pred == w) continue;

            // Page-flag exclusion: skip edges from non-page-owned to page-owned nodes
            if (pred != ROOT &&
                (page_flags[w] & PAGE_OBJECT_FLAG) &&
                !(page_flags[pred] & PAGE_OBJECT_FLAG)) {
                continue;
            }

            // Evaluate the predecessor: find the node with minimal semi-dominator
            // on the path from pred to the DSU root.
            uint32_t u_eval = lt_eval(pred);
            int u_semi = lt_semi[u_eval];
            if (u_semi < w_semi) {
                w_semi = u_semi;
            }
        }

        lt_semi[w] = w_semi;

        // ---- Bucket processing ----
        // Add w to the bucket of its semi-dominator node.
        // Later, when processing the parent of w, we resolve the dominators
        // of all nodes in the parent's bucket.
        uint32_t semi_node = lt_vertex[static_cast<size_t>(w_semi)];
        lt_bucket_next[w] = lt_bucket_head[semi_node];
        lt_bucket_head[semi_node] = w;

        // Link w into the DSU forest as a child of its DFS parent
        lt_link(lt_parent[w], w);

        // Process the bucket of parent[w]: for each node v in the bucket,
        // compute its immediate dominator using eval.
        uint32_t p = lt_parent[w];
        uint32_t b = lt_bucket_head[p];
        while (b != 0) {
            uint32_t v = b;
            b = lt_bucket_next[v];
            uint32_t u = lt_eval(v);
            // If semi(u) < semi(v), then u is a better candidate for idom(v);
            // otherwise the parent p is the idom.
            lt_idom[v] = (lt_semi[u] < lt_semi[v]) ? u : p;
        }
        lt_bucket_head[p] = 0;
    }

    // ---- 3c) Finalize immediate dominators in forward dfn order ----
    // For nodes where idom[w] != vertex[semi[w]], the true idom is the
    // idom of the current idom (implicit dominator resolution).
    for (uint32_t i = 2; i <= lt_dfn_count; i++) {
        uint32_t w = lt_vertex[i];
        if (lt_idom[w] != lt_vertex[static_cast<size_t>(lt_semi[w])]) {
            lt_idom[w] = lt_idom[lt_idom[w]];
        }
    }
    lt_idom[ROOT] = ROOT;  // root dominates itself

    // ---- Materialize dominators indexed by nodeIndex ----
    // Nodes not reached in the DFS (lt_dfn == 0) are marked UNREACHABLE.
    std::vector<uint32_t> dominators(ctx.node_count, UNREACHABLE);
    for (uint32_t i = 0; i < ctx.node_count; i++) {
        if (lt_dfn[i] != 0) dominators[i] = lt_idom[i];
    }
    dominators[ROOT] = ROOT;
    timer.tick("lt_dominator", std::to_string(lt_dfn_count) + " dfn");
    return dominators;
}

// ============================================================================
//  Step 4: retained size via post-order prefix sum
//
//  Retained size of a node = its self_size + sum of retained sizes of all
//  nodes it dominates.  This is computed by propagating sizes up the
//  dominator tree in ascending post-order (leaves first, root last).
//
//  Uses self_size only (not native_size), matching the reference
//  implementation's calculateRetainedSizesFromDominatorNodes.
// ============================================================================
static void compute_retained_sizes(
    DomContext& ctx,
    const PostOrderResult& po,
    const std::vector<uint32_t>& dominators,
    RetainedSizeResult& result)
{
    // ---- Initialize: each node's retained size starts as its self_size ----
    std::vector<double> retained_sizes(ctx.node_count, 0.0);
    double total_self_size = 0.0;

    for (uint32_t i = 0; i < ctx.node_count; i++) {
        double s = static_cast<double>(self_size_of(ctx.nodes, i, ctx.node_fields_count, ctx.self_size_off));
        retained_sizes[i] = s;
        total_self_size += s;
    }

    // ---- Compute reachable self_size for statistics ----
    double reachable_self_size = 0.0;
    for (uint32_t i = 0; i < po.reachable_count; i++) {
        reachable_self_size += static_cast<double>(
            self_size_of(ctx.nodes, po.post_order[i], ctx.node_fields_count, ctx.self_size_off));
    }

    // ---- Propagate retained sizes up the dominator tree ----
    // Iterate in ascending post-order (leaves first).
    // For each node, add its retained size to its immediate dominator.
    // Since we process in post-order, a node's retained size is fully
    // computed (all descendants already added) before it's propagated.
    for (uint32_t i = 0; i < po.root_po; i++) {
        uint32_t node_index = po.post_order[i];
        uint32_t dom_node_index = dominators[node_index];
        if (dom_node_index != UNREACHABLE) {
            retained_sizes[dom_node_index] += retained_sizes[node_index];
        }
    }

    // ---- Populate result ----
    result.retained_sizes = std::move(retained_sizes);
    result.dominators = dominators;
    result.post_order.assign(po.post_order.begin(), po.post_order.begin() + ctx.node_count);
    result.reachable = po.visited;
    result.reachable_count = po.reachable_count;
    result.total_self_size = total_self_size;
    result.reachable_self_size = reachable_self_size;
    result.root_retained_size = result.retained_sizes[ROOT];
}

} // anonymous namespace

// ============================================================================
//  Public API: compute_node_sizes
//
//  Orchestrates the four steps of dominator-based retained size computation:
//    0. page_flags (marks page-owned objects)
//    1. DFS post-order (for retained size propagation)
//    2. Reverse CSR (for predecessor iteration)
//    3. LT dominator (computes the dominator tree)
//    4. Retained size propagation (post-order prefix sum)
// ============================================================================

RetainedSizeResult compute_node_sizes(const HeapSnapshot& snapshot,
                                         bool exclude_weak_edges,
                                         bool exclude_shortcut_edges) {
    StageTimer timer("NodeSizes");

    // ---- Build the context struct from snapshot data ----
    DomContext ctx{
        snapshot.nodes,
        snapshot.edges,
        snapshot.strings,
        snapshot.node_type_names,
        snapshot.first_edge_pointers,
        snapshot.node_fields_count,
        snapshot.edge_fields_count,
        snapshot.node_self_size_offset,
        snapshot.node_native_size_offset,
        snapshot.edge_type_offset,
        snapshot.edge_name_or_index_offset,
        snapshot.edge_to_node_offset,
        snapshot.node_name_offset,
        snapshot.node_type_offset,
        exclude_weak_edges ? snapshot.edge_weak_type : -1,
        exclude_shortcut_edges ? snapshot.edge_shortcut_type : -1,
        snapshot.edge_element_type,
        static_cast<uint32_t>(snapshot.node_count),
    };

    if (ctx.node_count == 0) return {};

    RetainedSizeResult result;
    result.node_count = ctx.node_count;

    // ---- Execute the pipeline ----
    std::vector<uint32_t> page_flags;
    compute_page_flags(ctx, page_flags, timer);

    auto po = compute_dfs_post_order(ctx, page_flags, timer);

    auto rev = build_reverse_csr(ctx, timer);

    auto dominators = compute_lt_dominators(ctx, page_flags, rev, timer);

    compute_retained_sizes(ctx, po, dominators, result);

    timer.done();
    return result;
}

// ============================================================================
//  Shortest paths BFS — context and helpers
//
//  Computes the shortest reference chain from GC roots to every reachable
//  node.  Uses a layered BFS with priority queues, matching the reference
//  implementation's annotateShortestPaths.
//
//  Key design decisions:
//    - Layer-by-layer processing with LIFO pop within each layer
//    - 'queued' flag prevents nodes from being targeted by multiple edges
//    - Postpone queue for low-priority edges/nodes (grey lists)
//    - Fallback to low-priority roots when normal queue is exhausted
// ============================================================================

namespace {

// 查表越界时返回的空字符串（const std::string& 返回路径需要稳定的引用，
// 不能返回临时对象 ""，否则悬垂）。
const std::string kEmptyString;

/// Context struct for the BFS, mirroring DomContext for the dominator pipeline.
struct BfsContext {
    const std::vector<uint32_t>& edges;
    const std::vector<uint32_t>& nodes;
    const std::vector<std::string>& strings;
    const std::vector<std::string>& node_type_names;
    const std::vector<uint32_t>& first_edge_pointers;
    int edge_fields_count;
    int edge_type_off;
    int edge_name_off;
    int to_node_off;
    int node_fields_count;
    int node_type_off;
    int name_off;
    int id_off;            // offset of node id within node record
    int weak_type;
    int shortcut_type;
    int hidden_type;
    int internal_type;
    int element_type;
    uint32_t node_count;

    /// Look up an edge's name from the string table.
    const std::string& get_edge_name(size_t ptr) const {
        uint32_t name_idx = edges[ptr + static_cast<size_t>(edge_name_off)];
        return (name_idx < strings.size()) ? strings[name_idx] : kEmptyString;
    }

    /// Look up a node's name from the string table.
    const std::string& get_node_name(uint32_t idx) const {
        if (idx >= node_count) return kEmptyString;
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(node_fields_count);
        uint32_t ni = nodes[base + static_cast<size_t>(name_off)];
        return (ni < strings.size()) ? strings[ni] : kEmptyString;
    }

    /// Look up a node's type name (e.g. "object", "string", "code").
    const std::string& get_node_type(uint32_t idx) const {
        if (idx >= node_count || node_type_names.empty()) return kEmptyString;
        size_t base = static_cast<size_t>(idx) * static_cast<size_t>(node_fields_count);
        uint32_t ti = nodes[base + static_cast<size_t>(node_type_off)];
        return (ti < node_type_names.size()) ? node_type_names[ti] : kEmptyString;
    }
};

// ---- Block lists (matching the reference implementation's Config) ----

/// Edge name block list: edges with these names are never traversed.
static bool is_edge_name_blocked(const std::string& name) {
    return name == "feedback_cell" ||
           name == "part of key -> value pair in ephemeron table";
}

/// Node name block list: nodes with these names are skipped entirely.
static bool is_node_name_blocked(const std::string& name) {
    return name == "system / PropertyCell";
}

// ---- Grey lists (low priority) ----
// Edges/nodes matching these are deferred to the postpone queue.

static const std::unordered_set<std::string> edge_name_grey_list = {
    "alternate", "firstEffect", "lastEffect"
};

static const std::unordered_set<std::string> node_name_grey_list = {
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

/// Check if a node is a "C++ roots" synthetic node (low priority).
static bool is_cpp_roots_node(const BfsContext& ctx, uint32_t idx) {
    return ctx.get_node_name(idx) == "C++ roots" && ctx.get_node_type(idx) == "synthetic";
}

// ============================================================================
//  Collect root nodes (high-priority + low-priority)
//
//  Matches the reference implementation's getRootNodeList with prioritize:true.
//  High-priority roots are processed first; low-priority roots are only used
//  as a fallback when the normal queue is empty.
//
//  Root classification:
//    High-priority:
//      - Node 0 (SyntheticRoot): always
//      - id == 0 or id == 1
//      - type == "native" && name starts with "Window"
//      - type == "synthetic" && name == "(GC roots)"
//    Low-priority:
//      - type == "synthetic" && name is Blink roots
//      - (synthetic || native) && name == "Pending activities"
// ============================================================================
static void collect_shortest_path_roots(
    const BfsContext& ctx,
    std::vector<uint32_t>& high_pri,
    std::vector<uint32_t>& low_pri)
{
    // Node 0 (SyntheticRoot) is always a high-priority root
    high_pri.push_back(0);

    for (uint32_t i = 1; i < ctx.node_count; i++) {
        uint32_t nid = ctx.nodes[i * static_cast<size_t>(ctx.node_fields_count)
                               + static_cast<size_t>(ctx.id_off)];

        std::string type_name = ctx.get_node_type(i);
        std::string node_name = ctx.get_node_name(i);

        // High-priority root check (excludeBlinkRoot=true, excludePendingActivity=true)
        if (nid == 0 || nid == 1) {
            high_pri.push_back(i);
        } else if (type_name == "native" && node_name.find("Window") == 0) {
            high_pri.push_back(i);
        } else if (type_name == "synthetic" && node_name == "(GC roots)") {
            high_pri.push_back(i);
        }
        // Low-priority root check: Blink cross-thread roots, Blink roots, Pending activities
        else if (type_name == "synthetic" &&
                 (node_name == "Blink cross-thread roots" || node_name == "Blink roots")) {
            low_pri.push_back(i);
        } else if ((type_name == "synthetic" || type_name == "native") &&
                    node_name == "Pending activities") {
            low_pri.push_back(i);
        }
    }
}

// ============================================================================
//  Run layered BFS from roots
//
//  Implements the reference implementation's annotateShortestPaths:
//    1. Layer-by-layer processing with LIFO pop within each layer
//    2. Edge filtering: weak, shortcut, hidden, self-ref, blocklists,
//       WeakMap key edges, internal "code", element (map descriptors),
//       isMeaningfulNode checks
//    3. Postpone queue for low-priority edges/nodes (grey lists)
//    4. Fallback to postpone queue when normal queue is empty
//    5. Fallback to low-priority roots when both are empty
// ============================================================================
static void run_layered_bfs(
    BfsContext& ctx,
    std::vector<uint32_t>& high_pri_roots,
    std::vector<uint32_t>& low_pri_roots,
    ShortestPathsResult& result)
{
    constexpr uint32_t INVALID = 0xFFFFFFFF;

    // 'queued' is set immediately when a target is enqueued, matching the
    // reference implementation's queued[toNode.nodeIndex] = 1.
    // This is separate from hasPathEdge — nodes in the postpone queue have
    // queued[] = 1 but NOT hasPathEdge yet.
    std::vector<uint8_t> queued(ctx.node_count, 0);
    std::vector<uint32_t> cur_queue;

    // Enqueue high-priority roots in node-index order.
    // pop_back() then processes the highest-index root first (LIFO),
    // matching the reference implementation's curQueue.pop().
    for (uint32_t idx : high_pri_roots) {
        result.path_edges[idx] = {idx, INVALID};  // root points to itself
        cur_queue.push_back(idx);
        queued[idx] = 1;
        result.reachable_count++;
    }

    // Postpone queue: (parent_node_index, edge_field_start)
    std::vector<std::pair<uint32_t, size_t>> postpone_queue;

    while (!cur_queue.empty()) {
        std::vector<uint32_t> next_queue;

        // ---- Process current layer (LIFO) ----
        while (!cur_queue.empty()) {
            uint32_t cur = cur_queue.back();
            cur_queue.pop_back();

            size_t begin = ctx.first_edge_pointers[cur];
            size_t end = ctx.first_edge_pointers[static_cast<size_t>(cur) + 1];

            for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(ctx.edge_fields_count)) {
                int t = static_cast<int>(ctx.edges[ptr + static_cast<size_t>(ctx.edge_type_off)]);

                // ---- Basic edge-type exclusions ----
                if (ctx.weak_type >= 0 && t == ctx.weak_type) continue;
                if (ctx.shortcut_type >= 0 && t == ctx.shortcut_type && cur != 0) continue;
                if (ctx.hidden_type >= 0 && t == ctx.hidden_type) continue;

                uint32_t child = ctx.edges[ptr + static_cast<size_t>(ctx.to_node_off)]
                                / static_cast<uint32_t>(ctx.node_fields_count);
                if (child >= ctx.node_count) continue;

                // ---- isMeaningfulEdge checks (matching the reference implementation) ----

                if (cur == child) continue;  // self-reference

                // Edge name block list + WeakMap key edge check
                // WeakMap edges: keys are weakly referenced; skip the forward direction
                // where the edge name contains "@<key_id>) ->" and key_id == target.id
                {
                    const std::string& en = ctx.get_edge_name(ptr);
                    if (is_edge_name_blocked(en)) continue;
                    if (en.find("WeakMap") != std::string::npos) {
                        static const std::regex key_re(R"(@(\d+)\) ->)");
                        std::smatch match;
                        if (std::regex_search(en, match, key_re)) {
                            uint32_t key_id = 0xFFFFFFFFu;
                            try {
                                key_id = static_cast<uint32_t>(std::stoul(match[1].str()));
                            } catch (const std::exception&) {
                                // 越界/非法 key id：不当作 WeakMap key 匹配
                            }
                            uint32_t child_id = ctx.nodes[static_cast<size_t>(child) * static_cast<size_t>(ctx.node_fields_count)
                                                       + static_cast<size_t>(ctx.id_off)];
                            if (key_id == child_id) continue;
                        }
                    }
                }

                // Already queued — prevents multi-parent targeting
                if (queued[child]) continue;

                // Internal edge named "code" — V8 internal, not meaningful for leak analysis
                if (ctx.internal_type >= 0 && t == ctx.internal_type) {
                    const std::string& en = ctx.get_edge_name(ptr);
                    if (en == "code") continue;
                }

                // Element edge to (map descriptors): only specific indices are meaningful.
                // In V8, (map descriptors) are fixed-length arrays; only elements at
                // index < 2 or index % 3 != 1 hold representative references.
                if (ctx.element_type >= 0 && t == ctx.element_type) {
                    const std::string& child_name = ctx.get_node_name(child);
                    if (child_name == "(map descriptors)") {
                        uint32_t index = ctx.edges[ptr + static_cast<size_t>(ctx.edge_name_off)];
                        if (index >= 2 || index % 3 == 1) continue;
                    }
                }

                // ---- isMeaningfulNode check (applied to the TARGET node) ----
                {
                    const std::string& child_name = ctx.get_node_name(child);
                    if (child_name == "system / NativeContext" ||
                        child_name == "system / SourcePositionTableWithFrameCache" ||
                        child_name == "(map descriptors)") continue;
                    if (ctx.get_node_type(child) == "code") continue;
                    if (is_node_name_blocked(child_name)) continue;
                }

                // ---- Priority checks (isLessPreferableEdge / isLessPreferableNode) ----
                bool is_low_priority = false;

                // Edge name in grey list → defer
                {
                    const std::string& en = ctx.get_edge_name(ptr);
                    if (edge_name_grey_list.count(en)) is_low_priority = true;
                }

                // Node name in grey list → defer
                if (!is_low_priority) {
                    const std::string& child_name = ctx.get_node_name(child);
                    if (node_name_grey_list.count(child_name)) is_low_priority = true;
                }

                // C++ roots synthetic node → defer
                if (!is_low_priority && is_cpp_roots_node(ctx, child)) {
                    is_low_priority = true;
                }

                // ---- Enqueue ----
                if (is_low_priority) {
                    // Deferred: add to postpone queue (queued but no path edge yet)
                    postpone_queue.emplace_back(cur, ptr);
                    queued[child] = 1;
                } else {
                    // Normal priority: assign path edge and add to next layer
                    result.path_edges[child] = {cur, static_cast<uint32_t>(ptr)};
                    next_queue.push_back(child);
                    queued[child] = 1;
                }
            }
        }

        // ---- Fallback 1: process postponed queue ----
        // When no normal-priority nodes were found in this layer,
        // promote the postponed edges to the next layer.
        while (next_queue.empty() && !postpone_queue.empty()) {
            auto [parent, ptr] = postpone_queue.back();
            postpone_queue.pop_back();
            uint32_t child = target_index_of(ctx.edges, ptr, ctx.to_node_off, ctx.node_fields_count);
            if (result.path_edges[child].parent_node_index == INVALID) {
                result.path_edges[child] = {parent, static_cast<uint32_t>(ptr)};
                next_queue.push_back(child);
            }
        }

        // ---- Fallback 2: process low-priority roots ----
        // When both normal and postponed queues are empty,
        // promote low-priority roots to the next layer.
        while (next_queue.empty() && !low_pri_roots.empty()) {
            uint32_t idx = low_pri_roots.back();
            low_pri_roots.pop_back();
            if (!queued[idx]) {
                result.path_edges[idx] = {idx, INVALID};
                next_queue.push_back(idx);
                queued[idx] = 1;
                result.reachable_count++;
            }
        }

        cur_queue = std::move(next_queue);
    }
}

} // anonymous namespace

// ============================================================================
//  Public API: compute_shortest_paths
// ============================================================================

ShortestPathsResult compute_shortest_paths(const HeapSnapshot& snapshot,
                                           bool exclude_weak_edges,
                                           bool exclude_shortcut_edges,
                                           bool exclude_hidden_edges) {
    StageTimer timer("ShortPaths");

    BfsContext ctx{
        snapshot.edges,
        snapshot.nodes,
        snapshot.strings,
        snapshot.node_type_names,
        snapshot.first_edge_pointers,
        snapshot.edge_fields_count,
        snapshot.edge_type_offset,
        snapshot.edge_name_or_index_offset,
        snapshot.edge_to_node_offset,
        snapshot.node_fields_count,
        snapshot.node_type_offset,
        snapshot.node_name_offset,
        snapshot.node_id_offset,
        exclude_weak_edges ? snapshot.edge_weak_type : -1,
        exclude_shortcut_edges ? snapshot.edge_shortcut_type : -1,
        exclude_hidden_edges ? snapshot.edge_hidden_type : -1,
        snapshot.edge_internal_type,
        snapshot.edge_element_type,
        static_cast<uint32_t>(snapshot.node_count),
    };

    constexpr uint32_t INVALID = 0xFFFFFFFF;
    ShortestPathsResult result;
    // 与 compute_node_sizes 对称：空快照直接返回，避免后面 collect_shortest_path_roots
    // 把节点 0 当根、对空 vector 越界写。
    if (ctx.node_count == 0) return result;
    result.path_edges.resize(ctx.node_count, {INVALID, INVALID});

    std::vector<uint32_t> high_pri_roots, low_pri_roots;
    collect_shortest_path_roots(ctx, high_pri_roots, low_pri_roots);

    run_layered_bfs(ctx, high_pri_roots, low_pri_roots, result);

    timer.tick("bfs", std::to_string(result.reachable_count) + " reachable");
    timer.done();
    return result;
}

} // namespace mem_analyzer