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
#ifndef TS_SDK_API_H
#define TS_SDK_API_H
#include "rpc/rpc_server.h"

#include <string>
namespace SysTuning {
namespace TraceStreamer {
extern "C" {
extern bool g_isUseExternalModify;
int32_t SDK_SetTableName(const char* counterTableName,
                         const char* counterObjectTableName,
                         const char* sliceTableName,
                         const char* sliceObjectName);
int32_t SDK_AppendCounterObject(int32_t counterId, const char* columnName);
int32_t SDK_AppendCounter(int32_t counterId, uint64_t ts, double value);
int32_t SDK_AppendSliceObject(int32_t sliceId, const char* columnName);
int32_t SDK_AppendSlice(int32_t sliceId,
                        uint64_t ts,
                        uint64_t endTs,
                        std::string start_time,
                        std::string end_time,
                        double value);
void SetRpcServer(RpcServer* rpcServer);
}
} // namespace TraceStreamer
} // namespace SysTuning
#endif
