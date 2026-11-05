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
#include "jsleak/snapshot_compare.h"
#include "jsleak/property_tag_stage.h"
#include "jsleak/snapshot_parser.h"

#include "config.h"
#include "formatter/json/json_writer.h"
#include "shared/heapsnapshot/heap_snapshot.h"
#include "utils/file_utils.h"
#include "utils/platform.h"

#include "jsleak/formatters/icicle_tree.h"
#include "jsleak/formatters/tree_table.h"

#include "nlohmann/json.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using json = nlohmann::ordered_json;

namespace mem_analyzer {
namespace {

constexpr size_t REF_PATH_RESERVE_HINT = 32;
constexpr int IDENTITY_CACHE_LOAD_DIVISOR = 8;
constexpr size_t PATH_SEPARATOR_LEN = 4;

struct Aggregate {
    std::string key;
    std::string label;
    std::string nodeType;
    std::string path;
    uint64_t count = 0;
    uint64_t selfSize = 0;
    uint64_t retainedSize = 0;
    double retainedMax = 0;
    uint32_t representative = UINT32_MAX;
    uint32_t representativeId = UINT32_MAX;
    std::vector<uint32_t> nodeIds;
    std::vector<std::string> dominatorAncestors;
};

struct SnapshotIndex {
    uint64_t totalSelfSize = 0;
    uint64_t totalHeapSize = 0;
    uint64_t reachableCount = 0;
    uint64_t nodeCount = 0;
    std::unordered_map<std::string, Aggregate> groups;
};

std::pair<std::string, std::string> objectIdentity(const HeapSnapshot& heap, uint32_t index)
{
    // Keep comparison object names aligned with the single-snapshot report.
    // In particular, this reuses HeapSnapshot's existing JSObject formatter,
    // which emits names such as "JSObject(len=34, prop=...)".
    std::string raw = heap.GetExtendedNodeName(index);
    const std::string type = heap.GetNodeTypeName(index);
    if (raw.empty()) {
        raw = heap.GetNodeName(index);
    }
    if (raw.empty()) {
        raw = "(anonymous)";
    }
    return {type + "|" + raw, raw};
}

bool IsRetainerType(const std::string& type)
{
    return type == "object" || type == "array" || type == "closure" ||
           type == "regexp" || type == "concatenated string" || type == "slicedstring";
}

bool IsSourceLocatedArktsObject(const std::string& name)
{
    return name.find('#') != std::string::npos && name.find("(line:") != std::string::npos;
}

std::vector<std::string> dominatorAncestorKeys(
    const HeapSnapshot& heap, uint32_t node,
    std::unordered_map<uint32_t, std::pair<std::string, std::string>>& identityCache)
{
    std::vector<std::string> result;
    uint32_t current = node;
    for (size_t depth = 0; depth <= static_cast<size_t>(heap.nodeCount); ++depth) {
        if (current >= heap.dominators.size()) {
            break;
        }
        const uint32_t parent = heap.dominators[current];
        if (parent == UINT32_MAX || parent == current ||
            parent >= static_cast<uint32_t>(heap.nodeCount)) {
            break;
        }
        auto inserted = identityCache.emplace(parent, std::pair<std::string, std::string>{});
        if (inserted.second) {
            inserted.first->second = objectIdentity(heap, parent);
        }
        result.push_back(inserted.first->second.first);
        current = parent;
    }
    return result;
}

std::string FormatShortestReferencePath(
    const HeapSnapshot& heap, uint32_t node,
    std::unordered_map<uint32_t, std::pair<std::string, std::string>>& identityCache)
{
    constexpr uint32_t invalidNode = UINT32_MAX;
    if (node >= static_cast<uint32_t>(heap.nodeCount) ||
        node >= heap.pathParentIndex.size()) {
        return {};
    }
    std::vector<uint32_t> indices;
    indices.reserve(REF_PATH_RESERVE_HINT);
    uint32_t current = node;
    for (size_t depth = 0; depth <= static_cast<size_t>(heap.nodeCount); ++depth) {
        if (current >= static_cast<uint32_t>(heap.nodeCount)) {
            return {};
        }
        const uint32_t parent = heap.pathParentIndex[current];
        if (parent == invalidNode) {
            return {};
        }
        indices.push_back(current);
        if (parent == current) {
            break;
        }
        current = parent;
    }
    if (indices.empty() || heap.pathParentIndex[indices.back()] != indices.back()) {
        return {};
    }
    std::reverse(indices.begin(), indices.end());
    std::string result;
    for (size_t i = 0; i < indices.size(); ++i) {
        const uint32_t index = indices[i];
        auto inserted = identityCache.emplace(index, std::pair<std::string, std::string>{});
        if (inserted.second) {
            inserted.first->second = objectIdentity(heap, index);
        }
        if (i != 0) {
            result += " -> ";
        }
        result += inserted.first->second.second;
    }
    return result;
}

void ResolveRepresentatives(const HeapSnapshot& heap,
    std::unordered_map<std::string, Aggregate>& groups,
    std::unordered_map<uint32_t, std::pair<std::string, std::string>>& identityCache)
{
    for (auto& entry : groups) {
        Aggregate& group = entry.second;
        if (group.representative == UINT32_MAX) {
            continue;
        }
        group.dominatorAncestors =
            dominatorAncestorKeys(heap, group.representative, identityCache);
        group.path = FormatShortestReferencePath(heap, group.representative, identityCache);
    }
}

/**
 * Check if a node should be skipped during indexing (survived in other snapshot or non-retainer).
 */
static bool ShouldSkipForIndex(const HeapSnapshot& heap, uint32_t index,
    uint64_t selfSize, const std::string& type,
    const HeapSnapshot& other)
{
    // Snapshot comparison is identity based: an object that has the same
    // node ID in both snapshots survived the interval and must not
    // contribute to either side of the diff, even when its retained size
    // or display name changed.
    const uint32_t nodeId = heap.GetNodeId(index);
    if (nodeId != UINT32_MAX &&
        other.nodeIdToIdx.find(nodeId) != other.nodeIdToIdx.end()) {
        return true;
    }
    if (selfSize == 0 || !IsRetainerType(type)) {
        return true;
    }
    return false;
}

SnapshotIndex BuildIndex(const HeapSnapshot& heap, const HeapSnapshot& other)
{
    SnapshotIndex out;
    out.nodeCount = static_cast<uint64_t>(std::max(0, heap.nodeCount));
    out.totalHeapSize = heap.GetTotalHeapSize();

    std::unordered_map<uint32_t, std::pair<std::string, std::string>> identityCache;
    identityCache.reserve(static_cast<size_t>(heap.nodeCount / IDENTITY_CACHE_LOAD_DIVISOR));
    for (int i = 0; i < heap.nodeCount; ++i) {
        const uint32_t index = static_cast<uint32_t>(i);
        const uint64_t selfSize = heap.GetNodeSelfSize(index);
        out.totalSelfSize += selfSize;
        const bool reachable = index < heap.dominators.size() && heap.dominators[index] != UINT32_MAX;
        if (!reachable) {
            continue;
        }
        ++out.reachableCount;
        const std::string type = heap.GetNodeTypeName(index);
        if (ShouldSkipForIndex(heap, index, selfSize, type, other)) {
            continue;
        }
        const uint32_t nodeId = heap.GetNodeId(index);

        auto inserted = identityCache.emplace(index, std::pair<std::string, std::string>{});
        if (inserted.second) {
            inserted.first->second = objectIdentity(heap, index);
        }
        const auto& identity = inserted.first->second;
        auto& group = out.groups[identity.first];
        group.key = identity.first;
        group.label = identity.second;
        group.nodeType = type;
        ++group.count;
        group.selfSize += selfSize;
        const double retained = heap.GetRetainedSize(index);
        group.retainedSize += static_cast<uint64_t>(retained);
        if (nodeId != UINT32_MAX) {
            group.nodeIds.push_back(nodeId);
        }
        if (retained > group.retainedMax) {
            group.retainedMax = retained;
            group.representative = index;
            group.representativeId = heap.GetNodeId(index);
        }
    }
    // Representative details are resolved once per group after aggregation.
    // Computing the shortest reference path and dominator ancestry for every
    // node would be O(N * depth); only one representative per group needs
    // them, so this pass is O(groups * depth).
    ResolveRepresentatives(heap, out.groups, identityCache);
    if (out.totalHeapSize == 0) {
        out.totalHeapSize = out.totalSelfSize;
    }
    return out;
}

const Aggregate& findOrEmpty(const std::unordered_map<std::string, Aggregate>& map, const std::string& key)
{
    static const Aggregate empty;
    const auto it = map.find(key);
    return it == map.end() ? empty : it->second;
}

std::string ChangeKind(const Aggregate& a, const Aggregate& b)
{
    if (a.count == 0 && b.count > 0) {
        return "new";
    }
    if (a.count > 0 && b.count == 0) {
        return "removed";
    }
    if (b.retainedSize != a.retainedSize) {
        return b.retainedSize > a.retainedSize ? "growth" : "shrink";
    }
    if (b.selfSize != a.selfSize) {
        return b.selfSize > a.selfSize ? "growth" : "shrink";
    }
    if (b.count != a.count) {
        return b.count > a.count ? "growth" : "shrink";
    }
    // Equal net size/count can still mean that one object disappeared and a
    // different object was added. That is an identity replacement, not a
    // growth or shrink; report it as "replaced" and use the target side for
    // its details.
    if (!b.nodeIds.empty() || !a.nodeIds.empty()) {
        return "replaced";
    }
    return "stable";
}

json AggregateState(const Aggregate& value)
{
    json result = {{"count", value.count},
                   {"selfSize", value.selfSize},
                   {"retainedSize", value.retainedSize},
                   {"retainedMax", static_cast<uint64_t>(value.retainedMax)}};
    result["representativeNodeIndex"] = value.representative == UINT32_MAX ?
        json(nullptr) : json(value.representative);
    result["representativeNodeId"] = value.representativeId == UINT32_MAX ?
        json(nullptr) : json(value.representativeId);
    result["nodeIds"] = value.nodeIds;
    return result;
}

std::vector<uint32_t> deltaNodeIds(const Aggregate& current, const Aggregate& other)
{
    const std::unordered_set<uint32_t> otherIds(other.nodeIds.begin(), other.nodeIds.end());
    std::vector<uint32_t> result;
    result.reserve(current.nodeIds.size());
    for (const uint32_t nodeId : current.nodeIds) {
        if (otherIds.find(nodeId) == otherIds.end()) {
            result.push_back(nodeId);
        }
    }
    return result;
}

json DeltaRow(const Aggregate& a, const Aggregate& b)
{
    const std::string change = ChangeKind(a, b);
    const bool preferBaseline = change == "removed" || change == "shrink";
    const bool useBaseline = preferBaseline ? !a.path.empty() : b.path.empty() && !a.path.empty();
    const std::string path = useBaseline ? a.path : b.path;
    json baselineState = AggregateState(a);
    json targetState = AggregateState(b);
    baselineState["deltaNodeIds"] = deltaNodeIds(a, b);
    targetState["deltaNodeIds"] = deltaNodeIds(b, a);
    return {
        {"key", b.key.empty() ? a.key : b.key},
        {"label", b.label.empty() ? a.label : b.label},
        {"nodeType", b.nodeType.empty() ? a.nodeType : b.nodeType},
        {"path", path},
        {"pathKind", "shortest-reference"},
        {"pathSnapshot", useBaseline ? "A" : "B"},
        {"sourceLocated", IsSourceLocatedArktsObject(b.label.empty() ? a.label : b.label)},
        {"anonymous", (b.label.empty() ? a.label : b.label).find("#anonymous(") != std::string::npos},
        {"change", change},
        {"baseline", std::move(baselineState)},
        {"target", std::move(targetState)},
        {"delta", {{"count", static_cast<int64_t>(b.count) - static_cast<int64_t>(a.count)},
                    {"selfSize", static_cast<int64_t>(b.selfSize) - static_cast<int64_t>(a.selfSize)},
                    {"retainedSize", static_cast<int64_t>(b.retainedSize) - static_cast<int64_t>(a.retainedSize)},
                    {"retainedMax", static_cast<int64_t>(b.retainedMax) - static_cast<int64_t>(a.retainedMax)}}}
    };
}

json CompareAggregatesComplete(const std::unordered_map<std::string, Aggregate>& baseline,
    const std::unordered_map<std::string, Aggregate>& target)
{
    std::set<std::string> keys;
    for (const auto& item : baseline) {
        keys.insert(item.first);
    }
    for (const auto& item : target) {
        keys.insert(item.first);
    }
    std::vector<json> rows;
    rows.reserve(keys.size());
    for (const auto& key : keys) {
        json row = DeltaRow(findOrEmpty(baseline, key), findOrEmpty(target, key));
        if (row["baseline"]["deltaNodeIds"].empty() &&
            row["target"]["deltaNodeIds"].empty()) {
            continue;
        }
        rows.push_back(std::move(row));
    }
    std::sort(rows.begin(), rows.end(), [](const json& lhs, const json& rhs) {
        const int64_t lr = std::abs(lhs["delta"]["retainedSize"].get<int64_t>());
        const int64_t rr = std::abs(rhs["delta"]["retainedSize"].get<int64_t>());
        if (lr != rr) {
            return lr > rr;
        }
        const int64_t ls = std::abs(lhs["delta"]["selfSize"].get<int64_t>());
        const int64_t rs = std::abs(rhs["delta"]["selfSize"].get<int64_t>());
        if (ls != rs) {
            return ls > rs;
        }
        return std::abs(lhs["delta"]["count"].get<int64_t>()) >
               std::abs(rhs["delta"]["count"].get<int64_t>());
    });
    json result = json::array();
    for (auto& row : rows) {
        result.push_back(std::move(row));
    }
    return result;
}

struct LogicalGrowthResult {
    json groups = json::array();
};

static std::string ResolveLogicalRootKey(
    const json& rows,
    size_t index,
    const std::unordered_map<std::string, size_t>& rowByKey,
    const SnapshotIndex& baseline,
    const SnapshotIndex& target)
{
    const auto& row = rows[index];
    const std::string key = row["key"].get<std::string>();
    if (row["sourceLocated"].get<bool>()) {
        return key;
    }
    const bool existsInTarget = row["target"]["count"].get<uint64_t>() > 0;
    const auto& groups = existsInTarget ? target.groups : baseline.groups;
    const auto aggregateIt = groups.find(key);
    if (aggregateIt == groups.end()) {
        return key;
    }
    std::string nearestSource;
    std::string outermostChanged;
    for (const auto& ancestorKey : aggregateIt->second.dominatorAncestors) {
        const auto rowIt = rowByKey.find(ancestorKey);
        if (rowIt == rowByKey.end()) {
            continue;
        }
        outermostChanged = ancestorKey;
        if (rows[rowIt->second]["sourceLocated"].get<bool>()) {
            nearestSource = ancestorKey;
            break;
        }
    }
    if (!nearestSource.empty()) {
        return nearestSource;
    }
    if (!outermostChanged.empty()) {
        return outermostChanged;
    }
    return key;
}

static void AssignInitialLogicalGroups(
    json& rows,
    const std::unordered_map<std::string, size_t>& rowByKey,
    const SnapshotIndex& baseline,
    const SnapshotIndex& target)
{
    for (size_t i = 0; i < rows.size(); ++i) {
        const std::string key = rows[i]["key"].get<std::string>();
        const std::string rootKey = ResolveLogicalRootKey(rows, i, rowByKey, baseline, target);
        rows[i]["logicalGroupId"] = rootKey;
        rows[i]["includedIn"] = rootKey == key ? json(nullptr) : json(rootKey);
    }
}

static std::unordered_map<std::string, std::vector<size_t>> IndexChangedCandidatesByAncestor(
    const json& rows,
    const SnapshotIndex& baseline,
    const SnapshotIndex& target)
{
    std::unordered_map<std::string, std::vector<size_t>> candidatesByAncestor;
    for (size_t ci = 0; ci < rows.size(); ++ci) {
        const auto& candidate = rows[ci];
        const auto& candidateDelta = candidate["delta"];
        if (!candidate["sourceLocated"].get<bool>() ||
            (candidateDelta["count"].get<int64_t>() == 0 &&
             candidateDelta["selfSize"].get<int64_t>() == 0)) {
            continue;
        }
        const bool existsInTarget = candidate["target"]["count"].get<uint64_t>() > 0;
        const auto& groups = existsInTarget ? target.groups : baseline.groups;
        const auto aggregateIt = groups.find(candidate["key"].get<std::string>());
        if (aggregateIt == groups.end()) {
            continue;
        }
        for (const auto& ancestorKey : aggregateIt->second.dominatorAncestors) {
            candidatesByAncestor[ancestorKey].push_back(ci);
        }
    }
    return candidatesByAncestor;
}

static void ResolveCarrierDescendants(
    json& rows,
    const SnapshotIndex& baseline,
    const SnapshotIndex& target)
{
    // A stable container can gain retained size solely because a changed
    // descendant now owns more memory. Treat such zero-count/zero-self rows as
    // carriers of the descendant growth instead of separate logical growths.
    // Prefer the descendant whose retained delta explains the largest share.
    // Candidates are pre-indexed by their dominator-ancestor keys so each
    // carrier looks up its descendants instead of rescanning every row.
    auto candidatesByAncestor = IndexChangedCandidatesByAncestor(rows, baseline, target);
    for (size_t i = 0; i < rows.size(); ++i) {
        auto& carrier = rows[i];
        const auto& carrierDelta = carrier["delta"];
        if (carrierDelta["count"].get<int64_t>() != 0 ||
            carrierDelta["selfSize"].get<int64_t>() != 0 ||
            carrierDelta["retainedSize"].get<int64_t>() == 0) {
            continue;
        }
        const std::string carrierKey = carrier["key"].get<std::string>();
        const int64_t carrierRetained = carrierDelta["retainedSize"].get<int64_t>();
        const auto matched = candidatesByAncestor.find(carrierKey);
        if (matched == candidatesByAncestor.end()) {
            continue;
        }
        std::string bestDescendant;
        int64_t bestExplained = 0;
        for (const size_t ci : matched->second) {
            const int64_t candidateRetained =
                rows[ci]["delta"]["retainedSize"].get<int64_t>();
            if ((candidateRetained > 0) != (carrierRetained > 0)) {
                continue;
            }
            const int64_t explained = std::min(std::abs(candidateRetained),
                std::abs(carrierRetained));
            if (explained > bestExplained) {
                bestExplained = explained;
                bestDescendant = rows[ci]["key"].get<std::string>();
            }
        }
        if (!bestDescendant.empty()) {
            carrier["logicalGroupId"] = bestDescendant;
            carrier["includedIn"] = bestDescendant;
        }
    }
}

static LogicalGrowthResult AssembleGroupResult(const json& rows)
{
    LogicalGrowthResult result;
    std::unordered_map<std::string, size_t> groupByKey;
    for (const auto& row : rows) {
        const std::string key = row["key"].get<std::string>();
        if (row["logicalGroupId"].get<std::string>() != key) {
            continue;
        }
        json group = row;
        group["components"] = json::array();
        groupByKey[key] = result.groups.size();
        result.groups.push_back(std::move(group));
    }
    for (const auto& row : rows) {
        if (row["includedIn"].is_null()) {
            continue;
        }
        const auto& delta = row["delta"];
        if (delta["count"].get<int64_t>() == 0 && delta["selfSize"].get<int64_t>() == 0) {
            continue;
        }
        const std::string rootKey = row["logicalGroupId"].get<std::string>();
        const auto groupIt = groupByKey.find(rootKey);
        if (groupIt == groupByKey.end()) {
            continue;
        }
        result.groups[groupIt->second]["components"].push_back(row);
    }
    for (auto& group : result.groups) {
        auto& components = group["components"];
        std::sort(components.begin(), components.end(), [](const json& lhs, const json& rhs) {
            return std::abs(lhs["delta"]["selfSize"].get<int64_t>()) >
                   std::abs(rhs["delta"]["selfSize"].get<int64_t>());
        });
    }
    std::sort(result.groups.begin(), result.groups.end(), [](const json& lhs, const json& rhs) {
        const int64_t lr = lhs["delta"]["retainedSize"].get<int64_t>();
        const int64_t rr = rhs["delta"]["retainedSize"].get<int64_t>();
        if (lr != rr) {
            return lr > rr;
        }
        return lhs["delta"]["selfSize"].get<int64_t>() > rhs["delta"]["selfSize"].get<int64_t>();
    });
    return result;
}

LogicalGrowthResult BuildLogicalGrowthGroups(json rows, const SnapshotIndex& baseline, const SnapshotIndex& target)
{
    std::unordered_map<std::string, size_t> rowByKey;
    rowByKey.reserve(rows.size());
    for (size_t i = 0; i < rows.size(); ++i) {
        rowByKey[rows[i]["key"].get<std::string>()] = i;
    }

    AssignInitialLogicalGroups(rows, rowByKey, baseline, target);

    ResolveCarrierDescendants(rows, baseline, target);

    // Resolve transitive assignments (component -> carrier -> logical root).
    for (auto& row : rows) {
        std::string rootKey = row["logicalGroupId"].get<std::string>();
        std::set<std::string> visited;
        while (visited.insert(rootKey).second) {
            const auto rootIt = rowByKey.find(rootKey);
            if (rootIt == rowByKey.end()) {
                break;
            }
            const auto& rootRow = rows[rootIt->second];
            const std::string next = rootRow["logicalGroupId"].get<std::string>();
            if (next == rootKey) {
                break;
            }
            rootKey = next;
        }
        const std::string key = row["key"].get<std::string>();
        row["logicalGroupId"] = rootKey;
        row["includedIn"] = rootKey == key ? json(nullptr) : json(rootKey);
    }

    return AssembleGroupResult(rows);
}

json SelectLogicalGroups(const json& groups, size_t topN)
{
    json selected = json::array();
    size_t positiveCount = 0;
    for (const auto& group : groups) {
        const std::string change = group["change"].get<std::string>();
        if (change == "new" || change == "growth" || change == "replaced") {
            selected.push_back(group);
            if (++positiveCount == topN) {
                break;
            }
        }
    }
    size_t negativeCount = 0;
    for (auto it = groups.rbegin(); it != groups.rend() && negativeCount < topN; ++it) {
        const std::string change = (*it)["change"].get<std::string>();
        if (change == "removed" || change == "shrink") {
            selected.push_back(*it);
            ++negativeCount;
        }
    }
    return selected;
}

json CompactPathDeltas(const json& rows)
{
    json result = json::array();
    for (const auto& row : rows) {
        const std::string pathSnapshot = row.value("pathSnapshot", "B");
        const auto& state = pathSnapshot == "A" ? row["baseline"] : row["target"];
        result.push_back({
            {"objectName", row.value("label", "")},
            {"change", row.value("change", "stable")},
            {"path", row.value("path", "")},
            {"pathKind", row.value("pathKind", "shortest-reference")},
            {"pathSnapshot", pathSnapshot},
            {"retainedSizeDelta", row["delta"].value("retainedSize", int64_t{0})},
            {"selfSizeDelta", row["delta"].value("selfSize", int64_t{0})},
            {"nodeCountDelta", row["delta"].value("count", int64_t{0})},
            {"deltaNodeIds", state.value("deltaNodeIds", json::array())}
        });
    }
    return result;
}

std::string SnapshotInputFormat(const std::string& path)
{
    std::string ext = PathToUtf8(PathFromUtf8(path).extension());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    if (ext == ".heapsnapshot") {
        return "heapsnapshot";
    }
    if (ext == ".rawheap") {
        return "rawheap";
    }
    return {};
}

std::vector<uint32_t> leakEntryNodeIds(const json& entry)
{
    std::vector<uint32_t> result;
    std::unordered_set<uint32_t> seen;
    const auto& chains = entry.value("referenceChains", json::array());
    for (const auto& chain : chains) {
        for (const auto& nodeId : chain.value("nodeIds", json::array())) {
            if (!nodeId.is_number_unsigned()) {
                continue;
            }
            const uint32_t value = nodeId.get<uint32_t>();
            if (seen.insert(value).second) {
                result.push_back(value);
            }
        }
    }
    return result;
}

std::vector<uint32_t> nodeIdDifference(const std::vector<uint32_t>& source, const std::vector<uint32_t>& other)
{
    const std::unordered_set<uint32_t> otherIds(other.begin(), other.end());
    std::vector<uint32_t> result;
    result.reserve(source.size());
    for (const uint32_t nodeId : source) {
        if (otherIds.find(nodeId) == otherIds.end()) {
            result.push_back(nodeId);
        }
    }
    return result;
}

json FilterLeakReferenceChains(const json& chains, const std::vector<uint32_t>& deltaNodeIds)
{
    const std::unordered_set<uint32_t> deltaIds(deltaNodeIds.begin(), deltaNodeIds.end());
    json result = json::array();
    for (const auto& sourceChain : chains) {
        json chain = sourceChain;
        json filteredIds = json::array();
        for (const auto& nodeId : chain.value("nodeIds", json::array())) {
            if (nodeId.is_number_unsigned() &&
                deltaIds.find(nodeId.get<uint32_t>()) != deltaIds.end()) {
                filteredIds.push_back(nodeId);
            }
        }
        if (!deltaIds.empty() && filteredIds.empty()) {
            continue;
        }
        chain["nodeIds"] = std::move(filteredIds);
        result.push_back(std::move(chain));
    }
    return result;
}

// Totals are stored as unsigned JSON numbers; read them as uint64_t and
// derive the signed delta via two's-complement wrap-around, which is exact
// for any realistic heap size and never throws on oversized values.
struct LeakDelta {
    std::string name;
    json entry;
    int64_t sizeDelta = 0;
    std::string change;
};

/// Build leak deltas for all names present in either snapshot.
static std::vector<LeakDelta> BuildLeakDeltas(
    const json& baselineLeaks, const json& targetLeaks,
    const std::set<std::string>& names)
{
    std::vector<LeakDelta> deltas;
    deltas.reserve(names.size());
    for (const auto& name : names) {
        const bool inBaseline = baselineLeaks.contains(name);
        const bool inTarget = targetLeaks.contains(name);
        const uint64_t baselineSize = inBaseline ?
            baselineLeaks[name].value("totalRetainedSize", uint64_t{0}) : 0;
        const uint64_t targetSize = inTarget ?
            targetLeaks[name].value("totalRetainedSize", uint64_t{0}) : 0;
        const uint64_t baselineCount = inBaseline ?
            baselineLeaks[name].value("totalNodeCount", uint64_t{0}) : 0;
        const uint64_t targetCount = inTarget ?
            targetLeaks[name].value("totalNodeCount", uint64_t{0}) : 0;
        const int64_t sizeDelta = static_cast<int64_t>(targetSize - baselineSize);
        const int64_t countDelta = static_cast<int64_t>(targetCount - baselineCount);

        // An entry present on both sides with fully equal totals is an equal
        // replacement (leak analysis runs on the node-ID diff, so the node ids
        // necessarily differ), not a growth.
        const std::string change = !inBaseline ? "new" : (!inTarget ? "removed" :
            (sizeDelta == 0 && countDelta == 0 ? "replaced" :
             (sizeDelta >= 0 ? "growth" : "shrink")));
        const bool useBaseline = change == "removed" || change == "shrink";
        json entry = useBaseline ? baselineLeaks[name] : targetLeaks[name];
        const std::string detailsSnapshot = useBaseline ? "A" : "B";
        const std::vector<uint32_t> baselineNodeIds = inBaseline ?
            leakEntryNodeIds(baselineLeaks[name]) : std::vector<uint32_t>{};
        const std::vector<uint32_t> targetNodeIds = inTarget ?
            leakEntryNodeIds(targetLeaks[name]) : std::vector<uint32_t>{};
        const std::vector<uint32_t> deltaNodeIds = useBaseline ?
            nodeIdDifference(baselineNodeIds, targetNodeIds)
            : nodeIdDifference(targetNodeIds, baselineNodeIds);
        entry["deltaNodeIds"] = deltaNodeIds;
        entry["referenceChains"] = FilterLeakReferenceChains(
            entry.value("referenceChains", json::array()), deltaNodeIds);
        entry["retainedSizeDelta"] = sizeDelta;
        entry["nodeCountDelta"] = countDelta;
        entry["baselineRetainedSize"] = baselineSize;
        entry["targetRetainedSize"] = targetSize;
        entry["baselineNodeCount"] = baselineCount;
        entry["targetNodeCount"] = targetCount;
        entry["detailsSnapshot"] = detailsSnapshot;
        entry["change"] = change;
        deltas.push_back({name, std::move(entry), sizeDelta, change});
    }
    return deltas;
}

/// Sort deltas by absolute size and select top N per direction.
// Same per-direction budget as the object path: top_n positive + top_n negative.
static json SelectTopLeakDeltas(std::vector<LeakDelta>& deltas, size_t topN)
{
    std::sort(deltas.begin(), deltas.end(), [](const LeakDelta& lhs, const LeakDelta& rhs) {
        const int64_t ls = std::abs(lhs.sizeDelta);
        const int64_t rs = std::abs(rhs.sizeDelta);
        if (ls != rs) {
            return ls > rs;
        }
        return lhs.name < rhs.name;
    });
    const auto isPositive = [](const std::string& change) {
        return change == "new" || change == "growth" || change == "replaced";
    };
    json result = json::object();
    size_t positiveCount = 0;
    size_t negativeCount = 0;
    for (auto& delta : deltas) {
        const bool positive = isPositive(delta.change);
        if (positive ? positiveCount >= topN : negativeCount >= topN) {
            continue;
        }
        if (positive) {
            ++positiveCount;
        } else {
            ++negativeCount;
        }
        result[delta.name] = std::move(delta.entry);
    }
    return result;
}

json BuildChangedLeakObjects(const json& baselineLeaks, const json& targetLeaks, size_t topN)
{
    std::set<std::string> names;
    for (auto it = baselineLeaks.begin(); it != baselineLeaks.end(); ++it) {
        names.insert(it.key());
    }
    for (auto it = targetLeaks.begin(); it != targetLeaks.end(); ++it) {
        names.insert(it.key());
    }

    auto deltas = BuildLeakDeltas(baselineLeaks, targetLeaks, names);
    return SelectTopLeakDeltas(deltas, topN);
}

} // namespace

bool JsleakCompareInputsValid(const std::string& baselineFile, const std::string& targetFile, std::string& error)
{
    const std::string baselineFormat = SnapshotInputFormat(baselineFile);
    const std::string targetFormat = SnapshotInputFormat(targetFile);
    if (baselineFormat.empty() || targetFormat.empty()) {
        error = "Snapshot comparison accepts only .heapsnapshot or .rawheap files";
        return false;
    }
    if (baselineFormat != targetFormat) {
        error = "Baseline and target must use the same snapshot format";
        return false;
    }
    return true;
}

namespace {

// 与 build_index 的 heapSize 口径一致：get_total_heap_size() 为 0 时回退逐节点求和。
uint64_t HeapSizeOf(const HeapSnapshot& heap)
{
    uint64_t size = heap.GetTotalHeapSize();
    if (size != 0) {
        return size;
    }
    for (int i = 0; i < heap.nodeCount; ++i) {
        size += heap.GetNodeSelfSize(static_cast<uint32_t>(i));
    }
    return size;
}

json BuildPathDeltasIcicleChart(const json& pathDeltas)
{
    json chainItems = json::array();
    for (const auto& delta : pathDeltas) {
        if (!delta.contains("path") || !delta["path"].is_string()) {
            continue;
        }
        std::string pathStr = delta["path"].get<std::string>();
        if (pathStr.empty()) {
            continue;
        }
        json frames = json::array();
        size_t start = 0;
        while (start < pathStr.size()) {
            size_t sep = pathStr.find(" -> ", start);
            if (sep == std::string::npos) {
                frames.push_back(pathStr.substr(start));
                break;
            }
            frames.push_back(pathStr.substr(start, sep - start));
            start = sep + PATH_SEPARATOR_LEN;
        }
        json item;
        item["chain"] = std::move(frames);
        item["totalRetainedSize"] = std::abs(
            delta.value("retainedSizeDelta", static_cast<int64_t>(0)));
        item["nodeIds"] = delta.value("deltaNodeIds", json::array());
        chainItems.push_back(std::move(item));
    }
    if (!chainItems.empty()) {
        json tree = BuildRefChainIcicleTree(chainItems, "GC Roots");
        if (tree.contains("c") && tree["c"].is_array() && !tree["c"].empty()) {
            const std::string canvasId = "icicleCanvas_pathDeltas";
            return json({
                {"id", canvasId},
                {"tree", tree},
                {"chartHtml", BuildIcicleChartBody(
                    canvasId, TreeTableHtml(tree, canvasId))}});
        }
    }
    return json(nullptr);
}

json BuildLeakObjIcicleChart(const json& changedLeakObjects)
{
    json chainItems = json::array();
    for (const auto& [name, entry] : changedLeakObjects.items()) {
        for (const auto& chain : entry.value("referenceChains", json::array())) {
            const json& frames = chain.value("chain", json::array());
            if (!frames.is_array() || frames.empty()) {
                continue;
            }
            json item;
            item["chain"] = frames;
            item["totalRetainedSize"] = std::abs(
                entry.value("retainedSizeDelta", static_cast<int64_t>(0)));
            item["nodeIds"] = chain.value("nodeIds", json::array());
            chainItems.push_back(std::move(item));
        }
    }
    if (!chainItems.empty()) {
        json tree = BuildRefChainIcicleTree(chainItems, "GC Roots");
        if (tree.contains("c") && tree["c"].is_array() && !tree["c"].empty()) {
            const std::string canvasId = "icicleCanvas_leakObj";
            return json({
                {"id", canvasId},
                {"tree", tree},
                {"chartHtml", BuildIcicleChartBody(
                    canvasId, TreeTableHtml(tree, canvasId))}});
        }
    }
    return json(nullptr);
}

static json BuildCompareIcicleCharts(bool wantChains, bool wantLeaks,
    const json& pathDeltas, const json& changedLeakObjects)
{
    json icicleCharts = json::array();
    if (wantChains && !pathDeltas.empty()) {
        auto chart = BuildPathDeltasIcicleChart(pathDeltas);
        if (!chart.is_null()) {
            icicleCharts.push_back(std::move(chart));
        }
    }
    if (wantLeaks && !changedLeakObjects.empty()) {
        auto chart = BuildLeakObjIcicleChart(changedLeakObjects);
        if (!chart.is_null()) {
            icicleCharts.push_back(std::move(chart));
        }
    }
    return icicleCharts;
}

static json BuildCompareMethodology()
{
    return {
        {"objectIdentity", "snapshot node ID; IDs present in both snapshots are excluded "
                           "before aggregation"},
        {"pathMatching", "one representative shortest reference path per identity group; "
                         "new/growth/replaced rows report snapshot B's path (falling back to A "
                         "when B has none), removed/shrink rows report A's; weak, shortcut, and "
                         "hidden edges excluded from the path search"},
        {"retainedMetric", "summed retained size of added or removed nodes per exact "
                           "extended-name object bucket"},
        {"logicalGrouping", "dominator ancestry folds generic descendants and zero-count/zero-self "
                            "carrier ancestors into the changed source-located descendant that "
                            "explains their retained growth; component retained size is not "
                            "added again"},
        {"leakCandidateScope", "leak-rule matches among added and removed node IDs only; "
                               "surviving node IDs are excluded"},
        {"leakDetection", "property-tag rules only, as in the single-snapshot ArkUI/ArkTS leak "
                          "report; JSLeakWatcher hash rules are not applied because comparison "
                          "loads no leak list"},
        {"presentationFiltering", "JSON and HTML contain the same top-n logical changes and top-n "
                                  "changed leak objects per direction; only delta node IDs are "
                                  "retained; the analysis mode (all/property-tag/top-chain) "
                                  "selects which sections are produced"}
    };
}

struct CompareAnalysisResult {
    json pathDeltas;
    json changedLeakObjects;
    json icicleCharts;
};

static json BuildCompareReport(const CompareAnalysisInput& input,
    const CompareAnalysisResult& result)
{
    const std::string interpretation =
        input.mode == "property-tag" ? "difference-with-property-tag-leak-analysis" :
        input.mode == "top-chain"  ? "difference-with-shortest-reference-chain-analysis" :
                               "difference-with-changed-object-leak-analysis";
    return {
        {"schemaVersion", 6},
        {"reportType", "snapshot-difference"},
        {"generatedAt", FormatReadableTimestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())},
        {"duration", "未知（快照对比不采集运行时长）"},
        {"process", "未知（快照文件不含进程信息）"},
        {"mode", input.mode},
        {"inputFormat", SnapshotInputFormat(input.baselineFile)},
        {"analysisType", SnapshotInputFormat(input.baselineFile) + " comparison"},
        {"interpretation", interpretation},
        {"baseline", {{"role", "baseline/control"}, {"file", platform::Basename(input.baselineFile)},
                       {"heapSize", HeapSizeOf(input.baseline)}}},
        {"target", {{"role", "target/treatment"}, {"file", platform::Basename(input.targetFile)},
                     {"heapSize", HeapSizeOf(input.target)}}},
        {"summary", {{"deltaHeapSize", static_cast<int64_t>(HeapSizeOf(input.target)) -
                                        static_cast<int64_t>(HeapSizeOf(input.baseline))}}},
        {"pathDeltas", result.pathDeltas},
        {"leakObjInfo", result.changedLeakObjects},
        {"icicleCharts", result.icicleCharts},
        {"limits", {{"topNPerDirection", input.topN}}},
        {"methodology", BuildCompareMethodology()}
    };
}

json JsleakCompareAnalyzeImpl(const CompareAnalysisInput& input)
{
    // -m 选择输出维度：property-tag 跳过引用链聚合，top-chain 跳过泄漏规则。
    const bool wantChains = input.mode != "property-tag";
    const bool wantLeaks = input.mode != "top-chain";

    json pathDeltas = json::array();
    json changedLeakObjects = json::object();
    if (wantChains) {
        // Comparison indexes are built from the symmetric node-ID difference;
        // surviving objects never enter object/path aggregation.
        const SnapshotIndex a = BuildIndex(input.baseline, input.target);
        const SnapshotIndex b = BuildIndex(input.target, input.baseline);
        json completeObjectDeltas = CompareAggregatesComplete(a.groups, b.groups);
        LogicalGrowthResult logical = BuildLogicalGrowthGroups(
            std::move(completeObjectDeltas), a, b);
        pathDeltas = CompactPathDeltas(SelectLogicalGroups(logical.groups, input.topN));
    }
    if (wantLeaks) {
        // Leak rules run on the node-ID diff only: survivors are excluded on
        // both sides, matching the object-diff scope above.
        PropertyTagStage LeakProcessor("");
        const json baselineLeakObjects = LeakProcessor.Analyze(
            input.baseline, [&input](uint32_t nodeIndex) {
                const uint32_t nodeId = input.baseline.GetNodeId(nodeIndex);
                return nodeId != UINT32_MAX &&
                    input.target.nodeIdToIdx.find(nodeId) == input.target.nodeIdToIdx.end();
            });
        const json targetLeakObjects = LeakProcessor.Analyze(
            input.target, [&input](uint32_t nodeIndex) {
                const uint32_t nodeId = input.target.GetNodeId(nodeIndex);
                return nodeId != UINT32_MAX &&
                    input.baseline.nodeIdToIdx.find(nodeId) == input.baseline.nodeIdToIdx.end();
            });
        changedLeakObjects = BuildChangedLeakObjects(
            baselineLeakObjects, targetLeakObjects, input.topN);
    }

    // Build icicle chart trees for both sections (same pattern as single-snapshot report):
    // each reference chain becomes a path in a prefix-trie, aggregated by frame sequence.
    json icicleCharts = BuildCompareIcicleCharts(wantChains, wantLeaks,
        pathDeltas, changedLeakObjects);

    return BuildCompareReport(
        input,
        CompareAnalysisResult{std::move(pathDeltas), changedLeakObjects,
            std::move(icicleCharts)});
}

} // namespace

json JsleakCompareAnalyze(const CompareAnalysisInput& input)
{
    return JsleakCompareAnalyzeImpl(
        CompareAnalysisInput{input.baseline, input.target,
            input.baselineFile, input.targetFile,
            std::max(1, input.topN), input.mode});
}

} // namespace mem_analyzer
