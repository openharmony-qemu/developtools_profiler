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
#pragma once

// analysis_html_formatter_base.h — HTML分析报告公共工具
// 提供所有分析类型（htrace/tshandle/snapshot等）共享的HTML生成能力：
//   - TabGroup: Tab页数据结构（应用函数/系统函数/混合函数）
//   - 流式模板填充 / Tab栏与内容区构建 / 公共占位符替换
// P4-e 起本类不再参与多态：各域 writer 都是自己模块里的自由函数（强类型入参），
// 这里只作为一组无状态工具函数，供它们直接调用。

#include "formatter/html/html_analysis_common.h"
#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace mem_analyzer {

// 延迟构建回调：在流式写入时按需填充entry的flameTree/icicleTree
// 避免所有entry的flameTree/icicleTree同时驻留内存，大幅降低峰值内存
using FillTreeCallback = std::function<void(htmlAnalysis::SymbolDataEntry& entry, size_t groupIdx, size_t entryIdx)>;

// Tab页数据组，对应HTML中的"应用函数"/"系统函数"/"混合函数"Tab
struct TabGroup {
    std::string tabId;                                  // Tab的DOM id（如 "tabApp"）
    std::string tabLabel;                               // Tab显示标签（如 "TOP应用函数分析报告"）
    std::vector<htmlAnalysis::SymbolDataEntry> symbols; // 该Tab下的所有符号数据
    size_t startIndex;                                  // 全局序号起始值（用于表格行号）
};

// HTML模板片段集合：Tab栏/内容区/Meta/标题，两个FillHTMLTemplate函数共用
struct HtmlFragments {
    std::string tabBarHTML;
    std::string tabContentHTML;
    std::string metaDataItemsHTML;
    std::string title;
};

// HTML分析报告公共工具集，无状态、无多态。
class AnalysisHtmlFormatterBase {
public:
    // ---- 以下为各数据域的 HTML writer 共用工具 ----
    // 历史上仅供子类使用。P4 起各域不再继承本类（改为强类型入参直接调用），
    // 故这些工具对外可见，避免为了访问权限而保留一层空继承。

    // 生成Tab导航栏HTML（含各Tab的按钮）
    static std::string BuildTabBarHTML(const std::vector<TabGroup>& groups);

    // 生成Tab内容区HTML（含表格+详情区段+火焰图/冰柱图）
    // countMode: fd/thread 等计数类型时，大小字段直接显示整数个数而非字节
    static std::string BuildTabContentHTML(const std::vector<TabGroup>& groups, long long totalSize,
        bool countMode = false);

    // 流式HTML模板填充：逐个输出SymbolDataEntry，避免构建完整dataJSONArr
    // fillTree回调：在流式写入时按需构建flameTree/icicleTree，避免所有entry同时驻留内存
    static void FillHTMLTemplateStreamedDirect(
        std::ostream& out,
        std::vector<TabGroup>& groups,
        long long totalSize,
        const HtmlFragments& frags,
        FillTreeCallback fillTree = nullptr,
        bool countMode = false);

    // 流式HTML模板填充（通用版）：直接流式输出JSON数据数组
    // 适用于tshandle等非TabGroup模型的数据
    static void FillHTMLTemplateStreamed(
        std::ostream& out,
        const json& dataJSONArr,
        long long totalSize,
        const HtmlFragments& frags,
        bool countMode = false);

    // 模板中与具体数据无关的公共占位符（共享图表模块 CSS/JS、导航栏、页脚版本、全局开关）
    // 统一在此替换。模板被 __DATA_JSON__ 切成 before/after 两段，且存在 TabGroup / 通用
    // 两条注入路径，同一批占位符原先要在 4 处各写一遍；集中后新增占位符只需改这一处。
    // 各占位符只出现在固定片段，替换到不含它的片段上是空操作。
    static void ApplyCommonPlaceholders(std::string& htmlFrag, long long totalSize, bool countMode);

    // 页脚版本号片段（__VERSION_FORMAT__ / __VERSION_CONTENT__）
    static std::string SetVersionFormat();
    static std::string BuildVersionContent();
};

} // namespace mem_analyzer
