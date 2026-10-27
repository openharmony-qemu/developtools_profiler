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
#include "formatter/json/json_stream_writer.hpp"
#include "nlohmann/json.hpp"
#include "shared/htrace_model/htrace_data.hpp"
#include <string>

using json = nlohmann::ordered_json;

namespace mem_analyzer {

/// htrace 输出的 meta（含 sourceFileInfo）。`command` 取自 ctx.command_line。
json htrace_build_meta(const std::string& file_path, const RunContext& ctx);

/// htrace summary 的公共字段（isStatistics / duration / 时间戳 / 进程名）。
/// 各 writer 再补自己的资源类型字段。
json htrace_build_summary_common(const std::string& mode, long long duration_ns,
                                 long long start_ms, long long end_ms,
                                 const std::string& process_name);

/// 取指定资源类型对应的 summary 总计值。
long long htrace_summary_total_for_type(const json& htrace_summary, const std::string& category);

/// 写单个压缩帧（depth/symbol/lib/libOffset/buildId）。
/// analysis 与 parse 两个 JSON 写器共用，保证调用栈帧字节一致。
void htrace_write_callstack_frame(JsonStreamWriter& writer,
                                  const CompressedStackFrame& frame,
                                  const DataDict& dict);

} // namespace mem_analyzer
