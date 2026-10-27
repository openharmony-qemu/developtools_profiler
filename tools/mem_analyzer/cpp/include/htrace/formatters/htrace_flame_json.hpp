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
json buildFlameTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName,
                                     const std::string& rootLib, const std::string& rootLibOffset,
                                     const std::unordered_map<int, std::string>& data_dict,
                                     html_analysis::StringTable& st);

/// 从 FlameNode 直接构建冰柱图 JSON 树（跳过中间 JSON）。
json buildIcicleTreeJsonFromFlameNode(const FlameNode& node, const std::string& rootName,
                                      const std::string& rootLib, const std::string& rootLibOffset,
                                      const std::unordered_map<int, std::string>& data_dict,
                                      html_analysis::StringTable& st);

/// 从 TopSymbolInfo 构建轻量 SymbolDataEntry（仅 info + callChains，不含 flameTree/icicleTree）。
/// 用于延迟构建模式：flameTree/icicleTree 在流式写入时按需构建，大幅降低峰值内存。
html_analysis::SymbolDataEntry buildSymbolDataEntryFromContextLight(
    const TopSymbolInfo& ts, long long total_existing_size,
    const std::unordered_map<int, std::string>& data_dict);

} // namespace mem_analyzer
