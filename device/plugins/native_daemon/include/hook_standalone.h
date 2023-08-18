/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef HOOK_STANDALONE_H
#define HOOK_STANDALONE_H

#include <string>

using HookData = struct {
    int pid;
    uint32_t smbSize;
    uint64_t duration;
    uint32_t filterSize;
    uint32_t maxStackDepth;
    uint32_t statisticsInterval;
    std::string fileName;
    std::string processName;
    std::string performance_filename;
    bool mallocDisable;
    bool mmapDisable;
    bool freemsgstack;
    bool munmapmsgstack;
    bool fpUnwind;
    bool offlineSymbolization;
    bool callframeCompress;
    bool stringCompressed;
    bool rawString;
};

namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace Hook {
bool StartHook(HookData& hookData);
} // namespace Hook
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS

#endif // HOOK_STANDALONE_H