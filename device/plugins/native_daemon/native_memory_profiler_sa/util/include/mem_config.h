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

#ifndef MEM_CONFIG_H
#define MEM_CONFIG_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
#ifndef RET_OK
    #define RET_OK (0)
#endif

#ifndef RET_ERR
    #define RET_ERR (-1)
#endif

enum class ResProfilerErrorCode : uint32_t {
    RES_PROF_SUCCESS = 11400400,
    RES_PROF_INVALID_ARG = 11400410,
    RES_PROF_MAX_DURATION_INVALID_ARG = 11400411,
    RES_PROF_FILTER_SIZE_INVALID_ARG = 11400412,
    RES_PROF_MAX_STACK_DEPTH_INVALID_ARG = 11400413,
    RES_PROF_STATISTICS_INTERVAL_INVALID_ARG = 11400414,
    RES_PROF_SAMPLE_INTERVAL_INVALID_ARG = 11400415,
    RES_PROF_RESOURCE_TYPE_INVALID_ARG = 11400416,
    RES_PROF_PERMISSION_DENIED = 11400420,
    RES_PROF_ALREADY_STARTED = 11400421,
    RES_PROF_NOT_STARTED = 11400422,
    RES_PROF_PROCESS_OVERLIMIT = 11400423,
    RES_PROF_CONFLICT = 11400424,
    RES_PROF_AUTO_STOPPED_BY_DURATION = 11400425,
    RES_PROF_DAILY_QUOTA_EXCEEDED = 11400426,
    RES_PROF_CPU_OVERLOADED = 11400427,
    RES_PROF_MEM_PRESSURE_CRITICAL = 11400428,
    RES_PROF_STORAGE_PRESSURE_CRITICAL = 11400429,
    RES_PROF_ERROR = 11400430,
};

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
    bool asyncStackEnable = false;
    uint64_t asyncFlag = 0;
    std::string targetSoName;
    bool responseLibraryMode = false;
    bool nestedAsyncStackEnable = false;
    uint32_t asyncNestingDepth = 0;
    uint32_t asyncTaskStackDepth = 0;
};

struct MemAPIConfig {
    uint32_t maxDuration;
    uint32_t filterSize;
    uint32_t maxStackDepth;
    uint32_t statisticsInterval;
    uint32_t sampleInterval;
};

} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
#endif  // MEM_CONFIG_H
