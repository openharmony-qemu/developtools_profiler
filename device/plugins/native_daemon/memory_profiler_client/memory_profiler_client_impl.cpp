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

#include "memory_profiler_client_impl.h"

#include <unistd.h>

#include "logging.h"
#include "native_memory_profiler_sa_client_manager.h"
#include "native_memory_profiler_sa_config.h"

// using namespace OHOS::Developtools::NativeDaemon;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
namespace {
constexpr uint32_t DEFINE_MIN_MAX_DURATION = 1; // 1 second
constexpr uint32_t DEFINE_MAX_MAX_DURATION = 3600; // 60 * 60  seconds
constexpr uint32_t DEFINE_MIN_FILTER_SIZE = 1; // bytes
constexpr uint32_t DEFINE_MAX_STACK_DEPTH = 30;
constexpr uint32_t DEFINE_MAX_STATISTICS_INTERVAL = 3600; // seconds
constexpr uint32_t DEFINE_MIN_SAMPLE_INTERVAL = 384; // bytes
constexpr uint32_t DEFINE_MIN_ASYNC_NESTING_DEPTH = 1;
constexpr uint32_t DEFINE_MAX_ASYNC_NESTING_DEPTH = 16;
constexpr uint32_t DEFINE_MIN_ASYNC_TASK_STACK_DEPTH = 1;
constexpr uint32_t DEFINE_MAX_ASYNC_TASK_STACK_DEPTH = 256;
}

ResProfilerErrorCode MemoryProfilerClientImpl::Start(uint32_t duration, SimplifiedMemConfig& config,
    ProfilingCallback profilingCallback)
{
    CHECK_TRUE(duration >= DEFINE_MIN_MAX_DURATION && duration <= DEFINE_MAX_MAX_DURATION,
        ResProfilerErrorCode::RES_PROF_ERROR, "Invalid duration: %u", duration);
    CHECK_TRUE(config.sampleSize >= DEFINE_MIN_SAMPLE_INTERVAL, ResProfilerErrorCode::RES_PROF_ERROR,
        "Invalid sampleSize: %zu", config.sampleSize);
    if (profilingCallback == nullptr) {
        profilingCallback = [](int32_t errCode, const std::string& path) {
            PROFILER_LOG_INFO(LOG_CORE, "Hiprofiler native memory ret:%d", errCode);
        };
    }
    PROFILER_LOG_INFO(LOG_CORE, "Start memory profiler by simplified memory config");
    int32_t ret = NativeMemoryProfilerSaClientManager::StartMemProfiler(duration, config, profilingCallback);
    if (ret < static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS) ||
        ret > static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR)) {
        return ResProfilerErrorCode::RES_PROF_ERROR;
    }
    return static_cast<ResProfilerErrorCode>(ret);
}

ResProfilerErrorCode MemoryProfilerClientImpl::Start(int32_t resourceType, MemAPIConfig& config,
    ProfilingCallback profilingCallback)
{
    if (!CheckInput(resourceType, config)) {
        return ResProfilerErrorCode::RES_PROF_ERROR;
    }
    if (profilingCallback == nullptr) {
        profilingCallback = [](int32_t errCode, const std::string& path) {
            PROFILER_LOG_INFO(LOG_CORE, "Hiprofiler native memory ret:%d", errCode);
        };
    }
    PROFILER_LOG_INFO(LOG_CORE, "Start memory profiler by resourceType: %d", resourceType);
    int32_t ret = NativeMemoryProfilerSaClientManager::StartProfilerByType(resourceType, &config, profilingCallback);
    if (ret < static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS) ||
        ret > static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR)) {
        return ResProfilerErrorCode::RES_PROF_ERROR;
    }
    return static_cast<ResProfilerErrorCode>(ret);
}

int32_t MemoryProfilerClientImpl::StartPrintSimplifiedNmd(std::vector<SimplifiedMemStats>& memStats, bool isProfiling)
{
    PROFILER_LOG_INFO(LOG_CORE, "Start simplified nmd mode profiler");
    return NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(getpid(), memStats, isProfiling);
}

int32_t MemoryProfilerClientImpl::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "Stop memory profiler");
    int32_t ret =  NativeMemoryProfilerSaClientManager::Stop();
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop memory profiler failed, ret = %d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t MemoryProfilerClientImpl::UpdateSimplifiedMemConfig(const SimplifiedMemConfig& config)
{
    PROFILER_LOG_INFO(LOG_CORE, "Update simplified memory mem config");
    int32_t ret =  NativeMemoryProfilerSaClientManager::UpdateSimplifiedMemConfig(getpid(), config);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Update simplified memory mem config failed, ret = %d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

bool MemoryProfilerClientImpl::CheckInput(int32_t resourceType, const MemAPIConfig& config) const
{
    // value of resourceType : int32_t
    // OH_RES_TYPE_FD = 0
    // OH_RES_TYPE_THREAD = 1
    // OH_RES_TYPE_NATIVE = 2
    // OH_RES_TYPE_GPU = 3
    // OH_RES_TYPE_GLOBAL_HANDLE = 4
    // OH_RES_TYPE_DMA = 5
    if (resourceType < static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_FD) ||
        resourceType >= static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid resourceType: %d", resourceType);
        return false;
    }
    if (config.maxDuration < DEFINE_MIN_MAX_DURATION || config.maxDuration > DEFINE_MAX_MAX_DURATION) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid maxDuration: %u", config.maxDuration);
        return false;
    }
    if (config.filterSize < DEFINE_MIN_FILTER_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid filterSize: %u", config.filterSize);
        return false;
    }
    if (config.maxStackDepth == 0 || config.maxStackDepth > DEFINE_MAX_STACK_DEPTH) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid maxStackDepth: %u", config.maxStackDepth);
        return false;
    }
    if (config.statisticsInterval == 0 || config.statisticsInterval > DEFINE_MAX_STATISTICS_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid statisticsInterval: %u", config.statisticsInterval);
        return false;
    }
    if (config.sampleInterval < DEFINE_MIN_SAMPLE_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid sampleInterval: %u", config.sampleInterval);
        return false;
    }
    if (config.asyncStackEnable) {
        if (config.asyncTaskStackDepth < DEFINE_MIN_ASYNC_TASK_STACK_DEPTH ||
            config.asyncTaskStackDepth > DEFINE_MAX_ASYNC_TASK_STACK_DEPTH) {
            PROFILER_LOG_ERROR(LOG_CORE, "Invalid asyncTaskStackDepth: %u", config.asyncTaskStackDepth);
            return false;
        }
        if (config.nestedAsyncStackEnable && (config.asyncNestingDepth < DEFINE_MIN_ASYNC_NESTING_DEPTH ||
            config.asyncNestingDepth > DEFINE_MAX_ASYNC_NESTING_DEPTH)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Invalid asyncNestingDepth: %u", config.asyncNestingDepth);
            return false;
        }
    }
    return true;
}
}   // namespace NativeDaemon
}   // namespace Developtools
}   // namespace OHOS
