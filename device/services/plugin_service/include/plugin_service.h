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

#ifndef PLUGIN_SERVICE_H
#define PLUGIN_SERVICE_H

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "common_types.pb.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"
#include "i_semaphore.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "profiler_service_types.pb.h"
#include "service_entry.h"
#include "trace_file_writer.h"

class PluginServiceImpl;
class ProfilerDataRepeater;
class SocketContext;
class ShareMemoryBlock;
class PluginCommandBuilder;
class PluginSessionManager;

using ProfilerDataRepeaterPtr = STD_PTR(shared, ProfilerDataRepeater);
using ProfilerPluginStatePtr = STD_PTR(shared, ProfilerPluginState);

struct PluginInfo {
    uint32_t id = 0;
    std::string name;
    std::string path;
    std::string sha256;
    uint32_t bufferSizeHint;
    bool isStandaloneFileData = false;
    std::string outFileName = "";
    std::string pluginVersion = "";
    SocketContext* context = nullptr;
};

struct PluginContext {
    std::string name;
    std::string path;
    std::string sha256;
    uint32_t bufferSizeHint;
    bool isStandaloneFileData = false;
    std::string outFileName = "";
    std::string pluginVersion = "";
    SocketContext* context = nullptr;
    ProfilerPluginConfig config;
    ProfilerDataRepeaterPtr profilerDataRepeater;
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock;
    EventNotifierPtr eventNotifier;
    ProfilerPluginState profilerPluginState;
};

using PluginContextPtr = std::shared_ptr<PluginContext>;

using PluginSessionManagerPtr = std::shared_ptr<PluginSessionManager>;

class PluginService {
public:
    PluginService();
    ~PluginService();

    bool CreatePluginSession(const ProfilerPluginConfig& pluginConfig,
                             const ProfilerSessionConfig::BufferConfig& bufferConfig,
                             const ProfilerDataRepeaterPtr& dataRepeater);
    bool CreatePluginSession(const ProfilerPluginConfig& pluginConfig, const ProfilerDataRepeaterPtr& dataRepeater);
    bool StartPluginSession(const ProfilerPluginConfig& config);
    bool StopPluginSession(const std::string& pluginName);
    bool DestroyPluginSession(const std::string& pluginName);
    bool RefreshPluginSession(const std::string& pluginName);

    bool AddPluginInfo(const PluginInfo& pluginInfo);
    bool GetPluginInfo(const std::string& pluginName, PluginInfo& pluginInfo);
    bool RemovePluginInfo(const PluginInfo& pluginInfo);

    bool AppendResult(NotifyResultRequest& request);

    std::vector<ProfilerPluginState> GetPluginStatus();
    uint32_t GetPluginIdByName(std::string name);

    void SetPluginSessionManager(const PluginSessionManagerPtr& pluginSessionManager);
    void SetProfilerSessionConfig(const ProfilerSessionConfig& profilerSessionConfig);

    std::pair<uint32_t, PluginContextPtr> GetPluginContext(const std::string& pluginName);
    void SetTraceWriter(const TraceFileWriterPtr& traceWriter);

private:
    bool StartService(const std::string& unixSocketName);

    SemaphorePtr GetSemaphore(uint32_t) const;
    void ReadShareMemoryOffline(PluginContext&);
    void ReadShareMemoryOnline(PluginContext&);
    PluginContextPtr GetPluginContextById(uint32_t id);

    bool RemovePluginSessionCtx(const std::string& pluginName);

    mutable std::mutex mutex_;
    std::map<uint32_t, PluginContextPtr> pluginContext_;
    std::map<uint32_t, SemaphorePtr> waitSemphores_;
    std::map<std::string, uint32_t> nameIndex_;

    std::atomic<uint32_t> pluginIdCounter_;
    std::shared_ptr<ServiceEntry> serviceEntry_;
    std::shared_ptr<PluginServiceImpl> pluginServiceImpl_;
    std::shared_ptr<PluginCommandBuilder> pluginCommandBuilder_;
    std::unique_ptr<EpollEventPoller> eventPoller_;
    PluginSessionManagerPtr pluginSessionManager_;
    ProfilerSessionConfig profilerSessionConfig_;
    TraceFileWriterPtr traceWriter_ = nullptr;
    uint32_t dataFlushSize_ = 0;
};

#endif // PLUGIN_SERVICE_H