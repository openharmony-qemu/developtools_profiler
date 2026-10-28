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
#include "jsleak/formatters/icicle_tree.hpp"
#include "formatter/html/html_analysis_common.hpp"

#include <memory>
#include <unordered_map>

namespace mem_analyzer {
using htmlAnalysis::EscapeHtml;

struct RefChainTrieNode {
    std::string name;
    bool hasOwn = false;    // 该节点是否为某条引用链的链尾（即表格中的一行）
    int64_t ownSize = 0;    // 以该节点结尾的引用链 retained size 之和
    int64_t ownCount = 0;   // 以该节点结尾的引用链对象数之和
    int64_t size = 0;        // 展示用大小
    int64_t count = 0;       // 展示用节点数
    std::vector<std::unique_ptr<RefChainTrieNode>> children;
    std::unordered_map<std::string, size_t> childIndex;
};

// 自底向上计算展示值。
void ComputeRefChainTotals(RefChainTrieNode& node)
{
    int64_t childrenSize = 0;
    int64_t childrenCount = 0;
    for (auto& child : node.children) {
        ComputeRefChainTotals(*child);
        childrenSize += child->size;
        childrenCount += child->count;
    }
    if (node.hasOwn) {
        // 该节点本身是一条引用链，展示表格里该行的值。
        // retained size 已包含其子树，不能再叠加子分支，否则重复计数。
        node.size = node.ownSize;
        node.count = node.ownCount;
    } else {
        // 纯前缀节点（GC Roots 等）没有对应表格行，取子分支之和。
        node.size = childrenSize;
        node.count = childrenCount;
    }
}

json RefChainTrieToJson(const RefChainTrieNode& node, int depth)
{
    int64_t childSize = 0;
    json children = json::array();
    for (const auto& child : node.children) {
        children.push_back(RefChainTrieToJson(*child, depth + 1));
        childSize += child->size;
    }
    json result;
    result["n"] = node.name;
    result["d"] = depth;  // 根节点为 -1，用于前端置灰
    result["t"] = node.size;
    // v 为自身独占大小：总大小减去所有子节点总大小
    result["v"] = node.size > childSize ? node.size - childSize : 0;
    result["cnt"] = node.count;
    result["c"] = std::move(children);
    return result;
}

// 把引用链数组按前缀合并成前缀树。chainItems 元素含 chain/nodeIds/totalRetainedSize。
json BuildRefChainIcicleTree(const json& chainItems, const std::string& rootName)
{
    RefChainTrieNode root;
    root.name = rootName;

    if (chainItems.is_array()) {
        for (const auto& item : chainItems) {
            // chain 为帧名数组（根 -> 目标），见 HeapSnapshot 的引用链产出格式
            if (!item.contains("chain") || !item["chain"].is_array() || item["chain"].empty()) {
                continue;
            }
            const json& chainFrames = item["chain"];
            const int64_t size = item.value("totalRetainedSize", static_cast<int64_t>(0));
            int64_t count = 1;
            if (item.contains("nodeIds") && item["nodeIds"].is_array()) {
                count = static_cast<int64_t>(item["nodeIds"].size());
            }
            RefChainTrieNode* cur = &root;
            for (const auto& frame : chainFrames) {
                if (!frame.is_string()) {
                    continue;
                }
                const std::string part = frame.get<std::string>();
                if (part.empty()) {
                    continue;
                }
                auto it = cur->childIndex.find(part);
                RefChainTrieNode* child = nullptr;
                if (it == cur->childIndex.end()) {
                    cur->children.push_back(std::make_unique<RefChainTrieNode>());
                    child = cur->children.back().get();
                    child->name = part;
                    cur->childIndex[part] = cur->children.size() - 1;
                } else {
                    child = cur->children[it->second].get();
                }
                cur = child;
            }
            // 只归属到链尾节点：该节点的值就是这条链在表格中那一行的值
            cur->hasOwn = true;
            cur->ownSize += size;
            cur->ownCount += count;
        }
    }

    ComputeRefChainTotals(root);
    return RefChainTrieToJson(root, -1);
}

// 引用链总览图（冰柱图）内容：工具栏（重置按钮/搜索）+ 面包屑 + 画布 + 数据树表，
// 外层由 build_sub_tabs 包成二级页签；交互能力由共享图表模块（chart_module）提供。
// 数据树表由 tree_table 组件生成（见 jsleak/formatters/tree_table.hpp）。
std::string BuildIcicleChartBody(const std::string& canvasId, const std::string& treeTableHtml)
{
    const std::string safeId = EscapeHtml(canvasId);
    std::string body;
    body += "<div class=\"graphToolbar\">";
    // 按钮文案与 htrace 报告保持一致
    body += "<button class=\"reset-btn\" id=\"reset_" + safeId + "\">Reset Zoom</button>";
    body += "<input id=\"searchInput_" + safeId + "\" type=\"text\" placeholder=\"搜索节点名...\" class=\"graph-search-input\">";
    body += "<span class=\"search-match-count\" id=\"searchCount_" + safeId + "\">0个匹配</span>";
    body += "<span class=\"shortcut-hint\">w/s：放大/缩小&emsp;&emsp;a/d：左/右移动&emsp;&emsp;点击下钻&emsp;&emsp;右键菜单</span>";
    body += "</div>";
    body += "<div class=\"icicle-breadcrumb\" id=\"crumb_" + safeId + "\"></div>";
    body += "<div class=\"icicle-canvas-wrap\"><canvas id=\"" + safeId + "\"></canvas></div>";
    body += treeTableHtml;
    return body;
}
} // namespace mem_analyzer