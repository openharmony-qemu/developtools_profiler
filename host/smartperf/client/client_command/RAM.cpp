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
#include <iostream>
#include <string>
#include <climits>
#include "include/sp_utils.h"
#include "memory_collector.h"
#include "collect_result.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> RAM::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> ramInfo = RAM::GetRamInfo();
    result = ramInfo;
    return result;
}

void RAM::SetProcessId(std::string pid)
{
    processId = std::move(pid);
}
std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::map<std::string, std::string> ramInfo;
    if (processId.length > 0) {
        int32_t procId = 0;
        procId = std::stoi(processId);
        std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
        CollectResult<ProcessMemory> result = collector->CollectProcessMemory(procId);
        if (sizeof(result.data) != 0) {
            ramInfo["pss"] = std::to_string(result.data.pss);
            ramInfo["pid"] = std::to_string(result.data.pid);
            ramInfo["name"] = result.data.name;
            return ramInfo;
        }
    }
    return ramInfo;
}
}
}
