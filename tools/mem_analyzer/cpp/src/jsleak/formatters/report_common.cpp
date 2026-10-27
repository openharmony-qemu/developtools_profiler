#include "jsleak/formatters/report_common.hpp"

namespace mem_analyzer {

std::string report_common_css() {
    return R"CSS(
/* jsleak 域报告基础样式：单快照与 compare 报告共用。
   仅 compare 使用的样式（.summary-*/.delta-help 已移除；.positive/.negative/.path-table/.leak-table 已下放到 compare formatter）。 */
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'Helvetica Neue',sans-serif;background:#fff;color:#333;line-height:1.6;font-size:16px}
.report-title{text-align:center;font-size:24px;color:#e94560;font-weight:600;letter-spacing:2px;padding:18px 0}
.report-wrap{width:100%}
.meta-card{background:#f8f9fa;border:1px solid #dee2e6;border-radius:8px;padding:16px 24px;margin:20px}
.meta-grid{display:grid;grid-template-columns:2fr 1fr 1fr;gap:0}
.meta-item{padding:10px 12px;min-width:0}
.meta-item:nth-child(-n+3){border-bottom:1px solid #dee2e6}
.meta-label{font-weight:bold}
.meta-value{overflow-wrap:anywhere}
.report-section{background:#fff;border:1px solid #dee2e6;border-radius:6px;margin:16px 20px;padding:20px}
.report-section h2{font-size:18px;font-weight:600;color:#333}
.section-note,.report-note{color:#777;font-size:14px;margin:4px 0 14px}
.report-note{background:#f8f9fa;border-left:4px solid #667eea;padding:10px 14px;margin:20px}
.tab-bar{display:flex;gap:0;margin:12px 0 14px;flex-wrap:wrap;border-bottom:1px solid #dee2e6}
.filter{appearance:none;border:0;border-bottom:3px solid transparent;background:transparent;color:#888;padding:9px 16px 7px;margin:0 8px -2px 0;cursor:pointer;font-family:inherit;font-size:16px;font-weight:600;line-height:1.6}
.filter:hover,.filter.active{color:#e94560}
.filter.active{border-bottom-color:#e94560}
.table-wrap{overflow:auto}
.top-table{width:100%;border-collapse:collapse;margin-top:12px;min-width:800px}
.top-table th{background:#f0f0f0;color:#333;font-size:15px;padding:10px 12px;text-align:right;border:1px solid #dee2e6;white-space:nowrap}
.top-table td{padding:10px 12px;border:1px solid #dee2e6;font-size:15px;text-align:right}
.top-table th:first-child,.top-table td:first-child{text-align:left}
.detail-link{color:#1a73e8;cursor:pointer;text-decoration:underline;white-space:nowrap}
.detail-link:hover{color:#4285f4}
.top-item{background:#fff;border:1px solid #dee2e6;border-radius:6px;margin:12px 0;overflow:hidden;scroll-margin-top:12px}
.top-item-header{padding:12px 20px;cursor:pointer;display:block;font-size:17px;color:#333;font-weight:500;transition:background .2s ease}
.top-item-header:hover{background:#f0f0f0}
.top-item-header.collapsed{border-bottom:none}
.top-item-header.expanded{border-bottom:1px solid #dee2e6}
.top-item-header .arrow{margin-right:10px;font-size:14px;color:#999;transition:transform .3s ease;display:inline-block}
.top-item-header.expanded .arrow{transform:rotate(90deg)}
.header-rank{font-weight:600}
.header-func{font-weight:bold;word-break:break-word}
.header-stats{font-size:14px;color:#888;margin-top:4px}
.top-item-body{overflow:hidden;max-height:0;opacity:0;padding:0 20px;transition:max-height .4s ease,opacity .3s ease,padding .3s ease}
.top-item-body.expanded{opacity:1;padding:20px;max-height:none}
.call-chains-section{margin-bottom:0}
.chain-detail-section{padding:4px 0 12px;border-bottom:1px solid #dee2e6}
.chain-detail-section:last-child{border-bottom:none}
.call-chain-line{padding:4px 0;font-size:15px;color:#000;line-height:1.8;word-break:break-word}
.chain-arrow{display:inline-block;white-space:nowrap;color:#000;margin:0 4px}
.chain-node-size,.chain-node-ids{padding-top:8px;font-size:14px;color:#000}
.node-id-tag{display:inline-block;padding:2px 8px;background:#e8e8e8;border-radius:4px;margin-right:6px;margin-top:4px;font-size:14px}
.expand-btn{background:#667eea;color:#fff;cursor:pointer}
.expand-btn:hover{background:#764ba2}
.empty-msg{color:#888;font-size:17px;text-align:center;padding:30px}
.foot{color:#777;font-size:13px;margin-top:14px}
@media(max-width:850px){.meta-grid{grid-template-columns:1fr}
.meta-item{border-bottom:1px solid #dee2e6!important}
.report-section,.meta-card,.report-note{margin-left:12px;margin-right:12px}
}
)CSS";
}

std::string report_common_js() {
    return R"JS(
// jsleak 域报告公共交互脚本：单快照与 compare 报告共用。
// showDetail/toggleDetail/expandNodeIds 统一 id 约定 <prefix>Header_<i>/<prefix>Body_<i>/<prefix>Detail_<i>，
// 单快照与 compare 共用；expandHashValues 为单快照专属（留在 snapshot_html_js）。
const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({
  '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
}[c]));

const bytes = n => {
  const a = Math.abs(Number(n) || 0);
  const u = ['B', 'KiB', 'MiB', 'GiB'];
  let i = 0, v = a;
  while (v >= 1024 && i < 3) { v /= 1024; i++; }
  return (n < 0 ? '-' : n > 0 ? '+' : '') + v.toFixed(i ? 1 : 0) + ' ' + u[i];
};

const num = n => (Number(n) || 0).toLocaleString('zh-CN');
const delta = n => (n > 0 ? '+' : '') + num(n);

const nodeIdsHtml = (ids, containerId, label = '节点ID') => {
  const values = (ids || []).map(String);
  if (!values.length) return `<div class="chain-node-ids">${esc(label)}: 无</div>`;
  const shown = values.slice(0, 10).map(id => `<span class="node-id-tag">${esc(id)}</span>`).join('');
  const more = values.length > 10
    ? `<span class="node-id-tag expand-btn" id="expandBtn_${containerId}" onclick="expandNodeIds('${containerId}')">展开</span>`
    : '';
  return `<div class="chain-node-ids" id="${containerId}" data-label="${esc(label)}" data-all-ids="${esc(values.join(','))}">${esc(label)}: ${shown}${more}</div>`;
};

window.expandNodeIds = containerId => {
  const container = document.getElementById(containerId);
  if (!container) return;
  const values = (container.dataset.allIds || '').split(',').filter(Boolean);
  const label = container.dataset.label || '节点ID';
  const button = document.getElementById('expandBtn_' + containerId);
  const expanded = button ? button.dataset.expanded === 'true' : false;
  const MAX = 100;
  const limit = expanded ? 10 : MAX;
  const shown = values.slice(0, limit).map(id => `<span class="node-id-tag">${esc(id)}</span>`).join('');
  const remaining = !expanded && values.length > MAX
    ? `<span class="node-id-tag">remaining ${values.length - MAX} ...</span>`
    : '';
  const moreBtn = values.length > 10
    ? `<span class="node-id-tag expand-btn" id="expandBtn_${containerId}" data-expanded="${expanded ? 'false' : 'true'}" onclick="expandNodeIds('${containerId}')">${expanded ? '展开' : '收起'}</span>`
    : '';
  container.innerHTML = esc(label) + ': ' + shown + remaining + moreBtn;
};

const chainHtml = path => String(path).split(' -> ').map(esc).join('<span class="chain-arrow">-&gt;</span>');

window.showDetail = (prefix, i) => {
  const header = document.getElementById(prefix + 'Header_' + i);
  const body = document.getElementById(prefix + 'Body_' + i);
  if (!header || !body) return;
  header.classList.remove('collapsed');
  header.classList.add('expanded');
  body.classList.add('expanded');
  setTimeout(() => {
    const item = document.getElementById(prefix + 'Detail_' + i);
    if (item) item.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }, 100);
};

window.toggleDetail = (prefix, i) => {
  const header = document.getElementById(prefix + 'Header_' + i);
  const body = document.getElementById(prefix + 'Body_' + i);
  if (!header || !body) return;
  const expanded = header.classList.contains('expanded');
  header.classList.toggle('expanded', !expanded);
  header.classList.toggle('collapsed', expanded);
  body.classList.toggle('expanded', !expanded);
};
)JS";
}

std::string report_common_sub_tab_css() {
    return R"CSS(
/* 二级页签（详情列表 / 引用链总览图）+ 冰柱图画布容器：单快照与多快照报告共用。 */
.sub-tab-bar { display: flex; margin: 20px 0 0; border-bottom: 1px solid #dee2e6; }
.sub-tab-item { display: inline-block; padding: 8px 14px 7px; margin-right: 6px; cursor: pointer; font-size: 16px; font-weight: 600; color: #888; border-bottom: 2px solid transparent; margin-bottom: -1px; transition: color 0.2s, border-color 0.2s; }
.sub-tab-item:hover { color: #e94560; }
.sub-tab-item.active { color: #e94560; border-bottom-color: #e94560; }
.sub-tab-content { display: none; }
.sub-tab-content.active { display: block; }
.sub-tab-content .graphToolbar { margin-top: 20px; }
.icicle-breadcrumb { font-size: 13px; color: #666; margin-bottom: 6px; word-break: break-all; }
.icicle-canvas-wrap { overflow-x: hidden; overflow-y: auto; max-height: 640px; border: 1px solid #dee2e6; border-radius: 6px; background: #fff; }
.icicle-canvas-wrap canvas { display: block; cursor: pointer; }
)CSS";
}

std::string report_common_sub_tab_js() {
    return R"JS(
// 二级页签（详情列表 / 引用链总览图）+ 引用链树表交互：单快照与多快照报告共用。
(function() {
  // 最近一次在详情列表点过「详情」或「查看」的行，切回详情列表时据此滚回并保持高亮
  var lastSelectedRow = null;

  // 标记「详情」/「查看」所在行（同时取消上一个标记），供返回详情列表时定位
  function markSelectedRow(el) {
    var row = el && el.closest ? el.closest('tr') : null;
    if (lastSelectedRow && lastSelectedRow !== row) {
      lastSelectedRow.classList.remove('detail-row-active');
    }
    lastSelectedRow = row;
    if (row) { row.classList.add('detail-row-active'); }
  }

  // 二级页签切换：范围限定在目标所在的容器内，避免与主 tab 的 switchTab 互相干扰。
  // 按钮高亮按 data-subtab 反查（单/多快照的 sub-tab-item 均带 data-subtab），
  // 这样从详情列表的「查看」跳转时无需传按钮引用。
  function switchSubTab(subTabId) {
    var target = document.getElementById(subTabId);
    if (!target) return;
    var container = target.parentNode;
    var bar = null;
    var contents = [];
    for (var i = 0; i < container.children.length; i++) {
      var child = container.children[i];
      if (child.classList.contains('sub-tab-bar')) { bar = child; }
      else if (child.classList.contains('sub-tab-content')) { contents.push(child); }
    }
    for (var j = 0; j < contents.length; j++) {
      var isTarget = contents[j] === target;
      contents[j].classList.toggle('active', isTarget);
      contents[j].style.display = isTarget ? 'block' : 'none';
    }
    if (bar) {
      for (var k = 0; k < bar.children.length; k++) {
        bar.children[k].classList.toggle('active',
          bar.children[k].getAttribute('data-subtab') === subTabId);
      }
    }
    // 冰柱图在隐藏容器里宽度为 0 无法绘制，等布局完成后再重绘
    requestAnimationFrame(function() {
      requestAnimationFrame(function() {
        if (window.icicleRedrawAll) { window.icicleRedrawAll(); }
      });
    });
    // 切回「详情列表」且上次在某行点过「详情」/「查看」的，滚到那一行；否则滚到页签栏
    if (lastSelectedRow && target.contains(lastSelectedRow)) {
      lastSelectedRow.scrollIntoView({ behavior: 'smooth', block: 'center' });
    } else if (bar) {
      bar.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }

  // 详情列表「总览图」列的「查看」：切到引用链总览图并自动搜索该行关键词。
  // 关键词由后端给出（ArkTS 对象泄漏分析报告=对象名称；最短引用链=链名最后一个 "->" 后的名字）；
  // 关键词为空串时清空搜索框，等价于不带过滤、直接展示完整总览图。
  function showInOverview(el) {
    if (!el) return;
    var subTabId = el.getAttribute('data-chart-subtab');
    var keyword = el.getAttribute('data-keyword') || '';
    if (!subTabId || !document.getElementById(subTabId)) return;
    // 先记录并高亮本行，再切页签：switchSubTab 内部会顺势把视野滚到总览图
    markSelectedRow(el);
    switchSubTab(subTabId);
    // 画布 id 与二级页签 id 同源：subtabChart_<tabId> -> icicleCanvas_<tabId>
    var input = document.getElementById('searchInput_' + subTabId.replace('subtabChart_', 'icicleCanvas_'));
    if (!input) return;
    input.value = keyword;
    // 直接派发 input 事件，复用搜索框自身的逻辑（匹配集合、匹配计数、重绘）
    input.dispatchEvent(new Event('input', { bubbles: true }));
  }

  // 依据各行是否有处于折叠态的祖先，统一重算树表显隐。
  // 折叠态的唯一来源是 itt-collapsed 类（服务端渲染时也已打上）。
  function applyIcicleTreeVisibility(table) {
    var collapsedIds = {};
    var collapsedRows = table.querySelectorAll('tbody tr.itt-collapsed');
    for (var i = 0; i < collapsedRows.length; i++) { collapsedIds[collapsedRows[i].id] = true; }
    var allRows = table.querySelectorAll('tbody tr');
    for (var j = 0; j < allRows.length; j++) {
      var anc = (allRows[j].getAttribute('data-anc') || '').split(',');
      var hidden = false;
      for (var k = 0; k < anc.length; k++) {
        if (anc[k] && collapsedIds[anc[k]]) { hidden = true; break; }
      }
      allRows[j].style.display = hidden ? 'none' : '';
    }
  }

  // 单个节点展开/折叠
  function toggleIcicleTreeRow(btn) {
    var row = btn.closest ? btn.closest('tr') : null;
    if (!row) return;
    var table = row.closest('table');
    if (!table) return;
    var collapsed = row.classList.toggle('itt-collapsed');
    btn.textContent = collapsed ? '+' : '-';
    applyIcicleTreeVisibility(table);
  }

  // 一键全部展开 / 全部折叠。根节点（data-anc 为空）始终保持展开，
  // 于是「全部折叠」回到最紧凑的视图：只显示根 + 它的直接子节点。
  function toggleIcicleTreeAll(tableId, expand) {
    var table = document.getElementById(tableId);
    if (!table) return;
    var rows = table.querySelectorAll('tbody tr');
    for (var i = 0; i < rows.length; i++) {
      var btn = rows[i].querySelector('.itt-toggle');
      if (!btn) continue;                       // 叶子节点没有开关，保持可见
      if (!rows[i].getAttribute('data-anc')) continue;  // 根节点不参与折叠
      rows[i].classList.toggle('itt-collapsed', !expand);
      btn.textContent = expand ? '-' : '+';
    }
    applyIcicleTreeVisibility(table);
  }

  // 复制引用链：忽略当前折叠状态，按「全部展开」的树形导出纯文本。
  function copyIcicleTree(tableId, btn) {
    var table = document.getElementById(tableId);
    if (!table) return;
    // 表头取自 thead，列增减时自动同步
    var ths = table.querySelectorAll('thead th');
    var header = [];
    for (var t = 0; t < ths.length; t++) header.push(ths[t].textContent);
    var lines = [header.join('\t')];
    var rows = table.querySelectorAll('tbody tr');
    for (var i = 0; i < rows.length; i++) {
      var row = rows[i];
      var anc = row.getAttribute('data-anc') || '';
      var depth = anc ? anc.split(',').length : 0;
      var nameEl = row.querySelector('.itt-name-text');
      var name = nameEl ? nameEl.textContent : '';
      // 按表格列顺序取单元格；名称列前面补层级缩进（2 空格/层）
      var tds = row.querySelectorAll('td');
      var cells = [];
      for (var c = 0; c < tds.length; c++) {
        if (tds[c].classList.contains('itt-name')) {
          cells.push(new Array(depth + 1).join('  ') + name);
        } else {
          cells.push(tds[c].textContent);
        }
      }
      lines.push(cells.join('\t'));
    }
    var text = lines.join('\n');
    var restore = function() { if (btn) btn.textContent = '复制引用链'; };
    var showCopied = function() {
      if (btn) btn.textContent = '已拷贝';
      setTimeout(restore, 1500);
    };
    // 非安全上下文（部分 file:// 场景）或写入被拒时，回退到临时 textarea + execCommand
    var fallbackCopy = function() {
      var ta = document.createElement('textarea');
      ta.value = text;
      ta.style.position = 'fixed';
      ta.style.opacity = '0';
      document.body.appendChild(ta);
      ta.select();
      try { document.execCommand('copy'); showCopied(); } finally { document.body.removeChild(ta); }
    };
    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(text).then(showCopied, fallbackCopy);
      return;
    }
    fallbackCopy();
  }

  window.markSelectedRow = markSelectedRow;
  window.switchSubTab = switchSubTab;
  window.showInOverview = showInOverview;
  window.toggleIcicleTreeRow = toggleIcicleTreeRow;
  window.toggleIcicleTreeAll = toggleIcicleTreeAll;
  window.copyIcicleTree = copyIcicleTree;
})();
)JS";
}

std::string report_common_chart_init_js() {
    return R"JS(
// 引用链总览图：把 window.ICICLE_CHARTS_DATA 交给共享图表模块渲染。
// 单快照与多快照报告共用；缩放平移/悬停/搜索/下钻等交互由 chart_module 提供。
(function() {
  function initVisibleCharts() {
    var data = window.ICICLE_CHARTS_DATA || [];
    if (!data.length) return;
    window.ICICLE_CHARTS_DATA = [];
    var charts = [];
    for (var i = 0; i < data.length; i++) {
      var c = data[i];
      if (!c || !c.tree) continue;
      charts.push({
        icicleCanvasId: c.id,
        icicleTree: c.tree,
        flameCanvasId: null,
        downstreamTree: null,
        greyDepth: -1,          // 引用链根节点与 trace 冰柱图一致置灰
        useDisplayName: false,  // 引用链节点名已是可读文本，无需再解析
        envelopeLayout: true,   // 宽度按「子树上包络」分配，见 chart_module layoutTotalVal
        crumbId: 'crumb_' + c.id,
        resetBtnId: 'reset_' + c.id,
        searchInputId: 'searchInput_' + c.id,
        searchCountId: 'searchCount_' + c.id
      });
    }
    if (charts.length && window.MemChart) {
      MemChart.init({ charts: charts, unitCountMode: false });
      // 主 tab / 二级页签切换后画布宽度才有效，保留 icicleRedrawAll 供 switchTab/switchSubTab 调用
      window.icicleRedrawAll = function() { MemChart.redrawAll(); };
    }
  }
  // 等首帧布局完成后再初始化，避免隐藏容器里画布宽度为 0
  requestAnimationFrame(function() {
    requestAnimationFrame(function() { initVisibleCharts(); });
  });
})();
)JS";
}

} // namespace mem_analyzer