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
#pragma once
#include <string>

namespace mem_analyzer {

// 内容分类 tab 组件：.tab-bar / .tab-item / .tab-content（htrace/tshandle 一致的类名）。
// 无状态自由函数，只产出待注入的 HTML/CSS/JS 片段（见 navigation_bar.hpp 的同名约定）。
// 可选组件：需要分类 tab 的报告按需注入；不需要的报告零注入。
// 类名沿用现状 .tab-item/.tab-content；.filter 归一留 Phase 2a。

// tab 栏样式（.tab-bar/.tab-item/.tab-content）。
std::string TabBarCss();

// tab 栏开壳：<div class="tab-bar">
std::string TabBarOpen();

// 单个 tab 项：<div class="tab-item[ active]" data-tab="{id}" onclick="switchTab('{id}', this)">{label}</div>
std::string TabBarItem(const std::string& id, const std::string& label, bool active);

// tab 栏收壳：</div>
std::string TabBarClose();

// tab 内容开壳：<div class="tab-content[ active]" id="{id}">
std::string TabContentOpen(const std::string& id, bool active);

// tab 内容收壳：</div>
std::string TabContentClose();

// tab 切换逻辑（switchTab，含 tab-content 联动；有图时 redraw）。
std::string TabBarJs();

} // namespace mem_analyzer
