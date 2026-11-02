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
#include "htrace/htrace_analyzer.h"
#include "htrace/htrace_category.h"
#include "config_mgr/htrace_config_mgr.h"
#include "config.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/timer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <variant>

namespace mem_analyzer {

constexpr int UID_SHIFT_BITS = 32;
constexpr int PERCENT_SCALE = 100;
constexpr int MIN_PAIR_SYMBOLS = 2;
constexpr double PERCENT_PRECISION_SCALE = 10000.0;
constexpr int HEX_BUF_SIZE = 32;

// ===== N 轨 section 构造（HtraceRow 是 variant，构造时按同质轨装箱）=====
static HtraceSection MakeSymbolSection(std::vector<TopSymbolInfo> syms, const char* key = "", const char* label = "")
{
    HtraceSection s;
    s.key = key;
    s.label = label;
    s.rows.reserve(syms.size());
    for (auto& t : syms) {
        s.rows.emplace_back(std::move(t));
    }
    return s;
}

static HtraceSection MakeFilepageSection(std::vector<FilepageInfo> fps, const char* key, const char* label)
{
    HtraceSection s;
    s.key = key;
    s.label = label;
    s.rows.reserve(fps.size());
    for (auto& f : fps) {
        s.rows.emplace_back(std::move(f));
    }
    return s;
}

// 将 core 单轨符号结果（key=="" 的 section）改造为双轨：第一轨 = anon 符号，第二轨 = filepage。
// 三个 scope 恒写两轨（空也写），与旧 has_mmap_split 行为的 JSON 键结构字节一致。
static void AttachFilepages(ScopeResult& scope, std::vector<FilepageInfo> fps,
    const char* anonLabel, const char* fpLabel)
{
    if (scope.sections.empty()) {
        scope.sections.push_back(MakeSymbolSection({}, "mmapAnons", anonLabel));
    } else {
        scope.sections[0].key = "mmapAnons";
        scope.sections[0].label = anonLabel;
    }
    scope.sections.push_back(MakeFilepageSection(std::move(fps), "mmapFilepages", fpLabel));
}

// ===== ordered_set: vector (insertion order) + unordered_set (fast lookup) =====
template<typename T>
struct orderedSet {
    std::vector<T> items;
    std::unordered_set<T> set;

    void push_back(const T& val)
    {
        items.push_back(val);
        set.insert(val);
    }

    bool contains(const T& val) const
    {
        return set.find(val) != set.end();
    }

    std::size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear()
    {
        items.clear();
        set.clear();
    }

    // Range-based for support (iterate in insertion order)
    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
    auto begin()
    {
        return items.begin();
    }
    auto end()
    {
        return items.end();
    }

    // Pass to functions expecting const std::vector<T>&
    const std::vector<T>& vector() const { return items; }
};

// ===== Internal structures =====
struct SymbolData {
    uint64_t uid = 0;                    // (symbol_id << 32) | file_id
    int libId = 0;                    // file_id, 0 means null/unknown
    bool libIdNull = true;
    std::string type;
    long long totalLeakSize = 0;
    int maxDepth = 0;
    uint64_t vaddr = 0;                  // symbol vaddr (display use)
    bool vaddrNull = true;
    bool isApp = false;                 // cached result of is_app_symbol()
    int buildId = 0;                    // library build_id (data_dict index, library-level, from first seen frame)
    bool buildIdNull = true;
    orderedSet<int> chainIds;          // insertion order + fast O(1) membership
    std::unordered_map<int, int> depthPerChain;
};

// Helper: make uid from (symbol_id, file_id)
inline uint64_t MakeUid(int symbolId, int fileId)
{
    return (static_cast<uint64_t>(symbolId) << UID_SHIFT_BITS) | static_cast<uint32_t>(fileId);
}

// ===== Helper: check if a symbol is an app symbol =====
// Looks up the lib path via data_dict and delegates to is_app_symbol().
static void CheckAppSymbol(SymbolData& sd, const std::unordered_map<int, std::string>& dataDict)
{
    if (sd.libIdNull) {
        sd.isApp = false;
        return;
    }
    auto it = dataDict.find(sd.libId);
    if (it == dataDict.end()) {
        sd.isApp = false;
        return;
    }
    sd.isApp = HtraceConfigMgr::Instance().IsAppSymbol(it->second, sd.vaddrNull);
}

static SymbolData MakeSymbolData(uint64_t key, int fid, const CompressedStackFrame& frame,
    HtraceEventType chainType)
{
    SymbolData sd;
    sd.uid = key;
    sd.libId = fid;
    sd.libIdNull = frame.fileIdNull;
    sd.type = HtraceEventTypeName(chainType);
    sd.totalLeakSize = 0;
    sd.maxDepth = 0;
    sd.vaddrNull = frame.vaddrNull;
    if (!frame.vaddrNull) {
        sd.vaddr = frame.vaddr;
    }
    sd.buildIdNull = frame.buildIdNull;
    if (!frame.buildIdNull) {
        sd.buildId = frame.buildId;
    }
    return sd;
}

// ===== Build initial symbol data from call stacks =====
static void BuildSymbolData(
    const std::vector<CompressedCallStack>& callStacks,
    std::unordered_map<uint64_t, SymbolData>& symbols,
    const std::unordered_map<int, std::string>& dataDict,
    long long& totalLeakSize)
{
    totalLeakSize = 0;

    for (int ci = 0; ci < static_cast<int>(callStacks.size()); ci++) {
        auto& chain = callStacks[ci];
        long long size = chain.existingSize;
        if (size <= 0) {
            continue;
        }
        totalLeakSize += size;

        std::unordered_set<uint64_t> depthSeen;

        for (auto& frame : chain.frames) {
            int sid = frame.symbolId;
            int fid = frame.fileIdNull ? -1 : frame.fileId;
            if (sid == 0) {
                continue;
            }

            uint64_t key = MakeUid(sid, fid);
            auto it = symbols.find(key);
            if (it == symbols.end()) {
                symbols[key] = MakeSymbolData(key, fid, frame, chain.type);
                it = symbols.find(key);
            }
            auto& sd = it->second;
            int depth = frame.depth;

            if (depthSeen.find(key) != depthSeen.end()) {
                continue;
            }
            depthSeen.insert(key);
            sd.chainIds.push_back(ci);
            sd.depthPerChain[ci] = depth;
            sd.totalLeakSize += size;
            if (depth > sd.maxDepth) {
                sd.maxDepth = depth;
            }
        }
    }

    for (auto& [uid, sd] : symbols) {
        CheckAppSymbol(sd, dataDict);
    }
}

// ===== Check if two symbols are equivalent =====
static bool AreEquivalent(const SymbolData& a, const SymbolData& b)
{
    if (a.chainIds.size() != b.chainIds.size()) {
        return false;
    }
    for (int ci : a.chainIds) {
        if (!b.chainIds.contains(ci)) {
            return false;
        }
    }
    return true;
}

// ===== Find equivalent groups (deterministic: input sorted by leak size desc) =====
static size_t ChainIdsHash(const orderedSet<int>& ids)
{
    size_t h = 0;
    for (int ci : ids) {
        h += std::hash<int>{}(ci);
    }
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

static std::vector<std::vector<SymbolData*>> findEquivalentGroups(const std::vector<SymbolData*>& symbols)
{
    // symbols is already sorted by total_leak_size descending by caller

    // Bucket by (chain_ids size, hash) — only symbols in the same bucket can be equivalent
    std::unordered_map<BucketKey, std::vector<SymbolData*>, BucketKeyHash> buckets;
    for (auto* s : symbols) {
        buckets[{s->chainIds.size(), ChainIdsHash(s->chainIds)}].push_back(s);
    }

    std::set<uint64_t> visited;
    std::vector<std::vector<SymbolData*>> groups;

    for (auto* s : symbols) {
        if (visited.count(s->uid)) {
            continue;
        }
        std::vector<SymbolData*> group = {s};
        visited.insert(s->uid);

        // Only compare within same bucket (size+hash match)
        auto& bucket = buckets[{s->chainIds.size(), ChainIdsHash(s->chainIds)}];
        for (auto* other : bucket) {
            if (visited.count(other->uid)) {
                continue;
            }
            if (AreEquivalent(*s, *other)) {
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
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> upApprox;
    std::unordered_map<uint64_t, size_t> upIdx;
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> downApprox;
    std::unordered_map<uint64_t, size_t> downIdx;
};

// Insertion-order-preserving inner map (int key → long long value)
// Uses vector + index map to keep insertion order while providing O(1) lookup.
struct InnerMap {
    std::vector<std::pair<int, long long>> items;
    std::unordered_map<int, size_t> idx;

    void Add(int key, long long val)
    {
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
static long long RelThresholdFor(long long replacedTotal, const AnalysisConfig& config)
{
    long long t = replacedTotal * config.approRelRatio / PERCENT_SCALE;
    return t < config.approRelCap ? t : config.approRelCap;
}

static bool CanApproximate(long long value,
    long long replacedTotal,
    long long counterpartTotal,
    const AnalysisConfig& config)
{
    if (replacedTotal <= 0) {
        return false;
    }
    long long diff = std::abs(value - replacedTotal);
    if (diff > RelThresholdFor(replacedTotal, config)) {
        return false;
    }
    if (diff == 0) {
        return true;
    }
    return value > counterpartTotal * config.approRatioThreshold / PERCENT_SCALE;
}

// contrib: insertion order = first time each 'deep' is encountered during pair iteration
// alloc: insertion order = first time each 'shallow' is encountered
struct ApproxPairMaps {
    std::vector<std::pair<int, InnerMap>> contrib;          // (deep_idx, InnerMap)
    std::unordered_map<int, size_t> contribIdx;             // deep_idx → position in contrib
    std::vector<std::pair<int, InnerMap>> alloc;             // (shallow_idx, InnerMap)
    std::unordered_map<int, size_t> allocIdx;               // shallow_idx → position in alloc
};

static void BuildChainCache(
    const std::vector<SymbolData*>& filtered,
    const std::unordered_map<uint64_t, int>& symIndex,
    std::vector<std::pair<int, std::vector<std::pair<int, int>>>>& chainCacheVec,
    std::unordered_map<int, size_t>& chainCacheIdx)
{
    // Build chain cache (iterates filtered in sorted order to match TS Map insertion order)
    for (auto* sd : filtered) {
        int symIdx = symIndex.at(sd->uid);
        for (int ci : sd->chainIds) {
            size_t idx;
            auto it = chainCacheIdx.find(ci);
            if (it == chainCacheIdx.end()) {
                idx = chainCacheVec.size();
                chainCacheIdx[ci] = idx;
                chainCacheVec.emplace_back(ci, std::vector<std::pair<int, int>>());
            } else {
                idx = it->second;
            }
            chainCacheVec[idx].second.emplace_back(symIdx, sd->depthPerChain.at(ci));
        }
    }
}

static void BuildApproxPairs(
    std::vector<std::pair<int, std::vector<std::pair<int, int>>>>& chainCacheVec,
    const std::vector<CompressedCallStack>& callStacks,
    const std::vector<SymbolData*>& filtered,
    ApproxPairMaps& maps)
{
    // Pair building — all keys are int now, O(1) hash/comparison
    for (auto& [ci, chainSyms] : chainCacheVec) {
        if (chainSyms.size() < MIN_PAIR_SYMBOLS) {
            continue;
        }
        std::stable_sort(chainSyms.begin(), chainSyms.end(),
            [&filtered](const auto& a, const auto& b) {
                if (a.second != b.second) {
                    return a.second < b.second;
                }
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });

        if (ci < 0 || ci >= static_cast<int>(callStacks.size())) {
            continue;
        }
        long long leak = callStacks[ci].existingSize;
        if (leak <= 0) {
            continue;
        }

        for (size_t i = 0; i < chainSyms.size(); i++) {
            int shallowIdx = chainSyms[i].first;
            for (size_t j = i + 1; j < chainSyms.size(); j++) {
                int deepIdx = chainSyms[j].first;

                // contrib: insert deep in insertion order
                auto cIt = maps.contribIdx.find(deepIdx);
                if (cIt == maps.contribIdx.end()) {
                    maps.contribIdx[deepIdx] = maps.contrib.size();
                    maps.contrib.emplace_back(deepIdx, InnerMap());
                    cIt = maps.contribIdx.find(deepIdx);
                }
                maps.contrib[cIt->second].second.Add(shallowIdx, leak);

                // alloc: insert shallow in insertion order
                auto aIt = maps.allocIdx.find(shallowIdx);
                if (aIt == maps.allocIdx.end()) {
                    maps.allocIdx[shallowIdx] = maps.alloc.size();
                    maps.alloc.emplace_back(shallowIdx, InnerMap());
                    aIt = maps.allocIdx.find(shallowIdx);
                }
                maps.alloc[aIt->second].second.Add(deepIdx, leak);
            }
        }
    }
}

static void SortApproxMaps(
    const std::vector<SymbolData*>& filtered,
    std::vector<std::pair<int, InnerMap>>& contrib,
    std::vector<std::pair<int, InnerMap>>& alloc)
{
    // Sort contrib/alloc and their inner maps deterministically by uid
    for (auto& [deepIdx, cMap] : contrib) {
        std::sort(cMap.items.begin(), cMap.items.end(),
            [&filtered](const auto& a, const auto& b) {
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });
    }
    std::sort(contrib.begin(), contrib.end(),
        [&filtered](const auto& a, const auto& b) {
            return filtered[a.first]->uid < filtered[b.first]->uid;
        });
    for (auto& [shallowIdx, aMap] : alloc) {
        std::sort(aMap.items.begin(), aMap.items.end(),
            [&filtered](const auto& a, const auto& b) {
                return filtered[a.first]->uid < filtered[b.first]->uid;
            });
    }
    std::sort(alloc.begin(), alloc.end(),
        [&filtered](const auto& a, const auto& b) {
            return filtered[a.first]->uid < filtered[b.first]->uid;
        });
}

static void AddApproxEntry(
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>>& approx,
    std::unordered_map<uint64_t, size_t>& idx,
    uint64_t outerUid, uint64_t innerUid)
{
    auto it = idx.find(outerUid);
    if (it == idx.end()) {
        idx[outerUid] = approx.size();
        approx.emplace_back(outerUid, std::vector<uint64_t>());
        it = idx.find(outerUid);
    }
    approx[it->second].second.push_back(innerUid);
}

static void EvaluateApprox(
    const std::vector<SymbolData*>& filtered,
    const std::vector<std::pair<int, InnerMap>>& pairs,
    const AnalysisConfig& config,
    bool isUp,
    ApproxResult& result)
{
    // Evaluation — lookup total_leak_size directly from filtered array by index
    for (auto& [outerIdx, map] : pairs) {
        long long totalOuter = filtered[outerIdx]->totalLeakSize;
        if (totalOuter <= 0) {
            continue;
        }
        for (auto& [innerIdx, value] : map) {
            long long totalInner = filtered[innerIdx]->totalLeakSize;
            if (totalInner <= 0) {
                continue;
            }
            if (!CanApproximate(value, totalOuter, totalInner, config)) {
                continue;
            }
            uint64_t outerUid = filtered[outerIdx]->uid;
            uint64_t innerUid = filtered[innerIdx]->uid;
            if (isUp) {
                AddApproxEntry(result.upApprox, result.upIdx, outerUid, innerUid);
            } else {
                AddApproxEntry(result.downApprox, result.downIdx, outerUid, innerUid);
            }
        }
    }
}

static ApproxResult CalcApproximations(
    const std::vector<SymbolData*>& filtered,
    const std::vector<CompressedCallStack>& callStacks,
    const AnalysisConfig& config)
{
    int n = static_cast<int>(filtered.size());

    // Build uid → index map (int keys for all internal lookups)
    std::unordered_map<uint64_t, int> symIndex;
    symIndex.reserve(n);
    for (int i = 0; i < n; i++) {
        symIndex[filtered[i]->uid] = i;
    }

    // Chain cache: (chain_id, vector of (sym_idx, depth))
    std::vector<std::pair<int, std::vector<std::pair<int, int>>>> chainCacheVec;
    std::unordered_map<int, size_t> chainCacheIdx;
    BuildChainCache(filtered, symIndex, chainCacheVec, chainCacheIdx);

    ApproxPairMaps maps;
    BuildApproxPairs(chainCacheVec, callStacks, filtered, maps);
    SortApproxMaps(filtered, maps.contrib, maps.alloc);

    ApproxResult result;
    EvaluateApprox(filtered, maps.contrib, config, true, result);
    EvaluateApprox(filtered, maps.alloc, config, false, result);
    return result;
}

// ===== Algorithm runner =====
struct AlgorithmResult {
    std::vector<SymbolData> finalSymbols;
    AlgorithmStats stats;
};

struct EquivRefs {
    std::unordered_map<uint64_t, uint64_t> equivRefMap;
    int removedEquiv = 0;
};

static EquivRefs FindEquivRefs(
    const std::vector<std::vector<SymbolData*>>& equivGroups,
    std::unordered_set<uint64_t>& removedUids)
{
    std::unordered_map<uint64_t, uint64_t> equivRefMap;
    int removedEquiv = 0;
    for (auto& group : equivGroups) {
        if (group.size() <= 1) {
            continue;
        }
        // Find ref (deepest depth, tiebreaker: smallest uid)
        SymbolData* ref = group[0];
        for (auto* s : group) {
            if (s->maxDepth > ref->maxDepth ||
                (s->maxDepth == ref->maxDepth && s->uid < ref->uid)) ref = s;
        }
        for (auto* s : group) {
            if (s->uid != ref->uid) {
                equivRefMap[s->uid] = ref->uid;
                removedUids.insert(s->uid);
                removedEquiv++;
            }
        }
    }
    return {std::move(equivRefMap), removedEquiv};
}

static void FilterApproxRemoved(
    const ApproxResult& approx,
    std::unordered_set<uint64_t>& removedUids,
    int& downRemoved, int& upRemoved)
{
    // 4.6: Filter using approximations (order-independent fixed-point iteration)
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [sym, downSet] : approx.downApprox) {
            if (removedUids.count(sym)) {
                continue;
            }
            for (auto& ds : downSet) {
                if (!removedUids.count(ds)) {
                    removedUids.insert(sym);
                    downRemoved++;
                    changed = true;
                    break;
                }
            }
        }
        for (auto& [sym, upSet] : approx.upApprox) {
            if (removedUids.count(sym)) {
                continue;
            }
            for (auto& us : upSet) {
                if (!removedUids.count(us)) {
                    removedUids.insert(sym);
                    upRemoved++;
                    changed = true;
                    break;
                }
            }
        }
    }
}

static bool HasSurvivingApprox(
    uint64_t uid,
    const ApproxResult& approx,
    const std::unordered_set<uint64_t>& removedUids)
{
    auto uIt = approx.upIdx.find(uid);
    if (uIt != approx.upIdx.end() && uIt->second < approx.upApprox.size()) {
        for (auto& u : approx.upApprox[uIt->second].second) {
            if (!removedUids.count(u)) {
                return true;
            }
        }
    }
    auto dIt = approx.downIdx.find(uid);
    if (dIt != approx.downIdx.end() && dIt->second < approx.downApprox.size()) {
        for (auto& d : approx.downApprox[dIt->second].second) {
            if (!removedUids.count(d)) {
                return true;
            }
        }
    }
    return false;
}

static int CheckRepresentation(
    const std::vector<SymbolData*>& filtered,
    const ApproxResult& approx,
    const std::unordered_map<uint64_t, uint64_t>& equivRefMap,
    std::unordered_set<uint64_t>& removedUids)
{
    // 4.7: Check representation
    int representationAdded = 0;
    for (auto* s : filtered) {
        if (!removedUids.count(s->uid)) {
            continue;
        }
        bool inSurviving = HasSurvivingApprox(s->uid, approx, removedUids);
        if (!inSurviving) {
            auto eqIt = equivRefMap.find(s->uid);
            if (eqIt != equivRefMap.end() && !removedUids.count(eqIt->second)) {
                inSurviving = true;
            }
        }
        if (!inSurviving) {
            removedUids.erase(s->uid);
            representationAdded++;
        }
    }
    return representationAdded;
}

static std::vector<SymbolData> BuildFinalSymbols(
    const std::vector<SymbolData*>& filtered,
    const std::unordered_set<uint64_t>& removedUids)
{
    // 4.8: Build final result — only copy surviving symbols
    std::vector<SymbolData> finalSymbols;
    for (auto* s : filtered) {
        if (!removedUids.count(s->uid)) {
            finalSymbols.push_back(*s);
        }
    }
    std::stable_sort(finalSymbols.begin(), finalSymbols.end(),
        [](const auto& a, const auto& b) {
            if (a.totalLeakSize != b.totalLeakSize) {
                return a.totalLeakSize > b.totalLeakSize;
            }
            return a.uid < b.uid;
        });
    return finalSymbols;
}

static int CountEquivGroups(const std::vector<std::vector<SymbolData*>>& equivGroups)
{
    int count = 0;
    for (auto& g : equivGroups) {
        if (g.size() > 1) {
            count++;
        }
    }
    return count;
}

static AlgorithmResult RunAlgorithm(
    const std::vector<SymbolData*>& filtered,
    const std::vector<CompressedCallStack>& callStacks,
    const AnalysisConfig& config)
{
    StageTimer timer("Algo");

    AlgorithmResult algResult;
    int orgSetSize = 0;     // set by caller
    int filteredSetSize = static_cast<int>(filtered.size());

    // 4.2: Track removed symbols (no deep copy of SymbolData)
    std::unordered_set<uint64_t> removedUids;

    // 4.3: Equivalent symbols
    timer.Stage("Finding equivalent groups");
    auto equivGroups = findEquivalentGroups(filtered);
    auto equivRefs = FindEquivRefs(equivGroups, removedUids);
    timer.Tick("equiv");

    // 4.5: Calculate approximations on filtered set
    timer.Stage("Calculating approximations");
    auto approx = CalcApproximations(filtered, callStacks, config);
    timer.Tick("approx");

    // 4.6: Filter using approximations (order-independent fixed-point iteration)
    int downRemoved = 0;
    int upRemoved = 0;
    FilterApproxRemoved(approx, removedUids, downRemoved, upRemoved);

    // 4.7: Check representation
    int representationAdded = CheckRepresentation(filtered, approx, equivRefs.equivRefMap, removedUids);

    algResult.finalSymbols = BuildFinalSymbols(filtered, removedUids);
    timer.Tick("filter",
        std::to_string(filteredSetSize) + " in, " + std::to_string(algResult.finalSymbols.size()) + " out");

    int equivGroupCount = CountEquivGroups(equivGroups);

    algResult.stats = {
        orgSetSize,
        filteredSetSize,
        static_cast<int>(algResult.finalSymbols.size()),
        equivGroupCount,
        upRemoved,
        downRemoved,
        representationAdded
    };

    return algResult;
}

static void ResolveTopSymbolMeta(const SymbolData& s,
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict, TopSymbolInfo& ts)
{
    // Resolve symbol/lib from data_dict
    int sid = static_cast<int>(s.uid >> UID_SHIFT_BITS);
    int fid = static_cast<int>(s.uid & 0xFFFFFFFF);
    auto symIt = dataDict.find(sid);
    ts.symbol = (symIt != dataDict.end()) ? symIt->second : "unknown";
    auto libIt = dataDict.find(fid);
    std::string libStr = (libIt != dataDict.end()) ? libIt->second : "unknown";
    ts.libs = {libStr};
    // Resolve lib_offset from stored vaddr
    if (!s.vaddrNull) {
        char buf[HEX_BUF_SIZE];
        int ret = snprintf(buf, sizeof(buf), "0x%llx",
                           static_cast<unsigned long long>(s.vaddr));
        if (ret < 0) {
            buf[0] = '\0';
        }
        ts.libOffset = buf;
    } else {
        ts.libOffset = "";
    }
    ts.type = s.type;
    ts.totalLeakSize = s.totalLeakSize;
    // Sum existing_count over all chains that contain this symbol (one contribution per chain)
    ts.totalExistingCount = 0;
    for (int ci : s.chainIds) {
        if (ci >= 0 && ci < static_cast<int>(callStacks.size())) {
            ts.totalExistingCount += callStacks[ci].existingCount;
        }
    }
    ts.isApp = s.isApp;
    ts.buildIdNull = s.buildIdNull;
    if (!s.buildIdNull) {
        ts.buildId = s.buildId;
    }
}

// ===== Build flame/icicle graphs and call chains for top-N symbols only =====
static std::vector<TopSymbolInfo> buildAndEnrich(
    const std::vector<SymbolData>& finalSymbols,
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict,
    int topN)
{
    StageTimer timer("Analysis");

    timer.Stage("Building flame/icicle graphs for top symbols");
    std::vector<TopSymbolInfo> result;
    int limit = std::min(static_cast<int>(finalSymbols.size()), topN);
    result.reserve(limit);

    for (int i = 0; i < limit; i++) {
        auto& s = finalSymbols[i];
        TopSymbolInfo ts;
        ts.uid = s.uid;

        ResolveTopSymbolMeta(s, callStacks, dataDict, ts);

        // Build flame/icicle graphs with data_dict for ID→string resolution
        ts.flameGraph = BuildFlameGraph(ts.symbol, callStacks, s.chainIds.vector(), s.depthPerChain, dataDict);
        ts.icicleGraph = BuildIcicleGraph(ts.symbol, callStacks, s.chainIds.vector(), s.depthPerChain, dataDict);
        for (int ci : s.chainIds) {
            if (ci >= 0 && ci < static_cast<int>(callStacks.size())) {
                ts.callChains.push_back(callStacks[ci]);
            }
        }
        result.push_back(std::move(ts));
    }

    timer.Tick("build_graphs", std::to_string(limit) + " top symbols");

    return result;
}

static void LoadAlgorithmConfig(AnalysisConfig& config)
{
    // Lazy-load htrace config
    auto& htraceCfg = HtraceConfigMgr::Instance();
    if (!htraceCfg.IsLoaded()) {
        std::string err;
        if (!htraceCfg.LoadDefault(err)) {
            throw std::runtime_error("Failed to load trace_analyzer_config.json: " + err);
        }
    }

    // Apply algorithm parameters from config
    config.lowThreshold = htraceCfg.Algorithm().lowThreshold;
    config.approRelRatio = htraceCfg.Algorithm().approRelRatio;
    config.approRelCap = htraceCfg.Algorithm().approRelCap;
    config.approRatioThreshold = htraceCfg.Algorithm().approRatioThreshold;
    config.orphanRelRatio = htraceCfg.Algorithm().orphanRelRatio;
    config.orphanRelCap = htraceCfg.Algorithm().orphanRelCap;

    // fd/thread sizes are counts (not bytes); the byte-based low threshold
    // would wrongly filter them out, so disable it for these types.
    if (HtraceCategory(config.category).countMode) {
        config.lowThreshold = 0;
    }
}

static void ResolveSymbolNames(const SymbolData& sd,
    const std::unordered_map<int, std::string>& dataDict,
    std::string& libFile, std::string& symName)
{
    std::string libName;
    if (sd.libIdNull) {
        libName = "unknown";
    } else {
        auto lit = dataDict.find(sd.libId);
        if (lit != dataDict.end()) {
            libName = lit->second;
        }
    }
    libFile = libName.substr(libName.find_last_of("/\\") + 1);
    int sid = static_cast<int>(sd.uid >> UID_SHIFT_BITS);
    auto sit = dataDict.find(sid);
    symName = (sit != dataDict.end()) ? sit->second : "";
}

static bool ScopeOk(const SymbolData& sd, const AnalysisConfig& config, Scope filterScope)
{
    if (config.leakMode) { // -l: L3 已分栈，整组符号定性，跳过 isApp 过滤
        return true;
    }
    if (filterScope == Scope::APP_ONLY && !sd.isApp) {
        return false;
    }
    if (filterScope == Scope::SYSTEM_ONLY && sd.isApp) {
        return false;
    }
    return true;
}

static bool IsNoiseSymbol(const SymbolData& sd,
    const std::unordered_map<int, std::string>& dataDict, Scope filterScope)
{
    std::string libFile;
    std::string symName;
    ResolveSymbolNames(sd, dataDict, libFile, symName);
    return HtraceConfigMgr::Instance().ShouldFilter(libFile, symName, filterScope);
}

static bool IsOrphanRecoverableSym(const SymbolData& sd,
    const std::unordered_map<int, std::string>& dataDict, Scope filterScope)
{
    std::string libFile;
    std::string symName;
    ResolveSymbolNames(sd, dataDict, libFile, symName);
    return HtraceConfigMgr::Instance().IsOrphanRecoverable(libFile, symName, filterScope);
}

struct CoreCtx {
    const std::vector<CompressedCallStack>& callStacks;
    const std::unordered_map<int, std::string>& dataDict;
    const AnalysisConfig& config;
    int topN;
    const std::string& symbolTrackKey;
    std::unordered_map<uint64_t, SymbolData>& allSymbols;
    long long totalLeakSize;
    std::deque<SymbolData>& recoveredOwner;
};

static bool IsChainOrphan(
    const CompressedCallStack& chain,
    const CoreCtx& ctx,
    Scope filterScope)
{
    bool hasScopeSym = false;
    bool hasSurvivor = false;
    for (const auto& frame : chain.frames) {
        if (frame.symbolId == 0) {
            continue;
        }
        uint64_t key = MakeUid(frame.symbolId, frame.fileIdNull ? -1 : frame.fileId);
        auto it = ctx.allSymbols.find(key);
        if (it == ctx.allSymbols.end()) {
            continue;
        }
        const SymbolData& sd = it->second;
        // 幸存者看整条链：任何 scope 的非噪声符号都意味着泄漏未丢失
        if (!IsNoiseSymbol(sd, ctx.dataDict, filterScope)) {
            hasSurvivor = true;
            break;
        }
        // 只有当前 scope 的符号才计入 has_scope_sym（决定恢复归属）
        if (ScopeOk(sd, ctx.config, filterScope)) {
            hasScopeSym = true;
        }
    }
    return hasScopeSym && !hasSurvivor;
}

static void AggregateOrphanSymbols(
    const CompressedCallStack& chain,
    const CoreCtx& ctx,
    Scope filterScope,
    std::unordered_map<uint64_t, long long>& orphanAgg,
    std::unordered_map<uint64_t, const SymbolData*>& orphanSrc)
{
    // Orphan: aggregate each recoverable in-scope frame's leak size
    // (restricted to this stack; dedup'd by uid).
    std::unordered_set<uint64_t> depthSeen;
    long long size = chain.existingSize;
    for (const auto& frame : chain.frames) {
        if (frame.symbolId == 0) {
            continue;
        }
        int fid = frame.fileIdNull ? -1 : frame.fileId;
        uint64_t key = MakeUid(frame.symbolId, fid);
        auto it = ctx.allSymbols.find(key);
        if (it == ctx.allSymbols.end()) {
            continue;
        }
        const SymbolData& src = it->second;
        if (!ScopeOk(src, ctx.config, filterScope)) {
            continue;
        }
        if (!IsOrphanRecoverableSym(src, ctx.dataDict, filterScope)) {
            continue;
        }
        if (depthSeen.find(key) != depthSeen.end()) {
            continue;
        }
        depthSeen.insert(key);
        orphanAgg[key] += size;
        orphanSrc[key] = &src;
    }
}

static void RecoverOrphans(
    const CoreCtx& ctx,
    Scope filterScope,
    std::vector<SymbolData*>& filtered)
{
    // Pass 2: recover orphan stacks. An orphan stack has at least one
    // in-scope frame but none that survives the noise filter. Its symbols
    // are re-added so the algorithm can merge and approximate them as
    // usual. Whether a symbol is worth recovering is decided by its leak
    // size aggregated across orphan stacks (an adaptive fraction of the
    // total leak size, capped); once recovered, the symbol keeps its
    // full-chain data (chain_ids / total_leak_size) rather than being
    // restricted to orphan chains, so its reported size stays accurate.
    std::unordered_map<uint64_t, long long> orphanAgg;         // uid -> 孤儿栈内聚合泄漏量
    std::unordered_map<uint64_t, const SymbolData*> orphanSrc; // uid -> 原始符号（完整范围）
    for (int ci = 0; ci < static_cast<int>(ctx.callStacks.size()); ci++) {
        const auto& chain = ctx.callStacks[ci];
        long long size = chain.existingSize;
        if (size <= 0) {
            continue;
        }
        if (!IsChainOrphan(chain, ctx, filterScope)) {
            continue;
        }
        AggregateOrphanSymbols(chain, ctx, filterScope, orphanAgg, orphanSrc);
    }

    // 孤儿栈兜底显著度门槛 = min(总泄漏量 × orphan_rel_ratio/100, orphan_rel_cap)。
    // 只有孤儿栈内聚合泄漏量达到门槛的符号才恢复，避免引入碎片化干扰。
    long long orphanThreshold = ctx.totalLeakSize * ctx.config.orphanRelRatio / PERCENT_SCALE;
    if (orphanThreshold > ctx.config.orphanRelCap) {
        orphanThreshold = ctx.config.orphanRelCap;
    }

    for (auto& kv : orphanAgg) {
        if (kv.second < orphanThreshold) {
            continue;
        }
        // 完整复制原始符号（保留完整 chain_ids / total_leak_size）。
        ctx.recoveredOwner.push_back(*orphanSrc[kv.first]);
        filtered.push_back(&ctx.recoveredOwner.back());
    }
}

// Helper: filter symbols by scope and thresholds, return sorted pointer list.
// Also recovers "orphan" stacks (stacks whose in-scope frames are all noise
// symbols and would otherwise lose their leak size from the ranking), so the
// leak size stays visible. Only the noise filter is bypassed for recovery;
// recovered symbols are gated by an orphan-specific threshold.
static std::pair<std::vector<SymbolData*>, int> BuildFiltered(
    const CoreCtx& ctx, Scope filterScope)
{
    std::vector<SymbolData*> filtered;
    int orgCount = 0;

    // Pass 1: symbol-level scope + noise + low_threshold filtering.
    for (auto& kv : ctx.allSymbols) {
        SymbolData& sd = kv.second;
        if (!ScopeOk(sd, ctx.config, filterScope)) {
            continue;
        }
        orgCount++;
        if (IsNoiseSymbol(sd, ctx.dataDict, filterScope)) {
            continue;
        }
        // -l(leak_mode): 跳过 low_threshold 过滤，保留所有泄漏符号
        if (ctx.config.leakMode || sd.totalLeakSize >= ctx.config.lowThreshold) {
            filtered.push_back(&sd);
        }
    }

    RecoverOrphans(ctx, filterScope, filtered);

    std::stable_sort(filtered.begin(), filtered.end(),
        [](const auto* a, const auto* b) {
            if (a->totalLeakSize != b->totalLeakSize) {
                return a->totalLeakSize > b->totalLeakSize;
            }
            return a->uid < b->uid;
        });
    return {filtered, orgCount};
}

// leak_mode: 收集非 top N 覆盖的栈 -> other, 追加到对应 top_symbols 分类
static void AppendOther(
    const AnalysisConfig& config,
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict,
    std::vector<TopSymbolInfo>& targetVec,
    const std::vector<SymbolData>& finalSymbols)
{
    if (!config.leakMode) {
        return;
    }
    int limit = std::min(static_cast<int>(finalSymbols.size()), config.topN);
    std::unordered_set<int> covered;
    for (int i = 0; i < limit; i++) {
        for (int ci : finalSymbols[i].chainIds) {
            covered.insert(ci);
        }
    }
    std::vector<int> otherIds;
    long long otherLeak = 0;
    long long otherCount = 0;
    for (int ci = 0; ci < static_cast<int>(callStacks.size()); ci++) {
        if (covered.count(ci)) {
            continue;
        }
        if (callStacks[ci].existingSize <= 0) {
            continue;
        }
        otherIds.push_back(ci);
        otherLeak += callStacks[ci].existingSize;
        otherCount += callStacks[ci].existingCount;
    }
    if (otherIds.empty()) {
        return;
    }
    TopSymbolInfo ts;
    ts.symbol = OTHER_SYMBOL;
    ts.totalLeakSize = otherLeak;
    ts.totalExistingCount = otherCount;
    ts.icicleGraph = BuildMergedIcicle(callStacks, otherIds, dataDict);
    targetVec.push_back(std::move(ts));
}

static void ProcessSingleScope(
    HtraceAnalysisOutput& output,
    const CoreCtx& ctx,
    Scope filterScope,
    const std::string& modeStr,
    ScopeResult& scopeField)
{
    auto [filtered, orgCount] = BuildFiltered(ctx, filterScope);
    auto result = RunAlgorithm(filtered, ctx.callStacks, ctx.config);
    output.topSymbols.mode = modeStr;
    auto syms = buildAndEnrich(result.finalSymbols, ctx.callStacks, ctx.dataDict, ctx.topN);
    AppendOther(ctx.config, ctx.callStacks, ctx.dataDict, syms, result.finalSymbols);
    scopeField.sections.push_back(MakeSymbolSection(std::move(syms), ctx.symbolTrackKey.c_str()));
    output.stats = result.stats;
    output.stats.orgSetSize = orgCount;
}

static void ProcessAppAndSystem(
    HtraceAnalysisOutput& output,
    const CoreCtx& ctx)
{
    auto [appFiltered, appOrg] = BuildFiltered(ctx, Scope::APP_ONLY);
    auto [sysFiltered, sysOrg] = BuildFiltered(ctx, Scope::SYSTEM_ONLY);
    auto appResult = RunAlgorithm(appFiltered, ctx.callStacks, ctx.config);
    auto sysResult = RunAlgorithm(sysFiltered, ctx.callStacks, ctx.config);
    output.topSymbols.mode = "app-and-system";
    auto appSyms = buildAndEnrich(appResult.finalSymbols, ctx.callStacks, ctx.dataDict, ctx.topN);
    AppendOther(ctx.config, ctx.callStacks, ctx.dataDict, appSyms, appResult.finalSymbols);
    output.topSymbols.app.sections.push_back(
        MakeSymbolSection(std::move(appSyms), ctx.symbolTrackKey.c_str()));
    auto sysSyms = buildAndEnrich(sysResult.finalSymbols, ctx.callStacks, ctx.dataDict, ctx.topN);
    AppendOther(ctx.config, ctx.callStacks, ctx.dataDict, sysSyms, sysResult.finalSymbols);
    output.topSymbols.system.sections.push_back(
        MakeSymbolSection(std::move(sysSyms), ctx.symbolTrackKey.c_str()));
    output.stats.orgSetSize = appOrg + sysOrg;
    output.stats.filteredSetSize = appResult.stats.filteredSetSize + sysResult.stats.filteredSetSize;
    output.stats.finalSetSize = appResult.stats.finalSetSize + sysResult.stats.finalSetSize;
    output.stats.equivalentGroups = appResult.stats.equivalentGroups + sysResult.stats.equivalentGroups;
    output.stats.upApproxRemoved = appResult.stats.upApproxRemoved + sysResult.stats.upApproxRemoved;
    output.stats.downApproxRemoved = appResult.stats.downApproxRemoved + sysResult.stats.downApproxRemoved;
    output.stats.representationAdded = appResult.stats.representationAdded + sysResult.stats.representationAdded;
}

// ===== Core analysis (no mmap split) =====
static HtraceAnalysisOutput AnalyzeCallStacksCore(
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict,
    AnalysisConfig config)
{
    StageTimer timer("Analysis");

    Scope scope = config.useForceScope ? config.forceScope : config.scope;
    std::string scopeStr = ScopeToString(scope);
    int topN = config.topN;

    // 单轨 category 的 JSON 嵌套键（来自描述符表 tracks[0].json_key，如 "nativeHeaps"/"dmas"）。
    // 多轨（mmap）已按 is_multipart 分叉、不经过此函数；这里只服务单轨。
    const std::string& symbolTrackKey = HtraceCategory(config.category).tracks[0].jsonKey;

    LoadAlgorithmConfig(config);

    // Build symbol data
    timer.Stage("Building symbol data from call stacks");
    std::unordered_map<uint64_t, SymbolData> allSymbols;
    long long totalLeakSize;
    BuildSymbolData(callStacks, allSymbols, dataDict, totalLeakSize);
    timer.Tick("build_symbols", std::to_string(allSymbols.size()) + " symbols");

    HtraceAnalysisOutput output;
    output.totalLeakSize = totalLeakSize;

    // Owner for orphan-recovered SymbolData copies. Recovered symbols must
    // outlive run_algorithm()/build_and_enrich(), so they are kept here (deque
    // push_back does not invalidate pointers to existing elements).
    std::deque<SymbolData> recoveredOwner;

    CoreCtx ctx{callStacks, dataDict, config, topN, symbolTrackKey, allSymbols, totalLeakSize, recoveredOwner};

    if (scopeStr == "app-only") {
        ProcessSingleScope(output, ctx, Scope::APP_ONLY, "app-only", output.topSymbols.app);
    } else if (scopeStr == "system-only") {
        ProcessSingleScope(output, ctx, Scope::SYSTEM_ONLY, "system-only", output.topSymbols.system);
    } else if (scopeStr == "mixed") {
        ProcessSingleScope(output, ctx, Scope::MIXED, "mixed", output.topSymbols.mixed);
    } else {
        // app-and-system (default)
        ProcessAppAndSystem(output, ctx);
    }

    return output;
}

// ===== mmap-filepage 旁路分析：按文件（sub_type_id）分组聚合，无等价组/近似/scope =====
// 判定 filepage 调用栈是否含应用符号（任意帧是 app 符号）
static bool FilepageHasAppSymbol(const CompressedCallStack& cs, const std::unordered_map<int, std::string>& dataDict)
{
    for (const auto& f : cs.frames) {
        if (f.fileIdNull) {
            continue;
        }
        auto it = dataDict.find(f.fileId);
        if (it == dataDict.end()) {
            continue;
        }
        if (HtraceConfigMgr::Instance().IsAppSymbol(it->second, f.vaddrNull)) {
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

static FilepageSplit AnalyzeFilepages(
    const std::vector<CompressedCallStack>& filepageStacks,
    const std::unordered_map<int, std::string>& dataDict,
    long long mmapTotalSize,
    const std::string& mode,
    bool computeMixed)
{
    // 按 sub_type_id 分组
    std::map<int, FilepageInfo> grouped;
    for (const auto& cs : filepageStacks) {
        auto& info = grouped[cs.subTypeId];
        info.size += cs.existingSize;
        info.count += cs.existingCount;
        info.callStacks.push_back(cs);
    }

    FilepageSplit split;
    for (auto& [sid, info] : grouped) {
        auto it = dataDict.find(sid);
        info.filename = (it != dataDict.end()) ? it->second : "unknown";
        // detail 模式下 data_dict 的 sub_type 值可能带 "FilePage:" 前缀，输出时剥离
        info.filename = NormalizeFilepageFilename(info.filename, mode);
        if (mmapTotalSize > 0) {
            info.percentage = std::round(static_cast<double>(info.size) *
                PERCENT_PRECISION_SCALE /
                static_cast<double>(mmapTotalSize)) / PERCENT_SCALE;
        }
        // 文件级分类：任意栈的任意帧是应用符号 → app 类；否则 system 类
        bool isApp = false;
        for (const auto& cs : info.callStacks) {
            if (FilepageHasAppSymbol(cs, dataDict)) {
                isApp = true;
                break;
            }
        }
        if (isApp) {
            split.app.push_back(std::move(info));
        } else {
            split.system.push_back(std::move(info));
        }
    }
    auto bySize = [](const auto& a, const auto& b) { return a.size > b.size; };
    std::sort(split.app.begin(), split.app.end(), bySize);
    std::sort(split.system.begin(), split.system.end(), bySize);
    // mixed 只在 --scope mixed 时才需要（与符号维度一致）：避免 app-and-system / app-only /
    // system-only 模式下 filepage 数据经 mixed 轨漏进 JSON，造成与 HTML scopeActive 门控不对称。
    if (computeMixed) {
        split.mixed.reserve(split.app.size() + split.system.size());
        split.mixed.insert(split.mixed.end(), split.app.begin(), split.app.end());
        split.mixed.insert(split.mixed.end(), split.system.begin(), split.system.end());
        std::sort(split.mixed.begin(), split.mixed.end(), bySize);
    }
    return split;
}

// ===== Public API (with mmap split) =====
HtraceAnalysisOutput AnalyzeCallStacks(
    const std::vector<CompressedCallStack>& callStacks,
    const std::unordered_map<int, std::string>& dataDict,
    AnalysisConfig config,
    const std::string& mode)
{
    if (!HtraceCategory(config.category).IsMultipart()) {
        return AnalyzeCallStacksCore(callStacks, dataDict, config);
    }

    // category == "mmap": split into mmap-anon (MMAP_ANON) and mmap-filepage (FILE_PAGE_MSG).
    // anon 走 symbol 管道；filepage 走旁路（按文件聚合 + app/system 分类）。
    std::vector<CompressedCallStack> anonStacks;
    std::vector<CompressedCallStack> filepageStacks;
    for (const auto& cs : callStacks) {
        if (cs.type == HtraceEventType::MMAP_ANON) {
            anonStacks.push_back(cs);
        } else if (cs.type == HtraceEventType::FILE_PAGE_MSG) {
            filepageStacks.push_back(cs);
        }
    }

    // mmap-anon 走 symbol 管道（top_symbols 第一轨 = 纯 anon，filepage 走旁路）
    auto output = AnalyzeCallStacksCore(anonStacks, dataDict, config);

    // mmap 总大小 = anon + filepage（percentage 分母）
    long long filepageTotal = 0;
    for (const auto& cs : filepageStacks) {
        filepageTotal += cs.existingSize;
    }
    output.totalLeakSize += filepageTotal;

    // 有效 scope 与 analyze_call_stacks_core 一致（force_scope 优先）。filepage 轨只挂到
    // 活动 scope，与符号维度 + HTML 的 scopeActive 门控对齐：避免 inactive scope 因 filepage
    // 非空而漏进 JSON 输出（如 app-and-system 下 mixed 出现、app-only 下 system/mixed 出现）。
    Scope scope = config.useForceScope ? config.forceScope : config.scope;

    // mmap-filepage 走旁路（按文件聚合 + app/system 分类）；mixed 仅在 --scope mixed 时计算。
    FilepageSplit fpSplit = AnalyzeFilepages(filepageStacks, dataDict, output.totalLeakSize,
        mode, scope == Scope::MIXED);

    // 每个 scope 恒为双轨：[Symbol 轨, Filepage 轨]（空也写，保持 JSON 键结构字节一致）。
    // 三个 scope 一律挂 filepage 轨（非活动 scope 是空轨），保证不变式「mmap ⇒ 每 scope
    // 恰 2 个 section（sections[0]=Symbol、sections[1]=Filepage）」，让 JSON write_scope 与
    // HTML 的 sections[0]/sections[1] 直接下标不会越界（AppAndSystem 下 mixed 也有空双轨）。
    // 轨 label 单一来源 = category 描述符表 tracks[].label（analyzer 只搬运，不自己造显示名）。
    const auto& tracks = HtraceCategory(config.category).tracks;
    AttachFilepages(output.topSymbols.app, std::move(fpSplit.app),
        tracks[0].label.c_str(), tracks[1].label.c_str());
    AttachFilepages(output.topSymbols.system, std::move(fpSplit.system),
        tracks[0].label.c_str(), tracks[1].label.c_str());
    AttachFilepages(output.topSymbols.mixed, std::move(fpSplit.mixed),
        tracks[0].label.c_str(), tracks[1].label.c_str());
    return output;
}

// ===== JSON serialization (kept for formatter use only) =====
static json TopSymbolInfoToJson(const TopSymbolInfo& ts, const std::unordered_map<int, std::string>& dataDict)
{
    json j = {
        {"uid", std::to_string(ts.uid)},
        {"symbol", ts.symbol},
        {"libs", ts.libs},
        {"libOffset", ts.libOffset},
        {"type", ts.type},
        {"totalLeakSize", ts.totalLeakSize},
        {"totalExistingCount", ts.totalExistingCount},
        {"isApp", ts.isApp},
        {"buildId", (!ts.buildIdNull && ts.buildId != 0) ?
            json(dataDict.at(ts.buildId)) : ""}
    };
    // Serialize flame/icicle trees with proper recursive structure
    // Resolve int IDs to strings at output time via data_dict
    {
        json arr = json::array();
        for (const auto& c : ts.flameGraph.children) {
            arr.push_back(c.ToFlameJson(dataDict));
        }
        j["flameGraph"] = arr;
    }
    {
        json arr = json::array();
        for (const auto& c : ts.icicleGraph.children) {
            arr.push_back(c.ToIcicleJson(dataDict));
        }
        j["icicleGraph"] = arr;
    }
    return j;
}

json HtraceAnalysisOutput::SymbolsOf(const ScopeResult& scope) const
{
    json arr = json::array();
    for (const auto& s : scope.sections) {
        for (const auto& r : s.rows) {
            if (const auto* ts = std::get_if<TopSymbolInfo>(&r)) {
                arr.push_back(TopSymbolInfoToJson(*ts, this->dataDict));
            }
        }
    }
    return arr;
}

json HtraceAnalysisOutput::ToJson() const
{
    json j = {
        {"totalLeakSize", totalLeakSize},
        {"algorithmStats", {
            {"orgSetSize", stats.orgSetSize},
            {"filteredSetSize", stats.filteredSetSize},
            {"finalSetSize", stats.finalSetSize},
            {"equivalentGroups", stats.equivalentGroups},
            {"upApproxRemoved", stats.upApproxRemoved},
            {"downApproxRemoved", stats.downApproxRemoved},
            {"representationAdded", stats.representationAdded}
        }}
    };

    json top;
    top["mode"] = topSymbols.mode;
    // Only include arrays relevant to the mode (matching TS behavior)
    // app-and-system: app + system; app-only: app; system-only: system; mixed: mixed
    if (topSymbols.mode == "app-only" || topSymbols.mode == "app-and-system") {
        top["app"] = SymbolsOf(topSymbols.app);
    }
    if (topSymbols.mode == "system-only" || topSymbols.mode == "app-and-system") {
        top["system"] = SymbolsOf(topSymbols.system);
    }
    if (topSymbols.mode == "mixed") {
        top["mixed"] = SymbolsOf(topSymbols.mixed);
    }
    j["topSymbols"] = top;

    return j;
}

// ===== htrace_analyze =====
HtraceAnalysisOutput HtraceAnalyze(TraceDB& db, const RunContext& ctx)
{
    // Read TraceDB from the parse step
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) {
        throw std::runtime_error("htrace_analyze: expected CompressedCallStackList in TraceDB");
    }

    AnalysisConfig acfg;
    acfg.topN = ctx.htrace.topN;
    acfg.scope = ctx.htrace.scope;
    acfg.category = ctx.htrace.category;

    auto analysis = AnalyzeCallStacks(clist->stacks, clist->dict, acfg, db.meta.mode);

    // Populate analysis output with metadata from TraceDB
    analysis.dataDict = std::move(clist->dict);
    analysis.mode = std::move(db.meta.mode);
    analysis.processName = std::move(db.meta.processName);
    analysis.typeSizes = std::move(db.meta.typeSizes);
    analysis.traceMeta = std::move(db.meta);

    int totalTop = static_cast<int>(analysis.topSymbols.app.SymbolCount() +
                        analysis.topSymbols.system.SymbolCount() +
                        analysis.topSymbols.mixed.SymbolCount());
    log::Logok("  Analysis completed. ", totalTop, " top symbols.");

    return analysis;
}

} // namespace mem_analyzer
