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

#include <mem_config.h>

#include "async_stack.h"

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {

namespace {
constexpr uint32_t DEFAULT_ASYNC_NESTING_DEPTH = 1;
constexpr uint32_t DEFAULT_ASYNC_TASK_STACK_DEPTH = 16;
constexpr uint64_t DEFAULT_ASYNC_FLAG = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
                                        ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE |
                                        ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL;
}

SimplifiedMemConfig::SimplifiedMemConfig() : largestSize(0),
    secondLargestSize(0),
    maxGrowthSize(0),
    sampleSize(0),
    asyncStackEnable(true),
    asyncFlag(DEFAULT_ASYNC_FLAG),
    targetSoName(""),
    responseLibraryMode(false),
    nestedAsyncStackEnable(true),
    asyncNestingDepth(DEFAULT_ASYNC_NESTING_DEPTH),
    asyncTaskStackDepth(DEFAULT_ASYNC_TASK_STACK_DEPTH)
{
}

SimplifiedMemConfig::SimplifiedMemConfig(size_t largestSize, size_t secondLargestSize, size_t maxGrowthSize,
    size_t sampleSize) : largestSize(largestSize),
    secondLargestSize(secondLargestSize),
    maxGrowthSize(maxGrowthSize),
    sampleSize(sampleSize),
    asyncStackEnable(true),
    asyncFlag(DEFAULT_ASYNC_FLAG),
    targetSoName(""),
    responseLibraryMode(false),
    nestedAsyncStackEnable(true),
    asyncNestingDepth(DEFAULT_ASYNC_NESTING_DEPTH),
    asyncTaskStackDepth(DEFAULT_ASYNC_TASK_STACK_DEPTH)
{
}

SimplifiedMemConfig::SimplifiedMemConfig(size_t largestSize, size_t secondLargestSize, size_t maxGrowthSize,
    size_t sampleSize, bool asyncStackEnable, uint64_t asyncFlag, const std::string& targetSoName,
    bool responseLibraryMode, bool nestedAsyncStackEnable, uint32_t asyncNestingDepth,
    uint32_t asyncTaskStackDepth) : largestSize(largestSize),
    secondLargestSize(secondLargestSize),
    maxGrowthSize(maxGrowthSize),
    sampleSize(sampleSize),
    asyncStackEnable(asyncStackEnable),
    asyncFlag(asyncFlag),
    targetSoName(targetSoName),
    responseLibraryMode(responseLibraryMode),
    nestedAsyncStackEnable(nestedAsyncStackEnable),
    asyncNestingDepth(asyncNestingDepth),
    asyncTaskStackDepth(asyncTaskStackDepth)
{
}

}   // namespace NativeDaemon
}   // namespace Developtools
}   // namespace OHOS
