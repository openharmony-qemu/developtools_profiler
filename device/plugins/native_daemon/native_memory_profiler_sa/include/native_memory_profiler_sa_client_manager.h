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
#include <iservice_registry.h>

#include <memory>

namespace OHOS::Developtools::NativeDaemon {
struct SimplifiedMemStats {
    size_t size = 0;
    size_t allocated = 0;
    size_t nmalloc = 0;
    size_t ndalloc = 0;
};

struct SimplifiedMemConfig {
    size_t largestSize = 0;
    size_t secondLargestSize = 0;
    size_t maxGrowthSize = 0;
    size_t sampleSize = 0;
};

class NativeMemoryProfilerSaClientManager {
public:
    enum class NativeMemProfilerType : int32_t {
        MEM_PROFILER_LIBRARY,
        MEM_PROFILER_CALL_STACK,
    };
    static int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t Start(NativeMemProfilerType type, uint32_t pid, uint32_t duration, uint32_t sampleIntervel);
    static int32_t Stop(uint32_t pid);
    static int32_t Stop(const std::string& name);
    static int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
    static int32_t GetMallocStats(int fd, int pid, int type, bool printNmdOnly = false);
    static int32_t StartPrintSimplifiedNmd(pid_t pid, std::vector<SimplifiedMemStats>& memStats);
    static int32_t Start(int fd, pid_t pid, uint32_t duration, SimplifiedMemConfig& config);
    static sptr<IRemoteObject> GetRemoteService();

private:
    static bool CheckConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config);
};
}

#endif // NATIVE_MEMORY_PROFILER_SA_CLIENT_MANAGER_H