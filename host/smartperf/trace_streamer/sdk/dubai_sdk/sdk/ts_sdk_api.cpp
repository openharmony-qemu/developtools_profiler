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
#include "ts_sdk_api.h"
namespace SysTuning {
namespace TraceStreamer {
extern "C" {
RpcServer* rpcServer_;
bool g_isUseExternalModify = true;
int32_t SDK_SetTableName(const char* counterTableName,
                         const char* counterObjectTableName,
                         const char* sliceTableName,
                         const char* sliceObjectName)
{
    rpcServer_->ts_->sdkDataParser_->SetTableName(counterTableName, counterObjectTableName, sliceTableName,
                                                  sliceObjectName);
    if (g_isUseExternalModify) {
        TS_LOGE("If you want to use the SDK_SetTableName, please modify g_isUseExternalModify to false.");
    }
    return 0;
}

int32_t SDK_AppendCounterObject(int32_t counterId, const char* columnName)
{
    return rpcServer_->ts_->sdkDataParser_->AppendCounterObject(counterId, columnName);
}
int32_t SDK_AppendCounter(int32_t counterId, uint64_t ts, double value)
{
    return rpcServer_->ts_->sdkDataParser_->AppendCounter(counterId, ts, value);
}
int32_t SDK_AppendSliceObject(int32_t sliceId, const char* columnName)
{
    return rpcServer_->ts_->sdkDataParser_->AppendSliceObject(sliceId, columnName);
}
int32_t SDK_AppendSlice(int32_t sliceId,
                        uint64_t ts,
                        uint64_t endTs,
                        std::string start_time,
                        std::string end_time,
                        double value)
{
    return rpcServer_->ts_->sdkDataParser_->AppendSlice(sliceId, ts, endTs, start_time, end_time, value);
}
void SetRpcServer(RpcServer* rpcServer)
{
    rpcServer_ = std::move(rpcServer);
}
}
} // namespace TraceStreamer
} // namespace SysTuning
