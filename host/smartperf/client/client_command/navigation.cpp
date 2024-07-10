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
    const std::string winIdWithSpace = " " + winId + " ";
    const std::string cmd = "hidumper -s WindowManagerService -a '-w" + winIdWithSpace + "-navigation'";
    FILE *navfd = popen(cmd.c_str(), "r");
    if (navfd == nullptr) {
        navInfo["navPathName"] = nameStr;
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
    std::vector<DumpEntity> dumpEntityList;
    std::string wid;
    const std::string cmd = "hidumper -s WindowManagerService -a '-a'";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        wid = -1;
    }
    int lineNum = 0;
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        if (line.length() == 1) {
            continue;
        }
        if (line[0] == '-' || line[0] == ' ') {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params[paramTwo].find("Pid") != std::string::npos &&
            params[paramThree].find("WinId") != std::string::npos) {
            continue;
        }
        if (static_cast<int>(params.size()) > paramTwentyFour) {
            DumpEntity dumpEntity { params[2], params[3] };
            dumpEntityList.push_back(dumpEntity);
        }
        lineNum++;
    }
    pclose(fd);
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntity dumpItem = dumpEntityList[i];
        if (dumpItem.pid == processId) {
            wid = dumpItem.winId;
            break;
        }
    }
    return wid;
}
}
}
