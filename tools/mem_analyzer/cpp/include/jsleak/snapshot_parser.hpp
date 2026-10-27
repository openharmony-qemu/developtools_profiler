#pragma once
#include "core/run_context.hpp"
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include <string>

namespace mem_analyzer {

/// jsleak 解析步骤：heapsnapshot 直接加载；`.rawheap` 先用 rawheap_translator 转档；
/// 配置了 `--sourcemap` / `--namecache` 时先做反混淆。
/// 任一环节失败抛 std::runtime_error（由 `JsleakModule` 统一打印为 `Error: ...`）。
///
/// 强类型入参/出参：不再经过 ProcessData / std::any。
HeapSnapshot jsleak_parse(const std::string& file_path, const RunContext& ctx);

} // namespace mem_analyzer
