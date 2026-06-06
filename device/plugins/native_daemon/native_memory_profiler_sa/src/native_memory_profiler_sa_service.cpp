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

#include "native_memory_profiler_sa_service.h"
#include "native_memory_profiler_sa_death_recipient.h"
#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "trace_file_writer.h"
#include "socket_context.h"
#include "hook_common.h"
#include "init_param.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "common.h"
#include "logging.h"
#include "native_memory_profiler_sa_callback_proxy.h"
#include <sys/socket.h>
#include <unistd.h>

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr int32_t TIME_BASE = 1000;
const std::string FILE_PATH_HEAD = "/data/local/tmp/native_memory_";
const std::string FILE_PATH_TAIL = ".htrace";
const std::string PRODUCT_CONFIG_PATH = "/sys_prod/etc/hiview/hiprofiler/hiprofiler_cfg.json";
constexpr int32_t DELAYED_SHUTDOWN_TIME = 20;
constexpr int FILE_MODE = 0644;
constexpr int32_t SIMP_NMD = 3;
constexpr int32_t NMD_WAIT_MS = 5000;
constexpr int VEC_SHIFT = 2;
constexpr double MIN_MEM_ROM_PERCENT = 0.20;
constexpr double MIN_MEM_PROFILER_MEM_PERCENT = 0.20;
constexpr double MAX_MEM_PROFILER_CPU_PERCENT = 0.7;
const std::string API_SANDBOX_PATH_PREFIX = "/data/storage/el2/base/files/";
constexpr uint32_t START_TYPE_HIVIEW = 0;
constexpr uint32_t START_TYPE_API = 1;

static std::unordered_map<std::string, std::shared_ptr<TaskConfig>> g_nameAndFilePathCtx;
static std::unordered_map<int32_t, std::shared_ptr<TaskConfig>> g_pidCtx;
static std::mutex g_mtx;
static ScheduleTaskManager g_scheduleTaskManager;
static bool g_hasStartupMode{false};
static int32_t g_taskNum{0};
static std::shared_ptr<ServiceEntry> g_serviceEntry{nullptr};
static int32_t g_delayedShutdownTimerFd{-1};
static std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> g_nmdPidType;
static std::unordered_map<pid_t, int> g_pidFds;
static std::mutex g_pidFdMtx;
static std::unordered_map<pid_t, std::pair<uid_t, gid_t>> g_pidInfo;
static std::vector<std::vector<uint64_t>> g_gpuRange {GPU_TYPE_COUNT, std::vector<uint64_t>(GPU_RANGE_COUNT * 2, 0)};
static std::unordered_map<pid_t, std::string> g_fileNameInfo;
static std::unordered_map<int32_t, int32_t> g_timerFdInfo;
static std::mutex g_timerFdMtx;
static std::mutex g_fileNameMtx;
//2: double
static std::mutex g_nmdMtx;
static std::unordered_set<uint32_t> g_asyncStoppingPids;
static std::mutex g_asyncStoppingPidsMtx;
#ifdef ENABLE_SA_AUTO_START
const bool G_REGISTER_RESUTL = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance().get());
#endif

bool SendFileCacheFd(SocketContext &context, HookManager& hookMgr, int32_t pid, uint16_t fileIndex)
{
    int cacheFd = hookMgr.CreateFileCacheFd();
    if (cacheFd < 0) {
        return false;
    }
    HookCacheFileMsg cacheFileMsg(pid, fileIndex, CACHE_FILE_CREATED);
    bool ret = context.SendHookConfig(reinterpret_cast<uint8_t*>(&cacheFileMsg), sizeof(cacheFileMsg)) &&
        context.SendFileDescriptor(cacheFd);
    close(cacheFd);
    return ret;
}

void SendHookCommand(pid_t pid, uint32_t cmd)
{
    std::unique_lock<std::mutex> lock(g_pidFdMtx);
    auto it = g_pidFds.find(pid);
    if (it == g_pidFds.end() || it->second <= 0) {
        return;
    }
    int sockFd = it->second;
    lock.unlock();

    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_PROTOBUF;
    phead.protoSize = sizeof(cmd) + sizeof(struct ProtocolHead);
    if (send(sockFd, reinterpret_cast<int8_t*>(&phead), sizeof(phead), 0) == -1) {
        return;
    }
    (void)send(sockFd, reinterpret_cast<int8_t*>(&cmd), sizeof(cmd), 0);
}

bool SendInitialFileCacheFds(SocketContext &context, HookManager& hookMgr, int32_t pid, const std::string& name)
{
    if (!hookMgr.PrepareFileCacheContext(pid, name)) {
        return false;
    }
    for (uint16_t fileIndex = 0; fileIndex < FILE_CACHE_FD_COUNT; ++fileIndex) {
        int cacheFd = hookMgr.CreateFileCacheFd();
        if (cacheFd < 0) {
            return false;
        }
        bool ret = context.SendFileDescriptor(cacheFd);
        close(cacheFd);
        if (!ret) {
            return false;
        }
    }
    return true;
}

bool SendMemLeakFd(SocketContext &context, HookManager& hookMgr, const ClientConfig& clientConfig)
{
    if (clientConfig.memLeakFormat == MemLeakFormat::TYPE_DISABLE) {
        return true;
    }
    int fd = hookMgr.CreateMemLeakFile();
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "SendMemLeakFd invalid fd");
        return false;
    }
    bool ret = context.SendFileDescriptor(fd);
    PROFILER_LOG_INFO(LOG_CORE, "SendMemLeakFd sent fd %d", fd);
    return ret;
}

int32_t FindPidBySocket(int socketFd)
{
    std::lock_guard<std::mutex> lock(g_pidFdMtx);
    for (const auto& item : g_pidFds) {
        if (item.second == socketFd) {
            return item.first;
        }
    }
    return -1;
}

bool HandleMemLeakDoneBySocket(SocketContext &context)
{
    int32_t pid = FindPidBySocket(context.GetSocketHandle());
    if (pid <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc mem leak done without pid");
        return true;
    }
    std::lock_guard<std::mutex> guard(g_mtx);
    auto iter = g_pidCtx.find(pid);
    if (iter == g_pidCtx.end() || iter->second == nullptr || iter->second->hookMgr == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc mem leak done without hook manager");
        return true;
    }
    if (!iter->second->hookMgr->HandleMemLeakDone()) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc mem leak done failed");
    }
    return true;
}
}

NativeMemoryProfilerSaService::NativeMemoryProfilerSaService() : SystemAbility(NATIVE_DAEMON_SYSTEM_ABILITY_ID, true)
{
    char buffer[PATH_MAX + 1] = {0};
    readlink("/proc/self/exe", buffer, PATH_MAX);
    std::string processName = buffer;
    if (processName == "/system/bin/native_daemon") {
        return;
    }
    serviceName_ = "HookService";
    g_serviceEntry = std::make_shared<ServiceEntry>();
    if (!g_serviceEntry->StartServer(DEFAULT_UNIX_SOCKET_HOOK_PATH,
                                     reinterpret_cast<void (*)(int)>(&ClientDisconnectCallback))) {
        g_serviceEntry = nullptr;
        PROFILER_LOG_ERROR(LOG_CORE, "Start IPC Service FAIL");
        return;
    }
    g_serviceEntry->RegisterService(*this);
    DelayedShutdown(false);
}

NativeMemoryProfilerSaService::~NativeMemoryProfilerSaService()
{
    g_serviceEntry = nullptr;
}

void NativeMemoryProfilerSaService::ClientDisconnectCallback(int socketFd)
{
    std::unique_lock<std::mutex> lock(g_pidFdMtx);
    pid_t pidval = 0;
    for (auto iter = g_pidFds.begin(); iter != g_pidFds.end(); ++iter) {
        if (iter->second == socketFd) {
            pidval = iter->first;
            break;
        }
    }
    PROFILER_LOG_ERROR(LOG_CORE, "ClientDisconnectCallback get pid: %d", static_cast<int>(pidval));
    lock.unlock();
    if (!HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopHook failed, no profiling permission!");
        return;
    }
    bool isAPI = false;
    std::string name = "";
    // Lock the mutex to ensure safe access to the shared resource g_pidCtx
    {
        std::lock_guard<std::mutex> guard(g_mtx);
        std::shared_ptr<TaskConfig> config = nullptr;
        auto taskIter = g_pidCtx.find(pidval);
        if (taskIter != g_pidCtx.end()) {
            config = taskIter->second;
        }
        if (config == nullptr) {
            PROFILER_LOG_WARN(LOG_CORE,
                "ClientDisconnectCallback: config not found for pid %d, skip cleanup", pidval);
            return;
        }
        isAPI = (config->startType == START_TYPE_API);
        name = config->processName;
    }
    {
        std::lock_guard<std::mutex> guard(g_timerFdMtx);
        auto fdTimerIter = g_timerFdInfo.find(pidval);
        if (fdTimerIter != g_timerFdInfo.end()) {
            // delete schedule task.
            g_scheduleTaskManager.UnscheduleTask(fdTimerIter->second);
            g_timerFdInfo.erase(fdTimerIter);
        }
    }
    // Note: The same mutex g_mtx is also used in the StopHook function.
    // Be cautious to avoid potential deadlocks.
    StopHookByType(pidval, name, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS), isAPI);
}

bool NativeMemoryProfilerSaService::StartServiceAbility()
{
    sptr<ISystemAbilityManager> serviceManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(serviceManager, false, "serviceManager is nullptr");
    
    static sptr<NativeMemoryProfilerSaService> native(new NativeMemoryProfilerSaService());
    CHECK_NOTNULL(native, false, "native is nullptr");
    int32_t result = serviceManager->AddSystemAbility(NATIVE_DAEMON_SYSTEM_ABILITY_ID, native);
    if (result != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Service native memory failed to start");
        return false;
    }

    auto abilityObject = serviceManager->AsObject();
    CHECK_NOTNULL(abilityObject, false, "abilityObject is nullptr");

    bool ret = abilityObject->AddDeathRecipient(new NativeMemoryProfilerSaDeathRecipient());
    if (ret == false) {
        PROFILER_LOG_ERROR(LOG_CORE, "AddDeathRecipient failed");
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "Service native memory started successfully");
    return true;
}

bool NativeMemoryProfilerSaService::HasProfilingPermission()
{
    if (!COMMON::IsUserMode()) {
        return true;
    }
    uint32_t callingTokenID = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingTokenID,
                                                                       "ohos.permission.ENABLE_PROFILER");
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "No profiling permission, please check!");
        return false;
    }
    return true;
}

bool NativeMemoryProfilerSaService::ApplyNmdConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
    uint32_t fdForNmd)
{
    if (!config->printNmd_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(g_nmdMtx);
    g_nmdPidType[config->nmdPid_] = std::make_pair(fdForNmd, config->nmdType_);
    return !config->printNmdOnly_;
}

void NativeMemoryProfilerSaService::ApplyMemtraceConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (!config->memtraceEnable_) {
        return;
    }
    std::lock_guard<std::mutex> guard(gpuMtx_);
    g_gpuRange = config->gpuRange_;
}

int32_t NativeMemoryProfilerSaService::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    auto args = GetCmdArgs(config);
    if (config->startType_ != START_TYPE_API && !HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, no profiling permission!");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_NO_PERMISSION,
            "no profiling permission");
        return RET_ERR;
    }
    if (config->startType_ == START_TYPE_API &&
        config->pid_ != static_cast<int32_t>(IPCSkeleton::GetCallingPid())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, api pid mismatch!");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }
    if (IsNativeDeamonRunning()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Native Daemon is already running,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
        return RET_ERR; // inner interface error
    }
    if (ApplyNmdConfig(config, 0)) {
        return RET_OK;
    }
    if (StartHook(config) != RET_OK) { // inner interface error
        return RET_ERR;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                             MessageParcel &reply)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    auto args = GetCmdArgs(config);
    if (config->startType_ != START_TYPE_API && !HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, no profiling permission!");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_NO_PERMISSION,
            "no profiling permission");
        return RET_ERR;
    }
    if (config->startType_ == START_TYPE_API &&
        config->pid_ != static_cast<int32_t>(IPCSkeleton::GetCallingPid())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, api pid mismatch!");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }
    if (IsNativeDeamonRunning()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Native Daemon is already running,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
        return RET_ERR; // inner interface error
    }
    if (ApplyNmdConfig(config, 0)) {
        return RET_OK;
    }
    ApplyMemtraceConfig(config);
    if (StartHook(config, 0, reply) != RET_OK) { // inner interface error
        return RET_ERR;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::CheckSystemResource()
{
    const int oneHundred = 100;
    MemoryInfo memInfo;
    if (!GetSystemMemoryInfo(&memInfo)) {
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL);
    }

    if (memInfo.availablePercent < MIN_MEM_PROFILER_MEM_PERCENT) {
        PROFILER_LOG_ERROR(LOG_CORE, "System memory is too low: %.2f%%", memInfo.availablePercent * oneHundred);
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL);
    }
    
    double cpuUsage = 0.0;
    if (!GetSystemCpuUsageInfo(&cpuUsage)) {
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED);
    }
    if (cpuUsage > MAX_MEM_PROFILER_CPU_PERCENT) {
        PROFILER_LOG_ERROR(LOG_CORE, "System CPU usage is too high: %.2f%%", cpuUsage * oneHundred);
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED);
    }
    
    double freeRom = 0.0;
    freeRom = COMMON::GetFreeRomPercent(COMMON::DEFAULT_RW_DIR);
    if (freeRom < MIN_MEM_ROM_PERCENT) {
        PROFILER_LOG_ERROR(LOG_CORE, "System ROM is too low: %.2f%%", freeRom * oneHundred);
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STORAGE_PRESSURE_CRITICAL);
    }

    return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS);
}

int32_t NativeMemoryProfilerSaService::StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
    const sptr<IRemoteObject>& callback)
{
    int32_t pid = static_cast<int32_t>(IPCSkeleton::GetCallingPid());
    if (config->pid_ != pid) {
        PROFILER_LOG_ERROR(LOG_CORE, "Permission Denied,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }

    if (IsNativeDeamonRunning()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Native Daemon is already running,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT);
    }
    const std::string& fn = config->typeFileName_;
    if (fn.empty() || fn == "." || fn.find("..") != std::string::npos ||
        fn.find("/") != std::string::npos) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartPidVoid: invalid typeFileName_");
        return RET_ERR;
    }
    {
        std::lock_guard<std::mutex> guard(g_fileNameMtx);
        g_fileNameInfo[pid] = config->typeFileName_;
    }
    if (ApplyNmdConfig(config, 0)) {
        return RET_OK;
    }
    ApplyMemtraceConfig(config);
    RefreshFileCacheModeConfig(config);
    int32_t retResource = CheckSystemResource();
    if (retResource != static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS)) {
        PROFILER_LOG_ERROR(LOG_CORE, "System resource is not enough,errorcode is:%d.", retResource);
        return retResource;
    }
    auto args = GetCmdArgs(config);
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    uint32_t fd = 0;
    return StartHookLock(config, fd, hook, args, callback);
}

// only for API
int32_t NativeMemoryProfilerSaService::Stop()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> guard(g_timerFdMtx);
        auto fdTimerIter = g_timerFdInfo.find(pid);
        if (fdTimerIter != g_timerFdInfo.end()) {
            // delete schedule task.
            g_scheduleTaskManager.UnscheduleTask(fdTimerIter->second);
            g_timerFdInfo.erase(fdTimerIter);
        }
    }
    int32_t retStopHook = StopHookAsync(static_cast<uint32_t>(pid),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
    PROFILER_LOG_INFO(LOG_CORE, "StopHookAsync ret:%d", retStopHook);
    if (retStopHook != RET_OK) {
        return retStopHook;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::Stop(uint32_t pid)
{
    if (!HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopHook failed, no profiling permission!");
        return RET_ERR;
    }
    (void)StopHook(pid, "", static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::Stop(const std::string& name)
{
    if (!HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopHook failed, no profiling permission!");
        return RET_ERR;
    }
    (void)StopHook(0, name, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    auto args = GetCmdArgs(config);
    if (config->startType_ != START_TYPE_API && !HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, no profiling permission!");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_NO_PERMISSION,
            "no profiling permission");
        return RET_ERR;
    }
    if (config->startType_ == START_TYPE_API &&
        config->pid_ != static_cast<int32_t>(IPCSkeleton::GetCallingPid())) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed, api pid mismatch!");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }
    if (IsNativeDeamonRunning()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Native Daemon is already running,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
        return RET_ERR; // inner interface error
    }
    if (ApplyNmdConfig(config, fd)) {
        return RET_OK;
    }
    ApplyMemtraceConfig(config);
    if (StartHook(config, fd) != RET_OK) {
        close(fd);
        return RET_ERR;
    }
    return RET_OK;
}

void NativeMemoryProfilerSaService::CloseSocketFd(pid_t pid)
{
    std::unique_lock<std::mutex> lock(g_pidFdMtx);
    if ((g_pidFds.find(pid) != g_pidFds.end()) && (g_pidFds[pid] != 0)) {
        int socketFd = g_pidFds[pid];
        g_pidFds.erase(pid);
        lock.unlock();
        g_serviceEntry->RemoveContext(socketFd);
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService::CloseSocketFd pid invalid");
    }
}

int32_t NativeMemoryProfilerSaService::StopHook(uint32_t pid, std::string name, int32_t finishErrorCode)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    std::shared_ptr<TaskConfig> config = nullptr;
    if (pid > 0) {
        if (auto taskIter = g_pidCtx.find(pid); taskIter != g_pidCtx.end()) {
            config = taskIter->second;
        }
    } else if (auto taskIter = g_nameAndFilePathCtx.find(name); taskIter != g_nameAndFilePathCtx.end()) {
        config = taskIter->second;
        pid = static_cast<pid_t>(config->hookMgr->GetPid());
    }
    if (config == nullptr) {
        int32_t ret = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED);
        PROFILER_LOG_ERROR(LOG_CORE,
            "NativeMemoryProfilerSaService::hook has stop, pid: %d, process name: %s, errorcode is:%d.",
            pid, name.c_str(), ret);
        return ret;
    }
    std::string resultPath;
    int32_t retRelease = ReleaseForStopHook(pid, config, resultPath);
    int32_t callbackError = retRelease == RET_OK ? finishErrorCode :
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
    NotifyProfilingResult(config, callbackError, retRelease == RET_OK ? resultPath : "");
    if (--g_taskNum == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "StringViewMemoryHold clear");
        StringViewMemoryHold::GetInstance().Clear();
        DelayedShutdown(false);
    }
    return retRelease;
}

void NativeMemoryProfilerSaService::StopHookTask(uint32_t pid, std::shared_ptr<TaskConfig> config,
    int32_t finishErrorCode)
{
    std::string resultPath;
    int32_t retRelease = ReleaseForStopHook(pid, config, resultPath, true);
    int32_t callbackError = retRelease == RET_OK ? finishErrorCode :
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
    NotifyProfilingResult(config, callbackError, retRelease == RET_OK ? resultPath : "");

    std::lock_guard<std::mutex> guard(g_mtx);
    if (--g_taskNum == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "StringViewMemoryHold clear");
        StringViewMemoryHold::GetInstance().Clear();
        DelayedShutdown(false);
    }
}

int32_t NativeMemoryProfilerSaService::StopHookAsync(uint32_t pid, int32_t finishErrorCode)
{
    std::shared_ptr<TaskConfig> config = nullptr;
    if (pid > 0) {
        std::lock_guard<std::mutex> guard(g_mtx);
        if (auto taskIter = g_pidCtx.find(pid); taskIter != g_pidCtx.end()) {
            config = taskIter->second;
        }
    }
    if (config == nullptr) {
        int32_t ret = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED);
        PROFILER_LOG_ERROR(LOG_CORE, "StopHookAsync has stop, pid: %d, errorcode is:%d.", pid, ret);
        return ret;
    }

    {
        std::lock_guard<std::mutex> guard(g_asyncStoppingPidsMtx);
        if (g_asyncStoppingPids.find(pid) != g_asyncStoppingPids.end()) {
            PROFILER_LOG_INFO(LOG_CORE, "StopHookAsync: pid %d is already stopping, skip.", pid);
            return RET_OK;
        }
        g_asyncStoppingPids.insert(pid);
    }

    std::thread([pid, config, finishErrorCode]() {
        StopHookTask(pid, config, finishErrorCode);
        std::lock_guard<std::mutex> guard(g_asyncStoppingPidsMtx);
        g_asyncStoppingPids.erase(pid);
    }).detach();

    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StopHookByType(uint32_t pid, std::string& name, int32_t finishErrorCode,
    bool isAsync)
{
    if (isAsync) {
        return StopHookAsync(pid, finishErrorCode);
    }
    return StopHook(pid, name, finishErrorCode);
}

std::string NativeMemoryProfilerSaService::GetCmdArgs(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    std::stringstream args;
    args << "pid: " << COMMON::GetProcessNameByPid(config->pid_) << ", ";
    args << "filter_size: " << config->filterSize_ << ", ";
    args << "max_stack_depth: " << std::to_string(config->maxStackDepth_) << ", ";
    args << "process_name: " << config->processName_ << ", ";
    args << "malloc_disable: " << (config->mallocDisable_ ? "true" : "false") << ", ";
    args << "mmap_disable: " << (config->mmapDisable_ ? "true" : "false") << ", ";
    args << "free_stack_report: " << (config->freeStackData_ ? "true" : "false") << ", ";
    args << "munmap_stack_report: " << (config->munmapStackData_ ? "true" : "false") << ", ";
    args << "malloc_free_matching_interval: " << std::to_string(config->mallocFreeMatchingInterval_) << ", ";
    args << "malloc_free_matching_cnt: " << std::to_string(config->mallocFreeMatchingCnt_) << ", ";
    args << "string_compressed: " << (config->stringCompressed_ ? "true" : "false") << ", ";
    args << "fp_unwind: " << (config->fpUnwind_ ? "true" : "false") << ", ";
    args << "blocked: " << (config->blocked_ ? "true" : "false") << ", ";
    args << "record_accurately: " << (config->recordAccurately_ ? "true" : "false") << ", ";
    args << "startup_mode: " << (config->startupMode_ ? "true" : "false") << ", ";
    args << "memtrace_enable: " << (config->memtraceEnable_ ? "true" : "false") << ", ";
    args << "asyncStack_enable: " << (config->asyncStackEnable_ ? "true" : "false") << ", ";
    args << "offline_symbolization: " << (config->offlineSymbolization_ ? "true" : "false") << ", ";
    args << "callframe_compress: " << (config->callframeCompress_ ? "true" : "false") << ", ";
    args << "statistics_interval: " << std::to_string(config->statisticsInterval_) << ", ";
    args << "clock: " << std::to_string(config->clockId_) << ", ";
    args << "sample_interval: " << std::to_string(config->sampleInterval_) << ", ";
    args << "response_library_mode: " << (config->responseLibraryMode_ ? "true" : "false") << ", ";
    args << "js_stack_report: " << std::to_string(config->jsStackReport_) << ", ";
    args << "max_js_stack_depth: " << std::to_string(config->maxJsStackDepth_) << ", ";
    args << "filter_napi_name: " << config->filterNapiName_ << ", ";
    args << "nmd_pid: " << std::to_string(config->nmdPid_) << ", ";
    args << "nmd_type: " << std::to_string(config->nmdType_) << ", ";
    args << "mem_leak_format: " << std::to_string(config->memLeakFormat_) << ", ";
    return args.str();
}

int32_t NativeMemoryProfilerSaService::StartHookLock(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                                     uint32_t& fd, std::shared_ptr<HookManager>& hook,
                                                     std::string& args, const sptr<IRemoteObject>& callback)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    if (config->processName_.empty() && config->pid_ > 0) {
        config->processName_ = COMMON::GetBundleNameByPid(config->pid_);
    }
    if (config->filePath_.empty() && fd == 0) {
        std::string filePathStr = (config->pid_ > 0) ? std::to_string(config->pid_) : config->processName_;
        config->filePath_ = FILE_PATH_HEAD + filePathStr + FILE_PATH_TAIL;
    }
    PROFILER_LOG_INFO(LOG_CORE, "file path: %s", config->filePath_.c_str());
    int32_t checkRet = CheckConfig(config, fd);
    if (checkRet != RET_OK) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_MSG_EMPTY,
            "check config failed");
        return checkRet;
    }
    if (fd == 0) {
        auto retFile = COMMON::CheckNotExistsFilePath(config->filePath_);
        if (!retFile.first) {
            PROFILER_LOG_INFO(LOG_CORE, "%s:check file path %s fail", __func__, config->filePath_.c_str());
            COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_INVALID_PATH,
                "check file path failed");
            return RET_ERR;
        }
        int fdTemp = open(retFile.second.c_str(), O_RDWR | O_CREAT, FILE_MODE);
        if (config->startType_ == START_TYPE_API) {
            unlink(retFile.second.c_str());
        }
        CHECK_TRUE(fdTemp >= 0, RET_ERR, "Failed to open file(%s)", config->filePath_.c_str());
        fd = static_cast<uint32_t>(fdTemp);
    }
    uint32_t saFd = 0;
    if (!config->printNmdOnly_ && config->gwpAsanHoldTime_ == 0 && !config->nativeOrphanSnapshotsEnable_) {
        saFd = fd;
    }
    hook->RegisterFdForSa(saFd);
    hook->SetGwpFd(config->gwpAsanHoldTime_ > 0 ? fd : 0);
    hook->SetMemLeakFd(config->nativeOrphanSnapshotsEnable_ ? static_cast<int>(fd) : -1);
    hook->SetSaMode(true);
    hook->SetHookConfig(config);
    hook->SetSaServiceConfig(true, config->hookstandalone_);
    if (config->pid_ > 0) {
        std::lock_guard<std::mutex> nmdGuard(g_nmdMtx);
        if (g_nmdPidType.find(config->pid_) != g_nmdPidType.end()) {
            hook->SetNmdInfo(g_nmdPidType[config->pid_]);
        }
    }
    if (hook->CreatePluginSession() != RET_OK) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_FAIL,
            "create pluginsession failed");
        return RET_ERR;
    }
    if (!config->printNmdOnly_ && config->gwpAsanHoldTime_ == 0 && !config->nativeOrphanSnapshotsEnable_) {
        hook->WriteHookConfig();
    }
    hook->StartPluginSession(&g_pidInfo);

    int32_t timerFd = g_scheduleTaskManager.ScheduleTask(
        [config, filePath = config->filePath_] {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService:This task will be stopped.errorcode is:%d.",
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_AUTO_STOPPED_BY_DURATION));
            int32_t retStop = StopHookByType(config->pid_, config->processName_,
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_AUTO_STOPPED_BY_DURATION),
                config->startType_ == START_TYPE_API);
            if (retStop != RET_OK) {
                PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService StopHook failed");
            }
            PROFILER_LOG_INFO(LOG_CORE, "Auto stop completed, file: %s", filePath.c_str());
        },
        config->duration_ * TIME_BASE,
        true);
    if (timerFd == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService Start Schedule Task failed");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_FAIL,
            "start schedule task failed");
        return RET_ERR;
    }
    {
        std::lock_guard<std::mutex> guard(g_timerFdMtx);
        g_timerFdInfo[config->pid_] = timerFd;
    }

    std::shared_ptr<TaskConfig> configCtx =
        std::make_shared<TaskConfig>(hook, config->pid_, config->processName_, config->filePath_, timerFd,
                                     config->startupMode_, fd, config->startType_);
    configCtx->callback = callback;
    if (!g_hasStartupMode && config->startupMode_) {
        g_hasStartupMode = true;
        startupModeProcessName_ = config->processName_;
    }

    if (configCtx->pid > 0) {
        g_pidCtx[configCtx->pid] = configCtx;
    } else if (!configCtx->processName.empty()) {
        g_nameAndFilePathCtx[configCtx->processName] = configCtx;
    }

    if (fd == 0) {
        g_nameAndFilePathCtx[configCtx->filePath] = configCtx;
    }
    ++g_taskNum;
    DelayedShutdown(true);
    COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_SUCC, "success");
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    auto args = GetCmdArgs(config);
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    return StartHookLock(config, fd, hook, args);
}

int32_t NativeMemoryProfilerSaService::StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                                 uint32_t fd, MessageParcel &reply)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    auto args = GetCmdArgs(config);
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    bool isInputZeroFd = (fd == 0); // when fd is 0, need to bring fd out of function to close.
    int32_t ret = StartHookLock(config, fd, hook, args);
    if (ret != RET_OK) {
        if (isInputZeroFd && fd > 0) {
            close(fd);
        }
        return ret;
    }

    if (config->nmdType_ == SIMP_NMD) {
        std::string simplifiedNmdResult = hook->GetSimplifiedNmdResult(NMD_WAIT_MS);
        CHECK_TRUE(!simplifiedNmdResult.empty(), RET_ERR, "Get simplified nmd result FAIL");
        WRITESTRING(reply, simplifiedNmdResult, RET_ERR);
        return RET_OK;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::GetTaskNum(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return 0;
    }
    if (config->startType_ == START_TYPE_API) {
        auto it = g_pidInfo.find(config->pid_);
        if (it == g_pidInfo.end()) {
            return 0;
        }
        int32_t uidTaskModeCount = 0;
        auto targetUid = it->second.first;
        for (auto& item : g_pidInfo) {
            if (item.second.first == targetUid) {
                uidTaskModeCount++;
            }
        }
        return uidTaskModeCount;
    }
    if (config->startType_ == START_TYPE_HIVIEW) {
        int32_t hiviewTaskModeCount = 0;
        for (const auto& item : g_pidCtx) {
            if ((item.second != nullptr) && (item.second->startType == START_TYPE_HIVIEW)) {
                hiviewTaskModeCount++;
            }
        }
        return hiviewTaskModeCount;
    }

    return 0;
}

int32_t NativeMemoryProfilerSaService::PidAndNameConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    if (config->pid_ > 0) {
        config->processName_.clear();
        auto ctx = g_pidCtx[config->pid_];
        if (ctx != nullptr) {
            // Determine whether the existing task's startType matches the requested startType.
            // If they match, set RES_PROF_ALREADY_STARTED; otherwise set RES_PROF_CONFLICT.
            int32_t retErrorCode = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT);
            if (ctx->startType == config->startType_) {
                retErrorCode = static_cast<uint32_t>(ResProfilerErrorCode::RES_PROF_ALREADY_STARTED);
            }
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has started, pid: %d, errorcode: %d.",
                config->pid_, retErrorCode);
            return retErrorCode;
        }
    } else if (!config->processName_.empty()) {
        if (g_nameAndFilePathCtx.find(config->processName_) != g_nameAndFilePathCtx.end()) {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has started, process name: %s",
                config->processName_.c_str());
            return RET_ERR;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The PID and process name are not configured");
        return RET_ERR;
    }
    return RET_OK;
}

// Check whether task count (same UID for API mode, or all HIVIEW tasks)
// exceeds the per-app limit defined by taskMaxNum_.
// Also checks the global task limit across all UIDs (globalTaskMaxNum_).
// Returns RES_PROF_PROCESS_OVERLIMIT if either limit is exceeded.
int32_t NativeMemoryProfilerSaService::CheckTaskLimit(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    auto maxTaskCount = GetValueFromJsonFile(PRODUCT_CONFIG_PATH, "hiprofiler_hook_process_count");
    taskMaxNum_ = maxTaskCount > 0 ? maxTaskCount : taskMaxNum_;

    int taskCount = GetTaskNum(config);
    PROFILER_LOG_INFO(LOG_CORE, "GetTaskNum: taskCount:%d", taskCount);
    if ((taskCount + 1) > taskMaxNum_) {
        if (config->startType_ == START_TYPE_HIVIEW) {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has started, pid: %d", config->pid_);
            return RET_ERR;
        }
        int checkRet = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PROCESS_OVERLIMIT);
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: too many tasks, errorcode: %d.", checkRet);
        return checkRet;
    }

    int globalTaskCount = 0;
    for (const auto& item : g_pidCtx) {
        if (item.second != nullptr) {
            globalTaskCount++;
        }
    }
    if ((globalTaskCount + 1) > globalTaskMaxNum_) {
        int checkRet = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PROCESS_OVERLIMIT);
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: global task limit reached, errorcode: %d.",
            checkRet);
        return checkRet;
    }
    return RET_OK;
}

// Check whether a startup mode task conflicts with an already running startup mode task.
// Only one startup mode task is allowed at a time.
// Returns RET_ERR if a startup mode conflict is detected.
int32_t NativeMemoryProfilerSaService::CheckStartupModeConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (g_hasStartupMode && config->startupMode_) {
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: tasks with an existing startup mode, name: %s",
            startupModeProcessName_.c_str());
        return RET_ERR;
    }
    return RET_OK;
}

// Check whether the requested PID/process name conflicts with an already running task.
// For HIVIEW mode tasks, conflicts return RET_ERR directly.
// For API mode tasks, returns the specific conflict error code.
int32_t NativeMemoryProfilerSaService::CheckPidNameConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    int32_t conflictRet = PidAndNameConflict(config);
    if ((conflictRet != RET_OK) && (config->startType_ == START_TYPE_HIVIEW)) {
        return RET_ERR;
    } else if (conflictRet != RET_OK) {
        return conflictRet;
    }
    return RET_OK;
}

// Check whether the output file path is available.
// If fd > 0 (file-cache mode), file path check is skipped.
// Otherwise, validates that filePath_ is not empty and not already in use.
int32_t NativeMemoryProfilerSaService::CheckFilePath(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (fd > 0) {
        return RET_OK;
    }

    if (!config->filePath_.empty()) {
        if (g_nameAndFilePathCtx.find(config->filePath_) != g_nameAndFilePathCtx.end()) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: File %s is being used.",
                config->filePath_.c_str());
            return RET_ERR;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The file path are not configured");
        return RET_ERR;
    }
    return RET_OK;
}

// Entry point for config validation. Orchestrates all checks in order.
// Returns early on the first failure. If extendProcesses_ is true, skips all checks.
int32_t NativeMemoryProfilerSaService::CheckConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd)
{
    if (config->extendProcesses_) {
        return RET_OK;
    }

    int32_t ret = CheckTaskLimit(config);
    if (ret != RET_OK) {
        return ret;
    }

    ret = CheckStartupModeConflict(config);
    if (ret != RET_OK) {
        return ret;
    }

    ret = CheckPidNameConflict(config);
    if (ret != RET_OK) {
        return ret;
    }

    return CheckFilePath(fd, config);
}

void NativeMemoryProfilerSaService::FillTaskConfigContext(int32_t pid, const std::string& name)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    if (auto iter = g_pidCtx.find(pid); iter != g_pidCtx.end()) {
        iter->second->processName = name;
        g_nameAndFilePathCtx[name] = iter->second;
        if (iter->second->isStartupMode) {
            g_hasStartupMode = false;
        }
    } else if (auto it = g_nameAndFilePathCtx.find(name); it != g_nameAndFilePathCtx.end()) {
        it->second->pid = pid;
        g_pidCtx[pid] = it->second;
        if (it->second->isStartupMode) {
            g_hasStartupMode = false;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: fill TaskConfig context failed");
    }
}

void NativeMemoryProfilerSaService::RefreshFileCacheModeConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    config->enableFileCacheMode_ = true;
}

void NativeMemoryProfilerSaService::SetGpuConfig(ClientConfig& clientConfig)
{
    std::lock_guard<std::mutex> guard(gpuMtx_);
    for (size_t i = 0; i < GPU_RANGE_COUNT * 2; ++i) { // 2: double
        clientConfig.gpuRange.gpuVk[i] = g_gpuRange[GPU_VK_INDEX][i];
        clientConfig.gpuRange.gpuGlesImage[i] = g_gpuRange[GPU_GLES_IMAGE_INDEX][i];
        clientConfig.gpuRange.gpuGlesBuffer[i] = g_gpuRange[GPU_GLES_BUFFER_INDEX][i];
        clientConfig.gpuRange.gpuClImage[i] = g_gpuRange[GPU_CL_IMAGE_INDEX][i];
        clientConfig.gpuRange.gpuClBuffer[i] = g_gpuRange[GPU_CL_BUFFER_INDEX][i];
    }
}

bool NativeMemoryProfilerSaService::ReceiveCacheFileMsg(SocketContext &context, const int8_t *buf)
{
    if (buf == nullptr) {
        return false;
    }
    HookCacheFileMsg cacheFileMsg = *reinterpret_cast<HookCacheFileMsg *>(const_cast<int8_t*>(buf));
    std::lock_guard<std::mutex> guard(g_mtx);
    if (auto iter = g_pidCtx.find(cacheFileMsg.hookPid); iter != g_pidCtx.end()) {
        if (iter->second->hookMgr == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "hook manager is null");
            return false;
        }
        ClientConfig clientConfig;
        iter->second->hookMgr->GetClientConfig(clientConfig);
        if (clientConfig.enableFileCacheMode) {
            if (cacheFileMsg.msgType == CACHE_FILE_READY) {
                int cacheFd = context.ReceiveFileDiscriptor();
                PROFILER_LOG_INFO(LOG_CORE, "ReceiveCacheFileMsg pid:%d index %u",
                    cacheFileMsg.hookPid, cacheFileMsg.fileIndex);
                if (!iter->second->hookMgr->HandleCachedFileFdByPid(cacheFileMsg.hookPid,
                    cacheFd, cacheFileMsg.fileIndex)) {
                    close(cacheFd);
                    return false;
                }
                if (!iter->second->hookMgr->IsCacheFileStopRequested(cacheFileMsg.hookPid) &&
                    !SendFileCacheFd(context, *(iter->second->hookMgr),
                    cacheFileMsg.hookPid, cacheFileMsg.fileIndex)) {
                    PROFILER_LOG_ERROR(LOG_CORE, "ReceiveCacheFileMsg replenish fd failed");
                    return false;
                }
            } else if (cacheFileMsg.msgType == CACHE_FILE_STOP) {
                return iter->second->hookMgr->HandleCacheFileStopByPid(cacheFileMsg.hookPid);
            }
            return true;
        }
    }
    PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc: receive invalid config");
    return false;
}

bool NativeMemoryProfilerSaService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf,
    const uint32_t size)
{
    if (size == sizeof(HookCacheFileMsg)) {
        return ReceiveCacheFileMsg(context, buf);
    }

    if (size != sizeof(int)) {
        return false;
    }
    int peerConfig = *const_cast<int *>(reinterpret_cast<const int *>(buf));
    if (static_cast<uint32_t>(peerConfig) == MEM_LEAK_DONE_CMD) {
        return HandleMemLeakDoneBySocket(context);
    }
    if (peerConfig == -1) {
        return false;
    }

    std::string filePath = "/proc/" + std::to_string(peerConfig) + "/cmdline";
    std::string bundleName;
    if (!LoadStringFromFile(filePath, bundleName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!, pid: %d", peerConfig);
        return false;
    }
    bundleName.resize(strlen(bundleName.c_str()));

    if (bundleName.substr(0, 2) == "./") { // 2: size, Command line programs will be prefixed with "./"
        bundleName = bundleName.substr(2); // 2: point
    }
    bool startupMode = g_hasStartupMode;
    FillTaskConfigContext(peerConfig, bundleName); // Save the relevant context for subsequent inspection

    std::lock_guard<std::mutex> guard(g_mtx);
    if (auto iter = g_pidCtx.find(peerConfig); iter != g_pidCtx.end()) {
        if (!startupMode) {
            int socketHandle = context.GetSocketHandle();
            struct ucred cred;
            socklen_t len = sizeof(struct ucred);
            if (getsockopt(socketHandle, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1) {
                PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc getsockopt failed");
                return false;
            }
            if (!((g_pidInfo.find(cred.pid) != g_pidInfo.end()) && (g_pidInfo[cred.pid].first == cred.uid) &&
                (g_pidInfo[cred.pid].second == cred.gid))) {
                PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc process uid gid check failed");
                return false;
            }
        }
        if (iter->second->hookMgr == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "hook manager is null");
            return false;
        }
        iter->second->hookMgr->SetPid(peerConfig);
        {
            std::lock_guard<std::mutex> socketLock(g_pidFdMtx);
            g_pidFds[static_cast<pid_t>(peerConfig)] = context.GetSocketHandle();
        }
        PROFILER_LOG_INFO(LOG_CORE,
            "ProtocolProc, receive message from hook client, and send hook config to process %d, name: %s",
            peerConfig, bundleName.c_str());
        ClientConfig clientConfig;
        iter->second->hookMgr->GetClientConfig(clientConfig);
        SetGpuConfig(clientConfig);
        if (iter->second->hookMgr->GetNoDataQueueFlag()) {
            clientConfig.freeEventOnlyAddrEnable = true;
        }
        clientConfig.isSaMode = true;
        ClientConfigIpc clientConfigIpc;
        if (!ConvertClientConfigToIpc(clientConfig, clientConfigIpc) ||
            !context.SendHookConfig(reinterpret_cast<const uint8_t *>(&clientConfigIpc), sizeof(clientConfigIpc))) {
            CloseSocketFd(static_cast<pid_t>(peerConfig));
            return false;
        }
        if (clientConfig.enableFileCacheMode) {
            if (!SendInitialFileCacheFds(context, *(iter->second->hookMgr), peerConfig, bundleName)) {
                PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc send initial file cache fds failed");
                CloseSocketFd(static_cast<pid_t>(peerConfig));
                return false;
            }
        } else {
            int sharedMemCount = (clientConfig.offlineSymbolization) ? SHARED_MEMORY_NUM : 1;
            std::vector<int> fdVec = iter->second->hookMgr->GetFds(peerConfig, bundleName, sharedMemCount);
            for (int i = 0; i < sharedMemCount; ++i) {
                int eventFd = fdVec[i * VEC_SHIFT];
                int smbFd = fdVec[i * VEC_SHIFT + 1];
                if (eventFd == smbFd) {
                    PROFILER_LOG_ERROR(LOG_CORE,
                        "Get eventFd and smbFd failed!, name: %s, pid: %d", bundleName.c_str(), peerConfig);
                    return false;
                }
                context.SendFileDescriptor(smbFd);
                context.SendFileDescriptor(eventFd);
            }
        }
        // Send unique_stack_table fd for async stack collection
        if (clientConfig.asyncStackEnable && iter->second->hookMgr->uniqueTableBlock_ != nullptr) {
            int uniqueTableFd = iter->second->hookMgr->uniqueTableBlock_->GetfileDescriptor();
            context.SendFileDescriptor(uniqueTableFd);
            PROFILER_LOG_INFO(LOG_CORE, "ProtocolProc: sent unique_stack_table fd=%d to pid=%d",
                uniqueTableFd, peerConfig);
        } else if (clientConfig.asyncStackEnable) {
            PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc: uniqueTableBlock_ is nullptr for pid=%d", peerConfig);
        }
        if (!SendMemLeakFd(context, *(iter->second->hookMgr), clientConfig)) {
            CloseSocketFd(static_cast<pid_t>(peerConfig));
            return false;
        }
        iter->second->hookMgr->ResetStartupParam();
    } else {
        SendHookCommand(static_cast<pid_t>(peerConfig), STOP_HOOK_CMD);
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc: send config failed");
        return false;
    }
    return true;
}

void NativeMemoryProfilerSaService::DelayedShutdown(bool cancel)
{
    if (cancel) {
        g_scheduleTaskManager.UnscheduleTask(g_delayedShutdownTimerFd);
        g_delayedShutdownTimerFd = -1;
    } else {
        int32_t timerFd = g_scheduleTaskManager.ScheduleTask(
            []() {
                auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
                if (samgrProxy == nullptr) {
                    PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: get samgr failed");
                    return;
                }
                int32_t ret =
                    samgrProxy->UnloadSystemAbility(INativeMemoryProfilerSa::NATIVE_DAEMON_SYSTEM_ABILITY_ID);
                if (ret != ERR_OK) {
                    PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: remove system ability failed");
                    return;
                }
            },
            DELAYED_SHUTDOWN_TIME * TIME_BASE, true);
        if (timerFd == -1) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService:DelayedShutdown Schedule Task failed");
            return;
        }
        g_delayedShutdownTimerFd = timerFd;
    }
}

void NativeMemoryProfilerSaService::OnStart()
{
    if (started_) {
        PROFILER_LOG_INFO(LOG_CORE, "memory collector has already started");
        return;
    }
    if (!Publish(DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance().get())) {
        PROFILER_LOG_ERROR(LOG_CORE, "memory collector publish failed");
        return;
    }
    started_ = true;
}

void NativeMemoryProfilerSaService::OnStop()
{
    if (!started_) {
        PROFILER_LOG_INFO(LOG_CORE, "memory collector has already stopped or not started");
    }
    started_ = false;
    PROFILER_LOG_DEBUG(LOG_CORE, "NativeMemoryProfilerSaService::OnStop leave");
}

bool NativeMemoryProfilerSaService::GetSystemMemoryInfo(MemoryInfo *memInfo)
{
    if (!memInfo) {
        return false;
    }
    
    std::ifstream memInfoFile("/proc/meminfo");
    if (!memInfoFile.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: open meminfo file failed");
        return false;
    }

    std::string line;
    uint64_t totalMem = 0;
    uint64_t freeMem = 0;
    uint64_t availableMem = 0;

    while (std::getline(memInfoFile, line)) {
        std::istringstream iss(line);
        std::string key;
        uint64_t value;
        if (iss >> key >> value) {
            if (key == "MemTotal:") {
                totalMem = value;
            } else if (key == "MemFree:") {
                freeMem = value;
            } else if (key == "MemAvailable:") {
                availableMem = value;
            }
        }
    }
    memInfoFile.close();

    if (totalMem == 0 || freeMem == 0 || availableMem == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: invalid memory info");
        return false;
    }

    memInfo->totalMem = totalMem;
    memInfo->freeMem = freeMem;
    memInfo->usedMem = totalMem - availableMem;
    memInfo->availablePercent = static_cast<double>(availableMem) / static_cast<double>(totalMem);

    return true;
}

bool NativeMemoryProfilerSaService::ParseCpuStats(const std::string& content, unsigned long& total, unsigned long& idle)
{
    std::istringstream iss(content);
    std::string line;
    if (std::getline(iss, line)) {
        if (line.find("cpu ") == 0) { // The line starts with "cpu "
            std::istringstream lineStream(line);
            std::string cpuLabel;
            unsigned long user;
            unsigned long nice;
            unsigned long system;
            unsigned long idleVal;
            unsigned long iowait;
            unsigned long irq;
            unsigned long softirq;
            unsigned long steal;
            unsigned long guest;
            unsigned long guest_nice;

            lineStream >> cpuLabel >> user >> nice >> system >> idleVal >> iowait >> irq >> softirq >> steal
                >> guest >> guest_nice;
            total = user + nice + system + idleVal + iowait + irq + softirq + steal;
            idle = idleVal + iowait;

            return true;
        }
    }
    return false;
}

bool NativeMemoryProfilerSaService::GetSystemCpuUsageInfo(double* cpuUsage)
{
    if (cpuUsage == nullptr) {
        return false;
    }

    std::vector<std::string> validPaths = {"/proc"};
    std::string content1;
    if (!COMMON::ReadFile("/proc/stat", validPaths, content1)) {
        return false;
    }

    unsigned long total1 = 0;
    unsigned long idle1 = 0;
    if (!ParseCpuStats(content1, total1, idle1)) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 50ms 间隔

    std::string content2;
    if (!COMMON::ReadFile("/proc/stat", validPaths, content2)) {
        return false;
    }

    unsigned long total2 = 0;
    unsigned long idle2 = 0;
    if (!ParseCpuStats(content2, total2, idle2)) {
        return false;
    }

    if ((total2 <= total1) || (total2 - total1 == 0)) {
        return false;
    }

    unsigned long totalDiff = total2 - total1;
    unsigned long idleDiff = idle2 - idle1;

    double usage = 1.0 - (static_cast<double>(idleDiff) / static_cast<double>(totalDiff));

    usage = std::clamp(usage, 0.0, 1.0);

    *cpuUsage = usage;
    return true;
}

bool NativeMemoryProfilerSaService::IsNativeDeamonRunning()
{
    int nativeDaemonPid = 0;
    return COMMON::IsProcessExist("native_daemon", nativeDaemonPid);
}

void NativeMemoryProfilerSaService::NotifyProfilingResult(const std::shared_ptr<TaskConfig>& config,
    int32_t errorCode, const std::string& filePath)
{
    if (config == nullptr || config->callback == nullptr) {
        return;
    }
    NativeMemoryProfilerSaCallbackProxy callback(config->callback);
    callback.OnProfilingResult(errorCode, filePath);
}

int32_t NativeMemoryProfilerSaService::CopyApiCollectorFile(uint32_t pid, std::shared_ptr<TaskConfig> config,
    std::string& resultPath)
{
    std::lock_guard<std::mutex> guard(g_fileNameMtx);
    auto pidInfoIter = g_pidInfo.find(pid);
    auto fileNameInfoIter = g_fileNameInfo.find(pid);
    if (pidInfoIter == g_pidInfo.end() || fileNameInfoIter == g_fileNameInfo.end()) {
        PROFILER_LOG_WARN(LOG_CORE, "CopyApiCollectorFile: api task file info missing");
    } else {
        std::string destPath = COMMON::GetFullTracePath(pidInfoIter->second.first, pid, config->processName,
                                                        fileNameInfoIter->second);
        if (destPath.empty()) {
            PROFILER_LOG_ERROR(LOG_CORE, "CopyApiCollectorFile: invalid destPath");
            return RET_ERR;
        }
        if (!COMMON::CopyFileFast(config->fd, destPath)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Copy file into bundle failed");
            return RET_ERR;
        } else {
            resultPath = API_SANDBOX_PATH_PREFIX + fileNameInfoIter->second;
        }
    }
    g_fileNameInfo.erase(pid);
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::ReleaseForStopHook(uint32_t pid, std::shared_ptr<TaskConfig> config,
    std::string& resultPath, bool isAsync)
{
    int32_t ret = RET_OK;
    if (config->hookMgr->GetHookConfig().use_file_cache_mode()) {
        config->hookMgr->RequestCacheFileStop(static_cast<pid_t>(pid));
        SendHookCommand(static_cast<pid_t>(pid), STOP_HOOK_CMD);
    }
    config->hookMgr->StopPluginSession({});
    CloseSocketFd(static_cast<pid_t>(pid));
    config->hookMgr->DestroyPluginSession({});
    if (isAsync) {
        std::lock_guard<std::mutex> guard(g_mtx);
        g_nameAndFilePathCtx.erase(config->processName);
        g_nameAndFilePathCtx.erase(config->filePath);
        if (config->startType == START_TYPE_API) {
            ret = CopyApiCollectorFile(pid, config, resultPath);
        }
        g_pidCtx.erase(config->pid);
        g_pidInfo.erase(static_cast<pid_t>(pid));
    } else {
        g_nameAndFilePathCtx.erase(config->processName);
        g_nameAndFilePathCtx.erase(config->filePath);
        if (config->startType == START_TYPE_API) {
            ret = CopyApiCollectorFile(pid, config, resultPath);
        }
        g_pidCtx.erase(config->pid);
        g_pidInfo.erase(static_cast<pid_t>(pid));
    }
    {
        std::lock_guard<std::mutex> nmdGuard(g_nmdMtx);
        if (g_nmdPidType.find(config->pid) != g_nmdPidType.end()) {
            g_nmdPidType.erase(config->pid);
        }
    }
    if (config->isStartupMode) {
        g_hasStartupMode = false;
    }
    if (config->fd > 0) {
        close(config->fd);
    }
    return ret;
}

} // namespace OHOS::Developtools::NativeDaemon
