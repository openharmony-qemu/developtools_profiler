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
constexpr uint32_t CALL_STACK_SMS = 16384;
constexpr int FIVE_MINUTES = 300;
constexpr int JS_STACK_DEPS = 10;
constexpr uint32_t DEFINE_MIN_MAX_DURATION = 1; // 1 second
constexpr uint32_t DEFINE_MAX_MAX_DURATION = 3600; // 60 * 60  seconds
constexpr uint32_t DEFINE_MIN_FILTER_SIZE = 1; // bytes
constexpr uint32_t DEFINE_MAX_STACK_DEPTH = 30;
constexpr uint32_t DEFINE_MAX_STATISTICS_INTERVAL = 3600; // seconds
constexpr uint32_t DEFINE_MIN_SAMPLE_INTERVAL = 384; // bytes
}

int32_t MemoryProfilerClientImpl::Start(int fd, uint32_t duration, SimplifiedMemConfig& config)
{
    CHECK_TRUE(fd >= 0, RET_ERR, "Invalid file descriptor: %d", fd);
    std::shared_ptr<NativeMemoryProfilerSaConfig> nConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_NOTNULL(nConfig, RET_ERR, "Failed to create NativeMemoryProfilerSaConfig");
    CHECK_TRUE(duration >= DEFINE_MIN_MAX_DURATION && duration <= DEFINE_MAX_MAX_DURATION, RET_ERR,
        "Invalid duration: %u", duration);
    CHECK_TRUE(config.sampleSize >= DEFINE_MIN_SAMPLE_INTERVAL, RET_ERR, "Invalid sampleSize: %zu", config.sampleSize);
    nConfig->pid_ = getpid();
    nConfig->duration_ = duration;
    nConfig->largestSize_ = config.largestSize;
    nConfig->secondLargestSize_ = config.secondLargestSize;
    nConfig->maxGrowthSize_ = config.maxGrowthSize;
    nConfig->sampleInterval_ = config.sampleSize;
    nConfig->shareMemorySize_ = CALL_STACK_SMS;
    nConfig->statisticsInterval_ = FIVE_MINUTES;
    nConfig->jsStackReport_ = true;
    nConfig->maxJsStackDepth_ = JS_STACK_DEPS;
    nConfig->startType_ = 1; // 1:API
    return NativeMemoryProfilerSaClientManager::DumpData(fd, nConfig);
}

int32_t MemoryProfilerClientImpl::Start(int32_t resourceType, MemAPIConfig& config)
{
    if (!CheckInput(resourceType, config)) {
        return RET_ERR;
    }
    auto profilingCallback = [](int32_t errCode, const std::string& path) {
        PROFILER_LOG_INFO(LOG_CORE, "Hiprofiler native memory ret:%d", errCode);
    };
    int ret = NativeMemoryProfilerSaClientManager::StartProfilerByType(resourceType, &config, profilingCallback);
    // ResProfilerErrorCode::RES_PROF_SUCCESS = 11400400
    int successCode = 11400400;
    if (ret != successCode) {
        return RET_ERR;
    }
    return RET_OK;
}

int32_t MemoryProfilerClientImpl::StartPrintSimplifiedNmd(std::vector<SimplifiedMemStats>& memStats)
{
    return NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(getpid(), memStats);
}

int32_t MemoryProfilerClientImpl::Stop()
{
    return NativeMemoryProfilerSaClientManager::Stop();
}

bool MemoryProfilerClientImpl::CheckInput(int32_t resourceType, const MemAPIConfig& config)
{
    // value of resourceType : int32_t
    // OH_RES_TYPE_FD = 0
    // OH_RES_TYPE_THREAD = 1
    // OH_RES_TYPE_NATIVE = 2
    // OH_RES_TYPE_GPU = 3
    // OH_RES_TYPE_GLOBAL_HANDLE = 4
    if (resourceType < 0 || resourceType > 4) {
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
    if (config.maxStackDepth > DEFINE_MAX_STACK_DEPTH) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid maxStackDepth: %u", config.maxStackDepth);
        return false;
    }
    if (config.statisticsInterval > DEFINE_MAX_STATISTICS_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid statisticsInterval: %u", config.statisticsInterval);
        return false;
    }
    if (config.sampleInterval < DEFINE_MIN_SAMPLE_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid sampleInterval: %u", config.sampleInterval);
        return false;
    }
    return true;
}
}   // namespace NativeDaemon
}   // namespace Developtools
}   // namespace OHOS
