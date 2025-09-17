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

#ifndef FFRT_PROFILER_MANAGER_H
#define FFRT_PROFILER_MANAGER_H

#include <thread>
#include <memory>
#include <ctime>

#include "ffrt_profiler_socket_service.h"
#include "manager_interface.h"
#include "epoll_event_poller.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"
#include "ffrt_profiler_config.pb.h"
#include "buffer_writer.h"
#include "writer_adapter.h"
#include "ffrt_profiler_handle.h"

namespace OHOS::Developtools::Profiler {
class FfrtProfilerManager : public ManagerInterface, public std::enable_shared_from_this<FfrtProfilerManager> {
public:
    struct FfrtProfilerCtx {
        FfrtProfilerCtx(int32_t pid) : pid(pid) {}
        FfrtProfilerCtx(const std::string& name) : processName(name) {}
        FfrtProfilerCtx(int32_t pid, const std::string& name, bool restart = false)
            : pid(pid), processName(name), restart(restart) {}
        ~FfrtProfilerCtx() {}
        int32_t pid = -1;
        std::string processName;
        std::string smbName;
        std::shared_ptr<ShareMemoryBlock> shareMemoryBlock = nullptr;
        std::shared_ptr<EventNotifier> eventNotifier = nullptr;
        std::unique_ptr<EpollEventPoller> eventPoller = nullptr;
        std::shared_ptr<FfrtProfilerHandle> handle = nullptr;
        bool restart = false;
    };

    FfrtProfilerManager();
    ~FfrtProfilerManager();
#ifdef UNIT_TEST
    void SetConfig(FfrtProfilerConfig& config)
    {
        config_ = config;
        return;
    }
#endif
    void Init();
    bool StartFfrtProfiler();
    void StopFfrtProfiler();
    void ReadShareMemory(std::shared_ptr<FfrtProfilerCtx> ctx);
    std::pair<int, int> GetFfrtProfilerCtx(int32_t pid = 0, const std::string& name = "");

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

private:
    bool CheckConfig();
    bool HandleFfrtProfilerContext(const std::shared_ptr<FfrtProfilerCtx>& ctx);
    clockid_t GetClockId(FfrtProfilerConfig::ClockId clockType);
    bool CheckPid(std::set<int32_t>& pidCache);
    bool CheckStartupProcessName();
    bool CheckRestartProcessName(std::set<int32_t>& pidCache);

private:
    std::shared_ptr<FfrtProfilerSocketService> socketService_{nullptr};
    std::vector<std::shared_ptr<FfrtProfilerCtx>> ffrtCtx_;
    FfrtProfilerConfig config_;
    std::string paramValue_;
    std::shared_ptr<Writer> writer_{nullptr};
    std::shared_ptr<WriterAdapter> writerAdapter_{nullptr};
    bool isProtobufSerialize_{true};
    std::shared_ptr<CommandPoller> commandPoller_{nullptr};
    int agentIndex_ = -1;
};
} // namespace OHOS::Developtools::Profiler

#endif // FFRT_PROFILER_MANAGER_H