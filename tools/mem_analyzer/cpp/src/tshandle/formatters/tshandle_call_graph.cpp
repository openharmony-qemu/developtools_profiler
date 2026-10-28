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
#include "tshandle/formatters/tshandle_call_graph.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>

namespace mem_analyzer {
namespace {

std::string key(const ResolvedStackFrame& f)
{
    return f.symbol + '\0' + f.lib + '\0' + f.libOffset;
}

TshandleRenderNode MakeNode(const ResolvedStackFrame& f, int depth)
{
    const auto location = !f.lib.empty() ? (!f.libOffset.empty() ? f.lib + "+" + f.libOffset : f.lib) : "";
    const bool unresolved = f.symbol.empty() || f.symbol == location || f.symbol.find(".so") != std::string::npos;
    const auto slash = f.lib.find_last_of("/\\");
    const auto libname = slash == std::string::npos ? f.lib : f.lib.substr(slash + 1);
    const auto display = unresolved ? libname + (f.libOffset.empty() ? "" : "+" + f.libOffset) : f.symbol;
    const auto name = location.empty() ? (f.symbol.empty() ? "unknown" : f.symbol) : (unresolved ? location : location + " (" + f.symbol + ")");
    const bool isTs = HtraceConfigMgr::instance().IsTsSymbol(f.lib, f.libOffset.empty());
    return {name, display, f.symbol, f.lib, f.libOffset, depth, 0, 0, 0, {}, isTs, f.buildId};
}

void ComputeValue(TshandleRenderNode& node)
{
    double childTotal = 0;
    for (auto& child : node.children) {
        ComputeValue(child);
        childTotal += child.total;
    }
    node.value = node.total > childTotal ? node.total - childTotal : 0;
}

} // namespace

json TshandleRenderNode::ToJson() const
{
    json result = {
        {"n", name},
        {"displayName", displayName},
        {"symbol", symbol},
        {"lib", lib},
        {"libOffset", libOffset},
        {"d", depth},
        {"v", value},
        {"t", total},
        {"cnt", count},
        {"ts", ts}
    };
    result["c"] = json::array();
    for (const auto& child : children) {
        result["c"].push_back(child.ToJson());
    }
    return result;
}

TshandleRenderNode BuildTshandleDownstreamGraph(const std::string& objectName,
    const std::vector<TshandleCallChain>& chains,
    double objectExistingSize)
{
    TshandleRenderNode root{objectName, objectName, objectName, "", "", -1, 0, 0, 0, {}};
    for (const auto& chain : chains) {
        auto frames = chain.frames;
        std::stable_sort(frames.begin(), frames.end(), [](const ResolvedStackFrame& a, const ResolvedStackFrame& b)
        {
            return a.depth < b.depth;
        });

        const double size = chain.existingSize;
        const long long count = chain.count;
        root.count += count;
        auto* current = &root;

        for (std::size_t i = 0; i < frames.size(); ++i) {
            const auto id = key(frames[i]);
            auto child = std::find_if(current->children.begin(), current->children.end(), [&](const TshandleRenderNode& n)
            {
                return n.symbol == frames[i].symbol && n.lib == frames[i].lib && n.libOffset == frames[i].libOffset;
            });

            if (child == current->children.end()) {
                current->children.push_back(MakeNode(frames[i], static_cast<int>(i)));
                child = std::prev(current->children.end());
            }

            child->total += size;
            child->count += count;
            current = &*child;
        }
    }

    root.total = objectExistingSize;
    ComputeValue(root);
    return root;
}

} // namespace mem_analyzer
