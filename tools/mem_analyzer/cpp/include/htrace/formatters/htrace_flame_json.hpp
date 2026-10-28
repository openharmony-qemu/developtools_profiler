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

// htrace_flame_json.hpp — htrace 专用的火焰图/冰柱图 JSON 构建
//
// 这三个函数原先放在 `formatter/html/html_analysis_common.*`（三域共享头）里，
// 使得共享头反向依赖 `htrace/flame_icicle.hpp` 与 `htrace/htrace_analyzer.hpp`。
// 它们的唯一调用方是 htrace 域的 HTML writer，因此下沉回 htrace 域——
// 共享头从此不依赖任何数据域。

#include "formatter/html/html_analysis_common.hpp"
#include "htrace/flame_icicle.hpp"
#include "htrace/htrace_analyzer.hpp"

#include <string>
#include <unordered_map>

namespace mem_analyzer {

/// 从 FlameNode 直接构建火焰图 JSON 树（跳过 to_json → buildFlameTreeJson 的中间步骤）。
/// 根节点使用 TopSymbolInfo 的符号信息（FlameNode 根节点是合成节点，symbol_id/file_id 为空）。
json BuildFlameTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName,
    const std::string& rootLib, const std::string& rootLibOffset,
    const std::unordered_map<int, std::string>& dataDict,
    htmlAnalysis::StringTable& st);

/// 从 FlameNode 直接构建冰柱图 JSON 树（跳过中间 JSON）。
json BuildIcicleTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName,
    const std::string& rootLib, const std::string& rootLibOffset,
    const std::unordered_map<int, std::string>& dataDict,
    htmlAnalysis::StringTable& st);

/// 从 TopSymbolInfo 构建轻量 SymbolDataEntry（仅 info + callChains，不含 flameTree/icicleTree）。
/// 用于延迟构建模式：flameTree/icicleTree 在流式写入时按需构建，大幅降低峰值内存。
htmlAnalysis::SymbolDataEntry BuildSymbolDataEntryFromContextLight(
    const TopSymbolInfo& ts, long long totalExistingSize,
    const std::unordered_map<int, std::string>& dataDict);

} // namespace mem_analyzer
