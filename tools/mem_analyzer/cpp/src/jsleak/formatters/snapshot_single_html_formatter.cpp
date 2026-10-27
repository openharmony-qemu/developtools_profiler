#include "jsleak/formatters/snapshot_single_html_formatter.hpp"
#include "jsleak/formatters/snapshot_single_html_body.hpp"
#include "jsleak/formatters/snapshot_single_html_js.hpp"
#include "jsleak/formatters/icicle_tree.hpp"
#include "formatter/html/html_analysis_common.hpp"
#include "formatter/html/components/chart_module.hpp"
#include "formatter/html/components/help_tooltip.hpp"
#include "formatter/html/components/navigation_bar.hpp"
#include "jsleak/formatters/report_common.hpp"
#include "jsleak/formatters/tree_table.hpp"
#include "formatter/html/report_skeleton.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"
#include "utils/timer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>

namespace mem_analyzer {
using html_analysis::escapeHtml;
using html_analysis::formatBytes;
using html_analysis::formatSizeText;

namespace {

// 单文件报告专属覆盖与扩展（多快照对比报告不引用；二级页签/冰柱图容器见 report_common_sub_tab_css）。
std::string snapshot_html_css() {
    return R"CSS(
.meta-grid { grid-template-columns: 1fr 1fr 1fr; }
.meta-item:nth-child(n+4) { border-bottom: none; }
.top-table th { text-align: left; }
.top-table td { text-align: left; }
.top-table { margin-bottom: 16px; margin-top: 20px; }
.tab-bar { margin: 20px 20px 0; }
.tab-item { display: inline-block; padding: 10px 16px 8px; margin-right: 8px; cursor: pointer; font-size: 16px; font-weight: 600; color: #888; border-bottom: 3px solid transparent; margin-bottom: -2px; transition: color 0.2s, border-color 0.2s; }
.tab-item:hover { color: #e94560; }
.tab-item.active { color: #e94560; border-bottom-color: #e94560; }

.tab-content { margin: 0 20px; }
)CSS";
}

std::string generateSnapshotHTML(const json& data,
                                 const std::string& sourceFile,
                                 const std::string& jsleakMode) {
    std::string escapedSourceFile = escapeHtml(platform::basename(sourceFile));
    std::string fileType = ends_with_ignore_case(sourceFile, "rawheap") ? "rawheap" : "heapsnapshot";
    std::string parsedAt = data.value("parsedAt", "");
    int64_t totalHeapSize = data.value("totalHeapSize", (int64_t)0);
    std::string totalMem = formatBytes((double)totalHeapSize);

    const bool hasPropertyTag = data.contains("propertyTagObjects") && !data["propertyTagObjects"].empty();
    const bool hasTop  = data.contains("topRefChains") && !data["topRefChains"].empty();
    const bool hasNapiWrap = data.contains("napiWrapRefChains") && !data["napiWrapRefChains"].empty();
    // top-chain 模式下不渲染最短引用链报告（与原行为一致）
    const bool showTopTab = hasTop && jsleakMode != "top-chain";

    // tab 列表驱动渲染：新增报告类型时在列表中追加一项即可，
    // 不再用 if-else 铺组合、也不再用字符串 find 打补丁。
    struct TabDesc {
        std::string id;
        std::string label;
        std::string content;
        bool active;
        std::string tooltip; // 可选：tab 标题末尾的 ? 帮助图标说明
    };
    std::vector<TabDesc> tabs;
    // 收集各 tab 的冰柱图数据（id + 前缀合并后的引用链树），最后统一注入页面脚本
    json icicleCharts = json::array();

    // ArkUI组件/ArkTS对象泄漏报告：有泄漏数据时渲染并激活；
    // 无泄漏、但有最短引用链时补一个空 tab 作为入口（此时非激活）。
    if (hasPropertyTag || showTopTab) {
        tabs.push_back({"tabPropertyTag",
                        "ArkUI组件/ArkTS对象泄漏分析报告",
                        build_property_tag_tab_html(hasPropertyTag ? data["propertyTagObjects"] : json::object(),
                                                hasPropertyTag ? (hasTop ? "lc" : "") : "",
                                                "tabPropertyTag", hasPropertyTag, icicleCharts),
                        hasPropertyTag,
                        "若ArkUI组件/ArkTS对象的生命周期已结束（已执行aboutToDisappear、dispose、destroy等函数），无法且不应该被复用；但在快照导出触发GC后仍未被回收，则认为大概率存在泄露。\n对此类对象进行引用链聚类分析，得到的每条引用链通常对应一条泄露路径。此类泄露分析准确率高达90%+。"});
    }
    // 快照最短引用链报告
    if (showTopTab) {
        tabs.push_back({"tabTopchain",
                        "快照最短引用链分析报告",
                        build_top_chain_tab_html(data["topRefChains"],
                                             (hasPropertyTag && hasTop) ? "tc" : "",
                                             "tabTopchain", !hasPropertyTag, icicleCharts, true),
                        !hasPropertyTag});
    }
    // JSWrappedNapiObject 节点引用链 tab（仅 -m all 且开启 napi_wrap 预览时产出）
    if (hasNapiWrap) {
        const bool active = tabs.empty();
        // 该 tab 的引用链都以 JSWrappedNapiObject 结尾，行级搜索词只能命中同一个名字，
        // 因此「查看」不带搜索词，直接跳到完整总览图（见 buildTopChainTableRow）
        tabs.push_back({"tabNapiWrap",
                        "绑定了Native内存的ArkTS对象引用链分析报告",
                        build_top_chain_tab_html(data["napiWrapRefChains"], "np", "tabNapiWrap", active,
                                             icicleCharts, false),
                        active});
    }

    std::string tabItems;
    std::string tabContents;

    if (tabs.empty()) {
        tabItems = "<span class=\"tab-item active\" data-tab=\"tabMain\" onclick=\"switchTab('tabMain', this)\">数据分析报告</span>";
        tabContents =
            "<div class=\"tab-content\" id=\"tabMain\" data-tab=\"tabMain\" style=\"display:block;\">"
            "<div class=\"empty-msg\">无数据</div></div>";
    } else {
        for (const auto& t : tabs) {
            tabItems += "<span class=\"tab-item" + std::string(t.active ? " active" : "") +
                        "\" data-tab=\"" + t.id + "\" onclick=\"switchTab('" + t.id + "', this)\">" +
                        t.label;
            if (!t.tooltip.empty()) {
                tabItems += "<span class=\"help-icon\" data-tooltip=\"" + t.tooltip + "\">?</span>";
            }
            tabItems += "</span>";
        }
        for (const auto& t : tabs) {
            tabContents += t.content;
        }
    }

    // ========== 用 report_skeleton 组装页面 ==========
    std::ostringstream html;
    html << html_report_begin("资源泄漏日志分析报告");
    html << report_common_css();        // jsleak 域公共基础样式（单快照 + compare 共用）
    html << snapshot_html_css();        // 单文件报告专属覆盖 CSS
    html << report_common_sub_tab_css();      // 二级页签 + 冰柱图容器 CSS（与多快照共用）
    html << chart_module_css();         // 可选组件：图引擎
    html << tree_table_css();           // 可选组件：引用链树表
    html << html_report_css_end();      // 自动注入 version/help_tooltip/nav_bar CSS
    html << html_report_title("资源泄漏日志分析报告");
    html << html_report_body_open();
    html << html_report_meta({
        {"日志名", escapedSourceFile},
        {"类型", fileType},
        {"时间", escapeHtml(parsedAt)},
        {"总内存", totalMem},
        {"时长", "未知"},
        {"进程", "未知"},
    });
    html << "<div class=\"tab-bar\">" << tabItems << "</div>"
         << "<div class=\"tab-area\">" << tabContents << "</div>";
    html << html_report_body_close();
    html << html_report_core_script();
    html << html_report_script(report_common_js());   // jsleak 域公共交互脚本
    html << html_report_script(snapshot_html_js());
    html << html_report_script(report_common_sub_tab_js());
    html << html_report_script("window.ICICLE_CHARTS_DATA = " +
                               html_escape_script_json(icicleCharts.dump()) + ";");
    html << html_report_script(chart_module_js());
    html << html_report_script(report_common_chart_init_js());
    html << html_report_end();
    return html.str();
}

} // namespace

std::string jsleak_write_analysis_html(const json& data, const std::string& file_path,
                                       const RunContext& ctx, const std::string& out_dir) {
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

    ofs << generateSnapshotHTML(data, file_path, ctx.snapshot.mode);

    ofs.close();
    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer
