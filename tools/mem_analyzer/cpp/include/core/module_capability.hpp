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
#pragma once
#include <string>
#include <vector>

namespace mem_analyzer {

/// 单个模式（parse-only / analysis）下该模块的能力。
struct ModeCapability {
    /// 支持的格式关键词（如 "json" / "html"）。
    /// **为空 = 该模块明确不支持此模式**（如 snapshot 没有 parse 格式），此时 `-p` 会被拒绝。
    std::vector<std::string> formats;
    /// 未显式给 `-f` 时使用的格式；`formats` 为空时必须留空。
    std::string default_format;

    bool supported() const { return !formats.empty(); }
};

/// 一个模块对外声明的能力：各模式支持哪些格式、默认用哪个。
///
/// 格式关键词**直接就是路由键** —— 模块自己按 `(parse_only, keyword)` 选 writer，
/// 不存在「关键词 → formatter 名」的中间映射（P4-e 删掉 `IFormatter` / `FormatterFactory`
/// 之后，那种映射已无任何消费者）。
///
/// 形式化约定（启动期由 `ModuleRegistry::self_check()` 兜底校验）：
///   - 某模式声明了格式列表时，其 `default_format` 必须非空且是列表成员；
///   - 某模式格式列表为空（不支持该模式）时，`default_format` 必须留空。
struct ModuleCapability {
    ModeCapability parse;
    ModeCapability analysis;

    const ModeCapability& for_mode(bool parse_only) const {
        return parse_only ? parse : analysis;
    }
};

} // namespace mem_analyzer
