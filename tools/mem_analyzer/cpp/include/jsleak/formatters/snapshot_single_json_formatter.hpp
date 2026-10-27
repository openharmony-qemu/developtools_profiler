#pragma once
#include "core/run_context.hpp"
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

/// jsleak 分析结果的 JSON 输出：顶层 meta + content，结构对齐 docs/jsleak_output_example.json。
/// 入参为 `jsleak_analyze()` 产出的结果 json；writer 只做纯重组，不改分析层。
/// 返回写出的文件路径。
std::string jsleak_write_analysis_json(const json& data, const std::string& file_path,
                                       const RunContext& ctx, const std::string& out_dir);

} // namespace mem_analyzer
