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
#ifndef PLUGIN_SESSION_MANAGER_H
#define PLUGIN_SESSION_MANAGER_H

#include "plugin_service.h"
#include "plugin_session.h"

#include <mutex>

using PluginSessionPtr = std::shared_ptr<PluginSession>;

class PluginSessionManager {
public:
    PluginSessionManager(const PluginServiceWeakPtr& pluginService);
    ~PluginSessionManager();

    bool CreatePluginSessions(const std::vector<ProfilerPluginConfig>& pluginConfigs,
                              const std::vector<ProfilerSessionConfig::BufferConfig>& bufferConfigs,
                              const ProfilerDataRepeaterPtr& dataRepeater);

    bool CreatePluginSessions(const std::vector<ProfilerPluginConfig>& pluginConfigs,
                              const ProfilerDataRepeaterPtr& dataRepeater);

    bool RemovePluginSessions(const std::vector<std::string>& nameList);

    bool InvalidatePluginSessions(const std::vector<std::string>& nameList);

    bool StartPluginSessions(const std::vector<std::string>& nameList);
    bool StopPluginSessions(const std::vector<std::string>& nameList);
    bool RefreshPluginSession();

    bool CheckStatus(const std::vector<std::string>& nameList, PluginSession::State state);
    std::vector<PluginSession::State> GetStatus(const std::vector<std::string>& nameList);

private:
    bool CheckBufferConfig(const ProfilerSessionConfig::BufferConfig& bufferConfig);
    bool CheckPluginSha256(const ProfilerPluginConfig& pluginConfig);
    PluginSessionPtr CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                         const ProfilerSessionConfig::BufferConfig& bufferConfig,
                                         const ProfilerDataRepeaterPtr& dataRepeater);
    PluginSessionPtr CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                                         const ProfilerDataRepeaterPtr& dataRepeater);

private:
    std::mutex mutex_;
    PluginServiceWeakPtr pluginService_;

    // plugin session data:
    std::map<std::string, PluginSessionPtr> pluginSessions_;
    std::vector<std::string> pluginNameList_;
};

#endif // PLUGIN_SESSION_MANAGER_H