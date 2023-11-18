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

#ifndef RPC_WASM_FUNC_H
#define RPC_WASM_FUNC_H

#include <cstdio>
#ifndef IS_UT
#include <emscripten.h>
#endif
#ifdef IS_UT
#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

#ifdef __wasm__
#define EM_IMPORT(NAME) __attribute__((import_module("env"), import_name(#NAME)))
#else
#define EM_IMPORT(NAME)
#endif

#endif
#include "rpc_server.h"

namespace SysTuning {
namespace TraceStreamer {
extern "C" {
int32_t TraceStreamerParseData(const uint8_t* data, int32_t dataLen);
int32_t TraceStreamerParseDataEx(int32_t dataLen, bool isFinish);
int32_t TraceStreamerParseDataOver();
int32_t TraceStreamerSqlOperate(const uint8_t* sql, int32_t sqlLen);
int32_t TraceStreamerSqlOperateEx(int32_t sqlLen);
int32_t TraceStreamerReset();
int32_t TraceStreamerSqlQuery(const uint8_t* sql, int32_t sqlLen, uint8_t* out, int32_t outLen);
int32_t TraceStreamer_Plugin_Out_SendData(const char* pluginData, int32_t len, int32_t componentId);
int32_t TraceStreamer_Plugin_Out_Filter(const char* pluginData, int32_t len, const std::string& componentName);

int32_t TraceStreamerSqlQueryEx(int32_t sqlLen);
int32_t TraceStreamerCancel();
} // extern "C"
} // namespace TraceStreamer
} // namespace SysTuning

#endif // RPC_WASM_FUNC_H
