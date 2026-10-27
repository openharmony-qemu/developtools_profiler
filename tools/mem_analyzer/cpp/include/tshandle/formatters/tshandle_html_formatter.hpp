#pragma once
#include "core/run_context.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include <string>

namespace mem_analyzer {

/// analysis HTML 输出（global / local handle 两个 Tab + 引用关系图）。
/// 强类型入参：不再经过 ProcessData / std::any。
/// 返回写出的文件路径。
std::string tshandle_write_analysis_html(const TshandleAnalysisOutput& analysis,
                                         const std::string& file_path, const RunContext& ctx,
                                         const std::string& out_dir);

} // namespace mem_analyzer
