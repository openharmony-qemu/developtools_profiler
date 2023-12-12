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

#include "include/CPU.h"
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <climits>
#include "securec.h"
#include "include/sp_utils.h"
#include "cpu_collector.h"
#include "collect_result.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> CPU::ItemData()
{
    std::map<std::string, std::string> result;
    std::vector<CpuFreqs> cpuFreqInfo;
    cpuFreqInfo = GetCpuFreq();
    std::string cpuFreqStr;
    std::string cpuId;
    for (size_t i = 0; i < cpuFreqInfo.size(); i++) {
        cpuFreqStr = std::to_string(cpuFreqInfo[i].curFreq);
        cpuId = std::to_string(cpuFreqInfo[i].curId);
        result["cpu" + cpuId + "Frequency"] = cpuFreqStr;
    }
    std::vector<CpuUsageInfos> workLoads = GetCpuUsage();
    std::string cpuIdStr;
    std::string userUsageStr;
    std::string niceUsageStr;
    std::string systemUsageStr;
    std::string idleUsageStr;
    std::string ioWaitUsageStr;
    std::string irqUsageStr;
    std::string softIrqUsageStr;
    std::string totalUsageStr;
    const size_t oneHundred = 100;
    if (workLoads.empty()) {
        return result;
    }
    for (size_t i = 0; i < workLoads.size(); i++) {
        cpuIdStr = workLoads[i].cpuId;
        userUsageStr = std::to_string(workLoads[i].userUsage * oneHundred);
        niceUsageStr = std::to_string(workLoads[i].niceUsage * oneHundred);
        systemUsageStr = std::to_string(workLoads[i].systemUsage * oneHundred);
        idleUsageStr = std::to_string(workLoads[i].idleUsage * oneHundred);
        ioWaitUsageStr = std::to_string(workLoads[i].ioWaitUsage * oneHundred);
        irqUsageStr = std::to_string(workLoads[i].irqUsage * oneHundred);
        softIrqUsageStr = std::to_string(workLoads[i].softIrqUsage * oneHundred);
        totalUsageStr = std::to_string((workLoads[i].userUsage + workLoads[i].niceUsage +
            workLoads[i].systemUsage + workLoads[i].ioWaitUsage +
            workLoads[i].irqUsage + workLoads[i].softIrqUsage) * oneHundred);
        return[cpuIdStr + "userUsage"] = userUsageStr;
        return[cpuIdStr + "niceUsage"] = niceUsageStr;
        return[cpuIdStr + "systemUsage"] = systemUsageStr;
        return[cpuIdStr + "idleUsage"] = idleUsageStr;
        return[cpuIdStr + "ioWaitUsage"] = ioWaitUsageStr;
        return[cpuIdStr + "irqUsage"] = irqUsageStr;
        return[cpuIdStr + "softIrqUsage"] = softIrqUsageStr;
        return[cpuIdStr + "Usage"] = totalUsageStr;
    }
    return result;
}

std::vector<CpuFreqs> CPU::GetCpuFreq()
{
    OHOS::SmartPerf::CpuFreqs cpuFreqs;
    std::vector<CpuFreqs> cpuFreqs2;
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<std::vector<CpuFreq>> result = collector->CollectCpuFrequency();
    std::vector<CpuFreqs>& cpufreq = result.data;
    for (size_t i = 0; i < cpufreq.size(); i++) {
        cpuFreqs.cpuId = cpufreq[i].cpuId;
        cpuFreqs.curFreq = cpufreq[i].curFreq;
        cpuFreqs2.push_back(cpuFreqs);
    }
    return cpuFreqs2;
}

std::vector<CpuUsageInfos> CPU::GetCpuUsage()
{
    OHOS::SmartPerf::CpuUsageInfos cpuUsageInfos;
    std::vector<CpuUsageInfos> workload;
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<SysCpuUsage> result = collector->CollectSysCpuUsage(true);
    SysCpuUsage& sysCpuUsage = result.data;
    if (sysCpuUsage.cpuInfos.empty()) {
        return workload;
    }
    for (auto& cpuInfo : sysCpuUsage.cpuInfos) {
        cpuUsageInfos.cpuId = cpuInfo.cpuId;
        cpuUsageInfos.userUsage = cpuInfo.userUsage;
        cpuUsageInfos.niceUsage = cpuInfo.niceUsage;
        cpuUsageInfos.systemUsage = cpuInfo.systemUsage;
        cpuUsageInfos.idleUsage = cpuInfo.idleUsage;
        cpuUsageInfos.ioWaitUsage = cpuInfo.ioWaitUsage;
        cpuUsageInfos.irqUsage = cpuInfo.irqUsage;
        cpuUsageInfos.softIrqUsage = cpuInfo.softIrqUsage;
        workload.push_back(cpuUsageInfos);
    }
    return workload;
}
}
}
