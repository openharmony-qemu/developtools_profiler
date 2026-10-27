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
#include "htrace/formatters/htrace_flame_json.hpp"

#include "config_mgr/htrace_config_mgr.hpp"
#include "formatter/html/html_analysis_common.hpp"
#include "htrace/flame_icicle.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace mem_analyzer::html_analysis;

namespace mem_analyzer {

// ===== Build flame/icicle tree JSON directly from FlameNode (skip intermediate JSON) =====

// Internal: recursive helper for non-root nodes (reads symbol info from FlameNode)
static json buildFlameTreeNodeFromFlameNode(const FlameNode& node, const std::unordered_map<int, std::string>& data_dict, StringTable& st) {
    std::string sym = resolve_str(node.symbol_id, data_dict);
    std::string lib = node.file_id_null ? "unknown" : resolve_str(node.file_id, data_dict);
    std::string libOffset = resolve_offset_str(node.vaddr, node.vaddr_null);

    json j;
    j["n"] = st.intern(formatFuncName(sym, lib, libOffset));
    j["t"] = node.leak_size;
    j["d"] = node.depth;
    j["cnt"] = node.existing_count;
    j["ts"] = HtraceConfigMgr::instance().is_ts_symbol(lib, node.vaddr_null);

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = buildFlameTreeNodeFromFlameNode(child, data_dict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leak_size - childrenTotal;
    j["v"] = selfSize < 0 ? (long long)0 : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

json buildFlameTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName, const std::string& rootLib, const std::string& rootLibOffset, const std::unordered_map<int, std::string>& data_dict, StringTable& st) {
    // 根节点使用TopSymbolInfo的符号信息（FlameNode根节点是合成节点，symbol_id/file_id为空）
    json j;
    j["n"] = st.intern(formatFuncName(rootName, rootLib, rootLibOffset));
    j["t"] = node.leak_size;
    j["d"] = 0;
    j["cnt"] = node.existing_count;
    j["ts"] = HtraceConfigMgr::instance().is_ts_symbol(rootLib, rootLibOffset.empty());

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = buildFlameTreeNodeFromFlameNode(child, data_dict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leak_size - childrenTotal;
    j["v"] = selfSize < 0 ? (long long)0 : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

// Internal: recursive helper for icicle non-root nodes
static json buildIcicleTreeNodeFromFlameNode(const FlameNode& node, const std::unordered_map<int, std::string>& data_dict, StringTable& st) {
    std::string sym = resolve_str(node.symbol_id, data_dict);
    std::string lib = node.file_id_null ? "unknown" : resolve_str(node.file_id, data_dict);
    std::string libOffset = resolve_offset_str(node.vaddr, node.vaddr_null);

    json j;
    j["n"] = st.intern(formatFuncName(sym, lib, libOffset));
    j["t"] = node.leak_size;
    j["d"] = node.depth;
    j["cnt"] = node.existing_count;
    j["ts"] = HtraceConfigMgr::instance().is_ts_symbol(lib, node.vaddr_null);

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = buildIcicleTreeNodeFromFlameNode(child, data_dict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leak_size - childrenTotal;
    j["v"] = selfSize < 0 ? (long long)0 : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

json buildIcicleTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName, const std::string& rootLib, const std::string& rootLibOffset, const std::unordered_map<int, std::string>& data_dict, StringTable& st) {
    // 根节点使用TopSymbolInfo的符号信息
    json j;
    j["n"] = st.intern(formatFuncName(rootName, rootLib, rootLibOffset));
    j["t"] = node.leak_size;
    j["d"] = 0;
    j["cnt"] = node.existing_count;
    j["ts"] = HtraceConfigMgr::instance().is_ts_symbol(rootLib, rootLibOffset.empty());

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = buildIcicleTreeNodeFromFlameNode(child, data_dict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leak_size - childrenTotal;
    j["v"] = selfSize < 0 ? (long long)0 : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

// Build call chains directly from FlameNode (matching TS collectSelfNodes logic)
// Flame graph uses "callers" (upward), so we traverse children in the flame tree
static void collectSelfNodesFromFlameNode(
    const FlameNode& node,
    const std::unordered_map<int, std::string>& data_dict,
    std::vector<CallChainFrame> ancestorsPath,
    std::vector<std::pair<double, std::vector<CallChainFrame>>>& results)
{
    // Compute self size: total - sum of children totals
    long long childrenTotal = 0;
    for (const auto& child : node.children) {
        childrenTotal += child.leak_size;
    }
    long long selfSize = node.leak_size - childrenTotal;
    if (selfSize < 0) selfSize = 0;

    CallChainFrame frame;
    frame.name = resolve_str(node.symbol_id, data_dict);
    frame.lib = node.file_id_null ? "unknown" : resolve_str(node.file_id, data_dict);
    frame.libOffset = resolve_offset_str(node.vaddr, node.vaddr_null);

    std::vector<CallChainFrame> pathToRoot;
    pathToRoot.push_back(frame);
    pathToRoot.insert(pathToRoot.end(), ancestorsPath.begin(), ancestorsPath.end());

    if (selfSize > 0) {
        results.push_back({(double)selfSize, pathToRoot});
    }

    for (const auto& child : node.children) {
        collectSelfNodesFromFlameNode(child, data_dict, pathToRoot, results);
    }
}

static std::vector<CallChainResult> buildCallChainsFromFlameNode(
    const FlameNode& flame_root,
    const TopSymbolInfo& ts,
    const std::unordered_map<int, std::string>& data_dict)
{
    std::vector<std::pair<double, std::vector<CallChainFrame>>> candidates;

    CallChainFrame rootFrame;
    rootFrame.name = ts.symbol;
    rootFrame.lib = ts.libs.empty() ? "" : ts.libs[0];
    rootFrame.libOffset = ts.lib_offset;
    std::vector<CallChainFrame> initialPath;
    initialPath.push_back(rootFrame);

    for (const auto& child : flame_root.children) {
        collectSelfNodesFromFlameNode(child, data_dict, initialPath, candidates);
    }

    // Fallback: if no call chains from flame graph (callers), try icicle graph (callees)
    if (candidates.empty()) {
        for (const auto& child : ts.icicle_graph.children) {
            collectSelfNodesFromFlameNode(child, data_dict, initialPath, candidates);
        }
        // Reverse frames for icicle fallback: display from innermost callee to root
        for (auto& cand : candidates) {
            std::reverse(cand.second.begin(), cand.second.end());
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<double, std::vector<CallChainFrame>>& a,
           const std::pair<double, std::vector<CallChainFrame>>& b) {
            return a.first > b.first;
        });

    int takeCount = std::min(3, (int)candidates.size());
    std::vector<CallChainResult> topPaths;
    for (int i = 0; i < takeCount; i++) {
        CallChainResult cr;
        cr.frames = candidates[i].second;
        cr.size = candidates[i].first;
        topPaths.push_back(cr);
    }
    return topPaths;
}

// 轻量版本：仅构建info+callChains，跳过flameTree/icicleTree
// 用于延迟构建模式，flameTree/icicleTree在流式写入时按需构建，大幅降低峰值内存
SymbolDataEntry buildSymbolDataEntryFromContextLight(const TopSymbolInfo& ts, long long total_existing_size, const std::unordered_map<int, std::string>& data_dict) {
    SymbolDataEntry entry;

    double percentage = total_existing_size > 0
        ? std::round((double)ts.total_leak_size / (double)total_existing_size * 10000.0) / 100.0
        : 0.0;
    std::string lib = ts.libs.empty() ? "" : ts.libs[0];
    entry.info = json{
        {"name", ts.symbol},
        {"lib", lib},
        {"libOffset", ts.lib_offset},
        {"existingSize", ts.total_leak_size},
        {"existingCount", ts.total_existing_count},
        {"percentage", percentage},
        {"type", ts.type},
        {"isApp", ts.is_app}
    };

    // rank is set by the caller (index in the group + 1)

    // 仅构建callChains（tabContentHTML需要），不构建flameTree/icicleTree
    entry.callChains = buildCallChainsFromFlameNode(ts.flame_graph, ts, data_dict);
    return entry;
}

} // namespace mem_analyzer
