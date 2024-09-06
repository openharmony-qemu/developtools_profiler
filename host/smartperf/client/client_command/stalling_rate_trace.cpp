/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include <fstream>
#include <string>
#include <iostream>
#include <regex>
#include <cmath>
#include "include/stalling_rate_trace.h"
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
double StallingRateTrace::StallingRateResult(std::string file)
{
    double stalligRate = 0;
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(file.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    infile.open(realPath);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        LOGI("StallingRateTrace open file(%s) fialed ", file.c_str());
        return stalligRate;
    }
    stalligRate = SmartPerf::StallingRateTrace::CalculateTime();
    infile.close();
    return stalligRate;
}

double StallingRateTrace::CalculateTime()
{
    frameLossRate = 0;
    frameLossTime = 0;
    std::string signS = "S|";
    std::string signF = "F|";
    std::string line;
    while (getline(infile, line)) {
        AppList(line, signS, signF);
        AppSwiperScroll(line, signS);
        if (line.find("H:APP_SWIPER_FLING,") != std::string::npos ||
            line.find("H:APP_SWIPER_NO_ANIMATION_SWITCH") != std::string::npos ||
            line.find("H:APP_SWITCH_FRAME_ANIMATION") != std::string::npos) {
            if (swiperFlingFlag == 1) {
                dynamicFinishTime = GetTimes(line, signF);
                animalFlag = false;
            }
            if (dynamicFinishTime == 0) {
                swiperFlingFlag = 0;
            } else if (dynamicStartTime != 0) {
                break;
            }
            swiperFlingFlag++;
        }
        if (animalFlag) {
            if (line.find("H:RSHardwareThread::CommitAndReleaseLayers") != std::string::npos) {
                upperScreenFlag = true;
                nowFrameRate = GetFrameRate(line);
                LOGI("nowFrameRate1=====: %s", std::to_string(nowFrameRate).c_str());
                CalculateRoundTime();
            } else if (line.find("H:RSHardwareThread::PerformSetActiveMode setting active mode") != std::string::npos) {
                upperScreenFlag = true;
                nowFrameRate = GetFrameRate(line);
                LOGI("nowFrameRate2=====: %s", std::to_string(nowFrameRate).c_str());
                CalculateRoundTime();
            }
            GetScreenInfo(line);
        }
    }
    if (dynamicStartTime > dynamicFinishTime) {
        LOGI("StallingRateTrace::APP_SWIPER_FLING AND APP_LIST_FLING");
        dynamicFinishTime = nowTime;
    }
    if (dynamicStartTime == 0 || dynamicFinishTime == 0) {
        frameLossRate = -1;
    } else {
        frameLossRate = (frameLossTime / (dynamicFinishTime - dynamicStartTime) * oneThousand);
        LOGI("frameLossRate=====: %s", std::to_string(frameLossRate).c_str());
    }
    return frameLossRate;
}

void StallingRateTrace::AppList(std::string line, const std::string &signS, const std::string &signF)
{
    if (line.find("H:LAUNCHER_APP_LAUNCH_FROM_ICON,") != std::string::npos ||
        line.find("H:APP_LIST_FLING,") != std::string::npos ||
        line.find("H:WEB_LIST_FLING,") != std::string::npos ||
        line.find("H:ABILITY_OR_PAGE_SWITCH,") != std::string::npos ||
        line.find("H:APP_TRANSITION_TO_OTHER_APP,") != std::string::npos ||
        line.find("H:LAUNCHER_APP_LAUNCH_FROM_DOCK,") != std::string::npos ||
        line.find("H:LAUNCHER_APP_LAUNCH_FROM_APPCENTER,") != std::string::npos ||
        line.find("H:APP_TABS_NO_ANIMATION_SWITCH") != std::string::npos ||
        line.find("H:APP_TABS_FRAME_ANIMATION") != std::string::npos ||
        line.find("H:APP_TABS_SCROLL,") != std::string::npos) {
        if (animalFlag) {
            dynamicFinishTime = GetTimes(line, signF);
            LOGI("dynamicFinishTime=====: %s", std::to_string(dynamicFinishTime).c_str());
            animalFlag = false;
        } else {
            dynamicStartTime = GetTimes(line, signS);
            LOGI("dynamicStartTime=====: %s", std::to_string(dynamicStartTime).c_str());
            animalFlag = true;
            swiperScrollFlag = 0;
            frameLossTime = 0;
        }
    }
}

void StallingRateTrace::AppSwiperScroll(std::string line, const std::string &signS)
{
    if (line.find("H:APP_SWIPER_SCROLL,") != std::string::npos ||
        line.find("H:APP_SWIPER_NO_ANIMATION_SWITCH") != std::string::npos ||
        line.find("H:APP_SWITCH_FRAME_ANIMATION") != std::string::npos) {
        if (swiperScrollFlag == 0) {
            dynamicStartTime = GetTimes(line, signS);
            frameLossTime = 0;
            swiperScrollFlag = 1;
            animalFlag = true;
        }
    }
}

void StallingRateTrace::CalculateRoundTime()
{
    double kadunNum = 1.5;
    if (nowFrameRate != 0) {
        roundTime = (1 / nowFrameRate) * kadunNum;
    }
}
void StallingRateTrace::GetScreenInfo(std::string line)
{
    if (upperScreenFlag) {
        if (line.find("|H:Present Fence ") != std::string::npos) {
            fenceId = GetFenceId(line);
            LOGI("fenceId=====: %s", std::to_string(fenceId).c_str());
        }
        
        std::string waitFenceId = "|H:Waiting for Present Fence " + std::to_string(fenceId);
        if (line.find(waitFenceId) != std::string::npos) {
            nowTime = std::stod(SmartPerf::StallingRateTrace::GetOnScreenTimeStart(line));
            LOGI("nowTime=====: %s", std::to_string(nowTime).c_str());
            if ((nowTime - lastTime) > roundTime && lastTime != 0) {
                double diffTime = (nowTime - lastTime) - roundTime;
                frameLossTime += diffTime;
                LOGI("frameLossTime=====: %s", std::to_string(frameLossTime).c_str());
            }
            lastTime = nowTime;
            upperScreenFlag = false;
        }
    }
}

double StallingRateTrace::GetFrameRate(std::string line) const
{
    double rate = 0;
    std::string delimiter = "rate: ";
    if (line.find("now:") != std::string::npos && line.find("rate:") != std::string::npos) {
        std::string delimiter1 = ", now:";
        size_t pos1 = line.find(delimiter);
        std::string result1 = line.substr(pos1 + delimiter.length());
        size_t pos2 = line.find(delimiter1);
        std::string result2 = result1.substr(0, pos2);
        rate = std::stod(result2.c_str());
    }
    if (line.find("rate:") != std::string::npos) {
        size_t pos = line.find(delimiter);
        std::string result = line.substr(pos + delimiter.length());
        rate = std::stod(result.c_str());
    }
    return rate;
}

int StallingRateTrace::GetFenceId(std::string line) const
{
    std::string delimiter = "H:Present Fence ";
    size_t pos = line.find(delimiter);
    std::string result = line.substr(pos + delimiter.length());
    int presentFenceId = std::atoi(result.c_str());
    return presentFenceId;
}

std::string StallingRateTrace::GetOnScreenTimeStart(std::string line) const
{
    std::string startTime = "0";
    size_t subNum = 7;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    startTime = line.substr(position1 + subNum, position2 - position1 - subNum);
    return startTime;
}

double StallingRateTrace::GetTimes(std::string line, const std::string &sign) const
{
    double signTime = 0;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    if (line.find(sign) != std::string::npos) {
        size_t subNum = 7;
        signTime = std::stod(line.substr(position1 + subNum, position2 - position1 - subNum));
    }
    return signTime;
}
}
}
