/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hidebug_native_interface.h"

#include <chrono>
#include <memory>
#include <vector>
#include <unistd.h>

#include "dump_usage.h"
#include "hilog/log.h"
#include "client/cpu_collector.h"
#include "utility/memory_collector.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Native_Interface"

class HidebugNativeInterfaceImpl : public HidebugNativeInterface {
public:
    HidebugNativeInterfaceImpl() = default;
    HidebugNativeInterfaceImpl(const HidebugNativeInterfaceImpl&) = delete;
    HidebugNativeInterfaceImpl& operator =(const HidebugNativeInterfaceImpl&) = delete;
    double GetCpuUsage() override;
    std::map<uint32_t, double> GetAppThreadCpuUsage() override;
    std::string StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize) override;
    void StopAppTraceCapture() override;
    std::optional<double> GetSystemCpuUsage() override;
    std::optional<MemoryLimit> GetAppMemoryLimit() override;
private:
    /**
     * GetElapsedNanoSecondsSinceBoot
     *
     * @return NanoSecondsSinceBoot
     */
    int64_t GetElapsedNanoSecondsSinceBoot();
    constexpr static int SECOND_TO_NANOSECOND = 1 * 1000 * 1000 * 1000;
    constexpr static int CPU_USAGE_VALIDITY = 2 * SECOND_TO_NANOSECOND; // 2s
    int64_t lastCpuUsageGetTime_ = 0;
    double lastCpuUsage_ = 0;
};

std::unique_ptr<HidebugNativeInterface> HidebugNativeInterface::CreateInstance()
{
    return std::make_unique<HidebugNativeInterfaceImpl>();
}

double HidebugNativeInterfaceImpl::GetCpuUsage()
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getpid();
    float tmpCpuUsage = dumpUsage->GetCpuUsage(pid);
    double cpuUsage = static_cast<double>(tmpCpuUsage);
    return cpuUsage;
}

std::map<uint32_t, double> HidebugNativeInterfaceImpl::GetAppThreadCpuUsage()
{
    return {};
}

std::string HidebugNativeInterfaceImpl::StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize)
{
    return "";
}

void HidebugNativeInterfaceImpl::StopAppTraceCapture()
{
}

std::optional<double> HidebugNativeInterfaceImpl::GetSystemCpuUsage()
{
    HILOG_INFO(LOG_CORE, "GetSystemCpuUsage");
    int64_t now = GetElapsedNanoSecondsSinceBoot();
    if (lastCpuUsageGetTime_ > 0 && now <= lastCpuUsageGetTime_ + CPU_USAGE_VALIDITY) {
        HILOG_WARN(LOG_CORE, "GetSystemCpuUsage too frequently, return the last result");
        return lastCpuUsage_;
    }
    std::shared_ptr<UCollectClient::CpuCollector> collector = UCollectClient::CpuCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed, return the last result");
        return std::nullopt;
    }
    auto collectResult = collector->GetSysCpuUsage();
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed, retCode: %{public}d, return the last result",
                    static_cast<int>(collectResult.retCode));
        return std::nullopt;
    }
    lastCpuUsage_ = collectResult.data;
    lastCpuUsageGetTime_ = GetElapsedNanoSecondsSinceBoot();
    return lastCpuUsage_;
}

std::optional<MemoryLimit> HidebugNativeInterfaceImpl::GetAppMemoryLimit()
{
    auto collector = UCollectUtil::MemoryCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetAppMemoryLimit Failed");
        return {};
    }
    auto collectResult = collector->CollectMemoryLimit();
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetAppMemoryLimit Failed, retCode: %{public}d", static_cast<int>(collectResult.retCode));
        return {};
    }

    MemoryLimit memoryLimit;
    memoryLimit.vssLimit = collectResult.data.vssLimit;
    memoryLimit.rssLimit = collectResult.data.rssLimit;

    return memoryLimit;
}

int64_t HidebugNativeInterfaceImpl::GetElapsedNanoSecondsSinceBoot()
{
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    return times.tv_sec * SECOND_TO_NANOSECOND + times.tv_nsec;
}
}
}