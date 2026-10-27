#pragma once
#include "shared/htrace_model/htrace_data.hpp"
#include "core/run_context.hpp"
#include <string>

namespace mem_analyzer {

/// htrace 解析步骤：校验文件签名 → trace_streamer 转 DB → 读出压缩调用栈与元信息。
/// 强类型入参/出参：不再经过 ProcessData / std::any。
TraceDB htrace_parse(const std::string& file_path, const RunContext& ctx);

} // namespace mem_analyzer
