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
#include <mutex>
#include <new>
#include <securec.h>
#include <string>

#undef LOG_DOMAIN
#define LOG_DOMAIN "0xD002D0A"
#undef LOG_TAG
#define LOG_TAG "HidebugProfiler"
using ResProfilerErrorCode = OHOS::Developtools::NativeDaemon::ResProfilerErrorCode;

static std::mutex g_callbackMutex;
static OH_HiDebug_ProfilingCallback g_callback = nullptr;
static OH_HiDebug_ResourceType g_resourceType = OH_RES_TYPE_FD;

static OH_HiDebug_ProfilingCallback TakeProfilingCallback(OH_HiDebug_ResourceType& resourceType)
{
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    resourceType = g_resourceType;
    OH_HiDebug_ProfilingCallback callback = g_callback;
    g_callback = nullptr;
    return callback;
}

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

struct OH_HiDebug_ProfilerOptions {
    uint32_t maxDuration = 900;
    uint32_t filterSize = 1024;
    uint32_t maxStackDepth = 30;
    uint32_t statisticsInterval = 10;
    uint32_t sampleInterval = 384;
    uint32_t maxAsyncNestingDepth = 1;
    uint32_t maxAsyncStackDepth = 16;
};

static constexpr uint32_t MIN_DURATION_SEC = 1;
static constexpr uint32_t MAX_DURATION_SEC = 3600;
static constexpr uint32_t MIN_FILTER_SIZE = 1;
static constexpr uint32_t MAX_STACK_DEPTH = 30;
static constexpr uint32_t MAX_STATISTICS_INTERVAL_SEC = 3600;
static constexpr uint32_t MIN_SAMPLE_INTERVAL = 384;
static constexpr uint32_t MIN_ASYNC_NESTING_DEPTH = 1;
static constexpr uint32_t MAX_ASYNC_NESTING_DEPTH = 16;
static constexpr uint32_t MIN_ASYNC_STACK_DEPTH = 1;
static constexpr uint32_t MAX_ASYNC_STACK_DEPTH = 256;

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

static void NotifyProfilingResult(const std::string& filePath)
{
    OH_HiDebug_ResourceType resourceType = OH_RES_TYPE_FD;
    OH_HiDebug_ProfilingCallback callback = TakeProfilingCallback(resourceType);
    if (callback == nullptr) {
        return;
    }
    OH_HiDebug_ProfilingResult result;
    result.resourceType = resourceType;
    result.filePath = filePath.empty() ? nullptr : filePath.c_str();
    callback(&result);
}

static void ClearProfilingCallback()
{
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_callback = nullptr;
}

static HiDebug_ErrorCode StartProfilerWithConfig(OH_HiDebug_ResourceType type,
    OHOS::Developtools::NativeDaemon::MemAPIConfig* profilerConfig, OH_HiDebug_ProfilingCallback callback)
{
    if (callback == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    {
        std::lock_guard<std::mutex> lock(g_callbackMutex);
        if (g_callback != nullptr) {
            return HIDEBUG_RES_PROF_ALREADY_STARTED;
        }
        g_callback = callback;
        g_resourceType = type;
    }

    auto profilingCallback = [](int32_t errCode, const std::string& path) {
        HiDebug_ErrorCode code = ConvertProfilerErrorCode(errCode);
        if (code == HIDEBUG_RES_PROF_SUCCESS || code == HIDEBUG_RES_PROF_AUTO_STOPPED_BY_DURATION) {
            NotifyProfilingResult(path);
            return;
        }
        NotifyProfilingResult("");
    };

    int32_t retStart =
        OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::StartProfilerByType(
            static_cast<int32_t>(type), profilerConfig, profilingCallback);
    if (retStart != static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS)) {
        ClearProfilingCallback();
        return ConvertProfilerErrorCode(retStart);
    }
    return HIDEBUG_RES_PROF_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_StartProfiler(OH_HiDebug_ResourceType type,
    OH_HiDebug_ResProfilerConfig* config, OH_HiDebug_ProfilingCallback callback)
{
    if (config == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    OHOS::Developtools::NativeDaemon::MemAPIConfig profilerConfig;
    ConvertHidebugToProfilerConfig(config, &profilerConfig);
    return StartProfilerWithConfig(type, &profilerConfig, callback);
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

OH_HiDebug_ProfilerOptions* OH_HiDebug_CreateProfilerOptions(void)
{
    return new (std::nothrow) OH_HiDebug_ProfilerOptions();
}

HiDebug_ErrorCode OH_HiDebug_DestroyProfilerOptions(OH_HiDebug_ProfilerOptions* opts)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    delete opts;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetMaxAsyncNestingDepth(OH_HiDebug_ProfilerOptions* opts,
    uint32_t depth)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (depth < MIN_ASYNC_NESTING_DEPTH || depth > MAX_ASYNC_NESTING_DEPTH) {
        return HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_NESTING_DEPTH;
    }
    opts->maxAsyncNestingDepth = depth;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetMaxAsyncTaskStackDepth(OH_HiDebug_ProfilerOptions* opts,
    uint32_t depth)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (depth < MIN_ASYNC_STACK_DEPTH || depth > MAX_ASYNC_STACK_DEPTH) {
        return HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_TASK_STACK_DEPTH;
    }
    opts->maxAsyncStackDepth = depth;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetMaxDurationSec(OH_HiDebug_ProfilerOptions* opts,
    uint32_t seconds)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (seconds < MIN_DURATION_SEC || seconds > MAX_DURATION_SEC) {
        return HIDEBUG_RES_PROF_INVALID_MAX_DURATION;
    }
    opts->maxDuration = seconds;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetFilterSize(OH_HiDebug_ProfilerOptions* opts,
    uint32_t size)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (size < MIN_FILTER_SIZE) {
        return HIDEBUG_RES_PROF_INVALID_FILTER_SIZE;
    }
    opts->filterSize = size;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetMaxStackDepth(OH_HiDebug_ProfilerOptions* opts,
    uint32_t depth)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (depth == 0 || depth > MAX_STACK_DEPTH) {
        return HIDEBUG_RES_PROF_INVALID_MAX_STACK_DEPTH;
    }
    opts->maxStackDepth = depth;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetStatisticsIntervalSec(OH_HiDebug_ProfilerOptions* opts,
    uint32_t seconds)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (seconds == 0 || seconds > MAX_STATISTICS_INTERVAL_SEC) {
        return HIDEBUG_RES_PROF_INVALID_STATISTICS_INTERVAL;
    }
    opts->statisticsInterval = seconds;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetSampleIntervalBytes(OH_HiDebug_ProfilerOptions* opts,
    uint32_t bytes)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    if (bytes < MIN_SAMPLE_INTERVAL) {
        return HIDEBUG_RES_PROF_INVALID_SAMPLE_INTERVAL;
    }
    opts->sampleInterval = bytes;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_StartProfilerWithOptions(OH_HiDebug_ResourceType type,
    OH_HiDebug_ProfilerOptions* opts, OH_HiDebug_ProfilingCallback callback)
{
    if (opts == nullptr) {
        return HIDEBUG_RES_PROF_INVALID_ARG;
    }
    OHOS::Developtools::NativeDaemon::MemAPIConfig profilerConfig;
    profilerConfig.maxDuration = opts->maxDuration;
    profilerConfig.filterSize = opts->filterSize;
    profilerConfig.maxStackDepth = opts->maxStackDepth;
    profilerConfig.statisticsInterval = opts->statisticsInterval;
    profilerConfig.sampleInterval = opts->sampleInterval;
    profilerConfig.asyncNestingDepth = opts->maxAsyncNestingDepth;
    profilerConfig.asyncTaskStackDepth = opts->maxAsyncStackDepth;
    return StartProfilerWithConfig(type, &profilerConfig, callback);
}
