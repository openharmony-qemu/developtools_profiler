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
#include "include/common.h"

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
    for (const auto& item : sysRamInfo) {
        result.insert(item);
    }
    if (!processId.empty()) {
        std::map<std::string, std::string> procRamInfomation;
        if (g_flagFirst) {
            RAM::TriggerGetPss();
        } else {
            procRamInfoLast = RAM::GetRamInfo();
            g_flagFirst = true;
        }
        if (!procRamInfoLast.empty()) {
            procRamInfomation = procRamInfoLast;
            for (const auto& item : procRamInfomation) {
                result.insert(item);
            }
        } else {
            procRamInfomation = ProcMemNaInfo();
            for (const auto& item : procRamInfomation) {
                result.insert(item);
            }
        }
    } else if (!packageName.empty() && processId.empty()) {
        std::map<std::string, std::string> procMemInfo = RAM::ProcMemNaInfo();
        for (const auto& item : procMemInfo) {
            result.insert(item);
        }
    }
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

void RAM::SetHapFirstFlag()
{
    g_flagFirst = true;
}

std::map<std::string, std::string> RAM::ProcMemNaInfo() const
{
    std::map<std::string, std::string> procMemInfo;
    procMemInfo["arktsHeapPss"] = "NA";
    procMemInfo["gpuPss"] = "NA";
    procMemInfo["graphicPss"] = "NA";
    procMemInfo["heapAlloc"] = "NA";
    procMemInfo["heapFree"] = "NA";
    procMemInfo["heapSize"] = "NA";
    procMemInfo["nativeHeapPss"] = "NA";
    procMemInfo["privateClean"] = "NA";
    procMemInfo["privateDirty"] = "NA";
    procMemInfo["pss"] = "NA";
    procMemInfo["sharedClean"] = "NA";
    procMemInfo["sharedDirty"] = "NA";
    procMemInfo["stackPss"] = "NA";
    procMemInfo["swap"] = "NA";
    procMemInfo["swapPss"] = "NA";
    return procMemInfo;
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
    //整机内存信息
    LOGD("sysRamInfo map size(%u)", sysRamInfo.size());
    return sysRamInfo;
}

void RAM::SetPackageName(const std::string &pName)
{
    packageName = pName;
}

void RAM::SetProcessId(const std::string &pid)
{
    processId = pid;
}

std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::map<std::string, std::string> procRamInfo;
    std::map<std::string, std::string> emptyprocRamInfo;
    std::string pssValue = "";
    std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_MEM) + processId;
    if (cmd.empty()) {
        LOGE("RAM::GetRamInfo cmd is null");
        return emptyprocRamInfo;
    }
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        LOGD("RAM::fd is empty");
        emptyprocRamInfo = ProcMemNaInfo();
        for (auto item : emptyprocRamInfo) {
            item.second = "0";
    }
        return emptyprocRamInfo;
    }
    std::vector<std::string> paramsInfo;
    procRamInfo = GetPssRamInfo(fd, paramsInfo);
    if (procRamInfo.empty()) {
        return emptyprocRamInfo;
    }
    for (const auto& value : paramsInfo) {
        if (procRamInfo[value].empty()) {
            procRamInfo[value] = "0";
        }
    }
    int closeStatus = pclose(fd);
    if (closeStatus == -1) {
        LOGE("Error: Failed to close file");
        return emptyprocRamInfo;
    }
    return procRamInfo;
}

std::map<std::string, std::string> RAM::GetPssRamInfo(FILE *fd, std::vector<std::string> paramsInfo) const
{
    std::map<std::string, std::string> pssRamInfo = ParsePssValues(fd, paramsInfo);
    std::map<std::string, std::string> sumRamInfo = SaveSumRamInfo(paramsInfo);
    pssRamInfo.insert(sumRamInfo.cbegin(), sumRamInfo.cend());
    if (paramsInfo.empty()) {
        for (auto &pss : pssRamInfo) {
            pss.second = "0";
        }
        return pssRamInfo;
    }
    return pssRamInfo;
}

std::map<std::string, std::string> RAM::ParsePssValues(FILE *fd, std::vector<std::string> &paramsInfo) const
{
    std::map<std::string, std::string> pssRamInfo;
    std::string gpuPssValue = "";
    std::string graphicPssValue = "";
    std::string arktsHeapPssValue = "";
    std::string nativeHeapPssValue = "";
    std::string stackPssValue = "";
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line(buf);
        LOGD("ParsePssValues::line = %s", line.c_str());
        if (line[0] == '-') {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params.size() > RAM_SECOND && params[0].find("GL") != std::string::npos) {
            gpuPssValue = params[1];
        }
        if (params.size() > RAM_SECOND && params[0].find("Graph") != std::string::npos) {
            graphicPssValue = params[1];
        }
        if (params.size() > RAM_FOURTH && params[0].find("ark") != std::string::npos) {
            arktsHeapPssValue = params[RAM_THIRD];
        }
        if (params.size() > RAM_THIRD && params[0].find("native") != std::string::npos &&
                    params[1].find("heap") != std::string::npos) {
                    nativeHeapPssValue = params[RAM_SECOND];
        }
        if (params.size() > RAM_SECOND && params[0].find("stack") != std::string::npos) {
            stackPssValue = params[1];
        }
        if (!gpuPssValue.empty() && params.size() > 0 && params[0].find("Total") != std::string::npos) {
            paramsInfo = params;
        }
        if (paramsInfo.size() > 0) {
            break;
        }
    }
    pssRamInfo["gpuPss"] = gpuPssValue;
    pssRamInfo["graphicPss"] = graphicPssValue;
    pssRamInfo["arktsHeapPss"] = arktsHeapPssValue;
    pssRamInfo["nativeHeapPss"] = nativeHeapPssValue;
    pssRamInfo["stackPss"] = stackPssValue;
    //应用程序的内存占用信息
    LOGD("pssRamInfo map size(%u)", pssRamInfo.size());
    return pssRamInfo;
}
std::map<std::string, std::string> RAM::SaveSumRamInfo(std::vector<std::string> paramsInfo) const
{
    std::map<std::string, std::string> sumRamInfo;
    if (paramsInfo.empty()) {
        sumRamInfo = ProcMemNaInfo();
        for (auto &sumRam : sumRamInfo) {
            sumRam.second = "0";
        }
        return sumRamInfo;
    }
    std::vector<std::string> sumRamKeys = {"pss", "sharedClean", "sharedDirty", "privateClean",
    "privateDirty", "swap", "swapPss", "heapSize", "heapAlloc", "heapFree"};
    for (size_t i = 0; i < paramsInfo.size() - 1 && i < sumRamKeys.size(); i++) {
        if (i == RAM_NINTH) {
            sumRamInfo["heapFree"] = paramsInfo[RAM_TENTH].erase(static_cast<int>(paramsInfo[RAM_TENTH].size()) - 1);
            break;
        }
        sumRamInfo[sumRamKeys[i]] = paramsInfo[i + 1];
    }
    //应用程序的内存消耗信息
    LOGD("sumRamInfo map size(%u)", sumRamInfo.size());
    return sumRamInfo;
}
}
}
