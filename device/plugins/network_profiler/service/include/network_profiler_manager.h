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

#ifndef NETWORK_PROFILER_MANAGER_H
#define NETWORK_PROFILER_MANAGER_H

#include <thread>
#include <memory>
#include <ctime>

#include "network_profiler_socket_service.h"
#include "manager_interface.h"
#include "epoll_event_poller.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"
#include "network_profiler_config.pb.h"
#include "buffer_writer.h"
#include "writer_adapter.h"
#include "network_profiler_handle.h"

namespace OHOS::Developtools::Profiler {
class NetworkProfilerManager : public ManagerInterface, public std::enable_shared_from_this<NetworkProfilerManager> {
public:
    struct NetworkProfilerCtx {
        NetworkProfilerCtx(int32_t pid) : pid(pid) {}
        NetworkProfilerCtx(const std::string& name) : processName(name) {}
        NetworkProfilerCtx(int32_t pid, const std::string& name, bool restart = false)
            : pid(pid), processName(name), restart(restart) {}
        ~NetworkProfilerCtx() {}
        int32_t pid = -1;
        std::string processName;
        std::string smbName;
        std::shared_ptr<ShareMemoryBlock> shareMemoryBlock = nullptr;
        std::shared_ptr<EventNotifier> eventNotifier = nullptr;
        std::unique_ptr<EpollEventPoller> eventPoller = nullptr;
        std::shared_ptr<NetworkProfilerHandle> handle = nullptr;
        bool restart = false;
    };

    NetworkProfilerManager();
    ~NetworkProfilerManager();
#ifdef NETWORK_UNITTEST
    void SetConfig(NetworkProfilerConfig& config)
    {
        config_ = config;
        return;
    }
#endif
    void Init();
    bool StartNetworkProfiler();
    void StopNetworkProfiler();
    void ReadShareMemory(std::shared_ptr<NetworkProfilerCtx> ctx);
    std::pair<int, int> GetNetworkProfilerCtx(int32_t pid = 0, const std::string& name = "");

    bool LoadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const uint32_t pluginId) override;
    bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config) override;
    bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds) override;
    bool StartPluginSession(const std::vector<uint32_t>& pluginIds, const std::vector<ProfilerPluginConfig>& config,
        PluginResult& result) override;
    bool StopPluginSession(const std::vector<uint32_t>& pluginIds) override;
    bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds) override;
    bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
        bool isProtobufSerialize = true) override;
    bool ResetWriter(uint32_t pluginId) override;
    void SetCommandPoller(const std::shared_ptr<CommandPoller>& p) override;
    bool RegisterAgentPlugin(const std::string& pluginPath);
    std::string GetCmdArgs(const NetworkProfilerConfig& traceConfig);

private:
    virtual bool CheckConfig();
    bool CheckConfigPid(std::set<int32_t>& pidCache);
    bool CheckStartupProcessName();
    bool CheckRestartProcessName(std::set<int32_t>& pidCache);
    virtual bool HandleNetworkProfilerContext(const std::shared_ptr<NetworkProfilerCtx>& ctx);
    clockid_t GetClockId(NetworkProfilerConfig::ClockId clockType);

private:
    std::shared_ptr<NetworkProfilerSocketService> socketService_{nullptr};
    std::vector<std::shared_ptr<NetworkProfilerCtx>> networkCtx_;
    NetworkProfilerConfig config_;
    std::string paramValue_;
    std::shared_ptr<Writer> writer_{nullptr};
    std::atomic<bool> firstData_ = true;
    std::atomic<uint32_t> firstPluginId_ = 0;
    std::shared_ptr<WriterAdapter> writerAdapter_{nullptr};
    bool isProtobufSerialize_{true};
    std::shared_ptr<CommandPoller> commandPoller_{nullptr};
    int agentIndex_ = -1;
};
} // namespace OHOS::Developtools::Profiler

#endif // NETWORK_PROFILER_MANAGER_H