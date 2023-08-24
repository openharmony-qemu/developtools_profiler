/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef WASM_FUNC_H
#define WASM_FUNC_H

#include <cstdio>
#include <emscripten.h>
#include <string>
#include "rpc_server.h"
#include "trace_streamer/trace_streamer_selector.h"

namespace SysTuning {
namespace TraceStreamer {
extern "C" {
int32_t TraceStreamerSqlOperate(const uint8_t* sql, int32_t sqlLen);
}

} // namespace TraceStreamer
} // namespace SysTuning

#endif // RPC_WASM_FUNC_H
