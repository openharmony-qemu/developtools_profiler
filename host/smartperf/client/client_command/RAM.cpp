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
#include "common.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
bool g_flagFirst = false;
std::map<std::string, std::string> procRamInfoLast {
    {"pss", "NA"},
    {"gpuPss", "NA"},
    {"graphicPss", "NA"},
    {"arktsHeapPss", "NA"},
    {"nativeHeapPss", "NA"},
    {"stackPss", "NA"},
    {"sharedClean", "NA"},
    {"sharedDirty", "NA"},
    {"privateClean", "NA"},
    {"privateDirty", "NA"},
    {"swap", "NA"},
    {"swapPss", "NA"},
    {"heapSize", "NA"},
    {"heapAlloc", "NA"},
    {"heapFree", "NA"},
};
std::map<std::string, std::string> RAM::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> sysRamInfo = RAM::GetSysRamInfo();
    for (auto it = sysRamInfo.begin(); it != sysRamInfo.end(); ++it) {
        result.insert(*it);
    }
    if (packageName.length() > 0) {
        if (g_flagFirst) {
            RAM::TriggerGetPss();
        } else {
            procRamInfoLast = RAM::GetRamInfo();
            g_flagFirst = true;
        }
        for (auto it = procRamInfoLast.begin(); it != procRamInfoLast.end(); ++it) {
            result.insert(*it);
        }
    }
    LOGI("RAM::ItemData map size(%u)", result.size());
    return result;
}

void RAM::ThreadGetPss() const
{
    std::map<std::string, std::string> procRamInfo = RAM::GetRamInfo();
    procRamInfoLast = procRamInfo;
}

void RAM::TriggerGetPss() const
{
    auto tStart = std::thread([this]() {
        this->ThreadGetPss();
    });
    tStart.detach();
}

void RAM::SetFirstFlag()
{
    g_flagFirst = false;
}

std::map<std::string, std::string> RAM::GetSysRamInfo() const
{
    std::map<std::string, std::string> sysRamInfo;
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    if (collector == nullptr) {
        LOGE("RAM::GetSysRamInfo collector is nullptr!");
        return sysRamInfo;
    }
    CollectResult<SysMemory> result = collector->CollectSysMemory();
    sysRamInfo["memTotal"] = std::to_string(result.data.memTotal);
    sysRamInfo["memFree"] = std::to_string(result.data.memFree);
    sysRamInfo["memAvailable"] = std::to_string(result.data.memAvailable);
    return sysRamInfo;
}

void RAM::SetPackageName(const std::string &pName)
{
    packageName = pName;
}

std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::string processId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(packageName);
    std::map<std::string, std::string> procRamInfo;
    std::string pssValue = "";
    if (processId.size() == 0) {
        procRamInfo["gpuPss"] = "NA";
        procRamInfo["graphicPss"] = "NA";
        procRamInfo["arktsHeapPss"] = "NA";
        procRamInfo["nativeHeapPss"] = "NA";
        procRamInfo["stackPss"] = "NA";
        procRamInfo["pss"] = "NA";
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
    std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_MEM) + processId;
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return procRamInfo;
    }
    std::vector<std::string> paramsInfo;
    procRamInfo = GetPssRamInfo(fd, paramsInfo);
    if (procRamInfo.find("arktsHeapPss") != procRamInfo.end() && procRamInfo["gpuPss"].empty()) {
        procRamInfo["gpuPss"] = "NA";
        procRamInfo["graphicPss"] = "NA";
        procRamInfo["arktsHeapPss"] = "NA";
        procRamInfo["nativeHeapPss"] = "NA";
        procRamInfo["stackPss"] = "NA";
        procRamInfo["pss"] = "NA";
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
    if (paramsInfo.empty()) {
        return sumRamInfo;
    }
    sumRamInfo["pss"] = paramsInfo[RAM_ONE];
    sumRamInfo["sharedClean"] = paramsInfo[RAM_SECOND];
    sumRamInfo["sharedDirty"] = paramsInfo[RAM_THIRD];
    sumRamInfo["privateClean"] = paramsInfo[RAM_FOURTH];
    sumRamInfo["privateDirty"] = paramsInfo[RAM_FIFTH];
    sumRamInfo["swap"] = paramsInfo[RAM_SIXTH];
    sumRamInfo["swapPss"] = paramsInfo[RAM_SEVENTH];
    sumRamInfo["heapSize"] = paramsInfo[RAM_EIGHTH];
    sumRamInfo["heapAlloc"] = paramsInfo[RAM_NINTH];
    sumRamInfo["heapFree"] = paramsInfo[RAM_TENTH].erase(static_cast<int>(paramsInfo[RAM_TENTH].size()) - 1);
    return sumRamInfo;
}
}
}
