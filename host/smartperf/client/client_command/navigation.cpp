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
std::map<std::string, std::string> navigation::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> navInfo = navigation::GetNavInfo();
    for (auto it = navInfo.begin(); it != navInfo.end(); ++it) {
        result.insert(*it);
    }
    LOGI("navigation::ItemData map size(%u)", result.size());
    return result;
}

void navigation::SetPackageName(const std::string &pName)
{
    packageName = pName;
}

std::map<std::string, std::string> navigation::GetNavInfo() const
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
        navInfo["navPathName"] = "no-Navigation";
        navInfo["navPathParam"] = "";
    }
    return navInfo;
}

std::map<std::string, std::string> navigation::GetNavResult(std::string winId) const
{
    std::map<std::string, std::string> navInfo;
    std::string nameStr = "no-Navigation";
    std::sttring paramStr = "";
    const std::string winIdWitchSpace = " " + winId + " ";
    const std::string cmd = "hidumper -s WindowManagerService -a '-w" + winIdWitchSpace + "-navigation'";
    FILE *navfd = popen(cmd.c_str(), "r");
    if (navfd == nullptr) {
        navInfo["navPathName"] = nameStr;
        navInfo["navPathParam"] = "";
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), navfd)) != nullptr) {
        std::string line = buf;
        if (line.find("]{ name:") != std::string::npos) {
            size_t pos = line.find("]{ name:");
            size_t pos2 = line.find_last_of("}");
            if (line.find(",") != std::string::npos) {
                size_t pos1 = line.find_last_of(",");
                nameStr = line.substr(pos + 10, pos1 - pos - 11);
                paramStr = line.substr(pos + 9, pos2 - pos1 - 10);
            } else {
                nameStr = line.substr(pos + 10, pos2 - pos - 12);
                paramStr = "";
            }

        }
    }
    pclose(navfd);
    navInfo["navPathName"] = nameStr;
    if (paramStr != "") {
        replace(paramStr.begin(), paramStr.end(), ',', ';');
    }
    navInfo["navPathParam"] = paramStr;
    return navInfo;
}

std::string navigation::GetWinId(std::string processId) const 
{
    std::vector<DumpEntity> dumpEntityList;
    std::string wid = "-1";
    const std::string cmd = "hidumper -s WindowManagerService -a '-a'";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd != nullptr) {
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
            if (params[2].find("Pid") != std::string::npos && params[3].find("WinId") != std::string::npos) {
                continue;
            }
            if (params.size() > 24) {
                DumpEntity dumpEntity { params[2], params[3] };
                dumpEntityList.push_back(dumpEntity);
            }
            lineNum++;
        }
        pclose(fd);
    }
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntity dumpItem = dumpEntityList[i];
        if (dumpItem.pid == processId) {
            wid = dumpItem.pid;
            break;
        }

    }
    return wid;
}
}
}
