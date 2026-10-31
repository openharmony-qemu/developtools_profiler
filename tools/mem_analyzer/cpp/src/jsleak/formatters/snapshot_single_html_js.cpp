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
#include "jsleak/formatters/snapshot_single_html_js.h"

namespace mem_analyzer {

// 单快照报告专属 JS（主 tab 切换 / hash 值展开）。
// 详情展开折叠(showDetail/toggleDetail)、节点 ID 展开(expandNodeIds)、字节格式化(bytes)
// 已统一到 report_common_js（经 core_script 注入），单快照与 compare 共用。
// 二级页签切换、引用链树表交互、冰柱图初始化已抽到 report_common 供多快照共用。
std::string SnapshotHtmlJs()
{
    return R"JS(
(function() {
  function switchTab(tabId, el) {
    var target = document.getElementById(tabId);
    if (!target) {
      target = document.querySelector('.tab-content[data-tab="' + tabId + '"]');
    }
    if (!target) {
      console.error('Tab content not found:', tabId);
      return;
    }

    var allTabs = document.querySelectorAll('.tab-content');
    for (var i = 0; i < allTabs.length; i++) {
      allTabs[i].style.display = 'none';
      allTabs[i].classList.remove('active');
    }

    target.style.display = 'block';
    target.classList.add('active');

    var allItems = document.querySelectorAll('.tab-item');
    for (var i = 0; i < allItems.length; i++) {
      allItems[i].classList.remove('active');
    }
    if (el) {
      el.classList.add('active');
    } else {
      var items = document.querySelectorAll('.tab-item');
      for (var i = 0; i < items.length; i++) {
        if (items[i].getAttribute('data-tab') === tabId) {
          items[i].classList.add('active');
        }
      }
    }

    // 冰柱图在隐藏 tab 中宽度为 0 无法绘制，切换到可见后重绘
    if (window.icicleRedrawAll) { window.icicleRedrawAll(); }
  }

  function expandHashValues(containerId) {
    var container = document.getElementById(containerId);
    var btnId = "expandBtn_" + containerId;
    var btn = document.getElementById(btnId);
    if (!container || !btn) return;
    var allHashes = container.getAttribute('data-all-hashes');
    if (!allHashes) return;
    var hashes = allHashes.split(',');
    var isExpanded = btn.getAttribute('data-expanded') === 'true';
    if (isExpanded) {
      var html = 'hash值: ';
      for (var i = 0; i < 10 && i < hashes.length; i++) {
        html += '<span class="node-id-tag">' + hashes[i] + '</span>';
      }
      if (hashes.length > 10) {
        html += '<span class="node-id-tag expand-btn" id="' + btnId + '" onclick="expandHashValues(\'' + containerId + '\')">展开</span>';
      }
      container.innerHTML = html;
    } else {
      var html = 'hash值: ';
      for (var i = 0; i < hashes.length; i++) {
        html += '<span class="node-id-tag">' + hashes[i] + '</span>';
      }
      html += '<span class="node-id-tag expand-btn" id="' + btnId +  '" data-expanded="true" onclick="expandHashValues(\'' + containerId + '\')">收起</span>';
      container.innerHTML = html;
    }
  }

  window.switchTab = switchTab;
  window.expandHashValues = expandHashValues;
})();
)JS";
}

} // namespace mem_analyzer
