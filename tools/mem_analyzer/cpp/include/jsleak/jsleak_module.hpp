#pragma once
#include "core/module.hpp"
#include <string>

namespace mem_analyzer {

/// jsleak 数据域模块（输入 heapsnapshot / rawheap）。
///
/// 内部为编译期类型：解析产出 `HeapSnapshot`，分析产出结果 json（见
/// `jsleak_analyze()` 的说明——分析层保留「提前 JSON 化」的既有设计），
/// 不再经过 `ProcessData` / `std::any` / `from` 字符串协议；输出由域内 writer 直写文件。
///
/// 该域**不支持 parse-only**（能力表不声明 parse 格式），`-p` 会被明确拒绝。
class JsleakModule final : public AnalysisModule {
public:
    /// 该域的 parser 名历史上是 "snapshot"，`process_file()` 会打印 `Using parser: <name>`。
    /// 保持该名以免改变既有输出（域目录按 CLI 命令名归入 `jsleak/`）。
    std::string name() const override { return "snapshot"; }
    bool supports_extension(const std::string& ext) const override {
        return ext == "heapsnapshot" || ext == "rawheap";
    }
    ModuleCapability capability() const override;
    std::string run(const ModuleRequest& req, const RunContext& ctx) const override;
};

} // namespace mem_analyzer
