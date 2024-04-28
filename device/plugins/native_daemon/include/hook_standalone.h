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
#include <sstream>
#include <set>

struct HookData {
    std::set<std::string> pids = {};
    uint32_t smbSize {0};
    uint64_t duration {0};
    uint32_t filterSize {0};
    uint32_t maxStackDepth {100};
    uint32_t maxJsStackdepth = {0};
    uint32_t statisticsInterval {0};
    uint32_t sampleInterval {0};
    int32_t jsStackReport {0};
    uint32_t mallocFreeMatchingInterval {0};
    std::string fileName {"/data/local/tmp/data.txt"};
    std::string processName;
    std::string performanceFilename {"/data/local/tmp/performance.txt"};
    std::string filterNapiName = {""};
    bool mallocDisable {false};
    bool mmapDisable {false};
    bool freemsgstack {false};
    bool munmapmsgstack {false};
    bool fpUnwind {false};
    bool offlineSymbolization {false};
    bool callframeCompress {false};
    bool stringCompressed {false};
    bool rawString {false};
    bool startupMode {false};
    bool responseLibraryMode {false};
    std::string ToString() const
    {
        std::stringstream ss;
        ss << "smbSize:" << smbSize << ", duration:" << duration
        << ", filterSize:" << filterSize << ", maxStackDepth:" << maxStackDepth
        << ", statisticsInterval:" << statisticsInterval << ", fileName:" << fileName
        << ", performanceFilename:" << performanceFilename << ", mallocDisable:" << mallocDisable
        << ", mmapDisable:" << mmapDisable << ", freemsgstack:" << freemsgstack
        << ", munmapmsgstack:" << munmapmsgstack << ", fpUnwind:" << fpUnwind
        << ", offlineSymbolization:" << offlineSymbolization << ", callframeCompress:" << callframeCompress
        << ", stringCompressed:" << stringCompressed << ", rawString:" << rawString <<", pids:";
        for (std::string pid: pids) {
            ss << pid <<' ';
        }
        ss << ", processName:" << processName << ' ' <<",startupMode: " << startupMode
           << ", responseLibraryMode: " << responseLibraryMode << ", sampleInterval: " << sampleInterval
           << ", jsStackReport: " << jsStackReport << ", maxJsStackdepth: " << maxJsStackdepth
           <<", filterNapiName" << filterNapiName << "mallocFreeMatchingInterval: " << mallocFreeMatchingInterval;
        return ss.str();
    }
};

namespace OHOS::Developtools::Profiler::Hook {
bool StartHook(HookData& hookData);
void EndHook();
} // namespace OHOS::Developtools::Profiler::Hook

#endif // HOOK_STANDALONE_H