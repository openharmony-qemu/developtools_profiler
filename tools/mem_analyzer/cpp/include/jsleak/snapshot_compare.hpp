#pragma once

#include "formatter/json/json_writer.hpp"
#include "shared/heapsnapshot/heap_snapshot.hpp"

#include <string>

namespace mem_analyzer {

/// 校验快照对比的输入：双端扩展名均须为 .heapsnapshot / .rawheap 且格式一致。
/// 返回 false 时 error 说明原因（供模块层在解析前快速失败）。
bool jsleak_compare_inputs_valid(const std::string& baseline_file,
                                 const std::string& target_file,
                                 std::string& error);

/// 快照对比分析：双端已解析的 HeapSnapshot → Schema v6 对比报告 json。
/// 渲染由 jsleak/formatters/snapshot_compare_formatter 的 writer 负责。
///
/// mode 选择输出内容：
///   "all"           —— 属性标签泄漏对比 + 最短引用链对比（默认）
///   "property-tag"  —— 仅属性标签泄漏对比（leakObjInfo）
///   "top-chain"     —— 仅最短引用链对比（pathDeltas）
/// top_n 为每个方向（增长/缩减）的条数上限。
json jsleak_compare_analyze(const HeapSnapshot& baseline,
                            const HeapSnapshot& target,
                            const std::string& baseline_file,
                            const std::string& target_file,
                            int top_n,
                            const std::string& mode);

} // namespace mem_analyzer
