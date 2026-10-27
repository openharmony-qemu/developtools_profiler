#pragma once
#include "core/module.hpp"
#include <string>

namespace mem_analyzer {

/// htrace 数据域模块：自持 parse → analyze → render 整条管道。
///
/// 内部全为编译期类型（`TraceDB` / `HtraceAnalysisOutput`），不经过
/// `ProcessData` / `std::any` / `from` 字符串协议；输出由域内 writer 直写文件。
class HtraceModule final : public AnalysisModule {
public:
    std::string name() const override { return "htrace"; }
    bool supports_extension(const std::string& ext) const override {
        return ext == "htrace" || ext == "htrce";
    }
    ModuleCapability capability() const override;
    std::string run(const ModuleRequest& req, const RunContext& ctx) const override;
};

} // namespace mem_analyzer
