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
#include <iostream>
#include <regex>
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

void ParseSlideFpsTrace::GetLastCompleteLine()
{
    if (lastCompleteLine != "") {
        double completeNow = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(newCompleteLine));
        double completeLast = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(lastCompleteLine));
        float num = 0.1;
        if (completeNow - completeLast > num) {
            completeLine = lastCompleteLine;
            updateCount = true;
            frameNum = frameNow - 1;
        }
    }
}

double ParseSlideFpsTrace::CalculateTime()
{
    std::string line;
    while (getline(infile, line)) {
        if (line.find("H:RSJankStats::RecordAnimationDynamicFrameRate") != std::string::npos) {
            std::string delimiter = "frame rate is ";
            size_t pos1 = line.find(delimiter);
            std::string result1 = line.substr(pos1 + delimiter.length());
            std::string delimiter1 = ":";
            size_t pos2 = line.find(delimiter1);
            std::string result2 = result1.substr(0, pos2);
            frameRate = std::stod(result2);
        }
        if (line.find("H:touchEventDispatch") != std::string::npos) {
            count++;
            if (count == four) {
                needUpdateResponseLine = true;
                frameNow = 0;
            }
        } else if (line.find("H:RSMainThread::DoComposition") != std::string::npos && !updateCount) {
            frameNow++;
            if (needUpdateResponseLine) {
                responseLine = line;
                needUpdateResponseLine = false;
            }
            newCompleteLine = line;
            GetLastCompleteLine();
            lastCompleteLine = newCompleteLine;
        }
    }
    if (frameRate == 0) {
        completeLine = newCompleteLine;
        frameNum = frameNow;
        double responseTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(responseLine));
        double completeTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(completeLine));
        return frameNum / (completeTime - responseTime);
    } else {
        return frameRate;
    }
    return -1.0;
}
std::string ParseSlideFpsTrace::GetLineTime(std::string lineStr) const
{
    size_t num = 7;
    size_t position1 = lineStr.find("....");
    size_t position2 = lineStr.find(":");
    return lineStr.substr(position1 + num, position2 - position1 - num);
}
std::string ParseSlideFpsTrace::CutString(std::string lineStr, std::string start, std::string end, size_t offset) const
{
    size_t position1 = lineStr.find(start);
    size_t position2 = lineStr.find(end);
    return lineStr.substr(position1 + offset, position2 - position1 - offset);
}
}
}
