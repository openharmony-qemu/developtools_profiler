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
#include <sstream>
#include <fstream>
#include <climits>
#include "include/sp_utils.h"
#include "include/RAM.h"
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
