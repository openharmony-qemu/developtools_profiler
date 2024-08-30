/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NATIVE_MEMORY_PROFILER_SA_SERVICE_H
#define NATIVE_MEMORY_PROFILER_SA_SERVICE_H

#include <unordered_map>
#include <mutex>

#include "iremote_object.h"
#include "system_ability.h"
#include "hook_manager.h"
#include "native_memory_profiler_sa_stub.h"
#include "schedule_task_manager.h"
#include "service_entry.h"

namespace OHOS::Developtools::NativeDaemon {
class NativeMemoryProfilerSaService : public SystemAbility, public NativeMemoryProfilerSaStub, public ServiceBase {
    DECLARE_SYSTEM_ABILITY(NativeMemoryProfilerSaService);
public:
    NativeMemoryProfilerSaService();
    ~NativeMemoryProfilerSaService();
    static bool StartServiceAbility();
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;
    int32_t Stop(uint32_t pid) override;
    int32_t Stop(const std::string& name) override;
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;

private:
    void StopHook(uint32_t pid, std::string name = "", bool timeout = false);
    int32_t StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd = 0);
    struct TaskConfig;
    bool CheckConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd = 0);
    void FillTaskConfigContext(int32_t pid, const std::string& name);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    void DelayedShutdown(bool cancel);

private:
    struct TaskConfig {
        TaskConfig(std::shared_ptr<HookManager> manager, int32_t pid, const std::string& processName,
            const std::string& filePath, int32_t timerFd, bool startupMode, uint32_t fd = 0)
            : hookMgr(manager), pid(pid), processName(processName), filePath(filePath), timerFd(timerFd),
              isStartupMode(startupMode), fd(fd) {}
        ~TaskConfig() {};
        std::shared_ptr<HookManager> hookMgr = nullptr;
        int32_t pid{0};
        std::string processName;
        std::string filePath;
        int32_t timerFd{-1};
        bool isStartupMode{false};
        uint32_t fd{0};
    };
    bool HasProfilingPermission();
    std::unordered_map<std::string, std::shared_ptr<TaskConfig>> nameAndFilePathCtx_;
    std::unordered_map<int32_t, std::shared_ptr<TaskConfig>> pidCtx_;
    std::mutex mtx_;
    std::mutex nmdMtx_;
    ScheduleTaskManager scheduleTaskManager_;
    bool hasStartupMode_{false};
    std::string startupModeProcessName_;
    int32_t taskNum_{0};
    std::shared_ptr<ServiceEntry> serviceEntry_{nullptr};
    int32_t delayedShutdownTimerFd_{-1};
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> nmdPidType_;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_H