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

#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr int32_t TIME_BASE = 1000;
constexpr int32_t MAX_TASK_NUM = 4;
const std::string FILE_PATH_HEAD = "/data/local/tmp/native_memory_";
const std::string FILE_PATH_TAIL = ".htrace";
constexpr int32_t DELAYED_SHUTDOWN_TIME = 20;
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

    auto native = new NativeMemoryProfilerSaService();
    if (native == nullptr) {
        HILOG_ERROR(LOG_CORE, "native is nullptr");
        return false;
    }
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

int32_t NativeMemoryProfilerSaService::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    return StartHook(config);
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
    if (StartHook(config, fd) == RET_ERR) {
        close(fd);
        return RET_ERR;
    }
    return RET_OK;
}

void NativeMemoryProfilerSaService::StopHook(uint32_t pid, std::string name, bool timeout)
{
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
    if (timeout) {
        scheduleTaskManager_.UnscheduleTaskLockless(config->timerFd);
    } else {
        scheduleTaskManager_.UnscheduleTask(config->timerFd);
    }
    nameAndFilePathCtx_.erase(config->processName);
    nameAndFilePathCtx_.erase(config->filePath);
    pidCtx_.erase(config->pid);
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

int32_t NativeMemoryProfilerSaService::StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd)
{
    if (config == nullptr) {
        return RET_ERR;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    if (config->filePath_.empty() && fd == 0) {
        std::string filePathStr = (config->pid_ > 0) ? std::to_string(config->pid_) : config->processName_;
        config->filePath_ = FILE_PATH_HEAD + filePathStr + FILE_PATH_TAIL;
    }
    PROFILER_LOG_INFO(LOG_CORE, "file path: %s", config->filePath_.c_str());

    if (!CheckConfig(config, fd)) {
        return RET_ERR;
    }

    std::shared_ptr<TraceFileWriter> writeFile = nullptr;
    if (fd == 0) {
        writeFile = std::make_shared<TraceFileWriter>(config->filePath_);
    } else {
        writeFile = std::make_shared<TraceFileWriter>(fd);
    }
    CHECK_NOTNULL(writeFile, RET_ERR, "Failed to create TraceFileWriter");
    writeFile->SetTimeSource();

    std::shared_ptr<HookManager> hook = std::make_shared<HookManager>();
    CHECK_NOTNULL(hook, RET_ERR, "Failed to create HookManager");
    hook->RegisterWriter(writeFile);
    hook->SetHookConfig(config);
    hook->SetSaServiceFlag(true);
    if (hook->CreatePluginSession() != RET_OK) {
        return RET_ERR;
    }
    hook->WriteHookConfig();
    hook->StartPluginSession();

    int32_t timerFd = scheduleTaskManager_.ScheduleTask(
        std::bind(&NativeMemoryProfilerSaService::StopHook, this, config->pid_, config->processName_, true),
        config->duration_ * TIME_BASE,
        true);
    if (timerFd == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService Start Schedule Task failed");
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
        context.SendHookConfig(reinterpret_cast<uint8_t *>(&clientConfig), sizeof(clientConfig));
        context.SendFileDescriptor(smbFd);
        context.SendFileDescriptor(eventFd);
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
            DELAYED_SHUTDOWN_TIME * TIME_BASE,
            true);
        if (timerFd == -1) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService:DelayedShutdown Schedule Task failed");
            return;
        }
        delayedShutdownTimerFd_ = timerFd;
    }
}
} // namespace OHOS::Developtools::NativeDaemon