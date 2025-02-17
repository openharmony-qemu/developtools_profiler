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

class HidebugNativeInterfaceImpl : public HidebugNativeInterface {
public:
    HidebugNativeInterfaceImpl() = default;
    HidebugNativeInterfaceImpl(const HidebugNativeInterfaceImpl&) = delete;
    HidebugNativeInterfaceImpl& operator =(const HidebugNativeInterfaceImpl&) = delete;
    double GetCpuUsage() override;
    std::map<uint32_t, double> GetAppThreadCpuUsage() override;
    HiDebug_ErrorCode StartAppTraceCapture(uint64_t tags, uint32_t flag,
        uint32_t limitsize, std::string &file) override;
    HiDebug_ErrorCode StopAppTraceCapture() override;
    int GetMemoryLeakResource(const std::string& type, int32_t value, bool enabledDebugLog) override;
    std::optional<double> GetSystemCpuUsage() override;
    std::optional<MemoryLimit> GetAppMemoryLimit() override;
    std::optional<HiDebug_NativeMemInfo> GetAppNativeMemInfo() override;
    std::optional<SysMemory> GetSystemMemInfo() override;
    bool IsDebuggerConnected() override;
    std::optional<int32_t> GetGraphicsMemory() override;
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

HiDebug_ErrorCode HidebugNativeInterfaceImpl::StartAppTraceCapture(uint64_t tags, uint32_t flag,
    uint32_t limitsize, std::string &file)
{
    auto ret = StartCaptureAppTrace((TraceFlag)flag, tags, limitsize, file);
    if (ret == RET_SUCC) {
        return HIDEBUG_SUCCESS;
    }
    if (ret == RET_FAIL_INVALID_ARGS) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    if (ret == RET_STARTED) {
        return HIDEBUG_TRACE_CAPTURED_ALREADY;
    }
    if (ret == RET_FAIL_MKDIR || ret == RET_FAIL_SETACL || ret == RET_FAIL_EACCES || ret == RET_FAIL_ENOENT) {
        return HIDEBUG_NO_PERMISSION;
    }
    return HIDEBUG_TRACE_ABNORMAL;
}

HiDebug_ErrorCode HidebugNativeInterfaceImpl::StopAppTraceCapture()
{
    auto ret = StopCaptureAppTrace();
    if (ret == RET_SUCC) {
        return HIDEBUG_SUCCESS;
    }
    if (ret == RET_STOPPED) {
        return HIDEBUG_NO_TRACE_RUNNING;
    }
    return HIDEBUG_TRACE_ABNORMAL;
}

std::optional<double> HidebugNativeInterfaceImpl::GetSystemCpuUsage()
{
    static CachedValue<double> cachedCpuUsage;
    HILOG_INFO(LOG_CORE, "GetSystemCpuUsage");
    constexpr const int64_t effectiveTime = 2 * 1000 * 1000 * 1000; // 2s
    return cachedCpuUsage.GetOrUpdateCachedValue(effectiveTime, [](double& cachedValue) {
        std::shared_ptr<UCollectClient::CpuCollector> collector = UCollectClient::CpuCollector::Create();
        if (!collector) {
            HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed");
            return false;
        }
        auto collectResult = collector->GetSysCpuUsage();
        if (collectResult.retCode != UCollect::UcError::SUCCESS) {
            HILOG_ERROR(LOG_CORE, "GetSystemCpuUsage Failed, retCode: %{public}d",
                        static_cast<int>(collectResult.retCode));
            return false;
        }
        cachedValue = collectResult.data;
        return true;
    });
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

std::optional<HiDebug_NativeMemInfo> HidebugNativeInterfaceImpl::GetAppNativeMemInfo()
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

    HiDebug_NativeMemInfo nativeMemInfo;
    int32_t pssInfo = collectResult.data.pss + collectResult.data.swapPss;
    nativeMemInfo.pss = static_cast<uint32_t>(pssInfo);
    nativeMemInfo.rss = collectResult.data.rss;
    nativeMemInfo.sharedDirty = collectResult.data.sharedDirty;
    nativeMemInfo.privateDirty = collectResult.data.privateDirty;
    nativeMemInfo.sharedClean = static_cast<uint32_t>(collectResult.data.sharedClean);
    nativeMemInfo.privateClean = collectResult.data.privateClean;

    auto collectVss = collector->CollectProcessVss(pid);
    if (collectResult.retCode != UCollect::UcError::SUCCESS) {
        HILOG_ERROR(LOG_CORE, "CollectProcessVss Failed,retCode = %{public}d", static_cast<int>(collectResult.retCode));
        return {};
    }
    nativeMemInfo.vss = static_cast<uint32_t>(collectVss.data);
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

int HidebugNativeInterfaceImpl::GetMemoryLeakResource(const std::string& type,
    int32_t value, bool enabledDebugLog)
{
    HILOG_DEBUG(LOG_CORE, "GetMemoryLeakResource");
    auto memoryCollect = UCollectClient::MemoryCollector::Create();
    if (!memoryCollect) {
        HILOG_ERROR(LOG_CORE, "GetMemoryLeakResource Failed, return result");
        return MemoryState::MEMORY_FAILED;
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
        return MemoryState::MEMORY_FAILED;
    }
    HILOG_DEBUG(LOG_CORE, "GetMemoryLeakResource Success, retCode: %{public}d", static_cast<int>(result.retCode));
    return MemoryState::MEMORY_SUCCESS;
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
    static CachedValue<int32_t> cachedGraphicUsage;
    constexpr const int64_t effectiveTime = 2 * 1000 * 1000 * 1000; // 2s
    return cachedGraphicUsage.GetOrUpdateCachedValue(effectiveTime, [](int32_t& cachedValue) {
        auto collector = UCollectClient::MemoryCollector::Create();
        if (!collector) {
            HILOG_ERROR(LOG_CORE, "GetGraphicUsage Failed");
            return false;
        }
        auto collectResult = collector->GetGraphicUsage();
        if (collectResult.retCode != UCollect::UcError::SUCCESS) {
            HILOG_ERROR(LOG_CORE, "GetGraphicUsage Failed,retCode = %{public}d",
                        static_cast<int>(collectResult.retCode));
            return false;
        }
        cachedValue = collectResult.data;
        return true;
    });
}
}
}
