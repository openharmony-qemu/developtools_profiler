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
#include "tshandle/formatters/tshandle_html_formatter.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include "formatter/html/analysis_html_formatter_base.hpp"
#include "formatter/html/html_analysis_common.hpp"
#include "tshandle/formatters/tshandle_call_graph.hpp"
#include "tshandle/tshandle_processor.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace mem_analyzer::html_analysis;

namespace mem_analyzer {
namespace {

std::string format_timestamp(std::string value) {
    if (value.empty()) return "-";
    const size_t t = value.find('T');
    if (t != std::string::npos) value[t] = ' ';
    const size_t dot = value.find('.');
    if (dot != std::string::npos) {
        size_t end = dot + 1;
        while (end < value.size() && value[end] >= '0' && value[end] <= '9') ++end;
        value.erase(dot, end - dot);
    }
    if (!value.empty() && value.back() == 'Z') value.pop_back();
    return value;
}

std::string format_tshandle_duration(long long nanoseconds) {
    if (nanoseconds <= 0) return "-";
    return toFixed(static_cast<double>(nanoseconds) / 1000000000.0, 2) + "s";
}

std::vector<TshandleCallChain> parse_chains(const TshandleTopObject& object, double denominator, size_t max_chains = 3) {
    std::vector<TshandleCallChain> chains;
    for (const auto& src : object.call_chains) {
        TshandleCallChain chain = src;
        chain.percentage = denominator > 0.0 ? (chain.existing_size / denominator * 100.0) : 0.0;
        chains.push_back(std::move(chain));
        if (max_chains > 0 && chains.size() >= max_chains) break;
    }
    return chains;
}

json chain_json(const TshandleCallChain& chain) {
    json frames = json::array();
    for (const auto& frame : chain.frames) {
        frames.push_back({{"depth", frame.depth}, {"symbol", frame.symbol}, {"buildId", frame.build_id},
                          {"lib", frame.lib}, {"libOffset", frame.lib_offset}});
    }
    return {{"callStackId", chain.call_stack_id},
            {"existingSize", chain.existing_size},
            {"count", chain.count},
            {"percentage", chain.percentage},
            {"frames", std::move(frames)}};
}

std::vector<CallChainResult> display_chains(
    const std::vector<TshandleCallChain>& chains) {
    std::vector<CallChainResult> output;
    output.reserve(chains.size());
    for (const auto& chain : chains) {
        CallChainResult display;
        display.size = chain.existing_size;
        display.count = chain.count;
        for (const auto& frame : chain.frames) {
            display.frames.push_back({frame.symbol, frame.lib, frame.lib_offset});
        }
        output.push_back(std::move(display));
    }
    return output;
}

std::string build_meta_html(const std::string& source_file, const std::string& parsed_at,
                            const TraceMeta& trace_meta,
                            const TshandleHandleSections& global, const TshandleTypeSection& local) {
    std::string name = platform::basename(source_file);
    if (name.empty()) name = "-";
    const std::string timestamp = format_timestamp(parsed_at);
    const std::string process = trace_meta.process_name.empty() ? "-" : trace_meta.process_name;
    const std::string duration = format_tshandle_duration(trace_meta.duration_ns);
    const double total_size = global.app.total_size + global.sys.total_size + local.total_size;

    return "<div class=\"meta-card\">\n"
        "  <div class=\"meta-grid\">\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">日志名：</span><span class=\"meta-value\">" + escapeHtml(name) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">类型：</span><span class=\"meta-value\">tshandle</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">时间：</span><span class=\"meta-value\">" + escapeHtml(timestamp) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">总内存：</span><span class=\"meta-value\">" + formatBytes(total_size) + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">时长：</span><span class=\"meta-value\">" + duration + "</span></div>\n"
        "    <div class=\"meta-item\"><span class=\"meta-label\">进程：</span><span class=\"meta-value\">" + escapeHtml(process.empty() ? "-" : process) + "</span></div>\n"
        "  </div>\n"
        "</div>";
}

struct SectionResult {
    std::string tab_id;
    std::string tab_label;
    HtraceEventType handle_type = HtraceEventType::UNKNOWN;
    std::string content;
};

SectionResult build_section(const char* tab_id, const char* label,
                            HtraceEventType handle_type, const TshandleTypeSection& section,
                            double denominator,
                            json& all_data, size_t& graph_index) {
    SectionResult result{tab_id, label, handle_type, ""};
    
    const auto& objects = section.top_objects;
    // 如果完全没有抓到该类型数据，或者没有在堆快照中关联到 ArkTS 对象
    // 则直接在 Tab 内容区提示无数据
    if (!section.has_data || objects.empty()) {
        result.content = "<div class=\"empty-msg\">无分析数据</div>";
        return result;
    }

    const long long total_objects = section.total_objects;
    const double total_size = section.total_size;

    const char summary_prefix[] = "\xE6\x9C\x89\xE5\xA0\x86\xE6\xA0\x88 ";
    const char summary_mid[] = " \xE5\xAF\xB9\xE8\xB1\xA1\xE6\x80\xBB\xE6\x95\xB0\xE9\x87\x8F\xEF\xBC\x9A";
    const char summary_suffix[] = " \xE4\xB8\xAA\xEF\xBC\x8C\xE6\x80\xBB\xE5\xA4\xA7\xE5\xB0\x8F\xEF\xBC\x9A";

    std::ostringstream out;
    
    out << "<table class=\"top-table\">\n"
        "  <tr><th>详情</th><th>大小</th><th>数量</th><th>占比</th><th>TOP 对象名</th></tr>\n";
    size_t row_index = graph_index;
    for (const auto& object : objects) {
        out << "  <tr>\n"
            "    <td><a onclick=\"showDetail(" << row_index
            << "); return false;\" class=\"detail-link\">详情</a></td>\n"
            "    <td class=\"top-size-val\">" << formatBytes(object.existing_size) << "</td>\n"
            "    <td>" << object.count << "</td>\n"
            "    <td class=\"top-pct-val\">" << toFixed(std::min(100.0, object.percentage), 2) << "%</td>\n"
            "    <td class=\"top-func\">" << escapeHtml(object.object_name) << "</td>\n"
            "  </tr>\n";
        ++row_index;
    }
    out << "</table>\n";

    for (const auto& object : objects) {
        const std::string& object_name = object.object_name;
        const auto all_chains = parse_chains(object, denominator, 0);
        const auto tree = build_tshandle_downstream_graph(object_name, all_chains, object.existing_size);
        const auto chains = parse_chains(object, denominator, 3);
        const auto chain_entries = display_chains(chains);
        const std::string chain_html = buildCallChainHTML(chain_entries, denominator);

        json encoded_chains = json::array();
        for (const auto& chain : chains) encoded_chains.push_back(chain_json(chain));
        json info = {
            {"rank", object.rank},
            {"objectName", object_name},
            {"existingSize", object.existing_size},
            {"count", object.count},
            {"percentage", std::min(100.0, object.percentage)},
            {"type", htrace_event_type_name(object.type)}
        };
        all_data.push_back({
            {"info", std::move(info)},
            {"downstreamTree", tree.to_json()},
            {"callChains", std::move(encoded_chains)}
        });

        out << "<div class=\"top-item\" id=\"topItem" << graph_index << "\">\n"
            "  <div class=\"top-item-header expanded\" id=\"topItemHeader" << graph_index
            << "\" onclick=\"toggleDetail(" << graph_index << ")\">\n"
            "    <span class=\"arrow\">&#9654;</span>\n"
            "    <span class=\"header-rank\">TOP" << object.rank
            << "对象：</span><span class=\"header-func\">" << escapeHtml(object_name) << "</span>\n"
            "    <div class=\"header-stats\">大小 " << formatBytes(object.existing_size)
            << "，数量" << object.count << "，占比"
            << toFixed(std::min(100.0, object.percentage), 2) << "%</div>\n"
            "  </div>\n"
            "  <div class=\"top-item-body expanded\" id=\"topItemBody" << graph_index << "\">\n"
            "    <div class=\"call-chains-section\">\n      "
            << (chain_html.empty()
                ? "<div class=\"call-chain-line\" style=\"color:#a0a0b0;\">无调用链数据</div>"
                : chain_html)
            << "\n    </div>\n"
            "    <div class=\"graphToolbar\">\n"
            "      <button onclick=\"resetZoom(" << graph_index << ")\" class=\"reset-btn\">Reset Zoom</button>\n"
            "      <input id=\"searchInput" << graph_index
            << "\" type=\"text\" placeholder=\"搜索对象名...\" class=\"graph-search-input\">\n"
            "      <span class=\"search-match-count\" id=\"searchCount" << graph_index << "\">0个匹配</span>\n"
            "      <span class=\"shortcut-hint\">w/s：放大/缩小&emsp;&emsp;a/d：左/右移动</span>\n"
            "    </div>\n"
            "    <div class=\"dual-graph-container\">\n"
            "      <div class=\"graph-panel\">\n"
            "        <div class=\"graph-side-label\">下游调用链</div>\n"
            "        <div class=\"graph-canvas-area\"><canvas tabindex=\"0\" id=\"downstreamCanvas"
            << graph_index << "\"></canvas></div>\n"
            "      </div>\n"
            "    </div>\n"
            "  </div>\n"
            "</div>\n";
        ++graph_index;
    }
    result.content = out.str();
    return result;
}

std::string build_tab_bar(const std::vector<SectionResult>& sections) {
    std::string output = "<div class=\"tab-bar\">\n";
    bool first = true;
    for (const auto& section : sections) {
        if (section.content.empty()) continue;
        output += "  <div class=\"tab-item" + std::string(first ? " active" : "")
            + "\" data-tab=\"" + section.tab_id
            + "\" onclick=\"switchTab('" + section.tab_id + "', this)\">"
            + section.tab_label + "</div>\n";
        first = false;
    }
    output += "</div>";
    if (first) return "<div class=\"empty-msg\">无分析数据</div>";
    return output;
}

std::string escape_embedded_data(std::string page) {
    const std::string marker = "var allData = ";
    const size_t begin = page.find(marker);
    if (begin == std::string::npos) return page;
    const size_t value_begin = begin + marker.size();
    const size_t end = page.find(';', value_begin);
    if (end == std::string::npos) return page;
    std::string encoded = page.substr(value_begin, end - value_begin);
    encoded = replace_all(encoded, "<", "\\u003c");
    encoded = replace_all(encoded, ">", "\\u003e");
    encoded = replace_all(encoded, "&", "\\u0026");
    const std::string line_separator({
        static_cast<char>(0xE2), static_cast<char>(0x80), static_cast<char>(0xA8)});
    const std::string paragraph_separator({
        static_cast<char>(0xE2), static_cast<char>(0x80), static_cast<char>(0xA9)});
    encoded = replace_all(encoded, line_separator, "\\u2028");
    encoded = replace_all(encoded, paragraph_separator, "\\u2029");
    page.replace(value_begin, end - value_begin, encoded);
    return page;
}

std::string build_tab_content(const std::vector<SectionResult>& sections) {
    std::string output;
    bool first = true;
    for (const auto& section : sections) {
        if (section.content.empty()) continue;
        output += "<div class=\"tab-content" + std::string(first ? " active" : "")
            + "\" id=\"" + section.tab_id + "\">\n"
            + section.content + "\n</div>\n";
        first = false;
    }
    return output;
}

void format_with_stream(std::ostream& out, const TshandleAnalysisOutput& analysis,
                        const std::string& file_path) {
    auto& htrace_cfg = HtraceConfigMgr::instance();
    if (!htrace_cfg.is_loaded()) {
        std::string cfg_err;
        htrace_cfg.load_default(cfg_err);
    }

    json all_data = json::array();
    size_t graph_index = 0;
    std::vector<SectionResult> sections;

    const double global_total = analysis.global.app.total_size + analysis.global.sys.total_size;
    sections.push_back(build_section("tabGlobalApp", "TOP GlobalHandle（应用栈）引用ArkTS对象分析报告",
                                     HtraceEventType::ARK_GLOBAL_HANDLE, analysis.global.app,
                                     global_total, all_data, graph_index));
    sections.push_back(build_section("tabGlobalSys", "TOP GlobalHandle（系统栈）引用ArkTS对象分析报告",
                                     HtraceEventType::ARK_GLOBAL_HANDLE, analysis.global.sys,
                                     global_total, all_data, graph_index));
    sections.push_back(build_section("tabLocal", "TOP LocalHandle 引用 ArkTS 对象分析报告",
                                     HtraceEventType::ARK_LOCAL_HANDLE, analysis.local,
                                     analysis.local.total_size, all_data, graph_index));

    const double total_size = analysis.global.app.total_size + analysis.global.sys.total_size
                            + analysis.local.total_size;
    const long long total_for_template = total_size <= static_cast<double>(std::numeric_limits<long long>::max())
        ? static_cast<long long>(total_size) : 0;

    const std::string title = "资源泄漏日志分析报告";

    std::ostringstream page;
    AnalysisHtmlFormatterBase::fillHTMLTemplateStreamed(
        page, all_data, total_for_template, build_tab_bar(sections), build_tab_content(sections),
        build_meta_html(file_path, analysis.parsed_at, analysis.trace_meta, analysis.global,
                        analysis.local),
        escapeHtml(title));
    std::string html = page.str();

    html = replace_all(html,
        "</style>",
        ".arrow { transform: none !important; }\n"
        "</style>");
    html = replace_all(html,
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
    out << escape_embedded_data(std::move(html));
}

} // namespace

std::string tshandle_write_analysis_html(const TshandleAnalysisOutput& analysis,
                                         const std::string& file_path, const RunContext& ctx,
                                         const std::string& out_dir) {
    (void)ctx;  // 当前 tshandle HTML 不使用运行参数
    StageTimer timer("HTML write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".html");
    platform::create_directories(out_dir);

    auto ofs = open_ofstream(out_path);
    if (!ofs) {
        log::loge("Error: Cannot write to ", out_path);
        return {};
    }

    format_with_stream(ofs, analysis, file_path);

    ofs.close();
    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer
