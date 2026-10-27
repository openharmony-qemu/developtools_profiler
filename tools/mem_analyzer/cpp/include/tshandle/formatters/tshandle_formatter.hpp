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
#include "tshandle/tshandle_processor.hpp"
#include <string>

namespace mem_analyzer {

/// parse-only JSON 输出（含 `arkTsClasses`，见 `tshandle_parse()` 的约定）。
/// 强类型入参：不再经过 ProcessData / std::any。
/// 返回写出的文件路径。
std::string tshandle_write_parse_json(const TshandleParseResult& parsed, const std::string& file_path,
                                      const RunContext& ctx, const std::string& out_dir);

} // namespace mem_analyzer
