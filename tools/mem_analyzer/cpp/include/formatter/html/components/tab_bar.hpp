#pragma once
#include <string>

namespace mem_analyzer {

// 内容分类 tab 组件：.tab-bar / .tab-item / .tab-content（htrace/tshandle 一致的类名）。
// 无状态自由函数，只产出待注入的 HTML/CSS/JS 片段（见 navigation_bar.hpp 的同名约定）。
// 可选组件：需要分类 tab 的报告按需注入；不需要的报告零注入。
// 类名沿用现状 .tab-item/.tab-content；.filter 归一留 Phase 2a。

// tab 栏样式（.tab-bar/.tab-item/.tab-content）。
std::string tab_bar_css();

// tab 栏开壳：<div class="tab-bar">
std::string tab_bar_open();

// 单个 tab 项：<div class="tab-item[ active]" data-tab="{id}" onclick="switchTab('{id}', this)">{label}</div>
std::string tab_bar_item(const std::string& id, const std::string& label, bool active);

// tab 栏收壳：</div>
std::string tab_bar_close();

// tab 内容开壳：<div class="tab-content[ active]" id="{id}">
std::string tab_content_open(const std::string& id, bool active);

// tab 内容收壳：</div>
std::string tab_content_close();

// tab 切换逻辑（switchTab，含 tab-content 联动；有图时 redraw）。
std::string tab_bar_js();

} // namespace mem_analyzer
