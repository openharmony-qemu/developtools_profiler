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
const int COLLECT_INTERVAL = 6;
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
        {"childCarktsHeapPss", "NA"},
        {"childGpuPss", "NA"},
        {"childGraphicPss", "NA"},
        {"childHeapAlloc", "NA" },
        {"childHeapFree", "NA"},
        {"childHeapSize", "NA"},
        {"childNativeHeapPss", "NA"},
        {"childPrivateClean", "NA"},
        {"childPrivateDirty", "NA"},
        {"childPss", "NA"},
        {"childSharedClean", "NA"},
        {"childSharedDirty", "NA"},
        {"childStackPss", "NA"},
        {"childSwap", "NA"},
        {"childSwapPss", "NA"}
};
std::map<std::string, std::string> RAM::ItemData()
{
    static int collectCount = 0;
    if ((collectCount++ % COLLECT_INTERVAL) != 0) {
        return result;
    }
    result = RAM::GetSysRamInfo();
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
        } else {
            procRamInfomation = ProcMemNaInfo();
        }
        result.merge(procRamInfomation);
    } else if (!packageName.empty() && processId.empty()) {
        result.merge(RAM::ProcMemNaInfo());
    }
    LOGI("RAM:ItemData map size(%u)", result.size());
    return result;
}

void RAM::ThreadGetPss() const
{
    procRamInfoLast = RAM::GetRamInfo();
}

void RAM::TriggerGetPss() const
{
    std::thread([this]() {
        this->ThreadGetPss();
    }).detach();
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

std::map<std::string, std::string> RAM::ChildProcMemNaInfo() const
{
    std::map<std::string, std::string> procMemInfo;
    procMemInfo["childCarktsHeapPss"] = "NA";
    procMemInfo["childGpuPss"] = "NA";
    procMemInfo["childGraphicPss"] = "NA";
    procMemInfo["childHeapAlloc"] = "NA";
    procMemInfo["childHeapFree"] = "NA";
    procMemInfo["childHeapSize"] = "NA";
    procMemInfo["childNativeHeapPss"] = "NA";
    procMemInfo["childPrivateClean"] = "NA";
    procMemInfo["childPrivateDirty"] = "NA";
    procMemInfo["childPss"] = "NA";
    procMemInfo["childSharedClean"] = "NA";
    procMemInfo["childSharedDirty"] = "NA";
    procMemInfo["childStackPss"] = "NA";
    procMemInfo["childSwap"] = "NA";
    procMemInfo["childSwapPss"] = "NA";
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
    processId.clear();
    SPUtils::StrSplit(pid, " ", processId);
    LOGD("RAM SetProcessId (%s)", pid.c_str());
}

std::map<std::string, std::string> RAM::CollectRam(const std::string& ramPid, size_t index) const
{
    std::map<std::string, std::string> emptyprocRamInfo;

    if (!std::all_of(ramPid.begin(), ramPid.end(), ::isdigit)) {
        LOGE("RAM::CollectRam invalid ramPid");
        return emptyprocRamInfo;
    }
    const std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_MEM) + ramPid;
    if (cmd.empty()) {
        LOGE("RAM::GetRamInfo cmd is null");
        return emptyprocRamInfo;
    }
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        LOGD("RAM::fd is empty");
        emptyprocRamInfo = ProcMemNaInfo();
        for (auto& item : emptyprocRamInfo) {
            item.second = "0";
        }
        return emptyprocRamInfo;
    }

    std::map<std::string, std::string> procRamInfo = GetPssRamInfo(fd, ramPid, index);
    if (procRamInfo.empty()) {
        pclose(fd);
        return emptyprocRamInfo;
    }

    if (pclose(fd) == -1) {
        LOGE("Error: Failed to close file");
        return emptyprocRamInfo;
    }
    return procRamInfo;
}

void RAM::SetRamValue(std::promise<std::map<std::string, std::string>> p,
    std::string ramPid, size_t index) const
{
    p.set_value(CollectRam(ramPid, index));
}

std::future<std::map<std::string, std::string>> RAM::AsyncCollectRam(const std::string& ramPid, size_t index) const
{
    std::promise<std::map<std::string, std::string>> p;
    std::future<std::map<std::string, std::string>> futureResult = p.get_future();
    std::thread(&RAM::SetRamValue, this, std::move(p), ramPid, index).detach();
    return futureResult;
}

void RAM::CheckFutureRam(std::future<std::map<std::string, std::string>> &fdsResult,
                         std::map<std::string, std::string> &dataMap, const std::string& pid, size_t index) const
{
    if (fdsResult.valid()) {
        std::map<std::string, std::string> result = fdsResult.get();
        if (index == 0) {
            dataMap.insert(result.begin(), result.end());
        } else {
            for (auto &item : result) {
                dataMap[item.first].append(item.second);
            }
        }
    }
}

std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::map<std::string, std::string> dataMap;
    std::vector<std::future<std::map<std::string, std::string>>>
        fdsResult;
    std::vector<std::string> processIds = processId;
    for (size_t i = 0; i < processIds.size(); i++) {
        fdsResult.emplace_back(AsyncCollectRam(processIds[i], i));
    }
    for (size_t i = 0; i < processIds.size(); i++) {
        CheckFutureRam(fdsResult[i], dataMap, processIds[i], i);
    }
    if (processIds.size() == 1) {
        std::map<std::string, std::string> procMemInfo = RAM::ChildProcMemNaInfo();
        for (const auto& item : procMemInfo) {
            dataMap.insert(item);
        }
    }
    return dataMap;
}

std::map<std::string, std::string> RAM::GetPssRamInfo(FILE *fd, const std::string& pid, size_t index) const
{
    std::vector<std::string> paramsInfo;
    std::map<std::string, std::string> pssRamInfo = ParsePssValues(fd, paramsInfo, pid, index);
    std::map<std::string, std::string> sumRamInfo = SaveSumRamInfo(paramsInfo, pid, index);
    pssRamInfo.insert(std::make_move_iterator(sumRamInfo.begin()), std::make_move_iterator(sumRamInfo.end()));
    if (paramsInfo.empty()) {
        for (auto &pss : pssRamInfo) {
            pss.second = pid + ":" + "0" + "|";
        }
        return pssRamInfo;
    }
    return pssRamInfo;
}

std::map<std::string, std::string> RAM::ParsePssValues(FILE *fd, std::vector<std::string> &paramsInfo,
    std::string pid, size_t index) const
{
    std::map<std::string, std::string> pssRamInfo;
    struct PssValues pss;
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line(buf);
        if (line[0] == '-') {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params.size() > RAM_SECOND && params[0].find("GL") != std::string::npos) {
            pss.gpuPssValue = params[1];
        }
        if (params.size() > RAM_SECOND && params[0].find("Graph") != std::string::npos) {
            pss.graphicPssValue = params[1];
        }
        if (params.size() > RAM_FOURTH && params[0].find("ark") != std::string::npos) {
            pss.arktsHeapPssValue = params[RAM_THIRD];
        }
        if (params.size() > RAM_THIRD && params[0].find("native") != std::string::npos &&
            params[1].find("heap") != std::string::npos) {
            pss.nativeHeapPssValue = params[RAM_SECOND];
        }
        if (params.size() > RAM_SECOND && params[0].find("stack") != std::string::npos) {
            pss.stackPssValue = params[1];
        }
        if (!pss.gpuPssValue.empty() && params.size() > 0 && params[0].find("Total") != std::string::npos) {
            paramsInfo = params;
        }
        if (paramsInfo.size() > 0) {
            break;
        }
    }

    FillPssRamInfo(index, pid, pss, pssRamInfo);
    // 应用程序的内存占用信息
    LOGD("pssRamInfo map size(%u)", pssRamInfo.size());
    return pssRamInfo;
}

void RAM::FillPssRamInfo(size_t index, std::string pid,
    const PssValues &pss, std::map<std::string, std::string> &pssRamInfo) const
{
    if (index == 0) {
        pssRamInfo["gpuPss"] = pss.gpuPssValue;
        pssRamInfo["graphicPss"] = pss.graphicPssValue;
        pssRamInfo["arktsHeapPss"] = pss.arktsHeapPssValue;
        pssRamInfo["nativeHeapPss"] = pss.nativeHeapPssValue;
        pssRamInfo["stackPss"] = pss.stackPssValue;
    } else {
        pid.append(":");
        pssRamInfo["childGpuPss"].append(pid).append(pss.gpuPssValue).append("|");
        pssRamInfo["childGraphicPss"].append(pid).append(pss.graphicPssValue).append("|");
        pssRamInfo["childArktsHeapPss"].append(pid).append(pss.arktsHeapPssValue).append("|");
        pssRamInfo["childNativeHeapPss"].append(pid).append(pss.nativeHeapPssValue).append("|");
        pssRamInfo["childStackPss"].append(pid).append(pss.stackPssValue).append("|");
    }
}

std::map<std::string, std::string> RAM::SaveSumRamInfo(std::vector<std::string>& paramsInfo,
    const std::string& pid, size_t index) const
{
    std::map<std::string, std::string> sumRamInfo;
    if (paramsInfo.empty()) {
        if (index == 0) {
            sumRamInfo = ProcMemNaInfo();
        } else {
            sumRamInfo = ChildProcMemNaInfo();
        }
        for (auto &sumRam : sumRamInfo) {
            sumRam.second = "0";
        }
        return sumRamInfo;
    }
    std::vector<std::string> sumRamKeys = {"pss", "sharedClean", "sharedDirty", "privateClean",
        "privateDirty", "swap", "swapPss", "heapSize", "heapAlloc", "heapFree"};
    std::vector<std::string> childSumRamKeys = {"childPss", "childSharedClean", "childSharedDirty", "childPrivateClean",
        "childPrivateDirty", "childSwap", "childSwapPss", "childHeapSize", "childHeapAlloc", "childHeapFree"};
    if (index == 0) {
        for (size_t i = 0; i < paramsInfo.size() - 1 && i < sumRamKeys.size(); i++) {
            if (i == RAM_NINTH) {
                sumRamInfo["heapFree"] =
                    paramsInfo[RAM_TENTH].erase(static_cast<int>(paramsInfo[RAM_TENTH].size()) - 1);
                break;
            }
            sumRamInfo[sumRamKeys[i]] = paramsInfo[i + 1];
        }
    } else {
        for (size_t i = 0; i < paramsInfo.size() - 1 && i < childSumRamKeys.size(); i++) {
            if (i == RAM_NINTH) {
                sumRamInfo["childHeapFree"] = pid + ":" +
                    paramsInfo[RAM_TENTH].erase(static_cast<int>(paramsInfo[RAM_TENTH].size()) - 1).append("|");
                break;
            }
            sumRamInfo[childSumRamKeys[i]] = pid + ":" + paramsInfo[i + 1].append("|");
        }
    }
   
    //应用程序的内存消耗信息
    LOGD("sumRamInfo map size(%u)", sumRamInfo.size());
    return sumRamInfo;
}
}
}
