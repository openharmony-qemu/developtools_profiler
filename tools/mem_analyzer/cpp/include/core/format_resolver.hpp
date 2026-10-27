#pragma once
#include <string>

#include "core/module_capability.hpp"

namespace mem_analyzer {

/// 格式关键词的解析结果。`error` 非空表示该请求在当前模式下不可用。
struct FormatKeyword {
    std::string keyword;  ///< 归一化后的格式关键词（如 "json" / "html"）
    std::string error;
};

/// 只决定「用哪个格式关键词」，不涉及 writer 实例——各域的 writer 都是自己模块里的
/// 自由函数，模块只需要知道走哪条输出分支。
///
/// 规则（与历史 pipeline 行为一致）：
///   1. 该模式的格式列表为空 = 模块明确不支持此模式 → 立即报错（不用兜底关键词误导用户）；
///   2. 显式 `-f`：直接作为关键词，落空（不在该模式格式列表里）→ 报「格式不支持」，
///      **不回落默认格式**；
///   3. 未显式指定：用该模式的默认格式；默认缺失时沿用历史兜底关键词 "json"；
///   4. 最后用该模式的格式列表校验关键词。
FormatKeyword resolve_format_keyword(const std::string& module_name, const ModuleCapability& cap,
                                     const std::string& explicit_format, bool parse_only);

} // namespace mem_analyzer
