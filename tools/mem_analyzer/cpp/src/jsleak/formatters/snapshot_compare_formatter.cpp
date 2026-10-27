#include "jsleak/formatters/snapshot_compare_formatter.hpp"
#include "formatter/html/report_skeleton.hpp"
#include "formatter/html/components/chart_module.hpp"
#include "jsleak/formatters/tree_table.hpp"
#include "jsleak/formatters/report_common.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"

#include <fstream>
#include <sstream>

namespace mem_analyzer {

namespace {

std::string compare_render_js() {
    return R"JS(
const $ = s => document.querySelector(s);

$('#baselineFile').textContent = D.baseline.file;
$('#targetFile').textContent = D.target.file;
$('#analysisType').textContent = D.analysisType || '未知';
$('#generatedAt').textContent = D.generatedAt || '未知';
$('#duration').textContent = D.duration || '未知';
$('#processName').textContent = D.process || '未知';
$('#heapSizeDelta').textContent = bytes(D.summary.deltaHeapSize);

let mode = 'positive';
const paths = D.pathDeltas;

function renderPaths() {
  const list = paths.filter(x => mode === 'positive'
    ? (x.change === 'new' || x.change === 'growth' || x.change === 'replaced')
    : (x.change === 'removed' || x.change === 'shrink'));
  if (!list.length) {
    $('#paths').innerHTML = '<div class="empty-msg">当前筛选下没有差异。</div>';
    return;
  }
  const rows = list.map((x, i) => `<tr><td><a href="#pathDetail_${i}" onclick="showDetail('path',${i});return false" class="detail-link">详情</a></td><td><a href="#" data-chart-subtab="subtabChart_pathDeltas" data-keyword="${esc(x.objectName)}" onclick="showInOverview(this);return false" class="detail-link">查看</a></td><td class="${x.retainedSizeDelta>=0?'positive':'negative'}"><b>${bytes(x.retainedSizeDelta)}</b></td><td>${delta(x.nodeCountDelta)} 个</td><td><b>${chainHtml(x.path)}</b></td></tr>`).join('');
  const details = list.map((x, i) => {
    const source = x.pathSnapshot || (x.change === 'removed' || x.change === 'shrink' ? 'A' : 'B');
    const nodeLabel = source === 'A' ? '消失节点 ID' : '新增节点 ID';
    return `<div class="top-item" id="pathDetail_${i}"><div class="top-item-header expanded" id="pathHeader_${i}" onclick="toggleDetail('path',${i})"><span class="arrow">▶</span><span class="header-rank">TOP${i+1}引用链：</span><span class="header-func">${chainHtml(x.path)}</span><div class="header-stats">总大小 <span class="${x.retainedSizeDelta>=0?'positive':'negative'}">${bytes(x.retainedSizeDelta)}</span>，节点数 ${delta(x.nodeCountDelta)}</div></div><div class="top-item-body expanded" id="pathBody_${i}"><div class="call-chains-section"><div class="call-chain-line">引用链: ${chainHtml(x.path)}</div>${nodeIdsHtml(x.deltaNodeIds||[],`pathNodeIds_${i}`,nodeLabel)}</div></div></div>`;
  }).join('');
  $('#paths').innerHTML = `<div class="table-wrap"><table class="top-table path-table"><thead><tr><th>详情</th><th>总览图</th><th>总大小</th><th>节点数</th><th>引用链名称</th></tr></thead><tbody>${rows}</tbody></table></div>${details}`;
}
document.querySelectorAll('.filter').forEach(b => {
  b.onclick = () => {
    document.querySelectorAll('.filter').forEach(x => x.classList.remove('active'));
    b.classList.add('active');
    mode = b.dataset.kind;
    renderPaths();
  };
});
renderPaths();

document.querySelectorAll('.analysis-tab').forEach(tab => {
  tab.onclick = () => {
    document.querySelectorAll('.analysis-tab').forEach(x => {
      x.classList.remove('active');
      x.setAttribute('aria-selected', 'false');
    });
    document.querySelectorAll('.analysis-panel').forEach(x => x.classList.remove('active'));
    tab.classList.add('active');
    tab.setAttribute('aria-selected', 'true');
    document.getElementById(tab.dataset.panel).classList.add('active');
    if (window.icicleRedrawAll) window.icicleRedrawAll();
  };
});
window.ICICLE_CHARTS_DATA = D.icicleCharts || [];

const leakEntries = Object.entries(D.leakObjInfo || {}).sort((a, b) =>
  Math.abs(b[1].retainedSizeDelta || 0) - Math.abs(a[1].retainedSizeDelta || 0));
const leakRows = leakEntries.map(([name, x], i) => `<tr><td><a href="#leakDetail_${i}" onclick="showDetail('leak',${i});return false" class="detail-link">详情</a></td><td><a href="#" data-chart-subtab="subtabChart_leakObj" data-keyword="${esc(name)}" onclick="showInOverview(this);return false" class="detail-link">查看</a></td><td class="${x.retainedSizeDelta>=0?'positive':'negative'}"><b>${bytes(x.retainedSizeDelta)}</b></td><td>${delta(x.nodeCountDelta)} 个</td><td>${esc((x.hashValues||[]).join(', ')||'NA')}</td><td>${esc(x.propertyName||'NA')}</td><td>${esc(x.displayType||'NA')}</td><td><b>${esc(name)}</b></td></tr>`).join('');
const leakDetails = leakEntries.map(([name, x], i) => {
  const source = x.detailsSnapshot || 'B';
  const nodeLabel = source === 'A' ? '消失节点 ID' : '新增节点 ID';
  return `<div class="top-item" id="leakDetail_${i}"><div class="top-item-header expanded" id="leakHeader_${i}" onclick="toggleDetail('leak',${i})"><span class="arrow">▶</span><span class="header-rank">泄漏对象${i+1}：</span><span class="header-func">${esc(name)}</span><div class="header-stats">总大小 <span class="${x.retainedSizeDelta>=0?'positive':'negative'}">${bytes(x.retainedSizeDelta)}</span>，节点数 ${delta(x.nodeCountDelta)}</div></div><div class="top-item-body expanded" id="leakBody_${i}"><div class="call-chains-section">${(x.referenceChains||[]).map((c,j)=>`<div class="chain-detail-section"><div class="call-chain-line">引用链: ${chainHtml((c.chain||[]).join(' -> '))}</div>${nodeIdsHtml(c.nodeIds,`leakNodeIds_${i}_${j}`,nodeLabel)}</div>`).join('')||'<div class="call-chain-line" style="color:#a0a0b0">无引用链数据</div>'}</div></div></div>`;
}).join('');
$('#leaks').innerHTML = leakEntries.length
  ? `<div class="table-wrap"><table class="top-table leak-table"><thead><tr><th>详情</th><th>总览图</th><th>总大小</th><th>节点数</th><th>hash值<span class="help-icon" data-tooltip="JSLeakWatcher工具检测泄漏生成的泄漏对象hash值">?</span></th><th>属性名称<span class="help-icon" data-tooltip="ArkUI组件生命周期结束后，其对象上特定名称的属性会被置为TRUE，用来标记此状态。属性名和组件类型的对应关系为：\nisDeleting_对应Component类组件；_isDisposed对应NodeContainer和XComponent类组件；isDestroyed对应Ability和Window类组件。">?</span></th><th>类型<span class="help-icon" data-tooltip="检测到的有属性标签的组件或者对象类型。\n如果值为Others，说明该对象非泄漏标签方案命中的组件或者对象，可能是开发者自行添加的属性。">?</span></th><th>对象名称</th></tr></thead><tbody>${leakRows}</tbody></table></div>${leakDetails}`
  : '<div class="empty-msg">未发现发生变化且符合规则的泄漏对象。</div>';
)JS";
}

bool write_compare_file(const std::string& path, const std::string& content) {
    auto ofs = open_ofstream(path);
    if (!ofs) {
        log::loge("Error: Cannot write to ", path);
        return false;
    }
    ofs << content;
    return true;
}

std::string get_chart_html(const json& report, const std::string& id_prefix) {
    for (const auto& chart : report.value("icicleCharts", json::array())) {
        const std::string id = chart.value("id", "");
        if (id.find(id_prefix) != std::string::npos) {
            return chart.value("chartHtml", "");
        }
    }
    return "";
}

} // namespace

std::string jsleak_render_compare_html(const json& report) {
    const std::string data = html_escape_script_json(report.dump());
    std::ostringstream html;
    html << html_report_begin("ArkTS内存快照对比报告");
    html << report_common_css();        // jsleak 域公共基础样式（单快照 + compare 共用）
    html << R"HTML(
.analysis-tabs{display:flex;border-bottom:1px solid #dee2e6;margin:-4px 0 18px}.analysis-tab{appearance:none;border:0;border-bottom:3px solid transparent;background:transparent;color:#888;padding:10px 18px 8px;margin:0 12px -2px 0;cursor:pointer;font-family:inherit;font-size:18px;font-weight:600;line-height:1.6}.analysis-tab:hover,.analysis-tab.active{color:#e94560}.analysis-tab.active{border-bottom-color:#e94560}.analysis-panel{display:none}.analysis-panel.active{display:block}
.path-table{table-layout:fixed}.path-table th:nth-child(1){width:72px}.path-table th:nth-child(2){width:170px}.path-table th:nth-child(3){width:130px}.path-table th:nth-child(4),.path-table td:nth-child(4){white-space:normal;overflow-wrap:anywhere;word-break:break-word}
.positive{color:#e94560}
.negative{color:#1a73e8}
.leak-table th,.leak-table td,.path-table th,.path-table td{text-align:left}
)HTML";
    html << report_common_sub_tab_css();   // 二级页签 + 冰柱图容器（与单快照共用）
    html << chart_module_css();
    html << tree_table_css();
    html << html_report_css_end();
    html << html_report_title("ArkTS内存快照对比报告");
    html << html_report_body_open();
    html << html_report_meta({
        {"日志名", "基线快照 <span id=\"baselineFile\"></span>，对比快照 <span id=\"targetFile\"></span>"},
        {"类型", "<span id=\"analysisType\"></span>"},
        {"时间", "<span id=\"generatedAt\"></span>"},
        {"总内存", "<span id=\"heapSizeDelta\"></span>"},
        {"时长", "<span id=\"duration\"></span>"},
        {"进程", "<span id=\"processName\"></span>"},
    });

    const std::string leak_chart_html = get_chart_html(report, "leakObj");
    const std::string path_chart_html = get_chart_html(report, "pathDeltas");

    html << R"HTML(<section class="report-section"><div class="analysis-tabs" role="tablist"><button class="analysis-tab active" role="tab" aria-selected="true" data-panel="leaksPanel">ArkUI组件/ArkTS对象泄漏分析报告</button><button class="analysis-tab" role="tab" aria-selected="false" data-panel="objectsPanel">快照最短引用链分析报告</button></div>
<div id="leaksPanel" class="analysis-panel active" role="tabpanel"><div class="sub-tab-bar"><span class="sub-tab-item active" data-subtab="subtabLeakDetail" onclick="switchSubTab('subtabLeakDetail')">详情列表</span><span class="sub-tab-item" data-subtab="subtabChart_leakObj" onclick="switchSubTab('subtabChart_leakObj')">引用链总览图</span></div><div class="sub-tab-content active" id="subtabLeakDetail" data-subtab="subtabLeakDetail"><div id="leaks"></div></div><div class="sub-tab-content" id="subtabChart_leakObj" data-subtab="subtabChart_leakObj">)HTML";
    html << leak_chart_html;
    html << R"HTML(</div></div>
<div id="objectsPanel" class="analysis-panel" role="tabpanel"><div class="sub-tab-bar"><span class="sub-tab-item active" data-subtab="subtabPathDetail" onclick="switchSubTab('subtabPathDetail')">详情列表</span><span class="sub-tab-item" data-subtab="subtabChart_pathDeltas" onclick="switchSubTab('subtabChart_pathDeltas')">引用链总览图</span></div><div class="sub-tab-content active" id="subtabPathDetail" data-subtab="subtabPathDetail"><div class="tab-bar"><button class="filter active" data-kind="positive">当前快照中内存占用增长对象分析</button><button class="filter" data-kind="negative">当前快照中内存占用缩减对象分析</button></div><div id="paths"></div></div><div class="sub-tab-content" id="subtabChart_pathDeltas" data-subtab="subtabChart_pathDeltas">)HTML";
    html << path_chart_html;
    html << R"HTML(</div></div></section>
)HTML";
    html << html_report_body_close();
    html << html_report_data("D", data);
    html << html_report_core_script();
    html << html_report_script(report_common_js());   // jsleak 域公共交互脚本
    html << html_report_script(compare_render_js());
    html << html_report_script(report_common_sub_tab_js());
    html << html_report_script(chart_module_js());
    html << html_report_script(report_common_chart_init_js());
    html << html_report_end();
    return html.str();
}

std::string jsleak_write_compare_html(const json& report, const std::string& out_dir) {
    const auto baseline_stem = platform::stem(report["baseline"]["file"].get<std::string>());
    const auto target_stem = platform::stem(report["target"]["file"].get<std::string>());
    const std::string out_path =
        platform::join_path(out_dir, baseline_stem + "-vs-" + target_stem + ".html");
    platform::create_directories(out_dir);
    if (!write_compare_file(out_path, jsleak_render_compare_html(report))) return {};
    log::logok("Output written to: ", out_path);
    return out_path;
}

std::string jsleak_write_compare_json(const json& report, const std::string& out_dir) {
    const auto baseline_stem = platform::stem(report["baseline"]["file"].get<std::string>());
    const auto target_stem = platform::stem(report["target"]["file"].get<std::string>());
    const std::string out_path =
        platform::join_path(out_dir, baseline_stem + "-vs-" + target_stem + ".json");
    platform::create_directories(out_dir);
    if (!write_compare_file(out_path, report.dump(2))) return {};
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer