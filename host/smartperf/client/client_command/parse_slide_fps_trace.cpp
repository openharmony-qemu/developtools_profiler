/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <fstream>
#include <string>
#include "include/parse_slide_fps_trace.h"

namespace OHOS {
namespace SmartPerf {
double ParseSlideFpsTrace::ParseSlideFpsTraceNoh(std::string file)
{
    double fps = -1.0;
    infile.open(file);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        return fps;
    } else {
        fps = SmartPerf::ParseSlideFpsTrace::CalculateTime();
    }
    return fps;
}

bool ParseSlideFpsTrace::HandleTraceOne()
{
    if (needUpdateResponseLine) {
        if (line.find("H:RSMainThread::DoComposition") != maxnpos) {
            doFlag = true;
            if (firstFrame == 0) {
                responseLine = line;
                firstFrame = 1;
            }
            if (animEmptyFlag) {
                frameNum++;
                completeLine = line;
                return true;
            } else {
                completeLine = line;
                frameNum++;
            }
            animEmptyFlag = false;
        }
    }
    return false;
}

bool ParseSlideFpsTrace::HandleTraceThree()
{
    const double one = 0.1;
    nowTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(line));
    if (lastTime == 0) {
        lastTime = nowTime;
    } else {
        if (nowTime - lastTime > one) {
            return true;
        } else {
            lastTime = nowTime;
        }
    }
    return false;
}

bool ParseSlideFpsTrace::HandleTraceTwo()
{
    if (line.find("H:rs_RequestNextVSync") != maxnpos) {
        if (!animEmptyFlag) {
            return HandleTraceThree();
        }
    }
    return false;
}

double ParseSlideFpsTrace::CalculateTime()
{
    const int four = 4;
    while (getline(infile, line)) {
        if (line.find("H:touchEventDispatch") != maxnpos) {
            touchNum++;
            if (touchNum == four) {
                startLine = line;
                needUpdateResponseLine = true;
            }
            frameNum = 0;
        }
        if (HandleTraceOne()) {
            break;
        }
        if (line.find("H:Animate") != maxnpos) {
            animateFlag = true;
        }
        if (animateFlag && needUpdateResponseLine && doFlag) {
            if (HandleTraceTwo()) {
                break;
            }
            if (line.find("H:Animation running empty") != maxnpos) {
                animEmptyFlag = true;
            }
        }
    }
    if (startLine.compare("") == 0) {
        std::cout << "can not find start point {H:touchEventDispatch}" << std::endl;
    } else if (completeLine.compare("") == 0) {
        std::cout << "can not find complete point {H:rs_RequestNextVSync}" << std::endl;
    } else {
        double responseTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(responseLine));
        double completeTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(completeLine));
        return frameNum / (completeTime - responseTime);
    }
    return -1.0;
}
std::string ParseSlideFpsTrace::GetLineTime(std::string lineStr)
{
    size_t num = 7;
    size_t position1 = lineStr.find("....");
    size_t position2 = lineStr.find(":");
    return lineStr.substr(position1 + num, position2 - position1 - num);
}
std::string ParseSlideFpsTrace::CutString(std::string lineStr, std::string start, std::string end, size_t off)
{
    size_t position1 = lineStr.find(start);
    size_t position2 = lineStr.find(end);
    return lineStr.substr(position1 + off, position2 - position1 - off);
}
}
}
