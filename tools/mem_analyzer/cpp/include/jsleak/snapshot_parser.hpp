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
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include <string>

namespace mem_analyzer {

/// jsleak 解析步骤：heapsnapshot 直接加载；`.rawheap` 先用 rawheap_translator 转档；
/// 配置了 `--sourcemap` / `--namecache` 时先做反混淆。
/// 任一环节失败抛 std::runtime_error（由 `JsleakModule` 统一打印为 `Error: ...`）。
///
/// 强类型入参/出参：不再经过 ProcessData / std::any。
HeapSnapshot JsleakParse(const std::string& filePath, const RunContext& ctx);

} // namespace mem_analyzer
