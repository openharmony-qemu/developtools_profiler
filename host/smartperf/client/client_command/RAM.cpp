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
    std::string pssValue = "";
    ramInfo["pss"] = "-1";
    if (processId.size() == 0) {
        return ramInfo;
    }
    std::string cmd = "cat /proc/" + processId + "/smaps_rollup";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return ramInfo;
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        if (line[0] == '-' || line[0] == '') {
            continue;
        }
        if (line.find("Pss:") != std::string::npos) {
            std::regex pattern("\\d+");
            std::smatch result;
            if (std::regex_search(line, result, pattern)) {
                pssValue = result[0];
            }
        }
        if (pssValue.size() > 0) {
            break;
        }
    }
    pclose(fd);
    if (pssValue.size() > 0) {
        ramInfo["pss"] = pssValue;
    }
    return ramInfo;
}
}
}
