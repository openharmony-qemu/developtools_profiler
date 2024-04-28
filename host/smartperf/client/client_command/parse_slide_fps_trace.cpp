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
#include "include/parse_slide_fps_trace.h"
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
double ParseSlideFpsTrace::ParseSlideFpsTraceNoh(std::string file)
{
    double fps = -1.0;
    char realPath[PATH_MAX] = {0x00};
    if (realpath(file.c_str(), realPath) == nullptr) {
        std::cout << "" << std::endl;
    }
    infile.open(realPath);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        LOGE("ParseSlideFpsTrace open file(%s) fialed ", file.c_str());
        return fps;
    } else {
        fps = SmartPerf::ParseSlideFpsTrace::CalculateTime();
    }
    return fps;
}

double ParseSlideFpsTrace::CalculateTime()
{
    std::string line;
    while (getline(infile, line)) {
        if (line.find("H:touchEventDispatch") != std::string::npos) {
            count++;
            if (count == four) {
                needTime = true;
                frameNow = 0;
                touchTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(line));
            }
        } else if (line.find("H:RSMainThread::DoComposition") != std::string::npos) {
            frameNow++;
            doCompositionTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(line));
            if (flagWeb) {
                completeTime = doCompositionTime;
                frameNum = frameNow;
                flagWeb = false;
            }
        } else if (line.find("H:WEB_LIST_FLING") != std::string::npos ||
            line.find("H:APP_LIST_FLING 0") != std::string::npos) {
            flagWeb = true;
        }
        if (touchTime != 0 && (doCompositionTime - touchTime) > completionTime && needTime) {
            frameNow = 1;
            needTime = false;
            responseTime = doCompositionTime;
        }
    }
    if (completeTime == 0 || responseTime == 0) {
        return -1;
    } else {
        return frameNum / (completeTime - responseTime);
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
std::string ParseSlideFpsTrace::CutString(std::string lineStr, const std::string &start,
    const std::string &end, size_t offset) const
{
    size_t position1 = lineStr.find(start);
    size_t position2 = lineStr.find(end);
    return lineStr.substr(position1 + offset, position2 - position1 - offset);
}
}
}
