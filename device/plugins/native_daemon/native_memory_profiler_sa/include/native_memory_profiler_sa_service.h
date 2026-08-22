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
#include "singleton.h"

namespace OHOS::Developtools::NativeDaemon {

struct TaskConfig {
    TaskConfig(std::shared_ptr<HookManager> manager, int32_t pid, const std::string& processName, int32_t callingPid,
        const std::string& filePath, int32_t timerFd, bool startupMode, uint32_t fd = 0, uint32_t startType = 0)
        : hookMgr(manager), pid(pid), processName(processName), callingPid(callingPid), filePath(filePath),
        timerFd(timerFd), isStartupMode(startupMode), fd(fd), startType(startType) {}
    ~TaskConfig() {};
    std::shared_ptr<HookManager> hookMgr = nullptr;
    int32_t pid{0};
    std::string processName;
    int32_t callingPid{0};
    std::string filePath;
    int32_t timerFd{-1};
    bool isStartupMode{false};
    bool compositeHeapEnable{false};
    uint32_t fd{0};
    uint32_t startType{0}; // 0:Hiveiew 1:API
    std::string typeFileName;   // for API trace file move
    sptr<IRemoteObject> callback;
};

struct MemoryInfo {
    uint64_t totalMem;
    uint64_t usedMem;
    uint64_t freeMem;
    uint64_t availableMem;
    double availablePercent;
};

struct QuotaRecord {
    long long timestamp;
    int32_t count;
};

class NativeMemoryProfilerSaService : public SystemAbility, public NativeMemoryProfilerSaStub, public ServiceBase {
    DECLARE_SYSTEM_ABILITY(NativeMemoryProfilerSaService);
    DECLARE_DELAYED_SINGLETON(NativeMemoryProfilerSaService);
public:
    static bool StartServiceAbility();
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;
    int32_t StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override;
    int32_t Stop(uint32_t pid) override;
    int32_t Stop(const std::string& name) override;
    int32_t Stop() override;
    int32_t StopSync() override;
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;
    int32_t DumpDataAsync(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override;
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply) override;
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                  std::string& replyStats) override { return 0; }
    static void ClientDisconnectCallback(int socketFd);
    static bool HasProfilingPermission();
    static void CloseSocketFd(int pid);
    static void DelayedShutdown(bool cancel);
    virtual void OnStart() override;
    virtual void OnStop() override;

private:
    int32_t StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd = 0);
    int32_t StartHookLock(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                          uint32_t& fd, std::shared_ptr<HookManager>& hook, std::string& args,
                          const sptr<IRemoteObject>& callback = nullptr);
    int32_t StartHook(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd, MessageParcel &reply);
    static int32_t StopHook(uint32_t pid, std::string name = "",
        int32_t finishErrorCode = static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ERROR), bool innerCall = false);
    static int32_t StopHookAsync(uint32_t pid, int32_t finishErrorCode, bool innerCall = false);
    static int32_t StopHookTask(uint32_t pid, std::shared_ptr<TaskConfig> config, int32_t finishErrorCode);
    static int32_t StopHookByType(uint32_t pid, std::string& name, int32_t finishErrorCode, bool isAsync,
        bool innerCall = false);
    static bool TryBeginStopHook(uint32_t pid);
    static void EndStopHook(uint32_t pid);
    static bool IsAsyncRequest(uint32_t pid);
    static bool CheckStopPermission(int32_t callingPid, std::shared_ptr<TaskConfig>& config);
    int32_t CheckConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fd = 0);
    int32_t CheckTaskLimit(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    int32_t CheckStartupModeConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    int32_t CheckPidNameConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    int32_t CheckFilePath(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    void FillTaskConfigContext(int32_t pid, const std::string& name);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    std::string GetCmdArgs(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    void RefreshFileCacheModeConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    void SetGpuConfig(ClientConfig& clientConfig);
    // 统一应用 config 中的 NMD/memtrace 侧效应，减少 Start/DumpData 重复分支
    bool ApplyNmdConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, uint32_t fdForNmd = 0);
    void ApplyMemtraceConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static bool GetSystemMemoryInfo(MemoryInfo* memInfo);
    static bool GetSystemCpuUsageInfo(double* cpuUsage);
    static bool ParseCpuStats(const std::string& content, unsigned long& total, unsigned long& idle);
    static int32_t CheckSystemResource();
    static bool IsNativeDeamonRunning();
    bool ReceiveCacheFileMsg(SocketContext &context, const int8_t *buf);
    static int32_t GetTaskNum(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t PidAndNameConflict(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static void NotifyProfilingResult(const std::shared_ptr<TaskConfig>& config, int32_t errorCode,
        const std::string& filePath);
    static std::string CollectApm(std::shared_ptr<TaskConfig>& config);
    static int32_t CopyApiCollectorFile(uint32_t pid, std::shared_ptr<TaskConfig> config, std::string& resultPath);
    static int32_t ReleaseForStopHook(uint32_t pid, std::shared_ptr<TaskConfig> config, std::string& resultPath);
    int32_t PreCheckPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t GetMemProfilerUseCountToday(
        const std::unordered_map<std::string, QuotaRecord>& records, std::string name = "");
    static int32_t GetMemProfilerTotalUseCountToday(
        const std::unordered_map<std::string, QuotaRecord>& records);
    static bool UpdateMemProfilerUserCount(
        std::unordered_map<std::string, QuotaRecord>& records, std::string name = "");
    static bool RollbackMemProfilerUserCount(bool isDebug = false, std::string name = "");
    static bool ReadQuotaFile(std::unordered_map<std::string, QuotaRecord>& records);
    static bool WriteQuotaFile(const std::unordered_map<std::string, QuotaRecord>& records);
    static bool IsSeparateByMoreThan24Hours(long long t1, long long t2);
#ifdef NATIVE_DAEMON_UNITTEST
    static void ResetTaskContextsForTest();
    static void InjectPidInfoForTest(pid_t pid, uid_t uid, gid_t gid);
    static void InjectPidCtxForTest(int32_t pid, const std::shared_ptr<TaskConfig>& config);
    static void InjectPidFdsForTest(pid_t pid, int fd);
    static void SetHasStartupModeForTest(bool val);
    static void SetMockPermissionForTest(bool isUserMode, bool granted);
    static void SetMockNativeDaemonRunningForTest(bool running);
    static void SetMockStartServiceAbilityForTest(bool result);
    static void SetMockSystemResourceForTest(bool memFail, bool cpuFail, bool romFail);
    static void SetMockSendFailForTest(bool fail);
    static void SetMockNmdResultForTest(const std::string& result);
#endif

private:
    static bool IsMemoryPressureCritical(uint64_t availableMem, bool memReadOk);
    static bool IsCpuOverloaded(double cpuUsage, bool cpuReadOk);
    static bool IsRomInsufficient(uint64_t freeBytes, uint64_t totalBytes, bool romReadOk);
    std::mutex gpuMtx_;
    std::string startupModeProcessName_;
    int32_t taskMaxNum_ = 2;
    int32_t globalTaskMaxNum_ = 4;
    std::atomic<bool> started_ = false;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_H
