#pragma once
#include "core/run_context.hpp"
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

/// jsleak 分析结果的 HTML 输出（自包含交互式报告：属性标签 / 引用链两个 Tab + 冰柱图）。
/// 入参为 `jsleak_analyze()` 产出的结果 json；writer 只做纯重组，不改分析层。
/// 返回写出的文件路径。
std::string jsleak_write_analysis_html(const json& data, const std::string& file_path,
                                       const RunContext& ctx, const std::string& out_dir);

} // namespace mem_analyzer
