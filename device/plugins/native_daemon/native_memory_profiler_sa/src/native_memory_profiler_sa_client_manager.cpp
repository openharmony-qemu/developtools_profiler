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
#include "common.h"

#include "logging.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>

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
constexpr uint32_t DEFINE_MIN_MAX_DURATION = 1; // 1 second
constexpr uint32_t DEFINE_MAX_MAX_DURATION = 3600; // 60 * 60  seconds
constexpr uint32_t DEFINE_MIN_FILTER_SIZE = 1; // bytes
constexpr uint32_t DEFINE_MAX_STACK_DEPTH = 30;
constexpr uint32_t DEFINE_MAX_STATISTICS_INTERVAL = 3600; // seconds
constexpr uint32_t DEFINE_MIN_SAMPLE_INTERVAL = 384; // bytes
constexpr int32_t MAX_MEM_PROFILER_USE_COUNT_PER_DAY = 10;
constexpr int HOURS_PER_DAY = 24;
const std::string MEM_PROFILER_COUNT_FILE_NAME = "/MemProfilerCount";
const std::string FILE_SUFFIX = ".htrace";
const std::vector<std::string> resourceTypeValues = { "fd", "thread", "native", "gpu", "global_handle" };
}

static std::mutex g_profilingCallbackMutex;
static NativeMemoryProfilerSaClientManager::ProfilingCallback g_profilingCallback = nullptr;
static std::string g_currentFilePath;
static int32_t g_currentType = 0;

static NativeMemoryProfilerSaClientManager::ProfilingCallback TakeProfilingCallback(std::string& filePath)
{
    std::lock_guard<std::mutex> lock(g_profilingCallbackMutex);
    filePath = g_currentFilePath;
    NativeMemoryProfilerSaClientManager::ProfilingCallback callback = g_profilingCallback;
    g_profilingCallback = nullptr;
    g_currentFilePath.clear();
    return callback;
}

class NativeMemoryProfilerSaResultCallback final : public NativeMemoryProfilerSaCallbackStub {
public:
    void OnProfilingResult(int32_t errorCode, const std::string& filePath) override
    {
        std::string currentPath;
        NativeMemoryProfilerSaClientManager::ProfilingCallback callback = TakeProfilingCallback(currentPath);
        if (callback != nullptr) {
            callback(errorCode, filePath);
        }
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
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
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

int32_t NativeMemoryProfilerSaClientManager::Stop(uint32_t pid)
{
    CHECK_TRUE(pid != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: pid is 0");
    auto service = GetRemoteService();
    if (service == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: stop GetRemoteService failed");
        return RET_ERR;
    }
    NativeMemoryProfilerSaProxy proxy(service);
    return proxy.Stop(pid);
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
    int32_t ret = proxy.Stop();
    if (ret != RET_OK) {
        std::string filePath;
        ProfilingCallback callback = TakeProfilingCallback(filePath);
        if (callback != nullptr) {
            callback(ret, "");
        }
    }
    return ret;
}

int32_t NativeMemoryProfilerSaClientManager::Stop(std::string& filePath)
{
    filePath = g_currentFilePath;
    return Stop();
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

int32_t NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid_t pid,
                                                                     std::vector<SimplifiedMemStats>& memStats)
{
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: StartPrintSimplifiedNmd invalid pid");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    config->printNmd_ = true;
    config->printNmdOnly_ = true;
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
    int32_t ret = InvokeWithProxy([&config, &memStats](NativeMemoryProfilerSaProxy& proxy) {
        std::string replyStats;
        int32_t r = proxy.Start(config, replyStats);
        if (r != RET_OK) {
            return r;
        }
        size_t newlinePos = replyStats.find_first_of('\n');
        if (newlinePos != std::string::npos) {
            replyStats.erase(0, newlinePos + 1);
        }
        SimplifiedMemStats tmp;
        std::istringstream iss(replyStats);
        std::stringstream ss;
        std::string line;
        while (std::getline(iss, line, '\n')) {
            ss.str("");
            ss.clear();
            ss.str(line);
            ss >> tmp.size >> tmp.allocated >> tmp.nmalloc >> tmp.ndalloc;
            memStats.push_back(tmp);
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
    CHECK_NOTNULL(nConfig, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
    nConfig->pid_ = static_cast<int32_t>(pid);
    nConfig->duration_ = duration;
    nConfig->largestSize_ = config.largestSize;
    nConfig->secondLargestSize_ = config.secondLargestSize;
    nConfig->maxGrowthSize_ = config.maxGrowthSize;
    nConfig->sampleInterval_ = config.sampleSize;
    nConfig->shareMemorySize_ = CALL_STACK_SMS;
    nConfig->statisticsInterval_ = FIVE_MINUTES;
    nConfig->jsStackReport_ = true;
    nConfig->maxJsStackDepth_ = JS_STACK_DEPS;
    return InvokeWithProxy([fd, &nConfig](NativeMemoryProfilerSaProxy& proxy) {
        return proxy.DumpData(fd, nConfig);
    });
}

int32_t NativeMemoryProfilerSaClientManager::Start(int fd, pid_t pid, uint32_t duration, MemSaConfig& saConfig)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid pid with MemSaConfig");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: config is nullptr");
 
    config->pid_ = static_cast<int32_t>(pid);
    config->duration_ = duration;
    config->statisticsInterval_ = FIVE_MINUTES;
    config->jsStackReport_ = false;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->memtraceEnable_ = true;
    config->asyncStackEnable_ = false;
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

    if (GetMemProfilerUseCountToday() >= MAX_MEM_PROFILER_USE_COUNT_PER_DAY) {
        PROFILER_LOG_ERROR(LOG_CORE, "Daily use count exceeded,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED);
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
    config->pid_ = getpid();
    std::string filename = GenerateFileName(type, config->pid_);
    std::string fullPath = NAPI_FILE_PATH_PREFIX + filename;
    if (!COMMON::CreateFile(fullPath)) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: CreateFile failed");
        return RET_ERR;
    }
    config->typeFileName_ = filename;
    config->startType_ = 1; // 1:API

    if (!UpdateMemProfilerUserCount()) {
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG);
    }

    g_currentType = type;
    g_currentFilePath = fullPath;
    SetProfilingCallback(callback);
    sptr<IRemoteObject> callbackObject = GetProfilingResultCallbackObject();
    if (callbackObject == nullptr) {
        SetProfilingCallback(nullptr);
        if (!RollbackMemProfilerUserCount()) {
            PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaClientManager: rollback count failed");
        }
        return RET_ERR;
    }

    if (!COMMON::CleanOldenFiles(NAPI_FILE_PATH_PREFIX)) {
        PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaClientManager: CleanOldenFiles failed");
    }

    int32_t retStart = StartPidVoid(config, callbackObject);
    if (retStart != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaClientManager: StartPidVoid failed");
        SetProfilingCallback(nullptr);
        if (!RollbackMemProfilerUserCount()) {
            PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaClientManager: rollback count failed");
        }
        return retStart;
    }
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
        config->mmapDisable_ = false;
        config->memtraceEnable_ = false;
        return RET_OK;
    } else {
        config->mallocDisable_ = true;
        config->mmapDisable_ = true;
        config->memtraceEnable_ = true;
    }

    config->shareMemorySize_ = CALL_STACK_SMS;
    config->asyncStackEnable_ = false;
    if (internalType == InternalResourceType::OH_RES_TYPE_GPU) {
        config->traceMask_ = RES_GPU_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_FD) {
        config->traceMask_ = RES_FD_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_THREAD) {
        config->traceMask_ = RES_THREAD_MASK;
    } else if (internalType == InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE) {
        config->traceMask_ = RES_ARK_GLOBAL_HANDLE;
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
    if (inputConfig->maxStackDepth > DEFINE_MAX_STACK_DEPTH) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: maxStackDepth is invalid");
        return ResProfilerErrorCode::RES_PROF_MAX_STACK_DEPTH_INVALID_ARG;
    }
    if (inputConfig->statisticsInterval > DEFINE_MAX_STATISTICS_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: statisticsInterval is invalid");
        return ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG;
    }
    if (inputConfig->sampleInterval < DEFINE_MIN_SAMPLE_INTERVAL) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: sampleInterval is invalid");
        return ResProfilerErrorCode::RES_PROF_SAMPLE_INTERVAL_INVALID_ARG;
    }

    if (type < static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_FD) ||
        type > static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE)) {
        PROFILER_LOG_ERROR(LOG_CORE, "CheckInputConfig: type is invalid");
        return ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG;
    }
    return ResProfilerErrorCode::RES_PROF_SUCCESS;
}

bool NativeMemoryProfilerSaClientManager::ReadCountFile(const std::string &filepath,
    long long &timestamp, int32_t &count)
{
    char canonicalPath[PATH_MAX + 1] = {0};
    if (realpath(filepath.c_str(), canonicalPath) == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadCountFile: failed to get canonical path");
        return false;
    }

    std::ifstream ifs(canonicalPath);
    if (!ifs.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadCountFile: failed to open file");
        return false;
    }
    ifs >> timestamp >> count;
    if (!ifs.good() && !ifs.eof()) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadCountFile: failed to read data");
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}

bool NativeMemoryProfilerSaClientManager::WriteCountFileAtomic(const std::string &filepath,
    long long timestamp, int32_t count)
{
    std::string tpl = filepath + ".tmp.XXXXXX";
    std::vector<char> tmpl(tpl.begin(), tpl.end());
    tmpl.push_back('\0');

    int fd = mkstemp(tmpl.data());
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteCountFileAtomic: failed to create temp file, errno: %d", errno);
        return false;
    }

    std::string content = std::to_string(static_cast<long long>(timestamp)) + " " + std::to_string(count) + "\n";
    const char *buf = content.c_str();
    size_t toWrite = content.size();
    ssize_t written = write(fd, buf, toWrite);
    if (written != static_cast<ssize_t>(toWrite)) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteCountFileAtomic: failed to write data, errno: %d", errno);
        close(fd);
        unlink(tmpl.data());
        return false;
    }

    if (fsync(fd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteCountFileAtomic: failed to sync data, errno: %d", errno);
        close(fd);
        unlink(tmpl.data());
        return false;
    }

    if (close(fd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteCountFileAtomic: failed to close file, errno: %d", errno);
        unlink(tmpl.data());
        return false;
    }

    if (rename(tmpl.data(), filepath.c_str()) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteCountFileAtomic: failed to rename temp file, errno: %d", errno);
        unlink(tmpl.data());
        return false;
    }
    return true;
}

inline bool NativeMemoryProfilerSaClientManager::IsSeparateByMoreThan24Hours(long long t1, long long t2)
{
    using namespace std::chrono;
    if (t1 < 0 || t2 < 0) {
        return false;
    }
    if (t1 > t2) {
        std::swap(t1, t2);
    }
    return duration_cast<hours>(system_clock::from_time_t(t2) - system_clock::from_time_t(t1)) > hours(HOURS_PER_DAY);
}

bool NativeMemoryProfilerSaClientManager::UpdateMemProfilerUserCount()
{
    const std::string filepath = std::string(NAPI_FILE_PATH_PREFIX) + std::string(MEM_PROFILER_COUNT_FILE_NAME);

    time_t now = time(nullptr);
    if (now == static_cast<time_t>(-1)) {
        PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: failed to get current time");
        return false;
    }

    long long nowTime = static_cast<long long>(now);
    long long timeStamp = static_cast<long long>(now);
    int32_t count = 0;
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: filepath not exist,count set 1.");
        count = 1;
        if (!COMMON::CreateFile(filepath)) {
            PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: failed to create file,errorno=%d %s.",
                errno, strerror(errno));
            return false;
        }
    } else {
        if (!ReadCountFile(filepath, timeStamp, count)) {
            PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: failed to read count file");
            return false;
        }
        if (IsSeparateByMoreThan24Hours(timeStamp, nowTime)) {
            count = 1;
        } else {
            count++;
        }
    }

    if (!WriteCountFileAtomic(filepath, nowTime, count)) {
        PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: failed to write count file");
        return false;
    }
    return true;
}

bool NativeMemoryProfilerSaClientManager::RollbackMemProfilerUserCount()
{
    const std::string filepath = std::string(NAPI_FILE_PATH_PREFIX) + std::string(MEM_PROFILER_COUNT_FILE_NAME);
    long long timeStamp = 0;
    int32_t count = 0;
    if (!ReadCountFile(filepath, timeStamp, count)) {
        PROFILER_LOG_ERROR(LOG_CORE, "RollbackMemProfilerUserCount: failed to read count file");
        return false;
    }
    if (count <= 0) {
        return true;
    }
    if (!WriteCountFileAtomic(filepath, timeStamp, count - 1)) {
        PROFILER_LOG_ERROR(LOG_CORE, "RollbackMemProfilerUserCount: failed to write count file");
        return false;
    }
    return true;
}

int32_t NativeMemoryProfilerSaClientManager::GetMemProfilerUseCountToday()
{
    const std::string filepath = std::string(NAPI_FILE_PATH_PREFIX) + std::string(MEM_PROFILER_COUNT_FILE_NAME);
    int32_t count = 0;
    struct stat st;

    if (stat(filepath.c_str(), &st) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetMemProfilerUseCountToday: filepath not exist");
        count = 0;
    } else {
        long long timeStamp = static_cast<long long>(st.st_mtime);
        if (!ReadCountFile(filepath, timeStamp, count)) {
            PROFILER_LOG_ERROR(LOG_CORE, "GetMemProfilerUseCountToday: failed to read count file");
            return MAX_MEM_PROFILER_USE_COUNT_PER_DAY; // Return max count to prevent profiler start if read fails
        }

        long long nowTime = static_cast<long long>(time(nullptr));
        if (IsSeparateByMoreThan24Hours(timeStamp, nowTime)) {
            count = 0;
        }
    }
    return count;
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

// <resource-type>-<process_name>-<pid>-<timestamp>.htrace
std::string NativeMemoryProfilerSaClientManager::GenerateFileName(int type, int32_t pid)
{
    if (type < 0 || type >= static_cast<int>(resourceTypeValues.size())) {
        return "";
    }
    std::string typeVal = resourceTypeValues[type];
    std::string procName = COMMON::GetProcessNameByPid(pid);
    std::string timeStr = COMMON::GetTimeStr();
    return typeVal + "-" + procName + "-" + std::to_string(pid) + "-" + timeStr + FILE_SUFFIX;
}

int32_t NativeMemoryProfilerSaClientManager::GetGwpAsan(int fd, pid_t pid, GwpAsanConfig& gwpConfig)
{
    CHECK_TRUE(fd != 0, RET_ERR, "NativeMemoryProfilerSaClientManager: fd is 0");
    CHECK_TRUE(pid > 0, RET_ERR, "NativeMemoryProfilerSaClientManager: Start, invalid pid with MemSaConfig");
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    CHECK_NOTNULL(config, RET_ERR, "NativeMemoryProfilerSaClientManager: saConfig is nullptr");
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

void NativeMemoryProfilerSaClientManager::SetProfilingCallback(ProfilingCallback callback)
{
    std::lock_guard<std::mutex> lock(g_profilingCallbackMutex);
    g_profilingCallback = callback;
    if (callback == nullptr) {
        g_currentFilePath.clear();
    }
}

} // namespace OHOS::Developtools::NativeDaemon
