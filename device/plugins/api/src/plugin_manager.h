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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "manager_interface.h"
#include "plugin_module.h"
#include "schedule_task_manager.h"

class ProfilerPluginConfig;
class PluginResult;
class CommandPoller;

using CommandPollerPtr = STD_PTR(shared, CommandPoller);
using PluginModulePtr = STD_PTR(shared, PluginModule);

class PluginManager : public ManagerInterface {
public:
    virtual ~PluginManager();
    bool AddPlugin(const std::string& pluginPath);
    bool RemovePlugin(const std::string& pluginPath);

    bool LoadPlugin(const std::string& pluginName);
    bool UnloadPlugin(const std::string& pluginName);
    bool UnloadPlugin(const uint32_t pluginId);

    // CommandPoller will call the following four interfaces after receiving the command
    bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config);
    bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds);
    bool StartPluginSession(const std::vector<uint32_t>& pluginIds, const std::vector<ProfilerPluginConfig>& config,
                                PluginResult& result);
    bool StopPluginSession(const std::vector<uint32_t>& pluginIds);
    bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds);

    // call the 'PluginModule::ReportResult' and 'PluginManager::SubmitResult' according to 'pluginId'
    // creat PluginResult for  current plug-in inside
    bool PullResult(uint32_t pluginId);
    bool StopAllPluginSession();

    // for test
    virtual bool SubmitResult(const PluginResult& pluginResult);
    bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd);
    bool ResetWriter(uint32_t pluginId);
    void SetCommandPoller(const CommandPollerPtr& p);
    bool RegisterPlugin(const PluginModulePtr& plugin, const std::string& pluginPath,
                        const PluginModuleInfo& pluginInfo);

private:
    std::map<uint32_t, std::shared_ptr<PluginModule>> pluginModules_;
    std::map<std::string, uint32_t> pluginIds_; // pluginName maps to pluginId
    CommandPollerPtr commandPoller_;
    ScheduleTaskManager scheduleTaskManager_;
    std::map<std::string, std::string> pluginPathAndNameMap_;
};

#endif // PLUGIN_MANAGER_H
