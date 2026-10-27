#pragma once
#include <string>

namespace mem_analyzer {

/// 调用 trace_streamer 将 .htrace 转为 SQLite DB（htrace 与 tshandle 共用）。
///
/// 统一处理：取 streamer 路径 → 转绝对路径 → 组 `-e <db> <htrace>` 参数 → 可选
/// `--So_dir <dir>` → 执行转换。本函数只返回成功/失败与失败原因，不打印、不抛
/// 异常——失败后由调用方决定如何呈现（当前两处都抛 std::runtime_error）。
///
/// @param htrace_path 输入的 .htrace 文件
/// @param db_path     输出的 SQLite DB 文件（父目录需已存在）
/// @param so_dir      可选 native .so 符号表目录（空则跳过 --So_dir）
/// @param error       可选，失败原因；成功时保持不变
/// @return 转换成功返回 true；streamer 缺失 / 转换非零退出返回 false
bool run_trace_streamer(const std::string& htrace_path,
                        const std::string& db_path,
                        const std::string& so_dir,
                        std::string* error = nullptr);

} // namespace mem_analyzer
