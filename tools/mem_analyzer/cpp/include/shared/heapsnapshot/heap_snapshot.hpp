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
    std::size_t operator()(const RefChainKey& k) const {
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
    HeapSnapshot() : ext_names_once_(std::make_unique<std::once_flag>()),
                     chain_hash_once_(std::make_unique<std::once_flag>()) {}

    /// Node field offsets (discovered from meta.node_fields)
    int node_type_offset = -1;        // type field index within node record
    int node_name_offset = -1;        // name field index (string table index)
    int node_id_offset = -1;          // id field index
    int node_self_size_offset = -1;   // self_size field index
    int node_native_size_offset = -1; // native_size field index (V8 heap snapshots)
    int node_edge_count_offset = -1;  // edge_count field index
    int node_fields_count = 0;        // total fields per node
    int edge_fields_count = 0;        // total fields per edge
    int edge_type_offset = -1;        // type field index within edge record
    int edge_to_node_offset = -1;     // to_node field index
    int edge_name_or_index_offset = -1; // name_or_index field index (the remaining edge field)
    int edge_weak_type = -1;          // numeric type value for "weak" edges, or -1
    int edge_shortcut_type = -1;       // numeric type value for "shortcut" edges, or -1
    int edge_hidden_type = -1;         // numeric type value for "hidden" edges, or -1
    int edge_element_type = -1;        // numeric type value for "element" edges, or -1
    int edge_internal_type = -1;       // numeric type value for "internal" edges, or -1

    // Raw data
    std::vector<uint32_t> nodes;               // flat node field array
    std::vector<uint32_t> edges;               // flat edge field array
    std::vector<uint32_t> first_edge_pointers;  // CSR: for node i, edges start at first_edge_pointers[i]
    std::vector<std::string> strings;           // string table
    std::vector<std::string> node_type_names;    // type names from meta.node_types (e.g. "object", "string")

    int node_count = 0;
    int edge_count = 0;

    /// nodeId → nodeIndex lookup
    std::unordered_map<uint32_t, uint32_t> node_id_to_idx;

    /// SyntheticRoot 节点索引缓存（from_file 阶段一次性定位），
    /// 使 get_total_heap_size 由 O(N) 扫描降为 O(1)。UINT32_MAX = 未找到。
    uint32_t synthetic_root_index = UINT32_MAX;

    // ========================================================================
    //  Computed results (populated automatically during from_file)
    //  Populated during from_file.
    // ========================================================================

    /// Retained sizes per node index: self_size + dominated descendants' retained sizes
    /// (self_size only; native_size is NOT added — see node_size_calculator.cpp).
    std::vector<double> retained_sizes;

    /// Immediate dominator per node index (UINT32_MAX = unreachable).
    std::vector<uint32_t> dominators;

    /// Shortest-path parent node index (UINT32_MAX = root or unreachable).
    std::vector<uint32_t> path_parent_index;

    /// Shortest-path incoming edge field start (UINT32_MAX = root).
    std::vector<uint32_t> path_edge_start;

    /// 1 if reachable from GC root.
    std::vector<uint8_t> reachable;

    /// Get retained size for a node (dominator-based; self_size only, native_size excluded).
    double get_retained_size(uint32_t node_index) const {
        if (node_index < retained_sizes.size()) return retained_sizes[node_index];
        return 0.0;
    }

    /// Get the shallow/self size stored on a node.
    uint32_t get_node_self_size(uint32_t node_index) const {
        if (node_self_size_offset < 0 || node_index >= static_cast<uint32_t>(node_count)) return 0;
        return nodes[static_cast<size_t>(node_index) * static_cast<size_t>(node_fields_count)
                     + static_cast<size_t>(node_self_size_offset)];
    }

    /// Get numeric id for a node.
    uint32_t get_node_id(uint32_t node_index) const {
        if (node_id_offset < 0 || node_index >= static_cast<uint32_t>(node_count)) return UINT32_MAX;
        return nodes[static_cast<size_t>(node_index) * static_cast<size_t>(node_fields_count)
                     + static_cast<size_t>(node_id_offset)];
    }

    /// Format shortest reference chain as "RootNode -> NodeA -> TargetNode"
    /// matching TS's getSimpleRefChain() format.
    std::string format_ref_chain(uint32_t node_index) const;

    /// Return the shortest reference chain frame names (root -> target) as a list.
    /// Shares the same path collection as format_ref_chain, but emits structured
    /// frame names instead of a joined " -> " string.
    std::vector<std::string> format_ref_chain_frames(uint32_t node_index) const;

    /// 计算最短引用链的 128-bit 稳定标识（对 target->root 的扩展名序列做双路
    /// FNV hash），不可达返回 {0,0}。作为引用链聚合的分组 key。
    /// 注意：必须 hash 扩展名（名字/类型）序列，而非 node_index（具体对象）序列，
    /// 否则会把「走同一类型路径的多个对象」错误拆成多条单节点链。
    RefChainKey ref_chain_hash(uint32_t node_index) const;

    /// Result of getNodeById
    struct NodeView {
        uint32_t node_index;
        uint32_t id;
        std::string name;
        uint32_t self_size;
    };

    /// Lookup a node by its numeric id
    NodeView get_node_by_id(uint32_t id) const;

    /// Get node name from strings table
    std::string get_node_name(uint32_t node_index) const;

    // Get ArkInternalHash value from node's edge (returns 0 if not found)
    uint32_t get_node_internal_hash(uint32_t node_index) const;

    /// Build HeapSnapshot from a .heapsnapshot file
    static HeapSnapshot from_file(const std::string& path);

    /// Get type name for node at node_index (e.g. "object", "string", "number")
    std::string get_node_type_name(uint32_t node_index) const;

    /// Number of outgoing edges for a given node
    uint32_t get_node_edge_count(uint32_t node_index) const;

    // 获取节点的扩展名称
    const std::string& get_extended_node_name(uint32_t node_index) const;

    /// Per-edge info exposed via for_each_edge callback
    struct EdgeInfo {
        uint32_t type;              // numeric type index (matches edge_types meta)
        uint32_t name_or_index;     // edge name (string table index) or numeric index
        uint32_t to_node;           // target node index (edges[to_node_off] / node_fields_count)
        uint32_t to_node_id;        // target node id (decoded from to_node)
    };

    /// Iterate outgoing edges of node at node_index.
    /// Callback receives (const EdgeInfo&, size_t local_edge_index).
    template<typename Fn>
    void for_each_edge(uint32_t node_index, Fn&& fn) const {
        size_t begin = first_edge_pointers[node_index];
        size_t end = first_edge_pointers[static_cast<size_t>(node_index) + 1];
        const size_t edges_size = edges.size();
        const size_t nodes_size = nodes.size();
        size_t local_idx = 0;
        for (size_t ptr = begin; ptr < end; ptr += static_cast<size_t>(edge_fields_count)) {
            if (ptr + static_cast<size_t>(edge_fields_count) > edges_size) break;
            EdgeInfo edge;
            edge.type = edges[ptr + static_cast<size_t>(edge_type_offset)];
            edge.name_or_index = edges[ptr + static_cast<size_t>(edge_name_or_index_offset)];
            edge.to_node = edges[ptr + static_cast<size_t>(edge_to_node_offset)]
                           / static_cast<uint32_t>(node_fields_count);
            if (edge.to_node < static_cast<uint32_t>(node_count) &&
                edge.to_node * static_cast<uint32_t>(node_fields_count) + static_cast<size_t>(node_id_offset) < nodes_size) {
                edge.to_node_id = nodes[edge.to_node * static_cast<uint32_t>(node_fields_count)
                                        + static_cast<size_t>(node_id_offset)];
                fn(edge, local_idx);
            }
            local_idx++;
        }
    }

    uint64_t get_total_heap_size() const {
        if (synthetic_root_index == UINT32_MAX) return 0;
        return static_cast<uint64_t>(get_retained_size(synthetic_root_index));
    }

private:
    // 获取 JSObject 扩展名
    std::string get_js_object_ext_name(uint32_t node_index) const;

    // 获取 String 扩展名
    std::string get_string_ext_name(uint32_t node_index) const;

    // 获取 stateMgmt.js#observedClass 全局对象的扩展名（沿原型链提取 ArkTS 类名）
    std::string get_observed_class_ext_name(uint32_t node_index) const;

    // 沿 hclass → Proto 交替上溯，提取 stateMgmt 包装对象背后被包装的类名。
    // skip_markers 中的名字视为「仍是包装层」，跳过并继续上溯。返回空 = 未命中。
    std::string resolve_state_mgmt_wrapped_class(uint32_t node_index,
                                                 const std::vector<std::string>& skip_markers) const;

    // 获取 stateMgmt.js#JSObject 的扩展名（同 observedClass 的套路，展开被包装的 ArkTS 类）
    std::string get_state_mgmt_js_object_ext_name(uint32_t node_index,
                                                  const std::string& node_name) const;

    // declarative_frontend/stateMgmt.js 对象扩展名的统一入口
    // （observedClass 展开 / JSObject 展开 / 其余去前缀）
    std::string get_state_mgmt_ext_name(uint32_t node_index, const std::string& node_name) const;

    // 获取 SourceTextModule 扩展名
    std::string get_source_text_module_ext_name(uint32_t node_index) const;

    // 计算单个节点的扩展名（不查缓存、不递归调用 get_extended_node_name）。
    // 由 get_extended_node_name 在预构建阶段串行调用。
    std::string compute_extended_name(uint32_t node_index) const;

    // 一次性预构建的扩展名表（按 node_index 索引）。并行阶段只读、无锁。
    // once_flag 用 unique_ptr 持有（once_flag 不可拷贝/移动）。
    mutable std::unique_ptr<std::once_flag> ext_names_once_;
    mutable std::vector<std::string> ext_names_;

    // 一次性预构建的「正向链 hash」表（按 node_index 索引，存 root→node 的扩展名序列
    // 的 FNV 指纹）。ref_chain_hash 直接查表返回 O(1)；方向为正向才能前缀增量递推。
    mutable std::unique_ptr<std::once_flag> chain_hash_once_;
    mutable std::vector<RefChainKey> chain_hash_;
    mutable std::vector<uint8_t> chain_hash_computed_;
};

} // namespace mem_analyzer
