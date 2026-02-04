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
#include <fstream>
#include <memory>
#include <vector>
#include <unistd.h>

#include "dump_usage.h"
#include "hidebug_app_thread_cpu.h"
#include "hidebug_util.h"
#include "hitrace_meter.h"
#include "hilog/log.h"
#include "client/cpu_collector_client.h"
#include "client/memory_collector_client.h"
#include "utility/memory_collector.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Native_Interface"

namespace {
constexpr int64_t SECOND_TO_NANOSECOND = 1000 * 1000 * 1000;

int GetNativeMemInfo(NativeMemInfo& nativeMemInfo)
{
    if (!GetVssInfo(nativeMemInfo)) {
        HILOG_ERROR(LOG_CORE, "GetVss Failed");
        return NATIVE_FAIL;
    }
    if (!GetMemInfo(nativeMemInfo)) {
        HILOG_ERROR(LOG_CORE, "GetMemInfo Failed");
        return NATIVE_FAIL;
    }
    return NATIVE_SUCCESS;
}

int GetGraphicMemoryInfo(GraphicsMemorySummary& graphicMemoryInfo)
{
    if (IsHm()) {
        if (!GetGlAndGraph(graphicMemoryInfo)) {
            HILOG_ERROR(LOG_CORE, "GetGlAndGraph Failed");
            return NATIVE_FAIL;
        }
    } else {
        auto collector = UCollectClient::MemoryCollector::Create();
        if (!collector) {
            HILOG_ERROR(LOG_CORE, "GetGraphicUsage Failed");
            return NATIVE_FAIL;
        }
        auto collectResult = collector->GetGraphicUsage();
        if (collectResult.retCode != UCollect::UcError::SUCCESS ||
            collectResult.data.gl < 0 || collectResult.data.graph < 0) {
            HILOG_ERROR(LOG_CORE, "GetGraphicUsage Failed,retCode = %{public}d",
                        static_cast<int>(collectResult.retCode));
            return NATIVE_FAIL;
        }
        graphicMemoryInfo.gl = static_cast<uint32_t>(collectResult.data.gl);
        graphicMemoryInfo.graph = static_cast<uint32_t>(collectResult.data.graph);
    }
    return NATIVE_SUCCESS;
}

int GetSystemCpuUsageInfo(double& cpuUsage)
{
    std::shared_ptr<UCollectClient::CpuCollector> collector = UCollectClient::CpuCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed");
        return NATIVE_FAIL;
    }
    auto collectResult = collector->GetSysCpuUsage();
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed, retCode: %{public}d",
                    static_cast<int>(collectResult.retCode));
        return NATIVE_FAIL;
    }
    cpuUsage = collectResult.data;
    return NATIVE_SUCCESS;
}
}

class HidebugNativeInterfaceImpl : public HidebugNativeInterface {
public:
    HidebugNativeInterfaceImpl() = default;
    HidebugNativeInterfaceImpl(const HidebugNativeInterfaceImpl&) = delete;
    HidebugNativeInterfaceImpl& operator =(const HidebugNativeInterfaceImpl&) = delete;
    double GetCpuUsage() override;
    std::map<uint32_t, double> GetAppThreadCpuUsage() override;
    TraceErrorCode StartAppTraceCapture(uint64_t tags, uint32_t flag,
        uint32_t limitsize, std::string &file) override;
    TraceErrorCode StopAppTraceCapture() override;
    int GetMemoryLeakResource(const std::string& type, int32_t value, bool enabledDebugLog) override;
    std::optional<double> GetSystemCpuUsage() override;
    std::optional<MemoryLimitInfo> GetAppMemoryLimit() override;
    std::optional<uint64_t> GetVss() override;
    std::optional<NativeMemInfo> GetAppNativeMemInfo(bool withCache) override;
    std::optional<SystemMemoryInfo> GetSystemMemInfo() override;
    bool IsDebuggerConnected() override;
    std::optional<int32_t> GetGraphicsMemory() override;
    std::optional<GraphicsMemorySummary> GetGraphicsMemorySummary(uint32_t interval) override;
private:
    static inline HidebugAppThreadCpu threadCpu_; // It should be initialized at startup.
};

HidebugNativeInterface& HidebugNativeInterface::GetInstance()
{
    static HidebugNativeInterfaceImpl instance;
    return instance;
}

double HidebugNativeInterfaceImpl::GetCpuUsage()
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getprocpid();
    return dumpUsage->GetCpuUsage(pid);
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

TraceErrorCode HidebugNativeInterfaceImpl::StartAppTraceCapture(uint64_t tags, uint32_t flag,
    uint32_t limitsize, std::string &file)
{
    if (flag != TraceFlag::FLAG_MAIN_THREAD && flag != TraceFlag::FLAG_ALL_THREAD) {
        return TRACE_INVALID_ARGUMENT;
    }
    auto ret = StartCaptureAppTrace(static_cast<TraceFlag>(flag), tags, limitsize, file);
    if (ret == RET_SUCC) {
        return TRACE_SUCCESS;
    }
    if (ret == RET_FAIL_INVALID_ARGS) {
        return TRACE_INVALID_ARGUMENT;
    }
    if (ret == RET_STARTED) {
        return TRACE_CAPTURED_ALREADY;
    }
    if (ret == RET_FAIL_MKDIR || ret == RET_FAIL_SETACL || ret == RET_FAIL_EACCES || ret == RET_FAIL_ENOENT) {
        return TRACE_NO_PERMISSION;
    }
    return TRACE_ABNORMAL;
}

TraceErrorCode HidebugNativeInterfaceImpl::StopAppTraceCapture()
{
    auto ret = StopCaptureAppTrace();
    if (ret == RET_SUCC) {
        return TRACE_SUCCESS;
    }
    if (ret == RET_STOPPED) {
        return NO_TRACE_RUNNING;
    }
    return TRACE_ABNORMAL;
}

std::optional<double> HidebugNativeInterfaceImpl::GetSystemCpuUsage()
{
    static CachedValue<double> cachedCpuUsage;
    HILOG_INFO(LOG_CORE, "GetSystemCpuUsage");
    constexpr int64_t effectiveTime = 2 * SECOND_TO_NANOSECOND;
    auto ret = cachedCpuUsage.GetOrUpdateCachedValue(effectiveTime, GetSystemCpuUsageInfo);
    if (ret.first == NATIVE_SUCCESS) {
        return ret.second;
    }
    return {};
}

std::optional<MemoryLimitInfo> HidebugNativeInterfaceImpl::GetAppMemoryLimit()
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

    MemoryLimitInfo memoryLimit;
    memoryLimit.vssLimit = collectResult.data.vssLimit;
    memoryLimit.rssLimit = collectResult.data.rssLimit;
    return memoryLimit;
}

std::optional<NativeMemInfo> HidebugNativeInterfaceImpl::GetAppNativeMemInfo(bool withCache)
{
    static CachedValue<NativeMemInfo> cachedNativeMemInfo;
    constexpr int64_t effectiveTime = 5 * 60 * SECOND_TO_NANOSECOND;
    auto ret = cachedNativeMemInfo.GetOrUpdateCachedValue(withCache ? effectiveTime : 0, GetNativeMemInfo);
    if (ret.first == NATIVE_SUCCESS) {
        return ret.second;
    }
    return {};
}

std::optional<uint64_t> HidebugNativeInterfaceImpl::GetVss()
{
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (!collector) {
        HILOG_ERROR(LOG_CORE, "GetVssInfo Failed");
        return {};
    }
    int pid = getprocpid();
    auto collectVss = collector->CollectProcessVss(pid);
    if (collectVss.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "CollectProcessVss Failed,retCode = %{public}d", static_cast<int>(collectVss.retCode));
        return {};
    }
    return collectVss.data;
}

std::optional<SystemMemoryInfo> HidebugNativeInterfaceImpl::GetSystemMemInfo()
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
    SystemMemoryInfo systemMemoryInfo{};
    systemMemoryInfo.totalMem = static_cast<uint32_t>(collectResult.data.memTotal);
    systemMemoryInfo.freeMem = static_cast<uint32_t>(collectResult.data.memFree);
    systemMemoryInfo.availableMem = static_cast<uint32_t>(collectResult.data.memAvailable);
    return systemMemoryInfo;
}

int HidebugNativeInterfaceImpl::GetMemoryLeakResource(const std::string& type,
    int32_t value, bool enabledDebugLog)
{
    HILOG_DEBUG(LOG_CORE, "GetMemoryLeakResource");
    auto memoryCollect = UCollectClient::MemoryCollector::Create();
    if (!memoryCollect) {
        HILOG_ERROR(LOG_CORE, "GetMemoryLeakResource Failed, return result");
        return NATIVE_FAIL;
    }
    UCollectClient::MemoryCaller memoryCaller;
    memoryCaller.pid = getprocpid();
    memoryCaller.resourceType = type;
    memoryCaller.limitValue = value;
    memoryCaller.enabledDebugLog = enabledDebugLog;
    auto result = memoryCollect->SetAppResourceLimit(memoryCaller);
    if (result.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "GetMemoryLeakResource Failed, retCode: %{public}d, return the last result",
            static_cast<int>(result.retCode));
        return NATIVE_FAIL;
    }
    HILOG_DEBUG(LOG_CORE, "GetMemoryLeakResource Success, retCode: %{public}d", static_cast<int>(result.retCode));
    return NATIVE_SUCCESS;
}

bool HidebugNativeInterfaceImpl::IsDebuggerConnected()
{
    HILOG_DEBUG(LOG_CORE, "IsDebuggerConnected");
    std::ifstream file("/proc/self/status");
    if (!file.is_open()) {
        HILOG_ERROR(LOG_CORE, "IsDebuggerConnected:: open status file failed!");
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("TracerPid:") != std::string::npos) {
            std::string pidStr = line.substr(line.find(":") + 1);
            return std::atoi(pidStr.c_str()) != 0;
        }
    }
    HILOG_ERROR(LOG_CORE, "IsDebuggerConnected:: no find the TracerPid:");
    return false;
}

std::optional<int32_t> HidebugNativeInterfaceImpl::GetGraphicsMemory()
{
    constexpr int64_t effectiveTime = 2; // 2s
    auto ret = GetGraphicsMemorySummary(effectiveTime);
    if (ret) {
        return static_cast<int32_t>(ret->gl + ret->graph);
    }
    return {};
}

std::optional<GraphicsMemorySummary> HidebugNativeInterfaceImpl::GetGraphicsMemorySummary(uint32_t interval)
{
    constexpr uint32_t minInterval = 2;
    constexpr uint32_t maxInterval = 3600;
    constexpr uint32_t defaultInterval = 300;
    if (interval < minInterval || interval > maxInterval) {
        interval = defaultInterval;
    }
    static CachedValue<GraphicsMemorySummary> cachedGraphicUsage;
    int64_t effectiveTime = static_cast<int64_t>(interval) * SECOND_TO_NANOSECOND;
    auto ret = cachedGraphicUsage.GetOrUpdateCachedValue(effectiveTime, GetGraphicMemoryInfo);
    if (ret.first == NATIVE_SUCCESS) {
        return ret.second;
    }
    return {};
}
}
}
