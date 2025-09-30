/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <atomic>
#include <string>
#include <vector>

#include "hidebug_util.h"
#include "hidebug/hidebug.h"
#include "hilog/log.h"
#include "lite_perf.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Perf"

namespace OHOS::HiviewDFX {
bool CheckPerfArgs(const HiDebug_ProcessSamplerConfig& config, LitePerfConfig& litePerfConfig)
{
    constexpr uint32_t durationMax = 10000;
    constexpr uint32_t durationMin = 1000;
    if (config.duration < durationMin || config.tids == nullptr) {
        return false;
    }
    litePerfConfig.durationMs = std::min(config.duration, durationMax);
    constexpr uint32_t frequencyMax = 200;
    constexpr uint32_t frequencyMin = 1;
    if (config.frequency < frequencyMin || config.frequency > frequencyMax) {
        constexpr int32_t frequencyDefault = 100;
        litePerfConfig.freq = frequencyDefault;
    } else {
        litePerfConfig.freq = static_cast<int>(config.frequency);
    }
    constexpr uint32_t maxTidSize = 10;
    auto tidSize =  std::min(maxTidSize, config.size);
    litePerfConfig.tids.resize(tidSize);
    for (uint32_t i = 0; i < tidSize; i++) {
        litePerfConfig.tids[i] = static_cast<int>(config.tids[i]);
    }
    return true;
}

HiDebug_ErrorCode ConvertPerfErrorCode(PerfErrorCode errCode)
{
    switch (errCode) {
        case PerfErrorCode::RESOURCE_NOT_AVAILABLE:
            return HIDEBUG_RESOURCE_UNAVAILABLE;
        case PerfErrorCode::PERF_SAMPING:
            return HIDEBUG_UNDER_SAMPLING;
        case PerfErrorCode::UN_SUPPORTED:
            return HIDEBUG_NOT_SUPPORTED;
        case PerfErrorCode::INVALID_PARAM:
            return HIDEBUG_INVALID_ARGUMENT;
        default:
            return HIDEBUG_TRACE_ABNORMAL;
    }
}
}

HiDebug_ErrorCode OH_HiDebug_RequestThreadLiteSampling(
    HiDebug_ProcessSamplerConfig* config, OH_HiDebug_ThreadLiteSamplingCallback stacksCallBack)
{
    static std::atomic<bool> isRunning{false};
    bool expected = false;
    if (!isRunning.compare_exchange_strong(expected, true)) {
        return HIDEBUG_UNDER_SAMPLING;
    }
    auto autoResetFlag = std::unique_ptr<std::atomic<bool>, void(*)(std::atomic<bool>*)>(&isRunning,
        [] (std::atomic<bool>* flag) {
            flag->store(false);
        });
    if (config == nullptr || stacksCallBack == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    using namespace OHOS::HiviewDFX;
    LitePerfConfig litePerfConfig{};
    if (!CheckPerfArgs(*config, litePerfConfig)) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    std::string stacks;
    const bool checkLimit = !IsDebuggableHap() && !IsDeveloperOptionsEnabled();
    auto retCode = LitePerf().CollectProcessStackSampling(litePerfConfig, checkLimit, stacks);
    if (retCode == PerfErrorCode::SUCCESS) {
        stacksCallBack(stacks.c_str());
        return HIDEBUG_SUCCESS;
    }
    return ConvertPerfErrorCode(retCode);
}