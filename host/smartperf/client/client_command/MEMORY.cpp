/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include "include/MEMORY.h"
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
#include "include/startup_delay.h"
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> MEMORY::ItemData()
{
    std::map<std::string, std::string> result;
    if (packageName.length() > 0) {
        std::map<std::string, std::string> procRamInfo = MEMORY::GetRamInfo();
        for (auto it = procRamInfo.begin(); it != procRamInfo.end(); ++it) {
            result.insert(*it);
        }
    }
    return result;
}

void MEMORY::SetPackageName(const std::string pName)
{
    packageName = pName;
}

std::map<std::string, std::string> MEMORY::GetRamInfo() const
{
    std::string processId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(packageName);
    std::map<std::string, std::string> procRamInfo;
    if (processId.size() == 0) {
        procRamInfo["gpuPss"] = "NA";
        procRamInfo["graphicPss"] = "NA";
        procRamInfo["arktsHeapPss"] = "NA";
        procRamInfo["nativeHeapPss"] = "NA";
        procRamInfo["stackPss"] = "NA";
        procRamInfo["sharedClean"] = "NA";
        procRamInfo["sharedDirty"] = "NA";
        procRamInfo["privateClean"] = "NA";
        procRamInfo["privateDirty"] = "NA";
        procRamInfo["swap"] = "NA";
        procRamInfo["swapPss"] = "NA";
        procRamInfo["heapSize"] = "NA";
        procRamInfo["heapAlloc"] = "NA";
        procRamInfo["heapFree"] = "NA";
        return procRamInfo;
    }
    std::stringstream cmd;
    cmd << "hidumper --mem " << processId;
    FILE *fd = popen(cmd.str().c_str(), "r");
    if (fd == nullptr) {
        return procRamInfo;
    }
    std::vector<std::string> paramsInfo;
    procRamInfo = GetPssRamInfo(fd, paramsInfo);
    if (procRamInfo.find("gpuPss") != procRamInfo.end() && procRamInfo["gpuPss"].empty()) {
        procRamInfo["gpuPss"] = "NA";
        procRamInfo["graphicPss"] = "NA";
        procRamInfo["arktsHeapPss"] = "NA";
        procRamInfo["nativeHeapPss"] = "NA";
        procRamInfo["stackPss"] = "NA";
        procRamInfo["sharedClean"] = "NA";
        procRamInfo["sharedDirty"] = "NA";
        procRamInfo["privateClean"] = "NA";
        procRamInfo["privateDirty"] = "NA";
        procRamInfo["swap"] = "NA";
        procRamInfo["swapPss"] = "NA";
        procRamInfo["heapSize"] = "NA";
        procRamInfo["heapAlloc"] = "NA";
        procRamInfo["heapFree"] = "NA";
    }
    return procRamInfo;
}

std::map<std::string, std::string> MEMORY::GetPssRamInfo(FILE *fd, std::vector<std::string> paramsInfo) const
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
            arktsHeapPssValue = params[MEMORY_THIRD];
        }
        if (params[0].find("native") != std::string::npos && params[1].find("heap") != std::string::npos) {
            nativeHeapPssValue = params[MEMORY_SECOND];
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

std::map<std::string, std::string> MEMORY::SaveSumRamInfo(std::vector<std::string> paramsInfo) const
{
    std::map<std::string, std::string> sumRamInfo;
    sumRamInfo["sharedClean"] = paramsInfo[MEMORY_SECOND];
    sumRamInfo["sharedDirty"] = paramsInfo[MEMORY_THIRD];
    sumRamInfo["privateClean"] = paramsInfo[MEMORY_FOURTH];
    sumRamInfo["privateDirty"] = paramsInfo[MEMORY_FIFTH];
    sumRamInfo["swap"] = paramsInfo[MEMORY_SIXTH];
    sumRamInfo["swapPss"] = paramsInfo[MEMORY_SEVENTH];
    sumRamInfo["heapSize"] = paramsInfo[MEMORY_EIGHTH];
    sumRamInfo["heapAlloc"] = paramsInfo[MEMORY_NINTH];
    sumRamInfo["heapFree"] = paramsInfo[MEMORY_TENTH].erase(static_cast<int>(paramsInfo[MEMORY_TENTH].size()) - 1);
    return sumRamInfo;
}
}
}
