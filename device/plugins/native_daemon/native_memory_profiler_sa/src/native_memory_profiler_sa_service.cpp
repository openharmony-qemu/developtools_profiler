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

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr int32_t TIME_BASE = 1000;
constexpr int32_t MAX_TASK_NUM = 4;
const std::string FILE_PATH_HEAD = "/data/local/tmp/native_memory_";
const std::string FILE_PATH_TAIL = ".htrace";
constexpr int32_t DELAYED_SHUTDOWN_TIME = 20;
constexpr int FILE_MODE = 0644;
constexpr int32_t SIMP_NMD = 3;
constexpr int32_t NMD_WAIT_MS = 100;
constexpr int32_t NMD_WAIT_TIMES = 50;
}

NativeMemoryProfilerSaService::NativeMemoryProfilerSaService() : SystemAbility(NATIVE_DAEMON_SYSTEM_ABILITY_ID, true)
{
    serviceName_ = "HookService";
    serviceEntry_ = std::make_shared<ServiceEntry>();
    if (!serviceEntry_->StartServer(DEFAULT_UNIX_SOCKET_HOOK_PATH)) {
        serviceEntry_ = nullptr;
        PROFILER_LOG_ERROR(LOG_CORE, "Start IPC Service FAIL");
        return;
    }
    serviceEntry_->RegisterService(*this);
    DelayedShutdown(false);
}

NativeMemoryProfilerSaService::~NativeMemoryProfilerSaService()
{
    serviceEntry_ = nullptr;
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
    uint32_t callingTokenID = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingTokenID,
                                                                       "ohos.permission.ENABLE_PROFILER");
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "No profiling permission, please check!");
        return false;
    }
    return true;
}

int32_t NativeMemoryProfilerSaService::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config->printNmd_) {
        std::lock_guard<std::mutex> guard(nmdMtx_);
        nmdPidType_[config->nmdPid_] = std::make_pair(0, config->nmdType_);
        if (!config->printNmdOnly_) {
            return RET_OK;
        }
    }
    return StartHook(config);
}

int32_t NativeMemoryProfilerSaService::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                             MessageParcel &reply)
{
    if (config->printNmd_) {
        std::lock_guard<std::mutex> guard(nmdMtx_);
        nmdPidType_[config->nmdPid_] = std::make_pair(0, config->nmdType_);
        if (!config->printNmdOnly_) {
            return RET_OK;
        }
    }
    return StartHook(config, 0, reply);
}

int32_t NativeMemoryProfilerSaService::Stop(uint32_t pid)
{
    StopHook(pid);
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::Stop(const std::string& name)
{
    StopHook(0, name);
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    if (config->printNmd_) {
        std::lock_guard<std::mutex> guard(nmdMtx_);
        nmdPidType_[config->nmdPid_] = std::make_pair(fd, config->nmdType_);
        if (!config->printNmdOnly_) {
            return RET_OK;
        }
    }
    if (StartHook(config, fd) == RET_ERR) {
        close(fd);
        return RET_ERR;
    }
    return RET_OK;
}

void NativeMemoryProfilerSaService::StopHook(uint32_t pid, std::string name)
{
    if (!HasProfilingPermission()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopHook failed, no profiling permission!");
        return;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    std::shared_ptr<TaskConfig> config = nullptr;
    if (pid > 0) {
        if (auto taskIter = pidCtx_.find(pid); taskIter != pidCtx_.end()) {
            config = taskIter->second;
        }
    } else if (auto taskIter = nameAndFilePathCtx_.find(name); taskIter != nameAndFilePathCtx_.end()) {
        config = taskIter->second;
    }
    if (config == nullptr) {
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has stop, pid: %d, process name: %s",
            pid, name.c_str());
        return;
    }

    config->hookMgr->StopPluginSession({});
    config->hookMgr->DestroyPluginSession({});
    nameAndFilePathCtx_.erase(config->processName);
    nameAndFilePathCtx_.erase(config->filePath);
    pidCtx_.erase(config->pid);
    if (nmdPidType_.find(config->pid) != nmdPidType_.end()) {
        nmdPidType_.erase(config->pid);
    }
    if (config->isStartupMode) {
        hasStartupMode_ = false;
    }
    if (config->fd > 0) {
        close(config->fd);
    }
    if (--taskNum_ == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "StringViewMemoryHold clear");
        StringViewMemoryHold::GetInstance().Clear();
        DelayedShutdown(false);
    }
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
    return args.str();
}

int32_t NativeMemoryProfilerSaService::StartHookLock(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                                     uint32_t fd, std::shared_ptr<HookManager>& hook,
                                                     std::string& args)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (config->filePath_.empty() && fd == 0) {
        std::string filePathStr = (config->pid_ > 0) ? std::to_string(config->pid_) : config->processName_;
        config->filePath_ = FILE_PATH_HEAD + filePathStr + FILE_PATH_TAIL;
    }
    PROFILER_LOG_INFO(LOG_CORE, "file path: %s", config->filePath_.c_str());

    if (!CheckConfig(config, fd)) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_MSG_EMPTY,
            "check config failed");
        return RET_ERR;
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
        CHECK_TRUE(fdTemp >= 0, RET_ERR, "Failed to open file(%s)", config->filePath_.c_str());
        fd = static_cast<uint32_t>(fdTemp);
    }
    std::shared_ptr<TraceFileWriter> writeFile = nullptr;
    if (config->printNmdOnly_) {
        writeFile = std::make_shared<TraceFileWriter>(0);
    } else {
        writeFile = std::make_shared<TraceFileWriter>(fd);
    }
    CHECK_NOTNULL(writeFile, RET_ERR, "Failed to create TraceFileWriter");
    writeFile->SetTimeSource();

    hook->RegisterWriter(writeFile);
    hook->SetSaMode(true);
    hook->SetHookConfig(config);
    hook->SetSaServiceConfig(true, false);
    if (config->pid_ > 0) {
        std::lock_guard<std::mutex> nmdGuard(nmdMtx_);
        if (nmdPidType_.find(config->pid_) != nmdPidType_.end()) {
            hook->SetNmdInfo(nmdPidType_[config->pid_]);
        }
    }
    if (hook->CreatePluginSession() != RET_OK) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_FAIL,
            "create pluginsession failed");
        return RET_ERR;
    }
    if (!config->printNmdOnly_) {
        hook->WriteHookConfig();
    }
    hook->StartPluginSession();

    int32_t timerFd = scheduleTaskManager_.ScheduleTask(
        [this, config] { this->StopHook(config->pid_, config->processName_); },
        config->duration_ * TIME_BASE,
        true);
    if (timerFd == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService Start Schedule Task failed");
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_FAIL,
            "start schedule task failed");
        return RET_ERR;
    }

    std::shared_ptr<TaskConfig> configCtx =
        std::make_shared<TaskConfig>(hook, config->pid_, config->processName_, config->filePath_, timerFd,
                                     config->startupMode_, fd);
    CHECK_NOTNULL(hook, RET_ERR, "Failed to create TaskConfig");
    if (!hasStartupMode_ && config->startupMode_) {
        hasStartupMode_ = true;
        startupModeProcessName_ = config->processName_;
    }

    if (configCtx->pid > 0) {
        pidCtx_[configCtx->pid] = configCtx;
    } else if (!configCtx->processName.empty()) {
        nameAndFilePathCtx_[configCtx->processName] = configCtx;
    }

    if (fd == 0) {
        nameAndFilePathCtx_[configCtx->filePath] = configCtx;
    }
    ++taskNum_;
    DelayedShutdown(true);
    COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_SUCC, "success");
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd)
{
    auto args = GetCmdArgs(config);
    if (!HasProfilingPermission()) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_NO_PERMISSION,
            "no profiling permission");
        PROFILER_LOG_ERROR(LOG_CORE, "StartHook failed, no profiling permission!");
        return RET_ERR;
    }
    if (config == nullptr) {
        return RET_ERR;
    }
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    CHECK_NOTNULL(hook, RET_ERR, "Failed to create HookManager");

    return StartHookLock(config, fd, hook, args);
}

static int32_t WaitSimplifiedNmdTimeout(uint32_t times, std::shared_ptr<HookManager>& hook, MessageParcel &reply)
{
    uint32_t cnt = 0;
    while ((!hook->nmdComplete_) && (cnt < times)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(NMD_WAIT_MS));
        cnt++;
    }
    if (!hook->nmdComplete_) {
        PROFILER_LOG_ERROR(LOG_CORE, "WaitSimplifiedNmdTimeout, %d ms", times * NMD_WAIT_MS);
        return RET_ERR;
    }
    WRITESTRING(reply, hook->simplifiedNmd_, RET_ERR);
    return RET_OK;
}

int32_t NativeMemoryProfilerSaService::StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                                 uint32_t fd, MessageParcel &reply)
{
    auto args = GetCmdArgs(config);
    if (!HasProfilingPermission()) {
        COMMON::PluginWriteToHisysevent("native_hook_plugin", "hiview", args, COMMON::ErrorType::RET_NO_PERMISSION,
            "no profiling permission");
        PROFILER_LOG_ERROR(LOG_CORE, "StartHook failed, no profiling permission!");
        return RET_ERR;
    }
    if (config == nullptr) {
        return RET_ERR;
    }
    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    CHECK_NOTNULL(hook, RET_ERR, "Failed to create HookManager");

    int32_t ret = StartHookLock(config, fd, hook, args);
    if (ret != RET_OK) {
        return ret;
    }

    if (config->nmdType_ == SIMP_NMD) {
        return WaitSimplifiedNmdTimeout(NMD_WAIT_TIMES, hook, reply);
    }
    return RET_OK;
}

bool NativeMemoryProfilerSaService::CheckConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd)
{
    if (taskNum_ + 1 > MAX_TASK_NUM) {
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: Support up to 4 tasks at the same time");
        return false;
    }

    if (hasStartupMode_ && config->startupMode_) {
        PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: tasks with an existing startup mode, name: %s",
            startupModeProcessName_.c_str());
        return false;
    }

    if (config->pid_ > 0) {
        config->processName_.clear();
        if (pidCtx_.find(config->pid_) != pidCtx_.end()) {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has started, pid: %d", config->pid_);
            return false;
        }
    } else if (!config->processName_.empty()) {
        if (nameAndFilePathCtx_.find(config->processName_) != nameAndFilePathCtx_.end()) {
            PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaService: hook has started, process name: %s",
                              config->processName_.c_str());
            return false;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The PID and process name are not configured");
        return false;
    }

    if (fd > 0) {
        return true;
    }

    if (!config->filePath_.empty()) {
        if (nameAndFilePathCtx_.find(config->filePath_) != nameAndFilePathCtx_.end()) {
            PROFILER_LOG_ERROR(LOG_CORE,
                               "NativeMemoryProfilerSaService: File %s is being used.", config->filePath_.c_str());
            return false;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The file path are not configured");
        return false;
    }
    return true;
}

void NativeMemoryProfilerSaService::FillTaskConfigContext(int32_t pid, const std::string& name)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (auto iter = pidCtx_.find(pid); iter != pidCtx_.end()) {
        iter->second->processName = name;
        nameAndFilePathCtx_[name] = iter->second;
        if (iter->second->isStartupMode) {
            hasStartupMode_ = false;
        }
    } else if (auto it = nameAndFilePathCtx_.find(name); it != nameAndFilePathCtx_.end()) {
        it->second->pid = pid;
        pidCtx_[pid] = it->second;
        if (it->second->isStartupMode) {
            hasStartupMode_ = false;
        }
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService: fill TaskConfig context failed");
    }
}

bool NativeMemoryProfilerSaService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf,
    const uint32_t size)
{
    if (size != sizeof(int)) {
        return false;
    }
    int peerConfig = *const_cast<int *>(reinterpret_cast<const int *>(buf));
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
    FillTaskConfigContext(peerConfig, bundleName); // Save the relevant context for subsequent inspection

    std::lock_guard<std::mutex> guard(mtx_);
    if (auto iter = pidCtx_.find(peerConfig); iter != pidCtx_.end()) {
        auto [eventFd, smbFd] = iter->second->hookMgr->GetFds(peerConfig, bundleName);
        iter->second->hookMgr->SetPid(peerConfig);
        if (eventFd == smbFd) {
            PROFILER_LOG_ERROR(LOG_CORE,
                               "Get eventFd and smbFd failed!, name: %s, pid: %d", bundleName.c_str(), peerConfig);
            return false;
        }
        PROFILER_LOG_INFO(LOG_CORE,
            "ProtocolProc, receive message from hook client, and send hook config to process %d, name: %s",
            peerConfig, bundleName.c_str());
        ClientConfig clientConfig;
        iter->second->hookMgr->GetClientConfig(clientConfig);
        if (iter->second->hookMgr->GetNoDataQueueFlag()) {
            clientConfig.freeEventOnlyAddrEnable = true;
        }
        clientConfig.isSaMode = true;
        context.SendHookConfig(reinterpret_cast<uint8_t *>(&clientConfig), sizeof(clientConfig));
        context.SendFileDescriptor(smbFd);
        context.SendFileDescriptor(eventFd);
        iter->second->hookMgr->ResetStartupParam();
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc: send config failed");
        return false;
    }
    return true;
}

void NativeMemoryProfilerSaService::DelayedShutdown(bool cancel)
{
    if (cancel) {
        scheduleTaskManager_.UnscheduleTask(delayedShutdownTimerFd_);
        delayedShutdownTimerFd_ = -1;
    } else {
        int32_t timerFd = scheduleTaskManager_.ScheduleTask(
            []() {
                int ret = SystemSetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "0");
                if (ret < 0) {
                    PROFILER_LOG_ERROR(LOG_CORE, "DelayedShutdown close sa failed");
                } else {
                    PROFILER_LOG_INFO(LOG_CORE, "DelayedShutdown close sa success");
                }
            },
            DELAYED_SHUTDOWN_TIME * TIME_BASE, true);
        if (timerFd == -1) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService:DelayedShutdown Schedule Task failed");
            return;
        }
        delayedShutdownTimerFd_ = timerFd;
    }
}
} // namespace OHOS::Developtools::NativeDaemon