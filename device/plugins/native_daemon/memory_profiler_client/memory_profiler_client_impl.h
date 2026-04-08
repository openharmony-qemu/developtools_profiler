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
     * @param fd: The file descriptor to write the profiling result.
     * @param duration: The profiling duration in seconds.
     * @param config: The simplified memory profiler configuration.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t Start(int fd, uint32_t duration, SimplifiedMemConfig& config);

    /**
     * @desc: Start memory profiler on memory API configuration.
     * @param config: The memory API profiler configuration.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t Start(MemAPIConfig& config);

    /**
     * @desc: Start simplified nmd mode profiler.
     * @param memStats: The simplified memory stats vector to store the result.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t StartPrintSimplifiedNmd(std::vector<SimplifiedMemStats>& memStats);

    /**
     * @desc: Stop memory profiler.
     * @return: RET_OK on success, RET_ERR on failure.
     */
    int32_t Stop();

private:
    bool CheckMemApiConfig(const MemAPIConfig& config);
};
}   // namespace NativeDaemon
}   // namespace Developtools
}   // namespace OHOS

#endif // MEMORY_PROFILER_CLIENT_IMPL_H
