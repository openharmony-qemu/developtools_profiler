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

// help-icon / help-tooltip 公共组件：自适应 tooltip（视口边缘自动翻转/夹紧，
// 避免文字被裁切）的 CSS 样式与初始化脚本。无状态、只产出待注入的片段，
// 由各 HTML 报告通过占位符或流式拼接注入（见 navigation_bar.h 的同名约定）。

// help-icon / help-tooltip 样式（含箭头与 above/below 翻转态）
std::string HelpTooltipCss();

// initHelpTooltip 脚本（函数定义 + 调用，注入在 </script> 之前）
std::string HelpTooltipJs();

}
