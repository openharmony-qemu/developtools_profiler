#pragma once
#include "core/module.hpp"
#include <string>

namespace mem_analyzer {

/// tshandle 数据域模块（输入为 ZIP：.htrace + .rawheap + js_map）。
///
/// 内部全为编译期类型（`TshandleParseResult` / `TshandleAnalysisOutput`），不经过
/// `ProcessData` / `std::any` / `from` 字符串协议；输出由域内 writer 直写文件。
///
/// **约定（P1 已决策）**：parse 阶段就包含 ArkTS 类关联，`-p` 只跳过句柄聚合，
/// 因此 parse-only 产物仍带 `arkTsClasses` 与含分析值的 `summary`
/// （见 `tshandle_parse()` 的说明）；这不是缺陷，是有意为之。
class TshandleModule final : public AnalysisModule {
public:
    /// 该域的 parser 名历史上是 "zip"，`process_file()` 会打印 `Using parser: <name>`。
    /// 保持该名以免改变既有输出（域目录已按数据域归入 `tshandle/`）。
    std::string name() const override { return "zip"; }
    bool supports_extension(const std::string& ext) const override {
        return ext == "zip" || ext == "tshandle";
    }
    ModuleCapability capability() const override;
    std::string run(const ModuleRequest& req, const RunContext& ctx) const override;
};

} // namespace mem_analyzer
