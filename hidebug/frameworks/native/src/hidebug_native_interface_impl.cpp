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
#include "hidebug_app_thread_cpu.h"
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
    int StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize, std::string &file) override;
    int StopAppTraceCapture() override;
    std::optional<double> GetSystemCpuUsage() override;
    std::optional<MemoryLimit> GetAppMemoryLimit() override;
    std::optional<ProcessMemory> GetAppNativeMemInfo() override;
    std::optional<SysMemory> GetSystemMemInfo() override;
private:
    /**
     * GetElapsedNanoSecondsSinceBoot
     *
     * @return NanoSecondsSinceBoot
     */
    int64_t GetElapsedNanoSecondsSinceBoot();
    constexpr static int SECOND_TO_NANOSECOND = 1 * 1000 * 1000 * 1000;
    constexpr static int CPU_USAGE_VALIDITY = 2 * SECOND_TO_NANOSECOND; // 2s
    static inline int64_t lastCpuUsageGetTime_ = 0;
    static inline double lastCpuUsage_ = 0;
    static inline HidebugAppThreadCpu threadCpu_;
};

std::unique_ptr<HidebugNativeInterface> HidebugNativeInterface::CreateInstance()
{
    return std::make_unique<HidebugNativeInterfaceImpl>();
}

double HidebugNativeInterfaceImpl::GetCpuUsage()
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getprocpid();
    float tmpCpuUsage = dumpUsage->GetCpuUsage(pid);
    double cpuUsage = static_cast<double>(tmpCpuUsage);
    return cpuUsage;
}

std::map<uint32_t, double> HidebugNativeInterfaceImpl::GetAppThreadCpuUsage()
{
    auto collectResult = threadCpu_.CollectThreadStatInfos();
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetAppThreadCpuUsage fail, ret: %{public}d", static_cast<int>(collectResult.retCode));
        return {};
    }
    std::map<uint32_t, double> threadMap;
    for (const auto &threadCpuStatInfo : collectResult.data) {
        threadMap[threadCpuStatInfo.tid] = threadCpuStatInfo.cpuUsage;
    }
    return threadMap;
}

int HidebugNativeInterfaceImpl::StartAppTraceCapture(uint64_t tags, uint32_t flag,
    uint32_t limitsize, std::string &file)
{
    auto ret = StartCaptureAppTrace((TraceFlag)flag, tags, limitsize, file);
    if (ret == RET_SUCC) {
        return TRACE_SUCCESS;
    }
    if (ret == RET_FAIL_INVALID_ARGS) {
        return TRACE_INVALID_ARGS;
    }
    if (ret == RET_STARTED) {
        return TRACE_CAPTURED_ALREADY;
    }
    if (ret == RET_FAIL_MKDIR || ret == RET_FAIL_SETACL || ret == RET_FAIL_EACCES || ret == RET_FAIL_ENOENT) {
        return TRACE_NO_PERMISSION;
    }
    return TRACE_ABNORMAL;
}

int HidebugNativeInterfaceImpl::StopAppTraceCapture()
{
    auto ret = StopCaptureAppTrace();
    if (ret == RET_SUCC) {
        return TRACE_SUCCESS;
    }
    if (ret == RET_STOPPED) {
        return TRACE_NO_RUNNING;
    }
    return TRACE_ABNORMAL;
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

std::optional<ProcessMemory> HidebugNativeInterfaceImpl::GetAppNativeMemInfo()
{
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetAppNativeMemInfo Failed");
        return {};
    }
    int pid = getprocpid();
    auto collectResult = collector->CollectProcessMemory(pid);
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "CollectProcessMemory Failed,retCode = %{public}d",
                    static_cast<int>(collectResult.retCode));
        return {};
    }

    ProcessMemory nativeMemInfo;
    uint32_t pssInfo = collectResult.data.pss + collectResult.data.swapPss;
    nativeMemInfo.pss = pssInfo;
    nativeMemInfo.rss = collectResult.data.rss;
    nativeMemInfo.sharedDirty = collectResult.data.sharedDirty;
    nativeMemInfo.privateDirty = collectResult.data.privateDirty;
    nativeMemInfo.sharedClean = collectResult.data.sharedClean;
    nativeMemInfo.privateClean = collectResult.data.privateClean;

    auto collectVss = collector->CollectProcessVss(pid);
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "CollectProcessVss Failed,retCode = %{public}d", static_cast<int>(collectResult.retCode));
        return {};
    }
    nativeMemInfo.vss = collectVss.data;
    return nativeMemInfo;
}

std::optional<SysMemory> HidebugNativeInterfaceImpl::GetSystemMemInfo()
{
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetSystemMemInfo Failed");
        return {};
    }
    auto collectResult = collector->CollectSysMemory();
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetSystemMemInfo Failed,retCode = %{public}d",
                    static_cast<int>(collectResult.retCode));
        return {};
    }

    SysMemory sysMemInfo;
    sysMemInfo.memTotal = collectResult.data.memTotal;
    sysMemInfo.memFree = collectResult.data.memFree;
    sysMemInfo.memAvailable = collectResult.data.memAvailable;
    return sysMemInfo;
}

int64_t HidebugNativeInterfaceImpl::GetElapsedNanoSecondsSinceBoot()
{
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    return times.tv_sec * SECOND_TO_NANOSECOND + times.tv_nsec;
}
}
}
