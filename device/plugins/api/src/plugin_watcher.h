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

#ifndef PLUGIN_WATCHER_H
#define PLUGIN_WATCHER_H

#include <logging.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
class PluginManager;

using PluginManagerPtr = STD_PTR(shared, PluginManager);

class PluginWatcher {
public:
    explicit PluginWatcher(const PluginManagerPtr& pluginManager);
    ~PluginWatcher();
    bool ScanPlugins(const std::string& pluginDir);
    bool WatchPlugins(const std::string& pluginDir);

private:
    int inotifyFd_;
    std::weak_ptr<PluginManager> pluginManager_;
    std::map<int, std::string> wdToDir_;
    std::thread monitorThread_;
    std::mutex mtx_;
    bool runMonitor_;
    virtual void OnPluginAdded(const std::string& pluginPath);
    virtual void OnPluginRemoved(const std::string& pluginPath);
    void Monitor();
    bool MonitorIsSet();
};

#endif // !PLUGIN_WATCHER_H
