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

#include "ffrt_profiler_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

#include "logging.h"
#include "init_param.h"
#include "common.h"
#include "ffrt_profiler_common.h"
#include "plugin_service_types.pb.h"
#include "command_poller.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"

namespace {
constexpr int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
constexpr uint32_t PAGE_BYTES = 4096;
constexpr uint32_t BUFFER_SIZE = (1UL << 23);
const std::string VERSION = "1.02";
std::shared_ptr<Writer> g_buffWriter = nullptr;
}

namespace OHOS::Developtools::Profiler {
FfrtProfilerManager::FfrtProfilerManager()
{
}

FfrtProfilerManager::~FfrtProfilerManager()
{
    StopFfrtProfiler();
}

void FfrtProfilerManager::Init()
{
    auto commandPoller = std::make_shared<CommandPoller>(shared_from_this());
    CHECK_NOTNULL(commandPoller, NO_RETVAL, "create CommandPoller FAILED!");
    CHECK_TRUE(commandPoller->OnConnect(), NO_RETVAL, "FfrtProfilerManager::Init connect FAILED");
    SetCommandPoller(commandPoller);
    RegisterAgentPlugin("ffrt-profiler");
}

bool FfrtProfilerManager::CheckConfig()
{
    std::set<int32_t> pidCache;
    for (const auto& pid : config_.pid()) {
        if (pid > 0) {
            struct stat statBuf;
            std::string pidPath = "/proc/" + std::to_string(pid) + "/status";
            if (stat(pidPath.c_str(), &statBuf) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s: hook process does not exist", __func__);
                return false;
            } else {
                auto [iter, ret] = pidCache.emplace(pid);
                if (ret) {
                    ffrtCtx_.emplace_back(std::make_shared<FfrtProfilerCtx>(pid));
                    paramValue_ += std::to_string(pid) + ",";
                }
                continue;
            }
        }
    }

    for (const auto& name : config_.startup_process_name()) {
        if (name.empty()) {
            continue;
        }
        int pidValue = -1;
        bool isExist = COMMON::IsProcessExist(name, pidValue);
        if (isExist) {
            PROFILER_LOG_ERROR(LOG_CORE, "FfrtProfilerManager Process %s already exists", name.c_str());
            return false;
        } else {
            paramValue_ += name + ",";
            ffrtCtx_.emplace_back(std::make_shared<FfrtProfilerCtx>(name));
        }
    }

    for (const auto& name : config_.restart_process_name()) {
        if (name.empty()) {
            continue;
        }

        int pidValue = -1;
        bool isExist = COMMON::IsProcessExist(name, pidValue);
        if (isExist) {
            auto [iter, ret] = pidCache.emplace(pidValue);
            if (!ret) {
                PROFILER_LOG_ERROR(LOG_CORE,
                    "FfrtProfilerManager process %s pid is %d, duplicate of pid list in config",
                    name.c_str(), pidValue);
                return false;
            }
            paramValue_ += name + ",";
            ffrtCtx_.emplace_back(std::make_shared<FfrtProfilerCtx>(pidValue, name, true));
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "FfrtProfilerManager Process %s does not exist", name.c_str());
            return false;
        }
    }

    if (config_.flush_interval() == 0) {
        config_.set_flush_interval(1);
    }

    if (config_.clock_id() == FfrtProfilerConfig::UNKNOW) {
        PROFILER_LOG_ERROR(LOG_CORE, "FfrtProfilerManager clock_id is unknow");
        return false;
    }
    return true;
}

clockid_t FfrtProfilerManager::GetClockId(FfrtProfilerConfig::ClockId clockType)
{
    std::map<FfrtProfilerConfig::ClockId, clockid_t> clockMap = {
        {FfrtProfilerConfig::BOOTTIME, CLOCK_BOOTTIME},
        {FfrtProfilerConfig::REALTIME, CLOCK_REALTIME},
        {FfrtProfilerConfig::REALTIME_COARSE, CLOCK_REALTIME_COARSE},
        {FfrtProfilerConfig::MONOTONIC, CLOCK_MONOTONIC},
        {FfrtProfilerConfig::MONOTONIC_COARSE, CLOCK_MONOTONIC_COARSE},
        {FfrtProfilerConfig::MONOTONIC_RAW, CLOCK_MONOTONIC_RAW},
    };

    return clockMap.find(clockType) != clockMap.end() ? clockMap[clockType] : CLOCK_BOOTTIME;
}

bool FfrtProfilerManager::StartFfrtProfiler()
{
    if (!CheckConfig()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartFfrtProfiler CheckConfig failed");
        return false;
    }
    if (paramValue_.empty() || ffrtCtx_.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartFfrtProfiler ctx is empty");
        return false;
    }

    for (auto& item : ffrtCtx_) {
        CHECK_TRUE(HandleFfrtProfilerContext(item), false, "HandleFfrtProfilerContext failed");
    }

    socketService_ = std::make_shared<FfrtProfilerSocketService>(shared_from_this());
    socketService_->SetConfig(config_.smb_pages() * PAGE_BYTES, config_.flush_interval(),
        config_.block(), GetClockId(config_.clock_id()));

    int ret = SystemSetParameter(PARAM_KAY.c_str(), paramValue_.c_str());
    PROFILER_LOG_INFO(LOG_CORE, "StartFfrtProfiler parameter: %s", paramValue_.c_str());
    if (ret < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "StartFfrtProfiler set parameter failed");
        return false;
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "StartFfrtProfiler set parameter success");
    }
    return true;
}

void FfrtProfilerManager::StopFfrtProfiler()
{
    int ret = SystemSetParameter(PARAM_KAY.c_str(), "");
    if (ret < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopFfrtProfiler set parameter failed");
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "StopFfrtProfiler set parameter success");
    }

    socketService_ = nullptr;

    for (const auto& item : ffrtCtx_) {
        if (item->eventPoller != nullptr) {
            HILOG_ERROR(LOG_CORE, "eventPoller unset!");
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
        item->handle = nullptr;
    }
}

bool FfrtProfilerManager::HandleFfrtProfilerContext(const std::shared_ptr<FfrtProfilerCtx>& ctx)
{
    if (ctx == nullptr) {
        return false;
    }
    if (ctx->pid > 0) {
        ctx->smbName = "ffrt_profiler_smb_" + std::to_string(ctx->pid);
    } else if (!ctx->processName.empty()) {
        ctx->smbName = "ffrt_profiler_smb_" + ctx->processName;
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

    ctx->handle = std::make_shared<FfrtProfilerHandle>(BUFFER_SIZE, isProtobufSerialize_);
    if (isProtobufSerialize_) {
        ctx->handle->SetWriter(g_buffWriter);
    } else {
        ctx->handle->SetWriter(const_cast<WriterStructPtr>(writerAdapter_->GetStruct()));
    }

    ctx->eventPoller->AddFileDescriptor(
        ctx->eventNotifier->GetFd(),
        [this, ctx] { this->ReadShareMemory(ctx); }
    );
    PROFILER_LOG_DEBUG(LOG_CORE, "ffrt profiler context: pid: %d, processName: %s, eventFd: %d, shmFd: %d",
        ctx->pid, ctx->processName.c_str(), ctx->eventNotifier->GetFd(), ctx->shareMemoryBlock->GetfileDescriptor());
    return true;
}

void FfrtProfilerManager::ReadShareMemory(std::shared_ptr<FfrtProfilerCtx> ctx)
{
    ctx->eventNotifier->Take();
    while (true) {
        bool ret = ctx->shareMemoryBlock->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            ctx->handle->SerializeData(data, size);
            return true;
        });
        if (!ret) {
            break;
        }
    }
}

std::pair<int, int> FfrtProfilerManager::GetFfrtProfilerCtx(int32_t pid, const std::string& name)
{
    for (const auto& item : ffrtCtx_) {
        if (item->pid == pid || item->processName == name) {
            if (item->restart && (item->pid == pid)) {
                return {0, 0};
            }
            PROFILER_LOG_DEBUG(LOG_CORE, "GetFfrtProfilerCtx: pid: %d, name: %s, eventFd: %d, shmFd: %d",
                pid, name.c_str(), item->eventNotifier->GetFd(), item->shareMemoryBlock->GetfileDescriptor());
            item->handle->SetTargetProcessInfo(pid, name);
            return {item->eventNotifier->GetFd(), item->shareMemoryBlock->GetfileDescriptor()};
        }
    }
    return {-1, -1};
}

bool FfrtProfilerManager::LoadPlugin(const std::string& pluginPath)
{
    return true;
}

bool FfrtProfilerManager::UnloadPlugin(const std::string& pluginPath)
{
    return true;
}

bool FfrtProfilerManager::UnloadPlugin(const uint32_t pluginId)
{
    return true;
}

bool FfrtProfilerManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    std::string cfgData = config[0].config_data();
    if (config_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:parseFromArray failed!", __func__);
        return false;
    }
    return true;
}

bool FfrtProfilerManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    socketService_ = nullptr;
    return true;
}

bool FfrtProfilerManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
    const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    return StartFfrtProfiler();
}

bool FfrtProfilerManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    StopFfrtProfiler();
    return true;
}

bool FfrtProfilerManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    return true;
}

bool FfrtProfilerManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
    bool isProtobufSerialize)
{
    PROFILER_LOG_INFO(LOG_CORE, "ffrt CreateWriter isProtobufSerialize: %d", isProtobufSerialize);
    writer_ = std::make_shared<BufferWriter>("ffrt-profiler", VERSION, bufferSize, smbFd, eventFd, agentIndex_);
    isProtobufSerialize_ = isProtobufSerialize;
    if (!isProtobufSerialize_) {
        writerAdapter_ = std::make_shared<WriterAdapter>(isProtobufSerialize_);
        writerAdapter_->SetWriter(writer_);
    } else {
        g_buffWriter = writer_;
    }
    return true;
}

bool FfrtProfilerManager::ResetWriter(uint32_t pluginId)
{
    g_buffWriter = nullptr;
    return true;
}

void FfrtProfilerManager::SetCommandPoller(const std::shared_ptr<CommandPoller>& p)
{
    commandPoller_ = p;
}

bool FfrtProfilerManager::RegisterAgentPlugin(const std::string& pluginPath)
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
