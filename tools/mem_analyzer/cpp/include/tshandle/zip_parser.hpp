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
#include "core/run_context.hpp"
#include "tshandle/tshandle_processor.hpp"
#include <string>

namespace mem_analyzer {

/// tshandle 解析步骤：解包 ZIP → trace_streamer 转 DB → 提取 ARK handle 调用栈
/// →（有 rawheap 时）转 heapsnapshot 并做 ArkTS 类关联。
/// **约定（P1 已决策）**：parse 阶段就包含 ArkTS 类关联；`-p` 只跳过句柄聚合，
/// 因此 `tshandle -p -f json` 的产物仍带 `arkTsClasses` 与含分析值的 `summary`。
/// 这不是缺陷，是有意为之；模块化必须保持该行为。
/// 强类型入参/出参：不再经过 ProcessData / std::any。
TshandleParseResult TshandleParse(const std::string& filePath, const RunContext& ctx);

} // namespace mem_analyzer
