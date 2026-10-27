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
    bool SupportsExtension(const std::string& ext) const override
    {
        return ext == "heapsnapshot" || ext == "rawheap";
    }
    ModuleCapability capability() const override;
    std::string run(const ModuleRequest& req, const RunContext& ctx) const override;
};

} // namespace mem_analyzer
