#pragma once
#include "core/run_context.hpp"
#include "htrace/htrace_analyzer.hpp"
#include <string>

namespace mem_analyzer {

/// analysis HTML 输出。强类型入参：不再经过 ProcessData / std::any。
/// 返回写出的文件路径。
std::string htrace_write_analysis_html(const HtraceAnalysisOutput& full,
                                       const std::string& file_path, const RunContext& ctx,
                                       const std::string& out_dir);

} // namespace mem_analyzer
