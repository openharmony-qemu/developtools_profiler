/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "hook_manager.h"

#include <fcntl.h>
#include <limits>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <dirent.h>
#include "async_stack.h"
#include "cache_file_reader.h"
#include "command_poller.h"
#include "common.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"
#include "hook_common.h"
#include "hook_service.h"
#include "init_param.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "share_memory_allocator.h"
#include "utilities.h"
#include "virtual_runtime.h"
#include "native_memory_profiler_sa_service.h"
#include "hook_record.h"
#include "hook_record_factory.h"
#include "unique_stack_table.h"
#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "res_sched_client.h"
#include "res_type.h"
#include "nlohmann/json.hpp"

namespace OHOS::Developtools::NativeDaemon {
namespace {
inline void ApplyLiveDataByType(const BaseStackRawData* data,
    std::unordered_map<void*, struct timespec>::iterator iter,
    std::unordered_map<void*, struct timespec>& liveData)
{
    if (IsAllocRecord(data)) {
        iter->second = data->ts;
    } else if (IsFreeRecord(data)) {
        liveData.erase(iter);
    }
}

inline void UpdateLiveData(const BaseStackRawData* data,
    std::unordered_map<void*, struct timespec>& liveData,
    std::unordered_map<void*, struct timespec>& orphanFreeWhitelist)
{
    auto iter = liveData.find(data->addr);
    if (iter == liveData.end()) {
        if (IsAllocRecord(data)) {
            liveData[data->addr] = data->ts;
        } else if (IsFreeRecord(data)) {
            orphanFreeWhitelist.emplace(data->addr, data->ts);
        }
        return;
    }
    if (TsLess(iter->second, data->ts)) {
        ApplyLiveDataByType(data, iter, liveData);
    }
}

constexpr int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
constexpr uint32_t PAGE_BYTES = 4096;
constexpr uint32_t SHMEM_DRAIN_POLL_MS = 10;
constexpr uint32_t SHMEM_DRAIN_QUIET_ROUNDS = 3;
constexpr uint32_t SHMEM_DRAIN_TIMEOUT_MS = 5000;
constexpr uint32_t MEM_LEAK_DONE_POLL_MS = 10;
constexpr uint32_t MEM_LEAK_DONE_TIMEOUT_MS = 30000;
constexpr uint32_t NS_PER_MS = 1000 * 1000;
constexpr uint32_t MS_PER_SEC = 1000;
std::shared_ptr<Writer> g_buffWriter;
const std::string STARTUP = "startup:";
const std::string PARAM_NAME = "libc.hook_mode";
const std::string PRELOAD_NAME = "hiviewdfx.hiprofiler.preload";
const std::string ARKTS_DIAG_PATH = "const.ark.jsruntime.dfx_library";
const std::string PRODUCT_CONFIG_PATH = "/sys_prod/etc/hiview/hiprofiler/hiprofiler_cfg.json";
constexpr int SIGNAL_START_HOOK = 36;
constexpr int SIGNAL_STOP_HOOK = 37;
const std::string VERSION = "1.02";
constexpr int32_t RESPONSE_MAX_PID_COUNT = 8;
constexpr int32_t ONLY_NMD_TYPE = 2;
constexpr int32_t SIMP_NMD = 3;
constexpr int DOUBLE = 2;
constexpr mode_t FILE_CACHE_MODE = S_IRUSR | S_IWUSR;
const std::string FILE_CACHE_TEMPLATE = "native_hook_cache_XXXXXX";
constexpr uint64_t ALL_NESTED_MASK_TYPE = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
    ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE | ASYNC_TYPE_ARKTS_WORKER |
    ASYNC_TYPE_ARKTS_TASKPOOL;
const std::string MEM_LEAK_FILE_PERFIX = "memleak-native-orphansnapshot-";
const std::string MEM_LEAK_FILE_SUFFIX = ".txt";
const std::string MEM_LEAK_FILE_PATH = "/data/local/tmp/";
const std::string SIMPLIFIED_NMD_TEMP_FILE = "/data/log/hiprofiler/simplified_nmd.tmp.XXXXXX";
constexpr uint32_t MAX_BUFFER_SIZE = 8192;

// Constant definitions
constexpr uint32_t VALID_LIVE_DATA_RESERVE_COUNT = 9 * 1024 * 1024;

#ifndef __aarch64__
constexpr uint32_t MAX_MAP_BUFFER_SIZE = 2147479552;
#endif
uint32_t GetUniqueStackTableSizeWithDefault(const NativeHookConfig& config)
{
    uint32_t currSize = config.unique_stack_table_size() * ONE_MEGABYTE;
    return NormalizeUniqueStackTableSize(currSize);
}

bool CopyFdContent(int destFd, int srcFd)
{
    if (lseek(srcFd, 0, SEEK_SET) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyFdContent lseek failed errno=%d", errno);
        return false;
    }
    struct stat statBuf;
    if (fstat(srcFd, &statBuf) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyFdContent fstat failed errno=%d", errno);
        return false;
    }
    off_t offset = 0;
    while (offset < statBuf.st_size) {
        ssize_t copied = sendfile(destFd, srcFd, &offset, static_cast<size_t>(statBuf.st_size - offset));
        if (copied < 0 && errno == EINTR) {
            continue;
        }
        if (copied <= 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "CopyFdContent sendfile failed errno=%d", errno);
            return false;
        }
    }
    if (fsync(destFd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyFdContent fsync failed errno=%d", errno);
        return false;
    }
    return true;
}

bool ReadFdContent(std::string& destStr, int srcFd)
{
    if (lseek(srcFd, 0, SEEK_SET) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFdContent lseek failed errno=%d", errno);
        return false;
    }
    struct stat statBuf;
    if (fstat(srcFd, &statBuf) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFdContent fstat failed errno=%d", errno);
        return false;
    }
    if (statBuf.st_size <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFdContent fail, no content");
        return false;
    }
    destStr.reserve(destStr.size() + static_cast<size_t>(statBuf.st_size));
    char buf[MAX_BUFFER_SIZE];
    ssize_t readed = 0;
    while ((readed = read(srcFd, buf, sizeof(buf))) != 0) {
        if (readed < 0) {
            if (errno == EINTR) {
                continue;
            }
            PROFILER_LOG_ERROR(LOG_CORE, "ReadFdContent read failed errno=%d", errno);
            return false;
        }
        destStr.append(buf, static_cast<size_t>(readed));
    }
    return true;
}

static thread_local std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> g_rawDataArray = {};
static thread_local uint32_t g_rawStackCount = 0;
}

HookManager::HookManager()
{
}

HookManager::~HookManager()
{
    hookService_ = nullptr;
    for (const auto& item : hookCtx_) {
        for (size_t i = 0; i < item->shareMemoryBlockList.size(); ++i) {
            item->shareMemoryBlockList[i] = nullptr;
        }
        for (size_t i = 0; i < item->eventPollerList.size(); ++i) {
            item->eventPollerList[i] = nullptr;
        }
        if (item->readyFileNotifier != nullptr) {
            item->readyFileNotifier = nullptr;
        }
        if (item->readyFilePoller != nullptr) {
            item->readyFilePoller = nullptr;
        }
        if (item->stackData != nullptr) {
            item->stackData = nullptr;
        }
        item->CloseAllCacheFds();
    }
    if (inHookSimpNmdFd_ >= 0) {
        close(inHookSimpNmdFd_);
    }
}

bool HookManager::CheckProcess()
{
    if (hookConfig_.pid() > 0) {
        hookConfig_.add_expand_pids(hookConfig_.pid());
    }
    std::set<int32_t> pidCache;
    for (const auto& pid : hookConfig_.expand_pids()) {
        if (pid > 0) {
            struct stat statBuf;
            std::string pidPath = "/proc/" + std::to_string(pid) + "/status";
            if (stat(pidPath.c_str(), &statBuf) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s: hook process does not exist", __func__);
                return false;
            } else {
                auto [iter, isExist] = pidCache.emplace(pid);
                if (isExist) {
                    hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(pid));
                    PROFILER_LOG_INFO(LOG_CORE, "hook context: pid: %d", pid);
                }
                continue;
            }
        }
    }

    if (!hookConfig_.process_name().empty() && !CheckProcessName()) {
        return false;
    }

    if (hookConfig_.response_library_mode()) {
        if (hookCtx_.size() > RESPONSE_MAX_PID_COUNT) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: The maximum allowed is to set %d PIDs.",
                               __func__, RESPONSE_MAX_PID_COUNT);
            return false;
        }
    } else {
        auto maxPidCount = GetValueFromJsonFile(PRODUCT_CONFIG_PATH, "hiprofiler_hook_process_count");
        maxProcessCount_ = maxPidCount > 0 ? maxPidCount : maxProcessCount_;
        if (hookCtx_.size() > static_cast<size_t>(maxProcessCount_)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: The maximum allowed is to set %d PIDs.", __func__, maxProcessCount_);
            return false;
        }
    }

    if (hookCtx_.size() > 1) {
        isProtobufSerialize_ = true;
    }
    return true;
}

bool HookManager::CheckProcessName()
{
    int pidValue = -1;
    const std::string processName = hookConfig_.process_name();
    bool isExist = COMMON::IsProcessExist(processName, pidValue);
    if (hookConfig_.startup_mode() || !isExist) {
        PROFILER_LOG_INFO(LOG_CORE, "Wait process %s start or restart, set param", hookConfig_.process_name().c_str());
        std::string cmd = STARTUP + hookConfig_.process_name();
        int ret = SystemSetParameter(PARAM_NAME.c_str(), cmd.c_str());
        if (ret < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "set param failed, please manually set param and start process(%s)",
                               hookConfig_.process_name().c_str());
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "set param success, please start process(%s)",
                              hookConfig_.process_name().c_str());
            hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(hookConfig_.process_name()));
            hookConfig_.set_startup_mode(true);
        }
    } else if (pidValue != -1) {
        PROFILER_LOG_INFO(LOG_CORE, "Process %s exist, pid = %d", hookConfig_.process_name().c_str(), pidValue);
        for (const auto& item : hookCtx_) {
            if (item->pid == pidValue) {
                return true;
            }
        }
        hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(pidValue));
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The startup mode parameter is not set, name: %s",
                           hookConfig_.process_name().c_str());
        return false;
    }
    return true;
}

void HookManager::SetCommandPoller(const std::shared_ptr<CommandPoller>& p)
{
    commandPoller_ = p;
}

bool HookManager::RegisterAgentPlugin(const std::string& pluginPath)
{
    CHECK_TRUE(!pluginPath.empty(), false, "RegisterPlugin Path is empty");
    RegisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_path(pluginPath);
    request.set_sha256("");
    request.set_name(pluginPath);
    request.set_buffer_size_hint(0);
    RegisterPluginResponse response;

    if (commandPoller_->RegisterPlugin(request, response)) {
        if (response.status() == ResponseStatus::OK) {
            PROFILER_LOG_DEBUG(LOG_CORE, "response.plugin_id() = %d", response.plugin_id());
            agentIndex_ = response.plugin_id();
            PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin OK");
        } else {
            PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 1");
            return false;
        }
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 2");
        return false;
    }

    return true;
}

bool HookManager::UnregisterAgentPlugin(const std::string& pluginPath)
{
    CHECK_TRUE(!pluginPath.empty(), false, "UnregisterPlugin Path is empty");
    UnregisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_plugin_id(agentIndex_);
    UnregisterPluginResponse response;
    if (commandPoller_->UnregisterPlugin(request, response)) {
        CHECK_TRUE(response.status() == ResponseStatus::OK, false, "UnregisterPlugin FAIL 1");
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "UnregisterPlugin FAIL 2");
        return false;
    }
    agentIndex_ = -1;

    return true;
}

bool HookManager::LoadPlugin(const std::string& pluginPath)
{
    return true;
}

bool HookManager::UnloadPlugin(const std::string& pluginPath)
{
    return true;
}

bool HookManager::UnloadPlugin(const uint32_t pluginId)
{
    return true;
}

unsigned long long HookManager::ConvertTagToMask() const
{
    static const std::unordered_map<std::string_view, unsigned long long> tagToMask = {
        {TAG_RES_GPU_VK, RES_GPU_VK},
        {TAG_RES_GPU_GLES_IMAGE, RES_GPU_GLES_IMAGE},
        {TAG_RES_GPU_GLES_BUFFER, RES_GPU_GLES_BUFFER},
        {TAG_RES_GPU_CL_IMAGE, RES_GPU_CL_IMAGE},
        {TAG_RES_GPU_CL_BUFFER, RES_GPU_CL_BUFFER},
        {TAG_RES_FD_OPEN, RES_FD_OPEN},
        {TAG_RES_FD_EPOLL, RES_FD_EPOLL},
        {TAG_RES_FD_EVENTFD, RES_FD_EVENTFD},
        {TAG_RES_FD_SOCKET, RES_FD_SOCKET},
        {TAG_RES_FD_PIPE, RES_FD_PIPE},
        {TAG_RES_FD_DUP, RES_FD_DUP},
        {TAG_RES_FD_ALL, RES_FD_MASK},
        {TAG_RES_THREAD_PTHREAD, RES_THREAD_PTHREAD},
        {TAG_RES_ARKTS_STATIC_HEAP, RES_ARKTS_STATIC_HEAP},
        {TAG_RES_THREAD_ALL, RES_THREAD_MASK},
        {TAG_RES_ARKTS_HEAP_MASK, RES_ARKTS_HEAP_MASK},
        {TAG_RES_JS_HEAP_MASK, RES_JS_HEAP_MASK},
        {TAG_RES_KOTLIN_HEAP_MASK, RES_KOTLIN_HEAP_MASK},
        {TAG_RES_DMABUF_MASK, RES_DMABUF_MASK},
        {TAG_RES_ARK_GLOBAL_HANDLE, RES_ARK_GLOBAL_HANDLE},
        {TAG_RES_SO_MASK, RES_SO_MASK},
        {TAG_RES_ASHMEM_MASK, RES_ASHMEM_MASK},
        {TAG_RES_ARK_LOCAL_HANDLE, RES_ARK_LOCAL_HANDLE},
        {TAG_RES_VMA_ARKWEB, RES_VMA_ARKWEB},
        {TAG_RES_DART_HEAP, RES_DART_HEAP},
        {TAG_RES_COMPOSITE_HEAP, RES_COMPOSITE_HEAP},
        {TAG_RES_RN_HERMES_HEAP, RES_RN_HERMES_HEAP},
        {TAG_RES_CHROMIUM_HEAP, RES_CHROMIUM_HEAP},
        {TAG_RES_VMA_CHROMIUM, RES_VMA_CHROMIUM}};
    unsigned long long resMask = 0;
    for (const auto& tag : hookConfig_.restrace_tag()) {
        auto it = tagToMask.find(tag);
        if (it != tagToMask.end()) {
            resMask |= it->second;
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "invalid tag name : %s", tag.c_str());
        }
    }
    return resMask;
}

uint64_t HookManager::MergeAsyncFlagMaskFromEnum(uint64_t baseMask)
{
    constexpr uint64_t ALL_TYPE = ASYNC_TYPE_ARKTS_EXCLUSIVE_SCOPE | ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL |
        ASYNC_TYPE_FFRT_QUEUE | ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROMISE | ASYNC_TYPE_CUSTOMIZE | ASYNC_TYPE_JSVM |
        ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL | ASYNC_TYPE_ARKTS_STA_LAUNCH | ASYNC_TYPE_ARKWEB;
    uint64_t mask = baseMask;
    for (const auto value : hookConfig_.async_type()) {
        switch (value) {
            case AsyncFlagType::LIBUV_SEND:
                mask |= ASYNC_TYPE_LIBUV_SEND;
                break;
            case AsyncFlagType::FFRT_POOL:
                mask |= ASYNC_TYPE_FFRT_POOL;
                break;
            case AsyncFlagType::FFRT_QUEUE:
                mask |= ASYNC_TYPE_FFRT_QUEUE;
                break;
            case AsyncFlagType::EVENTHANDLER:
                mask |= ASYNC_TYPE_EVENTHANDLER;
                break;
            case AsyncFlagType::PROMISE:
                mask |= ASYNC_TYPE_PROMISE;
                break;
            case AsyncFlagType::CUSTOMIZE:
                mask |= ASYNC_TYPE_CUSTOMIZE;
                break;
            case AsyncFlagType::ARKTS_WORKER:
                mask |= ASYNC_TYPE_ARKTS_WORKER;
                break;
            case AsyncFlagType::ARKTS_TASKPOOL:
                mask |= ASYNC_TYPE_ARKTS_TASKPOOL;
                break;
            case AsyncFlagType::ARKWEB_ENGINE:
                mask |= ASYNC_TYPE_ARKWEB;
                break;
            case AsyncFlagType::JSVM_ASYNC:
                mask |= ASYNC_TYPE_JSVM;
                break;
            case AsyncFlagType::ALL_ASYNC_TYPE:
                mask |= ALL_TYPE;
                break;
            case AsyncFlagType::ALL_ASYNC_NESTED_TYPE:
                mask |= ALL_NESTED_MASK_TYPE;
                break;
            default:
                PROFILER_LOG_WARN(LOG_CORE, "invalid async_type: %d", value);
                break;
        }
    }
    return mask;
}

void HookManager::UpdateAsyncFlagConfig()
{
    if (!hookConfig_.async_stack_enable()) {
        asyncFlagMask_ = 0;
        return;
    }
    uint64_t configuredMask = asyncFlagMask_;
    if (hookConfig_.async_type_size() > 0) {
        configuredMask = MergeAsyncFlagMaskFromEnum(DEFAULT_ASYNC_TYPE);
    }
    asyncFlagMask_ = configuredMask;
    if (asyncFlagMask_ == DEFAULT_ASYNC_TYPE) {
        PROFILER_LOG_WARN(LOG_CORE, "async_stack_enable set but async_type is erro");
        return;
    }
    PROFILER_LOG_WARN(LOG_CORE, "async_flag final mask: 0x%llx", static_cast<unsigned long long>(asyncFlagMask_));
}

void HookManager::UpdateFileCacheModeConfig()
{
    uint32_t fileCacheBufferSize = hookConfig_.file_cache_buffer_size();
    if (fileCacheBufferSize < MIN_FILE_CACHE_MEMORY_SIZE || fileCacheBufferSize > MAX_FILE_CACHE_MEMORY_SIZE) {
        hookConfig_.set_file_cache_buffer_size(MAX_FILE_CACHE_MEMORY_SIZE);
    }
}

static std::string GetArktsDiagPathFromParam()
{
    uint32_t len = 0;
    int ret = SystemGetParameter(ARKTS_DIAG_PATH.c_str(), nullptr, &len);
    if (ret == 0) {
        std::vector<char> value(len + 1);
        ret = SystemGetParameter(ARKTS_DIAG_PATH.c_str(), value.data(), &len);
        if (ret == 0) {
            return std::string(value.data());
        }
    }

    return "";
}

bool HookManager::SetLocalHandle(const char *value)
{
    if (hookConfig_.restrace_tag().size() == 0) {
        return true;
    }

    for (int i = 0; i < hookConfig_.restrace_tag().size(); i++) {
        std::string resTag = hookConfig_.restrace_tag(i);
        if (resTag == TAG_RES_ARK_LOCAL_HANDLE) {
            std::string path = GetArktsDiagPathFromParam();
            if (access(path.c_str(), F_OK) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "arkts diag lib(%s) does not exist", path.c_str());
                return false;
            }

            int ret = SystemSetParameter(PRELOAD_NAME.c_str(), value);
            if (ret < 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "set preload param(%s) failed for process(%s)",
                                   value, hookConfig_.process_name().c_str());
                return false;
            }
        }
    }

    return true;
}

std::string HookManager::GetArktsDiagPath(const int8_t *buf, const uint32_t size)
{
    std::string data(reinterpret_cast<const char *>(buf), size);
    PROFILER_LOG_DEBUG(LOG_CORE, "GetArktsDiagPath, received data:%s", data.c_str());
    size_t pos = data.find(PRELOAD_APP_IDENTIFY);
    if (pos == std::string::npos) {
        PROFILER_LOG_ERROR(LOG_CORE, "received preload msg is invalid:%s", data.c_str());
        return "";
    }

    size_t start = pos + PRELOAD_APP_IDENTIFY.length();
    std::string appName = data.substr(start);
    PROFILER_LOG_INFO(LOG_CORE, "preload appName:%s", appName.c_str());

    if (appName != hookConfig_.process_name()) {
        PROFILER_LOG_ERROR(LOG_CORE, "received appName not equal, appName:%s, process_name:%s",
                           appName.c_str(), hookConfig_.process_name().c_str());
        return "";
    }

    return GetArktsDiagPathFromParam();
}

void HookManager::GetClientConfig(ClientConfig& clientConfig)
{
    int sharedMemCount = hookConfig_.offline_symbolization() ? SHARED_MEMORY_NUM : 1;
    clientConfig.shareMemorySize = (static_cast<uint32_t>(hookConfig_.smb_pages()) / sharedMemCount) * PAGE_BYTES;
#ifndef __aarch64__
    if (static_cast<int>(clientConfig.shareMemorySize) < 0) {
        PROFILER_LOG_WARN(LOG_CORE, "GetClientConfig shareMemorySize is too large.");
        clientConfig.shareMemorySize = MAX_MAP_BUFFER_SIZE / sharedMemCount / PAGE_BYTES * PAGE_BYTES;
    }
#endif
    clientConfig.uniqueStackTableSize = GetUniqueStackTableSizeWithDefault(hookConfig_);
    clientConfig.filterSize = static_cast<int32_t>(hookConfig_.filter_size());
    clientConfig.clockId = COMMON::GetClockId(hookConfig_.clock());
    clientConfig.maxStackDepth = hookConfig_.max_stack_depth();
    clientConfig.arktsConfig.maxJsStackDepth = hookConfig_.max_js_stack_depth();
    clientConfig.mallocDisable = hookConfig_.malloc_disable();
    clientConfig.mmapDisable = hookConfig_.mmap_disable();
    clientConfig.freeStackData = hookConfig_.free_stack_report();
    clientConfig.munmapStackData = hookConfig_.munmap_stack_report();
    clientConfig.fpunwind = hookConfig_.fp_unwind();
    clientConfig.arktsConfig.jsFpunwind = hookConfig_.fp_unwind();
    clientConfig.isBlocked = hookConfig_.blocked();
    clientConfig.memtraceEnable = hookConfig_.memtrace_enable();
    clientConfig.asyncStackEnable = hookConfig_.async_stack_enable();
    clientConfig.asyncFlag = asyncFlagMask_;
    clientConfig.nestedAsyncStackEnable = hookConfig_.nested_async_stack_enable();
    clientConfig.traceMask = saMode_ ? traceMask_ : ConvertTagToMask();
    clientConfig.statisticsInterval = hookConfig_.statistics_interval();
    clientConfig.sampleInterval = hookConfig_.sample_interval();
    clientConfig.offlineSymbolization = hookConfig_.offline_symbolization();
    clientConfig.responseLibraryMode = hookConfig_.response_library_mode();
    clientConfig.arktsConfig.jsStackReport = hookConfig_.js_stack_report();
#ifdef OHOS_ENABLE_SAMPLER
    clientConfig.allocatorSamplingEnable = hookConfig_.allocator_sampling_enable();
#else
    clientConfig.allocatorSamplingEnable = false;
#endif
    printMallocNmd_ = hookConfig_.dump_nmd() ? hookConfig_.dump_nmd() : printMallocNmd_;
    clientConfig.nmdType = (printMallocNmd_ && hookConfig_.dump_nmd()) ? ONLY_NMD_TYPE :
        static_cast<int>(nmdParamInfo_.type);
    const std::string& soName = hookConfig_.target_so_name();
    if (soName.length() < PATH_MAX && !COMMON::ContainsSpecialChars(soName)) {
        clientConfig.targetSoName = hookConfig_.target_so_name();
    }
    for (const auto& filterLib : hookConfig_.filter_lib_list()) {
        if (filterLib.length() < PATH_MAX && !COMMON::ContainsSpecialChars(filterLib)) {
            clientConfig.filterLibList.emplace_back(filterLib);
        }
    }
    clientConfig.printNmd = printMallocNmd_;
    clientConfig.largestSize = largestSize_;
    clientConfig.secondLargestSize = secondLargestSize_;
    clientConfig.maxGrowthSize = maxGrowthSize_;
    clientConfig.gwpAsanHoldTime = gwpAsanHoldTime_;
    clientConfig.gwpAsanMaxEntries = gwpAsanMaxEntries_;
    clientConfig.gwpAsanMaxStackDepth = gwpAsanMaxStackDepth_;
    clientConfig.asyncNestingDepth = hookConfig_.async_nesting_depth();
    clientConfig.asyncTaskStackDepth = hookConfig_.async_task_stack_depth();
    clientConfig.arkuiMemTraceEnable = hookConfig_.arkui_mem_trace_enable();
    clientConfig.discardDestroyedTraces = hookConfig_.discard_destroyed_traces();
    clientConfig.mmapSpecificAddrOnly = hookConfig_.mmap_specific_addr_only();
    GetClientConfigExt(clientConfig);
}

void HookManager::GetClientConfigExt(ClientConfig& clientConfig)
{
    if (hookConfig_.native_orphansnapshots_enable()) {
        if (saMode_) {
            clientConfig.memLeakFormat = memLeakFormat_;
            clientConfig.onlyMemLeak = true;
            clientConfig.memLeakLimit = memLeakLimit_;
        } else {
            clientConfig.memLeakFormat = MemLeakFormat::TYPE_SNAPSHOT;
            clientConfig.onlyMemLeak = false;
            clientConfig.memLeakLimit = MEM_LEAK_LIMIT_DEFAULT;
        }
    }
    // File cache mode configuration
    clientConfig.enableFileCacheMode = hookConfig_.use_file_cache_mode();
    clientConfig.fileCacheBufferSize = hookConfig_.file_cache_buffer_size();
    // -1 is save '\0'
    int ret = memcpy_s(clientConfig.arktsConfig.filterNapiName, sizeof(clientConfig.arktsConfig.filterNapiName) - 1,
                       hookConfig_.filter_napi_name().c_str(), hookConfig_.filter_napi_name().size());
    if (ret != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s filter_napi_name fail");
    }
}

bool HookManager::HandleFileCacheHookContext(const std::shared_ptr<HookManagerCtx>& ctx)
{
    // Initialize file cache mode
    ctx->readyFileNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK | EventNotifier::SEMAPHORE);
    CHECK_NOTNULL(ctx->readyFileNotifier, false, "create readyFileNotifier failed!");
    ctx->readyFilePoller = std::make_shared<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);

    ctx->readyFilePoller->Init();
    ctx->readyFilePoller->Start();
    ctx->cacheFileStopReceived = false;
    ctx->cacheFileStopRequested = false;

    shouldSkipGlobalFreed_ = hookConfig_.discard_destroyed_traces();
    if (shouldSkipGlobalFreed_) {
        totalRecord_ = 0;
        skipRecord_ = 0;
        validLiveData_.clear();
        validLiveData_.reserve(VALID_LIVE_DATA_RESERVE_COUNT);
    }

    return true;
}

bool HookManager::HandleSharedMemoryHookContext(const std::shared_ptr<HookManagerCtx>& ctx, int sharedMemCount)
{
    // shared memory mode
    uint32_t bufferSize = (static_cast<uint32_t>(hookConfig_.smb_pages()) / sharedMemCount) * PAGE_BYTES;
#ifndef __aarch64__
    if (static_cast<int>(bufferSize) < 0) {
        PROFILER_LOG_WARN(LOG_CORE, "HandleHookContext bufferSize is too large.");
        bufferSize = MAX_MAP_BUFFER_SIZE / sharedMemCount / PAGE_BYTES * PAGE_BYTES;
    }
#endif
    for (int i = 0; i < sharedMemCount; ++i) {
        std::string smbName = "";
        if (ctx->pid > 0) {
            smbName = "hooknativesmb_" + std::to_string(ctx->pid) + ":" + std::to_string(i);
        } else if (!ctx->processName.empty()) {
            smbName = "hooknativesmb_" + ctx->processName + ":" + std::to_string(i);
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext context error, pid: %d, process name: %s",
                ctx->pid, ctx->processName.c_str());
            return false;
        }
        ctx->smbNames.push_back(smbName);
        auto shareMemoryBlock = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(smbName, bufferSize);
        CHECK_TRUE(shareMemoryBlock != nullptr, false, "CreateMemoryBlockLocal FAIL %s", smbName.c_str());
        ctx->shareMemoryBlockList.push_back(shareMemoryBlock);
        auto eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
        CHECK_NOTNULL(eventNotifier, false, "create EventNotifier for %s failed!", smbName.c_str());
        ctx->eventNotifierList.push_back(eventNotifier);
        PROFILER_LOG_INFO(LOG_CORE, "hookservice smbFd = %d, eventFd = %d\n", shareMemoryBlock->GetfileDescriptor(),
            eventNotifier->GetFd());
        // start event poller task
        auto eventPoller = std::make_shared<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
        eventPoller->Init();
        eventPoller->Start();
        ctx->eventPollerList.push_back(eventPoller);
    }
    return true;
}

std::shared_ptr<HookManager::HookManagerCtx> HookManager::FindHookContext(int32_t pid, const std::string& name)
{
    for (const auto& item : hookCtx_) {
        if (item != nullptr && (item->pid == pid || item->processName == name)) {
            return item;
        }
    }
    return nullptr;
}

void HookManager::HandleReadyCacheFile(const std::shared_ptr<HookManagerCtx>& ctx)
{
    if (ctx == nullptr || ctx->GetFdQueueSize() == 0) {
        return;
    }
    int frontFd = ctx->GetFdQueueFront();
    ctx->cacheFileParsing = true;
    if (shouldSkipGlobalFreed_) {
        validLiveData_.clear();
        orphanFreeWhitelist_.clear();
        RefreshValidLiveDataFromFile(ctx, frontFd);
    }
    if (noDataQueue_) {
        ctx->stackPreprocess->SetValidLiveData(shouldSkipGlobalFreed_ ? &validLiveData_ : nullptr,
            shouldSkipGlobalFreed_ ? &orphanFreeWhitelist_ : nullptr);
        ctx->stackPreprocess->TakeResultsFromCacheFile(frontFd);
    } else {
        ReadFromCacheFile(ctx, frontFd);
    }
    ctx->PopFrontFdQueue();
    if (!hookConfig_.offline_symbolization()) {
        // only for online_symbolization
        HandleBackpressure(ctx);
    }
    ctx->cacheFileParsing = false;
    PROFILER_LOG_INFO(LOG_CORE, "HandleReadyCacheFile pop fd, queue size%zu", ctx->GetFdQueueSize());
    FinishFileCacheStopIfDrained(ctx);
}

void HookManager::WaitFinalSnapshotIfNeeded(const std::shared_ptr<HookManagerCtx>& item)
{
    if (hookConfig_.allocator_sampling_enable() && item->stackPreprocess != nullptr) {
        item->stackPreprocess->WaitForFinalSnapshot();
    }
}

void HookManager::StopSignalProcess(pid_t pid)
{
    if (kill(pid, SIGNAL_STOP_HOOK) != -1) {
        return;
    }
    const int bufSize = 256;
    char buf[bufSize] = {0};
    (void)strerror_r(errno, buf, bufSize);
}

bool HookManager::SendStopCommandIfConnected(pid_t pid)
{
    if (hookService_ == nullptr || !hookService_->HasSocketFd(pid)) {
        PROFILER_LOG_INFO(LOG_CORE, "SendStopCommandIfConnected: no control socket for pid %d", pid);
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "SendStopCommandIfConnected: send STOP_HOOK_CMD to pid %d", pid);
    hookService_->SendCommand(pid, STOP_HOOK_CMD);
    return true;
}

void HookManager::PrepareImmediateStop(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr || item->stackPreprocess == nullptr) {
        PROFILER_LOG_WARN(LOG_CORE, "PrepareImmediateStop: hook context invalid");
        return;
    }
    if (hookConfig_.use_file_cache_mode()) {
        PROFILER_LOG_INFO(LOG_CORE, "PrepareImmediateStop: file cache mode stop pid %d without active client",
            item->pid);
        (void)FinalizeCacheFileStop(item);
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "PrepareImmediateStop: mark data source terminated for pid %d", item->pid);
    FinalizeSharedMemoryTermination(item);
}

void HookManager::FinalizeSharedMemoryTermination(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr || item->stackPreprocess == nullptr || item->stackPreprocess->IsDataSourceTerminated()) {
        return;
    }
    StopSharedMemoryPollers(item);
    uint32_t quietRounds = 0;
    uint32_t waitedMs = 0;
    while (waitedMs < SHMEM_DRAIN_TIMEOUT_MS && quietRounds < SHMEM_DRAIN_QUIET_ROUNDS) {
        if (DrainPendingSharedMemory(item)) {
            quietRounds = 0;
        } else {
            quietRounds++;
            if (quietRounds >= SHMEM_DRAIN_QUIET_ROUNDS) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SHMEM_DRAIN_POLL_MS));
        waitedMs += SHMEM_DRAIN_POLL_MS;
    }
    item->stackPreprocess->SetDataSourceTerminated();
    if (item->stackData != nullptr) {
        item->stackData->SetProducerFinished();
    }
}

bool HookManager::DrainPendingSharedMemory(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr || item->stackPreprocess == nullptr) {
        return false;
    }
    bool drained = false;
    for (size_t i = 0; i < item->shareMemoryBlockList.size(); ++i) {
        if (item->shareMemoryBlockList[i] == nullptr) {
            continue;
        }
        if (noDataQueue_) {
            drained = item->stackPreprocess->DrainPendingShmemData(item->shareMemoryBlockList[i]) || drained;
        } else {
            drained = DrainSharedMemoryBlock(item, static_cast<int>(i), false) || drained;
        }
    }
    return drained;
}

void HookManager::StopSharedMemoryPollers(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr) {
        return;
    }
    for (const auto& poller : item->eventPollerList) {
        if (poller != nullptr) {
            (void)poller->Stop();
        }
    }
}

void HookManager::StopFileCacheProcess(const std::shared_ptr<HookManagerCtx>& item)
{
    pid_t pid = static_cast<pid_t>(item->pid);
    PROFILER_LOG_INFO(LOG_CORE, "StopFileCacheProcess: request stop for pid %d", pid);
    RequestCacheFileStop(pid);
    if (!SendStopCommandIfConnected(pid)) {
        PROFILER_LOG_INFO(LOG_CORE, "StopFileCacheProcess: client unavailable, finalize current cache files pid %d",
            pid);
        (void)FinalizeCacheFileStop(item);
    }
}

void HookManager::FinishFileCacheStopIfDrained(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr || item->stackPreprocess == nullptr) {
        return;
    }
    bool cacheInputEnded = item->cacheFileStopReceived || item->stackPreprocess->IsDataSourceTerminated();
    if (!cacheInputEnded || item->GetFdQueueSize() > 0 || item->cacheFileParsing) {
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "FinishFileCacheStopIfDrained: pid %d cache queue drained", item->pid);
    item->stackPreprocess->SetIsCacheFileParseDone(true);
    if (noDataQueue_ && (item->cacheFileClientDisconnected || item->stackPreprocess->IsDataSourceTerminated())) {
        PROFILER_LOG_INFO(LOG_CORE,
            "FinishFileCacheStopIfDrained: pid %d input ended, stop after current cache files", item->pid);
        item->stackPreprocess->SetIsStopTakeData(true);
    }
}

bool HookManager::FinalizeCacheFileStop(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item == nullptr || item->stackPreprocess == nullptr) {
        PROFILER_LOG_WARN(LOG_CORE, "FinalizeCacheFileStop: hook context invalid");
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "FinalizeCacheFileStop: pid %d, queue size %zu, clientDisconnected %d",
        item->pid, item->GetFdQueueSize(), item->cacheFileClientDisconnected.load());
    item->cacheFileStopReceived = true;
    FinishFileCacheStopIfDrained(item);
    return true;
}

void HookManager::HandleInvalidPidStop(const std::shared_ptr<HookManagerCtx>& item)
{
    PROFILER_LOG_INFO(LOG_CORE, "StopPluginSession: pid(%d) is less or equal zero.", item->pid);
    PrepareImmediateStop(item);
}

void HookManager::StopTargetProcess(const std::shared_ptr<HookManagerCtx>& item)
{
    if (item->pid <= 0) {
        HandleInvalidPidStop(item);
        return;
    }
    pid_t pid = static_cast<pid_t>(item->pid);
    PROFILER_LOG_INFO(LOG_CORE, "StopTargetProcess: begin stop pid %d, fileCacheMode %d", pid,
        hookConfig_.use_file_cache_mode());
    StopSignalProcess(pid);
    UpdateFreezeExcludedPid(false, static_cast<int32_t>(item->pid), getpid());
    if (hookConfig_.use_file_cache_mode()) {
        StopFileCacheProcess(item);
        return;
    }
    if (!SendStopCommandIfConnected(pid)) {
        PrepareImmediateStop(item);
    }
    PROFILER_LOG_INFO(LOG_CORE, "StopTargetProcess: finish stop dispatch pid %d", pid);
}

bool HookManager::FinalizeStoppedContext(const std::shared_ptr<HookManagerCtx>& item)
{
    CHECK_TRUE(item->stackPreprocess != nullptr, false, "stop StackPreprocess FAIL");
    if (!hookConfig_.use_file_cache_mode() && !item->stackPreprocess->IsDataSourceTerminated()) {
        FinalizeSharedMemoryTermination(item);
    }
    item->stackPreprocess->StopTakeResults();
    PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults success");
    if (hookConfig_.statistics_interval() > 0 && !hookConfig_.allocator_sampling_enable()) {
        item->stackPreprocess->UpdateStatisticsTime();
        item->stackPreprocess->FlushRecordStatistics();
    }
    if (hookConfig_.malloc_free_matching_interval() > 0) {
        item->stackPreprocess->FlushRecordApplyAndReleaseMatchData();
    }
    if (item->stackData != nullptr) {
        item->stackData->Close();
    }
    return true;
}

void HookManager::CleanupHookServiceContext(pid_t pid)
{
    if (pid > 0 && hookService_ != nullptr) {
        if (hookService_->HasSocketFd(pid)) {
            hookService_->CloseSocketFd(pid);
        }
        hookService_->RemovePidInfo(pid);
    }
}

void HookManager::CreateNoQueueDataHandler(const std::shared_ptr<HookManagerCtx>& ctx, int sharedMemCount)
{
    clockid_t pluginDataClockId = COMMON::GetClockId(hookConfig_.clock());
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookConfig_, pluginDataClockId,
        fpHookData_, isHookStandalone_, isSaService_, isProtobufSerialize_);
    ctx->stackPreprocess->SetFlushSize(shareMemorySize_);
    ctx->stackPreprocess->SetNmdFd(nmdParamInfo_.fd);

    ClientConfig clientConfig;
    GetClientConfig(clientConfig);
    if (!clientConfig.enableFileCacheMode) {
        for (int i = 0; i < sharedMemCount; ++i) {
            ctx->eventPollerList[i]->AddFileDescriptor(
                ctx->eventNotifierList[i]->GetFd(),
                std::bind(&StackPreprocess::TakeResultsFromShmem, ctx->stackPreprocess,
                ctx->eventNotifierList[i], ctx->shareMemoryBlockList[i]));
        }
    } else {
        ctx->readyFilePoller->AddFileDescriptor(
            ctx->readyFileNotifier->GetFd(),
            [this, &ctx] {
                ctx->readyFileNotifier->Take();
                this->HandleReadyCacheFile(ctx);
            });
        PROFILER_LOG_INFO(LOG_CORE, "NoQueueDataHandler add readyFilePoller");
    }
}

bool HookManager::CreateQueueDataHandler(const std::shared_ptr<HookManagerCtx>& ctx, int sharedMemCount)
{
    clockid_t pluginDataClockId = COMMON::GetClockId(hookConfig_.clock());
    int stackSize = (hookConfig_.fp_unwind() ? STACK_DATA_SIZE : STACK_DATA_SIZE_DWARF);
    ctx->stackData = std::make_shared<StackDataRepeater>(static_cast<size_t>(stackSize));
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(ctx->stackData, hookConfig_, pluginDataClockId,
        fpHookData_, isHookStandalone_, isSaService_, isProtobufSerialize_);
    ctx->stackPreprocess->SetFlushSize(shareMemorySize_);

    ClientConfig clientConfig;
    GetClientConfig(clientConfig);
    if (!clientConfig.enableFileCacheMode) {
        for (int i = 0; i < sharedMemCount; ++i) {
            ctx->eventPollerList[i]->AddFileDescriptor(
                ctx->eventNotifierList[i]->GetFd(),
                [this, &ctx, i] { this->ReadShareMemory(ctx, i); });
        }
    } else {
        ctx->readyFilePoller->AddFileDescriptor(
            ctx->readyFileNotifier->GetFd(),
            [this, &ctx] {
                ctx->readyFileNotifier->Take();
                this->HandleReadyCacheFile(ctx);
            });
        PROFILER_LOG_INFO(LOG_CORE, "QueueDataHandler add readyFilePoller");
    }
    return true;
}

bool HookManager::HandleHookContext(const std::shared_ptr<HookManagerCtx>& ctx)
{
    if (ctx == nullptr) {
        return false;
    }
    int sharedMemCount = hookConfig_.offline_symbolization() ? SHARED_MEMORY_NUM : 1;
    ClientConfig clientConfig;
    GetClientConfig(clientConfig);
    if (clientConfig.enableFileCacheMode) {
        if (!HandleFileCacheHookContext(ctx)) {
            return false;
        }
        sharedMemCount = 0;
    } else {
        if (!HandleSharedMemoryHookContext(ctx, sharedMemCount)) {
            return false;
        }
    }

    if (hookConfig_.async_stack_enable()) {
        const uint32_t uniqueStackTableSize = GetUniqueStackTableSizeWithDefault(hookConfig_);
        uniqueTableBlock_ = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("unique_stack_table",
            uniqueStackTableSize);
        if (uniqueTableBlock_ == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext uniqueTableBlock_ nullptr");
            return false;
        }
        void* memoryBase = uniqueTableBlock_->GetMMapBase();
        if (memoryBase == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext memoryBase nullptr");
            return false;
        }
        auto table = OHOS::HiviewDFX::UniqueStackTable::Instance();
        if (table == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext new UniqueStackTable failed.");
            return false;
        }
        // Use faultloggerd to record sync/async stack info.
        table->SwitchExternalBuffer(memoryBase, uniqueStackTableSize);
    }

    ctx->isRecordAccurately = hookConfig_.record_accurately();
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig filter size = %d, malloc disable = %d mmap disable = %d",
        hookConfig_.filter_size(), hookConfig_.malloc_disable(), hookConfig_.mmap_disable());
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig fp unwind = %d, max stack depth = %d, record_accurately=%d",
        hookConfig_.fp_unwind(), hookConfig_.max_stack_depth(), ctx->isRecordAccurately);
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig  offline_symbolization = %d", hookConfig_.offline_symbolization());
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig  js_stack_report = %d max_js_stack_depth = %u",
        hookConfig_.js_stack_report(), hookConfig_.max_js_stack_depth());
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig  native_orphansnapshots_enable = %d",
        hookConfig_.native_orphansnapshots_enable());

    if (noDataQueue_) {
        CreateNoQueueDataHandler(ctx, sharedMemCount);
    } else {
        if (!CreateQueueDataHandler(ctx, sharedMemCount)) {
            return false;
        }
    }
    if (isProtobufSerialize_) {
        ctx->stackPreprocess->SetWriter(g_buffWriter);
    } else if (isSaService_) {
        ctx->stackPreprocess->SetWriterForFdSa(saFd_);
    } else {
        ctx->stackPreprocess->SetWriter(const_cast<WriterStructPtr>(writerAdapter_->GetStruct()));
    }
    ctx->stackPreprocess->SetFactory(factory_);
    if (hookConfig_.allocator_sampling_enable() && hookConfig_.statistics_interval() > 0) {
        std::weak_ptr<HookManager> weakSelf = shared_from_this();
        std::weak_ptr<HookManagerCtx> weakCtx = ctx;
        ctx->stackPreprocess->SetServerCommandCallback([weakSelf, weakCtx](uint32_t cmd) {
            auto self = weakSelf.lock();
            auto hookCtx = weakCtx.lock();
            if (self && self->hookService_ && hookCtx && hookCtx->pid > 0) {
                self->hookService_->SendCommand(hookCtx->pid, cmd);
            }
        });
    }
    return true;
}

void HookManager::CheckHapEncryped()
{
    if (!COMMON::IsUserMode()) {
        PROFILER_LOG_INFO(LOG_CORE, "debug mode, skip encrypted app js stack check.");
        return;
    }
    if (saMode_ && hookConfig_.fp_unwind()) {
        PROFILER_LOG_DEBUG(LOG_CORE, "sa Mode, the encrypted app is allowed to use fp unwind to catch js stack.");
        return;
    }
    for (const auto& pid : hookConfig_.expand_pids()) {
        if (pid > 0 && COMMON::CheckApplicationEncryped(pid, "")) {
            hookConfig_.set_js_stack_report(0);
            hookConfig_.set_max_js_stack_depth(0);
            break;
        }
    }
    const std::string processName = hookConfig_.process_name();
    if (!processName.empty() && COMMON::CheckApplicationEncryped(0, processName)) {
        PROFILER_LOG_INFO(LOG_CORE, "Encryped Application don't unwind js stack:%s", processName.c_str());
        hookConfig_.set_js_stack_report(0);
        hookConfig_.set_max_js_stack_depth(0);
    }
}

void HookManager::UpdateFreezeExcludedPid(bool isAdd, int32_t targetPid, int32_t profilerPid)
{
    if (!saMode_) {
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get SystemAbilityManager failed");
            return;
        }
        sptr<AppExecFwk::IAppMgr> appManager =
            iface_cast<AppExecFwk::IAppMgr>(sam->GetSystemAbility(APP_MGR_SERVICE_ID));
        if (appManager == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get APP_MGR_SERVICE failed");
            return;
        }
        appManager->UpdateFreezeExcludedPid(isAdd, targetPid, profilerPid);
    }
}

int HookManager::CreateMemLeakFile()
{
    if (!hookConfig_.native_orphansnapshots_enable()) {
        return -1;
    }
    constexpr int FILE_MODE = 0644;
    std::string tempPath = fileCacheDir_ + memLeakFileName_;
    int fileFd = open(tempPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    if (fileFd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Create memLeak file failed!");
        return fileFd;
    }
    tempMemLeakFd_ = fileFd;
    memLeakDumpDone_ = false;
    PROFILER_LOG_INFO(LOG_CORE, "Create tempMemLeak file success. fd: %d, type: %d, filename: %s",
        tempMemLeakFd_, memLeakFormat_, memLeakFileName_.c_str());
    return fileFd;
}

std::string HookManager::GetMemLeakFileName(MemLeakFormat format)
{
    int pidValue = hookConfig_.pid();
    std::string processName = hookConfig_.process_name();
    COMMON::IsProcessExist(processName, pidValue);
    // startmode, pid is unknown, set default value 0
    pidValue = hookConfig_.startup_mode() ? 0 : pidValue;
    return MEM_LEAK_FILE_PERFIX + std::to_string(pidValue) + MEM_LEAK_FILE_SUFFIX;
}

bool HookManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "CreatePluginSession");
    // save config
    if (!config.empty()) {
        std::string cfgData = config[0].config_data();
        if (hookConfig_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: ParseFromArray failed", __func__);
            return false;
        }
    }
    if (hookConfig_.startup_mode()) {
        nlohmann::json payload;
        payload["bundleName"] = hookConfig_.process_name();
        nlohmann::json reply;
        int32_t ret = OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(
            OHOS::ResourceSchedule::ResType::SYNC_RES_TYPE_DFX_CTRL_IMAGE_INTERFACE, 0, payload, reply);
        if (ret != 0) {
            PROFILER_LOG_WARN(LOG_CORE, "ReportSyncEvent clear image cache failed, ret=%d", ret);
        }
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    int32_t uShortMax = (std::numeric_limits<unsigned short>::max)();
    if (hookConfig_.filter_size() > uShortMax) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: filter size invalid(size exceed 65535), reset to 65535!", __func__);
        hookConfig_.set_filter_size(uShortMax);
    }
    if (!CheckProcess()) { // Check and initialize the context for the target process.
        return false;
    }
    (void)CheckHapEncryped();
    if (hookConfig_.max_stack_depth() < DLOPEN_MIN_UNWIND_DEPTH) {
        // set default max depth
        hookConfig_.set_max_stack_depth(DLOPEN_MIN_UNWIND_DEPTH);
    }
#if defined(__arm__)
    hookConfig_.set_fp_unwind(false); // if OS is 32-bit,set fp_unwind false.
    hookConfig_.set_response_library_mode(false);
#endif
    if (hookConfig_.response_library_mode()) {
        hookConfig_.set_fp_unwind(true);
        hookConfig_.set_offline_symbolization(true);
        hookConfig_.set_js_stack_report(0);
    }
    // offlinem symbolization, callframe must be compressed
    if (hookConfig_.offline_symbolization()) {
        hookConfig_.set_callframe_compress(true);
    }

    // statistical reporting must be callframe compressed and accurate.
    if (hookConfig_.statistics_interval() > 0) {
        hookConfig_.set_callframe_compress(true);
        hookConfig_.set_record_accurately(true);
    }

    // malloc and free matching interval reporting must be callframe compressed and accurate.
    if (hookConfig_.malloc_free_matching_interval() > 0) {
        hookConfig_.set_callframe_compress(true);
        hookConfig_.set_record_accurately(true);
        hookConfig_.set_statistics_interval(0);
    }

    // callframe compressed, string must be compressed.
    if (hookConfig_.callframe_compress()) {
        hookConfig_.set_string_compressed(true);
    }

    if (hookConfig_.js_stack_report() > 0 && hookConfig_.max_js_stack_depth() == 0 && hookConfig_.fp_unwind()) {
        hookConfig_.set_max_js_stack_depth(DEFAULT_MAX_JS_STACK_DEPTH);
    }
    if (hookConfig_.native_orphansnapshots_enable()) {
        CleanOldSnapshotFiles();
    }

    UpdateAsyncFlagConfig();
    UpdateFileCacheModeConfig();
    if (hookConfig_.mmap_specific_addr_only()) {
        hookConfig_.set_statistics_interval(0);
        hookConfig_.clear_restrace_tag();
        hookConfig_.set_memtrace_enable(false);
        hookConfig_.set_malloc_disable(true);
    }

    if (!SetLocalHandle("1")) {
        return false;
    }
    
    if (hookCtx_.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookManager no task");
        return false;
    }
    if ((hookConfig_.save_file() || isHookStandalone_) && !hookConfig_.file_name().empty()) {
        auto retFile = COMMON::CheckNotExistsFilePath(hookConfig_.file_name());
        if (!retFile.first) {
            PROFILER_LOG_INFO(LOG_CORE, "check file path %s fail", hookConfig_.file_name().c_str());
            return false;
        }
        fpHookData_ = fopen(retFile.second.c_str(), "wb+");
        if (fpHookData_ == nullptr) {
            PROFILER_LOG_INFO(LOG_CORE, "fopen file %s fail", hookConfig_.file_name().c_str());
            return false;
        }
    }
    if (hookConfig_.fp_unwind() && hookConfig_.offline_symbolization()
        && hookConfig_.statistics_interval() > 0) {
        noDataQueue_ = true;
    }
    if (hookConfig_.allocator_sampling_enable()) {
        if (hookConfig_.statistics_interval() == 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "allocator_sampling_enable requires statistics_interval > 0");
            return false;
        }
    }

    if (!isSaService_) {
        CreateWriter();
    }
    factory_ = std::make_shared<HookRecordFactory>(hookConfig_);
    for (const auto& item : hookCtx_) {
        CHECK_TRUE(HandleHookContext(item), false, "handle hook context failed"); // Create the required resources.
    }

    if (!isSaService_) { // SA mode will start HookService in the service.
        ClientConfig clientConfig;
        GetClientConfig(clientConfig);
        if (noDataQueue_) {
            clientConfig.freeEventOnlyAddrEnable = true;
        }
        std::string clientConfigStr = clientConfig.ToString();
        PROFILER_LOG_INFO(LOG_CORE, "send hook client config:%s\n", clientConfigStr.c_str());
        hookService_ = std::make_shared<HookService>(clientConfig, shared_from_this(), (hookCtx_.size() > 1));
    }

    return true;
}
// Clean up old orphan snapshot files from previous sessions
void HookManager::CleanOldSnapshotFiles()
{
    DIR* dir = opendir(MEM_LEAK_FILE_PATH.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.find(MEM_LEAK_FILE_PERFIX) == 0) {
                std::string fullPath = MEM_LEAK_FILE_PATH + filename;
                std::remove(fullPath.c_str());
                PROFILER_LOG_INFO(LOG_CORE, "Removed old orphan snapshot file: %s", filename.c_str());
            }
        }
        closedir(dir);
    }
    memLeakFileName_ = GetMemLeakFileName(memLeakFormat_);
    std::string finalFileName = MEM_LEAK_FILE_PATH + memLeakFileName_;
    std::remove(finalFileName.c_str());
}

void HookManager::HookManagerCtx::FlushStackArray()
{
    if (g_rawDataArray.size() > 0 && stackData != nullptr) {
        if (!stackData->PutRawStackArray(g_rawDataArray, g_rawStackCount)) {
            PROFILER_LOG_INFO(LOG_CORE, "PutRawStackArray error");
        }
        g_rawStackCount = 0;
        g_rawDataArray = {};
    }
}

int HookManager::HookManagerCtx::GetFdQueueFront()
{
    std::lock_guard<std::mutex> guard(fdQueueMtx);
    if (cachedFileFdQueue.empty()) {
        return -1;
    }
    return cachedFileFdQueue.front();
}

size_t HookManager::HookManagerCtx::GetFdQueueSize()
{
    std::lock_guard<std::mutex> guard(fdQueueMtx);
    return cachedFileFdQueue.size();
}

void HookManager::HookManagerCtx::PushBackFdQueue(int fd)
{
    std::lock_guard<std::mutex> guard(fdQueueMtx);
    cachedFileFdQueue.push_back(fd);
}

void HookManager::HookManagerCtx::PopFrontFdQueue()
{
    std::lock_guard<std::mutex> guard(fdQueueMtx);
    if (!cachedFileFdQueue.empty()) {
        cachedFileFdQueue.pop_front();
    }
}

void HookManager::HookManagerCtx::SetCurrentActiveCacheFd(uint16_t fileIndex, int fd)
{
    std::lock_guard<std::mutex> lock(activeCacheFdMtx);
    if (currentActiveCacheFd >= 0) {
        close(currentActiveCacheFd);
    }
    currentActiveCacheFd = fd;
    currentActiveFileIndex = fileIndex;
}

void HookManager::HookManagerCtx::AddPendingCacheFd(uint16_t fileIndex, int fd)
{
    std::lock_guard<std::mutex> lock(activeCacheFdMtx);
    auto it = pendingCacheFds.find(fileIndex);
    if (it != pendingCacheFds.end() && it->second >= 0) {
        close(it->second);
    }
    pendingCacheFds[fileIndex] = fd;
}

void HookManager::HookManagerCtx::RotateActiveCacheFd(uint16_t returnedFileIndex, int newFd)
{
    std::lock_guard<std::mutex> lock(activeCacheFdMtx);
    if (currentActiveCacheFd >= 0) {
        close(currentActiveCacheFd);
        currentActiveCacheFd = -1;
    }
    uint16_t nextActiveIndex = (returnedFileIndex + 1) % FILE_CACHE_FD_COUNT;
    auto it = pendingCacheFds.find(nextActiveIndex);
    if (it != pendingCacheFds.end() && it->second >= 0) {
        currentActiveCacheFd = it->second;
        currentActiveFileIndex = nextActiveIndex;
        pendingCacheFds.erase(it);
    }
    
    if (newFd >= 0) {
        auto pendingIt = pendingCacheFds.find(returnedFileIndex);
        if (pendingIt != pendingCacheFds.end() && pendingIt->second >= 0) {
            close(pendingIt->second);
        }
        pendingCacheFds[returnedFileIndex] = newFd;
    }
}

void HookManager::HookManagerCtx::CloseAllCacheFds()
{
    std::lock_guard<std::mutex> lock(activeCacheFdMtx);
    if (currentActiveCacheFd >= 0) {
        close(currentActiveCacheFd);
        currentActiveCacheFd = -1;
    }
    for (auto& [index, fd] : pendingCacheFds) {
        if (fd >= 0) {
            close(fd);
        }
    }
    pendingCacheFds.clear();
}

void HookManager::HookManagerCtx::ProcessPendingCacheFdOnDisconnect()
{
    std::lock_guard<std::mutex> lock(activeCacheFdMtx);
    if (currentActiveCacheFd >= 0) {
        PROFILER_LOG_INFO(LOG_CORE,
            "ProcessPendingCacheFdOnDisconnect: processing active cache fd %d (index %u)",
            currentActiveCacheFd, currentActiveFileIndex);
        struct stat st;
        if (fstat(currentActiveCacheFd, &st) == 0 && st.st_size > 0) {
            PushBackFdQueue(currentActiveCacheFd);
            PROFILER_LOG_INFO(LOG_CORE,
                "ProcessPendingCacheFdOnDisconnect: add incomplete cache fd to queue (size %" PRId64 ")", st.st_size);
            currentActiveCacheFd = -1;
        } else {
            close(currentActiveCacheFd);
            currentActiveCacheFd = -1;
            PROFILER_LOG_INFO(LOG_CORE,
                "ProcessPendingCacheFdOnDisconnect: skipped empty cache fd, closed directly");
        }
    }
    if (readyFileNotifier != nullptr && GetFdQueueSize() > 0) {
        readyFileNotifier->Post(1);
    }

    for (auto& [index, fd] : pendingCacheFds) {
        if (fd >= 0) {
            PROFILER_LOG_INFO(LOG_CORE,
                "ProcessPendingCacheFdOnDisconnect: closed pending cache fd %d (index %u)", fd, index);
            close(fd);
            fd = -1;
        }
    }
    pendingCacheFds.clear();
}

BackpressureStateMsg HookManager::HookManagerCtx::CalculateBackpressureState()
{
    BackpressureStateMsg state;
    state.pid = pid;
    state.queueDepth = queueDepth_.load();
    state.recommendedDelayNs = recommendedDelayNs_.load();
    return state;
}

void HookManager::FlushRawStackArray(const std::shared_ptr<HookManagerCtx>& hookCtx,
                                     std::shared_ptr<HookRecord>& hookRecord)
{
    if (hookCtx == nullptr || hookRecord == nullptr) {
        return;
    }
    g_rawDataArray[g_rawStackCount] = hookRecord;
    ++g_rawStackCount;
    if (g_rawStackCount == CACHE_ARRAY_SIZE) {
        hookCtx->FlushStackArray();
    }
}

void HookManager::DumpNmdInfo(const char* nmdResult)
{
    std::string fileName = hookConfig_.process_name().empty() ? std::to_string(hookConfig_.pid()) :
                                                                hookConfig_.process_name();
    const std::string filePath = "/data/local/tmp/nmd_" + fileName + ".txt";
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "unable to write into nmd.txt");
        return;
    }
    outFile << nmdResult;
    if (!outFile) {
        PROFILER_LOG_ERROR(LOG_CORE, "Error: Failed to write data to file %s", filePath.c_str());
        outFile.close();
        return;
    }
    outFile.close();
    PROFILER_LOG_INFO(LOG_CORE, "Data successfully written to nmd file");
}

void HookManager::DumpMemLeakInfo()
{
    PROFILER_LOG_INFO(LOG_CORE, "DumpMemLeakInfo start");
    if (!hookConfig_.native_orphansnapshots_enable()) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpMemLeakInfo disabled");
        memLeakDumpDone_ = true;
        return;
    }
    if (tempMemLeakFd_ < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpMemLeakInfo tmpMemLeakFd_ err");
        memLeakDumpDone_ = true;
        return;
    }
    if (saMode_ && memLeakFd_ >= 0) {
        if (!CopyFdContent(static_cast<int>(memLeakFd_), tempMemLeakFd_)) {
            PROFILER_LOG_ERROR(LOG_CORE, "DumpMemLeakInfo CopyFdContent failed");
        }
    } else {
        std::string finalPath = MEM_LEAK_FILE_PATH + memLeakFileName_;
        if (!COMMON::CopyFileFast(tempMemLeakFd_, finalPath)) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleMemLeakDone CopyFileFast failed");
        }
    }
    CloseMemLeakTempFile();
    memLeakDumpDone_ = true;
}

bool HookManager::HandleMemLeakDone()
{
    if (memLeakDumpDone_.load()) {
        return true;
    }
    DumpMemLeakInfo();
    return memLeakDumpDone_.load();
}

int HookManager::CreateInHookSimpNmdFd()
{
    std::vector<char> tempPathBuf(SIMPLIFIED_NMD_TEMP_FILE.begin(), SIMPLIFIED_NMD_TEMP_FILE.end());
    tempPathBuf.push_back('\0');
    int fd = mkstemp(tempPathBuf.data());
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateInHookSimpNmdFd mkstemp failed errno=%d", errno);
        return -1;
    }
    if (unlink(tempPathBuf.data()) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateInHookSimpNmdFd unlink failed errno=%d", errno);
        close(fd);
        return -1;
    }
    return fd;
}

bool HookManager::StartInHookSimpNmdDump(std::function<bool(int recvFd)> postProcessor)
{
    CHECK_TRUE(postProcessor, false, "postProcessor is invalid");
    bool expect = false;
    CHECK_TRUE(inHookSimpNmdStarted_.compare_exchange_strong(expect, true), false, "already in dumping");
    inHookSimpNmdSem_ = GetSemaphoreFactory().Create(0);
    if (inHookSimpNmdSem_ == nullptr) {
        inHookSimpNmdStarted_.store(false);
        PROFILER_LOG_ERROR(LOG_CORE, "StartInHookSimpNmdDump fail, create sem fail");
        return false;
    }
    inHookSimpNmdFd_ = CreateInHookSimpNmdFd();
    if (inHookSimpNmdFd_ < 0) {
        inHookSimpNmdSem_ = nullptr;
        inHookSimpNmdStarted_.store(false);
        PROFILER_LOG_ERROR(LOG_CORE, "StartInHookSimpNmdDump fail, create simp nmd fd fail");
        return false;
    }
    if (!postProcessor(inHookSimpNmdFd_)) {
        close(inHookSimpNmdFd_);
        inHookSimpNmdFd_ = -1;
        inHookSimpNmdSem_ = nullptr;
        inHookSimpNmdStarted_.store(false);
        PROFILER_LOG_ERROR(LOG_CORE, "StartInHookSimpNmdDump fail, post process fail");
        return false;
    }
    return true;
}

std::string HookManager::GetInHookSimpNmdResult(uint32_t waitMs)
{
    std::string result;
    CHECK_TRUE(inHookSimpNmdStarted_.load(), result, "GetInHookSimpNmdResult fail, dump not started");
    CHECK_NOTNULL(inHookSimpNmdSem_, result, "GetInHookSimpNmdResult fail, sem is NULL");
    CHECK_TRUE(inHookSimpNmdFd_ >= 0, result, "GetInHookSimpNmdResult fail, recv fd is invalid");
    bool notified = inHookSimpNmdSem_->TimedWait(waitMs / MS_PER_SEC, (waitMs % MS_PER_SEC) * NS_PER_MS);
    if (!notified) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetInHookSimpNmdResult fail, timeout");
        close(inHookSimpNmdFd_);
        inHookSimpNmdFd_ = -1;
        inHookSimpNmdStarted_.store(false);
        return result;
    }
    if (!ReadFdContent(result, inHookSimpNmdFd_)) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetInHookSimpNmdResult fail, read fd content error");
        result.clear();
    }
    close(inHookSimpNmdFd_);
    inHookSimpNmdFd_ = -1;
    inHookSimpNmdStarted_.store(false);
    return result;
}

bool HookManager::HandleInHookSimpNmdDone()
{
    PROFILER_LOG_DEBUG(LOG_CORE, "HookManager::HandleInHookSimpNmdDone start");
    CHECK_TRUE(inHookSimpNmdStarted_.load(), false, "in hook simp nmd dump task not started");
    CHECK_NOTNULL(inHookSimpNmdSem_, false, "in hook simp nmd sem is NULL");
    inHookSimpNmdSem_->Post();
    PROFILER_LOG_DEBUG(LOG_CORE, "HookManager::HandleInHookSimpNmdDone done");
    return true;
}

void HookManager::WaitMemLeakDoneIfNeeded()
{
    if (!hookConfig_.native_orphansnapshots_enable() || memLeakDumpDone_.load()) {
        return;
    }
    uint32_t waitedMs = 0;
    while (!memLeakDumpDone_.load() && waitedMs < MEM_LEAK_DONE_TIMEOUT_MS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MEM_LEAK_DONE_POLL_MS));
        waitedMs += MEM_LEAK_DONE_POLL_MS;
    }
    if (!memLeakDumpDone_.load()) {
        PROFILER_LOG_WARN(LOG_CORE, "WaitMemLeakDoneIfNeeded timeout");
        memLeakDumpDone_ = true;
    }
}

void HookManager::CollectGwpAsanFrames(std::unordered_map<uintptr_t, DfxFrame>& frameMap, uint8_t* allocDataPtr,
                                       size_t size, size_t allocDataSize)
{
    std::vector<uintptr_t> pcs;
    for (size_t i = 0; i < size; ++i) {
        struct GwpAllocationData* allocData =
            reinterpret_cast<struct GwpAllocationData*>(allocDataPtr + i * allocDataSize);
        for (size_t j = 0; j < gwpAsanMaxStackDepth_ && allocData->stack[j] != 0; ++j) {
            pcs.push_back(allocData->stack[j]);
        }
    }
    
    std::unordered_set<uintptr_t> uniquePcs(pcs.begin(), pcs.end());
    pcs.assign(uniquePcs.begin(), uniquePcs.end());
    
    auto unwinder = std::make_shared<Unwinder>(hookConfig_.pid(), false);
    std::vector<DfxFrame> frames;
    unwinder->GetFramesByPcs(frames, pcs);
    for (size_t i = 0; i < frames.size() && i < pcs.size(); ++i) {
        frameMap[pcs[i]] = frames[i];
    }
}

void HookManager::FormatGwpAsanCallstack(std::stringstream& ss, const struct GwpAllocationData* allocData,
                                         const std::unordered_map<uintptr_t, DfxFrame>& frameMap)
{
    for (size_t j = 0; j < gwpAsanMaxStackDepth_ && allocData->stack[j] != 0; ++j) {
        auto it = frameMap.find(allocData->stack[j]);
        if (it != frameMap.end()) {
            const DfxFrame& frame = it->second;
            ss << "#" << j << " 0x" << std::hex << allocData->stack[j] << std::dec << " ";
            if (!frame.mapName.empty()) {
                ss << frame.mapName << "+" << frame.mapOffset << "(";
            }
            if (!frame.funcName.empty()) {
                ss << frame.funcName << "+" << frame.funcOffset << ")(";
            }
            ss << frame.buildId << ")\n";
        }
    }
}

void HookManager::FormatGwpAsanOutput(std::stringstream& ss, const struct GwpHeader* header, uint8_t* allocDataPtr,
                                      size_t size, size_t allocDataSize,
                                      const std::unordered_map<uintptr_t, DfxFrame>& frameMap)
{
    ss << "-----------GWP_ASAN_CONFIG Start-----------\n";
    ss << "SLOT:" << header->max_slots << ",\n";
    ss << "SAMPLE:" << header->sampled_rate << "\n";
    ss << "-----------GWP_ASAN_CONFIG End-----------\n";
    
    for (size_t i = 0; i < size; ++i) {
        struct GwpAllocationData* allocData =
            reinterpret_cast<struct GwpAllocationData*>(allocDataPtr + i * allocDataSize);
        ss << "-----------GWP_ASAN_MEMORY_SAMPLE Start-----------\n";
        ss << "========================================================\n";
        ss << "address:0x" << std::hex << allocData->addr << std::dec << "\n";
        ss << "size:" << allocData->size << " bytes\n";
        ss << "alloc_time:" << allocData->alive_time << " ms\n";
        ss << "callstack:\n";
        FormatGwpAsanCallstack(ss, allocData, frameMap);
        ss << "-----------endGWP_ASAN_MEMORY_SAMPLE End-----------\n";
    }
}

void HookManager::DumpGwpAsanInfo(uint8_t* data)
{
    if (data == nullptr) {
        return;
    }
    size_t *sizePtr = reinterpret_cast<size_t*>(data);
    struct GwpHeader *header = reinterpret_cast<struct GwpHeader*>(data + sizeof(size_t));
    if (header->sampled_rate == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "Invalid GWP-ASAN info: sampled_rate is 0");
        return;
    }

    if (gwpAsanMaxStackDepth_ == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "gwpAsanMaxStackDepth_ is 0");
        return;
    }
    size_t allocDataSize = sizeof(GwpAllocationData) + (gwpAsanMaxStackDepth_ - 1) * sizeof(uintptr_t);
    uint8_t* allocDataPtr = data + sizeof(size_t) + sizeof(struct GwpHeader);

    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    CollectGwpAsanFrames(frameMap, allocDataPtr, *sizePtr, allocDataSize);

    std::stringstream ss;
    FormatGwpAsanOutput(ss, header, allocDataPtr, *sizePtr, allocDataSize, frameMap);

    std::string result = ss.str();
    if (gwpFd_ > 0) {
        lseek(gwpFd_, 0, SEEK_END);
        write(gwpFd_, result.c_str(), result.size());
    }
}

bool HookManager::TakeDataHandler(const std::shared_ptr<HookManagerCtx>& hookCtx,
    std::shared_ptr<HookRecord>& hookRecord, std::shared_ptr<RawStack>& rawStack, const int8_t *data, uint32_t size)
{
    auto saveMemTagCallBack = [&hookCtx] (uint32_t tagId, const std::string& tagName) {
        hookCtx->stackPreprocess->SaveMemTag(tagId, tagName);
    };
    hookRecord = factory_->GetHookRecord(data, size, true, saveMemTagCallBack);
    CHECK_NOTNULL(hookRecord, false, "TakeDataHandler hookRecord invalid!");
    rawStack = hookRecord->GetRawStack();
    uint16_t type = hookRecord->GetType();
    if (type == FREE_MSG_SIMP) {
        return true;
    }

    if (type == GWP_ASAN_MSG) {
        DumpGwpAsanInfo(rawStack->data);
        return true;
    }

    if (type == MEM_LEAK_MSG) {
        DumpMemLeakInfo();
        return true;
    }

    if (type == NMD_MSG && printMallocNmd_) {
        const char* nmdResult = reinterpret_cast<const char*>(rawStack->data);
        if (!saMode_) {
            DumpNmdInfo(nmdResult);
        }
        if (nmdParamInfo_.type == SIMP_NMD) {
            CHECK_NOTNULL(nmdSemaphore_, false, "nmd semaphore is NULL");
            simplifiedNmd_ = std::string(nmdResult);
            nmdSemaphore_->Post();
            PROFILER_LOG_INFO(LOG_CORE, "receive simplified nmd info, target pid :%d, processName:%s.\n",
                hookCtx->pid, hookCtx->processName.c_str());
        } else {
            lseek(nmdParamInfo_.fd, 0, SEEK_END);
            (void)write(nmdParamInfo_.fd, nmdResult, strlen(nmdResult));
        }
        return true;
    } else if (type == END_MSG) {
        return true;
    }
    rawStack->reportFlag = true;
    if (type == MEMORY_TAG || type == THREAD_NAME_MSG ||
        type == MMAP_FILE_TYPE || type == PR_SET_VMA_MSG ||
        type == JS_STACK_MSG) {
        return true;
    }
    rawStack->reduceStackFlag = false;
    if (hookConfig_.fp_unwind()) {
        if (rawStack->stackContext->jsChainId > 0) {
            rawStack->jsStackData = hookCtx->stackPreprocess->GetJsRawStack(rawStack->stackContext->jsChainId);
        }
        return true;
    }
    return true;
}

bool HookManager::ProcessRawStackByType(const std::shared_ptr<HookManagerCtx>& hookCtx,
    std::shared_ptr<HookRecord>& hookRecord, std::shared_ptr<RawStack>& rawStack)
{
    uint16_t type = hookRecord->GetType();
    if (type == MEMORY_TAG) {
        return true;
    } else if (type == JS_STACK_MSG) {
        hookCtx->stackPreprocess->SaveJsRawStack(rawStack->stackContext->jsChainId,
            reinterpret_cast<char*>(rawStack->data));
        return true;
    } else if (type == END_MSG) {
        hookCtx->FlushStackArray();
        if (!hookCtx->stackData->PutRawStack(hookRecord, hookCtx->isRecordAccurately)) {
            return false;
        }
        if (!hookCtx->stackData->PutRawStack(nullptr, false)) {
            return false;
        }
        return true;
    } else if (type == NMD_MSG) {
        return true;
    }
    FlushRawStackArray(hookCtx, hookRecord);
    return true;
}

void HookManager::ReadShareMemory(const std::shared_ptr<HookManagerCtx>& hookCtx, int sharedMemoryIndex)
{
    DrainSharedMemoryBlock(hookCtx, sharedMemoryIndex, true);
}

void HookManager::FlushPendingRawStackArray(const std::shared_ptr<HookManagerCtx>& hookCtx)
{
    if (hookCtx == nullptr || g_rawStackCount == 0) {
        return;
    }
    hookCtx->FlushStackArray();
}

bool HookManager::DrainSharedMemoryBlock(const std::shared_ptr<HookManagerCtx>& hookCtx, int sharedMemoryIndex,
    bool takeNotifier)
{
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock = hookCtx->shareMemoryBlockList[sharedMemoryIndex];
    CHECK_NOTNULL(shareMemoryBlock, false, "smb is null!");
    if (takeNotifier && sharedMemoryIndex < hookCtx->eventNotifierList.size() &&
        hookCtx->eventNotifierList[sharedMemoryIndex] != nullptr) {
        hookCtx->eventNotifierList[sharedMemoryIndex]->Take();
    }
    bool drained = false;
    while (true) {
        std::shared_ptr<HookRecord> hookRecord = nullptr;
        std::shared_ptr<RawStack> rawStack = nullptr;
        ShareMemoryBlock::DataHandler dataHandler = std::bind(&HookManager::TakeDataHandler, this,
            std::cref(hookCtx), std::ref(hookRecord), std::ref(rawStack), std::placeholders::_1, std::placeholders::_2);
        bool ret = shareMemoryBlock->TakeData(dataHandler);
        if (!ret) {
            break;
        }
        drained = true;
        if (!ProcessRawStackByType(hookCtx, hookRecord, rawStack)) {
            break;
        }
    }
    FlushPendingRawStackArray(hookCtx);
    return drained;
}

bool HookManager::ShouldSkipGlobalFreedRecord(const int8_t* data, uint32_t size) const
{
    if (data == nullptr || size == 0) {
        return false;
    }
    if (!shouldSkipGlobalFreed_ || !IsBaseStackRawDataRecord(data, size)) {
        return false;
    }
    auto* rawData = reinterpret_cast<const BaseStackRawData*>(data);
    return (IsAllocRecord(rawData) || IsFreeRecord(rawData)) && ShouldSkipRecord(rawData, validLiveData_) &&
        !(IsFreeRecord(rawData) && InOrphanFreeWhitelist(rawData, orphanFreeWhitelist_));
}

bool HookManager::ParseRecordsFromBuffer(const std::shared_ptr<HookManagerCtx>& hookCtx, uint8_t* buffer,
    uint32_t totalAvail, uint32_t& offset, uint64_t& readfileSize, bool isCrashFile)
{
    while (offset + sizeof(uint32_t) <= totalAvail) {
        uint32_t totalSize = 0;
        if (isCrashFile) {
            if (!GetRecordHeadWithRetry(buffer, totalAvail, offset, totalSize, isCrashFile)) {
                // retries exhausted (all zeros within window) or invalid head: treat as end of data
                return true;
            }
        } else {
            RecordHeadStatus headStatus = GetRecordHead(buffer, offset, isCrashFile, totalSize);
            if (headStatus == RECORD_HEAD_INVALID) {
                return false;
            }
            if (headStatus == RECORD_HEAD_END) {
                return true;
            }
        }

        if (offset + sizeof(uint32_t) + totalSize > totalAvail) {
            break;
        }

        offset += sizeof(uint32_t);
        readfileSize += sizeof(uint32_t) + totalSize;

        std::shared_ptr<HookRecord> hookRecord = nullptr;
        std::shared_ptr<RawStack> rawStack = nullptr;
        const int8_t* dataPtr = reinterpret_cast<const int8_t*>(buffer + offset);

        totalRecord_++;
        if (ShouldSkipGlobalFreedRecord(dataPtr, totalSize)) {
            offset += totalSize;
            skipRecord_++;
            continue;
        }
        if (!TakeDataHandler(hookCtx, hookRecord, rawStack, dataPtr, totalSize)) {
            PROFILER_LOG_ERROR(LOG_CORE, "ParseRecordsFromBuffer TakeDataHandler failed, skip current record");
            offset += totalSize;
            continue;
        }
        if (!ProcessRawStackByType(hookCtx, hookRecord, rawStack)) {
            PROFILER_LOG_ERROR(LOG_CORE, "ParseRecordsFromBuffer ProcessRawStackByType failed, skip current record");
            offset += totalSize;
            continue;
        }
        offset += totalSize;
    }
    return true;
}

bool HookManager::RefreshValidLiveDataFromBuffer(uint8_t* buffer, uint32_t totalAvail,
                                                 uint32_t& offset, uint64_t& fileSize, bool isCrashFile)
{
    while (offset + sizeof(uint32_t) <= totalAvail) {
        uint32_t totalSize = 0;
        if (isCrashFile) {
            if (!GetRecordHeadWithRetry(buffer, totalAvail, offset, totalSize, isCrashFile)) {
                return true;
            }
        } else {
            RecordHeadStatus headStatus = GetRecordHead(buffer, offset, isCrashFile, totalSize);
            if (headStatus == RECORD_HEAD_INVALID) {
                return false;
            }
            if (headStatus == RECORD_HEAD_END) {
                return true;
            }
        }

        if (offset + sizeof(uint32_t) + totalSize > totalAvail) {
            break;
        }

        offset += sizeof(uint32_t);
        fileSize += sizeof(uint32_t) + totalSize;

        const int8_t* dataPtr = reinterpret_cast<const int8_t*>(buffer + offset);
        if (!IsBaseStackRawDataRecord(dataPtr, totalSize)) {
            offset += totalSize;
            continue;
        }
        auto* rawData = reinterpret_cast<const BaseStackRawData*>(dataPtr);
        if (IsAllocRecord(rawData) || IsFreeRecord(rawData)) {
            UpdateLiveData(rawData, validLiveData_, orphanFreeWhitelist_);
        }
        offset += totalSize;
    }
    return true;
}

uint32_t HookManager::HandleRemainData(uint8_t* buffer, uint32_t totalAvail, uint32_t offset,
                                       int fd, uint64_t& readfileSize)
{
    uint32_t remain = totalAvail - offset;
    if (remain > 0) {
        if (offset == 0) {
            uint32_t oversizeRecord = 0;
            if (memcpy_s(&oversizeRecord, sizeof(uint32_t), buffer,
                         sizeof(uint32_t)) != EOK) {
                return static_cast<uint32_t>(-1);
            }
            PROFILER_LOG_WARN(LOG_CORE,
                              "ScanFileForLiveData record too large, size=%u, skipping",
                              oversizeRecord);
            off_t skipBytes = static_cast<off_t>(oversizeRecord) + static_cast<off_t>(sizeof(uint32_t)) -
                              static_cast<off_t>(remain);
            if (lseek(fd, skipBytes, SEEK_CUR) < 0) return static_cast<uint32_t>(-1);
            readfileSize += sizeof(uint32_t) + oversizeRecord;
            return 0;
        }
        if (memmove_s(buffer, FILE_CACHE_READ_BUFFER_SIZE, buffer + offset,
                      remain) != EOK) {
            return static_cast<uint32_t>(-1);
        }
    }
    return remain;
}

void HookManager::RefreshValidLiveDataFromFile(const std::shared_ptr<HookManagerCtx>& hookCtx, int fd)
{
    if (hookCtx == nullptr) {
        PROFILER_LOG_WARN(LOG_CORE, "RefreshValidLiveDataFromFile invalid hook ctx");
        return;
    }
    if (fd < 0) {
        PROFILER_LOG_INFO(LOG_CORE, "RefreshValidLiveDataFromFile parse all cache file done.");
        return;
    }
    uint32_t dataSize = 0;
    if (!ReadCacheFileHeader(fd, dataSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "RefreshValidLiveDataFromFile read data size header failed, fd=%d errno=%d",
                           fd, errno);
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ReadCacheFileHeader fd=%d dataSize=%u", fd, dataSize);
    if (!SanitizeCacheDataSize(dataSize)) {
        PROFILER_LOG_WARN(LOG_CORE, "RefreshValidLiveDataFromFile dataSize exceeds max, fallback to crash mode");
    }
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.isCrashFile = (dataSize == 0);
    if (ctx.isCrashFile) {
        size_t fdQueueSize = hookCtx->GetFdQueueSize();
        if (fdQueueSize > 1) {
            // zero-header with other fds still queued: anomalous non-crash data, discard this fd
            PROFILER_LOG_ERROR(LOG_CORE, "RefreshValidLiveData crash header but %zu fds queued, discard fd=%d",
                fdQueueSize, fd);
            return;
        }
    }
    ctx.bytesToRead = ctx.isCrashFile ? MAX_CACHE_DATA_SIZE : dataSize;
    ScanFileForLiveData(ctx);
}

bool HookManager::ProcessFileChunk(CacheFileReadContext& ctx)
{
    uint32_t maxRead = GetCacheFileReadSize(ctx);
    if (ctx.readDone) {
        return false;
    }
    ssize_t bytesRead = read(ctx.fd, ctx.readBuffer + ctx.remainSize, maxRead);
    if (bytesRead < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ScanFileForLiveData read failed errno=%d", errno);
        return false;
    }

    if (bytesRead == 0) {
        if (ctx.remainSize > 0 && !ctx.isCrashFile) {
            PROFILER_LOG_ERROR(LOG_CORE, "ScanFileForLiveData incomplete record, size=%u", ctx.remainSize);
        }
        return false;
    }

    uint32_t totalAvail = ctx.remainSize + static_cast<uint32_t>(bytesRead);
    uint32_t offset = 0;
    if (!RefreshValidLiveDataFromBuffer(ctx.readBuffer, totalAvail, offset, ctx.readfileSize, ctx.isCrashFile)) {
        return false;
    }

    uint32_t newRemain = HandleRemainData(ctx.readBuffer, totalAvail, offset, ctx.fd, ctx.readfileSize);
    if (newRemain == static_cast<uint32_t>(-1)) {
        return false;
    }

    ctx.remainSize = newRemain;
    return true;
}

void HookManager::ScanFileForLiveData(CacheFileReadContext& ctx)
{
    auto readBuffer = std::make_unique<uint8_t[]>(FILE_CACHE_READ_BUFFER_SIZE);
    ctx.readBuffer = readBuffer.get();

    bool keepReading = true;
    while (keepReading) {
        keepReading = ProcessFileChunk(ctx);
    }
}

void HookManager::ReadFromCacheFile(const std::shared_ptr<HookManagerCtx>& hookCtx, int fd)
{
    if (fd < 0) {
        PROFILER_LOG_INFO(LOG_CORE, "ReadFromCacheFile parse all cache file done.");
        hookCtx->stackPreprocess->SetIsCacheFileParseDone(true);
        return;
    }
    uint32_t dataSize = 0;
    if (!ReadCacheFileHeader(fd, dataSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFromCacheFile read data size header failed, fd=%d errno=%d", fd, errno);
        close(fd);
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ReadCacheFileHeader fd=%d dataSize=%u", fd, dataSize);
    if (!SanitizeCacheDataSize(dataSize)) {
        PROFILER_LOG_WARN(LOG_CORE, "ReadFromCacheFile dataSize exceeds max, fallback to crash mode");
    }
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.isCrashFile = (dataSize == 0);
    size_t fdQueueSize = hookCtx->GetFdQueueSize();
    if (ctx.isCrashFile && fdQueueSize > 1) {
        // zero-header but other cache fds still queued: anomalous non-crash data, discard this fd
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFromCacheFile crash header but %zu fds queued, discard fd=%d",
            fdQueueSize, fd);
        close(fd);
        return;
    }
    ctx.bytesToRead = ctx.isCrashFile ? MAX_CACHE_DATA_SIZE : dataSize;
    auto readBuffer = std::make_unique<uint8_t[]>(FILE_CACHE_READ_BUFFER_SIZE);
    ctx.readBuffer = readBuffer.get();
    totalRecord_ = 0;
    skipRecord_ = 0;
    PROFILER_LOG_INFO(LOG_CORE, "ReadFromCacheFile dataSize=%u, crashFile=%d",
        dataSize, static_cast<int>(ctx.isCrashFile));

    bool ret = ParseCacheFileLoop(hookCtx, ctx);

    FlushPendingRawStackArray(hookCtx);
    close(fd);
    PROFILER_LOG_INFO(LOG_CORE, "ReadFromCacheFile [%llu]completed, ret %d.",
        static_cast<unsigned long long>(ctx.readfileSize), ret);
    PROFILER_LOG_INFO(LOG_CORE, "ReadFromCacheFile total record %zu, skip record %zu, ratio %f",
        totalRecord_, skipRecord_,
        static_cast<double>(skipRecord_) / static_cast<double>(totalRecord_));
}

bool HookManager::ParseCacheFileLoop(const std::shared_ptr<HookManagerCtx>& hookCtx,
    CacheFileReadContext& ctx)
{
    bool ret = false;
    bool hasError = false;
    while (!hasError) {
        uint32_t maxRead = GetCacheFileReadSize(ctx);
        if (ctx.readDone) {
            ret = true;
            break;
        }
        ssize_t bytesRead = read(ctx.fd, ctx.readBuffer + ctx.remainSize, maxRead);
        if (bytesRead < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "ReadFromCacheFile read failed errno=%d", errno);
            hasError = true;
            break;
        }
        if (bytesRead == 0) {
            if (ctx.remainSize != 0 && !ctx.isCrashFile) {
                PROFILER_LOG_ERROR(LOG_CORE, "ReadFromCacheFile incomplete record at EOF, remainSize=%u",
                    ctx.remainSize);
                hasError = true;
            } else {
                ret = true;
            }
            break;
        }

        uint32_t totalAvail = ctx.remainSize + static_cast<uint32_t>(bytesRead);
        uint32_t offset = 0;
        hookCtx->stackPreprocess->UpdateLastCacheFileActivityTime();
        if (!ParseRecordsFromBuffer(hookCtx, ctx.readBuffer, totalAvail, offset, ctx.readfileSize,
                                    ctx.isCrashFile)) {
            ret = ctx.isCrashFile;
            hasError = !ctx.isCrashFile;
            break;
        }

        if (!HandleParsedRemainder(ctx, totalAvail, offset)) {
            hasError = true;
            break;
        }
        if (ctx.readDone) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool HookManager::HandleParsedRemainder(CacheFileReadContext& ctx, uint32_t totalAvail, uint32_t offset)
{
    ctx.remainSize = totalAvail - offset;
    if (ctx.remainSize == 0) {
        return true;
    }
    if (offset == 0) {
        return HandleOversizeRecord(ctx);
    }
    if (memmove_s(ctx.readBuffer, FILE_CACHE_READ_BUFFER_SIZE, ctx.readBuffer + offset,
                  ctx.remainSize) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadFromCacheFile memmove_s failed");
        return false;
    }
    return true;
}

bool HookManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    ClientConfig clientConfig;
    GetClientConfig(clientConfig);
    for (const auto& item : hookCtx_) {
        if (clientConfig.enableFileCacheMode) {
            if (item->readyFilePoller && item->readyFileNotifier) {
                PROFILER_LOG_INFO(LOG_CORE, "FileCache mode eventPoller unset");
                item->readyFilePoller->RemoveFileDescriptor(item->readyFileNotifier->GetFd());
                item->readyFilePoller->Stop();
                item->readyFilePoller->Finalize();
            }
            while (item->GetFdQueueSize() > 0) {
                int cacheFd = item->GetFdQueueFront();
                item->PopFrontFdQueue();
                if (cacheFd >= 0) {
                    close(cacheFd);
                }
            }
        } else {
            for (size_t i = 0; i < item->eventPollerList.size(); ++i) {
                if (item->eventPollerList[i] && item->eventNotifierList[i]) {
                    PROFILER_LOG_INFO(LOG_CORE, "eventPoller unset! num: %zu", i);
                    item->eventPollerList[i]->RemoveFileDescriptor(item->eventNotifierList[i]->GetFd());
                    item->eventPollerList[i]->Stop();
                    item->eventPollerList[i]->Finalize();
                }
                if (item->shareMemoryBlockList[i]) {
                    ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(item->smbNames[i]);
                }
            }
        }
        if (item->stackPreprocess != nullptr) {
            item->stackPreprocess->FinishTraceFile();
        }
        if (item->stackData != nullptr) {
            item->stackData->ClearCache();
        }
        if (uniqueTableBlock_ != nullptr) {
            ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("unique_stack_table");
            uniqueTableBlock_ = nullptr;
        }
    }
    WaitMemLeakDoneIfNeeded();
    CloseResource();
    return true;
}

void HookManager::CloseResource()
{
    if (fpHookData_) {
        fclose(fpHookData_);
        fpHookData_ = nullptr;
    }
    if (gwpFd_ > 0) {
        close(gwpFd_);
        gwpFd_ = 0;
    }
    CloseMemLeakTempFile();
}

bool HookManager::CloseMemLeakTempFile()
{
    if (memLeakFd_ >= 0) {
        close(memLeakFd_);
        memLeakFd_ = -1;
    }
    if (tempMemLeakFd_ < 0) {
        return true;
    }
    close(tempMemLeakFd_);
    tempMemLeakFd_ = -1;
    std::string tempPath = fileCacheDir_ + memLeakFileName_;
    if (unlink(tempPath.data()) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "memLeak temp file unlink failed, filename: %s", tempPath.c_str());
        return false;
    }
    return true;
}

bool HookManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                     const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    UNUSED_PARAMETER(config);
    if (hookCtx_.empty()) {
        return false;
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    StartPluginSession();
    return true;
}

bool HookManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    if (hookCtx_.empty()) {
        return false;
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    for (const auto& item : hookCtx_) {
        WaitFinalSnapshotIfNeeded(item);
        StopTargetProcess(item);
        WaitMemLeakDoneIfNeeded();
        if (!FinalizeStoppedContext(item)) {
            return false;
        }
        CleanupHookServiceContext(static_cast<pid_t>(item->pid));
    }
    if (!SetLocalHandle("0")) {
        PROFILER_LOG_WARN(LOG_CORE, "reset preload param failed");
    }
    return true;
}

void HookManager::ResetStartupParam(pid_t pid)
{
    const std::string resetParam = "startup:disabled";
    if (hookConfig_.startup_mode()) {
        int ret = SystemSetParameter(PARAM_NAME.c_str(), resetParam.c_str());
        if (ret < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "set param failed, please reset param(%s)", PARAM_NAME.c_str());
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "reset param success");
        }
        UpdateFreezeExcludedPid(true, pid, getpid());
    }
}

bool HookManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    return true;
}

bool HookManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
                               bool isProtobufSerialize)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "agentIndex_ %d", agentIndex_);
    writer_ = std::make_shared<BufferWriter>(pluginName, VERSION, bufferSize, smbFd, eventFd, agentIndex_);
    isProtobufSerialize_ = isProtobufSerialize;
    shareMemorySize_ = bufferSize;
    return true;
}

void HookManager::CreateWriter()
{
    PROFILER_LOG_INFO(LOG_CORE, "CreateWriter isProtobufSerialize: %d, noDataQueue_: %d",
                      isProtobufSerialize_, noDataQueue_);
    if (isProtobufSerialize_) {
        RegisterWriter(writer_);
    } else {
        writerAdapter_ = std::make_shared<WriterAdapter>(isProtobufSerialize_);
        writerAdapter_->SetWriter(writer_);
    }
}

bool HookManager::ResetWriter(uint32_t pluginId)
{
    RegisterWriter(nullptr);
    return true;
}

void HookManager::RegisterWriter(const std::shared_ptr<Writer> writer)
{
    g_buffWriter = writer;
}

void HookManager::RegisterFdForSa(uint32_t saFd)
{
    saFd_ = saFd;
}

void HookManager::SetHookConfig(const NativeHookConfig& hookConfig)
{
    hookConfig_ = hookConfig;
}

void HookManager::SethookStandalone(bool HookStandalone)
{
    isHookStandalone_ = HookStandalone;
}

void HookManager::SetHookConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    hookConfig_.set_pid(config->pid_);
    if (!config->processName_.empty()) {
        hookConfig_.set_process_name(config->processName_);
    }
    hookConfig_.set_filter_size(config->filterSize_);
    hookConfig_.set_smb_pages(config->shareMemorySize_);
    hookConfig_.set_max_stack_depth(config->maxStackDepth_);
    hookConfig_.set_malloc_disable(config->mallocDisable_);
    hookConfig_.set_mmap_disable(config->mmapDisable_);
    hookConfig_.set_free_stack_report(config->freeStackData_);
    hookConfig_.set_munmap_stack_report(config->munmapStackData_);
    hookConfig_.set_malloc_free_matching_interval(config->mallocFreeMatchingInterval_);
    hookConfig_.set_malloc_free_matching_cnt(config->mallocFreeMatchingCnt_);
    hookConfig_.set_string_compressed(config->stringCompressed_);
    hookConfig_.set_fp_unwind(config->fpUnwind_);
    hookConfig_.set_blocked(config->blocked_);
    hookConfig_.set_record_accurately(config->recordAccurately_);
    hookConfig_.set_startup_mode(config->startupMode_);
    hookConfig_.set_memtrace_enable(config->memtraceEnable_);
    hookConfig_.set_async_stack_enable(config->asyncStackEnable_);
    asyncFlagMask_ = config->asyncFlag_;
    hookConfig_.set_nested_async_stack_enable(config->nestedAsyncStackEnable_);
    traceMask_ = config->traceMask_;
    hookConfig_.set_offline_symbolization(config->offlineSymbolization_);
    hookConfig_.set_callframe_compress(config->callframeCompress_);
    hookConfig_.set_statistics_interval(config->statisticsInterval_);
    hookConfig_.set_clock(COMMON::GetClockStr(config->clockId_));
    hookConfig_.set_sample_interval(config->sampleInterval_);
    hookConfig_.set_response_library_mode(config->responseLibraryMode_);
    hookConfig_.set_js_stack_report(config->jsStackReport_);
    hookConfig_.set_max_js_stack_depth(config->maxJsStackDepth_);
    hookConfig_.set_filter_napi_name(config->filterNapiName_);
    hookConfig_.set_target_so_name(config->targetSoName_);
    hookConfig_.clear_filter_lib_list();
    for (const auto& filterLib : config->filterLibList_) {
        hookConfig_.add_filter_lib_list(filterLib);
    }
    hookConfig_.set_allocator_sampling_enable(config->allocatorSamplingEnable_);
    hookConfig_.set_unique_stack_table_size(config->uniqueStackTableSize_);
    hookConfig_.set_use_file_cache_mode(config->enableFileCacheMode_);
    printMallocNmd_ = config->printNmd_;
    if (config->hookstandalone_) {
        isHookStandalone_ = true;
    }
    if (config->saveFile_ || config->hookstandalone_) {
        hookConfig_.set_save_file(config->saveFile_);
        hookConfig_.set_file_name(config->fileName_);
        isProtobufSerialize_ = true;
    }
    SetHookConfigExt(config);
}

void HookManager::SetHookConfigExt(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    hookConfig_.set_native_orphansnapshots_enable(config->nativeOrphanSnapshotsEnable_);
    hookConfig_.set_async_nesting_depth(config->asyncNestingDepth_);
    hookConfig_.set_async_task_stack_depth(config->asyncTaskStackDepth_);
    hookConfig_.set_arkui_mem_trace_enable(config->arkuiMemTraceEnable_);
    hookConfig_.set_discard_destroyed_traces(config->discardDestroyedTraces_);
    SetMemLeakFormat(config->memLeakFormat_);
    memLeakLimit_ = config->memLeakLimit_;
    largestSize_ = config->largestSize_;
    secondLargestSize_ = config->secondLargestSize_;
    maxGrowthSize_ = config->maxGrowthSize_;
    gwpAsanHoldTime_ = config->gwpAsanHoldTime_;
    gwpAsanMaxEntries_ = config->gwpAsanMaxEntries_;
    gwpAsanMaxStackDepth_ = config->gwpAsanMaxStackDepth_;
}

void HookManager::SetSimplifiedMemConfig(const std::shared_ptr<SimplifiedMemConfig>& config)
{
    if (config == nullptr) {
        return;
    }
    maxGrowthSize_ = config->maxGrowthSize;
}

void HookManager::GetSimplifiedMemConfig(std::shared_ptr<SimplifiedMemConfig>& config)
{
    if (config == nullptr) {
        return;
    }
    config->largestSize = largestSize_;
    config->secondLargestSize = secondLargestSize_;
    config->maxGrowthSize = maxGrowthSize_;
    config->sampleSize = hookConfig_.sample_interval();
}

int32_t HookManager::CreatePluginSession()
{
    if (CreatePluginSession({})) {
        return RET_OK;
    }
    return RET_ERR;
}

void HookManager::StartPluginSession(std::unordered_map<pid_t, std::pair<uid_t, gid_t>>* pidInfo)
{
    for (const auto& item : hookCtx_) {
        if (item->stackPreprocess == nullptr) {
            continue;
        }
        PROFILER_LOG_INFO(LOG_CORE, "StartPluginSession name: %s", item->processName.c_str());
        if (!noDataQueue_) {
            item->stackPreprocess->StartTakeResults();
        }
        item->stackPreprocess->InitStatisticsTime();
        if (item->pid > 0) {
            PROFILER_LOG_INFO(LOG_CORE, "start command : send 36 signal to process  %d", item->pid);
            UpdateFreezeExcludedPid(true, static_cast<int32_t>(item->pid), getpid());
            uid_t uid = 0;
            gid_t gid = 0;
            if (!COMMON::GetUidGidFromPid(static_cast<pid_t>(item->pid), uid, gid)) {
                item->stackPreprocess->SetStopTakeData();
                PROFILER_LOG_INFO(LOG_CORE, "get target process uid gid failed");
                continue;
            }
            if (kill(item->pid, SIGNAL_START_HOOK) == -1) {
                item->stackPreprocess->SetStopTakeData();
                const int bufSize = 256;
                char buf[bufSize] = {0};
                strerror_r(errno, buf, bufSize);
                PROFILER_LOG_ERROR(LOG_CORE, "send 36 signal error = %s", buf);
            }
            if (!saMode_) {
                hookService_->AddPidInfo(static_cast<pid_t>(item->pid), uid, gid);
            } else if (pidInfo != nullptr) {
                (*pidInfo)[static_cast<pid_t>(item->pid)] = std::make_pair(uid, gid);
            }
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "StartPluginSession: pid(%d) is less or equal zero.", item->pid);
        }
    }
    if (!saMode_) {
        int ret = COMMON::PluginWriteToHisysevent("native_hook_plugin", "sh", GetCmdArgs(hookConfig_),
            COMMON::ErrorType::RET_SUCC, "success");
        PROFILER_LOG_INFO(LOG_CORE, "hisysevent report native_hook_plugin result:%d", ret);
    }
}

std::string HookManager::GetCmdArgs(const NativeHookConfig& traceConfig)
{
    std::stringstream args;
    args << "pid: " << COMMON::GetProcessNameByPid(traceConfig.pid()) << ", ";
    args << "save_file: " << (traceConfig.save_file() ? "true" : "false") << ", ";
    args << "filter_size: " << std::to_string(traceConfig.filter_size()) << ", ";
    args << "smb_pages: " << std::to_string(traceConfig.smb_pages()) << ", ";
    args << "max_stack_depth: " << std::to_string(traceConfig.max_stack_depth()) << ", ";
    args << "process_name: " << traceConfig.process_name() << ", ";
    args << "malloc_disable: " << (traceConfig.malloc_disable() ? "true" : "false") << ", ";
    args << "mmap_disable: " << (traceConfig.mmap_disable() ? "true" : "false") << ", ";
    args << "free_stack_report: " << (traceConfig.free_stack_report() ? "true" : "false") << ", ";
    args << "munmap_stack_report: " << (traceConfig.munmap_stack_report() ? "true" : "false") << ", ";
    args << "malloc_free_matching_interval: " << std::to_string(traceConfig.malloc_free_matching_interval()) << ", ";
    args << "malloc_free_matching_cnt: " << std::to_string(traceConfig.malloc_free_matching_cnt()) << ", ";
    args << "string_compressed: " << (traceConfig.string_compressed() ? "true" : "false") << ", ";
    args << "fp_unwind: " << (traceConfig.fp_unwind() ? "true" : "false") << ", ";
    args << "blocked: " << (traceConfig.blocked() ? "true" : "false") << ", ";
    args << "unique_stack_table_size: " << std::to_string(traceConfig.unique_stack_table_size()) << ", ";
    args << "record_accurately: " << (traceConfig.record_accurately() ? "true" : "false") << ", ";
    args << "startup_mode: " << (traceConfig.startup_mode() ? "true" : "false") << ", ";
    args << "memtrace_enable: " << (traceConfig.memtrace_enable() ? "true" : "false") << ", ";
    args << "offline_symbolization: " << (traceConfig.offline_symbolization() ? "true" : "false") << ", ";
    args << "callframe_compress: " << (traceConfig.callframe_compress() ? "true" : "false") << ", ";
    args << "statistics_interval: " << std::to_string(traceConfig.statistics_interval()) << ", ";
    args << "clock: " << traceConfig.clock() << ", ";
    args << "sample_interval: " << std::to_string(traceConfig.sample_interval()) << ", ";
    args << "response_library_mode: " << (traceConfig.response_library_mode() ? "true" : "false") << ", ";
    args << "js_stack_report: " << std::to_string(traceConfig.js_stack_report()) << ", ";
    args << "max_js_stack_depth: " << std::to_string(traceConfig.max_js_stack_depth()) << ", ";
    args << "filter_napi_name: " << traceConfig.filter_napi_name() << ", ";
    args << "target_so_name: " << traceConfig.target_so_name() << ", ";
    for (const auto& filterLib : traceConfig.filter_lib_list()) {
        args << "filter_lib_list: " << filterLib << ", ";
    }
    args << "native_orphansnapshots_enable: " << (traceConfig.native_orphansnapshots_enable() ? "true" : "false")
         << ", ";
    for (const auto& pid : traceConfig.expand_pids()) {
        args << "expand_pids: " << std::to_string(pid) << ", ";
    }
    return args.str();
}

void HookManager::WriteHookConfig()
{
    for (const auto& item : hookCtx_) {
        if (item == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HookManager WriteHookConfig failed");
            return;
        }
        item->stackPreprocess->WriteHookConfig();
    }
}

bool HookManager::PrepareFileCacheContext(int32_t pid, const std::string& name)
{
    auto item = FindHookContext(pid, name);
    if (item == nullptr) {
        return false;
    }
    if (item->pid == -1) {
        item->pid = pid;
    }
    item->cacheFileStopReceived = false;
    item->cacheFileStopRequested = false;
    item->cacheFileClientDisconnected = false;
    item->cacheFileParsing = false;
    if (item->stackPreprocess != nullptr) {
        item->stackPreprocess->SetPid(pid);
        item->stackPreprocess->SetIsCacheFileParseDone(false);
        item->stackPreprocess->SetDataSourceTerminated(false);
    }
    return true;
}

int HookManager::CreateFileCacheFd()
{
    std::string tempPath = fileCacheDir_ + FILE_CACHE_TEMPLATE;
    std::vector<char> tempPathBuf(tempPath.begin(), tempPath.end());
    tempPathBuf.push_back('\0');
    int fd = mkstemp(tempPathBuf.data());
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateFileCacheFd mkstemp failed errno=%d", errno);
        return -1;
    }
    if (fchmod(fd, FILE_CACHE_MODE) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateFileCacheFd fchmod failed errno=%d", errno);
        close(fd);
        (void)unlink(tempPathBuf.data());
        return -1;
    }
    if (unlink(tempPathBuf.data()) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateFileCacheFd unlink failed errno=%d", errno);
        close(fd);
        return -1;
    }
    // pre-allocate physical blocks before fd handoff: fallocate must not run inside the hooked
    // app, whose seccomp filter rejects it (SIGSYS); same direct-call style as trace_file_writer
    if (fallocate(fd, 0, 0, static_cast<off_t>(MMAP_FILE_SIZE)) != 0) {
        PROFILER_LOG_WARN(LOG_CORE, "CreateFileCacheFd fallocate failed errno=%d, fallback ftruncate", errno);
        if (ftruncate(fd, static_cast<off_t>(MMAP_FILE_SIZE)) != 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "CreateFileCacheFd ftruncate failed errno=%d", errno);
            close(fd);
            return -1;
        }
    }
    return fd;
}

std::vector<int> HookManager::GetFds(int32_t pid, const std::string& name, int sharedMemCount)
{
    for (const auto& item : hookCtx_) {
        if (item->pid == pid || item->processName == name) {
            if (item->pid == -1) {
                item->pid = pid;
            }
            item->stackPreprocess->SetPid(pid);
            std::vector<int> fds;
            for (int i = 0; i < sharedMemCount; ++i) {
                fds.push_back(item->eventNotifierList[i]->GetFd());
                fds.push_back(item->shareMemoryBlockList[i]->GetfileDescriptor());
            }
            return fds;
        }
    }
    return std::vector<int>(sharedMemCount * DOUBLE, -1);
}

void HookManager::SetNmdInfo(std::pair<uint32_t, uint32_t> info)
{
    printMallocNmd_ = true;
    nmdParamInfo_.fd = info.first;
    nmdParamInfo_.type = info.second;
    nmdSemaphore_ = nmdParamInfo_.type == SIMP_NMD ? GetSemaphoreFactory().Create(0) : nullptr;
}

std::string HookManager::GetSimplifiedNmdResult(uint32_t waitMs)
{
    CHECK_NOTNULL(nmdSemaphore_, "", "nmd semaphore not created for type %u", nmdParamInfo_.type);
    CHECK_TRUE(nmdSemaphore_->TimedWait(waitMs / MS_PER_SEC, (waitMs % MS_PER_SEC) * NS_PER_MS), "",
        "wait simplified nmd result FAIL for %u ms", waitMs);
    std::string result = std::move(simplifiedNmd_);
    simplifiedNmd_.clear();
    return result;
}

void HookManager::SetGwpFd(uint32_t fd)
{
    gwpFd_ = fd;
}

void HookManager::SetMemLeakFd(int fd)
{
    memLeakFd_ = fd;
}

bool HookManager::HandleCachedFileFdByPid(pid_t pid, int fd, uint32_t fileIndex)
{
    (void)fileIndex;
    auto item = FindHookContext(pid, "");
    if (item == nullptr) {
        return false;
    }
    if (item->stackPreprocess != nullptr) {
        item->stackPreprocess->SetIsStartTakeData(true);
    }
    item->PushBackFdQueue(fd);
    PROFILER_LOG_INFO(LOG_CORE, "HandleCachedFileFdByPid push fd, queue size %zu", item->GetFdQueueSize());
    
    if (!hookConfig_.offline_symbolization()) {
        // only for online_symbolization
        HandleBackpressure(item);
    }
    
    if (item->readyFileNotifier != nullptr && !item->readyFileNotifier->Post(1)) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleCachedFileFdByPid post ready notifier failed");
        return false;
    }
    return true;
}

bool HookManager::HandleCacheFileStopByPid(pid_t pid)
{
    auto item = FindHookContext(pid, "");
    return FinalizeCacheFileStop(item);
}

bool HookManager::HandleClientDisconnectedByPid(pid_t pid)
{
    auto item = FindHookContext(pid, "");
    if (item == nullptr) {
        PROFILER_LOG_WARN(LOG_CORE, "HandleClientDisconnectedByPid: hook context not found for pid %d", pid);
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "HandleClientDisconnectedByPid: pid %d disconnected, fileCacheMode %d", pid,
        hookConfig_.use_file_cache_mode());
    if (!hookConfig_.use_file_cache_mode()) {
        if (item->stackPreprocess == nullptr) {
            PROFILER_LOG_WARN(LOG_CORE, "HandleClientDisconnectedByPid: stackPreprocess null for pid %d", pid);
            return false;
        }
        FinalizeSharedMemoryTermination(item);
        return true;
    }
    
    item->ProcessPendingCacheFdOnDisconnect();
    
    item->cacheFileClientDisconnected = true;
    return FinalizeCacheFileStop(item);
}

void HookManager::RequestCacheFileStop(pid_t pid)
{
    auto item = FindHookContext(pid, "");
    if (item != nullptr) {
        item->cacheFileStopRequested = true;
    }
}

bool HookManager::IsCacheFileStopRequested(pid_t pid)
{
    auto item = FindHookContext(pid, "");
    return item != nullptr && item->cacheFileStopRequested;
}

void HookManager::SaveInitialCacheFds(pid_t pid, uint16_t fileIndex, int fd)
{
    if (fd < 0 || fileIndex >= FILE_CACHE_FD_COUNT) {
        PROFILER_LOG_ERROR(LOG_CORE, "SaveInitialCacheFds:invalid fd %d or fileIndex %d", fd, fileIndex);
        return;
    }

    auto item = FindHookContext(pid, "");
    if (item == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "SaveInitialCacheFds:context not found for pid %d", pid);
        return;
    }
    int dupFd = dup(fd);
    if (fileIndex == 0) {
        item->SetCurrentActiveCacheFd(fileIndex, dupFd);
    } else {
        item->AddPendingCacheFd(fileIndex, dupFd);
    }
    PROFILER_LOG_INFO(LOG_CORE, "SaveInitialCacheFds: pid:%d dupFd:%d", pid, dupFd);
}

void HookManager::RotateCacheFdOnReturn(pid_t pid, uint16_t returnedFileIndex, int fd)
{
    auto item = FindHookContext(pid, "");
    if (item == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "RotateCacheFdOnReturn: context not found for pid %d", pid);
        return;
    }
    if (fd < 0 || returnedFileIndex >= FILE_CACHE_FD_COUNT) {
        item->SetCurrentActiveCacheFd(0, -1);
        PROFILER_LOG_INFO(LOG_CORE, "RotateCacheFdOnReturn:invalid fd %d or fileIndex %d", fd, returnedFileIndex);
        return;
    }

    int dupFd = dup(fd);
    item->RotateActiveCacheFd(returnedFileIndex, dupFd);
    PROFILER_LOG_INFO(LOG_CORE, "RotateCacheFdOnReturn: pid:%d returnedIndex:%u dupFd:%d",
        pid, returnedFileIndex, dupFd);
}

bool HookManager::CheckHookPid(int32_t pid)
{
    auto item = FindHookContext(pid, "");
    if (item == nullptr) {
        return false;
    }
    return true;
}

void HookManager::HandleBackpressure(const std::shared_ptr<HookManagerCtx>& ctx)
{
    uint32_t depth = ctx->GetFdQueueSize();
    ctx->queueDepth_ = depth;
    
    uint32_t delay = CalculateRecommendedDelay(depth);
    if (ctx->recommendedDelayNs_ == delay) {
        return;
    }
    ctx->recommendedDelayNs_ = delay;
    
    uint64_t now = GetBackpressureCurrentTimeMs();
    if (now - ctx->lastStateBroadcastTime_.load() >= bpConfig_.stateBroadcastIntervalMs) {
        BackpressureStateMsg state = ctx->CalculateBackpressureState();
        SendBackpressureState(ctx->pid, state);
        ctx->lastStateBroadcastTime_ = now;
    }
}

uint32_t HookManager::CalculateRecommendedDelay(uint32_t queueDepth)
{
    const BackpressureConfig& cfg = bpConfig_;
    
    if (queueDepth <= cfg.lowWatermark) {
        return cfg.minDelayNs;
    } else if (queueDepth <= cfg.midWatermark) {
        float ratio = static_cast<float>(queueDepth - cfg.lowWatermark) /
                      (cfg.midWatermark - cfg.lowWatermark);
        return static_cast<uint32_t>(cfg.minDelayNs + ratio * (cfg.midDelayNs - cfg.minDelayNs));
    } else if (queueDepth <= cfg.highWatermark) {
        float ratio = static_cast<float>(queueDepth - cfg.midWatermark) /
                      (cfg.highWatermark - cfg.midWatermark);
        return static_cast<uint32_t>(cfg.midDelayNs +
            (cfg.highDelayNs - cfg.midDelayNs) * ratio * ratio);
    } else {
        return cfg.highDelayNs;
    }
}

void HookManager::SendBackpressureState(pid_t pid, const BackpressureStateMsg& state)
{
    if (hookService_ == nullptr) {
        return;
    }
    
    hookService_->SendBackpressureState(pid, state);
}
}
