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

// html_components 下的组件统一为「命名空间内自由函数」：无状态、只产出待注入模板的
// HTML/CSS/JS 片段，由各 formatter 通过占位符注入（见 chart_module.h 的同名约定）。

// 设置导航栏格式
std::string NavBarFormat();

// 构建标题
std::string NavBarTitle();

// 构建资料文档导航栏内容
std::string NavBarDocContent();

// 构建帮助中心导航栏内容
std::string NavBarHelpCenterContent();

// 设置切换导航栏函数
std::string NavBarSwitchNavFunc();

}
