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
#include "core/module.hpp"
#include <string>

namespace mem_analyzer {

/// htrace 数据域模块：自持 parse → analyze → render 整条管道。
///
/// 内部全为编译期类型（`TraceDB` / `HtraceAnalysisOutput`），不经过
/// `ProcessData` / `std::any` / `from` 字符串协议；输出由域内 writer 直写文件。
class HtraceModule final : public AnalysisModule {
public:
    std::string Name() const override { return "htrace"; }
    bool SupportsExtension(const std::string& ext) const override
    {
        return ext == "htrace" || ext == "htrce";
    }
    ModuleCapability Capability() const override;
    std::string Run(const ModuleRequest& req, const RunContext& ctx) const override;
};

} // namespace mem_analyzer
