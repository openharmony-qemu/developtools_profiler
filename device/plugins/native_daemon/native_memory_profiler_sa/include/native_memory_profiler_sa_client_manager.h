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

#ifndef NATIVE_MEMORY_PROFILER_SA_CLIENT_MANAGER_H
#define NATIVE_MEMORY_PROFILER_SA_CLIENT_MANAGER_H

#include "i_native_memory_profiler_sa.h"
#include "mem_config.h"
#include "native_memory_profiler_sa_lazyload_callback.h"
#include <iservice_registry.h>
#include <map>
#include <memory>

namespace OHOS::Developtools::NativeDaemon {
using Range = std::pair<uint64_t, uint64_t>;

struct MemSaConfig {
    uint64_t mask = 0;
    std::map<std::string, std::pair<Range, Range>> hookSizes;
};

struct MemLeakConfig {
    uint32_t duration = 0;
    uint32_t limit = 0;
    uint32_t format = 0;
};

enum class InternalResourceType : int32_t {
    OH_RES_TYPE_FD = 0,
    OH_RES_TYPE_THREAD = 1,
    OH_RES_TYPE_NATIVE = 2,
    OH_RES_TYPE_GPU = 3,
    OH_RES_TYPE_GLOBAL_HANDLE = 4,
};

struct GwpAsanConfig {
    uint32_t holdTime = 0;
    uint32_t maxEntries = 0;
    uint32_t maxStackDepth = 0;
};

class NativeMemoryProfilerSaClientManager {
public:
    enum class NativeMemProfilerType : int32_t {
        MEM_PROFILER_LIBRARY,
        MEM_PROFILER_CALL_STACK,
    };
    static int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t Start(NativeMemProfilerType type, uint32_t pid, uint32_t duration, uint32_t sampleIntervel);
    static int32_t Stop(uint32_t pid);
    static int32_t Stop(const std::string& name);
    static int32_t Stop();
    static int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t GetMallocStats(int fd, int pid, int type, bool printNmdOnly = false);
    static int32_t StartPrintSimplifiedNmd(pid_t pid, std::vector<SimplifiedMemStats>& memStats);
    static int32_t Start(int fd, pid_t pid, uint32_t duration, SimplifiedMemConfig& config);
    static int32_t Start(int fd, pid_t pid, uint32_t duration, MemSaConfig& saConfig);
    static int32_t Start(int fd, pid_t pid, MemLeakConfig& memLeakConfig);
    static int32_t StartProfilerByType(int32_t type, MemAPIConfig *inputConfig);
    static int32_t GetGwpAsan(int fd, pid_t pid, GwpAsanConfig& config);
    static sptr<IRemoteObject> GetRemoteService();
    static int32_t Prepare();
    
private:
    static int32_t OnDemandStart(OHOS::sptr<OHOS::ISystemAbilityManager> sam,
        OHOS::sptr<OHOS::IRemoteObject> &remoteObject);
    static int32_t CheckStatus();
    static bool CheckConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t ParseResType(int32_t type, std::shared_ptr<NativeMemoryProfilerSaConfig> config);
    static int32_t ParseNormalConfig(std::shared_ptr<NativeMemoryProfilerSaConfig> config, MemAPIConfig* inputConfig);
    static ResProfilerErrorCode CheckInputConfig(int type, MemAPIConfig* inputConfig);
    static int32_t GetMemProfilerUseCountToday();
    static bool UpdateMemProfilerUserCount();
    static bool ReadCountFile(const std::string &filepath, long long &timestamp, int32_t &count);
    static bool WriteCountFileAtomic(const std::string &filepath, long long timestamp, int32_t count);
    static bool IsSeparateByMoreThan24Hours(long long t1, long long t2);
    static std::string GenerateFileName(int type, int32_t pid);
};
}

#endif // NATIVE_MEMORY_PROFILER_SA_CLIENT_MANAGER_H