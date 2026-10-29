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
#include "jsleak/formatters/snapshot_single_html_body.hpp"
#include "jsleak/formatters/icicle_tree.hpp"
#include "jsleak/formatters/tree_table.hpp"
#include "formatter/html/html_analysis_common.hpp"
#include "utils/file_utils.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace mem_analyzer {
using htmlAnalysis::EscapeHtml;
using htmlAnalysis::FormatBytes;
using htmlAnalysis::FormatSizeText;

namespace {

constexpr int MAX_NODE_IDS_DISPLAY = 10;
constexpr int MAX_HASH_DISPLAY = 10;

std::string BuildChainPathHtmlImpl(const json& chainFrames)
{
    // chain 现为帧名数组（根 -> 目标），相邻帧间插入箭头 span。
    if (!chainFrames.is_array()) {
        return "";
    }

    std::string result;
    bool first = true;
    for (const auto& frame : chainFrames) {
        if (!frame.is_string()) {
            continue;
        }
        if (!first) {
            result += "<span class=\"chain-arrow\">-&gt;</span>";
        }
        first = false;
        result += EscapeHtml(frame.get<std::string>());
    }
    return result;
}

namespace {

// 引用链的链尾帧名（chain 为帧名数组，根 -> 目标），用作总览图的搜索词。
std::string LastChainFrameName(const json& chainFrames)
{
    if (!chainFrames.is_array() || chainFrames.empty()) {
        return std::string();
    }
    for (auto it = chainFrames.end(); it != chainFrames.begin();) {
        --it;
        if (it->is_string()) {
            return it->get<std::string>();
        }
    }
    return std::string();
}

// 详情列表「总览图」列的查看入口：点击后切到该 tab 的「引用链总览图」并自动搜索 keyword；
// keyword 为空串时不带搜索词，直接展示完整总览图（绑定了 native 内存的 ArkTS 对象用此方式）。
// keyword 走 data- 属性而不是内联 JS 字符串，避免对象名里的引号破坏 onclick 表达式。
std::string BuildOverviewCell(const std::string& chartSubTabId, const std::string& keyword)
{
    return "<td><a class=\"detail-link\" data-chart-subtab=\"" + EscapeHtml(chartSubTabId) +
           "\" data-keyword=\"" + EscapeHtml(keyword) +
           "\" onclick=\"showInOverview(this); return false;\">查看</a></td>";
}

// 引用链前缀树节点：把大量重复前缀（a、a -> b、a -> b -> c）合并成一棵树，
// 供冰柱图按层级渲染。
//
// 数值口径与报告表格行保持一致：
//   own_size/own_count —— 以该节点结尾的那条引用链的值（即表格中该链那一行的
//                        「总大小 / 节点数」）。retained size 本身已包含其子树，
//                        因此不能把子分支再累加进来，否则会重复计算（此前会把
//                        244.22MB 的节点显示成 2.99G）。
//   size/count        —— 展示值：该节点若本身是引用链（表格有一行）就取自身值；
//                        纯前缀节点（如 GC Roots、SyntheticRoot）没有对应表格行，
//                        取各子分支之和。

struct SubTabItem {
    std::string id;
    std::string label;
    std::string body;
    bool active = false;
};

std::string BuildSubTabs(const std::vector<SubTabItem>& items)
{
    std::string html;
    html += "<div class=\"sub-tab-bar\">";
    std::string contents;
    for (const auto& item : items) {
        html += "<span class=\"sub-tab-item" + std::string(item.active ? " active" : "") +
                "\" data-subtab=\"" + EscapeHtml(item.id) + "\" onclick=\"switchSubTab('" +
                EscapeHtml(item.id) + "')\">" + EscapeHtml(item.label) + "</span>";
        contents += "<div class=\"sub-tab-content" + std::string(item.active ? " active" : "") +
                    "\" id=\"" + EscapeHtml(item.id) + "\" data-subtab=\"" + EscapeHtml(item.id) + "\">";
        contents += item.body;
        contents += "</div>";
    }
    html += "</div>";
    return html + contents;
}

} // namespace
namespace {

} // namespace

// 统一到 report_common 的 id 约定（<prefix>Header_<idx>/<prefix>Body_<idx>/<prefix>Detail_<idx>），
// 空前缀映射为 "main"（单快照只有一个列表时 prefix 为空）。
std::string NormPrefix(const std::string& prefix)
{
    return prefix.empty() ? "main" : prefix;
}

// ========== 构建节点 ID 显示 ==========

std::string BuildNodeIdsHtml(const json& nodeIdsJson, const std::string& containerId)
{
    if (!nodeIdsJson.is_array() || nodeIdsJson.empty()) {
        return "<div class=\"chain-node-ids\" id=\"" + containerId + "\" data-all-ids=\"\">节点ID: 无</div>";
    }

    std::vector<std::string> ids;
    ids.reserve(nodeIdsJson.size());
    for (const auto& id : nodeIdsJson) {
        ids.push_back(std::to_string(id.get<int64_t>()));
    }

    std::string allIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) {
            allIds += ",";
        }
        allIds += ids[i];
    }

    std::string html;
    html += "<div class=\"chain-node-ids\" id=\"" + containerId +
        "\" data-label=\"节点ID\" data-all-ids=\"" + allIds + "\">节点ID: ";

    size_t show = std::min(ids.size(), static_cast<size_t>(MAX_NODE_IDS_DISPLAY));
    for (size_t i = 0; i < show; ++i) {
        html += "<span class=\"node-id-tag\">" + ids[i] + "</span>";
    }
    if (ids.size() > MAX_NODE_IDS_DISPLAY) {
        html += "<span class=\"node-id-tag expand-btn\" id=\"expandBtn_" +
            containerId + "\" onclick=\"expandNodeIds('" + containerId + "')\">展开</span>";
    }
    html += "</div>";
    return html;
}

std::string BuildHashValuesHtml(const json &hashValuesJson, const std::string &containerId)
{
    if (!hashValuesJson.is_array() || hashValuesJson.empty()) {
        return "<div class=\"chain-node-ids\" id=\"" + containerId + "\" data-all-hashes=\"\">hash值: 无</div>";
    }

    std::vector<std::string> hashes;
    hashes.reserve(hashValuesJson.size());
    for (const auto &h : hashValuesJson) {
        hashes.push_back(std::to_string(h.get<uint32_t>()));
    }

    std::string allHashes;
    for (size_t i = 0; i < hashes.size(); ++i) {
        if (i > 0) { allHashes += ","; }
        allHashes += hashes[i];
    }

    std::string html;
    html += "<div class=\"chain-node-ids\" id=\"" + containerId +
        "\" data-all-hashes=\"" + allHashes + "\">hash值: ";
    
    size_t show = std::min(hashes.size(), static_cast<size_t>(MAX_HASH_DISPLAY));
    for (size_t i = 0; i < show; ++i) {
        html += "<span class=\"node-id-tag\">" + hashes[i] + "</span>";
    }
    if (hashes.size() > MAX_HASH_DISPLAY) {
        html += "<span class=\"node-id-tag expand-btn\" id=\"expandBtn_" +
            containerId + "\" onclick=\"expandHashValues('" + containerId + "')\">展开</span>";
    }
    html += "</div>";
    return html;
}

// ========== 表格行 ==========

std::string BuildPropertyTagTableRow(const std::string& objName,
    const json& objData,
    int idx,
    const std::string& prefix,
    const std::string& chartSubTabId)
{
    int64_t retainedSize = objData.value("totalRetainedSize", 0);
    int nodeCount = objData.value("totalNodeCount", 0);
    std::string propertyName = objData.value("propertyName", "");
    if (propertyName.empty()) { propertyName = "NA"; }
    std::string displayType = objData.value("displayType", "");
    if (displayType.empty()) { displayType = "NA"; }

    std::string hashValue = "NA";
    if (objData.contains("hashValues") && objData["hashValues"].is_array()) {
        auto &hashArr = objData["hashValues"];
        if (hashArr.size() == 1) {
            hashValue = std::to_string(hashArr[0].get<uint32_t>());
        } else if (hashArr.size() > 1) {
            hashValue = std::to_string(hashArr[0].get<uint32_t>()) + ", ......";
        }
    }

    std::string row;
    row += "<tr>";
    row += "<td><a onclick=\"markSelectedRow(this); showDetail('" + NormPrefix(prefix) + "', " +
           std::to_string(idx) + "); return false;\" class=\"detail-link\">详情</a></td>";
    // 「总览图」列以对象名称作为总览图里的搜索词
    row += BuildOverviewCell(chartSubTabId, objName);
    row += "<td class=\"top-size-val\">" + FormatBytes(retainedSize) + "</td>";
    row += "<td class=\"top-size-val\">" + std::to_string(nodeCount) + "</td>";
    row += "<td class=\"top-size-val\">" + EscapeHtml(hashValue) + "</td>";
    row += "<td class=\"top-func\">" + EscapeHtml(propertyName) + "</td>";
    row += "<td class=\"top-func\">" + EscapeHtml(displayType) + "</td>";
    row += "<td class=\"top-func\"><b>" + EscapeHtml(objName) + "</b></td>";
    row += "</tr>\n";
    return row;
}

std::string BuildTopChainTableRow(const json& chainData,
    int idx,
    const std::string& prefix,
    const std::string& chartSubTabId,
    bool searchOnOverview)
{
    int64_t retainedSize = chainData.value("totalRetainedSize", 0);
    int nodeCount = chainData.contains("nodeIds") ? static_cast<int>(chainData["nodeIds"].size()) : 0;
    std::string chainPath = BuildChainPathHtmlImpl(chainData["chain"]);

    std::string row;
    row += "<tr>";
    row += "<td><a onclick=\"markSelectedRow(this); showDetail('" + NormPrefix(prefix) + "', " +
           std::to_string(idx) + "); return false;\" class=\"detail-link\">详情</a></td>";
    // 「总览图」列以引用链链尾的帧名作为搜索词；
    // searchOnOverview 为 false 时不带搜索词（空串），点击后直接展示完整总览图
    row += BuildOverviewCell(chartSubTabId, searchOnOverview ? LastChainFrameName(chainData["chain"]) : std::string());
    row += "<td class=\"top-size-val\">" + FormatBytes(retainedSize) + "</td>";
    row += "<td class=\"top-size-val\">" + std::to_string(nodeCount) + "</td>";
    row += "<td class=\"top-func\"><b>" + chainPath + "</b></td>";
    row += "<td></td>";
    row += "</tr>\n";
    return row;
}

// ========== 详情条目（头部 + 主体） ==========

std::string BuildPropertyTagEntryHtml(const std::string& objName,
    const json& objData,
    int idx,
    const std::string& prefix)
{
    int64_t retainedSize = objData.value("totalRetainedSize", 0);
    int nodeCount = objData.value("totalNodeCount", 0);

    auto makeId = [&idx, &prefix](const std::string& type) {
        return NormPrefix(prefix) + type + "_" + std::to_string(idx);
    };
    std::string headerId = makeId("Header");
    std::string bodyId   = makeId("Body");

    std::string html;
    html += "<div class=\"top-item\" id=\"" + makeId("Detail") + "\">";
    html += "<div class=\"top-item-header expanded\" id=\"" + headerId +
            "\" onclick=\"toggleDetail('" + NormPrefix(prefix) + "', " + std::to_string(idx) + ")\">";
    html += "<span class=\"arrow\">&#9654;</span>";
    html += "<span class=\"header-rank\">泄漏对象" + std::to_string(idx + 1) +
            "：</span><span class=\"header-func\">" + EscapeHtml(objName) + "</span>";
    html += "<div class=\"header-stats\">总大小 " + FormatBytes(retainedSize) +
            "，节点数" + std::to_string(nodeCount) + "</div>";
    html += "</div>";
    html += "<div class=\"top-item-body expanded leak-detail-body\" id=\"" + bodyId + "\">";
    html += "<div class=\"call-chains-section\">";

    bool hasChain = false;
    if (objData.contains("referenceChains")) {
        // referenceChains 为数组，每项含 chain/nodeIds/totalRetainedSize
        const auto& refChains = objData["referenceChains"];
        std::vector<json> sortedChains;
        if (refChains.is_array()) {
            sortedChains.reserve(refChains.size());
            for (const auto& item : refChains) {
                sortedChains.push_back(item);
            }
        }
        std::sort(sortedChains.begin(), sortedChains.end(),
            [](const auto &a, const auto &b) {
                int64_t sizeA = a.value("totalRetainedSize", 0);
                int64_t sizeB = b.value("totalRetainedSize", 0);
                return sizeA > sizeB;
            });
        int chainIdx = 0;
        bool multipleChains = sortedChains.size() > 1;
        for (const auto &chainData : sortedChains) {
            std::string chainPath = BuildChainPathHtmlImpl(chainData["chain"]);
            int64_t chainSize = chainData.value("totalRetainedSize", 0);
            if (!chainPath.empty()) {
                std::string chainLabel = multipleChains ? ("引用链" + std::to_string(chainIdx + 1)) : "引用链";
                html += "<div class=\"call-chain-line\">" + chainLabel + ": " + chainPath + "</div>";
                hasChain = true;
            }
            html += "<div class=\"chain-detail-section\">";
            std::string containerId = "nodeIdsContainer" +
                (prefix.empty() ? "_" : ("_" + prefix + "_")) + std::to_string(idx) + "_" + std::to_string(chainIdx);
            html += BuildNodeIdsHtml(chainData["nodeIds"], containerId);
            if (multipleChains) {
                html += "<div class=\"chain-node-size\">大小: " + FormatBytes(chainSize) + "</div>";
            }
            html += "</div>";
            chainIdx++;
        }
    }
    if (!hasChain) {
        html += "<div class=\"call-chain-line\" style=\"color:#a0a0b0;\">无引用链数据</div>";
    }

    if (objData.contains("hashValues") && objData["hashValues"].is_array() && !objData["hashValues"].empty()) {
        std::string hashContainerId = "hashValuesContainer" +
            (prefix.empty() ? "_" : ("_" + prefix + "_")) + std::to_string(idx);
        html += BuildHashValuesHtml(objData["hashValues"], hashContainerId);
    }

    html += "</div></div></div>";
    return html;
}

std::string BuildTopChainEntryHtml(const json& chainData, int idx, const std::string& prefix)
{
    int64_t retainedSize = chainData.value("totalRetainedSize", 0);
    int nodeCount = chainData.contains("nodeIds") ? static_cast<int>(chainData["nodeIds"].size()) : 0;

    auto makeId = [&idx, &prefix](const std::string& type) {
        return NormPrefix(prefix) + type + "_" + std::to_string(idx);
    };
    std::string headerId = makeId("Header");
    std::string bodyId   = makeId("Body");

    std::string chainPath = BuildChainPathHtmlImpl(chainData["chain"]);

    std::string html;
    html += "<div class=\"top-item\" id=\"" + makeId("Detail") + "\">";
    html += "<div class=\"top-item-header expanded\" id=\"" + headerId +
            "\" onclick=\"toggleDetail('" + NormPrefix(prefix) + "', " + std::to_string(idx) + ")\">";
    html += "<span class=\"arrow\">&#9654;</span>";
    html += "<span class=\"header-rank\">TOP" + std::to_string(idx + 1) +
            "引用链：</span><span class=\"header-func\">" + chainPath + "</span>";
    html += "<div class=\"header-stats\">总大小 " + FormatBytes(retainedSize) +
            "，节点数" + std::to_string(nodeCount) + "</div>";
    html += "</div>";
    html += "<div class=\"top-item-body expanded\" id=\"" + bodyId + "\">";
    html += "<div class=\"call-chains-section\">";

    if (!chainPath.empty()) {
        html += "<div class=\"call-chain-line\">引用链: " + chainPath + "</div>";
    }

    std::string containerId = "nodeIdsContainer" + (prefix.empty() ? "_" : ("_" + prefix + "_")) + std::to_string(idx);
    if (chainData.contains("nodeIds")) {
        html += BuildNodeIdsHtml(chainData["nodeIds"], containerId);
    } else {
        html += "<div class=\"chain-node-ids\">节点ID: 无</div>";
    }

    html += "</div></div></div>";
    return html;
}

// ========== 构建完整 Tab 内容 ==========
std::string BuildPropertyTagTabHtmlImpl(const json& propertyTagObjects, const std::string& prefix,
    const std::string& tabId, bool active, json& icicleCharts)
{
    std::string tableRows;
    std::string detailSections;
    std::string emptyMsg;

    if (propertyTagObjects.empty()) {
        emptyMsg = "<div class=\"empty-msg\">无泄漏对象</div>";
        tableRows = "<tr><td colspan=\"8\" style=\"text-align:center;color:#888;\">无数据</td></tr>";
    } else {
        std::vector<std::pair<std::string, json>> sortedItems;
        sortedItems.reserve(propertyTagObjects.size());
        for (auto it = propertyTagObjects.begin(); it != propertyTagObjects.end(); ++it) {
            sortedItems.emplace_back(it.key(), it.value());
        }
        std::sort(sortedItems.begin(), sortedItems.end(),
            [](const auto& a, const auto& b) {
                int64_t sizeA = a.second.value("totalRetainedSize", 0);
                int64_t sizeB = b.second.value("totalRetainedSize", 0);
                return sizeA > sizeB;
            });

        int idx = 0;
        for (const auto& item : sortedItems) {
            tableRows += BuildPropertyTagTableRow(item.first, item.second, idx, prefix,
                                                  "subtabChart_" + tabId);
            detailSections += BuildPropertyTagEntryHtml(item.first, item.second, idx, prefix);
            ++idx;
        }
    }

    // 汇总所有泄漏对象的引用链，按前缀合并成一张冰柱图
    json mergedChains = json::array();
    for (auto it = propertyTagObjects.begin(); it != propertyTagObjects.end(); ++it) {
        const auto& obj = it.value();
        if (obj.contains("referenceChains") && obj["referenceChains"].is_array()) {
            for (const auto& chainItem : obj["referenceChains"]) {
                mergedChains.push_back(chainItem);
            }
        }
    }
    std::string icicleHtml;
    if (!mergedChains.empty()) {
        const std::string canvasId = "icicleCanvas_" + tabId;
        json tree = BuildRefChainIcicleTree(mergedChains, "GC Roots");
        if (tree["c"].is_array() && !tree["c"].empty()) {
            // 树表要在 move 之前生成（与图表共用同一棵树）
            const std::string treeTableHtml = TreeTableHtml(tree, canvasId);
            icicleCharts.push_back({{"id", canvasId}, {"tree", std::move(tree)}});
            icicleHtml = BuildIcicleChartBody(canvasId, treeTableHtml);
        }
    }
    const std::string chartBody =
        icicleHtml.empty() ? "<div class=\"empty-msg\">无引用链数据</div>" : icicleHtml;

    std::string detailList;
    detailList += "<table class=\"top-table\" id=\"table" + EscapeHtml(tabId) + "\">";
    // 表头行替换为带 ? 图标的版本 ==========
    detailList += "<tr><th>详情</th><th>总览图</th><th>总大小</th><th>节点数</th>"
                  "<th>hash值<span class=\"help-icon\" data-tooltip=\"JSLeakWatcher工具检测泄漏生成的泄漏对象hash值\">?</span></th>"
                  "<th>属性名称<span class=\"help-icon\" data-tooltip=\"ArkUI组件生命周期结束后，其对象上特定名称的属性会被置为TRUE，用来标记此状态。属性名和组件类型的对应关系为：\nisDeleting_对应Component类组件；_isDisposed对应NodeContainer和XComponent类组件；isDestroyed对应Ability和Window类组件。\">?</span></th>"
                  "<th>类型<span class=\"help-icon\" data-tooltip=\"检测到的有属性标签的组件或者对象类型。\n如果值为Others，说明该对象非泄漏标签方案命中的组件或者对象，可能是开发者自行添加的属性。\">?</span></th>"
                  "<th>对象名称</th></tr>";
    detailList += tableRows;
    detailList += "</table>";
    detailList += emptyMsg;
    detailList += detailSections;

    std::string content;
    content += "<div class=\"tab-content\" id=\"" + EscapeHtml(tabId) +
               "\" data-tab=\"" + EscapeHtml(tabId) + "\" style=\"display:" +
               (active ? "block" : "none") + ";\">";
    content += BuildSubTabs({{"subtabDetail_" + tabId, "详情列表", detailList, true},
                               {"subtabChart_" + tabId, "引用链总览图", chartBody, false}});
    content += "</div>";
    return content;
}

std::string BuildTopChainTabHtmlImpl(const json& topRefChains,
    const std::string& prefix,
    const std::string& tabId,
    bool active,
    json& icicleCharts,
    bool searchOnOverview)
{
    std::string tableRows;
    std::string detailSections;
    std::string emptyMsg;

    if (topRefChains.empty()) {
        emptyMsg = "<div class=\"empty-msg\">无top引用链信息</div>";
        tableRows = "<tr><td colspan=\"6\" style=\"text-align:center;color:#888;\">无数据</td></tr>";
    } else {
        // topRefChains 为数组，每项含 chain/nodeIds/totalRetainedSize
        int idx = 0;
        for (const auto& item : topRefChains) {
            tableRows += BuildTopChainTableRow(item, idx, prefix,
                                               "subtabChart_" + tabId, searchOnOverview);
            detailSections += BuildTopChainEntryHtml(item, idx, prefix);
            ++idx;
        }
    }

    // 所有引用链按前缀合并成一张冰柱图
    std::string icicleHtml;
    if (topRefChains.is_array() && !topRefChains.empty()) {
        const std::string canvasId = "icicleCanvas_" + tabId;
        json tree = BuildRefChainIcicleTree(topRefChains, "GC Roots");
        if (tree["c"].is_array() && !tree["c"].empty()) {
            // 树表要在 move 之前生成（与图表共用同一棵树）
            const std::string treeTableHtml = TreeTableHtml(tree, canvasId);
            icicleCharts.push_back({{"id", canvasId}, {"tree", std::move(tree)}});
            icicleHtml = BuildIcicleChartBody(canvasId, treeTableHtml);
        }
    }
    const std::string chartBody =
        icicleHtml.empty() ? "<div class=\"empty-msg\">无引用链数据</div>" : icicleHtml;

    std::string detailList;
    detailList += "<table class=\"top-table\" id=\"table" + EscapeHtml(tabId) + "\">";
    detailList += "<tr><th>详情</th><th>总览图</th><th>总大小</th><th>节点数</th><th>引用链名称</th><th></th></tr>";
    detailList += tableRows;
    detailList += "</table>";
    detailList += emptyMsg;
    detailList += detailSections;

    std::string content;
    content += "<div class=\"tab-content\" id=\"" + EscapeHtml(tabId) +
               "\" data-tab=\"" + EscapeHtml(tabId) + "\" style=\"display:" +
               (active ? "block" : "none") + ";\">";
    content += BuildSubTabs({{"subtabDetail_" + tabId, "详情列表", detailList, true},
                               {"subtabChart_" + tabId, "引用链总览图", chartBody, false}});
    content += "</div>";
    return content;
}

// ========== 主生成函数 ==========

} // namespace

// 公开接口：从匿名空间导出
std::string BuildChainPathHtml(const json& cf)
{
    return BuildChainPathHtmlImpl(cf);
}

std::string BuildPropertyTagTabHtml(const json& objs, const std::string& prefix,
    const std::string& tabId, bool active,
    json& icicleCharts)
{
    return BuildPropertyTagTabHtmlImpl(objs, prefix, tabId, active, icicleCharts);
}

std::string BuildTopChainTabHtml(const json& chains, const std::string& prefix,
    const std::string& tabId, bool active,
    json& icicleCharts, bool searchOnOverview)
{
    return BuildTopChainTabHtmlImpl(chains, prefix, tabId, active, icicleCharts, searchOnOverview);
}

} // namespace mem_analyzer
