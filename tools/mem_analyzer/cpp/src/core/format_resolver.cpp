/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "core/format_resolver.hpp"

namespace mem_analyzer {

FormatKeyword resolve_format_keyword(const std::string& module_name, const ModuleCapability& cap,
                                     const std::string& explicit_format, bool parse_only) {
    FormatKeyword out;
    const std::string mode_name = parse_only ? "parse-only" : "analysis";
    const ModeCapability& mode_cap = cap.for_mode(parse_only);

    // 能力表为空 = 该模块明确不支持此模式（如 snapshot 没有 parse 格式）。
    // 提前判定：否则会先兜底出一个关键词，再用它报「格式不支持」，误导用户。
    if (!mode_cap.supported()) {
        out.error = "Parser \"" + module_name + "\" does not support " + mode_name +
                    " mode. Skipping.";
        return out;
    }

    // 显式指定的 `-f` 必须可用，否则拦截：不回落默认格式，
    // 避免用户拿到与请求不符的产物。未显式指定则用该模式默认格式；
    // 默认格式缺失时取声明列表的第一个（formats 为空已在上方报错，此处必非空），
    // 保证选出来的关键词一定落在 formats 内。
    if (explicit_format.empty()) {
        out.keyword = mode_cap.default_format.empty() ? mode_cap.formats.front()
                                                      : mode_cap.default_format;
    } else {
        out.keyword = explicit_format;
    }

    for (const auto& f : mode_cap.formats) {
        if (f == out.keyword) return out;
    }
    out.error =
        "Format '" + out.keyword + "' is not supported in " + mode_name + " mode. Skipping.";
    return out;
}

} // namespace mem_analyzer
