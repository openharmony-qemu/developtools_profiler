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
#include "formatter/html/components/tab_bar.hpp"

namespace mem_analyzer {

std::string TabBarCss()
{
    return R"CSS(
.tab-bar { margin: 20px 20px 0; }
.tab-item { display: inline-block; padding: 10px 16px 8px; margin-right: 8px; cursor: pointer; font-size: 16px; font-weight: 600; color: #888; border-bottom: 3px solid transparent; margin-bottom: -2px; transition: color 0.2s, border-color 0.2s; }
.tab-item:hover { color: #e94560; }
.tab-item.active { color: #e94560; border-bottom-color: #e94560; }
.tab-content { margin: 0 20px; }
)CSS";
}

std::string TabBarOpen()
{
    return R"HTML(<div class="tab-bar">)HTML";
}

std::string TabBarItem(const std::string& id, const std::string& label, bool active)
{
    return "<div class=\"tab-item" + std::string(active ? " active" : "") + "\" data-tab=\"" + id +
           "\" onclick=\"switchTab('" + id + "', this)\">" + label + "</div>";
}

std::string TabBarClose()
{
    return "</div>";
}

std::string TabContentOpen(const std::string& id, bool active)
{
    return "<div class=\"tab-content" + std::string(active ? " active" : "") + "\" id=\"" + id +
           "\">";
}

std::string TabContentClose()
{
    return "</div>";
}

std::string TabBarJs()
{
    return R"JS(
function switchTab(tabId, tabEl)
{
  var tabs = document.querySelectorAll('.tab-item');
  for (var i = 0; i < tabs.length; i++) {
      tabs[i].classList.remove('active');
  }
  if (tabEl) {
      tabEl.classList.add('active');
  }
  var contents = document.querySelectorAll('.tab-content');
  for (var i = 0; i < contents.length; i++) {
      contents[i].classList.remove('active');
  }
  var content = document.getElementById(tabId);
  if (content) {
      content.classList.add('active');
  }
  if (window.MemChart) {
      MemChart.redrawAll();
  }
}
window.switchTab = switchTab;
)JS";
}

} // namespace mem_analyzer
