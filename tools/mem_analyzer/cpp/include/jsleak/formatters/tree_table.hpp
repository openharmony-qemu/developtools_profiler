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
#include "formatter/json/json_writer.hpp"

namespace mem_analyzer {

// 数据树表组件：树 JSON → 可折叠树表 HTML（jsleak 域：引用链总览图下方数据树表）。
// 无状态自由函数，只产出待注入的 CSS/HTML 片段。
// 树 JSON 契约（与 MemChart 冰柱图共用）：n=名称 / t=大小 / cnt=数量 / c=子节点数组。
// 数据构建见 icicle_tree.hpp 的 build_ref_chain_icicle_tree（引用链 → 前缀树）。

// 树表样式（.icicle-tree-* / .itt-*）。
std::string TreeTableCss();

// 树 JSON → 可折叠树表 HTML（含全部展开/全部折叠/复制引用链按钮）。
// canvas_id 为对应冰柱图的 canvas id，内部用 "treeTable_" + canvas_id 作为表格 id。
std::string TreeTableHtml(const json& tree, const std::string& canvasId);

} // namespace mem_analyzer