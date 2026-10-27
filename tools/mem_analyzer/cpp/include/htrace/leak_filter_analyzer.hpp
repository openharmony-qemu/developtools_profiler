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
#include "htrace/htrace_analyzer.hpp"
#include <unordered_set>
#include <cstdint>

namespace mem_analyzer {

// L1: 按 Native Leaks 不可达内存泄漏文件地址过滤（栈 existing_addrs 与 leaks 求交集，非空才留）。
// parse 后调用，解析/分析模式均执行。leaks 为空时直接返回（无操作）。
void apply_leak_addr_filter(TraceDB& db, const std::unordered_set<uint64_t>& leaks);

// -l 分析模式（不带 -p）：
//   L2 __emutls_get_address 排除 → L3 栈分 app/system（arkweb 例外）→
//   按 scope 走 analyze_call_stacks(leak_mode)：
//     app-only/system-only 各跑一遍；app-and-system 跑两遍（app+system）；mixed 不分栈跑一遍。
//   每组 top N + other（非 top N 覆盖的栈合并火焰图）。
//   组装 HtraceAnalysisOutput。L1 已由 apply_leak_addr_filter 在 parse 后完成。
HtraceAnalysisOutput htrace_leak_analyze(TraceDB& db, const RunContext& ctx);

} // namespace mem_analyzer
