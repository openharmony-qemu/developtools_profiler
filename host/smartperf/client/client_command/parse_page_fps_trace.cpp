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


#include <fstream>
#include <string>
#include "include/parse_page_fps_trace.h"
#include "include/sp_log.h"
namespace OHOS {
namespace SmartPerf {
double ParsePageFpsTrace::PageFpsTrace(std::string file)
{
    double fps = -1.0;
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(file.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    infile.open(realPath);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        LOGE("ParsePageFpsTrace open file(%s) fialed ", file.c_str());
        return fps;
    }
    fps = SmartPerf::ParsePageFpsTrace::CalculateTime();
    infile.close();
    return fps;
}
double ParsePageFpsTrace::CalculateTime()
{
    std::string line;
    size_t offset = 2;
    double minFrameInterval = 0.05;
    while (getline(infile, line)) {
        if (line.find("H:touchEventDispatch") != std::string::npos) {
            startLine = line;
            needUpdateResponseLine = true;
            frameNum = 0;
        } else if (line.find("H:RSMainThread::DoComposition") != std::string::npos) {
            updateCount = true;
            count = 0;
            pid = SmartPerf::ParsePageFpsTrace::CutString(line, "B|", "|H:RSMainThread::DoComposition", offset);
            frameNum++;
            if (needUpdateResponseLine) {
                responseLine = line;
                needUpdateResponseLine = false;
            }
            frameStartTime = std::stod(SmartPerf::ParsePageFpsTrace::GetLineTime(line));
        }
        if (updateCount) {
            if (line.find("B|" + pid + "|") != std::string::npos && line.find("-" + pid) != std::string::npos) {
                count++;
            } else if (line.find("E|" + pid + "|") != std::string::npos && line.find("-" + pid) != std::string::npos) {
                count--;
            }
            if (count == 0) {
                completeLine = line;
                frameStartInterval = frameEndTime;
                frameEndTime = std::stod(SmartPerf::ParsePageFpsTrace::GetLineTime(completeLine));
                updateCount = false;
            }
        }
        if (frameStartInterval != 0) {
            double frameInterval = frameStartTime - frameStartInterval;
            if (frameInterval > minFrameInterval) {
                std::cout << "NO." << frameNum << "fps Time: " << frameInterval << "s" << std::endl;
            }
        }
    }
    return CalculateTimeEnd();
}

double ParsePageFpsTrace::CalculateTimeEnd()
{
    if (startLine.compare("") == 0) {
        LOGW("can not find start point {H:touchEventDispatch}");
    } else if (completeLine.compare("") == 0) {
        LOGW("can not find response and complete point {H:RSMainThread::DoComposition}");
    } else {
        double responseTime = std::stod(SmartPerf::ParsePageFpsTrace::GetLineTime(responseLine));
        double completeTime = std::stod(SmartPerf::ParsePageFpsTrace::GetLineTime(completeLine));
        return frameNum / (completeTime - responseTime);
    }
    return -1.0;
}

std::string ParsePageFpsTrace::GetLineTime(std::string line)
{
    size_t num = 7;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    return line.substr(position1 + num, position2 - position1 - num);
}
std::string ParsePageFpsTrace::CutString(std::string line, const std::string &start,
    const std::string &end, size_t offset)
{
    size_t position1 = line.find(start);
    size_t position2 = line.find(end);
    return line.substr(position1 + offset, position2 - position1 - offset);
}
}
}