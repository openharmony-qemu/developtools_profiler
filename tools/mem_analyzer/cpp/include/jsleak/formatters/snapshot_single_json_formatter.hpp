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
#include "core/run_context.hpp"
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

/**
 * jsleak 分析结果的 JSON 输出：顶层 meta + content，结构对齐 docs/jsleak_output_example.json。
 * 入参为 `jsleak_analyze()` 产出的结果 json；writer 只做纯重组，不改分析层。
 * 返回写出的文件路径。
 */
std::string JsleakWriteAnalysisJson(const json& data, const std::string& filePath,
    const RunContext& ctx, const std::string& outDir);

} // namespace mem_analyzer
