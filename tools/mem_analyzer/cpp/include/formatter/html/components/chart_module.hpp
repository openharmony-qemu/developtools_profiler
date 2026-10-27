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

// chart_module.hpp — 火焰图/冰柱图共享图表模块
//
// 把原先内联在 html_analysis_common.cpp 模板里的图表引擎（布局、尺寸刻度、
// 缩放平移、悬停 tooltip、右键菜单、搜索高亮、下钻与祖先链、重置、resize 自适应）
// 抽成独立模块，供 htrace / tshandle / snapshot 等报告复用，避免多份实现分叉。
//
// 用法：
//   1. 在页面模板中注入 __CHART_MODULE_CSS__ 与 __CHART_MODULE_JS__ 占位符；
//   2. 页面脚本调用 MemChart.init({...}) 注册图表并绑定交互。
//
// JS 侧 API（window.MemChart）：
//   MemChart.init(config)  注册所有图表并绑定交互
//   MemChart.redrawAll()   重绘全部图表（如切换 tab 后调用）
//   MemChart.reset(idx)    重置第 idx 个图表的缩放/搜索状态
//   window.resetZoom(idx)  同上，保留给内联 onclick 使用

#include <string>

// 与 navigation_bar.hpp 一致：html_components 下的组件统一为「命名空间内自由函数」，
// 无状态、只产出待注入模板的 HTML/CSS/JS 片段。
namespace mem_analyzer {

// 图表模块的 CSS（工具栏/搜索框/快捷键提示/tooltip/右键菜单）。
// 注入占位符：__CHART_MODULE_CSS__
std::string chart_module_css();

// 图表模块的 JS 引擎（IIFE，暴露 window.MemChart）。
// 注入占位符：__CHART_MODULE_JS__
std::string chart_module_js();

} // namespace mem_analyzer
