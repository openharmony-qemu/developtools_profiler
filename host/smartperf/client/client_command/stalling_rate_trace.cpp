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
    infile.open(file);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        LOGE("ParseSlideFpsTrace open file(%s) fialed ", file.c_str());
    } else {
        stalligRate = SmartPerf::StallingRateTrace::CalculateTime();
    }
    return stalligRate;
}

double StallingRateTrace::CalculateTime()
{
    frameLossRate = 0;
    frameLossTime = 0;
    std::string signS = "|S";
    std::string signF = "|F";
    std::string line;
    while (getline(infile, line)) {
        if (line.find("H:LAUNCHER_APP_LAUNCH_FROM_ICON") != std::string::npos) {
            if (animalFlag) {
                dynamicFinishTime = GetTimes(line signF);
                animalFlag = false;
            } else {
                dynamicStartTime = GetTimes(line signS);
                animalFlag = true;
            }
        }
        if (animalFlag) {
            if (line.find("H:RSHardwareThread::CommirAndReleaseLayers") != std::string::npos) {
                upperScreenFlag = true;
                nowFrameRate = GetFrameRate(line);
                roundTime = oneThousand / nowFrameRate;
            } else if (line.find("H:RSHardwareThread::PerformSetActiveMode setting active mode") != std::string::npos) {
                upperScreenFlag = true;
                nowFrameRate = GetFrameRate(line);
                roundTime = oneThousand / nowFrameRate;
            }
            if (upperScreenFlag) {
                if (line.find("|H:Present Fence ") != std::string::npos) {
                    fenceId = GetFenceId(line);
                }
                syd::string waitFenceId = "|H:Waiting for Present Fence " + std::to_string(fenceId);
                if (line.find(waitFenceId) != std::string::npos) {
                   nowTime = std::stod(SmartPerf::StallingRateTrace::GetOnScreenTimeStart(line)); 
                }
                if ((nowTime - lastTime) > roundTime && lastTime != 0) {
                    double diffTime = (nowTime - lastTime) - roundTime;
                    frameLossTime += diffTime;
                }
                lastTime = nowTime;
                upperScreenFlag = false;
            }
        }
    }
    if (frameLossTime == 0) {
        frameLossRate = 0;
    } else {
        frameLossRate = (frameLossTime / (dynamicFinishTime - dynamicStartTime) * oneThousand);
    }
    return std::sqrt(frameLossRate);
}

double StallingRateTrace::GetFrameRate(std::string line)
{
    std::string delimiter = "rate: ";
    std::string delimiter1 = ", now:";
    size_t pos1 = line.find(delimiter);
    std::string result1 = line.substr(pos1 + delimiter.length());
    size_t pos2 = line.find(delimiter1);
    std::string result2 = result1.substr(0, pos2);
    double rate = std::stod(result2.c_str());
    return rate;
}

int StallingRateTrace::GetFenceId(std::string line)
{
    std::string delimiter = "H:Present Fence ";
    size_t pos = line.find(delimiter);
    std::string result = line.substr(pos + delimiter.length());
    int presentFenceId = std::atoi(result.c_str());
    return presentFenceId;
}

std::string StallingRateTrace::GetOnScreenTimeStart(td::string line)
{
    std::string startTime = "0";
    size_t subNum = 5;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    startTime = line.substr(position1 + subNum, position2 - position1 - subNum);
    return startTime;
}

double StallingRateTrace::GetTimes(std::string line, std::string sign)
{
    double signTime = 0;
    size_t subNum = 5;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    if (line.find(sign) != std::string::npos) {
        signTime = std::stod(line.substr(position1 + subNum, position2 - position1 - subNum));
    }
    return signTime;
}
}
}
