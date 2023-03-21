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
#include <thread>
#include <ios>
#include <vector>
#include <iostream>
#include <fstream>
#include <regex>
#include "include/startup_delay.h"
#include "include/sp_utils.h"
namespace OHOS {
namespace SmartPerf {
StartUpDelay::StartUpDelay() {}
StartUpDelay::~StartUpDelay() {}
void StartUpDelay::GetTrace(std::string sessionId, std::string traceName)
{
    std::string result;
    std::string cmdString{"bytrace -t 5 -b 20480 --overwrite idle ace app ohos ability graphic "};
    std::string cmdStringEnd{"sched freq irq sync workq pagecache multimodalinput > "};
    SPUtils::LoadCmd(cmdString + cmdStringEnd + traceName, result);
}
std::thread StartUpDelay::ThreadGetTrace(std::string sessionId, std::string traceName)
{
    std::thread thGetTrace(&StartUpDelay::GetTrace, this, sessionId, traceName);
    return thGetTrace;
}
void StartUpDelay::GetLayout()
{
    std::string result;
    SPUtils::LoadCmd("uitest dumpLayout", result);
}
std::thread StartUpDelay::ThreadGetLayout()
{
    std::thread thGetLayout(&StartUpDelay::GetLayout, this);
    return thGetLayout;
}
void StartUpDelay::ChangeToBackground()
{
    std::string result;
    SPUtils::LoadCmd("uinput -k -d 2 -u 2", result);
}
std::vector<std::string> StartUpDelay::GetPidByPkg(std::string curPkgName)
{
    std::string resultPids;
    SPUtils::LoadCmd("pidof" + curPkgName, resultPids);
    std::vector<std::string> pidV;
    SPUtils::StrSplit(resultPids, " ", pidV);
    return pidV;
}
void StartUpDelay::KillCurApp(std::string curPkgName)
{
    std::vector<std::string> pidV;
    do {
        pidV = GetPidByPkg(curPkgName);
        std::string result;
        for (std::string pid : pidV) {
            SPUtils::LoadCmd("Kill -9 " + pid, result);
        }
    }
    while (pidV.size() > 0);
}
}
}
