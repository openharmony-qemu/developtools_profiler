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
#include "tshandle/formatters/tshandle_html_formatter.h"
#include "config_mgr/htrace_config_mgr.h"
#include "formatter/html/analysis_html_formatter_base.h"
#include "formatter/html/html_analysis_common.h"
#include "tshandle/formatters/tshandle_call_graph.h"
#include "tshandle/tshandle_processor.h"
#include "tshandle/tshandle_analyzer.h"
#include "utils/file_utils.h"
#include "utils/platform.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/timer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace mem_analyzer::htmlAnalysis;

namespace mem_analyzer {
namespace {

constexpr double PERCENTAGE_SCALE = 100.0;
constexpr double NS_PER_SECOND = 1000000000.0;
constexpr int PERCENTAGE_DECIMAL_PLACES = 2;
constexpr int DURATION_DECIMAL_PLACES = 2;

std::string FormatTimestamp(std::string value)
{
    if (value.empty()) {
        return "-";
    }
    const size_t t = value.find('T');
    if (t != std::string::npos) {
        value[t] = ' ';
    }
    const size_t dot = value.find('.');
    if (dot != std::string::npos) {
        size_t end = dot + 1;
        while (end < value.size() && value[end] >= '0' && value[end] <= '9') {
            ++end;
        }
        value.erase(dot, end - dot);
    }
    if (!value.empty() && value.back() == 'Z') {
        value.pop_back();
    }
    return value;
}

std::string FormatTshandleDuration(long long nanoseconds)
{
    if (nanoseconds <= 0) {
        return "-";
    }
    return ToFixed(static_cast<double>(nanoseconds) / NS_PER_SECOND, DURATION_DECIMAL_PLACES) + "s";
}

std::vector<TshandleCallChain> parseChains(const TshandleTopObject& object, double denominator, size_t maxChains = 3)
{
    std::vector<TshandleCallChain> chains;
    for (const auto& src : object.callChains) {
        TshandleCallChain chain = src;
        chain.percentage = denominator > 0.0 ? (chain.existingSize / denominator * PERCENTAGE_SCALE) : 0.0;
        chains.push_back(std::move(chain));
        if (maxChains > 0 && chains.size() >= maxChains) {
            break;
        }
    }
    return chains;
}

json ChainJson(const TshandleCallChain& chain)
{
    json frames = json::array();
    for (const auto& frame : chain.frames) {
        frames.push_back({{"depth", frame.depth}, {"symbol", frame.symbol}, {"buildId", frame.buildId},
            {"lib", frame.lib}, {"libOffset", frame.libOffset}});
    }
    return {{"callStackId", chain.callStackId},
            {"existingSize", chain.existingSize},
            {"count", chain.count},
            {"percentage", chain.percentage},
            {"frames", std::move(frames)}};
}

std::vector<CallChainResult> displayChains(const std::vector<TshandleCallChain>& chains)
{
    std::vector<CallChainResult> output;
    output.reserve(chains.size());
    for (const auto& chain : chains) {
        CallChainResult display;
        display.size = chain.existingSize;
        display.count = chain.count;
        for (const auto& frame : chain.frames) {
            display.frames.push_back({frame.symbol, frame.lib, frame.libOffset});
        }
        output.push_back(std::move(display));
    }
    return output;
}

std::string BuildMetaHtml(const std::string& sourceFile, const std::string& parsedAt,
    const TraceMeta& traceMeta,
    const TshandleHandleSections& global, const TshandleTypeSection& local)
{
    std::string name = platform::Basename(sourceFile);
    if (name.empty()) {
        name = "-";
    }
    const std::string timestamp = FormatTimestamp(parsedAt);
    const std::string process = traceMeta.processName.empty() ? "-" : traceMeta.processName;
    const std::string duration = FormatTshandleDuration(traceMeta.durationNs);
    const double totalSize = global.app.totalSize + global.sys.totalSize + local.totalSize;

    return "<div class=\"meta-card\">\n"
        "  <div class=\"meta-grid\">\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">日志名：</span><span class=\"meta-value\">" + EscapeHtml(name) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">类型：</span><span class=\"meta-value\">tshandle</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">时间：</span><span class=\"meta-value\">" + EscapeHtml(timestamp) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">总内存：</span><span class=\"meta-value\">" + FormatBytes(totalSize) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">时长：</span><span class=\"meta-value\">" + duration + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">进程：</span><span class=\"meta-value\">" + EscapeHtml(process.empty() ? "-" : process) + "</span></div>\n"
        "  </div>\n"
        "</div>";
}

struct SectionResult {
    std::string tabId;
    std::string tabLabel;
    HtraceEventType handleType = HtraceEventType::UNKNOWN;
    std::string content;
};

void WriteTopTable(std::ostringstream& out,
    const std::vector<TshandleTopObject>& objects, size_t graphIndex)
{
    out << "<table class=\"top-table\">\n"
        "  <tr><th>详情</th><th>大小</th><th>数量</th><th>占比</th><th>TOP 对象名</th></tr>\n";
    size_t rowIndex = graphIndex;
    for (const auto& object : objects) {
        out << "  <tr>\n"
            "    <td><a onclick=\"showDetail(" << rowIndex <<
            "); return false;\" class=\"detail-link\">详情</a></td>\n"
            "    <td class=\"top-size-val\">" << FormatBytes(object.existingSize) << "</td>\n"
            "    <td>" << object.count << "</td>\n"
            "    <td class=\"top-pct-val\">" << ToFixed(std::min(PERCENTAGE_SCALE, object.percentage), PERCENTAGE_DECIMAL_PLACES) << "%</td>\n"
            "    <td class=\"top-func\">" << EscapeHtml(object.objectName) << "</td>\n"
            "  </tr>\n";
        ++rowIndex;
    }
    out << "</table>\n";
}

static void AppendDetailItemJson(const TshandleTopObject& object,
    const TshandleRenderNode& tree, const std::vector<TshandleCallChain>& chains, json& allData)
{
    json encodedChains = json::array();
    for (const auto& chain : chains) {
        encodedChains.push_back(ChainJson(chain));
    }
    json info = {
        {"rank", object.rank},
        {"objectName", object.objectName},
        {"existingSize", object.existingSize},
        {"count", object.count},
        {"percentage", std::min(PERCENTAGE_SCALE, object.percentage)},
        {"type", HtraceEventTypeName(object.type)}
    };
    allData.push_back({
        {"info", std::move(info)},
        {"downstreamTree", tree.ToJson()},
        {"callChains", std::move(encodedChains)}
    });
}

void WriteDetailItem(std::ostringstream& out, const TshandleTopObject& object,
    double denominator, json& allData, size_t& graphIndex)
{
    const std::string& objectName = object.objectName;
    const auto allChains = parseChains(object, denominator, 0);
    const auto tree = BuildTshandleDownstreamGraph(objectName, allChains, object.existingSize);
    const auto chains = parseChains(object, denominator, 3);
    const std::string chainHtml = BuildCallChainHTML(displayChains(chains), denominator);

    AppendDetailItemJson(object, tree, chains, allData);

    out << "<div class=\"top-item\" id=\"topItem" << graphIndex << "\">\n"
        "  <div class=\"top-item-header expanded\" id=\"topItemHeader" << graphIndex <<
        "\" onclick=\"toggleDetail(" << graphIndex << ")\">\n"
        "    <span class=\"arrow\">&#9654;</span>\n"
        "    <span class=\"header-rank\">TOP" << object.rank <<
        "对象：</span><span class=\"header-func\">" << EscapeHtml(objectName) << "</span>\n"
        "    <div class=\"header-stats\">大小 " << FormatBytes(object.existingSize) <<
        "，数量" << object.count << "，占比" <<
        ToFixed(std::min(PERCENTAGE_SCALE, object.percentage), PERCENTAGE_DECIMAL_PLACES) << "%</div>\n"
        "  </div>\n"
        "  <div class=\"top-item-body expanded\" id=\"topItemBody" << graphIndex << "\">\n"
        "    <div class=\"call-chains-section\">\n      " <<
        (chainHtml.empty() ?
            "<div class=\"call-chain-line\" style=\"color:#a0a0b0;\">无调用链数据</div>" :
            chainHtml) <<
        "\n    </div>\n"
        "    <div class=\"graphToolbar\">\n"
        "      <button onclick=\"resetZoom(" << graphIndex << ")\" class=\"reset-btn\">Reset Zoom</button>\n"
        "      <input id=\"searchInput" << graphIndex <<
        "\" type=\"text\" placeholder=\"搜索对象名...\" class=\"graph-search-input\">\n"
        "      <span class=\"search-match-count\" id=\"searchCount" << graphIndex << "\">0个匹配</span>\n"
        "      <span class=\"shortcut-hint\">w/s：放大/缩小&emsp;&emsp;a/d：左/右移动</span>\n"
        "    </div>\n"
        "    <div class=\"dual-graph-container\">\n"
        "      <div class=\"graph-panel\">\n"
        "        <div class=\"graph-side-label\">下游调用链</div>\n"
        "        <div class=\"graph-canvas-area\"><canvas tabindex=\"0\" id=\"downstreamCanvas" <<
        graphIndex << "\"></canvas></div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n";
    ++graphIndex;
}

SectionResult BuildSection(const char* tabId, const char* label,
    HtraceEventType handleType, const TshandleTypeSection& section,
    double denominator,
    json& allData, size_t& graphIndex)
{
    SectionResult result{tabId, label, handleType, ""};
    
    const auto& objects = section.topObjects;
    // 如果完全没有抓到该类型数据，或者没有在堆快照中关联到 ArkTS 对象
    // 则直接在 Tab 内容区提示无数据
    if (!section.hasData || objects.empty()) {
        result.content = "<div class=\"empty-msg\">无分析数据</div>";
        return result;
    }

    const long long totalObjects = section.totalObjects;
    const double totalSize = section.totalSize;

    const char summaryPrefix[] = "\xE6\x9C\x89\xE5\xA0\x86\xE6\xA0\x88 ";
    const char summaryMid[] = " \xE5\xAF\xB9\xE8\xB1\xA1\xE6\x80\xBB\xE6\x95\xB0\xE9\x87\x8F\xEF\xBC\x9A";
    const char summarySuffix[] = " \xE4\xB8\xAA\xEF\xBC\x8C\xE6\x80\xBB\xE5\xA4\xA7\xE5\xB0\x8F\xEF\xBC\x9A";

    std::ostringstream out;

    WriteTopTable(out, objects, graphIndex);

    for (const auto& object : objects) {
        WriteDetailItem(out, object, denominator, allData, graphIndex);
    }
    result.content = out.str();
    return result;
}

std::string BuildTabBar(const std::vector<SectionResult>& sections)
{
    std::string output = "<div class=\"tab-bar\">\n";
    bool first = true;
    for (const auto& section : sections) {
        if (section.content.empty()) {
            continue;
        }
        output += "  <div class=\"tab-item" + std::string(first ? " active" : "") +
            "\" data-tab=\"" + section.tabId +
            "\" onclick=\"switchTab('" + section.tabId + "', this)\">" +
            section.tabLabel + "</div>\n";
        first = false;
    }
    output += "</div>";
    if (first) {
        return "<div class=\"empty-msg\">无分析数据</div>";
    }
    return output;
}

std::string EscapeEmbeddedData(std::string page)
{
    const std::string marker = "var allData = ";
    const size_t begin = page.find(marker);
    if (begin == std::string::npos) {
        return page;
    }
    const size_t valueBegin = begin + marker.size();
    const size_t end = page.find(';', valueBegin);
    if (end == std::string::npos) {
        return page;
    }
    std::string encoded = page.substr(valueBegin, end - valueBegin);
    encoded = ReplaceAll(encoded, "<", "\\u003c");
    encoded = ReplaceAll(encoded, ">", "\\u003e");
    encoded = ReplaceAll(encoded, "&", "\\u0026");
    const std::string lineSeparator({
        static_cast<char>(0xE2), static_cast<char>(0x80), static_cast<char>(0xA8)});
    const std::string paragraphSeparator({
        static_cast<char>(0xE2), static_cast<char>(0x80), static_cast<char>(0xA9)});
    encoded = ReplaceAll(encoded, lineSeparator, "\\u2028");
    encoded = ReplaceAll(encoded, paragraphSeparator, "\\u2029");
    page.replace(valueBegin, end - valueBegin, encoded);
    return page;
}

std::string BuildTabContent(const std::vector<SectionResult>& sections)
{
    std::string output;
    bool first = true;
    for (const auto& section : sections) {
        if (section.content.empty()) {
            continue;
        }
        output += "<div class=\"tab-content" + std::string(first ? " active" : "") +
            "\" id=\"" + section.tabId + "\">\n" +
            section.content + "\n</div>\n";
        first = false;
    }
    return output;
}

void FormatWithStream(std::ostream& out, const TshandleAnalysisOutput& analysis, const std::string& filePath)
{
    auto& htraceCfg = HtraceConfigMgr::Instance();
    if (!htraceCfg.IsLoaded()) {
        std::string cfgErr;
        htraceCfg.LoadDefault(cfgErr);
    }

    json allData = json::array();
    size_t graphIndex = 0;
    std::vector<SectionResult> sections;

    const double globalTotal = analysis.global.app.totalSize + analysis.global.sys.totalSize;
    sections.push_back(BuildSection("tabGlobalApp", "TOP GlobalHandle（应用栈）引用ArkTS对象分析报告",
        HtraceEventType::ARK_GLOBAL_HANDLE, analysis.global.app,
        globalTotal, allData, graphIndex));
    sections.push_back(BuildSection("tabGlobalSys", "TOP GlobalHandle（系统栈）引用ArkTS对象分析报告",
        HtraceEventType::ARK_GLOBAL_HANDLE, analysis.global.sys,
        globalTotal, allData, graphIndex));
    sections.push_back(BuildSection("tabLocal", "TOP LocalHandle 引用 ArkTS 对象分析报告",
        HtraceEventType::ARK_LOCAL_HANDLE, analysis.local,
        analysis.local.totalSize, allData, graphIndex));

    const double totalSize = analysis.global.app.totalSize + analysis.global.sys.totalSize +
                            analysis.local.totalSize;
    const long long totalForTemplate = totalSize <= static_cast<double>(std::numeric_limits<long long>::max()) ?
        static_cast<long long>(totalSize) : 0;

    const std::string title = "资源泄漏日志分析报告";

    std::ostringstream page;
    AnalysisHtmlFormatterBase::FillHTMLTemplateStreamed(
        page, allData, totalForTemplate, BuildTabBar(sections), BuildTabContent(sections),
        BuildMetaHtml(filePath, analysis.parsedAt, analysis.traceMeta, analysis.global, analysis.local),
            EscapeHtml(title));
    std::string html = page.str();

    html = ReplaceAll(html,
        "</style>",
        ".arrow { transform: none !important; }\n"
        "</style>");
    html = ReplaceAll(html,
        "</script>",
        "var origToggleDetail = window.toggleDetail;\n"
        "window.toggleDetail = function(idx) {\n"
        "  var header = document.getElementById('topItemHeader' + idx);\n"
        "  var wasExpanded = header && header.classList.contains('expanded');\n"
        "  if (origToggleDetail) origToggleDetail(idx);\n"
        "  var arrow = header ? header.querySelector('.arrow') : null;\n"
        "  if (arrow) arrow.textContent = wasExpanded ? '\\u25B6' : '\\u25BC';\n"
        "};\n"
        "</script>");
    out << EscapeEmbeddedData(std::move(html));
}

} // namespace

std::string TshandleWriteAnalysisHtml(const TshandleAnalysisOutput& analysis,
    const std::string& filePath, const RunContext& ctx,
    const std::string& outDir)
{
    (void)ctx;  // 当前 tshandle HTML 不使用运行参数
    StageTimer timer("HTML write");

    auto base = platform::Stem(filePath);
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

    FormatWithStream(ofs, analysis, filePath);

    ofs.close();
    timer.Tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer
