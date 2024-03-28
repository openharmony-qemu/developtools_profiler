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
    if (packageName.length() > 0) {
        std::map<std::string, std::string> procRamInfo = RAM::GetRamInfo();
        for (auto it = procRamInfo.begin(); it != procRamInfo.end(); ++it) {
            result.insert(*it);
        }
    }

    return result;
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

void RAM::SetPackageName(std::string pName)
{
    packageName = pName;
}

std::map<std::string, std::string> RAM::GetRamInfo() const
{
    std::string processId = "";
    std::map<std::string, std::string> ramInfo;
    std::string pssValue = "";
    ramInfo["pss"] = "-1";
    if (processId.size() > 0) {
        const int zero = 0;
        const int one = 1;
        const int two = 2;
        std::ostringstream cmdGrep;
        cmdGrep.str("");
        cmdGrep << "/proc/" << processId << "/smaps_rollup";
        std::string cmdRam = cmdGrep.str();
        char realPath[PATH_MAX] = {0x00};
        if (realpath(cmdRam.c_str(), realPath) == nullptr) {
            std::cout << "" << std::endl;
        }
        std::ifstream infile(realPath, std::ios::in);
        if (!infile) {
            return ramInfo;
        }
        std::string textline;
        while (getline(infile, textline, '\n')) {
            if (textline[zero] == 'P' && textline[one] == 's' && textline[two] == 's') {
                pssValue = textline;
                break;
            }
        }
    }
    if (pssValue.size() > 0) {
        ramInfo["pss"] = SPUtils::ExtractNumber(pssValue.c_str());
    }
    return ramInfo;
}
}
}
