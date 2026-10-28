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
#include "htrace/formatters/htrace_html_formatter.hpp"
#include "htrace/htrace_category.hpp"
#include "htrace/formatters/htrace_flame_json.hpp"
#include "formatter/html/analysis_html_formatter_base.hpp"
#include "formatter/html/html_analysis_common.hpp"
#include "formatter/html/components/help_tooltip.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <variant>

using namespace mem_analyzer::htmlAnalysis;

namespace mem_analyzer {
namespace {

// mmap 二级 tab 内容：每个一级 tab（app/system/mixed）下拆 mmap-anon（symbol 表格）/ mmap-filepage（文件表格）两个子 tab。
// 一级 .tab-content 显隐复用现有 switchTab + .tab-content.active CSS；
// 二级 .sub-tab-content 显隐用内联 style.display（JS 控制，不依赖 CSS 类，避免改共享 HTML_TEMPLATE）。

// FilepageInfo → SymbolDataEntry（info 自包含，flameTree/icicleTree 在 fillTree 懒构建）
std::vector<SymbolDataEntry> buildFilepageEntries(const HtraceSection& section)
{
    std::vector<SymbolDataEntry> entries;
    entries.reserve(section.rows.size());
    int rank = 1;
    for (const auto& row : section.rows) {
        const auto* fp = std::get_if<FilepageInfo>(&row);
        if (!fp) {
            continue;
        }
        SymbolDataEntry entry;
        entry.info = json{
            {"name", fp->filename},
            {"lib", ""},
            {"libOffset", ""},
            {"existingSize", fp->size},
            {"existingCount", fp->count},
            {"percentage", fp->percentage},
            {"type", "mmap-filepage"},
            {"isApp", false}
        };
        entry.info["rank"] = rank++;
        entries.push_back(std::move(entry));
    }
    return entries;
}

// filepage 文件表格行：详情|大小|次数|占比|文件名
std::string BuildFilepageTableRows(const HtraceSection& section, size_t startIndex, bool countMode)
{
    std::vector<std::string> rows;
    size_t i = 0;
    for (const auto& row : section.rows) {
        const auto* fp = std::get_if<FilepageInfo>(&row);
        if (!fp) {
            continue;
        }
        size_t idx = startIndex + i;
        std::ostringstream pctSS;
        pctSS << std::fixed << std::setprecision(2) << fp->percentage;
        rows.push_back("<tr>\n"
            "  <td><a onclick=\"showDetail(" + std::to_string(idx) + "); return false;\" class=\"detail-link\">\u8be6\u60c5</a></td>\n" +
            std::string(countMode ? "" : "  <td class=\"top-size-val\">" + FormatSizeText((double)fp->size, false) + "</td>\n") +
            "  <td class=\"top-count-val\">" + std::to_string(fp->count) + "</td>\n"
            "  <td class=\"top-pct-val\">" + pctSS.str() + "%</td>\n"
            "  <td class=\"top-func\"><b>" + EscapeHtml(fp->filename) + "</b></td>\n"
            "</tr>");
        ++i;
    }
    std::string out;
    for (size_t j = 0; j < rows.size(); j++) {
        if (j > 0) {
            out += "\n";
        }
        out += rows[j];
    }
    return out;
}

// filepage 详情：文件名 header + 纯冰柱图（文件为根，无调用链列表）
std::string BuildFilepageDetailSectionContent(const FilepageInfo& fp, size_t idx, bool countMode)
{
    std::ostringstream pctSS;
    pctSS << std::fixed << std::setprecision(2) << fp.percentage;
    std::string stats = countMode ?
        ("\u6b21\u6570" + std::to_string(fp.count) + "\uff0c\u5360\u6bd4" + pctSS.str() + "%") :
        ("\u5927\u5c0f " + FormatSizeText((double)fp.size, false) + "\uff0c\u6b21\u6570" + std::to_string(fp.count) + "\uff0c\u5360\u6bd4" + pctSS.str() + "%");
    return "<div class=\"top-item\" id=\"topItem" + std::to_string(idx) + "\">\n"
        "  <div class=\"top-item-header collapsed\" id=\"topItemHeader" + std::to_string(idx) + "\" onclick=\"toggleDetail(" + std::to_string(idx) + ")\">\n"
        "    <span class=\"arrow\" id=\"topItemArrow" + std::to_string(idx) + "\">&#9654;</span>\n"
        "    <span class=\"header-rank\">\u6587\u4ef6\uff1a</span><span class=\"header-func\">" + EscapeHtml(fp.filename) + "</span>\n"
        "    <div class=\"header-stats\">" + stats + "</div>\n"
        "  </div>\n"
        "  <div class=\"top-item-body\" id=\"topItemBody" + std::to_string(idx) + "\">\n"
        "    <div class=\"graphToolbar\">\n"
        "      <button onclick=\"resetZoom(" + std::to_string(idx) + ")\" class=\"reset-btn\">Reset Zoom</button>\n"
        "      <input id=\"searchInput" + std::to_string(idx) + "\" type=\"text\" placeholder=\"\u641c\u7d22\u51fd\u6570\u540d...\" class=\"graph-search-input\">\n"
        "      <span class=\"search-match-count\" id=\"searchCount" + std::to_string(idx) + "\">0\u4e2a\u5339\u914d</span>\n"
        "      <span class=\"shortcut-hint\">w/s\uff1a\u653e\u5927/\u7f29\u5c0f&emsp;&emsp;a/d\uff1a\u5de6/\u53f3\u79fb\u52a8</span>\n"
        "    </div>\n"
        "    <div class=\"dual-graph-container\">\n"
        "      <div class=\"graph-panel\">\n"
        "        <div class=\"graph-side-label\">\u6587\u4ef6\u8c03\u7528\u6808</div>\n"
        "        <div class=\"graph-canvas-area\">\n"
        "          <canvas id=\"icicleCanvas" + std::to_string(idx) + "\"></canvas>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>";
}

std::string BuildFilepageDetailSections(const HtraceSection& section, size_t startIndex, bool countMode)
{
    std::vector<std::string> sections;
    size_t i = 0;
    for (const auto& row : section.rows) {
        const auto* fp = std::get_if<FilepageInfo>(&row);
        if (!fp) {
            continue;
        }
        sections.push_back(BuildFilepageDetailSectionContent(*fp, startIndex + i, countMode));
        ++i;
    }
    std::string out;
    for (size_t j = 0; j < sections.size(); j++) {
        if (j > 0) {
            out += "\n";
        }
        out += sections[j];
    }
    return out;
}

std::string BuildMmapTabContentHTML(const std::vector<TabGroup>& groups,
    const std::vector<TabGroup>& streamGroups,
    const std::vector<const HtraceSection*>& filepageLists,
    long long totalSize, bool countMode)
{
    std::string result;
    result += "<style>\n"
        ".sub-tab-bar { display: flex; margin: 20px 0 0; border-bottom: 1px solid #dee2e6; }\n"
        ".sub-tab-item { display: inline-block; padding: 8px 14px 7px; margin-right: 6px; cursor: pointer; font-size: 16px; font-weight: 600; color: #888; border-bottom: 2px solid transparent; margin-bottom: -1px; transition: color 0.2s, border-color 0.2s; }\n"
        ".sub-tab-item:hover { color: #e94560; }\n"
        ".sub-tab-item.active { color: #e94560; border-bottom-color: #e94560; }\n";
    result += HelpTooltipCss();
    result += "\n</style>\n";

    for (size_t scope = 0; scope < 3; ++scope) {
        if (scope >= groups.size()) {
            break;
        }
        const TabGroup& g = groups[scope];
        if (g.symbols.empty()) {
            continue;
        }// 一级 tab 无数据则跳过（与 tab bar 一致）

        const TabGroup& anonG = streamGroups[scope];                 // anon 子 tab（scope 对应）
        const TabGroup& filepageG = streamGroups[3 + scope];         // filepage 子 tab（app/system/mixed 对应）
        const HtraceSection& fps = *filepageLists[scope];            // 对应 scope 的 filepage 轨

        result += "<div class=\"tab-content\" id=\"" + g.tabId + "\">\n";

        bool anonEmpty = anonG.symbols.empty();
        bool fpEmpty = fps.rows.empty();
        bool anonActive = !anonEmpty || fpEmpty;
        bool fpActive = anonEmpty && !fpEmpty;

        result += "  <div class=\"sub-tab-bar\">\n";
        result += "    <div class=\"sub-tab-item" + std::string(anonActive ? " active" : "") +
               "\" onclick=\"switchSubTab('" + anonG.tabId + "', this, '" + g.tabId + "')\">" + anonG.tabLabel + "</div>\n";
        result += "    <div class=\"sub-tab-item" + std::string(fpActive ? " active" : "") +
               "\" onclick=\"switchSubTab('" + filepageG.tabId + "', this, '" + g.tabId + "')\">" + filepageG.tabLabel +
               "<span class=\"help-icon\" data-tooltip=\"当前将部分内核节点也认为是文件，例如 anon_inode:dmabuf节点、 ashmem节点和hvgr0节点。\">?</span></div>\n";
        result += "  </div>\n";

        // anon 子 tab：symbol 表格 + 详情
        result += "  <div class=\"sub-tab-content\" id=\"" + anonG.tabId + "\"" +
            std::string(anonActive ? " style=\"display:block;\"" : " style=\"display:none;\"") + ">\n";
        if (anonEmpty) {
            result += "    <div class=\"empty-msg\">\u65e0\u76f8\u5173\u6570\u636e</div>\n";
        } else {
            std::string tableRows = BuildTopTableRows(anonG.symbols, anonG.startIndex, countMode);
            std::string detailSections = BuildDetailSections(anonG.symbols, anonG.startIndex, totalSize, countMode);
            result += "    <table class=\"top-table\">\n"
                "      <tr><th>\u8be6\u60c5</th>" + std::string(countMode ? "" : "<th>\u5927\u5c0f</th>") + "<th>\u6b21\u6570</th><th>\u5360\u6bd4</th><th>TOP\u51fd\u6570</th><th>\u5b8c\u6574\u8def\u5f84</th></tr>\n"
                "      " + tableRows + "\n"
                "    </table>\n"
                "    " + detailSections + "\n";
        }
        result += "  </div>\n";

        // filepage 子 tab：文件表格 + 详情（按 app/system/mixed 分类）
        size_t fpStart = filepageG.startIndex;
        result += "  <div class=\"sub-tab-content\" id=\"" + filepageG.tabId + "\"" +
            std::string(fpActive ? " style=\"display:block;\"" : " style=\"display:none;\"") + ">\n";
        if (fpEmpty) {
            result += "    <div class=\"empty-msg\">\u65e0\u76f8\u5173\u6570\u636e</div>\n";
        } else {
            std::string fpRows = BuildFilepageTableRows(fps, fpStart, countMode);
            std::string fpDetails = BuildFilepageDetailSections(fps, fpStart, countMode);
            result += "    <table class=\"top-table\">\n"
                "      <tr><th>\u8be6\u60c5</th>" + std::string(countMode ? "" : "<th>\u5927\u5c0f</th>") + "<th>\u6b21\u6570</th><th>\u5360\u6bd4</th><th>\u6587\u4ef6\u540d</th></tr>\n"
                "      " + fpRows + "\n"
                "    </table>\n"
                "    " + fpDetails + "\n";
        }
        result += "  </div>\n";

        result += "</div>\n\n";
    }

    result += "<script>\n"
        "window.switchSubTab = function(subTabId, el, parentTabId) {\n"
        "  var scope = document.getElementById(parentTabId);\n"
        "  if (!scope) return;\n"
        "  var items = scope.querySelectorAll('.sub-tab-item');\n"
        "  for (var i = 0; i < items.length; i++) items[i].classList.remove('active');\n"
        "  if (el) el.classList.add('active');\n"
        "  var contents = scope.querySelectorAll('.sub-tab-content');\n"
        "  for (var i = 0; i < contents.length; i++) contents[i].style.display = 'none';\n"
        "  var content = document.getElementById(subTabId);\n"
        "  if (content) content.style.display = 'block';\n"
        "  MemChart.redrawAll();\n"
        "};\n";
    result += HelpTooltipJs();
    result += "\n</script>\n";

    return result;
}

// 流式组轨道元数据：fillTree 按 render 分派行种，按 section->rows[entryIdx] 取源行；
// 替代 isMmap && groupIdx>=3 / filepageLists[groupIdx-3] 魔法下标。
struct StreamGroupMeta {
    TrackRender render;
    const HtraceSection* section;
};

void FormatWithStream(std::ostream& out, const HtraceAnalysisOutput& full,
    const std::string& filePath, const RunContext& ctx)
{
    StageTimer timer("HTML gen");

    std::string processName = full.processName;
    if (processName.empty()) {
        processName = "未知";
    }

    long long durationNs = full.traceMeta.durationNs;
    long long startMs = full.traceMeta.startTimeMs;
    double durationSeconds = std::round(durationNs / 1e6) / 1e3;
    std::string duration;
    {
        std::ostringstream durSs;
        durSs << std::fixed << std::setprecision(3) << durationSeconds;
        duration = durSs.str();
    }
    long long totalExistingSize = full.totalLeakSize;

    std::vector<SymbolDataEntry> appSymbolData;
    std::vector<SymbolDataEntry> systemSymbolData;
    std::vector<SymbolDataEntry> mixedSymbolData;

    // Build SymbolDataEntry directly from HtraceAnalysisOutput (no JSON round-trip)
    auto buildEntries = [&](const ScopeResult& scope) -> std::vector<SymbolDataEntry> {
        std::vector<SymbolDataEntry> entries;
        entries.reserve(scope.SymbolCount());
        int rank = 1;
        for (const auto& section : scope.sections)
            for (const auto& row : section.rows)
                if (const auto* ts = std::get_if<TopSymbolInfo>(&row)) {
                    auto entry = BuildSymbolDataEntryFromContextLight(*ts, totalExistingSize, full.dataDict);
                    entry.info["rank"] = rank++;
                    entries.push_back(std::move(entry));
                }
        return entries;
    };

    appSymbolData = buildEntries(full.topSymbols.app);
    systemSymbolData = buildEntries(full.topSymbols.system);
    mixedSymbolData = buildEntries(full.topSymbols.mixed);

    const HtraceCategorySpec& categorySpec = HtraceCategory(ctx.htrace.category);
    const bool multipart = categorySpec.IsMultipart();

    // Build TabGroups（一级 tab：合并视图，用于 tab bar 的"是否有数据"判断）
    std::vector<TabGroup> groups;
    groups.push_back({"tabApp", "TOP应用函数分析报告", appSymbolData, 0});
    groups.push_back({"tabSystem", "TOP系统函数分析报告", systemSymbolData, appSymbolData.size()});
    groups.push_back({"tabMixed", "TOP应用与系统函数分析报告", mixedSymbolData, appSymbolData.size() + systemSymbolData.size()});

    // 流式组 + 每组轨道元数据：fillTree 据此按行种分派（Symbol→符号树 / Filepage→文件冰柱图），
    // 替代 isMmap && groupIdx>=3 / filepageLists[groupIdx-3] 魔法下标。
    // 非 mmap = 3 组（每 scope 一条 Symbol 轨）；mmap = 6 组（3 scope × [Symbol 轨, Filepage 轨]）。
    std::vector<TabGroup> streamGroups;
    std::vector<StreamGroupMeta> streamMeta;
    std::vector<const HtraceSection*> filepageLists;  // [3] = app/system/mixed 的 filepage 轨（sections[1]）
    if (multipart) {
        auto anonApp = buildEntries(full.topSymbols.app);
        auto anonSystem = buildEntries(full.topSymbols.system);
        auto anonMixed = buildEntries(full.topSymbols.mixed);

        // filepage 轨：三个 scope 的 sections[1]（analyzer 已按 app/system/mixed 预处理，mixed 已合并排序）
        const HtraceSection* appFp = &full.topSymbols.app.sections[1];
        const HtraceSection* systemFp = &full.topSymbols.system.sections[1];
        const HtraceSection* mixedFp = &full.topSymbols.mixed.sections[1];

        auto appFilepageEntries = buildFilepageEntries(*appFp);
        auto systemFilepageEntries = buildFilepageEntries(*systemFp);
        auto mixedFilepageEntries = buildFilepageEntries(*mixedFp);

        // 子 tab 标签单一来源：analyzer 已把 tracks[].label 写进 section.label
        const std::string& anonLabel = full.topSymbols.app.sections[0].label;
        const std::string& filepageLabel = appFp->label;

        size_t o = 0;
        streamGroups.push_back({"appMmapAnon", anonLabel, anonApp, o});
        o += anonApp.size();
        streamGroups.push_back({"systemMmapAnon", anonLabel, anonSystem, o});
        o += anonSystem.size();
        streamGroups.push_back({"mixedMmapAnon", anonLabel, anonMixed, o});
        o += anonMixed.size();

        size_t fpAppStart = o;
        streamGroups.push_back({"appMmapFilepage",    filepageLabel, appFilepageEntries,    fpAppStart});
        size_t fpSystemStart = fpAppStart + appFilepageEntries.size();
        streamGroups.push_back({"systemMmapFilepage", filepageLabel, systemFilepageEntries, fpSystemStart});
        size_t fpMixedStart = fpSystemStart + systemFilepageEntries.size();
        streamGroups.push_back({"mixedMmapFilepage",  filepageLabel, mixedFilepageEntries,  fpMixedStart});

        filepageLists.push_back(appFp);
        filepageLists.push_back(systemFp);
        filepageLists.push_back(mixedFp);

        streamMeta.push_back({TrackRender::Symbol,   &full.topSymbols.app.sections[0]});
        streamMeta.push_back({TrackRender::Symbol,   &full.topSymbols.system.sections[0]});
        streamMeta.push_back({TrackRender::Symbol,   &full.topSymbols.mixed.sections[0]});
        streamMeta.push_back({TrackRender::Filepage, appFp});
        streamMeta.push_back({TrackRender::Filepage, systemFp});
        streamMeta.push_back({TrackRender::Filepage, mixedFp});

        // mmap：一级 tab 的「是否有数据」判断要同时反映 anon（groups）和 filepage（filepageLists）。
        // 但只对当前 scope 模式的活动 scope 生效——AppAndSystem 时 mixed 非活动（不能因为
        // mixed 合并 filepage 非空就显示 mixed tab，那是 AppAndSystem 下本就该隐藏的 tab）。
        // 只有当活动 scope 的 anon 为空、但其 filepage 有数据时，塞一个占位 entry 保住该 tab。
        auto scopeActive = [&](size_t idx) -> bool {
            switch (ctx.htrace.scope) {
                case Scope::AppOnly:    return idx == 0;
                case Scope::SystemOnly: return idx == 1;
                case Scope::Mixed:      return idx == 2;
                case Scope::AppAndSystem:
                default:                return idx == 0 || idx == 1;
            }
        };
        for (size_t scope = 0; scope < 3 && scope < groups.size(); ++scope) {
            if (scopeActive(scope) && groups[scope].symbols.empty() && !filepageLists[scope]->rows.empty()) {
                SymbolDataEntry placeholder;
                placeholder.info = json{{"name", ""}};
                groups[scope].symbols.push_back(std::move(placeholder));
            }
        }
    } else {
        // 单轨：流式组 = groups 本身（3 个 scope，各一条 Symbol 轨）
        streamMeta.push_back({TrackRender::Symbol, &full.topSymbols.app.sections[0]});
        streamMeta.push_back({TrackRender::Symbol, &full.topSymbols.system.sections[0]});
        streamMeta.push_back({TrackRender::Symbol, &full.topSymbols.mixed.sections[0]});
    }

    // Build htrace-specific meta items
    std::string formattedDuration = FormatDuration(duration);
    std::string escapedSourceFile = EscapeHtml(platform::basename(filePath));

    // Generate timestamp from trace_range
    std::string timestamp;
    if (startMs > 0) {
        time_t t = static_cast<time_t>(startMs / 1000);
        char buf[64];
        struct tm tm_info;
#if defined(_WIN32)
        localtime_s(&tm_info, &t);
#else
        localtime_r(&t, &tm_info);
#endif
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm_info);
        timestamp = buf;
    }
    std::string formattedTimestamp = timestamp;
    {
        size_t tpos = formattedTimestamp.find('T');
        if (tpos != std::string::npos) {
            formattedTimestamp[tpos] = ' ';
        }
        std::regex dot_re("\\.\\d+");
        formattedTimestamp = std::regex_replace(formattedTimestamp, dot_re, "");
        std::regex tz_re("[+-]\\d{2}:\\d{2}$");
        formattedTimestamp = std::regex_replace(formattedTimestamp, tz_re, "");
    }
    std::string escapedTimestamp = EscapeHtml(formattedTimestamp);
    std::string escapedProcessName = EscapeHtml(processName);

    // Resource type display: htrace-<type>（描述符表单一来源，-l 泄漏报告固定 htrace-leaks）
    std::string typeDisplay = categorySpec.htmlTypeClass;
    if (!ctx.htrace.leakAddrs.empty()) {
        typeDisplay = "htrace-leaks";
    }
    // fd/thread 的大小语义是个数（计数），不以字节格式化
    bool countMode = categorySpec.countMode;

    std::string metaDataItemsHTML = "<div class=\"meta-card\">\n"
        "  <div class=\"meta-grid\">\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>日志名：</b></span><span class=\"meta-value\">" + escapedSourceFile + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>类型：</b></span><span class=\"meta-value\">" + typeDisplay + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>时间：</b></span><span class=\"meta-value\">" + escapedTimestamp + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>" + (countMode ? "\u603b\u4e2a\u6570\uff1a" : "\u603b\u5185\u5b58\uff1a") + "</b></span><span class=\"meta-value\">" + (countMode ? std::to_string(totalExistingSize) : FormatBytes((double)totalExistingSize)) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>Trace时长：</b></span><span class=\"meta-value\">" + formattedDuration + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\"><b>进程：</b></span><span class=\"meta-value\">" + escapedProcessName + "</span></div>\n"
        "  </div>\n"
        "</div>";

    std::string title = "资源泄漏日志分析报告";

    // Use direct streaming: outputs each SymbolDataEntry individually and releases
    // flameTree/icicleTree after streaming, reducing peak memory
    std::string tabBarHTML = AnalysisHtmlFormatterBase::buildTabBarHTML(groups);
    std::string tabContentHTML = multipart ?
        BuildMmapTabContentHTML(groups, streamGroups, filepageLists, totalExistingSize, countMode) :
        AnalysisHtmlFormatterBase::buildTabContentHTML(groups, totalExistingSize, countMode);

    // 延迟构建回调：在流式写入时按需构建flameTree/icicleTree
    // 避免所有entry的flameTree/icicleTree同时驻留内存，峰值内存从O(N)降至O(1)
    FillTreeCallback fillTree;
    fillTree = [&streamMeta, &full](SymbolDataEntry& entry, size_t groupIdx, size_t entryIdx) {
        const StreamGroupMeta& meta = streamMeta[groupIdx];
        if (meta.render == TrackRender::Filepage) {
            // 文件轨：文件名为根冰柱图（无调用链列表）
            const auto& fp = std::get<FilepageInfo>(meta.section->rows[entryIdx]);
            StringTable st;
            std::vector<int> ids(fp.callStacks.size());
            for (size_t k = 0; k < ids.size(); ++k) {
                ids[k] = static_cast<int>(k);
            }
            FlameNode merged = BuildMergedIcicle(fp.callStacks, ids, full.dataDict);
            entry.icicleTree = BuildIcicleTreeJsonFromFlameNode(merged, fp.filename, "", "", full.dataDict, st);
            entry.names = st.toArray();
            return;
        }
        // 符号轨
        const auto& symRows = meta.section->rows;
        if (entryIdx < symRows.size()) {
            const auto& ts = std::get<TopSymbolInfo>(symRows[entryIdx]);
            std::string lib = ts.libs.empty() ? "" : ts.libs[0];
            StringTable st;
            // other: 复用下游调用链(icicle)格式，用 build_merged_flame 的树序列化，根"Others"在顶；
            // 非 other 保持上游 flame + 下游 icicle 双 panel
            bool isOther = (ts.symbol == OTHER_SYMBOL);
            if (isOther) {
                entry.icicleTree = BuildIcicleTreeJsonFromFlameNode(
                    ts.icicleGraph, OTHER_SYMBOL, lib, std::string(), full.dataDict, st);
            } else {
                entry.flameTree = BuildFlameTreeJsonFromFlameNode(ts.flameGraph, ts.symbol, lib, ts.libOffset, full.dataDict, st);
                entry.icicleTree = BuildIcicleTreeJsonFromFlameNode(ts.icicleGraph, ts.symbol, lib, ts.libOffset, full.dataDict, st);
            }
            entry.names = st.toArray();
        }
    };

    std::vector<TabGroup>& streamTarget = multipart ? streamGroups : groups;
    AnalysisHtmlFormatterBase::fillHTMLTemplateStreamedDirect(
        out, streamTarget, totalExistingSize, tabBarHTML, tabContentHTML, metaDataItemsHTML, title, fillTree, countMode);

    timer.Tick("total");
}

} // namespace

std::string HtraceWriteAnalysisHtml(const HtraceAnalysisOutput& full,
    const std::string& filePath, const RunContext& ctx,
    const std::string& outDir)
{
    StageTimer timer("HTML write");

    auto base = platform::stem(filePath);
    if (base.empty()) {
        base = "output";
    }
    auto outPath = platform::JoinPath(outDir, base + ".html");
    platform::CreateDirectories(outDir);

    auto ofs = OpenOfstream(outPath);
    if (!ofs) {
        log::Loge("Error: Cannot write to ", outPath);
        return {};
    }

    FormatWithStream(ofs, full, filePath, ctx);

    ofs.close();
    timer.Tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer
