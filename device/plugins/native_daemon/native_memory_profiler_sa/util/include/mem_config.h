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
