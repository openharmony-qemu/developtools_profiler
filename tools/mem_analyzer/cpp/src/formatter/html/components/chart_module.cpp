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
#include "formatter/html/components/chart_module.hpp"

namespace mem_analyzer {

// 图表模块 CSS：工具栏、搜索框、快捷键提示、tooltip、右键菜单。
// 这些规则原先内联在 html_analysis_common.cpp 的模板中，抽取后由 htrace 与
// snapshot 共用（htrace 模板不再重复定义）。
std::string ChartModuleCss()
{
    return R"CHARTCSS(
/* ===== 共享图表模块 ===== */
.graphToolbar { display: flex; gap: 8px; align-items: center; margin-bottom: 8px; flex-wrap: wrap; }
.reset-btn { background: #f0f0f0; color: #333; border: 1px solid #e94560; padding: 6px 16px; cursor: pointer; border-radius: 4px; font-size: 15px; transition: background 0.2s; }
.reset-btn:hover { background: #e0e0e0; }
.shortcut-hint { font-size: 14px; color: #888; margin-left: 10px; }
.graph-search-input { padding: 5px 10px; background: #fff; color: #333; border: 1px solid #dee2e6; border-radius: 4px; font-size: 13px; outline: none; min-width: 360px; flex-shrink: 0; }
.graph-search-input:focus { border-color: #e94560; }
.graph-search-input::placeholder { color: #aaa; }
.search-match-count { font-size: 12px; color: #888; white-space: nowrap; }
.mem-chart-tooltip { position: fixed; background: #fff; border: 1px solid #dee2e6; padding: 10px 14px; border-radius: 6px; font-size: 15px; display: none; z-index: 1000; max-width: 600px; pointer-events: none; box-shadow: 0 4px 12px rgba(0,0,0,0.15); }
.mem-chart-tooltip .fn { color: #e94560; font-weight: bold; word-break: break-all; }
.mem-chart-tooltip .detail { color: #888; margin-top: 6px; line-height: 1.6; }
.mem-chart-tooltip .detail span { color: #333; }
.mem-chart-ctxmenu { position: fixed; background: #fff; border: 1px solid #dee2e6; border-radius: 6px; padding: 4px 0; display: none; z-index: 1001; min-width: 160px; box-shadow: 0 4px 12px rgba(0,0,0,0.15); }
.mem-chart-ctxmenu .ctx-item { padding: 8px 16px; font-size: 13px; color: #333; cursor: pointer; }
.mem-chart-ctxmenu .ctx-item:hover { background: #f0f0f0; }
.mem-chart-canvas { display: block; width: 100%; cursor: pointer; background: #fff; border: 1px solid #dee2e6; border-radius: 4px; }
)CHARTCSS";
}

// 图表模块 JS：布局 / 尺寸刻度 / 缩放平移 / 悬停 tooltip / 右键菜单 / 搜索高亮 /
// 下钻与祖先链 / 重置 / resize 自适应。对外暴露 window.MemChart。
std::string ChartModuleJs() {
  return R"CHARTJS(
(function () {
  'use strict';

  var ROW_HEIGHT = 20;
  var BAR_HEIGHT = 18;
  var MIN_WIDTH = 0.5;
  var SCALE_HEIGHT = 26;
  var PADDING_BOTTOM = 8;
  var PADDING_SIDE = 0;

  var UNIT_COUNT_MODE = false;
  var charts = [];
  var states = [];
  var tooltip = null;
  var ctxMenu = null;
  var activeIdx = -1;
  var mouseClientX = 0;
  var ctxNode = null;
  var ctxRoot = null;
  var ctxPanel = '';
  var heldKeys = {};
  var keyLoopRunning = false;
  var lastKeyExec = {};
  var pendingZoom = false;
  // 宽度是否按「子树上包络」分配，由 chart.envelopeLayout 在 drawPanel 里按图表切换。
  // 仅引用链冰柱图需要（见 layoutTotalVal 注释），trace 报告保持原行为。
  var envelopeLayout = false;

  // ===== 格式化与命名 =====
  function fmtBytes(bytes) {
    if (UNIT_COUNT_MODE) return String(Math.round(bytes));
    if (bytes === 0) return '0 B';
    var units = ['B', 'KB', 'MB', 'GB'];
    var k = 1024;
    var i = Math.floor(Math.log(bytes) / Math.log(k));
    if (i > 3) i = 3;
    return (bytes / Math.pow(k, i)).toFixed(2) + ' ' + units[i];
  }

  // Extract display name from full name (n) field
  // name format: "lib+libOffset (symbol)" or "lib (symbol)" or "lib+libOffset" or "symbol"
  function getDisplayName(name) {
    var urlIdx = name.indexOf(':[url:');
    if (urlIdx >= 0) return name;
    var tsParenMatch = name.match(/\((\S+)\)$/);
    if (tsParenMatch) {
      var funcName = tsParenMatch[1];
      var prefix = name.substring(0, name.length - tsParenMatch[0].length);
      if (/\.\w+:\d+:\d+\s*$/.test(prefix)) {
        var slashIdx = prefix.lastIndexOf('/');
        var shortPrefix = slashIdx >= 0 ? prefix.substring(slashIdx + 1) : prefix;
        return funcName + ':[' + shortPrefix.trim() + ']';
      }
    }
    var parenIdx = name.indexOf(' (');
    if (parenIdx >= 0) return name.substring(parenIdx + 2, name.length - 1);
    var plusIdx = name.indexOf('+');
    if (plusIdx >= 0) {
      var lib = name.substring(0, plusIdx);
      var slashIdx = lib.lastIndexOf('/');
      return (slashIdx >= 0 ? lib.substring(slashIdx + 1) : lib) + '+' + name.substring(plusIdx + 1);
    }
    var slashIdx = name.lastIndexOf('/');
    if (slashIdx >= 0 && /\.\w+/.test(name.substring(0, slashIdx))) {
      return name.substring(slashIdx + 1);
    }
    return name;
  }

  function nameOf(chart, name) {
    return chart && chart.useDisplayName === false ? name : getDisplayName(name);
  }

  // Resolve name indices in tree nodes to actual strings using the names string table
  function resolveNames(node, names) {
    if (!node || !names || !names.length) return;
    if (typeof node.n === 'number') node.n = names[node.n];
    if (node.c) {
      for (var i = 0; i < node.c.length; i++) resolveNames(node.c[i], names);
    }
  }

  // Use opaque RGB for crisper canvas rendering
  function getFlameColor(widthPct) {
    var pct = typeof widthPct === 'number' && widthPct >= 0 && widthPct <= 1 ? widthPct : 0.5;
    var r = Math.floor(245 + 10 * (1 - pct));
    var g = Math.floor(110 + 105 * (1 - pct));
    return 'rgb(' + r + ',' + g + ',100)';
  }

  function getGrayLevel(colorStr) {
    var prefix = colorStr.indexOf('rgba') !== -1 ? 'rgba' : colorStr.indexOf('rgb') !== -1 ? 'rgb' : '';
    if (!prefix) {
      if (colorStr.charAt(0) === '#') {
        var hex = colorStr.substring(1);
        if (hex.length === 3) hex = hex.charAt(0) + hex.charAt(0) + hex.charAt(1) + hex.charAt(1) + hex.charAt(2) + hex.charAt(2);
        var hr = parseInt(hex.substring(0, 2), 16), hg = parseInt(hex.substring(2, 4), 16), hb = parseInt(hex.substring(4, 6), 16);
        return hr * 0.299 + hg * 0.587 + hb * 0.114;
      }
      return 128;
    }
    var open = colorStr.indexOf('(');
    var close = colorStr.indexOf(')');
    if (open < 0 || close < 0) return 128;
    var nums = colorStr.substring(open + 1, close).split(',');
    var r = parseInt(nums[0]), g = parseInt(nums[1]), b = parseInt(nums[2]);
    return r * 0.299 + g * 0.587 + b * 0.114;
  }

  // ===== 搜索匹配 =====
  function matchesSearchTerm(chart, name, term) {
    var names = [nameOf(chart, name), name];
    for (var ni = 0; ni < names.length; ni++) {
      var displayName = names[ni];
      var lower = displayName.toLowerCase();
      var termLen = term.length;
      var plusOfsIdx = lower.indexOf('+0x');
      if (plusOfsIdx >= 0) {
        var ofsStart = plusOfsIdx + 1;
        for (var i = 0; i <= lower.length - termLen; i++) {
          if (lower.substring(i, i + termLen) !== term) continue;
          if (i >= ofsStart - 1 || i + termLen > ofsStart) return true;
        }
      }
      for (var j = 0; j <= lower.length - termLen; j++) {
        if (lower.substring(j, j + termLen) !== term) continue;
        if (j === 0) return true;
        var prevCh = displayName[j - 1];
        if (!/[a-zA-Z0-9]/.test(prevCh)) return true;
        if (/[a-z]/.test(prevCh) && /[A-Z]/.test(displayName[j])) return true;
      }
    }
    return false;
  }

  function buildSearchSet(chart, node, term, matchSet) {
    if (!node) return;
    if (matchesSearchTerm(chart, node.n, term)) matchSet.add(node.n);
    if (node.c) {
      for (var i = 0; i < node.c.length; i++) buildSearchSet(chart, node.c[i], term, matchSet);
    }
  }

  // ===== 树工具 =====
  // 布局总量：正常情况下等于节点自身值；引用链冰柱图里子链的 retained size 可能
  // 大于父链（父链与子链是不同对象集合，retained 集合并不嵌套），此时按子节点之和
  // 归一化，避免子条带溢出父条带。展示值仍用 node.t。
  //
  // envelopeLayout（引用链冰柱图）时进一步取「子树上包络」：
  //   layoutTotalVal(node) = max(node.t, Σ layoutTotalVal(children))
  // 原因：子条带的宽度只能在父条带分到的槽位里切分，若父链自身值很小、子树里却有
  // 一条大得多的深链（例如 VMRoot->GlobalEnv 很小，其下的 ...->ObserveV2 很大），
  // 整条深链会被压进父链那个窄槽位里，看起来「被缩短」。
  // 用上包络后父条带的槽位至少与它最宽的子树一样宽，宽度沿路径单调不增，深链得以
  // 按真实比例铺开。只影响宽度分配，展示值（node.t / 悬停提示 / 表格）仍是真实
  // retained size。对 trace 报告（子之和恒 ≤ 父自身）结果与 node.t 相同，行为不变。
  // 结果缓存在 node._lt 上：只依赖自身子树，下钻/重绘都安全。
  function layoutTotalVal(node) {
    var cs = node.c || [];
    if (!envelopeLayout) {
      var childSum = 0;
      for (var i = 0; i < cs.length; i++) childSum += cs[i].t;
      return Math.max(node.t, childSum);
    }
    if (node._lt !== undefined) return node._lt;
    var envSum = 0;
    for (var j = 0; j < cs.length; j++) envSum += layoutTotalVal(cs[j]);
    node._lt = Math.max(node.t, envSum);
    return node._lt;
  }

  // 分配子条带宽度时用的"大小值"：必须与 layoutTotalVal 的口径一致。
  // 包络模式下要把子节点的包络放进分子（否则 Σ子宽 << 父宽，条带右侧大片留白）；
  // 默认模式仍是节点自身值，与旧行为逐像素一致。
  function layoutChildVal(node) {
    return envelopeLayout ? layoutTotalVal(node) : node.t;
  }

  function visibleMaxDepth(node, width) {
    if (width < MIN_WIDTH) return 0;
    if (!node.c || node.c.length === 0) return 1;
    var totalVal = layoutTotalVal(node);
    var maxChild = 0;
    for (var i = 0; i < node.c.length; i++) {
      var child = node.c[i];
      var childW = totalVal > 0 ? (layoutChildVal(child) / totalVal) * width : 0;
      var d = visibleMaxDepth(child, childW);
      if (d > maxChild) maxChild = d;
    }
    return maxChild > 0 ? 1 + maxChild : 1;
  }

  function findPathFromRoot(root, target) {
    if (!root) return null;
    if (root === target) return [root];
    if (!root.c) return null;
    for (var i = 0; i < root.c.length; i++) {
      var subPath = findPathFromRoot(root.c[i], target);
      if (subPath) {
        var result = [root];
        for (var j = 0; j < subPath.length; j++) result.push(subPath[j]);
        return result;
      }
    }
    return null;
  }

  function searchNode(node, mx, my) {
    if (!node || node._width === undefined || node._width < MIN_WIDTH) return null;
    if (mx >= node._x && mx <= node._x + node._width) {
      if (node.c) {
        for (var i = 0; i < node.c.length; i++) {
          var child = node.c[i];
          if (child._width !== undefined && child._width >= MIN_WIDTH && mx >= child._x && mx <= child._x + child._width) {
            var found = searchNode(child, mx, my);
            if (found) return found;
          }
        }
      }
      if (my >= node._y && my <= node._y + ROW_HEIGHT - 1) return node;
    }
    return null;
  }

  function findNodeAt(root, mx, my) {
    if (!root) return null;
    return searchNode(root, mx, my);
  }

  // 命中已下钻路径上的祖先链条带（绘制在根节点上方/下方）
  function findChainNode(chain, mx, my) {
    if (!chain) return null;
    for (var ci = 0; ci < chain.length; ci++) {
      var cn = chain[ci];
      if (cn._x !== undefined && cn._width !== undefined &&
          mx >= cn._x && mx <= cn._x + cn._width &&
          my >= cn._y && my <= cn._y + ROW_HEIGHT - 1) {
        return cn;
      }
    }
    return null;
  }

  // ===== 布局 =====
  function layoutFlame(node, x, width, depth, canvasHeight, calleeShift) {
    node._x = x;
    node._width = width;
    node._y = canvasHeight - PADDING_BOTTOM - calleeShift - (depth + 1) * ROW_HEIGHT;
    var totalVal = layoutTotalVal(node);
    var childX = x;
    var sorted = (node.c || []).slice().sort(function (a, b) { return layoutChildVal(b) - layoutChildVal(a); });
    for (var i = 0; i < sorted.length; i++) {
      var child = sorted[i];
      var childWidth = totalVal > 0 ? (layoutChildVal(child) / totalVal) * width : 0;
      if (childWidth >= MIN_WIDTH) layoutFlame(child, childX, childWidth, depth + 1, canvasHeight, calleeShift);
      childX += childWidth;
    }
  }

  function layoutIcicle(node, x, width, depth, callerShift) {
    node._x = x;
    node._width = width;
    node._y = SCALE_HEIGHT + callerShift + depth * ROW_HEIGHT;
    var totalVal = layoutTotalVal(node);
    var childX = x;
    var sorted = (node.c || []).slice().sort(function (a, b) { return layoutChildVal(b) - layoutChildVal(a); });
    for (var i = 0; i < sorted.length; i++) {
      var child = sorted[i];
      var childWidth = totalVal > 0 ? (layoutChildVal(child) / totalVal) * width : 0;
      if (childWidth >= MIN_WIDTH) layoutIcicle(child, childX, childWidth, depth + 1, callerShift);
      childX += childWidth;
    }
  }

  // ===== 尺寸刻度 =====
  function drawRuler(ctx, canvasW, rectWidth, canvasH, xPoint, root) {
    if (!root) return;
    var rulerLeft = PADDING_SIDE + xPoint;
    var rulerRight = rulerLeft + rectWidth - 2 * PADDING_SIDE;
    var rulerWidth = rulerRight - rulerLeft;
    if (rulerWidth <= 0) return;

    var chartBottom = canvasH - PADDING_BOTTOM;
    // 标尺基数必须与条带宽度分配用的总量一致（见 layoutTotalVal），否则刻度与条宽对不上
    var rulerTotal = layoutTotalVal(root);

    ctx.save();

    ctx.lineWidth = 0.5;
    ctx.strokeStyle = '#ddd';
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(canvasW, 0);
    ctx.stroke();

    for (var i = 0; i <= 10; i++) {
      var startX = Math.floor((rulerWidth / 10) * i) + rulerLeft;
      if (startX < -10 || startX > canvasW + 10) continue;
      for (var j = 0; j < 10 && i < 10; j++) {
        var startItemX = startX + Math.floor((rulerWidth / 100) * j);
        if (startItemX < 0 || startItemX > canvasW) continue;
        ctx.lineWidth = 0.5;
        ctx.strokeStyle = '#ccc';
        ctx.beginPath();
        ctx.moveTo(startItemX, 0);
        ctx.lineTo(startItemX, 10);
        ctx.stroke();
      }
      if (i === 0) continue;
      ctx.lineWidth = 1;
      ctx.strokeStyle = '#e8e8e8';
      ctx.beginPath();
      ctx.moveTo(startX, 0);
      ctx.lineTo(startX, chartBottom);
      ctx.stroke();
      if (startX > 0 && startX < canvasW) {
        ctx.fillStyle = '#000';
        ctx.font = '12px bold -apple-system, BlinkMacSystemFont, sans-serif';
        var calibration = fmtBytes(Math.round((rulerTotal / 10) * i));
        var textSize = ctx.measureText(calibration).width;
        ctx.fillText(calibration, startX - textSize - 5, SCALE_HEIGHT - 4, 50);
      }
    }

    ctx.restore();
  }

  // ===== 绘制节点 =====
  function drawNode(ctx, node, state, canvasW, panelType) {
    var chart = state.chart;
    var x = node._x, y = node._y, w = node._width;
    if (w === undefined || w < MIN_WIDTH) return;
    if (x + w < 0 || x >= canvasW) return;
    x = Math.round(x * 2) / 2;
    w = Math.round(w * 2) / 2;
    var yBar = Math.round((y + 1) * 2) / 2;
    var hoveredNode = panelType === 'flame' ? state.flameHoveredNode : state.icicleHoveredNode;
    var currentRoot = panelType === 'flame' ? state.flameRoot : state.icicleRoot;
    var isHovered = hoveredNode === node;
    var isMatch = state.searchTerm && state.searchMatchSet.has(node.n);
    var isDimmed = state.searchTerm && !isMatch;
    if (isDimmed) ctx.fillStyle = '#e0e0e0';
    else if (isMatch) ctx.fillStyle = '#5290ff';
    else if (chart.greyDepth !== null && node.d === chart.greyDepth) ctx.fillStyle = '#919191';
    else if (node.ts) ctx.fillStyle = 'rgb(132,200,112)';
    else ctx.fillStyle = getFlameColor(node.t / currentRoot.t);
    ctx.fillRect(x, yBar, w, BAR_HEIGHT);
    ctx.lineWidth = 0.5;
    if (isHovered) ctx.strokeStyle = '#000';
    else ctx.strokeStyle = '#fff';
    ctx.strokeRect(x, yBar, w, BAR_HEIGHT);
    if (isHovered) { ctx.strokeStyle = '#000'; ctx.lineWidth = 1.5; ctx.strokeRect(x + 0.5, yBar + 0.5, w - 1, BAR_HEIGHT - 1); }
    if (w > 10) {
      if (isDimmed) ctx.fillStyle = '#aaa';
      else {
        var grayLevel = getGrayLevel(ctx.fillStyle);
        if (grayLevel >= 100) ctx.fillStyle = '#000';
        else ctx.fillStyle = '#fff';
      }
      ctx.font = '12px bold sans-serif';
      ctx.textBaseline = 'middle';
      var textPadding = 5;
      var fillTextW = w - textPadding * 2;
      var text = nameOf(chart, node.n);
      var textW = ctx.measureText(text).width;
      var textY = Math.round((yBar + BAR_HEIGHT / 2) * 2) / 2;
      if (textW < fillTextW) {
        var textX = Math.floor(w / 2 - textW / 2 + x + textPadding);
        ctx.fillText(text, textX, textY);
      } else if (fillTextW >= ctx.measureText('A').width) {
        var charW = Math.round(textW / text.length);
        var charNum = fillTextW / charW;
        if (charNum < 2) {
          ctx.fillText(text.substring(0, 1), x + textPadding, textY, fillTextW);
        } else {
          ctx.fillText(text.substring(0, Math.floor(charNum - 1)) + '..', x + textPadding, textY, fillTextW);
        }
      }
    }
    if (!node.c) return;
    for (var i = 0; i < node.c.length; i++) {
      var child = node.c[i];
      if (child._width !== undefined && child._width >= MIN_WIDTH) drawNode(ctx, child, state, canvasW, panelType);
    }
  }

  // ===== 绘制祖先链条带 =====
  function drawChainNodes(ctx, idx, canvasW, panelType) {
    var state = states[idx];
    var chart = state.chart;
    var chain = panelType === 'flame' ? state.calleeChain : state.callerChain;
    if (!chain || chain.length === 0) return;
    var hoveredNode = panelType === 'flame' ? state.flameHoveredNode : state.icicleHoveredNode;
    for (var ci = 0; ci < chain.length; ci++) {
      var node = chain[ci];
      var x = node._x, y = node._y, w = node._width;
      if (w === undefined || w < MIN_WIDTH || x + w < 0 || x >= canvasW) continue;
      x = Math.round(x * 2) / 2;
      w = Math.round(w * 2) / 2;
      var yBar = Math.round((y + 1) * 2) / 2;
      var isHovered = hoveredNode === node;
      var isMatch = state.searchTerm && state.searchMatchSet.has(node.n);
      var isDimmed = state.searchTerm && !isMatch;

      if (isDimmed) ctx.fillStyle = '#e0e0e0';
      else if (isMatch) ctx.fillStyle = '#5290ff';
      else if (node.ts) ctx.fillStyle = 'rgb(132,200,112)';
      else ctx.fillStyle = '#919191';

      ctx.fillRect(x, yBar, w, BAR_HEIGHT);
      ctx.lineWidth = 0.5;
      if (isHovered) ctx.strokeStyle = '#000';
      else ctx.strokeStyle = '#fff';
      ctx.strokeRect(x, yBar, w, BAR_HEIGHT);
      if (isHovered) { ctx.strokeStyle = '#000'; ctx.lineWidth = 1.5; ctx.strokeRect(x + 0.5, yBar + 0.5, w - 1, BAR_HEIGHT - 1); }

      if (w > 10) {
        if (isDimmed) ctx.fillStyle = '#aaa';
        else {
          var grayLevel = getGrayLevel(ctx.fillStyle);
          if (grayLevel >= 100) ctx.fillStyle = '#000';
          else ctx.fillStyle = '#fff';
        }
        ctx.font = '12px bold sans-serif';
        ctx.textBaseline = 'middle';
        var textPadding = 5;
        var fillTextW = w - textPadding * 2;
        var text = nameOf(chart, node.n);
        var textW = ctx.measureText(text).width;
        var textY = Math.round((yBar + BAR_HEIGHT / 2) * 2) / 2;
        if (textW < fillTextW) {
          var textX = Math.floor(w / 2 - textW / 2 + x + textPadding);
          ctx.fillText(text, textX, textY);
        } else if (fillTextW >= ctx.measureText('A').width) {
          var charW = Math.round(textW / text.length);
          var charNum = fillTextW / charW;
          if (charNum < 2) {
            ctx.fillText(text.substring(0, 1), x + textPadding, textY, fillTextW);
          } else {
            ctx.fillText(text.substring(0, Math.floor(charNum - 1)) + '..', x + textPadding, textY, fillTextW);
          }
        }
      }
    }
  }

  // ===== 绘制单个面板 =====
  function drawPanel(idx, panelType) {
    var state = states[idx];
    if (!state) return;
    var chart = state.chart;
    // 宽度分配口径随图表切换（引用链冰柱图用子树上包络，trace 报告用原口径）
    envelopeLayout = !!chart.envelopeLayout;
    var isFlame = panelType === 'flame';
    var canvas = isFlame ? state.flameCanvas : state.icicleCanvas;
    var ctx = isFlame ? state.flameCtx : state.icicleCtx;
    if (!canvas || !ctx) return;
    if (chart.gateId) {
      var gate = document.getElementById(chart.gateId);
      if (gate && !gate.classList.contains('expanded')) return;
    }

    var currentRoot = isFlame ? state.flameRoot : state.icicleRoot;
    if (!currentRoot) return;

    var canvasW = Math.round(canvas.parentElement.getBoundingClientRect().width);
    if (canvasW <= 0) return;

    if (isFlame) {
      if (state.flameRectWidth === 0) state.flameRectWidth = canvasW;
    } else {
      if (state.icicleRectWidth === 0) state.icicleRectWidth = canvasW;
    }
    var rectWidth = isFlame ? state.flameRectWidth : state.icicleRectWidth;
    var xPoint = isFlame ? state.flameXPoint : state.icicleXPoint;
    var contentX = PADDING_SIDE + xPoint;
    var contentW = rectWidth - 2 * PADDING_SIDE;
    var depth = visibleMaxDepth(currentRoot, contentW);
    var chain = isFlame ? state.calleeChain : state.callerChain;
    var chainLen = chain.length;
    var canvasH = SCALE_HEIGHT + (depth + chainLen) * ROW_HEIGHT + PADDING_BOTTOM;

    var dpr = Math.max(window.devicePixelRatio || 1, 2);
    canvas.style.width = canvasW + 'px';
    canvas.style.height = canvasH + 'px';
    canvas.width = Math.round(canvasW * dpr);
    canvas.height = Math.round(canvasH * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.imageSmoothingEnabled = false;
    ctx.clearRect(0, 0, canvasW, canvasH);

    if (isFlame) {
      var calleeShift = chainLen * ROW_HEIGHT;
      layoutFlame(currentRoot, contentX, contentW, 0, canvasH, calleeShift);
      var rootY = currentRoot._y;
      for (var fi = 0; fi < chain.length; fi++) {
        chain[fi]._x = contentX + xPoint;
        chain[fi]._width = contentW;
        chain[fi]._y = rootY + (fi + 1) * ROW_HEIGHT;
      }
    } else {
      var callerShift = chainLen * ROW_HEIGHT;
      layoutIcicle(currentRoot, contentX, contentW, 0, callerShift);
      for (var ci = 0; ci < chain.length; ci++) {
        chain[ci]._x = contentX + xPoint;
        chain[ci]._width = contentW;
        chain[ci]._y = SCALE_HEIGHT + ci * ROW_HEIGHT;
      }
    }

    drawRuler(ctx, canvasW, rectWidth, canvasH, xPoint, currentRoot);
    drawNode(ctx, currentRoot, state, canvasW, panelType);
    drawChainNodes(ctx, idx, canvasW, panelType);
  }

  function drawChart(idx) {
    var state = states[idx];
    if (!state) return;
    if (state.icicleOnly) {
      drawPanel(idx, 'icicle');
    } else {
      drawPanel(idx, 'flame');
      drawPanel(idx, 'icicle');
    }
  }

  function drawAllVisible() {
    for (var i = 0; i < states.length; i++) drawChart(i);
  }

  function scheduleRedraw() {
    if (!pendingZoom) {
      pendingZoom = true;
      requestAnimationFrame(function () {
        pendingZoom = false;
        if (activeIdx >= 0) drawChart(activeIdx);
      });
    }
  }

  // ===== 面包屑 =====
  function updateCrumb(state) {
    if (!state.chart.crumbId) return;
    var el = document.getElementById(state.chart.crumbId);
    if (!el) return;
    el.textContent = state.crumbPath.length ? ('当前层级：' + state.crumbPath.join(' \u2192 ')) : '';
  }

  // ===== 悬停 tooltip =====
  function showTooltip(e, node, root) {
    if (!tooltip) return;
    var pct = ((node.t / root.t) * 100).toFixed(2);
    var selfPct = ((node.v / root.t) * 100).toFixed(2);
    tooltip.innerHTML = '<div class="fn">' + node.n + '</div><div class="detail">' +
      'Total: <span>' + fmtBytes(node.t) + ' (' + pct + '%)</span><br>' +
      (UNIT_COUNT_MODE ? '' : 'Count: <span>' + (node.cnt || 0) + '</span><br>') +
      'Self: <span>' + fmtBytes(node.v) + ' (' + selfPct + '%)</span><br>' +
      'Depth: <span>' + (node.d || 0) + '</span></div>';
    tooltip.style.display = 'block';
    var left = e.clientX + 15, top = e.clientY + 15;
    var tw = tooltip.offsetWidth, th = tooltip.offsetHeight;
    if (left + tw > window.innerWidth - 10) left = e.clientX - tw - 10;
    if (top + th > window.innerHeight - 10) top = e.clientY - th - 10;
    tooltip.style.left = left + 'px';
    tooltip.style.top = top + 'px';
  }

  function hideTooltip() {
    if (tooltip) tooltip.style.display = 'none';
  }

  // ===== 节点命中 =====
  function hitTest(idx, panelType, clientX, clientY) {
    var state = states[idx];
    var canvas = panelType === 'flame' ? state.flameCanvas : state.icicleCanvas;
    if (!canvas) return null;
    var rect = canvas.getBoundingClientRect();
    var mx = clientX - rect.left;
    var my = clientY - rect.top;
    var currentRoot = panelType === 'flame' ? state.flameRoot : state.icicleRoot;
    var node = findNodeAt(currentRoot, mx, my);
    if (node) return node;
    return findChainNode(panelType === 'flame' ? state.calleeChain : state.callerChain, mx, my);
  }

  // ===== 画布事件 =====
  function setupCanvasEvents(idx, canvasEl, panelType) {
    var state = states[idx];

    canvasEl.addEventListener('mousemove', function (e) {
      activeIdx = idx;
      state.activePanel = panelType;
      mouseClientX = e.clientX;
      var node = hitTest(idx, panelType, e.clientX, e.clientY);
      var hoveredKey = panelType === 'flame' ? 'flameHoveredNode' : 'icicleHoveredNode';
      if (node !== state[hoveredKey]) {
        state[hoveredKey] = node;
        drawPanel(idx, panelType);
      }
      var currentRoot = panelType === 'flame' ? state.flameRoot : state.icicleRoot;
      if (node && currentRoot) showTooltip(e, node, currentRoot);
      else hideTooltip();
    });

    canvasEl.addEventListener('mouseleave', function () {
      hideTooltip();
      var hoveredKey = panelType === 'flame' ? 'flameHoveredNode' : 'icicleHoveredNode';
      if (state[hoveredKey]) {
        state[hoveredKey] = null;
        drawPanel(idx, panelType);
      }
    });

    canvasEl.addEventListener('click', function (e) {
      activeIdx = idx;
      if (ctxMenu) ctxMenu.style.display = 'none';
      var node = hitTest(idx, panelType, e.clientX, e.clientY);
      if (node) handleNodeClick(idx, node, panelType);
    });

    // 右键菜单
    canvasEl.addEventListener('contextmenu', function (e) {
      e.preventDefault();
      activeIdx = idx;
      state.activePanel = panelType;
      if (!ctxMenu) return;
      var node = hitTest(idx, panelType, e.clientX, e.clientY);
      var currentRoot = panelType === 'flame' ? state.flameRoot : state.icicleRoot;
      if (!node || !currentRoot) {
        ctxMenu.style.display = 'none';
        return;
      }
      ctxNode = node;
      ctxRoot = currentRoot;
      ctxPanel = panelType;
      var pct = ((node.t / currentRoot.t) * 100).toFixed(2);
      var selfPct = ((node.v / currentRoot.t) * 100).toFixed(2);
      var info = node.n + '\nTotal: ' + fmtBytes(node.t) + ' (' + pct + '%)\nSelf: ' + fmtBytes(node.v) + ' (' + selfPct + '%)\nDepth: ' + (node.d || 0);
      // 文案随图表业务切换：快照「引用链总览图」只有冰柱图（无调用图），节点是引用链上的一节，
      // 用「节点/引用链」描述；htrace 报告画的是调用图，沿用「函数/调用链」。
      var refChain = !!(states[idx] && states[idx].icicleOnly);
      var labelInfo = refChain ? '拷贝节点信息' : '拷贝函数信息';
      var labelName = refChain ? '拷贝节点名称' : '拷贝函数名';
      var labelChain = refChain ? '拷贝最大引用链' : '拷贝最大调用链';
      ctxMenu.innerHTML = '<div class="ctx-item" data-copy="' + encodeURIComponent(info) + '">' + labelInfo + '</div>' +
        '<div class="ctx-item" data-copy="' + encodeURIComponent(node.n) + '">' + labelName + '</div>' +
        '<div class="ctx-item" data-copy-chain-largest>' + labelChain + '</div>';
      ctxMenu.style.display = 'block';
      var left = e.clientX, top = e.clientY;
      if (left + 160 > window.innerWidth) left = e.clientX - 160;
      if (top + 130 > window.innerHeight) top = e.clientY - 130;
      ctxMenu.style.left = left + 'px';
      ctxMenu.style.top = top + 'px';
    });
  }

  // ===== 点击下钻（双视图联动） =====
  function handleNodeClick(idx, node, panelType) {
    var state = states[idx];
    if (!node) return;
    if (panelType === 'flame' && node === state.flameRoot) return;
    if (panelType === 'icicle' && node === state.icicleRoot) return;

    if (state.icicleOnly) {
      var path = findPathFromRoot(state.originalIcicleRoot, node);
      if (path && path.length > 0) {
        state.icicleZoomStack.push(state.icicleRoot);
        state.icicleRoot = node;
        state.icicleRectWidth = 0;
        state.icicleXPoint = 0;
        state.icicleHoveredNode = null;
        state.callerChain = path.length > 1 ? path.slice(0, path.length - 1) : [];
      }
    } else {
      var flamePath = findPathFromRoot(state.originalFlameRoot, node);
      if (flamePath && flamePath.length > 0) {
        state.flameZoomStack.push(state.flameRoot);
        state.flameRoot = node;
        state.flameRectWidth = 0;
        state.flameXPoint = 0;
        state.flameHoveredNode = null;
        state.calleeChain = flamePath.length > 1 ? flamePath.slice(0, flamePath.length - 1).reverse() : [];
      }
      var iciclePath = findPathFromRoot(state.originalIcicleRoot, node);
      if (iciclePath && iciclePath.length > 0) {
        state.icicleZoomStack.push(state.icicleRoot);
        state.icicleRoot = node;
        state.icicleRectWidth = 0;
        state.icicleXPoint = 0;
        state.icicleHoveredNode = null;
        state.callerChain = iciclePath.length > 1 ? iciclePath.slice(0, iciclePath.length - 1) : [];
      }
    }

    if (state.chart.crumbId && panelType === 'icicle') state.crumbPath.push(node.n);
    updateCrumb(state);
    hideTooltip();
    drawChart(idx);
  }

  // ===== 重置 =====
  function resetZoom(idx) {
    var state = states[idx];
    if (!state) return;
    state.flameRoot = state.originalFlameRoot;
    state.flameZoomStack = [];
    state.flameRectWidth = 0;
    state.flameXPoint = 0;
    state.flameHoveredNode = null;
    state.icicleRoot = state.originalIcicleRoot;
    state.icicleZoomStack = [];
    state.icicleRectWidth = 0;
    state.icicleXPoint = 0;
    state.icicleHoveredNode = null;
    state.calleeChain = [];
    state.callerChain = [];
    state.crumbPath = [];
    state.searchTerm = '';
    state.searchMatchSet.clear();
    state.objectMatches = false;
    var searchInput = state.chart.searchInputId ? document.getElementById(state.chart.searchInputId) : null;
    if (searchInput) searchInput.value = '';
    var countEl = state.chart.searchCountId ? document.getElementById(state.chart.searchCountId) : null;
    if (countEl) countEl.textContent = '0个匹配';
    updateCrumb(state);
    hideTooltip();
    drawChart(idx);
  }

  // ===== 缩放/平移（WASD） =====
  function execWASD(key) {
    if (activeIdx < 0 || activeIdx >= states.length) return;
    var state = states[activeIdx];
    if (!state) return;

    var isFlame = state.activePanel === 'flame';
    var canvasEl = isFlame ? state.flameCanvas : state.icicleCanvas;
    if (!canvasEl) return;
    var rectWidth = isFlame ? state.flameRectWidth : state.icicleRectWidth;
    var xPoint = isFlame ? state.flameXPoint : state.icicleXPoint;

    var parentRect = canvasEl.parentElement.getBoundingClientRect();
    var canvasW = Math.round(parentRect.width);
    var scaleRatio = 0.2;
    var canvasX = mouseClientX - parentRect.left;
    var deltaWidth, translationValue, offset;

    if (rectWidth <= 0) return;

    if (key === 'w') {
      deltaWidth = rectWidth * scaleRatio;
      translationValue = deltaWidth * (canvasX - xPoint) / rectWidth;
      xPoint -= translationValue;
      rectWidth += deltaWidth;
      if (xPoint > 0) xPoint = 0;
    } else if (key === 's') {
      deltaWidth = rectWidth * scaleRatio;
      var newWidth = rectWidth - deltaWidth;
      if (newWidth < canvasW) {
        rectWidth = canvasW;
        xPoint = 0;
      } else {
        translationValue = deltaWidth * (canvasX - xPoint) / rectWidth;
        xPoint += translationValue;
        rectWidth = newWidth;
        if (xPoint > 0) xPoint = 0;
        if (rectWidth + xPoint < canvasW) xPoint = canvasW - rectWidth;
      }
    } else if (key === 'a') {
      offset = canvasW / 10;
      xPoint += offset;
      if (xPoint > 0) xPoint = 0;
    } else if (key === 'd') {
      offset = canvasW / 10;
      xPoint -= offset;
      if (rectWidth + xPoint < canvasW) xPoint = canvasW - rectWidth;
    } else {
      return;
    }

    if (isFlame) {
      state.flameRectWidth = rectWidth;
      state.flameXPoint = xPoint;
    } else {
      state.icicleRectWidth = rectWidth;
      state.icicleXPoint = xPoint;
    }
    scheduleRedraw();
  }

  function startKeyLoop(key) {
    lastKeyExec[key] = performance.now();
    if (!keyLoopRunning) {
      keyLoopRunning = true;
      requestAnimationFrame(keyLoopTick);
    }
  }

  function keyLoopTick(now) {
    var anyHeld = false;
    for (var key in heldKeys) {
      if (heldKeys[key]) {
        anyHeld = true;
        var elapsed = now - (lastKeyExec[key] || 0);
        if (elapsed >= 80) {
          lastKeyExec[key] = now;
          execWASD(key);
        }
      }
    }
    if (anyHeld) requestAnimationFrame(keyLoopTick);
    else keyLoopRunning = false;
  }

  // ===== 右键菜单：拷贝最大链（htrace 调用图=最大调用链，快照引用链图=最大引用链） =====
  function findLargestCallChain(root, targetNode) {
    var pathToTarget = [];
    function findPath(node, target, path) {
      path.push(node);
      if (node === target) return true;
      if (node.c) {
        for (var i = 0; i < node.c.length; i++) {
          if (findPath(node.c[i], target, path)) return true;
        }
      }
      path.pop();
      return false;
    }
    findPath(root, targetNode, pathToTarget);

    var bestLeaf = null;
    var bestTotalSize = 0;
    var bestPathFromTarget = [];
    function findLeaves(node, pathFromTarget) {
      pathFromTarget.push(node);
      var isLeaf = !node.c || node.c.length === 0;
      if (isLeaf && node.t > bestTotalSize) {
        bestTotalSize = node.t;
        bestLeaf = node;
        bestPathFromTarget = pathFromTarget.slice();
      }
      if (node.c) {
        for (var i = 0; i < node.c.length; i++) findLeaves(node.c[i], pathFromTarget);
      }
      pathFromTarget.pop();
    }
    findLeaves(targetNode, []);

    if (!bestLeaf) {
      bestPathFromTarget = [targetNode];
      bestTotalSize = targetNode.t;
    }

    var fullPath = pathToTarget.slice(0, -1);
    fullPath = fullPath.concat(bestPathFromTarget);
    if (ctxPanel === 'flame') fullPath.reverse();

    var parts = [];
    for (var i = 0; i < fullPath.length; i++) parts.push(fullPath[i].n);
    var chainText = parts.join('\n');
    if (bestTotalSize > 0) {
      chainText += '\n' + (UNIT_COUNT_MODE ? '次数' : '大小') + ' ' + fmtBytes(bestTotalSize) +
        '\n占比' + ((bestTotalSize / root.t) * 100).toFixed(2) + '%';
    }
    return chainText;
  }

  // ===== 搜索绑定 =====
  function bindSearch(idx) {
    var state = states[idx];
    var chart = state.chart;
    if (!chart.searchInputId) return;
    var input = document.getElementById(chart.searchInputId);
    if (!input) return;
    input.addEventListener('input', function () {
      var term = this.value.trim().toLowerCase();
      state.searchTerm = term;
      state.searchMatchSet.clear();
      state.objectMatches = false;
      if (term) {
        var objectName = String(chart.objectName || '').toLowerCase();
        state.objectMatches = !!objectName && objectName.indexOf(term) >= 0;
        buildSearchSet(chart, state.icicleRoot, term, state.searchMatchSet);
        if (!state.icicleOnly) buildSearchSet(chart, state.flameRoot, term, state.searchMatchSet);
      }
      var countEl = chart.searchCountId ? document.getElementById(chart.searchCountId) : null;
      if (countEl) countEl.textContent = (state.searchMatchSet.size + (state.objectMatches ? 1 : 0)) + '个匹配';
      drawChart(idx);
    });
  }

  // ===== 初始化 =====
  function ensureOverlays() {
    tooltip = document.getElementById('memChartTooltip');
    if (!tooltip) {
      tooltip = document.createElement('div');
      tooltip.id = 'memChartTooltip';
      tooltip.className = 'mem-chart-tooltip';
      document.body.appendChild(tooltip);
    }
    ctxMenu = document.getElementById('memChartCtxMenu');
    if (!ctxMenu) {
      ctxMenu = document.createElement('div');
      ctxMenu.id = 'memChartCtxMenu';
      ctxMenu.className = 'mem-chart-ctxmenu';
      document.body.appendChild(ctxMenu);
    }
  }

  // 兼容 htrace：由数据条目（模板中固定名为 allData 的那个 JSON 数组）推导出图表描述，
  // 沿用既有 DOM id 约定（flameCanvasN / icicleCanvasN / downstreamCanvasN / searchInputN ...）。
  // 注意：这里不要写出数据占位符的字面量，否则会被模板注入的字符串替换误伤。
  function deriveCharts(entries) {
    var result = [];
    for (var i = 0; i < entries.length; i++) {
      var e = entries[i];
      var downstreamOnly = !!e.downstreamTree;
      result.push({
        flameTree: downstreamOnly ? null : (e.flameTree || null),
        icicleTree: e.icicleTree || null,
        downstreamTree: e.downstreamTree || null,
        names: e.names || null,
        objectName: (e.info && e.info.objectName) || '',
        greyDepth: downstreamOnly ? -1 : 0,
        useDisplayName: true,
        gateId: 'topItemBody' + i,
        flameCanvasId: downstreamOnly ? null : ('flameCanvas' + i),
        icicleCanvasId: downstreamOnly ? ('downstreamCanvas' + i) : ('icicleCanvas' + i),
        searchInputId: 'searchInput' + i,
        searchCountId: 'searchCount' + i
      });
    }
    return result;
  }

  function initChart(chart, i) {
    if (chart.names) {
      resolveNames(chart.flameTree, chart.names);
      resolveNames(chart.icicleTree, chart.names);
      resolveNames(chart.downstreamTree, chart.names);
      chart.names = null;
    }
    if (chart.greyDepth === undefined || chart.greyDepth === null) {
      chart.greyDepth = chart.downstreamTree ? -1 : 0;
    }
    if (chart.useDisplayName === undefined) chart.useDisplayName = true;

    var flameTree = chart.flameTree || null;
    var icicleTree = chart.icicleTree || chart.downstreamTree || null;

    var state = {
      idx: i,
      chart: chart,
      icicleOnly: !flameTree,
      flameCanvas: null,
      flameCtx: null,
      icicleCanvas: null,
      icicleCtx: null,
      flameRoot: flameTree,
      originalFlameRoot: flameTree,
      icicleRoot: icicleTree,
      originalIcicleRoot: icicleTree,
      flameZoomStack: [],
      icicleZoomStack: [],
      flameRectWidth: 0,
      flameXPoint: 0,
      icicleRectWidth: 0,
      icicleXPoint: 0,
      searchTerm: '',
      searchMatchSet: new Set(),
      objectMatches: false,
      flameHoveredNode: null,
      icicleHoveredNode: null,
      calleeChain: [],
      callerChain: [],
      crumbPath: [],
      activePanel: chart.icicleOnly ? 'icicle' : 'flame'
    };
    states.push(state);

    if (chart.flameCanvasId) {
      var flameEl = document.getElementById(chart.flameCanvasId);
      if (flameEl) {
        state.flameCanvas = flameEl;
        state.flameCtx = flameEl.getContext('2d');
        setupCanvasEvents(i, flameEl, 'flame');
      }
    }
    if (chart.icicleCanvasId) {
      var icicleEl = document.getElementById(chart.icicleCanvasId);
      if (icicleEl) {
        state.icicleCanvas = icicleEl;
        state.icicleCtx = icicleEl.getContext('2d');
        setupCanvasEvents(i, icicleEl, 'icicle');
      }
    }
    bindSearch(i);
    // 可选：绑定由页面提供的外部「重置」按钮
    if (chart.resetBtnId) {
      var resetBtn = document.getElementById(chart.resetBtnId);
      if (resetBtn) {
        resetBtn.addEventListener('click', (function (index) {
          return function () { resetZoom(index); };
        })(i));
      }
    }
  }

  function init(config) {
    config = config || {};
    UNIT_COUNT_MODE = !!config.unitCountMode;
    charts = config.charts ? config.charts.slice() : deriveCharts(config.entries || []);
    states = [];

    ensureOverlays();

    document.addEventListener('click', function () {
      if (ctxMenu) ctxMenu.style.display = 'none';
    });

    ctxMenu.addEventListener('click', function (e) {
      var item = e.target.closest('.ctx-item');
      if (!item) return;
      if (item.hasAttribute('data-copy-chain-largest')) {
        if (ctxNode && ctxRoot) {
          var chainText = findLargestCallChain(ctxRoot, ctxNode);
          // 恢复文案取点击时的原文，随图表业务可能是「拷贝最大引用链」或「拷贝最大调用链」
          var chainLabel = item.textContent;
          navigator.clipboard.writeText(chainText).then(function () {
            item.textContent = '已拷贝!';
            setTimeout(function () { item.textContent = chainLabel; }, 1000);
          });
        }
        ctxMenu.style.display = 'none';
        return;
      }
      var text = decodeURIComponent(item.getAttribute('data-copy') || '');
      var originalText = item.textContent;
      navigator.clipboard.writeText(text).then(function () {
        item.textContent = '已复制!';
        setTimeout(function () { item.textContent = originalText; }, 1000);
      });
      ctxMenu.style.display = 'none';
    });

    document.addEventListener('keydown', function (e) {
      if (document.activeElement && document.activeElement.classList.contains('graph-search-input')) return;
      var key = e.key.toLowerCase();
      if (key !== 'w' && key !== 's' && key !== 'a' && key !== 'd') return;
      e.preventDefault();
      if (heldKeys[key]) return;
      heldKeys[key] = true;
      execWASD(key);
      startKeyLoop(key);
    });

    document.addEventListener('keyup', function (e) {
      var key = e.key.toLowerCase();
      if (!heldKeys[key]) return;
      heldKeys[key] = false;
    });

    window.addEventListener('resize', function () { drawAllVisible(); });

    for (var i = 0; i < charts.length; i++) initChart(charts[i], i);

    requestAnimationFrame(function () {
      requestAnimationFrame(function () { drawAllVisible(); });
    });
  }

  window.MemChart = {
    init: init,
    redrawAll: drawAllVisible,
    redraw: drawChart,
    reset: resetZoom
  };
  // 保留给内联 onclick="resetZoom(N)" 使用
  window.resetZoom = resetZoom;
})();
)CHARTJS";
}

} // namespace mem_analyzer
