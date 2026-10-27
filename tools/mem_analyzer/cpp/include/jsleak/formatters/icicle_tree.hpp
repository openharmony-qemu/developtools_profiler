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
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

json build_ref_chain_icicle_tree(const json& chain_items, const std::string& root_name);

// 引用链总览图（冰柱图）骨架：工具栏/面包屑/画布 + 数据树表 HTML。
// 数据树表本身由 tree_table 组件生成（jsleak/formatters/tree_table.hpp）。
std::string build_icicle_chart_body(const std::string& canvasId,
                                    const std::string& treeTableHtml);

} // namespace mem_analyzer
