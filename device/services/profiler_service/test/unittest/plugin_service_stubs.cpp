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
#define LOG_TAG "PluginServiceStub"
#include "plugin_service_stubs.h"

#include <map>
#include <memory>
#include "logging.h"
#include "profiler_capability_manager.h"
#include "profiler_data_repeater.h"

#ifdef USE_PLUGIN_SERVICE_STUB
using PluginServiceStubPtr = STD_PTR(shared, PluginServiceStub);
PluginServiceStubPtr PluginServiceStub::GetInstance()
{
    static std::weak_ptr<PluginServiceStub> instance;
    auto stub = instance.lock();
    if (stub) {
        return stub;
    }
    stub = std::make_shared<PluginServiceStub>();
    instance = stub;
    return stub;
}

void PluginServiceStub::SetCreateResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    createResult_ = value;
}

bool PluginServiceStub::GetCreateResult() const
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, createResult_);
    return createResult_;
}

void PluginServiceStub::SetStartResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    startResult_ = value;
}

bool PluginServiceStub::GetStartResult() const
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, startResult_);
    return startResult_;
}

void PluginServiceStub::SetStopResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    stopResult_ = value;
}

bool PluginServiceStub::GetStopResult() const
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, stopResult_);
    return stopResult_;
}

void PluginServiceStub::SetDestroyResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    destroyResult_ = value;
}

bool PluginServiceStub::GetDestroyResult() const
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, destroyResult_);
    return destroyResult_;
}

void PluginServiceStub::SetAddResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    addResult_ = value;
}

bool PluginServiceStub::GetAddResult()
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, addResult_);
    return addResult_;
}

void PluginServiceStub::SetRemoveResult(bool value)
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, value);
    removeResult_ = value;
}

bool PluginServiceStub::GetRemoveResult()
{
    HILOG_DEBUG(LOG_CORE, "%s(%d)", __FUNCTION__, removeResult_);
    return removeResult_;
}
#endif

PluginService::PluginService()
{
    pluginIdCounter_ = 0;
    serviceEntry_ = nullptr;
    pluginServiceImpl_ = nullptr;
    pluginCommandBuilder_ = nullptr;
}

PluginService::~PluginService() {}

void PluginService::SetPluginSessionManager(const PluginSessionManagerPtr& pluginSessionManager)
{
    pluginSessionManager_ = pluginSessionManager;
}

void PluginService::SetProfilerSessionConfig(const ProfilerSessionConfig& profilerSessionConfig)
{
    profilerSessionConfig_ = profilerSessionConfig;
}

bool PluginService::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                        const ProfilerDataRepeaterPtr& dataRepeater)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::string pluginName = pluginConfig.name();
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    CHECK_NOTNULL(pluginCtx, false, "create PluginContext failed!");

    pluginCtx->profilerDataRepeater = dataRepeater;
    pluginCtx->shareMemoryBlock = nullptr;

    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);

    HILOG_DEBUG(LOG_CORE, "CreatePluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                        const ProfilerSessionConfig::BufferConfig& bufferConfig,
                                        const ProfilerDataRepeaterPtr& dataRepeater)
{
    return CreatePluginSession(pluginConfig, dataRepeater);
}

bool PluginService::StartPluginSession(const ProfilerPluginConfig& pluginConfig)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::string pluginName = pluginConfig.name();
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);

    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::IN_SESSION);
    HILOG_INFO(LOG_CORE, "StartPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::StopPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);

    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);

    HILOG_DEBUG(LOG_CORE, "StopPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::DestroyPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);

    if (pluginCtx->shareMemoryBlock) {
        pluginCtx->shareMemoryBlock = nullptr;
    }

    if (pluginCtx->eventNotifier) {
        eventPoller_->RemoveFileDescriptor(pluginCtx->eventNotifier->GetFd());
        pluginCtx->eventNotifier = nullptr;
    }

    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::REGISTERED);
    HILOG_INFO(LOG_CORE, "DestroyPluginSession %s done!", pluginName.c_str());
    return true;
}

bool PluginService::RefreshPluginSession(const std::string& pluginName)
{
    uint32_t pluginId = 0;
    PluginContextPtr pluginCtx = nullptr;
    std::tie(pluginId, pluginCtx) = GetPluginContext(pluginName);
    HILOG_INFO(LOG_CORE, "RefreshPluginSession %s done!", pluginName.c_str());
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

    std::unique_lock<std::mutex> lock(mutex_);
    nameIndex_.erase(pluginName);
    pluginContext_.erase(pluginId);
    HILOG_DEBUG(LOG_CORE, "RemovePluginInfo for %s done!", pluginName.c_str());
    return true;
}

void PluginService::SetTraceWriter(const TraceFileWriterPtr& traceWriter)
{
    traceWriter_ = traceWriter;
}