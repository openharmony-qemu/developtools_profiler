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
#include <mutex>
#include <sys/stat.h>
#include <vector>
#include <unistd.h>

#include "dump_usage.h"
#include "hidebug_app_thread_cpu.h"
#include "hidebug_util.h"
#include "hitrace_meter.h"
#include "hilog/log.h"
#include "client/cpu_collector_client.h"
#include "client/memory_collector_client.h"
#include "client/trace_collector_client.h"
#include "storage_acl.h"
#include "utility/memory_collector.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Native_Interface"

namespace {
constexpr int64_t SECOND_TO_NANOSECOND = 1000 * 1000 * 1000;
constexpr size_t MAX_LEN_OF_TRACE_PREFIX = 20;
constexpr uint32_t MIN_TRACE_BUFFER_SIZE = 1024; // 1M
constexpr uint32_t MAX_TRACE_BUFFER_SIZE = 15 * 1024; // 15M
constexpr uint32_t MIN_TRACE_DURATION = 1000; // 1s
constexpr uint32_t MAX_TRACE_DURATION = 15 * 1000; // 15s
constexpr uint8_t MAX_TRACE_FILE_NUM = 3;
constexpr const char TRACE_PATH[] = "/data/storage/el2/log/trace/";
constexpr const char TRACE_FILE_SUFFIX[] = ".sys";

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

void CheckAndReplaceTraceParam(std::string& prefix, uint32_t& bufferSize, uint32_t& duration)
{
    if (bufferSize < MIN_TRACE_BUFFER_SIZE) {
        bufferSize = MIN_TRACE_BUFFER_SIZE;
    } else if (bufferSize > MAX_TRACE_BUFFER_SIZE) {
        bufferSize = MAX_TRACE_BUFFER_SIZE;
    }
    if (duration < MIN_TRACE_DURATION) {
        duration = MIN_TRACE_DURATION;
    } else if (duration > MAX_TRACE_DURATION) {
        duration = MAX_TRACE_DURATION;
    }
    if (prefix.length() > MAX_LEN_OF_TRACE_PREFIX) {
        prefix = prefix.substr(0, MAX_LEN_OF_TRACE_PREFIX);
    }
    // char is [a-zA-Z0-9_]
    bool isValid = std::all_of(prefix.begin(), prefix.end(), [](char c) {
        return isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
    if (!isValid) {
        prefix = "";
    }
}

bool CreateTracePathAndSetAccess()
{
    mode_t permissions = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // 0755
    if (access(TRACE_PATH, F_OK) != 0) {
        if (mkdir(TRACE_PATH, permissions) == -1 && errno != EEXIST) {
            HILOG_ERROR(LOG_CORE, "failed to create dir trace:%{public}d, %{public}s", errno, strerror(errno));
            return false;
        }
    }
    if (OHOS::StorageDaemon::AclSetAccess(TRACE_PATH, "g:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set access");
        return false;
    }
    return true;
}

bool CheckTraceFileNum()
{
    std::vector<std::string> files;
    GetDirFiles(TRACE_PATH, files);
    uint32_t count = 0;
    constexpr size_t suffixLen = sizeof(TRACE_FILE_SUFFIX) - 1;
    for (const auto& file : files) {
        if (file.length() >= suffixLen && file.rfind(TRACE_FILE_SUFFIX) == file.length() - suffixLen &&
            (++count) >= MAX_TRACE_FILE_NUM) {
            return false;
        }
    }
    return true;
}

TraceErrorCode ConvertTraceErrorCode(uint32_t traceErrorCode)
{
    switch (traceErrorCode) {
        case UCollect::UcError::SUCCESS:
            return TRACE_SUCCESS;
        case UCollect::UcError::TRACE_OVER_FLOW:
            return RESOURCE_UNAVAILABLE;
        default:
            return TRACE_ABNORMAL;
    }
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
    TraceErrorCode RequestTrace(const std::string& identifier, uint32_t bufferSize, uint32_t duration,
        std::function<void(TraceErrorCode, const std::string&)> task) override;
    void SetTraceErrorCode(uint32_t code) override;
    int GetMemoryLeakResource(const std::string& type, int32_t value, bool enabledDebugLog) override;
    std::optional<double> GetSystemCpuUsage() override;
    std::optional<MemoryLimitInfo> GetAppMemoryLimit() override;
    std::optional<uint64_t> GetVss() override;
    std::optional<NativeMemInfo> GetAppNativeMemInfo(bool withCache) override;
    std::optional<SystemMemoryInfo> GetSystemMemInfo() override;
    bool IsDebuggerConnected() override;
    std::optional<int32_t> GetGraphicsMemory() override;
    std::optional<GraphicsMemorySummary> GetGraphicsMemorySummary(uint32_t interval) override;
    std::optional<RssInfo> GetRssInfo() override;
private:
    bool CheckTraceErrorCode();
private:
    static inline HidebugAppThreadCpu threadCpu_; // It should be initialized at startup.
    std::mutex traceMutex_;
    uint32_t traceErrorCode_ = 0;
    std::string date_;
};

class RequestTraceCallBackImpl : public UCollectClient::RequestTraceCallBack {
public:
    explicit RequestTraceCallBackImpl(std::function<void(TraceErrorCode, const std::string&)> task) : task_(task) {}
    void OnTraceResponse(uint32_t retCode, const std::string& traceName) override;
private:
    std::function<void(TraceErrorCode, const std::string&)> task_;
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

TraceErrorCode HidebugNativeInterfaceImpl::RequestTrace(const std::string& identifier, uint32_t bufferSize,
    uint32_t duration, std::function<void(TraceErrorCode, const std::string&)> task)
{
    std::string prefix = identifier;
    CheckAndReplaceTraceParam(prefix, bufferSize, duration);
    if (!CreateTracePathAndSetAccess()) {
        return TRACE_ABNORMAL;
    }
    if (!CheckTraceFileNum()) {
        HILOG_ERROR(LOG_CORE, "trace file num over %{public}d", MAX_TRACE_FILE_NUM);
        return TRACE_STORAGE_LIMIT;
    }
    if (!CheckTraceErrorCode()) {
        HILOG_ERROR(LOG_CORE, "trace dump over flow today");
        return RESOURCE_UNAVAILABLE;
    }
    auto traceCollector = UCollectClient::TraceCollector::Create();
    if (!traceCollector) {
        HILOG_ERROR(LOG_CORE, "create trace collector failed");
        return TRACE_ABNORMAL;
    }
    auto callback = std::make_shared<RequestTraceCallBackImpl>(std::move(task));
    UCollectClient::TraceConfig traceConfig;
    traceConfig.prefix = prefix;
    traceConfig.bufferSize = bufferSize;
    traceConfig.duration = duration;
    traceCollector->RequestAppTrace(traceConfig, callback);
    return TRACE_SUCCESS;
}

void HidebugNativeInterfaceImpl::SetTraceErrorCode(uint32_t code)
{
    std::lock_guard<std::mutex> guard(traceMutex_);
    date_ = GetDate();
    traceErrorCode_ = code;
}

bool HidebugNativeInterfaceImpl::CheckTraceErrorCode()
{
    std::lock_guard<std::mutex> guard(traceMutex_);
    if (GetDate() == date_) {
        return traceErrorCode_ != UCollect::UcError::TRACE_OVER_FLOW;
    }
    return true;
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

std::optional<RssInfo> HidebugNativeInterfaceImpl::GetRssInfo()
{
    RssInfo rssInfo{};
    bool ret = OHOS::HiviewDFX::GetRssInfo(rssInfo);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "GetRssInfo failed");
        return {};
    }
    return rssInfo;
}

void RequestTraceCallBackImpl::OnTraceResponse(uint32_t retCode, const std::string& traceName)
{
    if (task_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "callback task is null");
        return;
    }
    if (retCode != 0) {
        HILOG_ERROR(LOG_CORE, "callback retCode: %{public}d, traceName: %{public}s", retCode, traceName.c_str());
    }
    HidebugNativeInterface::GetInstance().SetTraceErrorCode(retCode);
    std::string tracePath = traceName.empty() ? "" : TRACE_PATH + traceName;
    task_(ConvertTraceErrorCode(retCode), tracePath);
}
}
}
