/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "plugin_service.h"

#include <cinttypes>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "plugin_command_builder.h"
#include "plugin_service_impl.h"
#include "plugin_session_manager.h"
#include "profiler_capability_manager.h"
#include "profiler_data_repeater.h"
#include "securec.h"
#include "share_memory_allocator.h"
#include "socket_context.h"

namespace {
const int PAGE_BYTES = 4096;
const int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
constexpr uint32_t FLUSH_BASELINE = (1U << 21); // need to flush data size with offline mode
constexpr uint32_t STOP_BASELINE = (1U << 22); // need to stop take data size with offline mode
} // namespace

PluginService::PluginService()
{
    pluginIdCounter_ = 0;
    if (getuid() == 0) {
        StartService(DEFAULT_UNIX_SOCKET_FULL_PATH);
    } else {
        StartService(DEFAULT_UNIX_SOCKET_PATH);
    }

    pluginCommandBuilder_ = std::make_shared<PluginCommandBuilder>();

    eventPoller_ = std::make_unique<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
    CHECK_NOTNULL(eventPoller_, NO_RETVAL, "create event poller FAILED!");

    eventPoller_->Init();
    eventPoller_->Start();
}

PluginService::~PluginService()
{
    if (eventPoller_) {
        eventPoller_->Stop();
        eventPoller_->Finalize();
    }
}

void PluginService::SetPluginSessionManager(const PluginSessionManagerPtr& pluginSessionManager)
{
    pluginSessionManager_ = pluginSessionManager;
}

void PluginService::SetProfilerSessionConfig(const ProfilerSessionConfig& profilerSessionConfig)
{
    profilerSessionConfig_ = profilerSessionConfig;
}

SemaphorePtr PluginService::GetSemaphore(uint32_t id) const
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = waitSemphores_.find(id);
    if (it != waitSemphores_.end()) {
        return it->second;
    }
    return nullptr;
}

bool PluginService::StartService(const std::string& unixSocketName)
{
    pluginServiceImpl_ = std::make_shared<PluginServiceImpl>(*this);
    serviceEntry_ = std::make_shared<ServiceEntry>();
    if (!serviceEntry_->StartServer(unixSocketName)) {
        pluginServiceImpl_ = nullptr;
        serviceEntry_ = nullptr;
        HILOG_DEBUG(LOG_CORE, "Start IPC Service FAIL");
        return false;
    }
    serviceEntry_->RegisterService(*pluginServiceImpl_.get());
    return true;
}

static ShareMemoryBlock::ReusePolicy GetReusePolicy(const ProfilerSessionConfig::BufferConfig& bufferConfig)
{
    if (bufferConfig.policy() == ProfilerSessionConfig::BufferConfig::RECYCLE) {
        return ShareMemoryBlock::DROP_OLD;
    }
    return ShareMemoryBlock::DROP_NONE;
}

// create plugin session with buffer config
bool PluginService::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                        const ProfilerSessionConfig::BufferConfig& bufferConfig,
                                        const ProfilerDataRepeaterPtr& dataRepeater)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::string pluginName = pluginConfig.name();
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    pluginCtx->profilerDataRepeater = dataRepeater;

    uint32_t bufferSize = bufferConfig.pages() * PAGE_BYTES;
    auto cmd = pluginCommandBuilder_->BuildCreateSessionCmd(pluginConfig, bufferSize);
    CHECK_TRUE(cmd != nullptr, false, "CreatePluginSession BuildCreateSessionCmd FAIL %s", pluginName.c_str());

    auto smb = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(pluginName, bufferSize);
    CHECK_TRUE(smb != nullptr, false, "CreateMemoryBlockLocal FAIL %s", pluginName.c_str());

    auto policy = GetReusePolicy(bufferConfig);
    HILOG_DEBUG(LOG_CORE, "CreatePluginSession policy = %d", (int)policy);
    smb->SetReusePolicy(policy);

    auto notifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    CHECK_NOTNULL(notifier, false, "create EventNotifier for %s failed!", pluginName.c_str());
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginCtx->shareMemoryBlock = smb;
    pluginCtx->eventNotifier = notifier;
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    pluginCtx->context->SendFileDescriptor(smb->GetfileDescriptor());
    pluginCtx->context->SendFileDescriptor(notifier->GetFd());

    if (profilerSessionConfig_.session_mode() == ProfilerSessionConfig::OFFLINE) {
        eventPoller_->AddFileDescriptor(notifier->GetFd(),
                                        std::bind(&PluginService::ReadShareMemoryOffline, this, *pluginCtx));
    } else if (profilerSessionConfig_.session_mode() == ProfilerSessionConfig::ONLINE) {
        eventPoller_->AddFileDescriptor(notifier->GetFd(),
                                        std::bind(&PluginService::ReadShareMemoryOnline, this, *pluginCtx));
    }
    HILOG_DEBUG(LOG_CORE, "CreatePluginSession %s done, shmem fd = %d", pluginName.c_str(), smb->GetfileDescriptor());
    return true;
}

// create plugin session without buffer config
bool PluginService::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                        const ProfilerDataRepeaterPtr& dataRepeater)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::string pluginName = pluginConfig.name();
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    pluginCtx->profilerDataRepeater = dataRepeater;
    pluginCtx->shareMemoryBlock = nullptr;

    auto cmd = pluginCommandBuilder_->BuildCreateSessionCmd(pluginConfig, 0);
    CHECK_TRUE(cmd != nullptr, false, "CreatePluginSession BuildCreateSessionCmd FAIL %s", pluginName.c_str());
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    HILOG_DEBUG(LOG_CORE, "CreatePluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::StartPluginSession(const ProfilerPluginConfig& config)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::string pluginName = config.name();
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    auto cmd = pluginCommandBuilder_->BuildStartSessionCmd(config, pluginId);
    CHECK_TRUE(cmd != nullptr, false, "StartPluginSession BuildStartSessionCmd FAIL %s", pluginName.c_str());
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::IN_SESSION);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    HILOG_INFO(LOG_CORE, "StartPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::StopPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    auto cmd = pluginCommandBuilder_->BuildStopSessionCmd(pluginId);
    CHECK_TRUE(cmd != nullptr, false, "StopPluginSession BuildStopSessionCmd FAIL %s", pluginName.c_str());
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    auto sem = GetSemaphoreFactory().Create(0);
    CHECK_NOTNULL(sem, false, "create Semaphore for stop %s FAILED!", pluginName.c_str());

    waitSemphores_[cmd->command_id()] = sem;
    HILOG_DEBUG(LOG_CORE, "=== StopPluginSession %s Waiting ... ===", pluginName.c_str());
    // wait on semaphore at most 30 seconds.
    if (!sem->TimedWait(30)) {
        // semaphore timeout
        HILOG_DEBUG(LOG_CORE, "=== StopPluginSession Waiting FAIL ===");
        return false;
    }

    if (!profilerSessionConfig_.discard_cache_data()) {
        if (profilerSessionConfig_.session_mode() == ProfilerSessionConfig::OFFLINE) {
            ReadShareMemoryOffline(*pluginCtx);
        } else if (profilerSessionConfig_.session_mode() == ProfilerSessionConfig::ONLINE) {
            ReadShareMemoryOnline(*pluginCtx);
        }
    }
    HILOG_DEBUG(LOG_CORE, "StopPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::DestroyPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    auto cmd = pluginCommandBuilder_->BuildDestroySessionCmd(pluginId);
    CHECK_TRUE(cmd != nullptr, false, "DestroyPluginSession BuildDestroySessionCmd FAIL %s", pluginName.c_str());

    if (pluginCtx->shareMemoryBlock) {
        ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(pluginName);
        pluginCtx->shareMemoryBlock = nullptr;
    }

    if (pluginCtx->eventNotifier) {
        eventPoller_->RemoveFileDescriptor(pluginCtx->eventNotifier->GetFd());
        pluginCtx->eventNotifier = nullptr;
    }
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::REGISTERED);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    HILOG_INFO(LOG_CORE, "DestroyPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::RefreshPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    auto cmd = pluginCommandBuilder_->BuildRefreshSessionCmd(pluginId);
    CHECK_TRUE(cmd != nullptr, false, "RefreshPluginSession BuildRefreshSessionCmd FAIL %s", pluginName.c_str());
    CHECK_TRUE(pluginServiceImpl_->SendHeartBeat(*pluginCtx->context), false,
               "%s hiprofiler_plugin process is off line!", __func__);
    pluginServiceImpl_->PushCommand(*pluginCtx->context, cmd);
    HILOG_INFO(LOG_CORE, "RefreshPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::RemovePluginSessionCtx(const std::string& pluginName)
{
    PluginContextPtr pluginCtx = GetPluginContext(pluginName).second;
    CHECK_NOTNULL(pluginCtx, false, "get PluginContext failed!");

    if (pluginCtx->shareMemoryBlock) {
        ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(pluginName);
        pluginCtx->shareMemoryBlock = nullptr;
    }

    if (pluginCtx->eventNotifier) {
        eventPoller_->RemoveFileDescriptor(pluginCtx->eventNotifier->GetFd());
        pluginCtx->eventNotifier = nullptr;
    }

    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::INITED);
    HILOG_INFO(LOG_CORE, "RemovePluginSessionCtx %s done!", pluginName.c_str());
    return true;
}

std::pair<uint32_t, PluginContextPtr> PluginService::GetPluginContext(const std::string& pluginName)
{
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_TRUE(nameIndex_.count(pluginName) > 0, std::make_pair(0, nullptr),
               "GetPluginContext failed, plugin name `%s` not found!", pluginName.c_str());
    uint32_t id = nameIndex_[pluginName];

    CHECK_TRUE(pluginContext_.count(id) > 0, std::make_pair(id, nullptr), "plugin id %u not found!", id);
    return std::make_pair(id, pluginContext_[id]);
}

PluginContextPtr PluginService::GetPluginContextById(uint32_t id)
{
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_TRUE(pluginContext_.count(id) > 0, nullptr, "plugin id %u not found!", id);
    return pluginContext_[id];
}

bool PluginService::AddPluginInfo(const PluginInfo& pluginInfo)
{
    if (nameIndex_.find(pluginInfo.name) == nameIndex_.end()) { // add new plugin
        auto pluginCtx = std::make_shared<PluginContext>();
        CHECK_NOTNULL(pluginCtx, false, "create PluginContext failed!");

        ProfilerPluginCapability capability;
        capability.set_path(pluginInfo.path);
        capability.set_name(pluginInfo.name);
        CHECK_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(capability), false,
                   "AddPluginInfo AddCapability FAIL");

        pluginCtx->name = pluginInfo.name;
        pluginCtx->path = pluginInfo.path;
        pluginCtx->context = pluginInfo.context;
        pluginCtx->config.set_name(pluginInfo.name);
        pluginCtx->config.set_plugin_sha256(pluginInfo.sha256);
        pluginCtx->profilerPluginState.set_name(pluginInfo.name);
        pluginCtx->profilerPluginState.set_state(ProfilerPluginState::REGISTERED);
        pluginCtx->sha256 = pluginInfo.sha256;
        pluginCtx->bufferSizeHint = pluginInfo.bufferSizeHint;
        pluginCtx->isStandaloneFileData = pluginInfo.isStandaloneFileData;
        pluginCtx->outFileName = pluginInfo.outFileName;
        pluginCtx->pluginVersion = pluginInfo.pluginVersion;

        uint32_t pluginId = ++pluginIdCounter_;
        std::unique_lock<std::mutex> lock(mutex_);
        pluginContext_[pluginId] = pluginCtx;
        nameIndex_[pluginInfo.name] = pluginId;
    } else { // update sha256 or bufferSizeHint
        std::unique_lock<std::mutex> lock(mutex_);
        CHECK_TRUE(nameIndex_.count(pluginInfo.name) > 0, false, "plugin name %s not found!", pluginInfo.name.c_str());

        uint32_t pluginId = nameIndex_[pluginInfo.name];
        CHECK_TRUE(pluginContext_.count(pluginId) > 0, false, "plugin id %u not found!", pluginId);
        auto pluginCtx = pluginContext_[pluginId];

        if (pluginInfo.sha256 != "") {
            pluginCtx->sha256 = pluginInfo.sha256;
        }
        if (pluginInfo.bufferSizeHint != 0) {
            pluginCtx->bufferSizeHint = pluginInfo.bufferSizeHint;
        }
        if (pluginInfo.isStandaloneFileData != false) {
            pluginCtx->isStandaloneFileData = pluginInfo.isStandaloneFileData;
        }
        if (pluginInfo.outFileName != "") {
            pluginCtx->outFileName = pluginInfo.outFileName;
        }
        if (pluginInfo.pluginVersion != "") {
            pluginCtx->pluginVersion = pluginInfo.pluginVersion;
        }
    }
    HILOG_DEBUG(LOG_CORE, "AddPluginInfo for %s done!", pluginInfo.name.c_str());

    return true;
}

bool PluginService::GetPluginInfo(const std::string& pluginName, PluginInfo& pluginInfo)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_TRUE(pluginId, false, "plugin name %s not found!", pluginName.c_str());
    CHECK_TRUE(pluginCtx, false, "plugin id %u not found!", pluginId);

    pluginInfo.id = pluginId;
    pluginInfo.name = pluginCtx->name;
    pluginInfo.path = pluginCtx->path;
    pluginInfo.sha256 = pluginCtx->sha256;
    pluginInfo.bufferSizeHint = pluginCtx->bufferSizeHint;
    return true;
}

bool PluginService::RemovePluginInfo(const PluginInfo& pluginInfo)
{
    uint32_t pluginId = pluginInfo.id;
    PluginContextPtr pluginCtx = GetPluginContextById(pluginId);
    CHECK_NOTNULL(pluginCtx, false, "RemovePluginInfo failed, id %d not found!", pluginId);

    std::string pluginName = pluginCtx->config.name();
    CHECK_TRUE(ProfilerCapabilityManager::GetInstance().RemoveCapability(pluginName), false,
               "RemovePluginInfo RemoveCapability FAIL %d", pluginId);

    auto pluginState = pluginCtx->profilerPluginState.state();
    if (pluginState == ProfilerPluginState::LOADED || pluginState == ProfilerPluginState::IN_SESSION) {
        std::vector<std::string> pluginNames = {pluginName};
        pluginSessionManager_->InvalidatePluginSessions(pluginNames);
        pluginSessionManager_->RemovePluginSessions(pluginNames);
        this->RemovePluginSessionCtx(pluginName);
    }

    std::unique_lock<std::mutex> lock(mutex_);
    nameIndex_.erase(pluginName);
    pluginContext_.erase(pluginId);
    HILOG_DEBUG(LOG_CORE, "RemovePluginInfo for %s done!", pluginName.c_str());
    return true;
}

void PluginService::ReadShareMemoryOffline(PluginContext& context)
{
    CHECK_NOTNULL(context.shareMemoryBlock, NO_RETVAL, "smb of %s is null!", context.path.c_str());
    CHECK_NOTNULL(traceWriter_, NO_RETVAL, "traceWriter_ is null!");
    if (context.eventNotifier) {
        context.eventNotifier->Take();
    }

    uint32_t stopTakeDataSize = 0;
    while (true) {
        int retval = 0;
        bool ret = context.shareMemoryBlock->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            retval = traceWriter_->Write(data, size);
            CHECK_TRUE(retval != -1, false, "need to splite file");
            CHECK_TRUE(retval > 0, false, "write %d bytes failed!", size);
            return true;
        });

        if (retval == -1) {
            HILOG_DEBUG(LOG_CORE, "need to clear share memory block and report the basic data");
            pluginSessionManager_->RefreshPluginSession();
            break;
        }

        dataFlushSize_ += static_cast<uint32_t>(retval);
        stopTakeDataSize += static_cast<uint32_t>(retval);
        if (stopTakeDataSize > STOP_BASELINE) {
            traceWriter_->Flush();
            break;
        } else if (dataFlushSize_ > FLUSH_BASELINE) {
            traceWriter_->Flush();
            dataFlushSize_ = 0;
        }

        if (!ret) { // no data to read
            break;
        }
    }
}

void PluginService::ReadShareMemoryOnline(PluginContext& context)
{
    CHECK_NOTNULL(context.shareMemoryBlock, NO_RETVAL, "smb of %s is null!", context.path.c_str());
    if (context.eventNotifier) {
        context.eventNotifier->Take();
    }

    while (true) {
        auto pluginData = std::make_shared<ProfilerPluginData>();
        bool ret = context.shareMemoryBlock->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            int retval = pluginData->ParseFromArray(reinterpret_cast<const char*>(data), size);
            CHECK_TRUE(retval, false, "parse %d bytes failed!", size);
            return true;
        });
        if (!ret) {
            break;
        }
        if (!context.profilerDataRepeater->PutPluginData(pluginData)) {
            break;
        }
    }
}

bool PluginService::AppendResult(NotifyResultRequest& request)
{
    pluginCommandBuilder_->GetedCommandResponse(request.command_id());
    auto sem = GetSemaphore(request.command_id());
    if (sem) {
        sem->Post();
    }

    int size = request.result_size();
    HILOG_DEBUG(LOG_CORE, "AppendResult size:%d, cmd id:%d", size, request.command_id());
    for (int i = 0; i < size; i++) {
        PluginResult pr = request.result(i);
        if (pr.data().size() > 0) {
            HILOG_DEBUG(LOG_CORE, "AppendResult Size : %zu", pr.data().size());
            uint32_t pluginId = pr.plugin_id();
            PluginContextPtr pluginCtx = GetPluginContextById(pluginId);
            CHECK_NOTNULL(pluginCtx, false, "plugin id %u not found!", pluginId);
            if (pluginCtx->profilerDataRepeater == nullptr) {
                HILOG_DEBUG(LOG_CORE, "AppendResult profilerDataRepeater==nullptr %s %d", pr.status().name().c_str(),
                            pluginId);
                return false;
            }
            auto pluginData = std::make_shared<ProfilerPluginData>();
            pluginData->set_name(pr.status().name());
            pluginData->set_status(0);
            pluginData->set_data(pr.data());
            if (!pluginCtx->profilerDataRepeater->PutPluginData(pluginData)) {
                return false;
            }
        } else if (pr.out_file_name() != "") { // updata plugin outFileName
            std::unique_lock<std::mutex> lock(mutex_);
            auto pluginId = pr.plugin_id();
            CHECK_TRUE(pluginContext_.count(pluginId) > 0, false, "plugin id %u not found!", pluginId);
            pluginContext_[pluginId]->outFileName = pr.out_file_name();
        } else {
            HILOG_DEBUG(LOG_CORE, "Flush?Data From ShareMemory?");
        }
    }
    return true;
}

std::vector<ProfilerPluginState> PluginService::GetPluginStatus()
{
    std::vector<ProfilerPluginState> status;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& entry : pluginContext_) {
        status.push_back(entry.second->profilerPluginState);
    }
    return status;
}

uint32_t PluginService::GetPluginIdByName(std::string name)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (nameIndex_.find(name) == nameIndex_.end()) {
        return 0;
    }
    return nameIndex_[name];
}

void PluginService::SetTraceWriter(const TraceFileWriterPtr& traceWriter)
{
    traceWriter_ = traceWriter;
}