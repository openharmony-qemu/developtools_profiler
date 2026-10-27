#pragma once
#include "shared/htrace_model/htrace_data.hpp"
#include "core/run_context.hpp"
#include <string>

namespace mem_analyzer {

/// parse-only JSON 输出。强类型入参：不再经过 ProcessData / std::any。
/// 返回写出的文件路径。
std::string htrace_write_parse_json(const TraceDB& db, const std::string& file_path,
                                    const RunContext& ctx, const std::string& out_dir);

} // namespace mem_analyzer
