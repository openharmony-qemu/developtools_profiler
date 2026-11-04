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
#include "htrace/formatters/htrace_flame_json.h"

#include "config_mgr/htrace_config_mgr.h"
#include "formatter/html/html_analysis_common.h"
#include "htrace/flame_icicle.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace mem_analyzer::htmlAnalysis;

namespace mem_analyzer {

namespace {

constexpr double PERCENT_PRECISION_SCALE = 10000.0;
constexpr int PERCENT_SCALE = 100;

} // namespace

// ===== Build flame/icicle tree JSON directly from FlameNode (skip intermediate JSON) =====

// Internal: recursive helper for non-root nodes (reads symbol info from FlameNode)
static json BuildFlameTreeNodeFromFlameNode(const FlameNode& node,
    const std::unordered_map<int, std::string>& dataDict, StringTable& st)
{
    std::string sym = ResolveStr(node.symbolId, dataDict);
    std::string lib = node.fileIdNull ? "unknown" : ResolveStr(node.fileId, dataDict);
    std::string libOffset = ResolveOffsetStr(node.vaddr, node.vaddrNull);

    json j;
    j["n"] = st.Intern(FormatFuncName(sym, lib, libOffset));
    j["t"] = node.leakSize;
    j["d"] = node.depth;
    j["cnt"] = node.existingCount;
    j["ts"] = HtraceConfigMgr::Instance().IsTsSymbol(lib, node.vaddrNull);

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = BuildFlameTreeNodeFromFlameNode(child, dataDict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leakSize - childrenTotal;
    j["v"] = selfSize < 0 ? static_cast<long long>(0) : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

json BuildFlameTreeJsonFromFlameNode(const FlameNode& node,
    const RootSymbolInfo& rootSym,
    const std::unordered_map<int, std::string>& dataDict, StringTable& st)
{
    // 根节点使用TopSymbolInfo的符号信息（FlameNode根节点是合成节点，symbol_id/file_id为空）
    json j;
    j["n"] = st.Intern(FormatFuncName(rootSym.name, rootSym.lib, rootSym.libOffset));
    j["t"] = node.leakSize;
    j["d"] = 0;
    j["cnt"] = node.existingCount;
    j["ts"] = HtraceConfigMgr::Instance().IsTsSymbol(rootSym.lib, rootSym.libOffset.empty());

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = BuildFlameTreeNodeFromFlameNode(child, dataDict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leakSize - childrenTotal;
    j["v"] = selfSize < 0 ? static_cast<long long>(0) : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

// Internal: recursive helper for icicle non-root nodes
static json BuildIcicleTreeNodeFromFlameNode(const FlameNode& node,
    const std::unordered_map<int, std::string>& dataDict, StringTable& st)
{
    std::string sym = ResolveStr(node.symbolId, dataDict);
    std::string lib = node.fileIdNull ? "unknown" : ResolveStr(node.fileId, dataDict);
    std::string libOffset = ResolveOffsetStr(node.vaddr, node.vaddrNull);

    json j;
    j["n"] = st.Intern(FormatFuncName(sym, lib, libOffset));
    j["t"] = node.leakSize;
    j["d"] = node.depth;
    j["cnt"] = node.existingCount;
    j["ts"] = HtraceConfigMgr::Instance().IsTsSymbol(lib, node.vaddrNull);

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = BuildIcicleTreeNodeFromFlameNode(child, dataDict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leakSize - childrenTotal;
    j["v"] = selfSize < 0 ? static_cast<long long>(0) : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

json BuildIcicleTreeJsonFromFlameNode(const FlameNode& node,
    const RootSymbolInfo& rootSym,
    const std::unordered_map<int, std::string>& dataDict, StringTable& st)
{
    // 根节点使用TopSymbolInfo的符号信息
    json j;
    j["n"] = st.Intern(FormatFuncName(rootSym.name, rootSym.lib, rootSym.libOffset));
    j["t"] = node.leakSize;
    j["d"] = 0;
    j["cnt"] = node.existingCount;
    j["ts"] = HtraceConfigMgr::Instance().IsTsSymbol(rootSym.lib, rootSym.libOffset.empty());

    long long childrenTotal = 0;
    json childrenArr = json::array();
    for (const auto& child : node.children) {
        json childJson = BuildIcicleTreeNodeFromFlameNode(child, dataDict, st);
        childrenTotal += childJson["t"].get<long long>();
        childrenArr.push_back(std::move(childJson));
    }

    long long selfSize = node.leakSize - childrenTotal;
    j["v"] = selfSize < 0 ? static_cast<long long>(0) : selfSize;
    j["c"] = std::move(childrenArr);
    return j;
}

// Build call chains directly from FlameNode (matching TS collectSelfNodes logic)
// Flame graph uses "callers" (upward), so we traverse children in the flame tree
static void CollectSelfNodesFromFlameNode(
    const FlameNode& node,
    const std::unordered_map<int, std::string>& dataDict,
    std::vector<CallChainFrame> ancestorsPath,
    std::vector<std::pair<double, std::vector<CallChainFrame>>>& results)
{
    // Compute self size: total - sum of children totals
    long long childrenTotal = 0;
    for (const auto& child : node.children) {
        childrenTotal += child.leakSize;
    }
    long long selfSize = node.leakSize - childrenTotal;
    if (selfSize < 0) {
        selfSize = 0;
    }

    CallChainFrame frame;
    frame.name = ResolveStr(node.symbolId, dataDict);
    frame.lib = node.fileIdNull ? "unknown" : ResolveStr(node.fileId, dataDict);
    frame.libOffset = ResolveOffsetStr(node.vaddr, node.vaddrNull);

    std::vector<CallChainFrame> pathToRoot;
    pathToRoot.push_back(frame);
    pathToRoot.insert(pathToRoot.end(), ancestorsPath.begin(), ancestorsPath.end());

    if (selfSize > 0) {
        results.push_back({static_cast<double>(selfSize), pathToRoot});
    }

    for (const auto& child : node.children) {
        CollectSelfNodesFromFlameNode(child, dataDict, pathToRoot, results);
    }
}

static std::vector<CallChainResult> buildCallChainsFromFlameNode(
    const FlameNode& flameRoot,
    const TopSymbolInfo& ts,
    const std::unordered_map<int, std::string>& dataDict)
{
    std::vector<std::pair<double, std::vector<CallChainFrame>>> candidates;

    CallChainFrame rootFrame;
    rootFrame.name = ts.symbol;
    rootFrame.lib = ts.libs.empty() ? "" : ts.libs[0];
    rootFrame.libOffset = ts.libOffset;
    std::vector<CallChainFrame> initialPath;
    initialPath.push_back(rootFrame);

    for (const auto& child : flameRoot.children) {
        CollectSelfNodesFromFlameNode(child, dataDict, initialPath, candidates);
    }

    // Fallback: if no call chains from flame graph (callers), try icicle graph (callees)
    if (candidates.empty()) {
        for (const auto& child : ts.icicleGraph.children) {
            CollectSelfNodesFromFlameNode(child, dataDict, initialPath, candidates);
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

    int takeCount = std::min(3, static_cast<int>(candidates.size()));
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
SymbolDataEntry BuildSymbolDataEntryFromContextLight(const TopSymbolInfo& ts,
    long long totalExistingSize, const std::unordered_map<int, std::string>& dataDict)
{
    SymbolDataEntry entry;

    double percentage = totalExistingSize > 0 ?
        std::round(static_cast<double>(ts.totalLeakSize) /
            static_cast<double>(totalExistingSize) *
            PERCENT_PRECISION_SCALE) /
            static_cast<double>(PERCENT_SCALE) :
        0.0;
    std::string lib = ts.libs.empty() ? "" : ts.libs[0];
    entry.info = json{
        {"name", ts.symbol},
        {"lib", lib},
        {"libOffset", ts.libOffset},
        {"existingSize", ts.totalLeakSize},
        {"existingCount", ts.totalExistingCount},
        {"percentage", percentage},
        {"type", ts.type},
        {"isApp", ts.isApp}
    };

    // rank is set by the caller (index in the group + 1)

    // 仅构建callChains（tabContentHTML需要），不构建flameTree/icicleTree
    entry.callChains = buildCallChainsFromFlameNode(ts.flameGraph, ts, dataDict);
    return entry;
}

} // namespace mem_analyzer
