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
#include "include/navigation.h"
#include <iostream>
#include "include/sp_utils.h"
#include "include/startup_delay.h"
#include "include/sp_log.h"
#include "common.h"

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> Navigation::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> navInfo = Navigation::GetNavInfo();
    for (auto it = navInfo.begin(); it != navInfo.end(); ++it) {
        result.insert(*it);
    }
    LOGI("Navigation::ItemData map size(%u)", result.size());
    return result;
}

void Navigation::SetPackageName(const std::string &pName)
{
    packageName = pName;
}

std::map<std::string, std::string> Navigation::GetNavInfo() const
{
    std::map<std::string, std::string> navInfo;
    std::string processId = "";
    std::string winId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(packageName);
    winId = GetWinId(processId);
    if (winId != "-1") {
        navInfo = GetNavResult(winId);
    } else {
        navInfo["navPathName"] = "No Navigation Info";
    }
    return navInfo;
}

std::map<std::string, std::string> Navigation::GetNavResult(std::string winId) const
{
    std::map<std::string, std::string> navInfo;
    std::string nameStr = "No Navigation Info";
    std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_NAV) + winId + " -navigation'";
    if (cmd.empty()) {
        navInfo["navPathName"] = nameStr;
        return navInfo;
    }
    FILE *navfd = popen(cmd.c_str(), "r");
    if (navfd == nullptr) {
        navInfo["navPathName"] = nameStr;
        return navInfo;
    }
    char buf[4096] = {'\0'};
    while ((fgets(buf, sizeof(buf), navfd)) != nullptr) {
        std::string line = buf;
        if (line.find("]{ name:") != std::string::npos) {
            size_t pos = line.find("]{ name:");
            size_t pos2 = line.find_last_of("}");
            if (line.find(",") != std::string::npos) {
                size_t pos1 = line.find(",");
                nameStr = line.substr(pos + paramTen, pos1 - pos - paramEleven);
            } else {
                nameStr = line.substr(pos + paramTen, pos2 - pos - paramTwelve);
            }
        }
    }
    pclose(navfd);
    navInfo["navPathName"] = nameStr;
    return navInfo;
}

std::string Navigation::GetWinId(std::string processId) const
{
    std::string wid;
    const std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_A_A);
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return "-1";
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        if (line.find("---") != std::string::npos || line.length() <= 1 ||
            line.find("WindowName") != std::string::npos) {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (static_cast<int>(params.size()) > paramThree) {
            if (params[paramTwo] == processId) {
                wid = params[paramThree];
                break;
            }
        }
    }
    pclose(fd);
    return wid;
}
}
}
