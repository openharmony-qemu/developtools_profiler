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

// 页脚版本号组件：footer 样式与内容片段。
// 无状态、只产出待注入的 CSS/HTML 片段（见 navigation_bar.hpp 的同名约定）。
// 三域（htrace/tshandle/jsleak）通用。

// footer 页脚样式（对应占位符 __VERSION_FORMAT__）。
std::string version_format();

// footer 页脚内容（含工具版本号，对应占位符 __VERSION_CONTENT__）。
std::string version_content();

} // namespace mem_analyzer
