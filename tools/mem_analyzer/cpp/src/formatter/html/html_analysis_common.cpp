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
#include "formatter/html/html_analysis_common.h"
#include "config_mgr/htrace_config_mgr.h"
#include "htrace/htrace_analyzer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <regex>
#include <sstream>
#include <set>
#include <string>
#include <vector>

namespace mem_analyzer::htmlAnalysis {

constexpr int DECIMAL_PRECISION = 2;
constexpr int MAX_BYTES_UNIT_INDEX = 3;
constexpr int ESCAPE_RESERVE_NUMERATOR = 11;
constexpr int SIMPLE_CALL_CHAIN_THRESHOLD = 2;

// ===== Helper functions =====

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
    if (from.empty()) {
        return str;
    }
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

std::string FormatFuncName(const std::string& symbol, const std::string& lib, const std::string& libOffset)
{
    // If symbol is unresolved (contains .so), skip (symbol) part to avoid duplication
    bool isUnresolved = symbol.find(".so") != std::string::npos;
    if (!lib.empty()) {
        if (isUnresolved) {
            return libOffset.empty() ? lib : (lib + "+" + libOffset);
        } else if (!symbol.empty()) {
            return libOffset.empty() ? (lib + " (" + symbol + ")") : (lib + "+" + libOffset + " (" + symbol + ")");
        } else {
            return libOffset.empty() ? lib : (lib + "+" + libOffset);
        }
    }
    if (!symbol.empty()) {
        return symbol;
    }
    return "unknown";
}

std::string FormatShortFuncName(const std::string& symbol, const std::string& lib, const std::string& libOffset)
{
    bool isUnresolved = symbol.find(".so") != std::string::npos;
    std::string libBase = platform::Basename(lib);
    if (isUnresolved) {
        if (!libBase.empty() && !libOffset.empty()) {
            return libBase + "+" + libOffset;
        }
        if (!libBase.empty()) {
            return libBase;
        }
        return symbol;
    }
    if (!symbol.empty()) {
        if (!libBase.empty()) {
            return libOffset.empty() ? (libBase + " (" + symbol + ")") :
                (libBase + "+" + libOffset + " (" + symbol + ")");
        }
        return symbol;
    }
    if (!libBase.empty() && !libOffset.empty()) {
        return libBase + "+" + libOffset;
    }
    if (!libBase.empty()) {
        return libBase;
    }
    return "unknown";
}

std::string GetLibForDisplay(const std::string& lib, const std::string& symbol)
{
    if (!lib.empty() && lib != "unknown" && lib != "unknow") {
        return lib;
    }
    std::regex tsMatch("(\\S+\\.ts):\\d+:\\d+");
    std::smatch m;
    if (std::regex_search(symbol, m, tsMatch) && m.size() > 1) {
        return m[1].str();
    }
    return lib.empty() ? "" : lib;
}

// Round half away from zero to match JavaScript's toFixed() behavior
double RoundHalfAwayFromZero(double value, int precision)
{
    double factor = std::pow(10.0, precision);
    double scaled = value * factor;
    double sign = scaled >= 0 ? 1.0 : -1.0;
    double rounded = std::floor(std::abs(scaled) + 0.5) * sign;
    return rounded / factor;
}

std::string FormatBytes(double bytes)
{
    if (bytes == 0) {
        return "0 B";
    }
    static const char* units[] = {"B", "KB", "MB", "GB"};
    int k = 1024;
    double dbytes = bytes;
    int i = 0;
    if (dbytes >= 1) {
        i = static_cast<int>(std::floor(std::log(dbytes) / std::log(static_cast<double>(k))));
        if (i > MAX_BYTES_UNIT_INDEX) {
            i = MAX_BYTES_UNIT_INDEX;
        }
    }
    double val = dbytes / std::pow(static_cast<double>(k), i);
    val = RoundHalfAwayFromZero(val, DECIMAL_PRECISION);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(DECIMAL_PRECISION) << val << " " << units[i];
    return oss.str();
}

std::string FormatSizeText(double val, bool countMode)
{
    if (countMode) {
        // fd/thread 等类型的“大小”实际是个数，直接显示整数
        return std::to_string(static_cast<long long>(std::llround(val)));
    }
    return FormatBytes(val);
}

std::string FormatDuration(const std::string& durationStr)
{
    if (durationStr.empty()) {
        return "0ms";
    }
    char* end = nullptr;
    double d = std::strtod(durationStr.c_str(), &end);
    if (end == durationStr.c_str() || d == 0) {
        return "0ms";
    }
    if (d >= 1.0) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(DECIMAL_PRECISION) << RoundHalfAwayFromZero(d, DECIMAL_PRECISION) << "s";
        return oss.str();
    }
    if (d >= 1e-3) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(DECIMAL_PRECISION) <<
            RoundHalfAwayFromZero(d * 1e3, DECIMAL_PRECISION) << "ms";
        return oss.str();
    }
    if (d >= 1e-6) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(DECIMAL_PRECISION) <<
            RoundHalfAwayFromZero(d * 1e6, DECIMAL_PRECISION) << "\u03bcs";
        return oss.str();
    }
    std::ostringstream oss;
    oss << static_cast<long long>(d * 1e9) << "ns";
    return oss.str();
}

std::string EscapeHtml(const std::string& str)
{
    std::string result;
    result.reserve(str.size() * ESCAPE_RESERVE_NUMERATOR / 10);  // ~10% headroom for escape expansion
    for (char c : str) {
        switch (c) {
            case '&':
                result += "&amp;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&#39;";
                break;
            default:  result += c;
        }
    }
    return result;
}

std::string ToFixed(double value, int precision)
{
    value = RoundHalfAwayFromZero(value, precision);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

// ===== HTML building helpers =====

json BuildSymbolDataEntryJSON(SymbolDataEntry& entry)
{
    json j;
    j["info"] = entry.info;
    j["names"] = std::move(entry.names);
    j["flameTree"] = std::move(entry.flameTree);
    j["icicleTree"] = std::move(entry.icicleTree);
    json chainsArr = json::array();
    for (const auto& chain : entry.callChains) {
        json ChainJson;
        json framesArr = json::array();
        for (const auto& frame : chain.frames) {
            json f;
            f["name"] = frame.name;
            f["lib"] = frame.lib;
            f["libOffset"] = frame.libOffset;
            framesArr.push_back(f);
        }
        ChainJson["frames"] = framesArr;
        ChainJson["size"] = (long long)chain.size;
        chainsArr.push_back(ChainJson);
    }
    j["callChains"] = chainsArr;
    return j;
}

namespace {

std::vector<int> SelectKeptFrameIndices(const std::vector<CallChainFrame>& frames)
{
    std::vector<int> keptIndices;
    if (static_cast<int>(frames.size()) <= SIMPLE_CALL_CHAIN_THRESHOLD) {
        for (int fi = 0; fi < static_cast<int>(frames.size()); fi++) {
            keptIndices.push_back(fi);
        }
    } else {
        // 1. 顶层函数
        keptIndices.push_back(0);
        // 2. 底层函数
        keptIndices.push_back(static_cast<int>(frames.size()) - 1);
        for (int fi = 1; fi < static_cast<int>(frames.size()) - 1; fi++) {
            bool curIsTs = HtraceConfigMgr::Instance().IsTsSymbol(frames[fi].lib, true);
            bool nextIsTs = (fi + 1 < static_cast<int>(frames.size())) &&
                HtraceConfigMgr::Instance().IsTsSymbol(frames[fi + 1].lib, true);
            bool curIsApp = HtraceConfigMgr::Instance().IsAppSymbol(frames[fi].lib, true);
            bool nextIsApp = (fi + 1 < static_cast<int>(frames.size())) &&
                HtraceConfigMgr::Instance().IsAppSymbol(frames[fi + 1].lib, true);
            // 3. JS函数调用Native函数时的JS函数
            if (curIsTs && !nextIsTs) {
                keptIndices.push_back(fi);
            }
            // 4. 应用Native函数调用系统so时应用Native函数
            if (curIsApp && !curIsTs && !nextIsApp) {
                keptIndices.push_back(fi);
            }
        }
    }

    // Deduplicate and sort
    std::set<int> uniqueSet(keptIndices.begin(), keptIndices.end());
    keptIndices.assign(uniqueSet.begin(), uniqueSet.end());
    std::sort(keptIndices.begin(), keptIndices.end());
    return keptIndices;
}

// Build plain text for full call chain (all frames, no ellipsis)
std::string BuildChainPlainText(const std::vector<CallChainFrame>& frames, size_t chainIdx,
    double chainSize, double pct, bool countMode)
{
    std::string plainText;
    plainText += "\u8c03\u7528\u94fe" + std::to_string(chainIdx + 1) + "\uff1a\n";
    for (size_t fi = 0; fi < frames.size(); fi++) {
        if (fi > 0) {
            plainText += "\n";
        }
        plainText += FormatShortFuncName(frames[fi].name, frames[fi].lib, frames[fi].libOffset);
    }
    plainText += std::string("\n") + (countMode ? "\u6b21\u6570" : "\u5927\u5c0f") + " " +
        FormatSizeText(chainSize, countMode) + "\n\u5360\u6bd4" +
        ToFixed(pct, DECIMAL_PRECISION) + "%";
    return plainText;
}

} // namespace

std::string BuildCallChainHTML(const std::vector<CallChainResult>& callChains, double totalExistingSize, bool countMode)
{
    std::string result;
    for (size_t ci = 0; ci < callChains.size(); ci++) {
        if (ci > 0) {
            result += "\n";
        }
        const auto& chain = callChains[ci];
        double chainSize = chain.size;
        double pct = totalExistingSize > 0 ? (chainSize / totalExistingSize * 100.0) : 0.0;

        const auto& frames = chain.frames;
        std::vector<int> keptIndices = SelectKeptFrameIndices(frames);

        std::vector<std::string> parts;
        for (size_t k = 0; k < keptIndices.size(); k++) {
            if (k > 0 && keptIndices[k] - keptIndices[k - 1] > 1) {
                parts.push_back("<span class=\"chain-ellipsis\">...</span>");
            }
            int fi = keptIndices[k];
            bool isFirst = (fi == 0);
            std::string cls = isFirst ? "frame target" : "frame";
            std::string frameName = FormatShortFuncName(frames[fi].name, frames[fi].lib, frames[fi].libOffset);
            parts.push_back("<span class=\"" + cls + "\">" +
                    EscapeHtml(frameName) +
                    "</span>");
        }

        std::string joinedParts;
        for (size_t p = 0; p < parts.size(); p++) {
            if (p > 0) {
                joinedParts += "<span class=\"chain-arrow\">-&gt;</span>";
            }
            joinedParts += parts[p];
        }

        std::string plainText = BuildChainPlainText(frames, ci, chainSize, pct, countMode);

        result += "<div class=\"call-chain-line\">\u8c03\u7528\u94fe" +
            std::to_string(ci + 1) +
            "\uff1a" +
            joinedParts +
            ", " + (countMode ? "\u6b21\u6570" : "\u5927\u5c0f") + " <span class=\"chain-size\">" +
            FormatSizeText(chainSize, countMode) +
            "</span>, \u5360\u6bd4<span class=\"chain-pct\">" +
            ToFixed(pct, DECIMAL_PRECISION) + "%</span>" +
            "<span class=\"copy-chain-btn\" data-copy-chain=\"" + EscapeHtml(plainText) + "\" title=\"\u590d\u5236\u5b8c\u6574\u8c03\u7528\u94fe\">\u590d\u5236</span>" +
            "</div>";
    }
    return result;
}

std::string BuildTopTableRows(const std::vector<SymbolDataEntry>& symbols, size_t startIndex, bool countMode)
{
    // Build each row as a string, then join with \n to match TS .join('\n') behavior
    std::vector<std::string> rowStrs;
    for (size_t i = 0; i < symbols.size(); i++) {
        size_t idx = startIndex + i;

        const auto& s = symbols[i].info;
        std::string sym = s.value("name", "");
        std::string lib = s.value("lib", "");
        std::string libOffset = s.value("libOffset", "");
        std::string shortFuncName = FormatShortFuncName(sym, lib, libOffset);
        std::string fullFuncName = FormatFuncName(sym, lib, libOffset);
        std::string libDisplay = GetLibForDisplay(lib, sym);
        long long existingSize = s.value("existingSize", (long long)0);
        long long existingCount = s.value("existingCount", (long long)0);
        double percentage = s.value("percentage", 0.0);
        std::ostringstream pctSS;
        pctSS << percentage;
        std::string pctStr = pctSS.str();

        rowStrs.push_back("<tr>\n"
            "  <td><a onclick=\"showDetail(" +
            std::to_string(idx) +
            "); return false;\" class=\"detail-link\">\u8be6\u60c5</a></td>\n" +
            std::string(countMode ? "" : "  <td class=\"top-size-val\">" +
            FormatSizeText(static_cast<double>(existingSize), false) + "</td>\n") +
            "  <td class=\"top-count-val\">" + std::to_string(existingCount) + "</td>\n"
            "  <td class=\"top-pct-val\">" + pctStr + "%</td>\n"
            "  <td class=\"top-func\"><b>" + EscapeHtml(shortFuncName) + "</b></td>\n"
            "  <td class=\"top-lib\">" + EscapeHtml(libDisplay) + "</td>\n"
            "</tr>");
    }
    // Join with \n to match TS .join('\n') — no trailing \n after last element
    std::string rows;
    for (size_t i = 0; i < rowStrs.size(); i++) {
        if (i > 0) {
            rows += "\n";
        }
        rows += rowStrs[i];
    }
    return rows;
}

std::string BuildDetailHeaderHtml(const SymbolDataEntry& entry, size_t idx,
    long long totalExistingSize, bool countMode)
{
    const auto& s = entry.info;
    std::string shortName = FormatShortFuncName(
        s.value("name", ""),
        s.value("lib", ""),
        s.value("libOffset", "")
    );
    long long existingSize = s.value("existingSize", (long long)0);
    long long existingCount = s.value("existingCount", (long long)0);
    double percentage = s.value("percentage", 0.0);
    int rank = s.value("rank", static_cast<int>(idx + 1));

    std::ostringstream pctSS;
    pctSS << percentage;
    std::string pctStr = pctSS.str();

    std::string callChainLines = BuildCallChainHTML(entry.callChains, totalExistingSize, countMode);
    std::string noCallChainLine;
    if (callChainLines.empty()) {
        noCallChainLine = "<div class=\"call-chain-line\" style=\"color:#a0a0b0;\">\u65e0\u8c03\u7528\u94fe\u6570\u636e</div>";
    }

    std::string result = "<div class=\"top-item\" id=\"topItem" + std::to_string(idx) + "\">\n"
        "  <div class=\"top-item-header collapsed\" id=\"topItemHeader" + std::to_string(idx) + "\" onclick=\"toggleDetail(" + std::to_string(idx) + ")\">\n"
        "    <span class=\"arrow\" id=\"topItemArrow" + std::to_string(idx) + "\">&#9654;</span>\n"
        "    <span class=\"header-rank\">TOP" + std::to_string(rank) + "\u51fd\u6570\uff1a</span><span class=\"header-func\">" + EscapeHtml(shortName) + "</span>\n"
        "    <div class=\"header-stats\">" + (countMode ?
            "\u6b21\u6570" + std::to_string(existingCount) + "\uff0c\u5360\u6bd4" + pctStr + "%" :
            "\u5927\u5c0f " + FormatSizeText(static_cast<double>(existingSize), false) +
            "\uff0c\u6b21\u6570" + std::to_string(existingCount) +
            "\uff0c\u5360\u6bd4" + pctStr + "%") + "</div>\n"
        "  </div>\n"
        "  <div class=\"top-item-body\" id=\"topItemBody" + std::to_string(idx) + "\">\n"
        "    <div class=\"call-chains-section\">\n"
        "      " + callChainLines + "\n"
        "      " + noCallChainLine + "\n"
        "    </div>\n"
        "    <div class=\"graphToolbar\">\n"
        "      <button onclick=\"resetZoom(" + std::to_string(idx) + ")\" class=\"reset-btn\">Reset Zoom</button>\n"
        "      <input id=\"searchInput" + std::to_string(idx) +
            "\" type=\"text\" placeholder=\"\u641c\u7d22\u51fd\u6570\u540d...\" class=\"graph-search-input\">\n"
        "      <span class=\"search-match-count\" id=\"searchCount" + std::to_string(idx) + "\">0\u4e2a\u5339\u914d</span>\n"
        "      <span class=\"shortcut-hint\">w/s\uff1a\u653e\u5927/\u7f29\u5c0f&emsp;&emsp;a/d\uff1a\u5de6/\u53f3\u79fb\u52a8</span>\n"
        "    </div>\n";
    return result;
}

std::string BuildGraphContainerHtml(size_t idx, bool isOther)
{
    std::string result;
    // other: 只一个火焰图（整栈合并），无上下游之分；其余 top 函数保持上游+下游双 panel
    if (isOther) {
        result += "    <div class=\"dual-graph-container\">\n"
            "      <div class=\"graph-panel\">\n"
            "        <div class=\"graph-side-label\">\u4e0b\u6e38\u8c03\u7528\u94fe</div>\n"
            "        <div class=\"graph-canvas-area\">\n"
            "          <canvas id=\"icicleCanvas" + std::to_string(idx) + "\"></canvas>\n"
            "        </div>\n"
            "      </div>\n"
            "    </div>\n";
    } else {
        result += "    <div class=\"dual-graph-container\">\n"
            "      <div class=\"graph-panel\">\n"
            "        <div class=\"graph-side-label\">\u4e0a\u6e38\u8c03\u7528\u94fe</div>\n"
            "        <div class=\"graph-canvas-area\">\n"
            "          <canvas id=\"flameCanvas" + std::to_string(idx) + "\"></canvas>\n"
            "        </div>\n"
            "      </div>\n"
            "      <div class=\"graph-panel\">\n"
            "        <div class=\"graph-side-label\">\u4e0b\u6e38\u8c03\u7528\u94fe</div>\n"
            "        <div class=\"graph-canvas-area\">\n"
            "          <canvas id=\"icicleCanvas" + std::to_string(idx) + "\"></canvas>\n"
            "        </div>\n"
            "      </div>\n"
            "    </div>\n";
    }
    return result;
}

std::string BuildDetailSectionContent(
    const SymbolDataEntry& entry, size_t idx, long long totalExistingSize, bool addBlankLine, bool countMode)
{
    std::string result = BuildDetailHeaderHtml(entry, idx, totalExistingSize, countMode);
    bool isOther = (entry.info.value("name", "") == OTHER_SYMBOL);
    result += BuildGraphContainerHtml(idx, isOther);
    if (addBlankLine) {
        result += "\n"
            "  </div>\n"
            "</div>";
    } else {
        result += "  </div>\n"
            "</div>";
    }
    return result;
}

std::string BuildDetailSections(const std::vector<SymbolDataEntry>& symbols,
    size_t startIndex,
    long long totalExistingSize,
    bool countMode)
{
    std::vector<std::string> sectionStrs;
    for (size_t i = 0; i < symbols.size(); i++) {
        sectionStrs.push_back(
            BuildDetailSectionContent(symbols[i], startIndex + i, totalExistingSize, true, countMode));
    }
    std::string sections;
    for (size_t i = 0; i < sectionStrs.size(); i++) {
        if (i > 0) {
            sections += "\n";
        }
        sections += sectionStrs[i];
    }
    return sections;
}

// ===== HTML template =====
// Shared HTML template with customizable __TITLE__ and __META_ITEMS_HTML__ placeholders.
// Other placeholders: __DATA_JSON__, __TOTAL_SIZE__, __TAB_BAR_HTML__, __TAB_CONTENT_HTML__

const std::string HTML_TEMPLATE = R"HTMLTMPL(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<title>__TITLE__</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', sans-serif; background: #fff; color: #333; line-height: 1.6; font-size: 16px; }

/* Report title */
.report-title { text-align: center; font-size: 24px; color: #e94560; font-weight: 600; letter-spacing: 2px; padding: 18px 0; }

/* Meta card */
.meta-card { background: #f8f9fa; border: 1px solid #dee2e6; border-radius: 8px; padding: 16px 24px; margin: 20px; }
.meta-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 0; }
.meta-item { padding: 10px 12px; border-bottom: 1px solid #dee2e6; }
.meta-item:nth-child(n+4) { border-bottom: none; }
.meta-label { color: #333; font-size: 16px; font-weight: bold; }
.meta-value { color: #333; font-size: 16px; }


/* Tab bar */
.tab-bar { display: flex; gap: 0; margin: 20px 20px 0; border-bottom: 2px solid #dee2e6; }
.tab-item { display: inline-block; padding: 10px 4px 8px; margin-right: 12px; cursor: pointer; font-size: 16px; font-weight: 600; color: #888; border-bottom: 2px solid transparent; margin-bottom: -2px; transition: color 0.2s, border-color 0.2s; }
.tab-item:hover { color: #e94560; }
.tab-item.active { color: #e94560; border-bottom: 2px solid #e94560; }

/* Tab content */
.tab-content { margin: 0 20px; display: none; }
.tab-content.active { display: block; }

/* TOP table */
.top-table { width: calc(100% - 4px); border-collapse: collapse; margin-bottom: 16px; margin-left: 4px; margin-top: 20px; }
.top-table th { background: #f0f0f0; color: #333; font-size: 15px; padding: 10px 12px; text-align: left; border: 1px solid #dee2e6; }
.top-table td { padding: 10px 12px; border: 1px solid #dee2e6; font-size: 15px; }
.top-func { word-break: break-word; color: #333; font-weight: bold; }
.top-lib { word-break: break-word; color: #888; font-size: 14px; }
.top-size-val { color: #333; }
.top-count-val { color: #333; }
.top-pct-val { color: #e94560; }
.detail-link { color: #1a73e8; cursor: pointer; text-decoration: underline; white-space: nowrap; }
.detail-link:hover { color: #4285f4; }

/* TOP item detail (accordion) */
.top-item { background: #fff; border: 1px solid #dee2e6; border-radius: 6px; margin: 12px 0; overflow: hidden; }
.top-item-header { padding: 12px 20px; cursor: pointer; display: block; font-size: 17px; color: #333; font-weight: 500; transition: background 0.2s ease; }
.top-item-header:hover { background: #f0f0f0; }
.top-item-header.collapsed { border-bottom: none; }
.top-item-header.expanded { border-bottom: 1px solid #dee2e6; }
.top-item-header .arrow { margin-right: 10px; font-size: 14px; color: #999; transition: transform 0.3s ease; }
.top-item-header.expanded .arrow { transform: rotate(90deg); }
.top-item-header .header-rank { font-weight: 600; }
.top-item-header .header-func { font-weight: bold; word-break: break-word; }
.top-item-header .header-stats { font-size: 14px; color: #888; margin-top: 4px; }

.top-item-body { overflow: hidden; max-height: 0; opacity: 0; padding: 0 20px; transition: max-height 0.4s ease, opacity 0.3s ease, padding 0.3s ease; }
.top-item-body.expanded { opacity: 1; padding: 20px; max-height: none; }

/* Call chains */
.call-chains-section { margin-bottom: 20px; }
.call-chain-line { padding: 8px 0; font-size: 15px; color: #333; border-bottom: 1px solid #dee2e6; line-height: 1.8; }
.call-chain-line:last-child { border-bottom: none; }
.call-chain-line .frame { color: #333; }
.call-chain-line .frame.target { color: #333; }
.call-chain-line .chain-arrow { display: inline-block; white-space: nowrap; color: #333; margin: 0 4px; }
.call-chain-line .chain-size { color: #333; }
.call-chain-line .chain-pct { color: #e94560; }
.call-chain-line .chain-ellipsis { color: #333; margin: 0 2px; }
.copy-chain-btn { display: inline-block; margin-left: 8px; padding: 2px 8px; font-size: 12px; color: #1a73e8; border: 1px solid #1a73e8; border-radius: 3px; cursor: pointer; vertical-align: middle; background: #fff; user-select: none; }
.copy-chain-btn:hover { background: #e8f0fe; }
.copy-chain-btn.copied { color: #34a853; border-color: #34a853; }

/* Graph area */
__CHART_MODULE_CSS__
.dual-graph-container { display: block; margin-bottom: 8px; }
.graph-panel { width: 100%; position: relative; display: flex; margin-bottom: 8px; }
.graph-side-label { width: 28px; flex-shrink: 0; display: flex; align-items: center; justify-content: center; writing-mode: vertical-rl; text-orientation: mixed; font-size: 14px; color: #888; background: #f8f9fa; border-right: 1px solid #dee2e6; border-radius: 4px 0 0 4px; }
.graph-canvas-area { flex: 1; min-width: 0; overflow: hidden; }
.graph-canvas-area canvas { display: block; width: 100%; cursor: pointer; background: #fff; border: 1px solid #dee2e6; border-radius: 4px; }

/* Empty message */
.empty-msg { color: #888; font-size: 17px; text-align: center; padding: 30px; }

/* Footer */
__VERSION_FORMAT__

__NAV_BAR_FORMAT__

</style>
</head>
<body>

<!-- Title bar -->
<h1 class="report-title">资源泄漏日志分析报告</h1>

<!-- Navigation bar -->
__NAV_BAR_TITLE__

<div id="navReport" class="nav-content active">
<!-- Meta card -->
__META_ITEMS_HTML__

<!-- Tab analysis area -->
__TAB_BAR_HTML__
__TAB_CONTENT_HTML__

<!-- Footer -->
__VERSION_CONTENT__
</div>

<div id="navDoc" class="nav-content">
__NAV_BAR_DOC_CONTENT__
</div>

<div id="navHelp" class="nav-content">
__NAV_BAR_HELP_CENTER_CONTENT__
</div>

<script>
var allData = __DATA_JSON__;
__CHART_MODULE_JS__
</script>
<script>
(function() {
  var autoExpand = __AUTO_EXPAND__;

__NAV_BAR_SWITCH_NAV_FUNC__

  // ===== 手风琴：展开/收起 =====
  function toggleDetail(idx) {
    var body = document.getElementById('topItemBody' + idx);
    var header = document.getElementById('topItemHeader' + idx);
    if (!body) return;

    if (body.classList.contains('expanded')) {
      body.classList.remove('expanded');
      header.classList.remove('expanded');
      header.classList.add('collapsed');
    } else {
      showDetail(idx);
    }
  }

  function showDetail(idx) {
    var body = document.getElementById('topItemBody' + idx);
    var header = document.getElementById('topItemHeader' + idx);
    if (!body) return;

    body.classList.add('expanded');
    header.classList.remove('collapsed');
    header.classList.add('expanded');

    // 展开后画布宽度才有效，重绘对应图表
    setTimeout(function() { MemChart.redraw(idx); }, 50);

    var item = document.getElementById('topItem' + idx);
    if (item) item.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }

  // Expose to HTML onclick handlers
  window.toggleDetail = toggleDetail;
  window.showDetail = showDetail;

  // ===== Tab switching =====
  function switchTab(tabId, tabEl) {
    var tabs = document.querySelectorAll('.tab-item');
    for (var i = 0; i < tabs.length; i++) tabs[i].classList.remove('active');
    if (tabEl) tabEl.classList.add('active');
    var contents = document.querySelectorAll('.tab-content');
    for (var i = 0; i < contents.length; i++) contents[i].classList.remove('active');
    var content = document.getElementById(tabId);
    if (content) content.classList.add('active');
    MemChart.redrawAll();
  }
  window.switchTab = switchTab;

  // Copy chain button click
  document.addEventListener('click', function(e) {
    var btn = e.target.closest('.copy-chain-btn');
    if (!btn) return;
    var text = btn.getAttribute('data-copy-chain');
    if (!text) return;
    navigator.clipboard.writeText(text).then(function() {
      btn.textContent = '已拷贝';
      btn.classList.add('copied');
      setTimeout(function() { btn.textContent = '复制'; btn.classList.remove('copied'); }, 1500);
    });
  });

  // ===== 初始化：激活首个页签 =====
  // 页签按钮默认带 active，但其内容容器（.tab-content）默认 display:none，
  // 必须同步激活，否则首屏正文空白，且图表在隐藏容器里量不到画布尺寸。
  var firstTabItem = document.querySelector('.tab-item.active');
  var firstTabContent = firstTabItem ? document.getElementById(firstTabItem.getAttribute('data-tab')) : null;
  if (firstTabContent) firstTabContent.classList.add('active');

  // ===== 初始化图表（共享图表模块） =====
  MemChart.init({ entries: allData, unitCountMode: __UNIT_COUNT_MODE__ });

  // Set initial arrow for expanded headers
  var allHeaders = document.querySelectorAll('.top-item-header.expanded');
  for (var h = 0; h < allHeaders.length; h++) {
    var arr = allHeaders[h].querySelector('.arrow');
    if (arr) arr.textContent = '\u25BC';
  }

  // Auto-expand all details
  if (autoExpand) {
    function expandNextAuto(idx) {
      if (idx >= allData.length) return;
      var body = document.getElementById('topItemBody' + idx);
      var header = document.getElementById('topItemHeader' + idx);
      if (!body || !header) { expandNextAuto(idx + 1); return; }

      body.classList.add('expanded');
      header.classList.remove('collapsed');
      header.classList.add('expanded');

      setTimeout(function() {
        MemChart.redraw(idx);
        expandNextAuto(idx + 1);
      }, 50);
    }
    expandNextAuto(0);
  }
})();
</script>
</body>
</html>)HTMLTMPL";

} // namespace mem_analyzer::html_analysis
