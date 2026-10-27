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
#include "formatter/html/analysis_html_formatter_base.hpp"
#include "formatter/html/components/navigation_bar.hpp"
#include "formatter/html/components/chart_module.hpp"
#include "formatter/html/components/version.hpp"

using namespace mem_analyzer::html_analysis;

namespace mem_analyzer {

namespace {
// 按 __DATA_JSON__ 切割模板时，必须精确跳过占位符本身的长度。
// 多跳 1 个字符会吞掉模板中紧跟占位符之后的 ';'，使生成的前端代码变成
// `var allData = [...](function(){...})()`——数组被当作函数调用而抛 TypeError，
// 后续图表引擎整体不执行。
constexpr size_t kDataJsonPlaceholderLen = sizeof("__DATA_JSON__") - 1;
}  // namespace

std::string AnalysisHtmlFormatterBase::buildTabBarHTML(const std::vector<TabGroup>& groups) {
    std::vector<std::string> tabItems;
    bool firstActiveSet = false;

    for (const auto& group : groups) {
        if (group.symbols.empty()) continue;
        std::string cls = firstActiveSet ? "" : " active";
        firstActiveSet = true;
        tabItems.push_back(
            "<div class=\"tab-item" + cls + "\" data-tab=\"" + group.tabId
            + "\" onclick=\"switchTab('" + group.tabId + "', this)\">"
            + group.tabLabel + "</div>");
    }

    if (!tabItems.empty()) {
        std::string tabBarHTML = "<div class=\"tab-bar\">";
        for (size_t i = 0; i < tabItems.size(); i++) {
            if (i > 0) tabBarHTML += "\n";
            tabBarHTML += tabItems[i];
        }
        tabBarHTML += "</div>";
        return tabBarHTML;
    }
    return "<div class=\"empty-msg\">无分析数据</div>";
}

std::string AnalysisHtmlFormatterBase::buildTabContentHTML(const std::vector<TabGroup>& groups, long long totalSize, bool countMode) {
    std::string result;
    for (const auto& group : groups) {
        if (group.symbols.empty()) continue;

        std::string tableRows = buildTopTableRows(group.symbols, group.startIndex, countMode);
        std::string detailSections = buildDetailSections(group.symbols, group.startIndex, totalSize, countMode);

        result += "<div class=\"tab-content\" id=\"" + group.tabId + "\">\n"
            "  <table class=\"top-table\">\n"
            "    <tr><th>详情</th>" + std::string(countMode ? "" : "<th>大小</th>") + "<th>次数</th><th>占比</th><th>TOP函数</th><th>完整路径</th></tr>\n"
            "    " + tableRows + "\n"
            "  </table>\n"
            "  " + detailSections + "\n"
            "</div>";
    }
    return result;
}


// 公共占位符统一替换：两条注入路径（TabGroup 流式 / 通用流式）× 两段片段（before/after）
// 原先各写一遍，新增占位符极易漏改其中一处，这里收敛为唯一入口。
// 各占位符只出现在模板的固定片段（如 __CHART_MODULE_CSS__ 只在头部、__CHART_MODULE_JS__
// 只在尾部），对不含该占位符的片段替换是空操作，故可对任意片段统一调用。
void AnalysisHtmlFormatterBase::applyCommonPlaceholders(std::string& htmlFrag, long long totalSize, bool countMode) {
    // 共享图表模块（chart_module）
    htmlFrag = replace_all(htmlFrag, "__CHART_MODULE_CSS__", chart_module_css());
    htmlFrag = replace_all(htmlFrag, "__CHART_MODULE_JS__", chart_module_js());
    // 全局开关
    htmlFrag = replace_all(htmlFrag, "__TOTAL_SIZE__", std::to_string(totalSize));
    htmlFrag = replace_all(htmlFrag, "__UNIT_COUNT_MODE__", countMode ? "true" : "false");
    htmlFrag = replace_all(htmlFrag, "__AUTO_EXPAND__", "true");
    // 导航栏（navigation_bar）
    htmlFrag = replace_all(htmlFrag, "__NAV_BAR_FORMAT__", nav_bar_format());
    htmlFrag = replace_all(htmlFrag, "__NAV_BAR_TITLE__", nav_bar_title());
    htmlFrag = replace_all(htmlFrag, "__NAV_BAR_DOC_CONTENT__", nav_bar_doc_content());
    htmlFrag = replace_all(htmlFrag, "__NAV_BAR_HELP_CENTER_CONTENT__", nav_bar_help_center_content());
    htmlFrag = replace_all(htmlFrag, "__NAV_BAR_SWITCH_NAV_FUNC__", nav_bar_switch_nav_func());
    // 页脚版本
    htmlFrag = replace_all(htmlFrag, "__VERSION_FORMAT__", setVersionFormat());
    htmlFrag = replace_all(htmlFrag, "__VERSION_CONTENT__", buildVersionContent());
}

// Streaming version: outputs each SymbolDataEntry individually to avoid building full dataJSONArr
// fillTree callback: lazily builds flameTree/icicleTree during streaming, avoiding all entries in memory at once
void AnalysisHtmlFormatterBase::fillHTMLTemplateStreamedDirect(
    std::ostream& out,
    std::vector<TabGroup>& groups,
    long long totalSize,
    const std::string& tabBarHTML,
    const std::string& tabContentHTML,
    const std::string& metaDataItemsHTML,
    const std::string& title,
    FillTreeCallback fillTree,
    bool countMode)
{
    std::string html = HTML_TEMPLATE;

    // Split template at __DATA_JSON__
    size_t jsonPos = html.find("__DATA_JSON__");
    std::string beforeJson = html.substr(0, jsonPos);
    std::string afterJson = html.substr(jsonPos + kDataJsonPlaceholderLen);

    // Replace placeholders in before part
    beforeJson = replace_all(beforeJson, "__DATA_JSON__", "");
    beforeJson = replace_all(beforeJson, "__TAB_BAR_HTML__", tabBarHTML);
    beforeJson = replace_all(beforeJson, "__TAB_CONTENT_HTML__", tabContentHTML);
    beforeJson = replace_all(beforeJson, "__TITLE__", title);
    beforeJson = replace_all(beforeJson, "__META_ITEMS_HTML__", metaDataItemsHTML);
    applyCommonPlaceholders(beforeJson, totalSize, countMode);
    applyCommonPlaceholders(afterJson, totalSize, countMode);

    // Stream all data inline
    out << beforeJson;
    out << "[";
    bool firstEntry = true;
    size_t groupIdx = 0;
    for (auto& group : groups) {
        size_t entryIdx = 0;
        for (auto& entry : group.symbols) {
            if (!firstEntry) out << ",";
            firstEntry = false;
            if (fillTree) {
                fillTree(entry, groupIdx, entryIdx);
            }
            json entryJson = buildSymbolDataEntryJSON(entry);
            out << entryJson;
            entryIdx++;
        }
        groupIdx++;
    }
    out << "]";
    out << afterJson;
}

// 通用版：直接流式输出JSON数据数组（适用于tshandle等非TabGroup模型）
void AnalysisHtmlFormatterBase::fillHTMLTemplateStreamed(
    std::ostream& out,
    const json& dataJSONArr,
    long long totalSize,
    const std::string& tabBarHTML,
    const std::string& tabContentHTML,
    const std::string& metaDataItemsHTML,
    const std::string& title,
    bool countMode)
{
    std::string html = HTML_TEMPLATE;
    size_t jsonPos = html.find("__DATA_JSON__");
    std::string beforeJson = html.substr(0, jsonPos);
    std::string afterJson = html.substr(jsonPos + kDataJsonPlaceholderLen);

    beforeJson = replace_all(beforeJson, "__TAB_BAR_HTML__", tabBarHTML);
    beforeJson = replace_all(beforeJson, "__TAB_CONTENT_HTML__", tabContentHTML);
    beforeJson = replace_all(beforeJson, "__TITLE__", title);
    beforeJson = replace_all(beforeJson, "__META_ITEMS_HTML__", metaDataItemsHTML);
    applyCommonPlaceholders(beforeJson, totalSize, countMode);
    applyCommonPlaceholders(afterJson, totalSize, countMode);

    out << beforeJson;
    out << dataJSONArr;
    out << afterJson;
}

// 设置格式（转发到 version 组件）
std::string AnalysisHtmlFormatterBase::setVersionFormat() {
    return version_format();
}

// 构建内容（转发到 version 组件）
std::string AnalysisHtmlFormatterBase::buildVersionContent() {
    return version_content();
}

} // namespace mem_analyzer
