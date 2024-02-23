/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "include/RAM.h"
#include <sstream>
#include <fstream>
#include <climits>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <thread>
#include <string>
#include <regex>
#include "include/sp_utils.h"
#include "memory_collector.h"
#include "collect_result.h"
#include "include/startup_delay.h"
#include "include/sp_log.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> RAM::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> sysRamInfo = RAM::GetSysRamInfo();
    for (auto it = sysRamInfo.begin(); it != sysRamInfo.end(); ++it) {
        result.insert(*it);
    }
    std::map<std::string, std::string> procRamInfo = RAM::GetRamInfo();
    for (auto it = procRamInfo.begin(); it != procRamInfo.end(); ++it) {
        result.insert(*it);
    }
    return result;
}

void RAM::SetPackageName(std::string pName)
{
    packageName = pName;
}

std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::string processId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(packageName);
    std::map<std::string, std::string> procRamInfo;
    if (processId.size() == 0) {
        return procRamInfo;
    }
    std::string cmd = "hidumper --mem "+ processId;
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return procRamInfo;
    }
    std::vector<std::string> paramsInfo;
    std::map<std::string, std::string> totalRamInfo = GetPssRamInfo(fd, paramsInfo);
    if (totalRamInfo.empty()) {
        LOGE("RAM::GetRamInfo totalRamInfo is empty");
    }
    procRamInfo.insert(totalRamInfo.cbegin(), totalRamInfo.cend());

    return procRamInfo;
}

std::map<std::string, std::string> RAM::GetPssRamInfo(FILE *fd, std::vector<std::string> paramsInfo) const
{
    std::map<std::string, std::string> pssRamInfo;
    std::string gpuPssValue = "";
    std::string graphicPssValue = "";
    std::string arktsHeapPssValue = "";
    std::string nativeHeapPssValue = "";
    std::string stackPssValue = "";
    const int paramEleven = 11;
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        if (line[0] == '-') {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params.size() == paramEleven && params[0].find("GL") != std::string::npos) {
            gpuPssValue = params[1];
        }
        if (params.size() == paramEleven && params[0].find("Graph") != std::string::npos) {
            graphicPssValue = params[1];
        }
        if (params[0].find("ark") != std::string::npos) {
            arktsHeapPssValue = params[RAM_THIRD];
        }
        if (params[0].find("native") != std::string::npos && params[1].find("heap") != std::string::npos) {
            nativeHeapPssValue = params[RAM_SECOND];
        }
        if (params.size() == paramEleven && params[0].find("stack") != std::string::npos) {
            stackPssValue = params[1];
        }
        if (params.size() == paramEleven && params[0].find("Total") != std::string::npos) {
            paramsInfo = params;
        }
        if (paramsInfo.size() > 0) {
            break;
        }
    }
    pclose(fd);
    std::map<std::string, std::string> sumRamInfo = SaveSumRamInfo(paramsInfo);
    pssRamInfo.insert(sumRamInfo.cbegin(), sumRamInfo.cend());
    pssRamInfo["gpuPss"] = gpuPssValue;
    pssRamInfo["graphicPss"] = graphicPssValue;
    pssRamInfo["arktsHeapPss"] = arktsHeapPssValue;
    pssRamInfo["nativeHeapPss"] = nativeHeapPssValue;
    pssRamInfo["stackPss"] = stackPssValue;

    return pssRamInfo;
}

std::map<std::string, std::string> RAM::SaveSumRamInfo(std::vector<std::string> paramsInfo) const
{
    std::map<std::string, std::string> sumRamInfo;
    if (sumRamInfo.size() > 0) {
        sumRamInfo["pss"] = paramsInfo[RAM_ONE];
        sumRamInfo["sharedClean"] = paramsInfo[RAM_SECOND];
        sumRamInfo["sharedDirty"] = paramsInfo[RAM_THIRD];
        sumRamInfo["privateClean"] = paramsInfo[RAM_FOURTH];
        sumRamInfo["privateDirty"] = paramsInfo[RAM_FIFTH];
        sumRamInfo["swap"] = paramsInfo[RAM_SIXTH];
        sumRamInfo["swapPss"] = paramsInfo[RAM_SEVENTH];
        sumRamInfo["heapSize"] = paramsInfo[RAM_EIGHTH];
        sumRamInfo["heapAlloc"] = paramsInfo[RAM_NINTH];
        sumRamInfo["heapFree"] = paramsInfo[RAM_TENTH].erase(paramsInfo[RAM_TENTH].size() - 1);
    }

    return sumRamInfo;
}

std::map<std::string, std::string> RAM::GetSysRamInfo() const
{
    std::map<std::string, std::string> sysRamInfo;
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<SysMemory> result = collector->CollectSysMemory();
    sysRamInfo["memTotal"] = std::to_string(result.data.memTotal);
    sysRamInfo["memFree"] = std::to_string(result.data.memFree);
    sysRamInfo["memAvailable"] = std::to_string(result.data.memAvailable);
    return sysRamInfo;
}
}
}
