/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "network_profiler_manager.h"

#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

#include "logging.h"
#include "init_param.h"
#include "common.h"
#include "network_profiler_common.h"
#include "plugin_service_types.pb.h"
#include "command_poller.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"

namespace {
constexpr int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
constexpr uint32_t PAGE_BYTES = 4096;
constexpr uint32_t BUFFER_SIZE = (1UL << 23);
const std::string VERSION = "1.02";
const std::string PARAM_KAY = "hiviewdfx.hiprofiler.networkprofiler.target";
std::shared_ptr<Writer> g_buffWriter = nullptr;
}

namespace OHOS::Developtools::Profiler {
NetworkProfilerManager::NetworkProfilerManager()
{
}

NetworkProfilerManager::~NetworkProfilerManager()
{
    StopNetworkProfiler();
}

void NetworkProfilerManager::Init()
{
    auto commandPoller = std::make_shared<CommandPoller>(shared_from_this());
    CHECK_NOTNULL(commandPoller, NO_RETVAL, "create CommandPoller FAILED!");
    CHECK_TRUE(commandPoller->OnConnect(), NO_RETVAL, "connect FAILED");
    SetCommandPoller(commandPoller);
    RegisterAgentPlugin("network-profiler");
}

bool NetworkProfilerManager::CheckConfigPid(std::set<int32_t>& pidCache)
{
    for (const auto& pid : config_.pid()) {
        if (pid > 0) {
            if (COMMON::IsUserMode() && (!COMMON::CheckApplicationPermission(pid, ""))) {
                continue;
            }
            struct stat statBuf;
            std::string pidPath = "/proc/" + std::to_string(pid) + "/status";
            if (stat(pidPath.c_str(), &statBuf) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s: hook process does not exist", __func__);
                return false;
            }
            auto [iter, ret] = pidCache.emplace(pid);
            if (ret) {
                networkCtx_.emplace_back(std::make_shared<NetworkProfilerCtx>(pid));
                paramValue_ += std::to_string(pid) + ",";
            }
        }
    }
    return true;
}

bool NetworkProfilerManager::CheckStartupProcessName()
{
    for (const auto& name : config_.startup_process_name()) {
        if (name.empty()) {
            continue;
        }
        if (COMMON::IsUserMode() && (!COMMON::CheckApplicationPermission(0, name))) {
            continue;
        }
        int pidValue = -1;
        bool isExist = COMMON::IsProcessExist(name, pidValue);
        if (isExist) {
            PROFILER_LOG_ERROR(LOG_CORE, "NetworkProfilerManager Process %s already exists", name.c_str());
            return false;
        } else {
            paramValue_ += name + ",";
            networkCtx_.emplace_back(std::make_shared<NetworkProfilerCtx>(name));
        }
    }
    return true;
}

bool NetworkProfilerManager::CheckRestartProcessName(std::set<int32_t>& pidCache)
{
    for (const auto& name : config_.restart_process_name()) {
        if (name.empty()) {
            continue;
        }
        if (COMMON::IsUserMode() && (!COMMON::CheckApplicationPermission(0, name))) {
            continue;
        }
        int pidValue = -1;
        bool isExist = COMMON::IsProcessExist(name, pidValue);
        if (isExist) {
            auto [iter, ret] = pidCache.emplace(pidValue);
            if (!ret) {
                PROFILER_LOG_ERROR(LOG_CORE,
                    "NetworkProfilerManager process %s pid is %d, duplicate of pid list in config",
                    name.c_str(), pidValue);
                return false;
            }
            paramValue_ += name + ",";
            networkCtx_.emplace_back(std::make_shared<NetworkProfilerCtx>(pidValue, name, true));
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "NetworkProfilerManager Process %s does not exist", name.c_str());
            return false;
        }
    }
    return true;
}

bool NetworkProfilerManager::CheckConfig()
{
    std::set<int32_t> pidCache;
    if (!CheckConfigPid(pidCache)) {
        return false;
    }
    if (!CheckStartupProcessName()) {
        return false;
    }
    if (!CheckRestartProcessName(pidCache)) {
        return false;
    }

    if (config_.flush_interval() == 0) {
        config_.set_flush_interval(1);
    }

    if (config_.clock_id() == NetworkProfilerConfig::UNKNOW) {
        PROFILER_LOG_ERROR(LOG_CORE, "NetworkProfilerManager clock_id is unknow");
        return false;
    }
    return true;
}

clockid_t NetworkProfilerManager::GetClockId(NetworkProfilerConfig::ClockId clockType)
{
    std::map<NetworkProfilerConfig::ClockId, clockid_t> clockMap = {
        {NetworkProfilerConfig::BOOTTIME, CLOCK_BOOTTIME},
        {NetworkProfilerConfig::REALTIME, CLOCK_REALTIME},
        {NetworkProfilerConfig::REALTIME_COARSE, CLOCK_REALTIME_COARSE},
        {NetworkProfilerConfig::MONOTONIC, CLOCK_MONOTONIC},
        {NetworkProfilerConfig::MONOTONIC_COARSE, CLOCK_MONOTONIC_COARSE},
        {NetworkProfilerConfig::MONOTONIC_RAW, CLOCK_MONOTONIC_RAW},
    };

    return clockMap.find(clockType) != clockMap.end() ? clockMap[clockType] : CLOCK_BOOTTIME;
}

bool NetworkProfilerManager::StartNetworkProfiler()
{
    if (!CheckConfig()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartNetworkProfiler CheckConfig failed");
        return false;
    }
    if (paramValue_.empty() || networkCtx_.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartNetworkProfiler ctx is empty");
        return false;
    }

    for (auto& item : networkCtx_) {
        CHECK_TRUE(HandleNetworkProfilerContext(item), false, "HandleNetworkProfilerContext failed");
    }

    socketService_ = std::make_shared<NetworkProfilerSocketService>(shared_from_this());
    socketService_->SetConfig(config_.smb_pages() * PAGE_BYTES, config_.flush_interval(),
        config_.block(), GetClockId(config_.clock_id()));

    int ret = SystemSetParameter(PARAM_KAY.c_str(), paramValue_.c_str());
    PROFILER_LOG_INFO(LOG_CORE, "StartNetworkProfiler parameter: %s", paramValue_.c_str());

    auto args = GetCmdArgs(config_);
    if (ret < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartNetworkProfiler set parameter failed");
        COMMON::PluginWriteToHisysevent("network_profiler_plugin", "sh", args, COMMON::ErrorType::RET_FAIL,
                                        "set param failed");
        return false;
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "StartNetworkProfiler set parameter success");
    }

    int res = COMMON::PluginWriteToHisysevent("memory_plugin", "sh", args, COMMON::ErrorType::RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "hisysevent report network_profiler_plugin ret: %d.", res);
    return true;
}

std::string NetworkProfilerManager::GetCmdArgs(const NetworkProfilerConfig& traceConfig)
{
    std::stringstream args;
    for (const auto& p : traceConfig.pid()) {
        args << "pid: " << COMMON::GetProcessNameByPid(p) << ", ";
    }
    for (const auto& name : traceConfig.startup_process_name()) {
        args << "startup_process_name: " << name << ", ";
    }
    for (const auto& name : traceConfig.restart_process_name()) {
        args << "restart_process_name: " << name << ", ";
    }
    args << "clock_id: " << std::to_string(traceConfig.clock_id()) << ", ";
    args << "smb_pages: " << std::to_string(traceConfig.smb_pages()) << ", ";
    args << "flush_interval: " << std::to_string(traceConfig.flush_interval()) << ", ";
    args << "block: " << (traceConfig.block() ? "true" : "false");
    return args.str();
}

void NetworkProfilerManager::StopNetworkProfiler()
{
    int ret = SystemSetParameter(PARAM_KAY.c_str(), "");
    if (ret < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopNetworkProfiler set parameter failed");
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "StopNetworkProfiler set parameter success");
    }

    socketService_ = nullptr;

    for (const auto& item : networkCtx_) {
        if (item->eventPoller != nullptr) {
            HILOG_BASE_ERROR(LOG_CORE, "eventPoller unset!");
            if (item->eventNotifier != nullptr) {
                item->eventPoller->RemoveFileDescriptor(item->eventNotifier->GetFd());
            }
            item->eventPoller->Stop();
            item->eventPoller->Finalize();
            item->eventPoller = nullptr;
        }

        if (item->shareMemoryBlock != nullptr) {
            ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(item->smbName);
            item->shareMemoryBlock = nullptr;
        }
        if (item->eventNotifier != nullptr) {
            item->eventNotifier = nullptr;
        }
        if (item->handle != nullptr) {
            item->handle->StopHandle();
            item->handle = nullptr;
        }
    }
}

bool NetworkProfilerManager::HandleNetworkProfilerContext(const std::shared_ptr<NetworkProfilerCtx>& ctx)
{
    if (ctx == nullptr) {
        return false;
    }
    if (ctx->pid > 0) {
        ctx->smbName = "network_profiler_smb_" + std::to_string(ctx->pid);
    } else if (!ctx->processName.empty()) {
        ctx->smbName = "network_profiler_smb_" + ctx->processName;
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext context error, pid: %d, process name: %s",
            ctx->pid, ctx->processName.c_str());
        return false;
    }

    uint32_t bufferSize = static_cast<uint32_t>(config_.smb_pages() * PAGE_BYTES);
    ctx->shareMemoryBlock = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(ctx->smbName, bufferSize);
    CHECK_TRUE(ctx->shareMemoryBlock != nullptr, false, "CreateMemoryBlockLocal FAIL %s", ctx->smbName.c_str());

    ctx->eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    CHECK_NOTNULL(ctx->eventNotifier, false, "create EventNotifier for %s failed!", ctx->smbName.c_str());

    // start event poller task
    ctx->eventPoller = std::make_unique<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
    CHECK_NOTNULL(ctx->eventPoller, false, "create event poller FAILED!");

    ctx->eventPoller->Init();
    ctx->eventPoller->Start();

    PROFILER_LOG_INFO(LOG_CORE, "%s smbFd = %d, eventFd = %d\n", ctx->smbName.c_str(),
        ctx->shareMemoryBlock->GetfileDescriptor(), ctx->eventNotifier->GetFd());

    ctx->handle = std::make_shared<NetworkProfilerHandle>(CLOCK_REALTIME, BUFFER_SIZE, isProtobufSerialize_);
    if (isProtobufSerialize_) {
        ctx->handle->SetWriter(g_buffWriter);
    } else {
        ctx->handle->SetWriter(const_cast<WriterStructPtr>(writerAdapter_->GetStruct()));
    }

    ctx->eventPoller->AddFileDescriptor(
        ctx->eventNotifier->GetFd(),
        [this, ctx] { this->ReadShareMemory(ctx); }
    );
    PROFILER_LOG_DEBUG(LOG_CORE, "network profiler context: pid: %d, processName: %s, eventFd: %d, shmFd: %d",
        ctx->pid, ctx->processName.c_str(), ctx->eventNotifier->GetFd(), ctx->shareMemoryBlock->GetfileDescriptor());
    return true;
}

void NetworkProfilerManager::ReadShareMemory(std::shared_ptr<NetworkProfilerCtx> ctx)
{
    ctx->eventNotifier->Take();
    while (true) {
        bool ret = ctx->shareMemoryBlock->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            if (firstData_ && firstPluginId_ != 0) {
                firstData_ = false;
                ProfilerPluginState pluginState;
                pluginState.set_version(COMMON::STATE_VERSION);
                pluginState.set_event_detail("Report data success: network-profiler get data!");
                pluginState.set_event(ProfilerPluginState::DATA_READY);
                pluginState.set_name("network-profiler");
                if (commandPoller_ != nullptr) {
                    commandPoller_->PushResult(pluginState, firstPluginId_);
                }
            }
            ctx->handle->SerializeData(data, size);
            return true;
        });
        if (!ret) {
            break;
        }
    }
}

std::pair<int, int> NetworkProfilerManager::GetNetworkProfilerCtx(int32_t pid, const std::string& name)
{
    for (const auto& item : networkCtx_) {
        if (item->pid == pid || item->processName == name) {
            if (item->restart && (item->pid == pid)) {
                return {0, 0};
            }
            PROFILER_LOG_DEBUG(LOG_CORE, "GetNetworkProfilerCtx: pid: %d, name: %s, eventFd: %d, shmFd: %d",
                pid, name.c_str(), item->eventNotifier->GetFd(), item->shareMemoryBlock->GetfileDescriptor());
            item->handle->SetTargetProcessInfo(pid, name);
            return {item->eventNotifier->GetFd(), item->shareMemoryBlock->GetfileDescriptor()};
        }
    }
    return {-1, -1};
}

bool NetworkProfilerManager::LoadPlugin(const std::string& pluginPath)
{
    return true;
}

bool NetworkProfilerManager::UnloadPlugin(const std::string& pluginPath)
{
    return true;
}

bool NetworkProfilerManager::UnloadPlugin(const uint32_t pluginId)
{
    return true;
}

bool NetworkProfilerManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    std::string cfgData = config[0].config_data();
    if (config_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:parseFromArray failed!", __func__);
        return false;
    }
    return true;
}

bool NetworkProfilerManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    socketService_ = nullptr;
    return true;
}

bool NetworkProfilerManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
    const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    bool isStarted = StartNetworkProfiler();
    if (pluginIds.size() > 0) {
        ProfilerPluginState pluginState;
        pluginState.set_version(COMMON::STATE_VERSION);
        pluginState.set_name("network-profiler");
        if (isStarted) {
            pluginState.set_event_detail("network profiler plugin load success");
            pluginState.set_event(ProfilerPluginState::PLUGIN_LOAD_SUCC);
        } else {
            pluginState.set_event_detail("plugin load failed: network-profiler");
            pluginState.set_event(ProfilerPluginState::PLUGIN_ERR);
        }
        CHECK_NOTNULL(commandPoller_, false, "command poller is invalid, send state info failed!");
        firstPluginId_ = pluginIds[0];
        commandPoller_->PushResult(pluginState, firstPluginId_);
    }
    return isStarted;
}

bool NetworkProfilerManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    StopNetworkProfiler();
    return true;
}

bool NetworkProfilerManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    return true;
}

bool NetworkProfilerManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
    bool isProtobufSerialize)
{
    PROFILER_LOG_INFO(LOG_CORE, "network CreateWriter isProtobufSerialize: %d", isProtobufSerialize);
    writer_ = std::make_shared<BufferWriter>("network-profiler", VERSION, bufferSize, smbFd, eventFd, agentIndex_);
    isProtobufSerialize_ = isProtobufSerialize;
    if (!isProtobufSerialize_) {
        writerAdapter_ = std::make_shared<WriterAdapter>(isProtobufSerialize_);
        writerAdapter_->SetWriter(writer_);
    } else {
        g_buffWriter = writer_;
    }
    return true;
}

bool NetworkProfilerManager::ResetWriter(uint32_t pluginId)
{
    g_buffWriter = nullptr;
    return true;
}

void NetworkProfilerManager::SetCommandPoller(const std::shared_ptr<CommandPoller>& p)
{
    commandPoller_ = p;
}

bool NetworkProfilerManager::RegisterAgentPlugin(const std::string& pluginPath)
{
    RegisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_path("builtin/" + pluginPath);
    request.set_sha256("");
    request.set_name(pluginPath);
    request.set_buffer_size_hint(0);
    RegisterPluginResponse response;

    if (commandPoller_->RegisterPlugin(request, response)) {
        if (response.status() == ResponseStatus::OK) {
            PROFILER_LOG_DEBUG(LOG_CORE, "response.plugin_id() = %d", response.plugin_id());
            agentIndex_ = static_cast<int>(response.plugin_id());
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
} // namespace OHOS::Developtools::Profiler
