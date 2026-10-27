#include "jsleak/snapshot_compare.hpp"
#include "jsleak/property_tag_stage.hpp"
#include "jsleak/snapshot_parser.hpp"

#include "config.hpp"
#include "formatter/json/json_writer.hpp"
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"

#include "jsleak/formatters/icicle_tree.hpp"
#include "jsleak/formatters/tree_table.hpp"

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

struct Aggregate {
    std::string key;
    std::string label;
    std::string node_type;
    std::string path;
    uint64_t count = 0;
    uint64_t self_size = 0;
    uint64_t retained_size = 0;
    double retained_max = 0;
    uint32_t representative = UINT32_MAX;
    uint32_t representative_id = UINT32_MAX;
    std::vector<uint32_t> node_ids;
    std::vector<std::string> dominator_ancestors;
};

struct SnapshotIndex {
    uint64_t total_self_size = 0;
    uint64_t total_heap_size = 0;
    uint64_t reachable_count = 0;
    uint64_t node_count = 0;
    std::unordered_map<std::string, Aggregate> groups;
};

std::pair<std::string, std::string> object_identity(const HeapSnapshot& heap, uint32_t index) {
    // Keep comparison object names aligned with the single-snapshot report.
    // In particular, this reuses HeapSnapshot's existing JSObject formatter,
    // which emits names such as "JSObject(len=34, prop=...)".
    std::string raw = heap.get_extended_node_name(index);
    const std::string type = heap.get_node_type_name(index);
    if (raw.empty()) raw = heap.get_node_name(index);
    if (raw.empty()) raw = "(anonymous)";
    return {type + "|" + raw, raw};
}

bool is_retainer_type(const std::string& type) {
    return type == "object" || type == "array" || type == "closure" ||
           type == "regexp" || type == "concatenated string" || type == "slicedstring";
}

bool is_source_located_arkts_object(const std::string& name) {
    return name.find('#') != std::string::npos && name.find("(line:") != std::string::npos;
}

std::vector<std::string> dominator_ancestor_keys(
    const HeapSnapshot& heap, uint32_t node,
    std::unordered_map<uint32_t, std::pair<std::string, std::string>>& identity_cache) {
    std::vector<std::string> result;
    uint32_t current = node;
    for (size_t depth = 0; depth <= static_cast<size_t>(heap.node_count); ++depth) {
        if (current >= heap.dominators.size()) break;
        const uint32_t parent = heap.dominators[current];
        if (parent == UINT32_MAX || parent == current ||
            parent >= static_cast<uint32_t>(heap.node_count)) {
            break;
        }
        auto inserted = identity_cache.emplace(parent, std::pair<std::string, std::string>{});
        if (inserted.second) inserted.first->second = object_identity(heap, parent);
        result.push_back(inserted.first->second.first);
        current = parent;
    }
    return result;
}

std::string format_shortest_reference_path(
    const HeapSnapshot& heap, uint32_t node,
    std::unordered_map<uint32_t, std::pair<std::string, std::string>>& identity_cache) {
    constexpr uint32_t INVALID = UINT32_MAX;
    if (node >= static_cast<uint32_t>(heap.node_count) ||
        node >= heap.path_parent_index.size()) {
        return {};
    }
    std::vector<uint32_t> indices;
    indices.reserve(32);
    uint32_t current = node;
    for (size_t depth = 0; depth <= static_cast<size_t>(heap.node_count); ++depth) {
        if (current >= static_cast<uint32_t>(heap.node_count)) return {};
        const uint32_t parent = heap.path_parent_index[current];
        if (parent == INVALID) return {};
        indices.push_back(current);
        if (parent == current) break;
        current = parent;
    }
    if (indices.empty() || heap.path_parent_index[indices.back()] != indices.back()) return {};
    std::reverse(indices.begin(), indices.end());
    std::string result;
    for (size_t i = 0; i < indices.size(); ++i) {
        const uint32_t index = indices[i];
        auto inserted = identity_cache.emplace(index, std::pair<std::string, std::string>{});
        if (inserted.second) inserted.first->second = object_identity(heap, index);
        if (i != 0) result += " -> ";
        result += inserted.first->second.second;
    }
    return result;
}

SnapshotIndex build_index(const HeapSnapshot& heap, const HeapSnapshot& other) {
    SnapshotIndex out;
    out.node_count = static_cast<uint64_t>(std::max(0, heap.node_count));
    out.total_heap_size = heap.get_total_heap_size();

    std::unordered_map<uint32_t, std::pair<std::string, std::string>> identity_cache;
    identity_cache.reserve(static_cast<size_t>(heap.node_count / 8));
    for (int i = 0; i < heap.node_count; ++i) {
        const uint32_t index = static_cast<uint32_t>(i);
        const uint64_t self_size = heap.get_node_self_size(index);
        out.total_self_size += self_size;
        const bool reachable = index < heap.dominators.size() && heap.dominators[index] != UINT32_MAX;
        if (!reachable) continue;
        ++out.reachable_count;
        // Snapshot comparison is identity based: an object that has the same
        // node ID in both snapshots survived the interval and must not
        // contribute to either side of the diff, even when its retained size
        // or display name changed.
        const uint32_t node_id = heap.get_node_id(index);
        if (node_id != UINT32_MAX &&
            other.node_id_to_idx.find(node_id) != other.node_id_to_idx.end()) {
            continue;
        }
        const std::string type = heap.get_node_type_name(index);
        if (self_size == 0 || !is_retainer_type(type)) continue;

        auto inserted = identity_cache.emplace(index, std::pair<std::string, std::string>{});
        if (inserted.second) inserted.first->second = object_identity(heap, index);
        const auto& identity = inserted.first->second;
        auto& group = out.groups[identity.first];
        group.key = identity.first;
        group.label = identity.second;
        group.node_type = type;
        ++group.count;
        group.self_size += self_size;
        const double retained = heap.get_retained_size(index);
        group.retained_size += static_cast<uint64_t>(retained);
        if (node_id != UINT32_MAX) group.node_ids.push_back(node_id);
        if (retained > group.retained_max) {
            group.retained_max = retained;
            group.representative = index;
            group.representative_id = heap.get_node_id(index);
        }
    }
    // Representative details are resolved once per group after aggregation.
    // Computing the shortest reference path and dominator ancestry for every
    // node would be O(N * depth); only one representative per group needs
    // them, so this pass is O(groups * depth).
    for (auto& entry : out.groups) {
        Aggregate& group = entry.second;
        if (group.representative == UINT32_MAX) continue;
        group.dominator_ancestors =
            dominator_ancestor_keys(heap, group.representative, identity_cache);
        group.path = format_shortest_reference_path(heap, group.representative, identity_cache);
    }
    if (out.total_heap_size == 0) out.total_heap_size = out.total_self_size;
    return out;
}

const Aggregate& find_or_empty(const std::unordered_map<std::string, Aggregate>& map,
                               const std::string& key) {
    static const Aggregate empty;
    const auto it = map.find(key);
    return it == map.end() ? empty : it->second;
}

std::string change_kind(const Aggregate& a, const Aggregate& b) {
    if (a.count == 0 && b.count > 0) return "new";
    if (a.count > 0 && b.count == 0) return "removed";
    if (b.retained_size != a.retained_size) {
        return b.retained_size > a.retained_size ? "growth" : "shrink";
    }
    if (b.self_size != a.self_size) {
        return b.self_size > a.self_size ? "growth" : "shrink";
    }
    if (b.count != a.count) return b.count > a.count ? "growth" : "shrink";
    // Equal net size/count can still mean that one object disappeared and a
    // different object was added. That is an identity replacement, not a
    // growth or shrink; report it as "replaced" and use the target side for
    // its details.
    if (!b.node_ids.empty() || !a.node_ids.empty()) return "replaced";
    return "stable";
}

json aggregate_state(const Aggregate& value) {
    json result = {{"count", value.count},
                   {"selfSize", value.self_size},
                   {"retainedSize", value.retained_size},
                   {"retainedMax", static_cast<uint64_t>(value.retained_max)}};
    result["representativeNodeIndex"] = value.representative == UINT32_MAX
        ? json(nullptr) : json(value.representative);
    result["representativeNodeId"] = value.representative_id == UINT32_MAX
        ? json(nullptr) : json(value.representative_id);
    result["nodeIds"] = value.node_ids;
    return result;
}

std::vector<uint32_t> delta_node_ids(const Aggregate& current, const Aggregate& other) {
    const std::unordered_set<uint32_t> other_ids(other.node_ids.begin(), other.node_ids.end());
    std::vector<uint32_t> result;
    result.reserve(current.node_ids.size());
    for (const uint32_t node_id : current.node_ids) {
        if (other_ids.find(node_id) == other_ids.end()) result.push_back(node_id);
    }
    return result;
}

json delta_row(const Aggregate& a, const Aggregate& b) {
    const std::string change = change_kind(a, b);
    const bool prefer_baseline = change == "removed" || change == "shrink";
    const bool use_baseline = prefer_baseline ? !a.path.empty() : b.path.empty() && !a.path.empty();
    const std::string path = use_baseline ? a.path : b.path;
    json baseline_state = aggregate_state(a);
    json target_state = aggregate_state(b);
    baseline_state["deltaNodeIds"] = delta_node_ids(a, b);
    target_state["deltaNodeIds"] = delta_node_ids(b, a);
    return {
        {"key", b.key.empty() ? a.key : b.key},
        {"label", b.label.empty() ? a.label : b.label},
        {"nodeType", b.node_type.empty() ? a.node_type : b.node_type},
        {"path", path},
        {"pathKind", "shortest-reference"},
        {"pathSnapshot", use_baseline ? "A" : "B"},
        {"sourceLocated", is_source_located_arkts_object(b.label.empty() ? a.label : b.label)},
        {"anonymous", (b.label.empty() ? a.label : b.label).find("#anonymous(") != std::string::npos},
        {"change", change},
        {"baseline", std::move(baseline_state)},
        {"target", std::move(target_state)},
        {"delta", {{"count", static_cast<int64_t>(b.count) - static_cast<int64_t>(a.count)},
                    {"selfSize", static_cast<int64_t>(b.self_size) - static_cast<int64_t>(a.self_size)},
                    {"retainedSize", static_cast<int64_t>(b.retained_size) - static_cast<int64_t>(a.retained_size)},
                    {"retainedMax", static_cast<int64_t>(b.retained_max) - static_cast<int64_t>(a.retained_max)}}}
    };
}

json compare_aggregates_complete(const std::unordered_map<std::string, Aggregate>& baseline,
                                 const std::unordered_map<std::string, Aggregate>& target) {
    std::set<std::string> keys;
    for (const auto& item : baseline) keys.insert(item.first);
    for (const auto& item : target) keys.insert(item.first);
    std::vector<json> rows;
    rows.reserve(keys.size());
    for (const auto& key : keys) {
        json row = delta_row(find_or_empty(baseline, key), find_or_empty(target, key));
        if (row["baseline"]["deltaNodeIds"].empty() &&
            row["target"]["deltaNodeIds"].empty()) {
            continue;
        }
        rows.push_back(std::move(row));
    }
    std::sort(rows.begin(), rows.end(), [](const json& lhs, const json& rhs) {
        const int64_t lr = std::abs(lhs["delta"]["retainedSize"].get<int64_t>());
        const int64_t rr = std::abs(rhs["delta"]["retainedSize"].get<int64_t>());
        if (lr != rr) return lr > rr;
        const int64_t ls = std::abs(lhs["delta"]["selfSize"].get<int64_t>());
        const int64_t rs = std::abs(rhs["delta"]["selfSize"].get<int64_t>());
        if (ls != rs) return ls > rs;
        return std::abs(lhs["delta"]["count"].get<int64_t>()) >
               std::abs(rhs["delta"]["count"].get<int64_t>());
    });
    json result = json::array();
    for (auto& row : rows) result.push_back(std::move(row));
    return result;
}

struct LogicalGrowthResult {
    json groups = json::array();
};

LogicalGrowthResult build_logical_growth_groups(
    json rows, const SnapshotIndex& baseline, const SnapshotIndex& target) {
    std::unordered_map<std::string, size_t> row_by_key;
    row_by_key.reserve(rows.size());
    for (size_t i = 0; i < rows.size(); ++i) {
        row_by_key[rows[i]["key"].get<std::string>()] = i;
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        auto& row = rows[i];
        const std::string key = row["key"].get<std::string>();
        std::string root_key = key;
        if (!row["sourceLocated"].get<bool>()) {
            const bool exists_in_target = row["target"]["count"].get<uint64_t>() > 0;
            const auto& groups = exists_in_target ? target.groups : baseline.groups;
            const auto aggregate_it = groups.find(key);
            std::string nearest_source;
            std::string outermost_changed;
            if (aggregate_it != groups.end()) {
                for (const auto& ancestor_key : aggregate_it->second.dominator_ancestors) {
                    const auto row_it = row_by_key.find(ancestor_key);
                    if (row_it == row_by_key.end()) continue;
                    outermost_changed = ancestor_key;
                    if (rows[row_it->second]["sourceLocated"].get<bool>()) {
                        nearest_source = ancestor_key;
                        break;
                    }
                }
            }
            if (!nearest_source.empty()) root_key = nearest_source;
            else if (!outermost_changed.empty()) root_key = outermost_changed;
        }
        row["logicalGroupId"] = root_key;
        row["includedIn"] = root_key == key ? json(nullptr) : json(root_key);
    }

    // A stable container can gain retained size solely because a changed
    // descendant now owns more memory. Treat such zero-count/zero-self rows as
    // carriers of the descendant growth instead of separate logical growths.
    // Prefer the descendant whose retained delta explains the largest share.
    // Candidates are pre-indexed by their dominator-ancestor keys so each
    // carrier looks up its descendants instead of rescanning every row.
    std::unordered_map<std::string, std::vector<size_t>> candidates_by_ancestor;
    for (size_t ci = 0; ci < rows.size(); ++ci) {
        const auto& candidate = rows[ci];
        const auto& candidate_delta = candidate["delta"];
        if (!candidate["sourceLocated"].get<bool>() ||
            (candidate_delta["count"].get<int64_t>() == 0 &&
             candidate_delta["selfSize"].get<int64_t>() == 0)) {
            continue;
        }
        const bool exists_in_target = candidate["target"]["count"].get<uint64_t>() > 0;
        const auto& groups = exists_in_target ? target.groups : baseline.groups;
        const auto aggregate_it = groups.find(candidate["key"].get<std::string>());
        if (aggregate_it == groups.end()) continue;
        for (const auto& ancestor_key : aggregate_it->second.dominator_ancestors) {
            candidates_by_ancestor[ancestor_key].push_back(ci);
        }
    }
    for (size_t i = 0; i < rows.size(); ++i) {
        auto& carrier = rows[i];
        const auto& carrier_delta = carrier["delta"];
        if (carrier_delta["count"].get<int64_t>() != 0 ||
            carrier_delta["selfSize"].get<int64_t>() != 0 ||
            carrier_delta["retainedSize"].get<int64_t>() == 0) {
            continue;
        }
        const std::string carrier_key = carrier["key"].get<std::string>();
        const int64_t carrier_retained = carrier_delta["retainedSize"].get<int64_t>();
        const auto matched = candidates_by_ancestor.find(carrier_key);
        if (matched == candidates_by_ancestor.end()) continue;
        std::string best_descendant;
        int64_t best_explained = 0;
        for (const size_t ci : matched->second) {
            const int64_t candidate_retained =
                rows[ci]["delta"]["retainedSize"].get<int64_t>();
            if ((candidate_retained > 0) != (carrier_retained > 0)) continue;
            const int64_t explained = std::min(std::abs(candidate_retained),
                                               std::abs(carrier_retained));
            if (explained > best_explained) {
                best_explained = explained;
                best_descendant = rows[ci]["key"].get<std::string>();
            }
        }
        if (!best_descendant.empty()) {
            carrier["logicalGroupId"] = best_descendant;
            carrier["includedIn"] = best_descendant;
        }
    }

    // Resolve transitive assignments (component -> carrier -> logical root).
    for (auto& row : rows) {
        std::string root_key = row["logicalGroupId"].get<std::string>();
        std::set<std::string> visited;
        while (visited.insert(root_key).second) {
            const auto root_it = row_by_key.find(root_key);
            if (root_it == row_by_key.end()) break;
            const auto& root_row = rows[root_it->second];
            const std::string next = root_row["logicalGroupId"].get<std::string>();
            if (next == root_key) break;
            root_key = next;
        }
        const std::string key = row["key"].get<std::string>();
        row["logicalGroupId"] = root_key;
        row["includedIn"] = root_key == key ? json(nullptr) : json(root_key);
    }

    LogicalGrowthResult result;
    std::unordered_map<std::string, size_t> group_by_key;
    for (const auto& row : rows) {
        const std::string key = row["key"].get<std::string>();
        if (row["logicalGroupId"].get<std::string>() != key) continue;
        json group = row;
        group["components"] = json::array();
        group_by_key[key] = result.groups.size();
        result.groups.push_back(std::move(group));
    }
    for (const auto& row : rows) {
        if (row["includedIn"].is_null()) continue;
        const auto& delta = row["delta"];
        if (delta["count"].get<int64_t>() == 0 && delta["selfSize"].get<int64_t>() == 0) {
            continue;
        }
        const std::string root_key = row["logicalGroupId"].get<std::string>();
        const auto group_it = group_by_key.find(root_key);
        if (group_it == group_by_key.end()) continue;
        result.groups[group_it->second]["components"].push_back(row);
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
        if (lr != rr) return lr > rr;
        return lhs["delta"]["selfSize"].get<int64_t>() >
               rhs["delta"]["selfSize"].get<int64_t>();
    });
    return result;
}

json select_logical_groups(const json& groups, size_t top_n) {
    json selected = json::array();
    size_t positive_count = 0;
    for (const auto& group : groups) {
        const std::string change = group["change"].get<std::string>();
        if (change == "new" || change == "growth" || change == "replaced") {
            selected.push_back(group);
            if (++positive_count == top_n) break;
        }
    }
    size_t negative_count = 0;
    for (auto it = groups.rbegin(); it != groups.rend() && negative_count < top_n; ++it) {
        const std::string change = (*it)["change"].get<std::string>();
        if (change == "removed" || change == "shrink") {
            selected.push_back(*it);
            ++negative_count;
        }
    }
    return selected;
}

json compact_path_deltas(const json& rows) {
    json result = json::array();
    for (const auto& row : rows) {
        const std::string path_snapshot = row.value("pathSnapshot", "B");
        const auto& state = path_snapshot == "A" ? row["baseline"] : row["target"];
        result.push_back({
            {"objectName", row.value("label", "")},
            {"change", row.value("change", "stable")},
            {"path", row.value("path", "")},
            {"pathKind", row.value("pathKind", "shortest-reference")},
            {"pathSnapshot", path_snapshot},
            {"retainedSizeDelta", row["delta"].value("retainedSize", int64_t{0})},
            {"selfSizeDelta", row["delta"].value("selfSize", int64_t{0})},
            {"nodeCountDelta", row["delta"].value("count", int64_t{0})},
            {"deltaNodeIds", state.value("deltaNodeIds", json::array())}
        });
    }
    return result;
}

std::string snapshot_input_format(const std::string& path) {
    std::string ext = path_to_utf8(path_from_utf8(path).extension());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    if (ext == ".heapsnapshot") return "heapsnapshot";
    if (ext == ".rawheap") return "rawheap";
    return {};
}

std::vector<uint32_t> leak_entry_node_ids(const json& entry) {
    std::vector<uint32_t> result;
    std::unordered_set<uint32_t> seen;
    const auto& chains = entry.value("referenceChains", json::array());
    for (const auto& chain : chains) {
        for (const auto& node_id : chain.value("nodeIds", json::array())) {
            if (!node_id.is_number_unsigned()) continue;
            const uint32_t value = node_id.get<uint32_t>();
            if (seen.insert(value).second) result.push_back(value);
        }
    }
    return result;
}

std::vector<uint32_t> node_id_difference(const std::vector<uint32_t>& source,
                                         const std::vector<uint32_t>& other) {
    const std::unordered_set<uint32_t> other_ids(other.begin(), other.end());
    std::vector<uint32_t> result;
    result.reserve(source.size());
    for (const uint32_t node_id : source) {
        if (other_ids.find(node_id) == other_ids.end()) result.push_back(node_id);
    }
    return result;
}

json filter_leak_reference_chains(const json& chains,
                                  const std::vector<uint32_t>& delta_node_ids) {
    const std::unordered_set<uint32_t> delta_ids(delta_node_ids.begin(), delta_node_ids.end());
    json result = json::array();
    for (const auto& source_chain : chains) {
        json chain = source_chain;
        json filtered_ids = json::array();
        for (const auto& node_id : chain.value("nodeIds", json::array())) {
            if (node_id.is_number_unsigned() &&
                delta_ids.find(node_id.get<uint32_t>()) != delta_ids.end()) {
                filtered_ids.push_back(node_id);
            }
        }
        if (!delta_ids.empty() && filtered_ids.empty()) continue;
        chain["nodeIds"] = std::move(filtered_ids);
        result.push_back(std::move(chain));
    }
    return result;
}

json build_changed_leak_objects(const json& baseline_leaks,
                                const json& target_leaks,
                                size_t top_n) {
    std::set<std::string> names;
    for (auto it = baseline_leaks.begin(); it != baseline_leaks.end(); ++it) names.insert(it.key());
    for (auto it = target_leaks.begin(); it != target_leaks.end(); ++it) names.insert(it.key());

    // Totals are stored as unsigned JSON numbers; read them as uint64_t and
    // derive the signed delta via two's-complement wrap-around, which is exact
    // for any realistic heap size and never throws on oversized values.
    struct LeakDelta {
        std::string name;
        json entry;
        int64_t size_delta = 0;
        std::string change;
    };
    std::vector<LeakDelta> deltas;
    deltas.reserve(names.size());
    for (const auto& name : names) {
        const bool in_baseline = baseline_leaks.contains(name);
        const bool in_target = target_leaks.contains(name);
        const uint64_t baseline_size = in_baseline
            ? baseline_leaks[name].value("totalRetainedSize", uint64_t{0}) : 0;
        const uint64_t target_size = in_target
            ? target_leaks[name].value("totalRetainedSize", uint64_t{0}) : 0;
        const uint64_t baseline_count = in_baseline
            ? baseline_leaks[name].value("totalNodeCount", uint64_t{0}) : 0;
        const uint64_t target_count = in_target
            ? target_leaks[name].value("totalNodeCount", uint64_t{0}) : 0;
        const int64_t size_delta = static_cast<int64_t>(target_size - baseline_size);
        const int64_t count_delta = static_cast<int64_t>(target_count - baseline_count);

        // An entry present on both sides with fully equal totals is an equal
        // replacement (leak analysis runs on the node-ID diff, so the node ids
        // necessarily differ), not a growth.
        const std::string change = !in_baseline ? "new" : (!in_target ? "removed" :
            (size_delta == 0 && count_delta == 0 ? "replaced" :
             (size_delta >= 0 ? "growth" : "shrink")));
        const bool use_baseline = change == "removed" || change == "shrink";
        json entry = use_baseline ? baseline_leaks[name] : target_leaks[name];
        const std::string details_snapshot = use_baseline ? "A" : "B";
        const std::vector<uint32_t> baseline_node_ids = in_baseline
            ? leak_entry_node_ids(baseline_leaks[name]) : std::vector<uint32_t>{};
        const std::vector<uint32_t> target_node_ids = in_target
            ? leak_entry_node_ids(target_leaks[name]) : std::vector<uint32_t>{};
        const std::vector<uint32_t> delta_node_ids = use_baseline
            ? node_id_difference(baseline_node_ids, target_node_ids)
            : node_id_difference(target_node_ids, baseline_node_ids);
        entry["deltaNodeIds"] = delta_node_ids;
        entry["referenceChains"] = filter_leak_reference_chains(
            entry.value("referenceChains", json::array()), delta_node_ids);
        entry["retainedSizeDelta"] = size_delta;
        entry["nodeCountDelta"] = count_delta;
        entry["baselineRetainedSize"] = baseline_size;
        entry["targetRetainedSize"] = target_size;
        entry["baselineNodeCount"] = baseline_count;
        entry["targetNodeCount"] = target_count;
        entry["detailsSnapshot"] = details_snapshot;
        entry["change"] = change;
        deltas.push_back({name, std::move(entry), size_delta, change});
    }

    // Same per-direction budget as the object path: top_n positive + top_n negative.
    std::sort(deltas.begin(), deltas.end(), [](const LeakDelta& lhs, const LeakDelta& rhs) {
        const int64_t ls = std::abs(lhs.size_delta);
        const int64_t rs = std::abs(rhs.size_delta);
        if (ls != rs) return ls > rs;
        return lhs.name < rhs.name;
    });
    const auto is_positive = [](const std::string& change) {
        return change == "new" || change == "growth" || change == "replaced";
    };
    json result = json::object();
    size_t positive_count = 0;
    size_t negative_count = 0;
    for (auto& delta : deltas) {
        const bool positive = is_positive(delta.change);
        if (positive ? positive_count >= top_n : negative_count >= top_n) continue;
        if (positive) ++positive_count; else ++negative_count;
        result[delta.name] = std::move(delta.entry);
    }
    return result;
}

} // namespace

bool jsleak_compare_inputs_valid(const std::string& baseline_file,
                                 const std::string& target_file,
                                 std::string& error) {
    const std::string baseline_format = snapshot_input_format(baseline_file);
    const std::string target_format = snapshot_input_format(target_file);
    if (baseline_format.empty() || target_format.empty()) {
        error = "Snapshot comparison accepts only .heapsnapshot or .rawheap files";
        return false;
    }
    if (baseline_format != target_format) {
        error = "Baseline and target must use the same snapshot format";
        return false;
    }
    return true;
}

namespace {

// 与 build_index 的 heapSize 口径一致：get_total_heap_size() 为 0 时回退逐节点求和。
uint64_t heap_size_of(const HeapSnapshot& heap) {
    uint64_t size = heap.get_total_heap_size();
    if (size != 0) return size;
    for (int i = 0; i < heap.node_count; ++i) {
        size += heap.get_node_self_size(static_cast<uint32_t>(i));
    }
    return size;
}

json jsleak_compare_analyze_impl(const HeapSnapshot& baseline,
                                 const HeapSnapshot& target,
                                 const std::string& baseline_file,
                                 const std::string& target_file,
                                 size_t top_n,
                                 const std::string& mode) {
    // -m 选择输出维度：property-tag 跳过引用链聚合，top-chain 跳过泄漏规则。
    const bool want_chains = mode != "property-tag";
    const bool want_leaks = mode != "top-chain";

    json path_deltas = json::array();
    json changed_leak_objects = json::object();
    if (want_chains) {
        // Comparison indexes are built from the symmetric node-ID difference;
        // surviving objects never enter object/path aggregation.
        const SnapshotIndex a = build_index(baseline, target);
        const SnapshotIndex b = build_index(target, baseline);
        json complete_object_deltas = compare_aggregates_complete(a.groups, b.groups);
        LogicalGrowthResult logical = build_logical_growth_groups(
            std::move(complete_object_deltas), a, b);
        path_deltas = compact_path_deltas(select_logical_groups(logical.groups, top_n));
    }
    if (want_leaks) {
        // Leak rules run on the node-ID diff only: survivors are excluded on
        // both sides, matching the object-diff scope above.
        PropertyTagStage leak_processor("");
        const json baseline_leak_objects = leak_processor.analyze(
            baseline, [&](uint32_t node_index) {
                const uint32_t node_id = baseline.get_node_id(node_index);
                return node_id != UINT32_MAX &&
                    target.node_id_to_idx.find(node_id) == target.node_id_to_idx.end();
            });
        const json target_leak_objects = leak_processor.analyze(
            target, [&](uint32_t node_index) {
                const uint32_t node_id = target.get_node_id(node_index);
                return node_id != UINT32_MAX &&
                    baseline.node_id_to_idx.find(node_id) == baseline.node_id_to_idx.end();
            });
        changed_leak_objects = build_changed_leak_objects(
            baseline_leak_objects, target_leak_objects, top_n);
    }

    // Build icicle chart trees for both sections (same pattern as single-snapshot report):
    // each reference chain becomes a path in a prefix-trie, aggregated by frame sequence.
    json icicle_charts = json::array();
    if (want_chains && !path_deltas.empty()) {
        json chain_items = json::array();
        for (const auto& delta : path_deltas) {
            if (!delta.contains("path") || !delta["path"].is_string()) continue;
            std::string path_str = delta["path"].get<std::string>();
            if (path_str.empty()) continue;
            json frames = json::array();
            size_t start = 0;
            while (start < path_str.size()) {
                size_t sep = path_str.find(" -> ", start);
                if (sep == std::string::npos) {
                    frames.push_back(path_str.substr(start));
                    break;
                }
                frames.push_back(path_str.substr(start, sep - start));
                start = sep + 4;
            }
            json item;
            item["chain"] = std::move(frames);
            item["totalRetainedSize"] = std::abs(
                delta.value("retainedSizeDelta", static_cast<int64_t>(0)));
            item["nodeIds"] = delta.value("deltaNodeIds", json::array());
            chain_items.push_back(std::move(item));
        }
        if (!chain_items.empty()) {
            json tree = build_ref_chain_icicle_tree(chain_items, "GC Roots");
            if (tree.contains("c") && tree["c"].is_array() && !tree["c"].empty()) {
                const std::string canvas_id = "icicleCanvas_pathDeltas";
                icicle_charts.push_back({
                    {"id", canvas_id},
                    {"tree", tree},
                    {"chartHtml", build_icicle_chart_body(
                        canvas_id, tree_table_html(tree, canvas_id))}});
            }
        }
    }
    if (want_leaks && !changed_leak_objects.empty()) {
        json chain_items = json::array();
        for (const auto& [name, entry] : changed_leak_objects.items()) {
            for (const auto& chain : entry.value("referenceChains", json::array())) {
                const json& frames = chain.value("chain", json::array());
                if (!frames.is_array() || frames.empty()) continue;
                json item;
                item["chain"] = frames;
                item["totalRetainedSize"] = std::abs(
                    entry.value("retainedSizeDelta", static_cast<int64_t>(0)));
                item["nodeIds"] = chain.value("nodeIds", json::array());
                chain_items.push_back(std::move(item));
            }
        }
        if (!chain_items.empty()) {
            json tree = build_ref_chain_icicle_tree(chain_items, "GC Roots");
            if (tree.contains("c") && tree["c"].is_array() && !tree["c"].empty()) {
                const std::string canvas_id = "icicleCanvas_leakObj";
                icicle_charts.push_back({
                    {"id", canvas_id},
                    {"tree", tree},
                    {"chartHtml", build_icicle_chart_body(
                        canvas_id, tree_table_html(tree, canvas_id))}});
            }
        }
    }

    const std::string interpretation =
        mode == "property-tag" ? "difference-with-property-tag-leak-analysis"
        : mode == "top-chain"  ? "difference-with-shortest-reference-chain-analysis"
                               : "difference-with-changed-object-leak-analysis";
    return {
        {"schemaVersion", 6},
        {"reportType", "snapshot-difference"},
        {"generatedAt", format_readable_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())},
        {"duration", "未知（快照对比不采集运行时长）"},
        {"process", "未知（快照文件不含进程信息）"},
        {"mode", mode},
        {"inputFormat", snapshot_input_format(baseline_file)},
        {"analysisType", snapshot_input_format(baseline_file) + " comparison"},
        {"interpretation", interpretation},
        {"baseline", {{"role", "baseline/control"}, {"file", platform::basename(baseline_file)},
                       {"heapSize", heap_size_of(baseline)}}},
        {"target", {{"role", "target/treatment"}, {"file", platform::basename(target_file)},
                     {"heapSize", heap_size_of(target)}}},
        {"summary", {{"deltaHeapSize", static_cast<int64_t>(heap_size_of(target)) -
                                        static_cast<int64_t>(heap_size_of(baseline))}}},
        {"pathDeltas", std::move(path_deltas)},
        {"leakObjInfo", changed_leak_objects},
        {"icicleCharts", std::move(icicle_charts)},
        {"limits", {{"topNPerDirection", top_n}}},
        {"methodology", {{"objectIdentity", "snapshot node ID; IDs present in both snapshots are excluded before aggregation"},
                           {"pathMatching", "one representative shortest reference path per identity group; new/growth/replaced rows report snapshot B's path (falling back to A when B has none), removed/shrink rows report A's; weak, shortcut, and hidden edges excluded from the path search"},
                          {"retainedMetric", "summed retained size of added or removed nodes per exact extended-name object bucket"},
                          {"logicalGrouping", "dominator ancestry folds generic descendants and zero-count/zero-self carrier ancestors into the changed source-located descendant that explains their retained growth; component retained size is not added again"},
                          {"leakCandidateScope", "leak-rule matches among added and removed node IDs only; surviving node IDs are excluded"},
                           {"leakDetection", "property-tag rules only, as in the single-snapshot ArkUI/ArkTS leak report; JSLeakWatcher hash rules are not applied because comparison loads no leak list"},
                          {"presentationFiltering", "JSON and HTML contain the same top-n logical changes and top-n changed leak objects per direction; only delta node IDs are retained; the analysis mode (all/property-tag/top-chain) selects which sections are produced"}}}
    };
}

} // namespace

json jsleak_compare_analyze(const HeapSnapshot& baseline,
                            const HeapSnapshot& target,
                            const std::string& baseline_file,
                            const std::string& target_file,
                            int top_n,
                            const std::string& mode) {
    return jsleak_compare_analyze_impl(baseline, target, baseline_file, target_file,
                                       static_cast<size_t>(std::max(1, top_n)), mode);
}

} // namespace mem_analyzer
