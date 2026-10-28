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

#include "core/module_capability.hpp"
#include "core/run_context.hpp"

namespace mem_analyzer {

/// 单个文件的处理请求。
struct ModuleRequest {
    std::string filePath;
    bool parseOnly = false;
    std::string format;  ///< 显式 `-f`；空 = 用模块的默认格式
};

/// 一个数据域 = 一个模块，自持 parse → analyze → render 整条管道。
///
/// 这是 L0 与数据域之间唯一的接口：除 `Run()` 之外，管道层不知道模块内部怎么切分。
/// 类型在模块内编译期确定，不再经过 `ProcessData` / `std::any` / `from` 字符串协议。
class AnalysisModule {
public:
    virtual ~AnalysisModule() = default;

    /// 模块名（沿用历史 parser 名：htrace / zip / snapshot）。
    virtual std::string Name() const = 0;

    /// 是否接受该扩展名（小写、不含点）。
    virtual bool SupportsExtension(const std::string& ext) const = 0;

    /// 当前能力表。调用期求值，避免静态初始化顺序问题。
    virtual ModuleCapability Capability() const = 0;

    /// 跑完整条管道并把产物写到 `ctx.output.out_dir`。
    /// 返回写出的文件路径（供 `--auto-open` 使用）；被跳过或失败时返回空串。
    virtual std::string Run(const ModuleRequest& req, const RunContext& ctx) const = 0;
};

} // namespace mem_analyzer
