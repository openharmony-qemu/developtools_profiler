/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
double ParseSlideFpsTrace::CalculateTime()
{
    while (getline(infile, line)) {
        if (line.find("H:OnTouchEvent") != maxnpos) {
            startLine = line;
            needUpdateResponseLine = true;
            frameNum = 0;
        }  else if (line.find("H:RSMainThread::DoComposition") != maxnpos && animatorSecond < numA) {
            updateCount = true;
            count = 0;
            pid = CutString(line, "B|", "H:RSMainThread::DoComposition", numA);
            frameNum++;
            if (needUpdateResponseLine && frameNum == numA) {
                responseLine = line;
            }
            frameStartTime = std::stod(GetLineTime(line));
            if (animatorFlag) {
                animatorSecond++;
                completeLine = line;
            }
        }
        if (line.find("H:Animator: friction") != maxnpos && line.find("F|") != maxnpos) {
            animatorFlag = true;
        }
    }
    if (startLine.compare("") == 0) {
        std::cout << "can not find start point {H:touchEventDispatch}" << std::endl;
    } else if (completeLine.compare("") == 0) {
        std::cout << "can not find complete point {H:RSMainThread::leashWindow}" << std::endl;
    } else {
        double responseTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(responseLine));
        double completeTime = std::stod(SmartPerf::ParseSlideFpsTrace::GetLineTime(completeLine));
        int endFrameNum = frameNum - numA;
        return endFrameNum / (completeTime - responseTime);
    }
    return -1.0;
}
std::string ParseSlideFpsTrace::GetLineTime(std::string lineStr)
{
    size_t num = 7;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    return line.substr(position1 + num, position2 - position1 - num);
}
std::string ParseSlideFpsTrace::CutString(std::string lineStr, std::string start, std::string end, size_t off)
{
    size_t position1 = line.find(start);
    size_t position2 = line.find(end);
    return line.substr(position1 + off, position2 - position1 - off);
}
}
}
