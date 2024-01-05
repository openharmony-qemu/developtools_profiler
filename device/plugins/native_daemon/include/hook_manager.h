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

#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "buffer_writer.h"
#include "manager_interface.h"
#include "epoll_event_poller.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"
#include "native_hook_config.pb.h"
#include "native_hook_result.pb.h"
#include "virtual_runtime.h"
#include "stack_data_repeater.h"
#include "stack_preprocess.h"
#include "native_memory_profiler_sa_config.h"

using BatchNativeHookDataPtr = STD_PTR(shared, BatchNativeHookData);
class ProfilerPluginConfig;
class PluginResult;
class CommandPoller;

struct HookContext {
    int type;
    pid_t pid;
    pid_t tid;
    void* addr;
    uint32_t mallocSize;
};

namespace OHOS::Developtools::NativeDaemon {
class HookService;
class HookManager : public ManagerInterface, public std::enable_shared_from_this<HookManager> {
public:
    struct HookManagerCtx {
        HookManagerCtx(int32_t pid) : pid(pid) {}
        HookManagerCtx(const std::string& name) : processName(name) {}
        ~HookManagerCtx() {}
        int32_t pid = -1;
        std::string processName;
        std::string smbName;
        std::shared_ptr<ShareMemoryBlock> shareMemoryBlock = nullptr;
        std::shared_ptr<EventNotifier> eventNotifier = nullptr;
        std::unique_ptr<EpollEventPoller> eventPoller = nullptr;
        std::shared_ptr<StackDataRepeater> stackData = nullptr;
        std::shared_ptr<StackPreprocess> stackPreprocess = nullptr;
        bool isRecordAccurately = false;
    };
    HookManager() = default;
    bool RegisterAgentPlugin(const std::string& pluginPath);
    bool UnregisterAgentPlugin(const std::string& pluginPath);

    bool LoadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const uint32_t pluginId) override;

    // CommandPoller will call the following four interfaces after receiving the command
    bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config) override;
    bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds) override;
    bool StartPluginSession(const std::vector<uint32_t>& pluginIds,
                            const std::vector<ProfilerPluginConfig>& config, PluginResult& result) override;
    bool StopPluginSession(const std::vector<uint32_t>& pluginIds) override;
    bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds) override;

    bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
                        bool isProtobufSerialize = true) override;
    bool ResetWriter(uint32_t pluginId) override;
    void SetCommandPoller(const std::shared_ptr<CommandPoller>& p) override;
    void ResetStartupParam();
    void SethookStandalone(bool);
    bool HandleHookContext(const std::shared_ptr<HookManagerCtx>& ctx);
    void StartPluginSession();
    void ReadShareMemory(const std::shared_ptr<HookManagerCtx>& hookCtx);
    void SetHookConfig(const NativeHookConfig& hookConfig);
    void SetHookConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    int32_t CreatePluginSession();
    void RegisterWriter(const std::shared_ptr<Writer> writer);
    void WriteHookConfig();
    std::pair<int, int> GetFds(int32_t pid, const std::string& name);
    inline void SetSaServiceFlag(bool flag)
    {
        isSaService_ = flag;
    }
    void GetClientConfig(ClientConfig& clientConfig);

private:
    bool CheckProcess();
    bool CheckProcessName();
    void SetHookData(HookContext& hookContext, struct timespec ts,
        std::vector<OHOS::Developtools::NativeDaemon::CallFrame>& callFrames,
        BatchNativeHookDataPtr& batchNativeHookData);

    std::shared_ptr<HookService> hookService_;
    std::shared_ptr<CommandPoller> commandPoller_;
    int agentIndex_ = -1;
    std::string agentPluginName_;
    NativeHookConfig hookConfig_;
    std::unique_ptr<uint8_t[]> buffer_;
    bool isHookStandalone_ {false};
    FILE* fpHookData_ {nullptr};
    std::vector<std::shared_ptr<HookManagerCtx>> hookCtx_;
    bool isSaService_{false};
    bool noDataQueue_{false};
};
}
#endif // AGENT_MANAGER_H