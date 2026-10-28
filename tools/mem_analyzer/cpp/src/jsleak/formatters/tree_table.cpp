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
#include "jsleak/formatters/tree_table.hpp"

#include "formatter/html/html_analysis_common.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace mem_analyzer {

using htmlAnalysis::EscapeHtml;
using htmlAnalysis::FormatBytes;

namespace {

// 百分比口径与图表 tooltip 一致：以整棵树的根节点（GC Roots）总大小/总节点数为分母。
std::string FormatTreePct(int64_t part, int64_t total)
{
    if (total <= 0) {
        return "0%";
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f%%", static_cast<double>(part) / total * 100.0);
    return buf;
}

// 树表默认展开阈值（百分比，相对根节点总大小）：占比高于它的节点默认展开，
// 使主干分支一进来就可见；小于等于它的分支收起，避免一开始就铺开上百行。
constexpr double kIttDefaultExpandPct = 15.0;

// 默认展开的最大层数（根节点为第 0 层）：占比再高的分支也不会自动展开到更深，
// 避免链特别深时一开始就铺开过多行。达到该深度的节点一律收起，仍可手动展开。
constexpr int kIttMaxExpandDepth = 8;

// 层级配色表：每一层一个颜色。竖引导线取「该线所代表的层」的颜色，节点 +/- 取
// 「该节点所在层」的颜色，二者同色，因此顺着一条线的颜色就能找到它属于哪一层。
// 层数超过配色表长度时循环取用。
const std::vector<std::string>& ittLevelColors()
{
    static const std::vector<std::string> colors = {
        "#4e79a7", "#59a14f", "#e15759", "#f28e2b",
        "#b07aa1", "#76b7b2", "#8c6d31", "#d37295",
    };
    return colors;
}

std::string IttLevelColor(int level)
{
    const std::vector<std::string>& colors = ittLevelColors();
    const size_t idx = static_cast<size_t>(level < 0 ? 0 : level) % colors.size();
    return colors[idx];
}

// 子树内的最大节点值（含自身）。
// 判定默认展开必须看子树而非自身：引用链 trie 按名字前缀合并，父节点自身值可能远小于其子链
// （父级占比不到阈值、子孙却超过阈值）。只看自身会让父级被收起，把超过阈值的子孙链一并隐藏。
int64_t IttSubtreeMaxSize(const json& node)
{
    int64_t maxSize = node.value("t", static_cast<int64_t>(0));
    if (node.contains("c") && node["c"].is_array()) {
        for (const json& child : node["c"]) {
            const int64_t childMax = IttSubtreeMaxSize(child);
            if (childMax > maxSize) {
                maxSize = childMax;
            }
        }
    }
    return maxSize;
}

// 递归输出树表行（前序）。path 为该行在排序后兄弟序列中的路径，用于生成行 id 与父子关系；
// anc 为所有祖先行 id 的逗号列表，前端据此判断该行是否应随某个折叠的祖先一起隐藏；
// anc_collapsed 表示是否存在处于折叠态的祖先（决定本行初始是否隐藏）。
void EmitIcicleTreeRows(const json& node, const std::string& tableId,
    const std::string& path, int depth, const std::string& anc,
    bool ancCollapsed,
    int64_t rootSize, std::string& out)
{
    const std::string rowId = tableId + "_r" + (path.empty() ? "root" : path);
    const std::string name = node.value("n", std::string());
    const int64_t size = node.value("t", static_cast<int64_t>(0));
    const int64_t count = node.value("cnt", static_cast<int64_t>(0));
    const bool hasChildren = node.contains("c") && node["c"].is_array() && !node["c"].empty();
    // 默认：占比超过阈值的节点展开（根节点占比恒为 100%，自然也展开），其余有子节点的层级收起；
    // 同时限制默认展开的最大层数，超过 kIttMaxExpandDepth 的节点即使占比达标也收起。
    // 占比按「子树内最大占比」判定，使超过阈值的子孙链不会被未达阈值的父级折叠隐藏。
    const double pct = rootSize > 0 ?
                           100.0 * static_cast<double>(IttSubtreeMaxSize(node)) /
                                 static_cast<double>(rootSize) :
                           0.0;
    const bool collapsed =
        hasChildren && depth >= 1 && (pct <= kIttDefaultExpandPct || depth >= kIttMaxExpandDepth);

    // 折叠态的唯一来源是 itt-collapsed 类，初始渲染就必须打上：
    // 否则首次点击时 classList.toggle 反而会「加上」该类，表现为点了仍是 "+"，
    // 且重新计算显隐时中间层没有折叠标记，会把整棵子树一次性放出来。
    out += "<tr id=\"" + EscapeHtml(rowId) + "\" data-anc=\"" + EscapeHtml(anc) + "\"";
    if (collapsed) {
        out += " class=\"itt-collapsed\"";
    }
    if (ancCollapsed) {
        out += " style=\"display:none\"";
    }
    out += ">";
    // 列顺序：大小 / 占比 / 节点数 / 节点名称（名称放最后，见 tree_table_html）
    out += "<td class=\"itt-num\">" + FormatBytes(static_cast<double>(size)) + "</td>";
    out += "<td class=\"itt-num itt-pct\">" + FormatTreePct(size, rootSize) + "</td>";
    out += "<td class=\"itt-num\">" + std::to_string(count) + "</td>";
    // 名称列允许换行，超长节点名完整展示（因此不再需要 title / 省略号）
    out += "<td class=\"itt-name" + std::string(depth == 0 ? " itt-root" : "") +
           "\" style=\"padding-left:" + std::to_string(10 + depth * 16) + "px\">";
    // 垂直引导线：每个祖先层级一条，位置对准该层展开/折叠开关的中心（18 + 16k），
    // 把同一祖先下的各级子节点串起来，避免层级多时看串行。
    // 颜色取该线所代表的层的颜色，与本层节点的 +/- 同色。
    // 绝对定位铺满整行高度，名称换行导致行变高时线也不会断开。
    for (int k = 0; k < depth; ++k) {
        out += "<span class=\"itt-guide\" style=\"left:" + std::to_string(18 + k * 16) +
               "px;background:" + IttLevelColor(k) + "\"></span>";
    }
    if (hasChildren) {
        // 颜色通过 CSS 变量传入，同时作用于 +/- 字形与方框边框
        out += "<span class=\"itt-toggle\" style=\"--itt-c:" + IttLevelColor(depth) +
               "\" onclick=\"toggleIcicleTreeRow(this)\">" +
               std::string(collapsed ? "+" : "-") + "</span>";
    } else {
        // 叶子节点没有展开开关，放一个同层颜色的小圆点占位：
        // 避免该列空白导致与上方带方框的 +/- 对不齐，且保持层级配色连续。
        out += "<span class=\"itt-leaf\" style=\"--itt-c:" + IttLevelColor(depth) + "\">•</span>";
    }
    out += "<span class=\"itt-name-text\">" + EscapeHtml(name) + "</span></td>";
    out += "</tr>";

    if (hasChildren) {
        const json& children = node["c"];
        // 树表内按总大小降序（与 Analyzer 视图一致）；仅影响表格顺序，不改动图表的树
        std::vector<size_t> order;
        order.reserve(children.size());
        for (size_t i = 0; i < children.size(); ++i) {
            order.push_back(i);
        }
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b)
        {
            const int64_t ta = children[a].value("t", static_cast<int64_t>(0));
            const int64_t tb = children[b].value("t", static_cast<int64_t>(0));
            if (ta != tb) {
                return ta > tb;
            }
            return children[a].value("n", std::string()) < children[b].value("n", std::string());
        });
        const std::string childAnc = anc.empty() ? rowId : anc + "," + rowId;
        for (size_t k = 0; k < order.size(); ++k) {
            const std::string childPath =
                path.empty() ? std::to_string(k) : path + "-" + std::to_string(k);
            EmitIcicleTreeRows(children[order[k]], tableId, childPath, depth + 1, childAnc,
                ancCollapsed || collapsed, rootSize, out);
        }
    }
}

} // namespace

std::string TreeTableCss()
{
    return R"CSS(
/* ========== 引用链总览图下方的数据树表（参考 Analyzer 的 Analysis 列表） ==========
   table-layout:fixed + colgroup：名称列自适应拿走剩余宽度，后三列固定像素宽。
   名称列允许换行，超长节点名完整展示（不截断）。 */
.itt-actions { display: flex; justify-content: flex-end; gap: 8px; margin: 14px 0 8px; }
.itt-btn {
    padding: 4px 12px; font-size: 13px; color: #333; background: #f5f5f5;
    border: 1px solid #ccc; border-radius: 4px; cursor: pointer;
}
.itt-btn:hover { background: #e8eaed; border-color: #999; }
/* 高度：clamp(下限, 视口比例, 上限)。默认展开主干后可见行较多，表格要占满大部分窗口高度；
   下限保证窗口偏矮（如 IDE 内置预览）时也有足够展示区域，上限避免超宽屏下无限拉高。 */
.icicle-tree-wrap { margin: 0 0 20px; max-height: clamp(800px, 88vh, 1600px); overflow: auto; border: 1px solid #dee2e6; border-radius: 6px; }
/* 注意：sticky 表头要求 border-collapse: separate（collapse 下浏览器会把 th 的 sticky 降级为 static），
   用 border-spacing:0 + 单元格底部边框模拟 collapse 的视觉效果。 */
.icicle-tree-table { width: 100%; border-collapse: separate; border-spacing: 0; table-layout: fixed; font-size: 14px; }
.icicle-tree-table th {
    position: sticky; top: 0; z-index: 1;
    background: #f0f0f0; color: #333; font-size: 14px; font-weight: 600;
    padding: 8px 10px; text-align: left; border-bottom: 1px solid #dee2e6; white-space: nowrap;
}
.icicle-tree-table td { padding: 6px 10px; border-bottom: 1px solid #f0f0f0; font-size: 14px; }
.icicle-tree-table tbody tr:hover { background: #f5f8ff; }
/* 数值列（前三列：大小/占比/节点数）右对齐、等宽数字，方便纵向比较；
   内边距收窄，避免数值列占用过多宽度。名称列在最后（左对齐，不参与此规则）。 */
.icicle-tree-table th:nth-child(-n+3), .icicle-tree-table td:nth-child(-n+3) { text-align: right; }
.icicle-tree-table td:nth-child(-n+3) { padding: 6px 8px; }
.itt-num { white-space: nowrap; color: #333; font-variant-numeric: tabular-nums; }
.itt-pct { color: #888; }
/* 名称列：换行完整展示；长标识符（无空格）也可在任意位置断行。
   position:relative 作为垂直引导线的定位参照。 */
.itt-name { position: relative; white-space: normal; overflow-wrap: anywhere; }
/* 垂直引导线：串联同一祖先下的各级子节点。绝对定位铺满整行高度（bottom:-1px 覆盖行间
   分隔线，保证跨行连续），left 与颜色均由行内样式按层级给出（颜色见 itt_level_colors）。 */
.itt-guide { position: absolute; top: 0; bottom: -1px; width: 1px; }
.itt-name.itt-root { font-weight: 600; }
.itt-name-text { color: #333; }
.itt-toggle {
    display: inline-block; width: 16px; height: 16px; margin-right: 6px;
    line-height: 14px; text-align: center; vertical-align: middle;
    /* --itt-c 由行内样式按层级给出（见 itt_level_colors）：字形与方框取同一颜色。
       这里用 CSS 变量而不是行内 color/border-color，是因为行内样式优先级高于任何选择器，
       会盖掉下面的 :hover 规则，悬停反馈就失效了。 */
    color: var(--itt-c, #555); border: 1px solid var(--itt-c, #bbb); border-radius: 3px;
    font-size: 12px; cursor: pointer; user-select: none;
}
/* 悬停只压暗（保留层级色相）并换底色，避免方框变回灰色而丢掉层级对应关系 */
.itt-toggle:hover { background: #e8eaed; filter: brightness(0.85); }
/* 叶子节点占位：与 .itt-toggle 同尺寸（16px + 6px 右边距，见上方 box-sizing:border-box）、
   同样带层级色方框，内含一个同色小圆点表示「到此为止」，保持整列竖向对齐与层级配色连续。
   无 cursor / 无 hover，避免被误认为可点击的展开开关。颜色由行内 --itt-c 给出。 */
.itt-leaf {
    display: inline-block; width: 16px; height: 16px; margin-right: 6px;
    line-height: 14px; text-align: center; vertical-align: middle;
    color: var(--itt-c, #bbb); border: 1px solid var(--itt-c, #bbb); border-radius: 3px;
    font-size: 12px; user-select: none;
}
)CSS";
}

std::string TreeTableHtml(const json& tree, const std::string& canvasId)
{
    const std::string tableId = "treeTable_" + canvasId;
    std::string rows;
    EmitIcicleTreeRows(tree, tableId, "", 0, "", false,
        tree.value("t", static_cast<int64_t>(0)), rows);
    std::string html;
    html += "<div class=\"itt-actions\">";
    html += "<button class=\"itt-btn\" onclick=\"toggleIcicleTreeAll('" + EscapeHtml(tableId) +
            "', true)\">全部展开</button>";
    html += "<button class=\"itt-btn\" onclick=\"toggleIcicleTreeAll('" + EscapeHtml(tableId) +
            "', false)\">全部折叠</button>";
    html += "<button class=\"itt-btn\" onclick=\"copyIcicleTree('" + EscapeHtml(tableId) +
            "', this)\">复制引用链</button>";
    html += "</div>";
    html += "<div class=\"icicle-tree-wrap\">";
    html += "<table class=\"icicle-tree-table\" id=\"" + EscapeHtml(tableId) + "\">";
    html += "<colgroup><col style=\"width:96px\"><col style=\"width:76px\">"
            "<col style=\"width:80px\"><col></colgroup>";
    html += "<thead><tr><th>大小</th><th>占比</th><th>节点数</th>"
            "<th>节点名称</th></tr></thead>";
    html += "<tbody>" + rows + "</tbody>";
    html += "</table></div>";
    return html;
}

} // namespace mem_analyzer
