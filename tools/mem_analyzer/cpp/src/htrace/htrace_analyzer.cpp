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
#include "htrace/htrace_analyzer.hpp"
#include "htrace/htrace_category.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include "config.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <variant>

namespace mem_analyzer {

// ===== N 轨 section 构造（HtraceRow 是 variant，构造时按同质轨装箱）=====
static HtraceSection make_symbol_section(std::vector<TopSymbolInfo> syms,
                                         const char* key = "", const char* label = "") {
    HtraceSection s;
    s.key = key;
    s.label = label;
    s.rows.reserve(syms.size());
    for (auto& t : syms) s.rows.emplace_back(std::move(t));
    return s;
}

static HtraceSection make_filepage_section(std::vector<FilepageInfo> fps,
                                           const char* key, const char* label) {
    HtraceSection s;
    s.key = key;
    s.label = label;
    s.rows.reserve(fps.size());
    for (auto& f : fps) s.rows.emplace_back(std::move(f));
    return s;
}

// 将 core 单轨符号结果（key=="" 的 section）改造为双轨：第一轨 = anon 符号，第二轨 = filepage。
// 三个 scope 恒写两轨（空也写），与旧 has_mmap_split 行为的 JSON 键结构字节一致。
static void attach_filepages(ScopeResult& scope, std::vector<FilepageInfo> fps,
                             const char* anon_label, const char* fp_label) {
    if (scope.sections.empty()) {
        scope.sections.push_back(make_symbol_section({}, "mmapAnons", anon_label));
    } else {
        scope.sections[0].key = "mmapAnons";
        scope.sections[0].label = anon_label;
    }
    scope.sections.push_back(make_filepage_section(std::move(fps), "mmapFilepages", fp_label));
}

// ===== ordered_set: vector (insertion order) + unordered_set (fast lookup) =====
template<typename T>
struct ordered_set {
    std::vector<T> items;
    std::unordered_set<T> set;

    void push_back(const T& val) {
        items.push_back(val);
        set.insert(val);
    }

    bool contains(const T& val) const {
        return set.find(val) != set.end();
    }

    std::size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear() { items.clear(); set.clear(); }

    // Range-based for support (iterate in insertion order)
    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
    auto begin() { return items.begin(); }
    auto end() { return items.end(); }

    // Pass to functions expecting const std::vector<T>&
    const std::vector<T>& vector() const { return items; }
};

// ===== Internal structures =====
struct SymbolData {
    uint64_t uid = 0;                    // (symbol_id << 32) | file_id
    int lib_id = 0;                    // file_id, 0 means null/unknown
    bool lib_id_null = true;
    std::string type;
    long long total_leak_size = 0;
    int max_depth = 0;
    uint64_t vaddr = 0;                  // symbol vaddr (display use)
    bool vaddr_null = true;
    bool is_app = false;                 // cached result of is_app_symbol()
    int build_id = 0;                    // library build_id (data_dict index, library-level, from first seen frame)
    bool build_id_null = true;
    ordered_set<int> chain_ids;          // insertion order + fast O(1) membership
    std::unordered_map<int, int> depth_per_chain;
};

// Helper: make uid from (symbol_id, file_id)
inline uint64_t make_uid(int symbol_id, int file_id) {
    return ((uint64_t)symbol_id << 32) | (uint32_t)file_id;
}

// ===== Helper: check if a symbol is an app symbol =====
// Looks up the lib path via data_dict and delegates to is_app_symbol().
static void check_app_symbol(SymbolData& sd,
                           const std::unordered_map<int, std::string>& data_dict) {
    if (sd.lib_id_null) { sd.is_app = false; return; }
    auto it = data_dict.find(sd.lib_id);
    if (it == data_dict.end()) { sd.is_app = false; return; }
    sd.is_app = HtraceConfigMgr::instance().is_app_symbol(it->second, sd.vaddr_null);
}

// ===== Build initial symbol data from call stacks =====
static void build_symbol_data(
    const std::vector<CompressedCallStack>& call_stacks,
    std::unordered_map<uint64_t, SymbolData>& symbols,
    const std::unordered_map<int, std::string>& data_dict,
    long long& total_leak_size)
{
    total_leak_size = 0;

    for (int ci = 0; ci < (int)call_stacks.size(); ci++) {
        auto& chain = call_stacks[ci];
        long long size = chain.existing_size;
        if (size <= 0) continue;
        total_leak_size += size;

        std::unordered_set<uint64_t> depth_seen;

        for (auto& frame : chain.frames) {
            int sid = frame.symbol_id;
            int fid = frame.file_id_null ? -1 : frame.file_id;
            if (sid == 0) continue;

            uint64_t key = make_uid(sid, fid);

            auto it = symbols.find(key);
            if (it == symbols.end()) {
                SymbolData sd;
                sd.uid = key;
                sd.lib_id = fid;
                sd.lib_id_null = frame.file_id_null;
                sd.type = htrace_event_type_name(chain.type);
                sd.total_leak_size = 0;
                sd.max_depth = 0;
                sd.vaddr_null = frame.vaddr_null;
                if (!frame.vaddr_null) sd.vaddr = frame.vaddr;
                sd.build_id_null = frame.build_id_null;
                if (!frame.build_id_null) sd.build_id = frame.build_id;
                symbols[key] = sd;
                it = symbols.find(key);
            }

            auto& sd = it->second;
            int depth = frame.depth;

            if (depth_seen.find(key) == depth_seen.end()) {
                depth_seen.insert(key);
                sd.chain_ids.push_back(ci);
                sd.depth_per_chain[ci] = depth;
                sd.total_leak_size += size;
                if (depth > sd.max_depth) sd.max_depth = depth;
            }
        }
    }

    for (auto& [uid, sd] : symbols) {
        check_app_symbol(sd, data_dict);
    }
}

// ===== Check if two symbols are equivalent =====
static bool are_equivalent(const SymbolData& a, const SymbolData& b) {
    if (a.chain_ids.size() != b.chain_ids.size()) return false;
    for (int ci : a.chain_ids) {
        if (!b.chain_ids.contains(ci)) return false;
    }
    return true;
}

// ===== Find equivalent groups (deterministic: input sorted by leak size desc) =====
static size_t chain_ids_hash(const ordered_set<int>& ids) {
    size_t h = 0;
    for (int ci : ids) h += std::hash<int>{}(ci);
    return h;
}

struct BucketKey {
    size_t size;
    size_t hash;
    bool operator==(const BucketKey& o) const { return size == o.size && hash == o.hash; }
};
struct BucketKeyHash {
    size_t operator()(const BucketKey& k) const { return k.size ^ (k.hash << 1); }
};

static std::vector<std::vector<SymbolData*>> find_equivalent_groups(
    const std::vector<SymbolData*>& symbols)
{
    // symbols is already sorted by total_leak_size descending by caller

    // Bucket by (chain_ids size, hash) — only symbols in the same bucket can be equivalent
    std::unordered_map<BucketKey, std::vector<SymbolData*>, BucketKeyHash> buckets;
    for (auto* s : symbols) {
        buckets[{s->chain_ids.size(), chain_ids_hash(s->chain_ids)}].push_back(s);
    }

    std::set<uint64_t> visited;
    std::vector<std::vector<SymbolData*>> groups;

    for (auto* s : symbols) {
        if (visited.count(s->uid)) continue;
        std::vector<SymbolData*> group = {s};
        visited.insert(s->uid);

        // Only compare within same bucket (size+hash match)
        auto& bucket = buckets[{s->chain_ids.size(), chain_ids_hash(s->chain_ids)}];
        for (auto* other : bucket) {
            if (visited.count(other->uid)) continue;
            if (are_equivalent(*s, *other)) {
                group.push_back(other);
                visited.insert(other->uid);
            }
        }
        groups.push_back(group);
    }
    return groups;
}

// ===== Calculate approximations =====
// Use insertion-order-preserving containers to match TS Map behavior
struct ApproxResult {
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> up_approx;
    std::unordered_map<uint64_t, size_t> up_idx;
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> down_approx;
    std::unordered_map<uint64_t, size_t> down_idx;
};

// Insertion-order-preserving inner map (int key → long long value)
// Uses vector + index map to keep insertion order while providing O(1) lookup.
struct InnerMap {
    std::vector<std::pair<int, long long>> items;
    std::unordered_map<int, size_t> idx;

    void add(int key, long long val) {
        auto it = idx.find(key);
        if (it == idx.end()) {
            idx[key] = items.size();
            items.emplace_back(key, val);
        } else {
            items[it->second].second += val;
        }
    }

    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
};

// ===== 方案2：近似判定的相对差值阈值 =====
// diff 阈值从「全局绝对量」改为「相对被取代者大小」，允许：
//   diff < min(被取代者 total * appro_rel_ratio / 100, appro_rel_cap)
// 同时保留贡献比例下限（共同链值需占 counterpart 的 appro_ratio_threshold%），
// 避免把只覆盖一小部分的大容器符号误当作近似。
static long long rel_threshold_for(long long replaced_total, const AnalysisConfig& config) {
    long long t = replaced_total * config.appro_rel_ratio / 100;
    return t < config.appro_rel_cap ? t : config.appro_rel_cap;
}

static bool can_approximate(long long value,
                            long long replaced_total,
                            long long counterpart_total,
                            const AnalysisConfig& config) {
    if (replaced_total <= 0) return false;
    long long diff = std::abs(value - replaced_total);
    if (diff > rel_threshold_for(replaced_total, config)) return false;
    if (diff == 0) return true;
    return value > counterpart_total * config.appro_ratio_threshold / 100;
}

static ApproxResult calc_approximations(
    const std::vector<SymbolData*>& filtered,
    const std::vector<CompressedCallStack>& call_stacks,
    const AnalysisConfig& config)
{
    int N = (int)filtered.size();

    // Build uid → index map (int keys for all internal lookups)
    std::unordered_map<uint64_t, int> sym_index;
    sym_index.reserve(N);
    for (int i = 0; i < N; i++) {
        sym_index[filtered[i]->uid] = i;
    }

    // contrib: insertion order = first time each 'deep' is encountered during pair iteration
    std::vector<std::pair<int, InnerMap>> contrib;          // (deep_idx, InnerMap)
    std::unordered_map<int, size_t> contrib_idx;             // deep_idx → position in contrib
    // alloc: insertion order = first time each 'shallow' is encountered
    std::vector<std::pair<int, InnerMap>> alloc;             // (shallow_idx, InnerMap)
    std::unordered_map<int, size_t> alloc_idx;               // shallow_idx → position in alloc

    // Chain cache: (chain_id, vector of (sym_idx, depth))
    std::vector<std::pair<int, std::vector<std::pair<int, int>>>> chain_cache_vec;
    std::unordered_map<int, size_t> chain_cache_idx;

    // Build chain cache (iterates filtered in sorted order to match TS Map insertion order)
    for (auto* sd : filtered) {
        int sym_idx = sym_index[sd->uid];
        for (int ci : sd->chain_ids) {
            size_t idx;
            auto it = chain_cache_idx.find(ci);
            if (it == chain_cache_idx.end()) {
                idx = chain_cache_vec.size();
                chain_cache_idx[ci] = idx;
                chain_cache_vec.emplace_back(ci, std::vector<std::pair<int, int>>());
            } else {
                idx = it->second;
            }
            chain_cache_vec[idx].second.emplace_back(sym_idx, sd->depth_per_chain.at(ci));
        }
    }

    // Pair building — all keys are int now, O(1) hash/comparison
    for (auto& [ci, chain_syms] : chain_cache_vec) {
        if (chain_syms.size() < 2) continue;
        std::stable_sort(chain_syms.begin(), chain_syms.end(),
            [&](const auto& a, const auto& b) {
                if (a.second != b.second) return a.second < b.second;
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });

        if (ci < 0 || ci >= (int)call_stacks.size()) continue;
        long long leak = call_stacks[ci].existing_size;
        if (leak <= 0) continue;

        for (size_t i = 0; i < chain_syms.size(); i++) {
            int shallow_idx = chain_syms[i].first;
            for (size_t j = i + 1; j < chain_syms.size(); j++) {
                int deep_idx = chain_syms[j].first;

                // contrib: insert deep in insertion order
                auto c_it = contrib_idx.find(deep_idx);
                if (c_it == contrib_idx.end()) {
                    contrib_idx[deep_idx] = contrib.size();
                    contrib.emplace_back(deep_idx, InnerMap());
                    c_it = contrib_idx.find(deep_idx);
                }
                contrib[c_it->second].second.add(shallow_idx, leak);

                // alloc: insert shallow in insertion order
                auto a_it = alloc_idx.find(shallow_idx);
                if (a_it == alloc_idx.end()) {
                    alloc_idx[shallow_idx] = alloc.size();
                    alloc.emplace_back(shallow_idx, InnerMap());
                    a_it = alloc_idx.find(shallow_idx);
                }
                alloc[a_it->second].second.add(deep_idx, leak);
            }
        }
    }

    // Sort contrib/alloc and their inner maps deterministically by uid
    for (auto& [deep_idx, c_map] : contrib) {
        std::sort(c_map.items.begin(), c_map.items.end(),
            [&](const auto& a, const auto& b) {
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });
    }
    std::sort(contrib.begin(), contrib.end(),
        [&](const auto& a, const auto& b) {
            return filtered[a.first]->uid < filtered[b.first]->uid;
        });
    for (auto& [shallow_idx, a_map] : alloc) {
        std::sort(a_map.items.begin(), a_map.items.end(),
            [&](const auto& a, const auto& b) {
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });
    }
    std::sort(alloc.begin(), alloc.end(),
        [&](const auto& a, const auto& b) {
            return filtered[a.first]->uid < filtered[b.first]->uid;
        });

    // Evaluation — lookup total_leak_size directly from filtered array by index
    ApproxResult result;

    for (auto& [deep_idx, c_map] : contrib) {
        long long total_deep = filtered[deep_idx]->total_leak_size;
        if (total_deep <= 0) continue;

        for (auto& [shallow_idx, value] : c_map) {
            long long total_shallow = filtered[shallow_idx]->total_leak_size;
            if (total_shallow <= 0) continue;
            if (can_approximate(value, total_deep, total_shallow, config)) {
                auto& deep_uid = filtered[deep_idx]->uid;
                auto& shallow_uid = filtered[shallow_idx]->uid;
                auto u_it = result.up_idx.find(deep_uid);
                if (u_it == result.up_idx.end()) {
                    result.up_idx[deep_uid] = result.up_approx.size();
                    result.up_approx.emplace_back(deep_uid, std::vector<uint64_t>());
                    u_it = result.up_idx.find(deep_uid);
                }
                result.up_approx[u_it->second].second.push_back(shallow_uid);
            }
        }
    }

    for (auto& [shallow_idx, a_map] : alloc) {
        long long total_shallow = filtered[shallow_idx]->total_leak_size;
        if (total_shallow <= 0) continue;

        for (auto& [deep_idx, value] : a_map) {
            long long total_deep = filtered[deep_idx]->total_leak_size;
            if (total_deep <= 0) continue;
            if (can_approximate(value, total_shallow, total_deep, config)) {
                auto& shallow_uid = filtered[shallow_idx]->uid;
                auto& deep_uid = filtered[deep_idx]->uid;
                auto d_it = result.down_idx.find(shallow_uid);
                if (d_it == result.down_idx.end()) {
                    result.down_idx[shallow_uid] = result.down_approx.size();
                    result.down_approx.emplace_back(shallow_uid, std::vector<uint64_t>());
                    d_it = result.down_idx.find(shallow_uid);
                }
                result.down_approx[d_it->second].second.push_back(deep_uid);
            }
        }
    }

    return result;
}

// ===== Algorithm runner =====
struct AlgorithmResult {
    std::vector<SymbolData> final_symbols;
    AlgorithmStats stats;
};

static AlgorithmResult run_algorithm(
    const std::vector<SymbolData*>& filtered,
    const std::vector<CompressedCallStack>& call_stacks,
    const AnalysisConfig& config)
{
    StageTimer timer("Algo");

    AlgorithmResult alg_result;
    int org_set_size = 0;     // set by caller
    int filtered_set_size = (int)filtered.size();

    // 4.2: Track removed symbols (no deep copy of SymbolData)
    std::unordered_set<uint64_t> removed_uids;

    // 4.3: Equivalent symbols
    timer.stage("Finding equivalent groups");
    auto equiv_groups = find_equivalent_groups(filtered);
    std::unordered_map<uint64_t, uint64_t> equiv_ref_map;
    int removed_equiv = 0;
    for (auto& group : equiv_groups) {
        if (group.size() <= 1) continue;
        // Find ref (deepest depth, tiebreaker: smallest uid)
        SymbolData* ref = group[0];
        for (auto* s : group) {
            if (s->max_depth > ref->max_depth ||
                (s->max_depth == ref->max_depth && s->uid < ref->uid)) ref = s;
        }
        for (auto* s : group) {
            if (s->uid != ref->uid) {
                equiv_ref_map[s->uid] = ref->uid;
                removed_uids.insert(s->uid);
                removed_equiv++;
            }
        }
    }
    timer.tick("equiv");

    // 4.5: Calculate approximations on filtered set
    timer.stage("Calculating approximations");
    auto approx = calc_approximations(filtered, call_stacks, config);
    timer.tick("approx");

    // 4.6: Filter using approximations (order-independent fixed-point iteration)
    int down_removed = 0;
    int up_removed = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [sym, down_set] : approx.down_approx) {
            if (removed_uids.count(sym)) continue;
            for (auto& ds : down_set) {
                if (!removed_uids.count(ds)) {
                    removed_uids.insert(sym);
                    down_removed++;
                    changed = true;
                    break;
                }
            }
        }
        for (auto& [sym, up_set] : approx.up_approx) {
            if (removed_uids.count(sym)) continue;
            for (auto& us : up_set) {
                if (!removed_uids.count(us)) {
                    removed_uids.insert(sym);
                    up_removed++;
                    changed = true;
                    break;
                }
            }
        }
    }

    // 4.7: Check representation
    int representation_added = 0;
    for (auto* s : filtered) {
        if (!removed_uids.count(s->uid)) continue;
        bool in_surviving = false;
        auto u_it = approx.up_idx.find(s->uid);
        if (u_it != approx.up_idx.end() && u_it->second < approx.up_approx.size()) {
            for (auto& u : approx.up_approx[u_it->second].second) { if (!removed_uids.count(u)) { in_surviving = true; break; } }
        }
        if (!in_surviving) {
            auto d_it = approx.down_idx.find(s->uid);
            if (d_it != approx.down_idx.end() && d_it->second < approx.down_approx.size()) {
                for (auto& d : approx.down_approx[d_it->second].second) { if (!removed_uids.count(d)) { in_surviving = true; break; } }
            }
        }
        if (!in_surviving) {
            auto eq_it = equiv_ref_map.find(s->uid);
            if (eq_it != equiv_ref_map.end() && !removed_uids.count(eq_it->second)) {
                in_surviving = true;
            }
        }
        if (!in_surviving) {
            removed_uids.erase(s->uid);
            representation_added++;
        }
    }

    // 4.8: Build final result — only copy surviving symbols
    for (auto* s : filtered) {
        if (!removed_uids.count(s->uid))
            alg_result.final_symbols.push_back(*s);
    }
    std::stable_sort(alg_result.final_symbols.begin(), alg_result.final_symbols.end(),
        [](const auto& a, const auto& b) {
            if (a.total_leak_size != b.total_leak_size)
                return a.total_leak_size > b.total_leak_size;
            return a.uid < b.uid;
        });
    timer.tick("filter",
        std::to_string(filtered_set_size) + " in, " + std::to_string(alg_result.final_symbols.size()) + " out");

    int equiv_group_count = 0;
    for (auto& g : equiv_groups) if (g.size() > 1) equiv_group_count++;

    alg_result.stats = {
        org_set_size,
        filtered_set_size,
        (int)alg_result.final_symbols.size(),
        equiv_group_count,
        up_removed,
        down_removed,
        representation_added
    };

    return alg_result;
}

// ===== Build flame/icicle graphs and call chains for top-N symbols only =====
static std::vector<TopSymbolInfo> build_and_enrich(
    const std::vector<SymbolData>& final_symbols,
    const std::vector<CompressedCallStack>& call_stacks,
    const std::unordered_map<int, std::string>& data_dict,
    int top_n)
{
    StageTimer timer("Analysis");

    timer.stage("Building flame/icicle graphs for top symbols");
    std::vector<TopSymbolInfo> result;
    int limit = std::min((int)final_symbols.size(), top_n);
    result.reserve(limit);

    for (int i = 0; i < limit; i++) {
        auto& s = final_symbols[i];
        TopSymbolInfo ts;
        ts.uid = s.uid;

        // Resolve symbol/lib from data_dict
        int sid = (int)(s.uid >> 32);
        int fid = (int)(s.uid & 0xFFFFFFFF);
        auto sym_it = data_dict.find(sid);
        ts.symbol = (sym_it != data_dict.end()) ? sym_it->second : "unknown";
        auto lib_it = data_dict.find(fid);
        std::string lib_str = (lib_it != data_dict.end()) ? lib_it->second : "unknown";
        ts.libs = {lib_str};
        // Resolve lib_offset from stored vaddr
        if (!s.vaddr_null) {
            char buf[32];
            snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)s.vaddr);
            ts.lib_offset = buf;
        } else {
            ts.lib_offset = "";
        }
        ts.type = s.type;
        ts.total_leak_size = s.total_leak_size;
        // Sum existing_count over all chains that contain this symbol (one contribution per chain)
        ts.total_existing_count = 0;
        for (int ci : s.chain_ids) {
            if (ci >= 0 && ci < (int)call_stacks.size())
                ts.total_existing_count += call_stacks[ci].existing_count;
        }
        ts.is_app = s.is_app;
        ts.build_id_null = s.build_id_null;
        if (!s.build_id_null) ts.build_id = s.build_id;

        // Build flame/icicle graphs with data_dict for ID→string resolution
        ts.flame_graph = build_flame_graph(ts.symbol, call_stacks, s.chain_ids.vector(), s.depth_per_chain, data_dict);
        ts.icicle_graph = build_icicle_graph(ts.symbol, call_stacks, s.chain_ids.vector(), s.depth_per_chain, data_dict);
        for (int ci : s.chain_ids) {
            if (ci >= 0 && ci < (int)call_stacks.size())
                ts.call_chains.push_back(call_stacks[ci]);
        }
        result.push_back(std::move(ts));
    }

    timer.tick("build_graphs", std::to_string(limit) + " top symbols");

    return result;
}

// ===== Core analysis (no mmap split) =====
static HtraceAnalysisOutput analyze_call_stacks_core(
    const std::vector<CompressedCallStack>& call_stacks,
    const std::unordered_map<int, std::string>& data_dict,
    AnalysisConfig config)
{
    StageTimer timer("Analysis");

    Scope scope = config.use_force_scope ? config.force_scope : config.scope;
    std::string scope_str = scope_to_string(scope);
    int top_n = config.top_n;

    // 单轨 category 的 JSON 嵌套键（来自描述符表 tracks[0].json_key，如 "nativeHeaps"/"dmas"）。
    // 多轨（mmap）已按 is_multipart 分叉、不经过此函数；这里只服务单轨。
    const std::string& symbol_track_key = htrace_category(config.category).tracks[0].json_key;

    // Lazy-load htrace config
    auto& htrace_cfg = HtraceConfigMgr::instance();
    if (!htrace_cfg.is_loaded()) {
        std::string err;
        if (!htrace_cfg.load_default(err)) {
            throw std::runtime_error("Failed to load trace_analyzer_config.json: " + err);
        }
    }

    // Apply algorithm parameters from config
    config.low_threshold = htrace_cfg.algorithm().low_threshold;
    config.appro_rel_ratio = htrace_cfg.algorithm().appro_rel_ratio;
    config.appro_rel_cap = htrace_cfg.algorithm().appro_rel_cap;
    config.appro_ratio_threshold = htrace_cfg.algorithm().appro_ratio_threshold;
    config.orphan_rel_ratio = htrace_cfg.algorithm().orphan_rel_ratio;
    config.orphan_rel_cap = htrace_cfg.algorithm().orphan_rel_cap;

    // fd/thread sizes are counts (not bytes); the byte-based low threshold
    // would wrongly filter them out, so disable it for these types.
    if (htrace_category(config.category).count_mode) {
        config.low_threshold = 0;
    }

    // Build symbol data
    timer.stage("Building symbol data from call stacks");
    std::unordered_map<uint64_t, SymbolData> all_symbols;
    long long total_leak_size;
    build_symbol_data(call_stacks, all_symbols, data_dict, total_leak_size);
    timer.tick("build_symbols", std::to_string(all_symbols.size()) + " symbols");

    HtraceAnalysisOutput output;
    output.total_leak_size = total_leak_size;

    // Owner for orphan-recovered SymbolData copies. Recovered symbols must
    // outlive run_algorithm()/build_and_enrich(), so they are kept here (deque
    // push_back does not invalidate pointers to existing elements).
    std::deque<SymbolData> recovered_owner;

    // Helper: filter symbols by scope and thresholds, return sorted pointer list.
    // Also recovers "orphan" stacks (stacks whose in-scope frames are all noise
    // symbols and would otherwise lose their leak size from the ranking), so the
    // leak size stays visible. Only the noise filter is bypassed for recovery;
    // recovered symbols are gated by an orphan-specific threshold.
    auto build_filtered = [&](Scope filter_scope) -> std::pair<std::vector<SymbolData*>, int> {
        std::vector<SymbolData*> filtered;
        int org_count = 0;

        auto scope_ok = [&](const SymbolData& sd) {
            if (config.leak_mode) return true;  // -l: L3 已分栈，整组符号定性，跳过 is_app 过滤
            if (filter_scope == Scope::AppOnly && !sd.is_app) return false;
            if (filter_scope == Scope::SystemOnly && sd.is_app) return false;
            return true;
        };
        auto resolve_names = [&](const SymbolData& sd, std::string& lib_file, std::string& sym_name) {
            std::string lib_name;
            if (sd.lib_id_null) {
                lib_name = "unknown";
            } else {
                auto lit = data_dict.find(sd.lib_id);
                if (lit != data_dict.end()) lib_name = lit->second;
            }
            lib_file = lib_name.substr(lib_name.find_last_of("/\\") + 1);
            int sid = (int)(sd.uid >> 32);
            auto sit = data_dict.find(sid);
            sym_name = (sit != data_dict.end()) ? sit->second : "";
        };
        auto is_noise = [&](const SymbolData& sd) {
            std::string lib_file, sym_name;
            resolve_names(sd, lib_file, sym_name);
            return HtraceConfigMgr::instance().should_filter(lib_file, sym_name, filter_scope);
        };
        auto is_orphan_recoverable = [&](const SymbolData& sd) {
            std::string lib_file, sym_name;
            resolve_names(sd, lib_file, sym_name);
            return HtraceConfigMgr::instance().is_orphan_recoverable(lib_file, sym_name, filter_scope);
        };

        // Pass 1: symbol-level scope + noise + low_threshold filtering.
        for (auto& kv : all_symbols) {
            SymbolData& sd = kv.second;
            if (!scope_ok(sd)) continue;
            org_count++;
            if (is_noise(sd)) continue;
            // -l(leak_mode): 跳过 low_threshold 过滤，保留所有泄漏符号
            if (config.leak_mode || sd.total_leak_size >= config.low_threshold) {
                filtered.push_back(&sd);
            }
        }

        // Pass 2: recover orphan stacks. An orphan stack has at least one
        // in-scope frame but none that survives the noise filter. Its symbols
        // are re-added so the algorithm can merge and approximate them as
        // usual. Whether a symbol is worth recovering is decided by its leak
        // size aggregated across orphan stacks (an adaptive fraction of the
        // total leak size, capped); once recovered, the symbol keeps its
        // full-chain data (chain_ids / total_leak_size) rather than being
        // restricted to orphan chains, so its reported size stays accurate.
        std::unordered_map<uint64_t, long long> orphan_agg;         // uid -> 孤儿栈内聚合泄漏量
        std::unordered_map<uint64_t, const SymbolData*> orphan_src; // uid -> 原始符号（完整范围）
        for (int ci = 0; ci < (int)call_stacks.size(); ci++) {
            const auto& chain = call_stacks[ci];
            long long size = chain.existing_size;
            if (size <= 0) continue;

            bool has_scope_sym = false;
            bool has_survivor = false;
            for (const auto& frame : chain.frames) {
                if (frame.symbol_id == 0) continue;
                uint64_t key = make_uid(frame.symbol_id, frame.file_id_null ? -1 : frame.file_id);
                auto it = all_symbols.find(key);
                if (it == all_symbols.end()) continue;
                const SymbolData& sd = it->second;
                // 幸存者看整条链：任何 scope 的非噪声符号都意味着泄漏未丢失
                if (!is_noise(sd)) { has_survivor = true; break; }
                // 只有当前 scope 的符号才计入 has_scope_sym（决定恢复归属）
                if (scope_ok(sd)) has_scope_sym = true;
            }
            if (!has_scope_sym || has_survivor) continue;

            // Orphan: aggregate each recoverable in-scope frame's leak size
            // (restricted to this stack; dedup'd by uid).
            std::unordered_set<uint64_t> depth_seen;
            for (const auto& frame : chain.frames) {
                if (frame.symbol_id == 0) continue;
                int fid = frame.file_id_null ? -1 : frame.file_id;
                uint64_t key = make_uid(frame.symbol_id, fid);
                auto it = all_symbols.find(key);
                if (it == all_symbols.end()) continue;
                const SymbolData& src = it->second;
                if (!scope_ok(src)) continue;
                if (!is_orphan_recoverable(src)) continue;
                if (depth_seen.find(key) != depth_seen.end()) continue;
                depth_seen.insert(key);
                orphan_agg[key] += size;
                orphan_src[key] = &src;
            }
        }

        // 孤儿栈兜底显著度门槛 = min(总泄漏量 × orphan_rel_ratio/100, orphan_rel_cap)。
        // 只有孤儿栈内聚合泄漏量达到门槛的符号才恢复，避免引入碎片化干扰。
        long long orphan_threshold = total_leak_size * config.orphan_rel_ratio / 100;
        if (orphan_threshold > config.orphan_rel_cap) {
            orphan_threshold = config.orphan_rel_cap;
        }

        for (auto& kv : orphan_agg) {
            if (kv.second < orphan_threshold) continue;
            // 完整复制原始符号（保留完整 chain_ids / total_leak_size）。
            recovered_owner.push_back(*orphan_src[kv.first]);
            filtered.push_back(&recovered_owner.back());
        }

        std::stable_sort(filtered.begin(), filtered.end(),
            [](const auto* a, const auto* b) {
                if (a->total_leak_size != b->total_leak_size)
                    return a->total_leak_size > b->total_leak_size;
                return a->uid < b->uid;
            });
        return {filtered, org_count};
    };

    // leak_mode: 收集非 top N 覆盖的栈 -> other, 追加到对应 top_symbols 分类
    auto append_other = [&](std::vector<TopSymbolInfo>& target_vec,
                            const std::vector<SymbolData>& final_symbols) {
        if (!config.leak_mode) return;
        int limit = std::min((int)final_symbols.size(), top_n);
        std::unordered_set<int> covered;
        for (int i = 0; i < limit; i++) {
            for (int ci : final_symbols[i].chain_ids) covered.insert(ci);
        }
        std::vector<int> other_ids;
        long long other_leak = 0;
        long long other_count = 0;
        for (int ci = 0; ci < (int)call_stacks.size(); ci++) {
            if (covered.count(ci)) continue;
            if (call_stacks[ci].existing_size <= 0) continue;
            other_ids.push_back(ci);
            other_leak += call_stacks[ci].existing_size;
            other_count += call_stacks[ci].existing_count;
        }
        if (other_ids.empty()) return;
        TopSymbolInfo ts;
        ts.symbol = OTHER_SYMBOL;
        ts.total_leak_size = other_leak;
        ts.total_existing_count = other_count;
        ts.icicle_graph = build_merged_icicle(call_stacks, other_ids, data_dict);
        target_vec.push_back(std::move(ts));
    };

    if (scope_str == "app-only") {
        auto [app_filtered, app_org] = build_filtered(Scope::AppOnly);
        auto result = run_algorithm(app_filtered, call_stacks, config);
        output.top_symbols.mode = "app-only";
        auto app_syms = build_and_enrich(result.final_symbols, call_stacks, data_dict, top_n);
        append_other(app_syms, result.final_symbols);
        output.top_symbols.app.sections.push_back(make_symbol_section(std::move(app_syms), symbol_track_key.c_str()));
        output.stats = result.stats;
        output.stats.org_set_size = app_org;
    }
    else if (scope_str == "system-only") {
        auto [sys_filtered, sys_org] = build_filtered(Scope::SystemOnly);
        auto result = run_algorithm(sys_filtered, call_stacks, config);
        output.top_symbols.mode = "system-only";
        auto sys_syms = build_and_enrich(result.final_symbols, call_stacks, data_dict, top_n);
        append_other(sys_syms, result.final_symbols);
        output.top_symbols.system.sections.push_back(make_symbol_section(std::move(sys_syms), symbol_track_key.c_str()));
        output.stats = result.stats;
        output.stats.org_set_size = sys_org;
    }
    else if (scope_str == "mixed") {
        auto [mixed_filtered, mixed_org] = build_filtered(Scope::Mixed);
        auto result = run_algorithm(mixed_filtered, call_stacks, config);
        output.top_symbols.mode = "mixed";
        auto mixed_syms = build_and_enrich(result.final_symbols, call_stacks, data_dict, top_n);
        append_other(mixed_syms, result.final_symbols);
        output.top_symbols.mixed.sections.push_back(make_symbol_section(std::move(mixed_syms), symbol_track_key.c_str()));
        output.stats = result.stats;
        output.stats.org_set_size = mixed_org;
    }
    else {
        // app-and-system (default)
        auto [app_filtered, app_org] = build_filtered(Scope::AppOnly);
        auto [sys_filtered, sys_org] = build_filtered(Scope::SystemOnly);
        auto app_result = run_algorithm(app_filtered, call_stacks, config);
        auto sys_result = run_algorithm(sys_filtered, call_stacks, config);
        output.top_symbols.mode = "app-and-system";
        auto app_syms = build_and_enrich(app_result.final_symbols, call_stacks, data_dict, top_n);
        append_other(app_syms, app_result.final_symbols);
        output.top_symbols.app.sections.push_back(make_symbol_section(std::move(app_syms), symbol_track_key.c_str()));
        auto sys_syms = build_and_enrich(sys_result.final_symbols, call_stacks, data_dict, top_n);
        append_other(sys_syms, sys_result.final_symbols);
        output.top_symbols.system.sections.push_back(make_symbol_section(std::move(sys_syms), symbol_track_key.c_str()));
        output.stats.org_set_size = app_org + sys_org;
        output.stats.filtered_set_size = app_result.stats.filtered_set_size + sys_result.stats.filtered_set_size;
        output.stats.final_set_size = app_result.stats.final_set_size + sys_result.stats.final_set_size;
        output.stats.equivalent_groups = app_result.stats.equivalent_groups + sys_result.stats.equivalent_groups;
        output.stats.up_approx_removed = app_result.stats.up_approx_removed + sys_result.stats.up_approx_removed;
        output.stats.down_approx_removed = app_result.stats.down_approx_removed + sys_result.stats.down_approx_removed;
        output.stats.representation_added = app_result.stats.representation_added + sys_result.stats.representation_added;
    }

    return output;
}

// ===== mmap-filepage 旁路分析：按文件（sub_type_id）分组聚合，无等价组/近似/scope =====
// 判定 filepage 调用栈是否含应用符号（任意帧是 app 符号）
static bool filepage_has_app_symbol(
    const CompressedCallStack& cs,
    const std::unordered_map<int, std::string>& data_dict)
{
    for (const auto& f : cs.frames) {
        if (f.file_id_null) continue;
        auto it = data_dict.find(f.file_id);
        if (it == data_dict.end()) continue;
        if (HtraceConfigMgr::instance().is_app_symbol(it->second, f.vaddr_null)) {
            return true;
        }
    }
    return false;
}

struct FilepageSplit {
    std::vector<FilepageInfo> app;
    std::vector<FilepageInfo> system;
    std::vector<FilepageInfo> mixed;  // app + system 合并，按 size 降序（供 mixed scope 单轨）
};

static FilepageSplit analyze_filepages(
    const std::vector<CompressedCallStack>& filepage_stacks,
    const std::unordered_map<int, std::string>& data_dict,
    long long mmap_total_size,
    const std::string& mode,
    bool compute_mixed)
{
    // 按 sub_type_id 分组
    std::map<int, FilepageInfo> grouped;
    for (const auto& cs : filepage_stacks) {
        auto& info = grouped[cs.sub_type_id];
        info.size += cs.existing_size;
        info.count += cs.existing_count;
        info.call_stacks.push_back(cs);
    }

    FilepageSplit split;
    for (auto& [sid, info] : grouped) {
        auto it = data_dict.find(sid);
        info.filename = (it != data_dict.end()) ? it->second : "unknown";
        // detail 模式下 data_dict 的 sub_type 值可能带 "FilePage:" 前缀，输出时剥离
        info.filename = normalize_filepage_filename(info.filename, mode);
        if (mmap_total_size > 0) {
            info.percentage = std::round((double)info.size * 10000.0 / (double)mmap_total_size) / 100.0;
        }
        // 文件级分类：任意栈的任意帧是应用符号 → app 类；否则 system 类
        bool is_app = false;
        for (const auto& cs : info.call_stacks) {
            if (filepage_has_app_symbol(cs, data_dict)) { is_app = true; break; }
        }
        if (is_app) split.app.push_back(std::move(info));
        else split.system.push_back(std::move(info));
    }
    auto by_size = [](const auto& a, const auto& b) { return a.size > b.size; };
    std::sort(split.app.begin(), split.app.end(), by_size);
    std::sort(split.system.begin(), split.system.end(), by_size);
    // mixed 只在 --scope mixed 时才需要（与符号维度一致）：避免 app-and-system / app-only /
    // system-only 模式下 filepage 数据经 mixed 轨漏进 JSON，造成与 HTML scopeActive 门控不对称。
    if (compute_mixed) {
        split.mixed.reserve(split.app.size() + split.system.size());
        split.mixed.insert(split.mixed.end(), split.app.begin(), split.app.end());
        split.mixed.insert(split.mixed.end(), split.system.begin(), split.system.end());
        std::sort(split.mixed.begin(), split.mixed.end(), by_size);
    }
    return split;
}

// ===== Public API (with mmap split) =====
HtraceAnalysisOutput analyze_call_stacks(
    const std::vector<CompressedCallStack>& call_stacks,
    const std::unordered_map<int, std::string>& data_dict,
    AnalysisConfig config,
    const std::string& mode)
{
    if (!htrace_category(config.category).is_multipart()) {
        return analyze_call_stacks_core(call_stacks, data_dict, config);
    }

    // category == "mmap": split into mmap-anon (MMAP_ANON) and mmap-filepage (FILE_PAGE_MSG).
    // anon 走 symbol 管道；filepage 走旁路（按文件聚合 + app/system 分类）。
    std::vector<CompressedCallStack> anon_stacks, filepage_stacks;
    for (const auto& cs : call_stacks) {
        if (cs.type == HtraceEventType::MMAP_ANON) {
            anon_stacks.push_back(cs);
        } else if (cs.type == HtraceEventType::FILE_PAGE_MSG) {
            filepage_stacks.push_back(cs);
        }
    }

    // mmap-anon 走 symbol 管道（top_symbols 第一轨 = 纯 anon，filepage 走旁路）
    auto output = analyze_call_stacks_core(anon_stacks, data_dict, config);

    // mmap 总大小 = anon + filepage（percentage 分母）
    long long filepage_total = 0;
    for (const auto& cs : filepage_stacks) filepage_total += cs.existing_size;
    output.total_leak_size += filepage_total;

    // 有效 scope 与 analyze_call_stacks_core 一致（force_scope 优先）。filepage 轨只挂到
    // 活动 scope，与符号维度 + HTML 的 scopeActive 门控对齐：避免 inactive scope 因 filepage
    // 非空而漏进 JSON 输出（如 app-and-system 下 mixed 出现、app-only 下 system/mixed 出现）。
    Scope scope = config.use_force_scope ? config.force_scope : config.scope;

    // mmap-filepage 走旁路（按文件聚合 + app/system 分类）；mixed 仅在 --scope mixed 时计算。
    FilepageSplit fp_split = analyze_filepages(filepage_stacks, data_dict, output.total_leak_size,
                                               mode, scope == Scope::Mixed);

    // 每个 scope 恒为双轨：[Symbol 轨, Filepage 轨]（空也写，保持 JSON 键结构字节一致）。
    // 三个 scope 一律挂 filepage 轨（非活动 scope 是空轨），保证不变式「mmap ⇒ 每 scope
    // 恰 2 个 section（sections[0]=Symbol、sections[1]=Filepage）」，让 JSON write_scope 与
    // HTML 的 sections[0]/sections[1] 直接下标不会越界（AppAndSystem 下 mixed 也有空双轨）。
    // 轨 label 单一来源 = category 描述符表 tracks[].label（analyzer 只搬运，不自己造显示名）。
    const auto& tracks = htrace_category(config.category).tracks;
    attach_filepages(output.top_symbols.app,    std::move(fp_split.app),    tracks[0].label.c_str(), tracks[1].label.c_str());
    attach_filepages(output.top_symbols.system, std::move(fp_split.system), tracks[0].label.c_str(), tracks[1].label.c_str());
    attach_filepages(output.top_symbols.mixed,  std::move(fp_split.mixed),  tracks[0].label.c_str(), tracks[1].label.c_str());
    return output;
}

// ===== JSON serialization (kept for formatter use only) =====
static json top_symbol_info_to_json(const TopSymbolInfo& ts, const std::unordered_map<int, std::string>& data_dict) {
    json j = {
        {"uid", std::to_string(ts.uid)},
        {"symbol", ts.symbol},
        {"libs", ts.libs},
        {"libOffset", ts.lib_offset},
        {"type", ts.type},
        {"totalLeakSize", ts.total_leak_size},
        {"totalExistingCount", ts.total_existing_count},
        {"isApp", ts.is_app},
        {"buildId", (!ts.build_id_null && ts.build_id != 0)
            ? json(data_dict.at(ts.build_id)) : ""}
    };
    // Serialize flame/icicle trees with proper recursive structure
    // Resolve int IDs to strings at output time via data_dict
    {
        json arr = json::array();
        for (const auto& c : ts.flame_graph.children) {
            arr.push_back(c.to_flame_json(data_dict));
        }
        j["flameGraph"] = arr;
    }
    {
        json arr = json::array();
        for (const auto& c : ts.icicle_graph.children) {
            arr.push_back(c.to_icicle_json(data_dict));
        }
        j["icicleGraph"] = arr;
    }
    return j;
}

json HtraceAnalysisOutput::to_json() const {
    json j = {
        {"totalLeakSize", total_leak_size},
        {"algorithmStats", {
            {"orgSetSize", stats.org_set_size},
            {"filteredSetSize", stats.filtered_set_size},
            {"finalSetSize", stats.final_set_size},
            {"equivalentGroups", stats.equivalent_groups},
            {"upApproxRemoved", stats.up_approx_removed},
            {"downApproxRemoved", stats.down_approx_removed},
            {"representationAdded", stats.representation_added}
        }}
    };

    json top;
    top["mode"] = top_symbols.mode;
    // Only include arrays relevant to the mode (matching TS behavior)
    // app-and-system: app + system; app-only: app; system-only: system; mixed: mixed
    auto symbols_of = [&](const ScopeResult& scope) {
        json arr = json::array();
        for (const auto& s : scope.sections)
            for (const auto& r : s.rows)
                if (const auto* ts = std::get_if<TopSymbolInfo>(&r))
                    arr.push_back(top_symbol_info_to_json(*ts, this->data_dict));
        return arr;
    };
    if (top_symbols.mode == "app-only" || top_symbols.mode == "app-and-system") {
        top["app"] = symbols_of(top_symbols.app);
    }
    if (top_symbols.mode == "system-only" || top_symbols.mode == "app-and-system") {
        top["system"] = symbols_of(top_symbols.system);
    }
    if (top_symbols.mode == "mixed") {
        top["mixed"] = symbols_of(top_symbols.mixed);
    }
    j["topSymbols"] = top;

    return j;
}

// ===== htrace_analyze =====
HtraceAnalysisOutput htrace_analyze(TraceDB& db, const RunContext& ctx) {
    // Read TraceDB from the parse step
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) {
        throw std::runtime_error("htrace_analyze: expected CompressedCallStackList in TraceDB");
    }

    AnalysisConfig acfg;
    acfg.top_n = ctx.htrace.top_n;
    acfg.scope = ctx.htrace.scope;
    acfg.category = ctx.htrace.category;

    auto analysis = analyze_call_stacks(clist->stacks, clist->dict, acfg, db.meta.mode);

    // Populate analysis output with metadata from TraceDB
    analysis.data_dict = std::move(clist->dict);
    analysis.mode = std::move(db.meta.mode);
    analysis.process_name = std::move(db.meta.process_name);
    analysis.type_sizes = std::move(db.meta.type_sizes);
    analysis.trace_meta = std::move(db.meta);

    int total_top = (int)(analysis.top_symbols.app.symbol_count()
                        + analysis.top_symbols.system.symbol_count()
                        + analysis.top_symbols.mixed.symbol_count());
    log::logok("  Analysis completed. ", total_top, " top symbols.");

    return analysis;
}

} // namespace mem_analyzer
