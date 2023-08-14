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

#include "plugin_session_manager.h"

namespace {
constexpr uint32_t MAX_BUFFER_PAGES = 512 * 1024 * 1024 / 4096;
}

PluginSessionManager::PluginSessionManager(const PluginServiceWeakPtr& pluginService) : pluginService_(pluginService) {}

PluginSessionManager::~PluginSessionManager() {}

bool PluginSessionManager::CheckBufferConfig(const ProfilerSessionConfig::BufferConfig& bufferConfig)
{
    const uint32_t pages = bufferConfig.pages();
    const auto policy = bufferConfig.policy();
    return pages > 0 && pages <= MAX_BUFFER_PAGES &&
           (policy == ProfilerSessionConfig::BufferConfig::RECYCLE ||
            policy == ProfilerSessionConfig::BufferConfig::FLATTEN);
}

bool PluginSessionManager::CheckPluginSha256(const ProfilerPluginConfig& pluginConfig)
{
    std::string reqSha = pluginConfig.plugin_sha256();
    if (reqSha.size() > 0) { // only check when SHA256 provided in request
        auto pluginSvc = pluginService_.lock();
        CHECK_NOTNULL(pluginSvc, false, "plugin service null!");

        PluginInfo info = {};
        std::string name = pluginConfig.name();
        CHECK_TRUE(pluginSvc->GetPluginInfo(name, info), false, "get plugin info %s failed!", name.c_str());

        std::string devSha = info.sha256;
        CHECK_TRUE(devSha == reqSha, false, "SHA256 mismatch: %s, %s!", devSha.c_str(), reqSha.c_str());
    }
    return true;
}

PluginSessionPtr PluginSessionManager::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                                           const ProfilerSessionConfig::BufferConfig& bufferConfig,
                                                           const ProfilerDataRepeaterPtr& dataRepeater)
{
    auto name = pluginConfig.name();
    CHECK_TRUE(pluginSessions_.count(name) == 0, nullptr, "plugin name %s exists!", name.c_str());
    CHECK_TRUE(CheckBufferConfig(bufferConfig), nullptr, "buffer config invalid!");
    CHECK_TRUE(CheckPluginSha256(pluginConfig), nullptr, "SHA256 check failed!");

    auto session = std::make_shared<PluginSession>(pluginConfig, bufferConfig, pluginService_, dataRepeater);
    CHECK_NOTNULL(session, nullptr, "allocate plugin session for %s failed!", name.c_str());
    CHECK_TRUE(session->IsAvailable(), nullptr, "config plugin for %s failed!", name.c_str());
    return session;
}

PluginSessionPtr PluginSessionManager::CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                                           const ProfilerDataRepeaterPtr& dataRepeater)
{
    auto name = pluginConfig.name();
    CHECK_TRUE(pluginSessions_.count(name) == 0, nullptr, "plugin name %s exists!", name.c_str());
    CHECK_TRUE(CheckPluginSha256(pluginConfig), nullptr, "SHA256 check failed!");

    auto session = std::make_shared<PluginSession>(pluginConfig, pluginService_, dataRepeater);
    CHECK_NOTNULL(session, nullptr, "allocate plugin session for %s failed!", name.c_str());
    CHECK_TRUE(session->IsAvailable(), nullptr, "config plugin for %s failed!", name.c_str());
    return session;
}

bool PluginSessionManager::CreatePluginSessions(const std::vector<ProfilerPluginConfig>& pluginConfigs,
                                                const std::vector<ProfilerSessionConfig::BufferConfig>& bufferConfigs,
                                                const ProfilerDataRepeaterPtr& dataRepeater)
{
    CHECK_TRUE(pluginConfigs.size() == bufferConfigs.size(), false, "buffer and config vector size mismatch %zu, %zu",
               bufferConfigs.size(), pluginConfigs.size());

    std::map<std::string, PluginSessionPtr> tmpPluginSessions;
    for (size_t i = 0; i < pluginConfigs.size(); i++) {
        auto session = CreatePluginSession(pluginConfigs[i], bufferConfigs[i], dataRepeater);

        // tmpPluginSessions will released if some plugin session create failed
        CHECK_NOTNULL(session, false, "create plugin-%zu session failed!", i);
        tmpPluginSessions[pluginConfigs[i].name()] = session;
    }

    // insert all plugin session to pluginSessions_ map
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& entry : tmpPluginSessions) {
        pluginSessions_.insert(entry);
    }
    return true;
}

bool PluginSessionManager::CreatePluginSessions(const std::vector<ProfilerPluginConfig>& pluginConfigs,
                                                const ProfilerDataRepeaterPtr& dataRepeater)
{
    std::map<std::string, PluginSessionPtr> tmpPluginSessions;
    for (size_t i = 0; i < pluginConfigs.size(); i++) {
        auto session = CreatePluginSession(pluginConfigs[i], dataRepeater);

        // tmpPluginSessions will released if some plugin session create failed
        CHECK_NOTNULL(session, false, "create plugin-%zu session without shmem buffer failed!", i);
        tmpPluginSessions[pluginConfigs[i].name()] = session;
    }

    // insert all plugin session to pluginSessions_ map
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& entry : tmpPluginSessions) {
        pluginSessions_.insert(entry);
    }
    return true;
}

bool PluginSessionManager::RemovePluginSessions(const std::vector<std::string>& nameList)
{
    CHECK_TRUE(nameList.size() > 0, false, "nameList empty!");
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        pluginSessions_.erase(it);
    }
    return true;
}

bool PluginSessionManager::InvalidatePluginSessions(const std::vector<std::string>& nameList)
{
    CHECK_TRUE(nameList.size() > 0, false, "nameList empty!");
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        it->second->Invalidate();
    }
    return true;
}

bool PluginSessionManager::StartPluginSessions(const std::vector<std::string>& nameList)
{
    CHECK_TRUE(nameList.size() > 0, false, "nameList empty!");
    pluginNameList_ = std::move(nameList);
    // start each plugin sessions
    size_t failureCount = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        if (!it->second->Start()) {
            HILOG_INFO(LOG_CORE, "start session %s FAILED!", it->first.c_str());
            failureCount++;
        }
    }
    return failureCount == 0;
}

bool PluginSessionManager::StopPluginSessions(const std::vector<std::string>& nameList)
{
    // stop each plugin sessions
    size_t failureCount = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        if (!it->second->Stop()) {
            HILOG_INFO(LOG_CORE, "stop session %s FAILED!", it->first.c_str());
            failureCount++;
        }
    }
    return failureCount == 0;
}

bool PluginSessionManager::RefreshPluginSession()
{
    size_t failureCount = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : pluginNameList_) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        if (!it->second->Refresh()) {
            HILOG_INFO(LOG_CORE, "refresh data %s FAILED!", it->first.c_str());
            failureCount++;
        }
    }
    return failureCount == 0;
}

bool PluginSessionManager::CheckStatus(const std::vector<std::string>& nameList, PluginSession::State state)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        auto actualState = it->second->GetState();
        if (actualState != state) {
            HILOG_INFO(LOG_CORE, "plugin %s state is %d not %d!", it->first.c_str(), actualState, state);
            return false;
        }
    }
    return true;
}

std::vector<PluginSession::State> PluginSessionManager::GetStatus(const std::vector<std::string>& nameList)
{
    std::vector<PluginSession::State> status;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& name : nameList) {
        auto it = pluginSessions_.find(name);
        if (it == pluginSessions_.end()) {
            continue;
        }
        status.push_back(it->second->GetState());
    }
    return status;
}
