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
#ifndef STALLINGRATETRACE_H
#define STALLINGRATETRACE_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace OHOS {
namespace SmartPerf {
class StallingRateTrace {
public:
    double StallingRateResult(std::string file);
    double CalculateTime();
    double GetFrameRate(std::string line) const;
    int GetFenceId(std::string line) const;
    std::string GetOnScreenTimeStart(std::string line) const;
    double GetTimes(std::string line, const std::string &sign) const;
    void GetScreenInfo(std::string line);
    void CalculateRoundTime();
    void AppList(std::string line, const std::string &signS, const std::string &signF);
    void AppSwiperScroll(std::string line, const std::string &signS);
private:
    std::ifstream infile;
    double nowFrameRate = 0;
    double oneThousand = 1000;
    double roundTime = 0;
    int fenceId = 0;
    double nowTime = 0;
    double lastTime = 0;
    double frameLossTime = 0;
    double frameLossRate = 0;
    double dynamicStartTime = 0;
    double dynamicFinishTime = 0;
    bool animalFlag = false;
    bool upperScreenFlag = false;
    int swiperScrollFlag = 0;
    int swiperFlingFlag = 0;
};
}
}
#endif // STALLINGRATETRACE_H