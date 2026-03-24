/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hidebug/hidebug.h"
#include "native_memory_profiler_sa_client_manager.h"

#include "hilog/log.h"

#include <map>

#undef LOG_DOMAIN
#define LOG_DOMAIN "0xD002D0A"
#undef LOG_TAG
#define LOG_TAG "HidebugProfiler"
using ResProfilerErrorCode = OHOS::Developtools::NativeDaemon::ResProfilerErrorCode;
void ConvertHidebugToProfilerConfig(
    OH_HiDebug_ResProfilerConfig* src, OHOS::Developtools::NativeDaemon::MemAPIConfig* dst)
{
    if (src == nullptr || dst == nullptr) {
        return;
    }
    dst->maxDuration = src->maxDuration;
    dst->filterSize = src->filterSize;
    dst->statisticsInterval = src->statisticsInterval;
    dst->sampleInterval = src->sampleInterval;
    dst->maxStackDepth = src->maxStackDepth;
}

std::map<int32_t, HiDebug_ErrorCode> resProfErrorCodeMap = {
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_SUCCESS},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_ARG},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MAX_DURATION_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_MAX_DURATION},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_FILTER_SIZE_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_FILTER_SIZE},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MAX_STACK_DEPTH_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_MAX_STACK_DEPTH},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_STATISTICS_INTERVAL},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SAMPLE_INTERVAL_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_SAMPLE_INTERVAL},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_INVALID_RESOURCE_TYPE},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_PERMISSION_DENIED},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ALREADY_STARTED),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_ALREADY_STARTED},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_NOT_STARTED},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PROCESS_OVERLIMIT),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_PROCESS_OVERLIMIT},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_CONFLICT},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_AUTO_STOPPED_BY_DURATION),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_AUTO_STOPPED_BY_DURATION},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_DAILY_QUOTA_EXCEEDED},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_CPU_OVERLOADED},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_MEM_PRESSURE_CRITICAL},
    {static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STORAGE_PRESSURE_CRITICAL),
        HiDebug_ErrorCode::HIDEBUG_RES_PROF_STORAGE_PRESSURE_CRITICAL},
};

HiDebug_ErrorCode ConvertProfilerErrorCode(int32_t errCode)
{
    auto it = resProfErrorCodeMap.find(errCode);
    return (it != resProfErrorCodeMap.end()) ? it->second : HiDebug_ErrorCode::HIDEBUG_RES_PROF_FAILURE;
}

HiDebug_ErrorCode OH_HiDebug_StartProfiler(OH_HiDebug_ResourceType type, OH_HiDebug_ResProfilerConfig* config)
{
    if (config == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    OHOS::Developtools::NativeDaemon::MemAPIConfig profilerConfig;
    ConvertHidebugToProfilerConfig(config, &profilerConfig);
    int32_t retStart =
        OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::StartProfilerByType(
            static_cast<int32_t>(type), &profilerConfig);
    HiDebug_ErrorCode ret = ConvertProfilerErrorCode(retStart);
    if (ret != HIDEBUG_RES_PROF_SUCCESS) {
        return ret;
    }
    return HIDEBUG_RES_PROF_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_StopProfiler()
{
    int32_t retStop = OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::Stop();
    if (retStop != 0) {
        HiDebug_ErrorCode ret = ConvertProfilerErrorCode(retStop);
        return ret;
    }
    return HIDEBUG_RES_PROF_SUCCESS;
}
