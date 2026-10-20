/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "native_memory_profiler_sa_client_manager.h"
#include "memory_trace.h"
#include "native_memory_profiler_sa_callback_stub.h"
#include "native_memory_profiler_sa_proxy.h"
#include "schedule_task_manager.h"
#include "common.h"
#include "mem_config.h"

#include "logging.h"

#include <fstream>
#include <functional>
#include <variant>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "application_context.h"
#include "async_stack.h"
#include "storage_acl.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr uint32_t LIB_SMS = 4096;
constexpr uint32_t CALL_STACK_SMS = 16384;
constexpr uint32_t NMD_DURATION = 5;
constexpr uint32_t NMD_SHMEM = 300;
constexpr uint32_t GWP_DURATION = 10;
constexpr uint32_t GWP_SHMEM = 300;
constexpr uint32_t MEM_LEAK_SHMEM = 300;
constexpr int ONLY_NMD = 2;
constexpr int SIMP_NMD = 3;
constexpr int FIVE_MINUTES = 300;
constexpr int JS_STACK_DEPS = 10;
const std::string NAPI_FILE_PATH_PREFIX = "/data/storage/el2/base/files/";
const std::string NAPI_FILE_PATH = "/data/storage/el2/base/files";
const std::string NAPI_BASE_PATH = "/data/storage/el2/base";
constexpr uint32_t DEFINE_MIN_MAX_DURATION = 1; // 1 second
constexpr uint32_t DEFINE_MAX_MAX_DURATION = 3600; // 60 * 60  seconds
constexpr uint32_t DEFINE_MIN_FILTER_SIZE = 1; // bytes
constexpr uint32_t DEFINE_MAX_STACK_DEPTH = 30;
constexpr uint32_t DEFINE_MAX_STATISTICS_INTERVAL = 3600; // seconds
constexpr uint32_t DEFINE_MIN_SAMPLE_INTERVAL = 384; // bytes
constexpr uint32_t ASYNC_REQUEST_TIMEOUT_DELTA = 600; // seconds
constexpr int32_t TIME_BASE = 1000;
const std::string FILE_SUFFIX = ".htrace";
const std::string COMPOSITE_HEAP_SUFFIX = ".zip";
const std::map<InternalResourceType, std::string> resourceTypeValues = {
    {InternalResourceType::OH_RES_TYPE_FD, "fd"},
    {InternalResourceType::OH_RES_TYPE_THREAD, "thread"},
    {InternalResourceType::OH_RES_TYPE_NATIVE, "native"},
    {InternalResourceType::OH_RES_TYPE_GPU, "gpu"},
    {InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE, "global_handle"},
    {InternalResourceType::OH_RES_TYPE_DMA, "dma"},
    {InternalResourceType::OH_RES_TYPE_ASHMEM, "ashmem"},
    {InternalResourceType::OH_RES_TYPE_COMPOSITE_HEAP, "composite_heap"},
};

using ProfilingCallbackVar = std::variant<
    NativeMemoryProfilerSaClientManager::ProfilingCallback,
    NativeMemoryProfilerSaClientManager::ProfilingDumpCallback
>;

struct AsyncHookRequest {
    int32_t pid;
    uint32_t duration;
    ProfilingCallbackVar callbackVar;
    int fd;
    std::string filepath;
    int32_t timeoutFd;
    AsyncHookRequest(int32_t pid, uint32_t duration, NativeMemoryProfilerSaClientManager::ProfilingCallback callback,
        int fd, const std::string& filepath) : pid(pid), duration(duration), fd(fd), filepath(filepath)
    {
        timeoutFd = -1;
        callbackVar = callback;
    }

    AsyncHookRequest(int32_t pid, uint32_t duration,
        NativeMemoryProfilerSaClientManager::ProfilingDumpCallback callback, int fd,
        const std::string& filepath) : pid(pid), duration(duration), fd(fd), filepath(filepath)
    {
        timeoutFd = -1;
        callbackVar = callback;
    }
};

void ParseMemStatsFromString(std::string& stats, std::vector<SimplifiedMemStats>& memStats)
{
    SimplifiedMemStats tmp;
    std::istringstream iss(stats);
    std::stringstream ss;
    std::string line;
    while (std::getline(iss, line, '\n')) {
        ss.str("");
        ss.clear();
        ss.str(line);
        ss >> tmp.size >> tmp.allocated >> tmp.nmalloc >> tmp.ndalloc;
        memStats.push_back(tmp);
    }
}

void SetDefaultGpuRange(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return;
    }
    std::vector<std::vector<uint64_t>> defaultRange;
    constexpr uint64_t UINT64_MAX_VAL = 0xFFFFFFFFFFFFFFFFULL;
    constexpr size_t GPU_TYPE_NUM = 5;
    for (size_t i = 0; i < GPU_TYPE_NUM; i++) {
        defaultRange.push_back({0, UINT64_MAX_VAL, 0, UINT64_MAX_VAL});
    }
    std::swap(config->gpuRange_, defaultRange);
}
}

static void NotifyProfilingResult(int32_t pid, int32_t errorCode, const std::string& filepath);
class NativeMemoryProfilerAsyncRequestManager {
public:
    bool TryAddRequest(int32_t pid, uint32_t duration,
        NativeMemoryProfilerSaClientManager::ProfilingCallback callback, int fd, const std::string& filepath)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto request = std::make_shared<AsyncHookRequest>(pid, duration, callback, fd, filepath);
        return requests_.insert(std::make_pair(pid, request)).second;
    }

    bool TryAddRequest(int32_t pid, uint32_t duration,
        NativeMemoryProfilerSaClientManager::ProfilingDumpCallback callback, int fd, const std::string& filepath)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto request = std::make_shared<AsyncHookRequest>(pid, duration, callback, fd, filepath);
        return requests_.insert(std::make_pair(pid, request)).second;
    }

    void DeleteRequest(int32_t pid)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        requests_.erase(pid);
    }

    std::shared_ptr<AsyncHookRequest> TakeOutRequest(int32_t pid)
    {
        std::shared_ptr<AsyncHookRequest> result = nullptr;
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto it = requests_.find(pid);
        if (it != requests_.end()) {
            result = it->second;
            requests_.erase(pid);
        }
        if (result != nullptr && result->timeoutFd != -1) {
            timeoutTaskMgr_.UnscheduleTask(result->timeoutFd);
        }
        return result;
    }

    void CreateTimeoutMonitor(int32_t pid)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto it = requests_.find(pid);
        CHECK_TRUE(it != requests_.end(), NO_RETVAL, "no async req found for pid:%d", pid);
        std::shared_ptr<AsyncHookRequest> request = it->second;
        CHECK_NOTNULL(request, NO_RETVAL, "no async req found for pid:%d", pid);
        if (request->duration == 0) {
            PROFILER_LOG_WARN(LOG_CORE, "not set duration");
            return;
        }
        int32_t timeoutFd = timeoutTaskMgr_.ScheduleTask(
            [pid] {
                PROFILER_LOG_WARN(LOG_CORE, "Profiler async request timeout");
                NotifyProfilingResult(pid, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR), "");
            },
            (request->duration + ASYNC_REQUEST_TIMEOUT_DELTA) * TIME_BASE,
            true);
        CHECK_TRUE(timeoutFd != -1, NO_RETVAL, "Profiler async request create monitor for %d FAIL", pid);
        request->timeoutFd = timeoutFd;
    }

    std::string GetFilepath(int32_t pid)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto it = requests_.find(pid);
        CHECK_TRUE(it != requests_.end(), "", "no async req found for pid:%d", pid);
        std::shared_ptr<AsyncHookRequest> request = it->second;
        CHECK_NOTNULL(request, "", "no async req found for pid:%d", pid);
        return request->filepath;
    }

private:
    std::unordered_map<int32_t, std::shared_ptr<AsyncHookRequest>> requests_;
    std::mutex requestsMutex_;
    ScheduleTaskManager timeoutTaskMgr_;
};

static NativeMemoryProfilerAsyncRequestManager g_profilerAsyncReqMgr;

static void NotifyProfilingResult(int32_t pid, int32_t errorCode, const std::string& filepath)
{
    std::shared_ptr<AsyncHookRequest> request = g_profilerAsyncReqMgr.TakeOutRequest(pid);
    CHECK_NOTNULL(request, NO_RETVAL, "no async req found for pid %d", pid);
    std::visit([pid, errorCode, filepath](auto& callback) {
            CHECK_TRUE(callback, NO_RETVAL, "invalid callback for pid %d", pid);
            using T = std::decay_t<decltype(callback)>;
            if constexpr (std::is_same_v<T, NativeMemoryProfilerSaClientManager::ProfilingCallback>) {
                callback(errorCode, filepath);
            } else if constexpr (std::is_same_v<T, NativeMemoryProfilerSaClientManager::ProfilingDumpCallback>) {
                callback(errorCode);
            } else {
                PROFILER_LOG_ERROR(LOG_CORE, "invalid callback for pid %d", pid);
            }
        }, request->callbackVar);
}

class NativeMemoryProfilerSaResultCallback final : public NativeMemoryProfilerSaCallbackStub {
public:
    void OnProfilingResult(int32_t pid, int32_t errorCode, const std::string& filePath) override
    {
        PROFILER_LOG_DEBUG(LOG_CORE, "OnProfilingResult CALL, pid: %d, errorCode: %d", pid, errorCode);
        NotifyProfilingResult(pid, errorCode, filePath);
    }
};

static sptr<IRemoteObject> GetProfilingResultCallbackObject()
{
    static sptr<NativeMemoryProfilerSaResultCallback> callback =
        new (std::nothrow) NativeMemoryProfilerSaResultCallback();
    if (callback == nullptr) {
        return nullptr;
    }
    return callback->AsObject();
}

static int32_t InvokeWithProxy(std::function<int32_t(NativeMemoryProfilerSaProxy&)> invoker)
{
    int32_t retPrepare = NativeMemoryProfilerSaClientManager::Prepare();
    CHECK_TRUE(retPrepare == RET_OK, retPrepare, "NativeMemoryProfilerSaClientManager: Prepare failed");
    auto service = NativeMemoryProfilerSaClientManager::GetRemoteService();
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: GetRemoteService failed");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return invoker(proxy);
}

int32_t NativeMemoryProfilerSaClientManager::Start(int fd, pid_t pid, MemLeakConfig& memLeakConfig)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: pid is 0");

    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = memLeakConfig.duration;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->shareMemorySize_ = MEM_LEAK_SHMEM;
    config->nativeOrphanSnapshotsEnable_ = true;
    config->memLeakFormat_ = memLeakConfig.format;
    config->memLeakLimit_ = memLeakConfig.limit;
    return InvokeWithProxy([fd, &config](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, config);
    });
}

int32_t NativeMemoryProfilerSaClientManager::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    CHECK_TRUE(CheckConfig(config), RET_ERR, "CheckConfig failed");
    return InvokeWithProxy([&config](NativeMemoryProfilerSaProxy& proxy) { return proxy.Start(config); });
}

int32_t NativeMemoryProfilerSaClientManager::StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
    const sptr<IRemoteObject>& callback)
{
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    CHECK_TRUE(CheckConfig(config), RET_ERR, "CheckConfig failed");
    return InvokeWithProxy([&config, &callback](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.StartPidVoid(config, callback);
    });
}

int32_t NativeMemoryProfilerSaClientManager::Start(NativeMemProfilerType type, uint32_t pid, uint32_t duration,
    uint32_t sampleIntervel)
{
    if (pid == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: pid cannot be 0");
        return RET_ERR;
    }
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = duration;
    config->sampleInterval_ = sampleIntervel;
    if (type == NativeMemProfilerType::MEM_PROFILER_LIBRARY) {
        config->responseLibraryMode_ = true;
    } else if (type == NativeMemProfilerType::MEM_PROFILER_CALL_STACK) {
        config->responseLibraryMode_ = false;
    }
    return Start(config);
}

int32_t NativeMemoryProfilerSaClientManager::StopWithService(const sptr<IRemoteObject>& service, uint32_t pid)
{
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: stop service is null");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return proxy.Stop(pid);
}

int32_t NativeMemoryProfilerSaClientManager::Stop(uint32_t pid)
{
    CHECK_TRUE(pid != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: pid is 0");
    return StopWithService(GetRemoteService(), pid);
}

int32_t NativeMemoryProfilerSaClientManager::Stop(const std::string& name)
{
    CHECK_TRUE(!name.empty(), RET_ERR, "NativeMemoryProfilerSaClientManager: name is empty");
    auto service = GetRemoteService();
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: stop GetRemoteService failed");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return proxy.Stop(name);
}

// only for API
int32_t NativeMemoryProfilerSaClientManager::Stop()
{
    auto service = GetRemoteService();
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: stop GetRemoteService failed");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return proxy.Stop();
}

int32_t NativeMemoryProfilerSaClientManager::Stop(std::string& filePath)
{
    filePath = g_profilerAsyncReqMgr.GetFilepath(getpid());
    return Stop();
}

int32_t NativeMemoryProfilerSaClientManager::StopSync()
{
    auto service = GetRemoteService();
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: stop GetRemoteService failed");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return proxy.StopSync();
}

int32_t NativeMemoryProfilerSaClientManager::DumpData(uint32_t fd,
                                                      std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    CHECK_TRUE(CheckConfig(config), RET_ERR, "CheckConfig failed");
    return InvokeWithProxy([fd, &config](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, config);
    });
}

sptr<IRemoteObject> NativeMemoryProfilerSaClientManager::GetRemoteService()
{
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        return nullptr;
    }
    return abilityManager->CheckSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID);
}

bool NativeMemoryProfilerSaClientManager::CheckConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    CHECK_NOTNULL(config, false, "NativeMemoryProfilerSaClientManager: config is nullptr");
    if (config->duration_ == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: duration cannot be 0");
        return false;
    }
    if (config->filePath_.length() >= PATH_MAX) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: filePath_ invalid");
        return false;
    }
    if (config->shareMemorySize_ == 0) {
        config->shareMemorySize_ = config->responseLibraryMode_ ? LIB_SMS : CALL_STACK_SMS;
    }
    return true;
}

int32_t NativeMemoryProfilerSaClientManager::GetMallocStats(int fd, int pid, int type, bool printNmdOnly)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: GetMallocStats fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: GetMallocStats invalid pid");
    CHECK_TRUE(type == 0 || type == 1 || type == ONLY_NMD, RET_ERR,
               "NativeMemoryProfilerSaClientManager: type is invalid");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->printNmd_ = true;
    config->printNmdOnly_ = printNmdOnly;
    if (config->printNmdOnly_) {
        config->pid_ = static_cast<int32_t>(pid);
        config->duration_ = NMD_DURATION;
        config->mallocDisable_ = true;
        config->mmapDisable_ = true;
        config->shareMemorySize_ = NMD_SHMEM;
    }
    config->nmdPid_ = static_cast<uint32_t>(pid);
    config->nmdType_ = static_cast<uint32_t>(type);
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    CHECK_TRUE(CheckConfig(config), RET_ERR, "CheckConfig failed");
    return InvokeWithProxy([fd, &config](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, config);
    });
}

int32_t NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(
    pid_t pid, std::vector<SimplifiedMemStats>& memStats, bool isProfiling)
{
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: StartPrintSimplifiedNmd invalid pid");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->printNmd_ = true;
    config->printNmdOnly_ = true;
    config->printNmdInHook_ = isProfiling;
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = NMD_DURATION;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->shareMemorySize_ = NMD_SHMEM;
    config->nmdPid_ = static_cast<uint32_t>(pid);
    config->nmdType_ = static_cast<uint32_t>(SIMP_NMD);
    if (pid == getpid()) {
        config->startType_ = 1; // 1:API
    }
    CHECK_TRUE(CheckConfig(config), RET_ERR, "CheckConfig failed");
    int32_t ret = InvokeWithProxy([&config, &memStats, isProfiling, pid](NativeMemoryProfilerSaProxy& proxy) {
        std::string replyStats;
        int32_t r = proxy.Start(config, replyStats);
        if (r != RET_OK) {
            if (!isProfiling && proxy.Stop(static_cast<uint32_t>(pid)) != RET_OK) {
                PROFILER_LOG_ERROR(LOG_CORE, "StartPrintSimplifiedNmd Stop failed.");
            }
            return r;
        }
        size_t newlinePos = replyStats.find_first_of('\n');
        if (newlinePos != std::string::npos) {
            replyStats.erase(0, newlinePos + 1);
        }
        ParseMemStatsFromString(replyStats, memStats);
        if (!isProfiling) {
            r = proxy.Stop(static_cast<uint32_t>(pid));
            CHECK_TRUE(r == RET_OK, r, "StartPrintSimplifiedNmd Stop failed.");
        }
        return RET_OK;
    });
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartPrintSimplifiedNmd failed");
    }
    return ret;
}

std::vector<uint64_t> ConvertToVec(std::pair<Range, Range> range)
{
    std::vector<uint64_t> result;
    result.push_back(range.first.first);
    result.push_back(range.first.second);
    result.push_back(range.second.first);
    result.push_back(range.second.second);
    return result;
}

int32_t NativeMemoryProfilerSaClientManager::Start(int fd, pid_t pid, uint32_t duration, SimplifiedMemConfig& config)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Simplified Start invalid pid");
    std::shared_ptr<NativeMemoryProfilerSaConfig> nConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    nConfig->pid_ = static_cast<int32_t>(pid);
    CHECK_TRUE(ParseSimplifiedConfig(nConfig, config, duration) == RET_OK,
        RET_ERR, "NativeMemoryProfilerSaClientManager: Parse simplified config failed");
    return InvokeWithProxy([fd, &nConfig](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, nConfig);
    });
}

int32_t NativeMemoryProfilerSaClientManager::Start(int fd, pid_t pid, uint32_t duration, MemSaConfig& saConfig)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid pid with MemSaConfig");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = duration;
    config->statisticsInterval_ = FIVE_MINUTES;
    config->jsStackReport_ = true;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->memtraceEnable_ = true;
    config->asyncStackEnable_ = saConfig.asyncStackEnable;
    config->asyncFlag_ = saConfig.asyncFlag;
    config->traceMask_ = saConfig.mask;
    config->shareMemorySize_ = CALL_STACK_SMS;
    std::vector<std::vector<uint64_t>> gpuRange;
    if (!saConfig.hookSizes.empty()) {
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_VK]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_GLES_IMAGE]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_GLES_BUFFER]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_CL_IMAGE]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_CL_BUFFER]));
        std::swap(config->gpuRange_, gpuRange);
    }
    return InvokeWithProxy([fd, &config](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, config);
    });
}

int32_t NativeMemoryProfilerSaClientManager::StartAsync(int fd, pid_t pid, uint32_t duration, MemSaConfig& saConfig,
    ProfilingDumpCallback callback)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid pid with MemSaConfig");
    CHECK_TRUE(callback, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid callback with MemSaConfig");
    if ((saConfig.mask & RES_COMPOSITE_HEAP) && !(saConfig.mask & RES_ARK_GLOBAL_HANDLE)) {
        PROFILER_LOG_ERROR(LOG_CORE, "invalid mask, enable composite heap must enable global handle too");
        return RET_ERR;
    }
    CHECK_TRUE(g_profilerAsyncReqMgr.TryAddRequest(static_cast<int32_t>(pid), duration, callback, fd, ""), RET_ERR,
        "NativeMemoryProfilerSaClientManager: Started already with MemSaConfig for pid:%d", pid);
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = duration;
    config->statisticsInterval_ = FIVE_MINUTES;
    config->jsStackReport_ = true;
    config->mallocDisable_ = true;
    config->mmapAnonymousOnly_ = saConfig.mmapAnonymousOnly;
    if (!saConfig.mmapAnonymousOnly) {
        config->mmapDisable_ = true;
        config->memtraceEnable_ = true;
    }
    config->asyncStackEnable_ = saConfig.asyncStackEnable;
    config->asyncFlag_ = saConfig.asyncFlag;
    config->traceMask_ = saConfig.mask;
    config->shareMemorySize_ = CALL_STACK_SMS;
    std::vector<std::vector<uint64_t>> gpuRange;
    if (!saConfig.hookSizes.empty()) {
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_VK]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_GLES_IMAGE]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_GLES_BUFFER]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_CL_IMAGE]));
        gpuRange.push_back(ConvertToVec(saConfig.hookSizes[TAG_RES_GPU_CL_BUFFER]));
        std::swap(config->gpuRange_, gpuRange);
    }
    sptr<IRemoteObject> callbackObject = GetProfilingResultCallbackObject();
    if (callbackObject == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "get callback object FAIL");
        g_profilerAsyncReqMgr.DeleteRequest(config->pid_);
        return RET_ERR;
    }

    int32_t result = InvokeWithProxy([fd, &config, &callbackObject](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpDataAsync(fd, config, callbackObject);
    });
    if (result != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "invoke FAIL");
        g_profilerAsyncReqMgr.DeleteRequest(config->pid_);
    } else {
        g_profilerAsyncReqMgr.CreateTimeoutMonitor(config->pid_);
    }
    return result;
}

int32_t NativeMemoryProfilerSaClientManager::StartAsync(int fd, pid_t pid, uint32_t duration,
    SimplifiedMemConfig& config, ProfilingDumpCallback callback)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Simplified Start invalid pid");
    CHECK_TRUE(callback, RET_ERR, "NativeMemoryProfilerSaClientManager: invalid callback with SimplifiedMemConfig");
    std::shared_ptr<NativeMemoryProfilerSaConfig> nConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    nConfig->pid_ = static_cast<int32_t>(pid);
    CHECK_TRUE(ParseSimplifiedConfig(nConfig, config, duration) == RET_OK,
        RET_ERR, "NativeMemoryProfilerSaClientManager: Parse simplified config failed");
    CHECK_TRUE(g_profilerAsyncReqMgr.TryAddRequest(nConfig->pid_, duration, callback, fd, ""), RET_ERR,
        "NativeMemoryProfilerSaClientManager: Started already with SimplifiedMemConfig for pid:%d", pid);
    sptr<IRemoteObject> callbackObject = GetProfilingResultCallbackObject();
    if (callbackObject == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "get callback object FAIL");
        g_profilerAsyncReqMgr.DeleteRequest(nConfig->pid_);
        return RET_ERR;
    }

    int32_t result = InvokeWithProxy([fd, &nConfig, &callbackObject](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpDataAsync(fd, nConfig, callbackObject);
    });
    if (result != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "invoke FAIL");
        g_profilerAsyncReqMgr.DeleteRequest(nConfig->pid_);
    } else {
        g_profilerAsyncReqMgr.CreateTimeoutMonitor(nConfig->pid_);
    }
    return result;
}

int32_t NativeMemoryProfilerSaClientManager::StartMemProfiler(uint32_t duration, SimplifiedMemConfig& config,
    ProfilingCallback callback)
{
    CHECK_TRUE(duration >= DEFINE_MIN_MAX_DURATION && duration <= DEFINE_MAX_MAX_DURATION,
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG), "Invalid duration: %u", duration);
    CHECK_TRUE(config.sampleSize >= DEFINE_MIN_SAMPLE_INTERVAL,
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG), "Invalid sampleSize: %zu", config.sampleSize);

    std::shared_ptr<NativeMemoryProfilerSaConfig> nConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_TRUE(ParseSimplifiedConfig(nConfig, config, duration) == RET_OK,
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG), "Parse simplified config failed");
    return StartByType(static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_NATIVE), nConfig, callback);
}

int32_t NativeMemoryProfilerSaClientManager::UpdateSimplifiedMemConfig(pid_t pid, const SimplifiedMemConfig& config)
{
    CHECK_TRUE(pid > 0, RET_ERR, "UpdateSimplifiedMemConfig: invalid pid:%d", pid);
    std::shared_ptr<NativeMemoryProfilerSaConfig> nConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_TRUE(ParseSimplifiedConfig(nConfig, config, 0) == RET_OK, RET_ERR, "Parse simplified config failed");
    nConfig->pid_ = static_cast<int32_t>(pid);
    if (pid == getpid()) {
        nConfig->startType_ = 1; // 1:API
    }
    return InvokeWithProxy([&nConfig](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.Update(nConfig);
    });
}

int32_t NativeMemoryProfilerSaClientManager::StartProfilerByType(
    int32_t type, MemAPIConfig* inputConfig, ProfilingCallback callback)
{
    if (inputConfig == nullptr) {
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG);
    }

    ResProfilerErrorCode inputConfRet = CheckInputConfig(type, inputConfig);
    if (inputConfRet != ResProfilerErrorCode::RES_PROF_SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: Args invalid.errorcode is:%d.",
            static_cast<int32_t>(inputConfRet));
        return static_cast<int32_t>(inputConfRet);
    }

    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    int32_t retParse = ParseResType(type, config);
    if (retParse != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: ParseResType failed");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG);
    }
    int32_t retNormalConfig = ParseNormalConfig(config, inputConfig);
    if (retNormalConfig != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: ParseNormalConfig failed");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG);
    }
    return StartByType(type, config, callback);
}

int32_t NativeMemoryProfilerSaClientManager::StartByType(int32_t type,
    std::shared_ptr<NativeMemoryProfilerSaConfig>& config, ProfilingCallback callback)
{
    config->pid_ = getpid();
    config->bundleName_ = COMMON::GetProcessNameByPid(config->pid_);

    std::string filename = GenerateFileName(type, config->pid_);
    std::string fullPath = CreateFileAndGrantPermission(filename);
    CHECK_TRUE(!fullPath.empty(), RET_ERR, "create file or grant permission FAIL");
    config->typeFileName_ = filename;
    config->startType_ = 1; // 1:API

    if (!g_profilerAsyncReqMgr.TryAddRequest(config->pid_, config->duration_, callback, -1, fullPath)) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartByType already started for pid: %d", config->pid_);
        (void)unlink(fullPath.c_str());
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ALREADY_STARTED);
    }

    sptr<IRemoteObject> callbackObject = GetProfilingResultCallbackObject();
    if (callbackObject == nullptr) {
        (void)unlink(fullPath.c_str());
        g_profilerAsyncReqMgr.DeleteRequest(config->pid_);
        return RET_ERR;
    }

    if (!COMMON::CleanOldenFiles(NAPI_FILE_PATH_PREFIX)) {
        PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaClientManager: CleanOldenFiles failed");
    }
    if (static_cast<InternalResourceType>(type) == InternalResourceType::OH_RES_TYPE_COMPOSITE_HEAP) {
        auto filter = [type, &config](const std::string &filename) {
            return COMMON::StringMatch(filename, GenerateFileNamePrefix(type, config->pid_), COMPOSITE_HEAP_SUFFIX) &&
                filename != config->typeFileName_;
        };
        if (!COMMON::DeleteFiles(std::filesystem::path(fullPath).parent_path().string(), filter)) {
            PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaClientManager: Clean composite file failed");
        }
    }

    int32_t retStart = StartPidVoid(config, callbackObject);
    if (retStart != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: StartPidVoid failed");
        (void)unlink(fullPath.c_str());
        g_profilerAsyncReqMgr.DeleteRequest(config->pid_);
        return retStart;
    }
    g_profilerAsyncReqMgr.CreateTimeoutMonitor(config->pid_);
    return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS);
}

int32_t NativeMemoryProfilerSaClientManager::ParseResType(int32_t type,
    std::shared_ptr<NativeMemoryProfilerSaConfig> config)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    InternalResourceType internalType = static_cast<InternalResourceType>(type);
    if (internalType == InternalResourceType::OH_RES_TYPE_NATIVE) {
        config->mallocDisable_ = false;
        config->memtraceEnable_ = false;
        return RET_OK;
    } else {
        config->mallocDisable_ = true;
        config->memtraceEnable_ = true;
    }

    config->shareMemorySize_ = CALL_STACK_SMS;
    if (internalType == InternalResourceType::OH_RES_TYPE_GPU) {
        config->traceMask_ = RES_GPU_MASK;
        SetDefaultGpuRange(config);
    } else if (internalType == InternalResourceType::OH_RES_TYPE_FD) {
        config->traceMask_ = RES_FD_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_THREAD) {
        config->traceMask_ = RES_THREAD_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE) {
        config->traceMask_ = RES_ARK_GLOBAL_HANDLE;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_DMA) {
        config->traceMask_ = RES_FD_DMABUF | RES_DMABUF_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_ASHMEM) {
        config->traceMask_ = RES_FD_ASHMEM | RES_ASHMEM_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_COMPOSITE_HEAP) {
        config->traceMask_ = RES_COMPOSITE_HEAP | RES_ARK_GLOBAL_HANDLE;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaClientManager::ParseNormalConfig(
    std::shared_ptr<NativeMemoryProfilerSaConfig> config, MemAPIConfig* inputConfig)
{
    if (config == nullptr || inputConfig == nullptr) {
        return RET_ERR;
    }
    config->duration_ = inputConfig->maxDuration;
    config->jsStackReport_ = 1;
    config->maxStackDepth_ = inputConfig->maxStackDepth;
    config->sampleInterval_ = inputConfig->sampleInterval;
    config->filterSize_ = static_cast<int32_t>(inputConfig->filterSize);
    config->statisticsInterval_ = inputConfig->statisticsInterval;
    config->asyncStackEnable_ = inputConfig->asyncStackEnable;
    config->nestedAsyncStackEnable_ = inputConfig->nestedAsyncStackEnable;
    config->asyncNestingDepth_ = inputConfig->asyncNestingDepth;
    config->asyncTaskStackDepth_ = inputConfig->asyncTaskStackDepth;
    config->discardDestroyedTraces_ = inputConfig->discardDestroyedTraces;
    if (config->traceMask_ == RES_GPU_MASK && !inputConfig->hookSizes.empty()) {
        std::vector<std::vector<uint64_t>> gpuRange;
        gpuRange.push_back(ConvertToVec(inputConfig->hookSizes[TAG_RES_GPU_VK]));
        gpuRange.push_back(ConvertToVec(inputConfig->hookSizes[TAG_RES_GPU_GLES_IMAGE]));
        gpuRange.push_back(ConvertToVec(inputConfig->hookSizes[TAG_RES_GPU_GLES_BUFFER]));
        gpuRange.push_back(ConvertToVec(inputConfig->hookSizes[TAG_RES_GPU_CL_IMAGE]));
        gpuRange.push_back(ConvertToVec(inputConfig->hookSizes[TAG_RES_GPU_CL_BUFFER]));
        std::swap(config->gpuRange_, gpuRange);
    }
    if (inputConfig->asyncStackEnable && inputConfig->nestedAsyncStackEnable) {
        config->asyncFlag_ = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
                             ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE |
                             ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL;
    } else if (inputConfig->asyncStackEnable) {
        config->asyncFlag_ = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
                             ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROMISE | ASYNC_TYPE_CUSTOMIZE |
                             ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL | ASYNC_TYPE_ARKTS_STA_LAUNCH |
                             ASYNC_TYPE_ARKWEB | ASYNC_TYPE_JSVM;
    } else {
        config->asyncFlag_ = 0;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaClientManager::ParseSimplifiedConfig(
    std::shared_ptr<NativeMemoryProfilerSaConfig> config, const SimplifiedMemConfig& inputConfig, uint32_t duration)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    config->duration_ = duration;
    config->largestSize_ = inputConfig.largestSize;
    config->secondLargestSize_ = inputConfig.secondLargestSize;
    config->maxGrowthSize_ = inputConfig.maxGrowthSize;
    config->sampleInterval_ = inputConfig.sampleSize;
    config->shareMemorySize_ = inputConfig.responseLibraryMode ? LIB_SMS : CALL_STACK_SMS;
    config->statisticsInterval_ = FIVE_MINUTES;
    config->jsStackReport_ = true;
    config->maxJsStackDepth_ = JS_STACK_DEPS;
    config->mallocDisable_ = false;
    config->mmapDisable_ = false;
    config->memtraceEnable_ = false;
    config->asyncStackEnable_ = inputConfig.asyncStackEnable;
    config->nestedAsyncStackEnable_ = inputConfig.nestedAsyncStackEnable;
    config->asyncNestingDepth_ = inputConfig.asyncNestingDepth;
    config->asyncTaskStackDepth_ = inputConfig.asyncTaskStackDepth;
    config->asyncFlag_ = inputConfig.asyncFlag;
    config->targetSoName_ = inputConfig.targetSoName;
    config->responseLibraryMode_ = inputConfig.responseLibraryMode;
    return RET_OK;
}

ResProfilerErrorCode NativeMemoryProfilerSaClientManager::CheckInputConfig(int type, MemAPIConfig* inputConfig)
{
    if (inputConfig == nullptr) {
        return ResProfilerErrorCode::RES_PROF_INVALID_ARG;
    }
    if (inputConfig->maxDuration < DEFINE_MIN_MAX_DURATION || inputConfig->maxDuration > DEFINE_MAX_MAX_DURATION) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: maxDuration is invalid");
        return ResProfilerErrorCode::RES_PROF_MAX_DURATION_INVALID_ARG;
    }
    if (inputConfig->filterSize < DEFINE_MIN_FILTER_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: filterSize is invalid");
        return ResProfilerErrorCode::RES_PROF_FILTER_SIZE_INVALID_ARG;
    }
    if (inputConfig->maxStackDepth == 0 || inputConfig->maxStackDepth > DEFINE_MAX_STACK_DEPTH) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: maxStackDepth is invalid");
        return ResProfilerErrorCode::RES_PROF_MAX_STACK_DEPTH_INVALID_ARG;
    }
    if (inputConfig->statisticsInterval == 0 || inputConfig->statisticsInterval > DEFINE_MAX_STATISTICS_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: statisticsInterval is invalid");
        return ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG;
    }
    if (inputConfig->sampleInterval < DEFINE_MIN_SAMPLE_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: sampleInterval is invalid");
        return ResProfilerErrorCode::RES_PROF_SAMPLE_INTERVAL_INVALID_ARG;
    }

    if (type < static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_FD) ||
        type >= static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX)) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: type is invalid");
        return ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG;
    }
    return ResProfilerErrorCode::RES_PROF_SUCCESS;
}

int32_t NativeMemoryProfilerSaClientManager::Prepare()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> sam =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: Prepare failed");
        return RET_PREPARE_FAIL;
    }
    auto remoteObject = sam->CheckSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID);
    if (remoteObject == nullptr) {
        PROFILER_LOG_DEBUG(LOG_CORE, "NativeMemoryProfilerSaClientManager: Prepare start");
        int32_t nativeDaemonPid = 0;
        if (COMMON::IsProcessExist("native_daemon", nativeDaemonPid)) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "NativeMemoryProfilerSaClientManager: Native Daemon is already running, skip SA load, errorcode is:%d.",
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
            return RET_PREPARE_FAIL;
        }
        int32_t retStart = OnDemandStart(sam, remoteObject);
        if (remoteObject == nullptr || retStart != 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: Prepare start failed");
            return RET_PREPARE_FAIL;
        }
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaClientManager::CheckStatus()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> sam =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remoteObject = sam->CheckSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID);
    if (remoteObject == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: CheckStatus failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaClientManager::OnDemandStart(
    OHOS::sptr<OHOS::ISystemAbilityManager> sam, OHOS::sptr<OHOS::IRemoteObject> &remoteObject)
{
    OHOS::sptr<OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaLazyLoadCallback> loadCallback =
        new (std::nothrow) OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaLazyLoadCallback();
    CHECK_NOTNULL(loadCallback, RET_ERR, "NativeMemoryProfilerSaClientManager: OnDemandStart loadCallback is nullptr");
    int32_t ret = sam->LoadSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID, loadCallback);
    if (ret != OHOS::ERR_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: OnDemandStart failed");
        return RET_ERR;
    }
    static constexpr int32_t SLEEP_DUR = 5 * 1000 * 1000;
    static constexpr int32_t SLEEP_UNIT = 100 * 1000;

    int32_t loop = SLEEP_DUR / SLEEP_UNIT;
    while (loop-- > 0) {
        if (!loadCallback->CheckLoadSystemAbilityStatus()) {
            usleep(SLEEP_UNIT);
            continue;
        }
        remoteObject = sam->CheckSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID);
        if (remoteObject != nullptr) {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaClientManager: OnDemandStart success");
            return RET_OK;
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaClientManager: OnDemandStart remoteObject is nullptr");
            usleep(SLEEP_UNIT);
        }
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "NativeMemoryProfilerSaClientManager: OnDemandStart timeout");
    return RET_OK;
}

// <resource-type>-<process_name>
std::string NativeMemoryProfilerSaClientManager::GenerateFileNamePrefix(int type, int32_t pid)
{
    auto it = resourceTypeValues.find(static_cast<InternalResourceType>(type));
    if (it == resourceTypeValues.end()) {
        return "";
    }
    std::string typeVal = it->second;
    std::string procName = COMMON::GetProcessNameByPid(pid);
    return typeVal + "-" + procName;
}

// <resource-type>-<process_name>-<pid>-<timestamp>.suffix(.trace or .zip)
std::string NativeMemoryProfilerSaClientManager::GenerateFileName(int type, int32_t pid)
{
    std::string prefix = GenerateFileNamePrefix(type, pid);
    if (prefix.empty()) {
        return "";
    }
    std::string timeStr = COMMON::GetTimeStr();
    bool isCompositeHeap = type == static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_COMPOSITE_HEAP);
    std::string suffix = isCompositeHeap ? COMPOSITE_HEAP_SUFFIX : FILE_SUFFIX;
    return prefix + "-" + std::to_string(pid) + "-" + timeStr + suffix;
}

int32_t NativeMemoryProfilerSaClientManager::GetGwpAsan(int fd, pid_t pid, GwpAsanConfig& gwpConfig)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid pid with MemSaConfig");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = GWP_DURATION;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->shareMemorySize_ = GWP_SHMEM;
    config->gwpAsanHoldTime_ = gwpConfig.holdTime;
    config->gwpAsanMaxEntries_ = gwpConfig.maxEntries;
    config->gwpAsanMaxStackDepth_ = gwpConfig.maxStackDepth;
    return InvokeWithProxy([fd, &config](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, config);
    });
}

std::string NativeMemoryProfilerSaClientManager::CreateFileAndGrantPermission(std::string& filename)
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    CHECK_NOTNULL(context, "", "context is NULL");
    CHECK_TRUE(OHOS::StorageDaemon::AclSetAccess(NAPI_BASE_PATH, "u:3063:x") == 0, "", "grant base dir FAIL");
    CHECK_TRUE(OHOS::StorageDaemon::AclSetAccess(NAPI_FILE_PATH, "u:3063:x") == 0, "", "grant files dir FAIL");
    std::string fullPath = NAPI_FILE_PATH + "/" + filename;
    CHECK_TRUE(COMMON::CreateFile(fullPath), "", "create file FAIL");
    if (OHOS::StorageDaemon::AclSetAccess(fullPath, "u:3063:rw") != 0) {
        (void)unlink(fullPath.c_str());
        PROFILER_LOG_ERROR(LOG_CORE, "grant file permission FAIL");
        return "";
    }
    return fullPath;
}

} // namespace OHOS::Developtools::NativeDaemon
