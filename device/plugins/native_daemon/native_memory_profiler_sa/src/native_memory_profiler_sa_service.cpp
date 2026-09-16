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
#include "client_connection.h"
#include "hook_common.h"
#include "init_param.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "common.h"
#include "logging.h"
#include "native_memory_profiler_sa_callback_proxy.h"
#include <sys/socket.h>
#include <unistd.h>
#include "apm_collect_mgr.h"
#include "htrace_collector.h"
#include "rawheap_collector.h"
#include "jsmap_collector.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr int32_t TIME_BASE = 1000;
const std::string FILE_PATH_HEAD = "/data/local/tmp/native_memory_";
const std::string FILE_PATH_TAIL = ".htrace";
const std::string COMPOSITE_HEAP_PATH_HEAD = "/data/local/tmp/composite_heap_";
const std::string COMPOSITE_HEAP_PATH_TAIL = ".zip";
const std::string PRODUCT_CONFIG_PATH = "/sys_prod/etc/hiview/hiprofiler/hiprofiler_cfg.json";
constexpr int32_t DELAYED_SHUTDOWN_TIME = 20;
constexpr int FILE_MODE = 0644;
constexpr int32_t SIMP_NMD = 3;
[[maybe_unused]] constexpr int32_t NMD_WAIT_MS = 5000;
constexpr int VEC_SHIFT = 2;
constexpr double MIN_MEM_ROM_PERCENT = 0.03;
constexpr uint64_t MIN_MEM_AVAILABLE_KB = 2ULL * 1024 * 1024; // 2GB, /proc/meminfo unit is kB
constexpr uint64_t MIN_FREE_ROM_BYTES = 15ULL * 1024 * 1024 * 1024; // 15GB
constexpr double MAX_MEM_PROFILER_CPU_PERCENT = 0.7;
const std::string API_SANDBOX_PATH_PREFIX = "/data/storage/el2/base/files/";
const std::string DAILY_COUNT_DIR = "/data/log/hiprofiler/";
const std::string MEM_PROFILER_QUOTA_FILE_NAME = "profiling_rate_limits_quota";
constexpr int HOURS_PER_DAY = 24;
constexpr int32_t MAX_MEM_PROFILER_USE_COUNT_PER_DAY = 2;
constexpr int32_t MAX_MEM_PROFILER_TOTAL_USE_COUNT_PER_DAY = 4;
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
static std::unordered_map<int32_t, int32_t> g_timerFdInfo;
static std::mutex g_timerFdMtx;
//2: double
static std::mutex g_nmdMtx;
static std::unordered_set<uint32_t> g_stoppingPids;
static std::mutex g_stoppingPidsMtx;
static std::mutex g_countFileMutex;
static std::mutex g_socketSendMtx;
#ifdef NATIVE_DAEMON_UNITTEST
static bool g_mockIsUserMode = false;
static bool g_mockPermissionGranted = true;
static bool g_mockNativeDaemonRunning = false;
static bool g_mockStartServiceAbility = true;
static bool g_mockMemFail = false;
static bool g_mockCpuFail = false;
static bool g_mockRomFail = false;
static bool g_mockSendSuccess = true;
static std::string g_mockNmdResult;
constexpr uint64_t MOCK_ROM_TOTAL_BYTES = 100ULL * 1024 * 1024 * 1024; // 100GB
#endif
#ifdef ENABLE_SA_AUTO_START
const bool G_REGISTER_RESUTL = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance().get());
#endif

std::shared_ptr<ClientConnection> GetClientConnectionByPid(pid_t pid)
{
    CHECK_NOTNULL(g_serviceEntry, nullptr, "service Entry is NULL");
    std::unique_lock<std::mutex> lock(g_pidFdMtx);
    auto it = g_pidFds.find(pid);
    CHECK_TRUE(it != g_pidFds.end() && it->second >= 0, nullptr, "fd not found for pid:%d", static_cast<int32_t>(pid));
    int sockFd = it->second;
    lock.unlock();
    return g_serviceEntry->GetClientConnection(sockFd);
}

bool SendFileCacheFd(SocketContext &context, HookManager& hookMgr, int32_t pid, uint16_t fileIndex)
{
    int cacheFd = hookMgr.CreateFileCacheFd();
    if (cacheFd < 0) {
        return false;
    }
    HookCacheFileMsg cacheFileMsg(pid, fileIndex, CACHE_FILE_CREATED);
    std::unique_lock<std::mutex> sendLock(g_socketSendMtx);
    bool ret = context.SendHookConfig(reinterpret_cast<uint8_t*>(&cacheFileMsg), sizeof(cacheFileMsg)) &&
        context.SendFileDescriptor(cacheFd);
    sendLock.unlock();
    hookMgr.RotateCacheFdOnReturn(pid, fileIndex, cacheFd);
    close(cacheFd);
    return ret;
}

bool SendHookConfigMsg(pid_t pid, const uint8_t *msg, size_t size)
{
#ifdef NATIVE_DAEMON_UNITTEST
    if (g_mockSendSuccess) {
        return true;
    }
#endif
    std::shared_ptr<ClientConnection> connection = GetClientConnectionByPid(pid);
    CHECK_NOTNULL(connection, false, "connection not found for pid:%d", static_cast<int32_t>(pid));
    std::lock_guard<std::mutex> sendLock(g_socketSendMtx);
    CHECK_TRUE(connection->SendHookConfig(msg, size), false, "send hook config msg fail");
    return true;
}

bool SendHookCommand(pid_t pid, uint32_t cmd)
{
    return SendHookConfigMsg(pid, reinterpret_cast<const uint8_t *>(&cmd), sizeof(cmd));
}

bool SendInitialFileCacheFds(SocketContext &context, HookManager& hookMgr, int32_t pid, const std::string& name)
{
    if (!hookMgr.PrepareFileCacheContext(pid, name)) {
        return false;
    }
    std::lock_guard<std::mutex> sendLock(g_socketSendMtx);
    for (uint16_t fileIndex = 0; fileIndex < FILE_CACHE_FD_COUNT; ++fileIndex) {
        int cacheFd = hookMgr.CreateFileCacheFd();
        if (cacheFd < 0) {
            return false;
        }
        bool ret = context.SendFileDescriptor(cacheFd);
        hookMgr.SaveInitialCacheFds(pid, fileIndex, cacheFd);
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
    std::lock_guard<std::mutex> sendLock(g_socketSendMtx);
    bool ret = context.SendFileDescriptor(fd);
    PROFILER_LOG_INFO(LOG_CORE, "SendMemLeakFd sent fd %d", fd);
    return ret;
}

bool SendDumpSimplifiedNmdRequest(pid_t pid, uint32_t cmd, int recvFd)
{
    std::shared_ptr<ClientConnection> connection = GetClientConnectionByPid(pid);
    CHECK_NOTNULL(connection, false, "connection not found for pid:%d", static_cast<int32_t>(pid));
    std::lock_guard<std::mutex> sendLock(g_socketSendMtx);
    CHECK_TRUE(connection->SendHookConfig(reinterpret_cast<const uint8_t *>(&cmd), sizeof(cmd)), false, "send fail");
    CHECK_TRUE(connection->SendFileDescriptor(recvFd), false, "send recvFd fail");
    PROFILER_LOG_INFO(LOG_CORE, "send dump simplified nmd request success: %d", static_cast<int32_t>(pid));
    return true;
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

void GetPeerUcred(SocketContext &context, struct ucred &cred)
{
    int socketHandle = context.GetSocketHandle();
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(socketHandle, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetPeerUcred getsockopt failed");
        return;
    }
}

bool HandleMemLeakDoneBySocket(SocketContext &context)
{
    int32_t pid = FindPidBySocket(context.GetSocketHandle());
    if (pid <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc mem leak done without pid");
        return true;
    }
    struct ucred cred;
    GetPeerUcred(context, cred);
    if (pid != static_cast<int32_t>(cred.pid)) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleMemLeakDoneBySocket check client pid failed");
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

bool HandleInHookSimpNmdDoneBySocket(SocketContext &context)
{
    int32_t pid = FindPidBySocket(context.GetSocketHandle());
    if (pid <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc in hook simp nmd done without pid");
        return true;
    }
    struct ucred cred;
    GetPeerUcred(context, cred);
    if (pid != static_cast<int32_t>(cred.pid)) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleInHookSimpNmdDoneBySocket check client pid failed");
        return true;
    }
    std::lock_guard<std::mutex> guard(g_mtx);
    auto iter = g_pidCtx.find(pid);
    if (iter == g_pidCtx.end() || iter->second == nullptr || iter->second->hookMgr == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc simp nmd done without hook manager");
        return true;
    }
    if (!iter->second->hookMgr->HandleInHookSimpNmdDone()) {
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
    bool isAsync = false;
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
        isAsync = config->callback != nullptr;
        name = config->processName;
    }
    // Note: The same mutex g_mtx is also used in the StopHook function.
    // Be cautious to avoid potential deadlocks.
    StopHookByType(pidval, name, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS), isAsync, true);
}

bool NativeMemoryProfilerSaService::StartServiceAbility()
{
#ifdef NATIVE_DAEMON_UNITTEST
    return g_mockStartServiceAbility;
#else
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
#endif
}

bool NativeMemoryProfilerSaService::HasProfilingPermission()
{
#ifdef NATIVE_DAEMON_UNITTEST
    if (g_mockIsUserMode) {
        return g_mockPermissionGranted;
    }
    return true;
#else
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
#endif
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
    if (config->startType_ != START_TYPE_API && !HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed, no profiling permission!");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", GetCmdArgs(config),
            COMMON::ErrorType::RET_NO_PERMISSION, "no profiling permission");
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
    if (config->printNmdInHook_) {
        return PrintSimplifiedNmdInHook(config, reply);
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

int32_t NativeMemoryProfilerSaService::Update(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    if (config->startType_ != START_TYPE_API && !HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Update failed, no profiling permission!");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", GetCmdArgs(config),
            COMMON::ErrorType::RET_NO_PERMISSION, "no profiling permission");
        return RET_ERR;
    }
    if (config->startType_ == START_TYPE_API &&
        config->pid_ != static_cast<int32_t>(IPCSkeleton::GetCallingPid())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Update failed, api pid mismatch!");
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }

    std::shared_ptr<SimplifiedMemConfig> updateConfig = std::make_shared<SimplifiedMemConfig>();
    updateConfig->largestSize = config->largestSize_;
    updateConfig->secondLargestSize = config->secondLargestSize_;
    updateConfig->maxGrowthSize = config->maxGrowthSize_;
    updateConfig->sampleSize = config->sampleInterval_;
    updateConfig->asyncStackEnable =  config->asyncStackEnable_;
    updateConfig->asyncFlag = config->asyncFlag_;
    updateConfig->targetSoName = config->targetSoName_;
    updateConfig->responseLibraryMode = config->responseLibraryMode_;
    updateConfig->nestedAsyncStackEnable = config->nestedAsyncStackEnable_;
    updateConfig->asyncNestingDepth = config->asyncNestingDepth_;
    updateConfig->asyncTaskStackDepth = config->asyncTaskStackDepth_;
    return UpdateSimplifiedMemConfig(config->pid_, updateConfig);
}

int32_t NativeMemoryProfilerSaService::UpdateSimplifiedMemConfig(
    pid_t pid, std::shared_ptr<SimplifiedMemConfig>& config)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    auto taskIter = g_pidCtx.find(pid);
    if (taskIter == g_pidCtx.end()) {
        PROFILER_LOG_ERROR(LOG_CORE, "hook task of pid(%d) not started", pid);
        return RET_ERR;
    }
    std::shared_ptr<TaskConfig> task = taskIter->second;
    if (!task || !task->hookMgr) {
        PROFILER_LOG_ERROR(LOG_CORE, "hook task of pid(%d) is invalid", pid);
        return RET_ERR;
    }
    CHECK_TRUE(CheckCallingPermission(static_cast<int32_t>(IPCSkeleton::GetCallingPid()), task),
        RET_ERR, "CheckCallingPermission fail");
    std::shared_ptr<SimplifiedMemConfig> origin = std::make_shared<SimplifiedMemConfig>();
    task->hookMgr->GetSimplifiedMemConfig(origin);
    task->hookMgr->SetSimplifiedMemConfig(config);
    SimplifiedMemConfigMsg msg;
    msg.pid = static_cast<int32_t>(pid);
    msg.largestSize = config->largestSize;
    msg.secondLargestSize = config->secondLargestSize;
    msg.maxGrowthSize = config->maxGrowthSize;
    msg.sampleSize = config->sampleSize;
    if (!SendHookConfigMsg(pid, reinterpret_cast<const uint8_t *>(&msg), sizeof(msg))) {
        task->hookMgr->SetSimplifiedMemConfig(origin);
        PROFILER_LOG_ERROR(LOG_CORE, "send notify nook config change cmd fail");
        return RET_ERR;
    }
    PROFILER_LOG_INFO(LOG_CORE, "hook tast of pid(%d) udate simplified mem config done", pid);
    return RET_OK;
}

bool NativeMemoryProfilerSaService::IsMemoryPressureCritical(uint64_t availableMem, bool memReadOk)
{
    if (!memReadOk) {
        return true;
    }
    return availableMem < MIN_MEM_AVAILABLE_KB;
}

bool NativeMemoryProfilerSaService::IsCpuOverloaded(double cpuUsage, bool cpuReadOk)
{
    if (!cpuReadOk) {
        return true;
    }
    return cpuUsage > MAX_MEM_PROFILER_CPU_PERCENT;
}

bool NativeMemoryProfilerSaService::IsRomInsufficient(uint64_t freeBytes, uint64_t totalBytes, bool romReadOk)
{
    if (!romReadOk) {
        return true;
    }
    uint64_t percentThreshold = static_cast<uint64_t>(static_cast<double>(totalBytes) * MIN_MEM_ROM_PERCENT);
    uint64_t threshold = std::max(percentThreshold, MIN_FREE_ROM_BYTES);
    return freeBytes < threshold;
}

int32_t NativeMemoryProfilerSaService::CheckSystemResource()
{
    const int oneHundred = 100;
    MemoryInfo memInfo;
#ifdef NATIVE_DAEMON_UNITTEST
    bool memOk = !g_mockMemFail;
    if (memOk) { memInfo.availableMem = MIN_MEM_AVAILABLE_KB + 1; }
#else
    bool memOk = GetSystemMemoryInfo(&memInfo);
#endif
    if (IsMemoryPressureCritical(memInfo.availableMem, memOk)) {
        PROFILER_LOG_ERROR(LOG_CORE, "System memory is too low: %llu KB",
            static_cast<unsigned long long>(memInfo.availableMem));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL);
    }
    double cpuUsage = 0.0;
#ifdef NATIVE_DAEMON_UNITTEST
    bool cpuOk = !g_mockCpuFail;
#else
    bool cpuOk = GetSystemCpuUsageInfo(&cpuUsage);
#endif
    if (IsCpuOverloaded(cpuUsage, cpuOk)) {
        PROFILER_LOG_ERROR(LOG_CORE, "System CPU usage is too high: %.2f%%", cpuUsage * oneHundred);
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED);
    }
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
#ifdef NATIVE_DAEMON_UNITTEST
    bool romOk = !g_mockRomFail;
    if (romOk) {
        totalBytes = MOCK_ROM_TOTAL_BYTES;
        freeBytes = MOCK_ROM_TOTAL_BYTES;
    }
#else
    bool romOk = COMMON::GetRomSpaceInfo(COMMON::DEFAULT_RW_DIR, totalBytes, freeBytes);
#endif
    if (IsRomInsufficient(freeBytes, totalBytes, romOk)) {
        PROFILER_LOG_ERROR(LOG_CORE, "System ROM is too low: %llu bytes",
            static_cast<unsigned long long>(freeBytes));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STORAGE_PRESSURE_CRITICAL);
    }
    return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS);
}

int32_t NativeMemoryProfilerSaService::PreCheckPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
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
    return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS);
}

int32_t NativeMemoryProfilerSaService::StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
    const sptr<IRemoteObject>& callback)
{
#ifdef NATIVE_DAEMON_UNITTEST
    bool isDebugApp = false;
#else
    bool isDebugApp = COMMON::CheckApplicationPermission(config->pid_, config->processName_);
#endif
    if (!isDebugApp) {
        std::lock_guard<std::mutex> lock(g_countFileMutex);
        std::unordered_map<std::string, QuotaRecord> records;
        if (!ReadQuotaFile(records)) {
            PROFILER_LOG_ERROR(LOG_CORE, "StartPidVoid: failed to read quota file");
            return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
        }
        if (GetMemProfilerUseCountToday(records, config->bundleName_) >= MAX_MEM_PROFILER_USE_COUNT_PER_DAY) {
            PROFILER_LOG_ERROR(LOG_CORE, "Daily use count exceeded,errorcode is:%d.",
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED));
            return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED);
        }
        if (GetMemProfilerTotalUseCountToday(records) >= MAX_MEM_PROFILER_TOTAL_USE_COUNT_PER_DAY) {
            PROFILER_LOG_ERROR(LOG_CORE, "Global daily use count exceeded,errorcode is:%d.",
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED));
            return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED);
        }
        if (!UpdateMemProfilerUserCount(records, config->bundleName_)) {
            return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
        }
        if (!WriteQuotaFile(records)) {
            PROFILER_LOG_ERROR(LOG_CORE, "StartPidVoid: failed to write quota file");
            return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
        }
    }
    int32_t retPreCheck = PreCheckPidVoid(config);
    if (retPreCheck != static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS)) {
        if (!RollbackMemProfilerUserCount(isDebugApp, config->bundleName_)) {
            PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaService: rollback count failed");
        }
        return retPreCheck;
    }
    auto args = GetCmdArgs(config);
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    uint32_t fd = 0;
    int32_t retHookLock = StartHookLock(config, fd, hook, args, callback);
    if (retHookLock != RET_OK && !RollbackMemProfilerUserCount(isDebugApp, config->bundleName_)) {
        PROFILER_LOG_WARN(LOG_CORE, "NativeMemoryProfilerSaService: rollback count failed");
    }
    return retHookLock;
}

// only for API
int32_t NativeMemoryProfilerSaService::Stop()
{
    int32_t retStopHook = StopHookAsync(static_cast<uint32_t>(IPCSkeleton::GetCallingPid()),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
    PROFILER_LOG_INFO(LOG_CORE, "StopHookAsync ret:%d", retStopHook);
    if (retStopHook != RET_OK) {
        return retStopHook;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StopSync()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    if (pid <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "invalid pid: %d", pid);
        return RET_ERR;
    }
    int32_t retStopHook = StopHook(static_cast<uint32_t>(pid), "",
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
    PROFILER_LOG_INFO(LOG_CORE, "StopHook ret:%d", retStopHook);
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
    std::string name;
    (void)StopHookByType(pid, name, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS), IsAsyncRequest(pid));
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
        return RET_ERR;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::DumpDataAsync(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
    const sptr<IRemoteObject>& callback)
{
    if (config == nullptr || callback == nullptr) {
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
    RefreshFileCacheModeConfig(config);
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    if (StartHookLock(config, fd, hook, args, callback) != RET_OK) {
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
        if (g_serviceEntry != nullptr) {
            g_serviceEntry->RemoveContext(socketFd);
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService::CloseSocketFd pid invalid");
    }
}

int32_t NativeMemoryProfilerSaService::StopHook(uint32_t pid, std::string name, int32_t finishErrorCode, bool innerCall)
{
    std::unique_lock<std::mutex> lock(g_mtx);
    std::shared_ptr<TaskConfig> config = nullptr;
    if (pid > 0) {
        if (auto taskIter = g_pidCtx.find(pid); taskIter != g_pidCtx.end()) {
            config = taskIter->second;
        }
    } else if (auto taskIter = g_nameAndFilePathCtx.find(name); taskIter != g_nameAndFilePathCtx.end()) {
        config = taskIter->second;
        pid = static_cast<pid_t>(config->hookMgr->GetPid());
    }
    lock.unlock();
    if (config == nullptr) {
        int32_t ret = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED);
        PROFILER_LOG_ERROR(LOG_CORE,
            "NativeMemoryProfilerSaService::hook has stop, pid: %d, process name: %s, errorcode is:%d.",
            pid, name.c_str(), ret);
        return ret;
    }
    if (!innerCall && !CheckCallingPermission(static_cast<int32_t>(IPCSkeleton::GetCallingPid()), config)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Permission Denied,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }
    if (!TryBeginStopHook(pid)) {
        PROFILER_LOG_INFO(LOG_CORE, "StopHook: pid %d is already stopping, skip.", pid);
        return RET_OK;
    }
    int32_t stopResult = StopHookTask(pid, config, finishErrorCode);
    EndStopHook(pid);
    return stopResult;
}

int32_t NativeMemoryProfilerSaService::StopHookTask(uint32_t pid, std::shared_ptr<TaskConfig> config,
    int32_t finishErrorCode)
{
    {
        std::lock_guard<std::mutex> guard(g_timerFdMtx);
        auto fdTimerIter = g_timerFdInfo.find(static_cast<int32_t>(pid));
        if (fdTimerIter != g_timerFdInfo.end()) {
            // delete schedule task.
            g_scheduleTaskManager.UnscheduleTask(fdTimerIter->second);
            g_timerFdInfo.erase(fdTimerIter);
        }
    }
    std::string resultPath;
    int32_t retRelease = ReleaseForStopHook(pid, config, resultPath);
    int32_t callbackError = retRelease == RET_OK ? finishErrorCode :
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR);
    NotifyProfilingResult(config, callbackError, retRelease == RET_OK ? resultPath : "");

    std::lock_guard<std::mutex> guard(g_mtx);
    if (--g_taskNum == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "StringViewMemoryHold clear");
        StringViewMemoryHold::GetInstance().Clear();
        DelayedShutdown(false);
    }
    return retRelease;
}

int32_t NativeMemoryProfilerSaService::StopHookAsync(uint32_t pid, int32_t finishErrorCode, bool innerCall)
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
    if (!innerCall && !CheckCallingPermission(static_cast<int32_t>(IPCSkeleton::GetCallingPid()), config)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Permission Denied,errorcode is:%d.",
            static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
        return static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED);
    }

    if (!TryBeginStopHook(pid)) {
        PROFILER_LOG_INFO(LOG_CORE, "StopHookAsync: pid %d is already stopping, skip.", pid);
        return RET_OK;
    }

    std::thread([pid, config, finishErrorCode]() {
        StopHookTask(pid, config, finishErrorCode);
        EndStopHook(pid);
    }).detach();

    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StopHookByType(uint32_t pid, std::string& name, int32_t finishErrorCode,
    bool isAsync, bool innerCall)
{
    if (isAsync) {
        return StopHookAsync(pid, finishErrorCode, innerCall);
    }
    return StopHook(pid, name, finishErrorCode, innerCall);
}

bool NativeMemoryProfilerSaService::TryBeginStopHook(uint32_t pid)
{
    std::lock_guard<std::mutex> guard(g_stoppingPidsMtx);
    if (g_stoppingPids.find(pid) != g_stoppingPids.end()) {
        return false;
    }
    g_stoppingPids.insert(pid);
    return true;
}
 	 
void NativeMemoryProfilerSaService::EndStopHook(uint32_t pid)
{
    std::lock_guard<std::mutex> guard(g_stoppingPidsMtx);
    g_stoppingPids.erase(pid);
}

bool NativeMemoryProfilerSaService::IsAsyncRequest(uint32_t pid)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    std::shared_ptr<TaskConfig> config = nullptr;
    if (auto taskIter = g_pidCtx.find(pid); taskIter != g_pidCtx.end()) {
        config = taskIter->second;
    }
    bool async = config && config->callback;
    PROFILER_LOG_INFO(LOG_CORE, "pid %d hasCallback:%d", pid, async);
    return async;
}

bool NativeMemoryProfilerSaService::CheckCallingPermission(int32_t callingPid, std::shared_ptr<TaskConfig>& config)
{
    if (callingPid != config->callingPid) {
        PROFILER_LOG_ERROR(LOG_CORE, "calling pid check fail, (%d, %d)", config->callingPid, callingPid);
        return false;
    }
    return true;
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
    args << "nested_async_stack_enable: " << (config->nestedAsyncStackEnable_ ? "true" : "false") << ", ";
    args << "async_nesting_depth: " << std::to_string(config->asyncNestingDepth_) << ", ";
    args << "async_task_stack_depth: " << std::to_string(config->asyncTaskStackDepth_) << ", ";
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
    bool compositeHeapEnable = config->traceMask_ & RES_COMPOSITE_HEAP;
    uint32_t originFd = fd;
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
    if (fd > 0 && compositeHeapEnable) {
        std::string filePathStr = (config->pid_ > 0) ? std::to_string(config->pid_) : config->processName_;
        config->filePath_ = FILE_PATH_HEAD + filePathStr + FILE_PATH_TAIL;
    }
    if (fd == 0 || compositeHeapEnable) {
        auto retFile = COMMON::CheckNotExistsFilePathSafe(config->filePath_);
        if (!retFile.first) {
            PROFILER_LOG_INFO(LOG_CORE, "%s:check file path %s fail", __func__, config->filePath_.c_str());
            COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_INVALID_PATH,
                "check file path failed");
            return RET_ERR;
        }
        int fdTemp = open(retFile.second.c_str(), O_RDWR | O_CREAT, FILE_MODE);
        if (config->startType_ == START_TYPE_API && !compositeHeapEnable) {
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
    bool isAsync = callback != nullptr;
    int32_t timerFd = g_scheduleTaskManager.ScheduleTask(
        [config, filePath = config->filePath_, isAsync] {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService:This task will be stopped.errorcode is:%d.",
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_AUTO_STOPPED_BY_DURATION));
            int32_t retStop = StopHookByType(config->pid_, config->processName_,
                static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_AUTO_STOPPED_BY_DURATION),
                isAsync, true);
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

    std::shared_ptr<TaskConfig> configCtx = std::make_shared<TaskConfig>(hook, config->pid_, config->processName_,
        static_cast<int32_t>(IPCSkeleton::GetCallingPid()), config->filePath_, timerFd, config->startupMode_, fd,
        config->startType_);
    configCtx->callback = callback;
    configCtx->compositeHeapEnable = compositeHeapEnable;
    configCtx->typeFileName = config->typeFileName_;
    if (compositeHeapEnable && originFd > 0) {
        configCtx->compositeHeapFd = originFd;
    }
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
#ifdef NATIVE_DAEMON_UNITTEST
        std::string simplifiedNmdResult = g_mockNmdResult;
#else
        std::string simplifiedNmdResult = hook->GetSimplifiedNmdResult(NMD_WAIT_MS);
#endif
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
    if (COMMON::CheckApplicationPermission(config->pid_, config->processName_)) {
        PROFILER_LOG_INFO(LOG_CORE, "debug app skip task limit, pid: %d", config->pid_);
        return RET_OK;
    }
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
    if (auto iter = g_pidCtx.find(pid); iter != g_pidCtx.end()) {
        if (iter->second == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "FillTaskConfigContext: null task for pid %d", pid);
            return;
        }
        iter->second->processName = name;
        g_nameAndFilePathCtx[name] = iter->second;
        if (iter->second->isStartupMode) {
            g_hasStartupMode = false;
        }
    } else if (auto it = g_nameAndFilePathCtx.find(name); it != g_nameAndFilePathCtx.end()) {
        if (it->second == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "FillTaskConfigContext: null task for name %s", name.c_str());
            return;
        }
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
    struct ucred cred;
    GetPeerUcred(context, cred);
    if (cacheFileMsg.hookPid != cred.pid) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReceiveCacheFileMsg check client pid failed");
        return false;
    }

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
                if (!iter->second->hookMgr->IsCacheFileStopRequested(cacheFileMsg.hookPid)) {
                    if (!SendFileCacheFd(context, *(iter->second->hookMgr), cacheFileMsg.hookPid,
                        cacheFileMsg.fileIndex)) {
                        PROFILER_LOG_ERROR(LOG_CORE, "ReceiveCacheFileMsg replenish fd failed");
                        return false;
                    }
                } else {
                    iter->second->hookMgr->RotateCacheFdOnReturn(cacheFileMsg.hookPid, cacheFileMsg.fileIndex, -1);
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
    if (static_cast<uint32_t>(peerConfig) == DUMP_SIMP_NMD_DONE_CMD) {
        return HandleInHookSimpNmdDoneBySocket(context);
    }
    if (peerConfig == -1) {
        return false;
    }
    struct ucred cred;
    GetPeerUcred(context, cred);
    if (static_cast<pid_t>(peerConfig) != cred.pid) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc check client pid failed");
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

    std::lock_guard<std::mutex> guard(g_mtx);
    bool startupMode = g_hasStartupMode;
    FillTaskConfigContext(peerConfig, bundleName); // Save the relevant context for subsequent inspection
    if (auto iter = g_pidCtx.find(peerConfig); iter != g_pidCtx.end()) {
        if (!startupMode) {
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
    memInfo->availableMem = availableMem;
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
#ifdef NATIVE_DAEMON_UNITTEST
    return g_mockNativeDaemonRunning;
#else
    int nativeDaemonPid = 0;
    return COMMON::IsProcessExist("native_daemon", nativeDaemonPid);
#endif
}

void NativeMemoryProfilerSaService::NotifyProfilingResult(const std::shared_ptr<TaskConfig>& config,
    int32_t errorCode, const std::string& filePath)
{
    if (config == nullptr || config->callback == nullptr) {
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "NotifyProfilingResult pid:%d, startType:%u", config->pid, config->startType);
    NativeMemoryProfilerSaCallbackProxy callback(config->callback);
    callback.OnProfilingResult(config->pid, errorCode, filePath);
}

std::string NativeMemoryProfilerSaService::CollectApm(std::shared_ptr<TaskConfig>& config)
{
    std::string filePathStr = (config->pid > 0) ? std::to_string(config->pid) : config->processName;
    std::string compositeFilepath = COMPOSITE_HEAP_PATH_HEAD + filePathStr + COMPOSITE_HEAP_PATH_TAIL;
    uint32_t pid = static_cast<uint32_t>(config->pid);
    std::string traceFilepath = config->filePath;
    ApmCollectMgr apmMgr(compositeFilepath);
    apmMgr.RegisterCollector(std::static_pointer_cast<IDataCollector>(std::make_shared<RawheapCollector>(pid, pid)));
    apmMgr.RegisterCollector(std::static_pointer_cast<IDataCollector>(std::make_shared<JsmapCollector>(pid, pid)));
    apmMgr.RegisterCollector(std::static_pointer_cast<IDataCollector>(std::make_shared<TraceCollector>(traceFilepath)));
    return apmMgr.Collect() ? compositeFilepath : "";
}

int32_t NativeMemoryProfilerSaService::CopyApiCollectorFile(uint32_t pid, std::shared_ptr<TaskConfig> config,
    std::string& resultPath)
{
    if (config->typeFileName.empty()) {
        if (config->compositeHeapEnable) {
            PROFILER_LOG_ERROR(LOG_CORE, "CopyApiCollectorFile: api composite task file info missing");
            return RET_ERR;
        } else {
            PROFILER_LOG_WARN(LOG_CORE, "CopyApiCollectorFile: api task file info missing");
            return RET_OK;
        }
    }
    auto pidInfoIter = g_pidInfo.find(pid);
    if (pidInfoIter == g_pidInfo.end()) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyApiCollectorFile: api task uid info missing");
        return RET_ERR;
    }
    std::string destPath = COMMON::GetFullTracePath(
        pidInfoIter->second.first, pid, config->processName, config->typeFileName);
    if (destPath.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyApiCollectorFile: invalid destPath");
        return RET_ERR;
    }
    if (config->compositeHeapEnable) {
        std::string compositeFilepath = CollectApm(config);
        if (compositeFilepath.empty()) {
            PROFILER_LOG_ERROR(LOG_CORE, "Collect composite heap fail");
            return RET_ERR;
        }
        if (!COMMON::CopyFileFastFromPath(compositeFilepath, destPath)) {
            (void)unlink(compositeFilepath.c_str());
            PROFILER_LOG_ERROR(LOG_CORE, "Copy composite file into bundle failed");
            return RET_ERR;
        }
        (void)unlink(compositeFilepath.c_str());
    } else {
        if (!COMMON::CopyFileFast(config->fd, destPath)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Copy file into bundle failed");
            return RET_ERR;
        }
    }
    resultPath = API_SANDBOX_PATH_PREFIX + config->typeFileName;
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::ReleaseForStopHook(uint32_t pid, std::shared_ptr<TaskConfig> config,
    std::string& resultPath)
{
    int32_t ret = RET_OK;
    if (config->hookMgr->GetHookConfig().use_file_cache_mode()) {
        config->hookMgr->RequestCacheFileStop(static_cast<pid_t>(pid));
        SendHookCommand(static_cast<pid_t>(pid), STOP_HOOK_CMD);
    }
    config->hookMgr->StopPluginSession({});
    CloseSocketFd(static_cast<pid_t>(pid));
    config->hookMgr->DestroyPluginSession({});
    {
        std::lock_guard<std::mutex> guard(g_mtx);
        g_nameAndFilePathCtx.erase(config->processName);
        g_nameAndFilePathCtx.erase(config->filePath);
        if (config->startType == START_TYPE_API) {
            ret = CopyApiCollectorFile(pid, config, resultPath);
        } else if (config->compositeHeapEnable && config->compositeHeapFd > 0) {
            std::string compositeFilepath = CollectApm(config);
            if (compositeFilepath.empty()) {
                PROFILER_LOG_ERROR(LOG_CORE, "Collect composite heap fail");
                return RET_ERR;
            }
            if (!COMMON::CopyFileToFdFastFromPath(compositeFilepath, config->compositeHeapFd)) {
                (void)unlink(compositeFilepath.c_str());
                PROFILER_LOG_ERROR(LOG_CORE, "Copy composite file into bundle fd failed");
                return RET_ERR;
            }
            (void)unlink(compositeFilepath.c_str());
        }
        g_pidCtx.erase(config->pid);
        g_pidInfo.erase(static_cast<pid_t>(pid));

        std::lock_guard<std::mutex> nmdGuard(g_nmdMtx);
        if (g_nmdPidType.find(config->pid) != g_nmdPidType.end()) {
            g_nmdPidType.erase(config->pid);
        }

        if (config->isStartupMode) {
            g_hasStartupMode = false;
        }
    }
    if (config->fd > 0) {
        close(config->fd);
    }
    return ret;
}

bool NativeMemoryProfilerSaService::ReadQuotaFile(std::unordered_map<std::string, QuotaRecord>& records)
{
    records.clear();
    const std::string filepath = std::string(DAILY_COUNT_DIR) + std::string(MEM_PROFILER_QUOTA_FILE_NAME);
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return true;
    }
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(filepath.c_str(), canonicalPath) == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadQuotaFile: failed to get canonical path");
        return false;
    }
    std::ifstream ifs(canonicalPath);
    if (!ifs.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadQuotaFile: failed to open file");
        return false;
    }
    std::string name;
    long long timestamp = 0;
    int32_t count = 0;
    while (ifs >> name >> timestamp >> count) {
        records[name] = {timestamp, count};
    }
    ifs.close();
    return true;
}

bool NativeMemoryProfilerSaService::WriteQuotaFile(const std::unordered_map<std::string, QuotaRecord>& records)
{
    const std::string filepath = std::string(DAILY_COUNT_DIR) + std::string(MEM_PROFILER_QUOTA_FILE_NAME);
    std::string tpl = filepath + ".tmp.XXXXXX";
    std::vector<char> tmpl(tpl.begin(), tpl.end());
    tmpl.push_back('\0');
    int fd = mkstemp(tmpl.data());
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteQuotaFile: failed to create temp file, errno: %d", errno);
        return false;
    }
    std::string content;
    for (const auto& item : records) {
        content += item.first + " " + std::to_string(item.second.timestamp) + " " +
            std::to_string(item.second.count) + "\n";
    }
    const char* buf = content.c_str();
    size_t toWrite = content.size();
    ssize_t written = write(fd, buf, toWrite);
    if (written != static_cast<ssize_t>(toWrite)) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteQuotaFile: failed to write data, errno: %d", errno);
        close(fd);
        unlink(tmpl.data());
        return false;
    }
    if (fsync(fd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteQuotaFile: failed to sync data, errno: %d", errno);
        close(fd);
        unlink(tmpl.data());
        return false;
    }
    if (close(fd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteQuotaFile: failed to close file, errno: %d", errno);
        unlink(tmpl.data());
        return false;
    }
    if (rename(tmpl.data(), filepath.c_str()) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteQuotaFile: failed to rename temp file, errno: %d", errno);
        unlink(tmpl.data());
        return false;
    }
    return true;
}

bool NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(long long t1, long long t2)
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

int32_t NativeMemoryProfilerSaService::PrintSimplifiedNmdInHook(
    std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply)
{
    if (config->pid_ <= 0 || !config->printNmd_ || !config->printNmdOnly_|| config->nmdType_ != SIMP_NMD) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid param, pid: %d, printNmd:%d, printNmdOnly:%d, nmdType:%d",
            config->pid_, config->printNmd_, config->printNmdOnly_, config->nmdType_);
        return RET_ERR;
    }
    std::unique_lock<std::mutex> lock(g_mtx);
    auto taskIter = g_pidCtx.find(config->pid_);
    CHECK_TRUE(taskIter != g_pidCtx.end(), RET_ERR, "hook task of pid(%d) not started", config->pid_);
    std::shared_ptr<TaskConfig> task = taskIter->second;
    CHECK_TRUE(task && task->hookMgr, RET_ERR, "hook task of pid(%d) is invalid", config->pid_);
    pid_t pid = static_cast<pid_t>(config->pid_);
    auto postProcessor = [pid](int recvFd) { return SendDumpSimplifiedNmdRequest(pid, DUMP_SIMP_NMD_CMD, recvFd); };
    CHECK_TRUE(task->hookMgr->StartInHookSimpNmdDump(postProcessor), RET_ERR, "StartInHookSimpNmdDump fail");
    lock.unlock();
    std::string simplifiedNmdResult = task->hookMgr->GetInHookSimpNmdResult(NMD_WAIT_MS);
    CHECK_TRUE(!simplifiedNmdResult.empty(), RET_ERR, "Dump in hook simplified nmd result FAIL");
    WRITESTRING(reply, simplifiedNmdResult, RET_ERR);
    return RET_OK;
}

#ifdef NATIVE_DAEMON_UNITTEST
void NativeMemoryProfilerSaService::ResetTaskContextsForTest()
{
    std::lock_guard<std::mutex> guard(g_mtx);
    g_pidInfo.clear();
    g_pidCtx.clear();
    g_nameAndFilePathCtx.clear();
    g_hasStartupMode = false;
    g_taskNum = 0;
    g_nmdPidType.clear();
    g_timerFdInfo.clear();
}

void NativeMemoryProfilerSaService::InjectPidInfoForTest(pid_t pid, uid_t uid, gid_t gid)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    g_pidInfo[pid] = {uid, gid};
}

void NativeMemoryProfilerSaService::InjectPidCtxForTest(int32_t pid, const std::shared_ptr<TaskConfig>& config)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    g_pidCtx[pid] = config;
}

void NativeMemoryProfilerSaService::SetHasStartupModeForTest(bool val)
{
    std::lock_guard<std::mutex> guard(g_mtx);
    g_hasStartupMode = val;
}

void NativeMemoryProfilerSaService::SetMockPermissionForTest(bool isUserMode, bool granted)
{
    g_mockIsUserMode = isUserMode;
    g_mockPermissionGranted = granted;
}
void NativeMemoryProfilerSaService::SetMockNativeDaemonRunningForTest(bool running)
{
    g_mockNativeDaemonRunning = running;
}
void NativeMemoryProfilerSaService::InjectPidFdsForTest(pid_t pid, int fd)
{
    std::lock_guard<std::mutex> guard(g_pidFdMtx);
    g_pidFds[pid] = fd;
}
void NativeMemoryProfilerSaService::SetMockStartServiceAbilityForTest(bool result)
{
    g_mockStartServiceAbility = result;
}
void NativeMemoryProfilerSaService::SetMockSystemResourceForTest(bool memFail, bool cpuFail, bool romFail)
{
    g_mockMemFail = memFail;
    g_mockCpuFail = cpuFail;
    g_mockRomFail = romFail;
}
void NativeMemoryProfilerSaService::SetMockSendSuccessForTest(bool success)
{
    g_mockSendSuccess = success;
}
void NativeMemoryProfilerSaService::SetMockNmdResultForTest(const std::string& result)
{
    g_mockNmdResult = result;
}
#endif

int32_t NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(
    const std::unordered_map<std::string, QuotaRecord>& records, std::string name)
{
    auto it = records.find(name);
    if (it == records.end()) {
        return 0;
    }
    long long nowTime = static_cast<long long>(time(nullptr));
    if (IsSeparateByMoreThan24Hours(it->second.timestamp, nowTime)) {
        return 0;
    }
    return it->second.count;
}

int32_t NativeMemoryProfilerSaService::GetMemProfilerTotalUseCountToday(
    const std::unordered_map<std::string, QuotaRecord>& records)
{
    int32_t totalCount = 0;
    long long nowTime = static_cast<long long>(time(nullptr));
    for (const auto& item : records) {
        if (!IsSeparateByMoreThan24Hours(item.second.timestamp, nowTime)) {
            totalCount += item.second.count;
        }
    }
    return totalCount;
}

bool NativeMemoryProfilerSaService::UpdateMemProfilerUserCount(
    std::unordered_map<std::string, QuotaRecord>& records, std::string name)
{
    time_t now = time(nullptr);
    if (now == static_cast<time_t>(-1)) {
        PROFILER_LOG_ERROR(LOG_CORE, "UpdateMemProfilerUserCount: failed to get current time");
        return false;
    }
    long long nowTime = static_cast<long long>(now);
    auto it = records.find(name);
    if (it == records.end() || IsSeparateByMoreThan24Hours(it->second.timestamp, nowTime)) {
        records[name] = {nowTime, 1};
    } else {
        it->second.count++;
        it->second.timestamp = nowTime;
    }
    return true;
}

bool NativeMemoryProfilerSaService::RollbackMemProfilerUserCount(bool isDebug, std::string name)
{
    if (isDebug) {
        return true;
    }
    std::lock_guard<std::mutex> lock(g_countFileMutex);
    std::unordered_map<std::string, QuotaRecord> records;
    if (!ReadQuotaFile(records)) {
        PROFILER_LOG_ERROR(LOG_CORE, "RollbackMemProfilerUserCount: failed to read quota file");
        return false;
    }
    auto it = records.find(name);
    if (it != records.end() && it->second.count > 0) {
        it->second.count--;
        if (!WriteQuotaFile(records)) {
            PROFILER_LOG_ERROR(LOG_CORE, "RollbackMemProfilerUserCount: failed to write quota file");
            return false;
        }
    }
    return true;
}

} // namespace OHOS::Developtools::NativeDaemon
