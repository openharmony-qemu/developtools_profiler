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
#include "shared/htrace_model/htrace_data.hpp"
#include "core/run_context.hpp"
#include <string>

namespace mem_analyzer {

/**
 * htrace 解析步骤：校验文件签名 → trace_streamer 转 DB → 读出压缩调用栈与元信息。
 * 强类型入参/出参：不再经过 ProcessData / std::any。
 */
TraceDB HtraceParse(const std::string& filePath, const RunContext& ctx);

} // namespace mem_analyzer
