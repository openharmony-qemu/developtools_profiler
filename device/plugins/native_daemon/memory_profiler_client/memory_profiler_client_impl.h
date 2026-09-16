/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#ifndef MEMORY_PROFILER_CLIENT_IMPL_H
#define MEMORY_PROFILER_CLIENT_IMPL_H

#include "memory_profiler_client.h"

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class MemoryProfilerClientImpl : public MemoryProfilerClient {
public:
    MemoryProfilerClientImpl() = default;
    ~MemoryProfilerClientImpl() = default;

    /**
     * @desc: Start memory profiler on simplified memory configuration.
     * @param duration: The profiling duration in seconds.
     * @param config: The simplified memory profiler configuration.
     * @param profilingCallback: The callback to be invoked when profiling result is ready, can be nullptr.
     * @return: RES_PROF_SUCCESS on success, specific error code on failure.
     */
    ResProfilerErrorCode Start(uint32_t duration, SimplifiedMemConfig& config,
        ProfilingCallback profilingCallback = nullptr) override;

    /**
     * @desc: Start memory profiler on memory API configuration.
     * @param resourceType: The resource type of profiler.
     * value of resourceType : int32_t
     * OH_RES_TYPE_FD = 0
     * OH_RES_TYPE_THREAD = 1
     * OH_RES_TYPE_NATIVE = 2
     * OH_RES_TYPE_GPU = 3
     * OH_RES_TYPE_GLOBAL_HANDLE = 4
     * OH_RES_TYPE_DMA = 5
     * OH_RES_TYPE_ASHMEM = 6
     * @param config: The memory API profiler configuration.
     * @param profilingCallback: The callback to be invoked when profiling result is ready, can be nullptr.
     * @return: RES_PROF_SUCCESS on success, specific error code on failure.
     */
    ResProfilerErrorCode Start(int32_t resourceType, MemAPIConfig& config,
        ProfilingCallback profilingCallback = nullptr) override;

    /**
     * @desc: Start simplified nmd mode profiler.
     * @param memStats: The simplified memory stats vector to store the result.
     * @param isProfiling: Is profiling task exist or not.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t StartPrintSimplifiedNmd(std::vector<SimplifiedMemStats>& memStats, bool isProfiling = false) override;

    /**
     * @desc: Stop memory profiler.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t Stop() override;

    /**
     * @desc: UpdateSimplifiedMemConfig.
     * @param config: The simplified memory profiler configuration.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t UpdateSimplifiedMemConfig(const SimplifiedMemConfig& config) override;

private:
    bool CheckInput(int32_t resourceType, const MemAPIConfig& config) const;
};
}   // namespace NativeDaemon
}   // namespace Developtools
}   // namespace OHOS

#endif // MEMORY_PROFILER_CLIENT_IMPL_H
