/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "include/processes_cpu.h"
#include <sstream>
#include <cstdio>
#include <iostream>
#include <string>
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
std::map<std::string, std::string> ProcessesCpu::ItemData()
{
    std::map<std::string, std::string> result;
    uint64_t procStartTime = GetSysProcessCpuLoad();
    if (procStartTime == 0) {
        return result;
    }
    std::map<std::string, std::string> processCpuInfo = ProcessesCpu::GetSpecProcessCpuLoad();
    result = processCpuInfo;
    return result;
}

void ProcessesCpu::SetProcessId(std::string pid)
{
    processId = std::move(pid);
}

uint64_t ProcessesCpu::GetSysProcessCpuLoad()
{
    uint64_t startTime = 0;
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    auto collectResult = collector->CollectProcessCpuStatInfos(true);
    for (size_t i = 0; i < collectResult.data.size(); i++) {
        startTime = collectResult.data[i].startTime;
    }
    return startTime;
}

std::map<std::string, std::string> ProcessesCpu::GetSpecProcessCpuLoad() const
{
    std::map<std::string, std::string> processCpuInfo;
    const size_t oneHundred = 100;
    if (processId.length() > 0) {
        int32_t procId = 0;
        procId = std::stoi(processId);
        std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
        auto collectResult = collector->CollectProcessCpuStatInfo(procId);
        auto data = collectResult.data;
        processCpuInfo["ProcId"] = std::to_string(data.pid);
        processCpuInfo["ProcAppName"] = data.procName;
        processCpuInfo["ProcCpuLoad"] = std::to_string(data.cpuLoad * oneHundred);
        processCpuInfo["ProcCpuUsage"] = std::to_string(data.cpuUsage * oneHundred);
        processCpuInfo["ProcUCpuUsage"] = std::to_string(data.uCpuUsage * oneHundred);
        processCpuInfo["ProcSCpuUsage"] = std::to_string(data.sCpuUsage * oneHundred);
        return processCpuInfo;
    }
    return processCpuInfo;
}
}
}

