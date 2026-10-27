#pragma once
#include <string>

#include "core/module.hpp"
#include "core/run_context.hpp"

namespace mem_analyzer {

/// 单个文件的处理流程，供 trace / tshandle / jsleak 三个分析类命令与 auto-detect 共用。
///
/// L0 只负责：路径长度检查、为当前文件派生一份上下文（输出目录嵌套 source stem）、
/// 以及 `--auto-open`；parse → analyze → render 全部交给模块。
///
/// `base_ctx` 为运行级参数，本函数不修改它。
/// 单文件通用管道：选模块 → 派生 RunContext → module.run → 按需自动打开。
/// 返回 false = 处理失败（路径超限 / 模块返回空产物），调用方应传导为非零退出码。
bool process_file(const std::string& file_path, AnalysisModule& module, const RunContext& base_ctx);

} // namespace mem_analyzer
