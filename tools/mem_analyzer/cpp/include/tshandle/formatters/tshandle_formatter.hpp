#pragma once
#include "core/run_context.hpp"
#include "tshandle/tshandle_processor.hpp"
#include <string>

namespace mem_analyzer {

/// parse-only JSON 输出（含 `arkTsClasses`，见 `tshandle_parse()` 的约定）。
/// 强类型入参：不再经过 ProcessData / std::any。
/// 返回写出的文件路径。
std::string tshandle_write_parse_json(const TshandleParseResult& parsed, const std::string& file_path,
                                      const RunContext& ctx, const std::string& out_dir);

} // namespace mem_analyzer
