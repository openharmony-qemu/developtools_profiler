/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class ProfilerPluginConfig;
class PluginResult;
class CommandPoller;

class ManagerInterface {
public:
    virtual ~ManagerInterface() {}

    virtual bool LoadPlugin(const std::string& pluginPath) = 0;
    virtual bool UnloadPlugin(const std::string& pluginPath) = 0;
    virtual bool UnloadPlugin(const uint32_t pluginId) = 0;

    // CommandPoller will call the following four interfaces after receiving the command
    virtual bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config) = 0;
    virtual bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds) = 0;
    virtual bool StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                        const std::vector<ProfilerPluginConfig>& config, PluginResult& result) = 0;
    virtual bool StopPluginSession(const std::vector<uint32_t>& pluginIds) = 0;
    virtual bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds) = 0;

    virtual bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd) = 0;
    virtual bool ResetWriter(uint32_t pluginId) = 0;
    virtual void SetCommandPoller(const std::shared_ptr<CommandPoller>& p) = 0;
};

#endif // PLUGIN_INTERFACE_H