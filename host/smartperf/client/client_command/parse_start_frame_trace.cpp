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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "include/parse_start_frame_trace.h"
#include "include/sp_utils.h"
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
    const unsigned int SFTN_UI_INDEX_2 = 2;
    const unsigned int SFTN_UI_INDEX_3 = 3;
double StartFrameTraceNoh::ParseStartFrameTraceNoh(std::string file)
{
    double frameRate = -1.0;
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(file.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    infile.open(realPath);
    if (infile.fail()) {
        std::cout << "file open fail:" << file << std::endl;
        LOGE("ParseStartFrameTraceNoh open file(%s) failed ", file.c_str());
        return frameRate;
    }
    frameRate = SmartPerf::StartFrameTraceNoh::CalculateTime();
    infile.close();
    return frameRate;
}
double StartFrameTraceNoh::CalculateTime()
{
    std::string line;
    std::string screenStr = SPUtils::GetScreen();
    size_t pos = screenStr.find("x");
    std::string width = screenStr.substr(twelve, pos - twelve);
    std::string height = screenStr.substr(pos + 1, four);
    while (getline(infile, line)) {
        if (line.find("H:touchEventDispatch") != std::string::npos) {
            startLine = line;
            frameNum = 0;
        } else if (line.find("H:RSUniRender::Process:[WindowScene") != std::string::npos) {
            std::string tmpStr = Split(line, "H:RSUniRender::Process:[WindowScene")[1];
            std::string numsStr = CutString(tmpStr, "(", ")", 1);
            std::vector<std::string> nums = Split(numsStr, ",");
            std::string alpha = Split(tmpStr, "Alpha:")[1];
            if ((stof(nums[0]) == 0 && stof(nums[1]) == 0 && stof(nums[SFTN_UI_INDEX_2]) == stof(width) &&
                stof(nums[SFTN_UI_INDEX_3]) == stof(height)) &&
                stof(alpha) == 1) {
                fullScreenLine = line;
                break;
            }
        } else if (line.find("H:RSMainThread::DoComposition") != std::string::npos) {
            isNeedEnd = true;
            frameNum++;
            count = 0;
            int size = 2;
            pid = CutString(line, "B|", "|H:RSMainThread::DoComposition", size);
            frameStartTime = std::stod(GetLineTime(line));
            startTimes.frameId = frameNum;
            startTimes.times = frameStartTime;
            startTimeArr.push_back(startTimes);
        }
        if (isNeedEnd) {
            if (line.find("B|" + pid + "|") != std::string::npos && line.find("-" + pid) != std::string::npos) {
                count++;
            } else if (line.find("E|" + pid + "|") != maxnpos && line.find("-" + pid) != maxnpos) {
                count--;
            }
            if (count == 0) {
                frameEndTime = std::stod(GetLineTime(line));
                endTimes.frameId = frameNum;
                endTimes.times = frameEndTime;
                endTimeArr.push_back(endTimes);
                isNeedEnd = false;
            }
        }
    }
    double fps = GetFps();
    return fps;
}
double StartFrameTraceNoh::GetFps()
{
    if (startLine.compare("") == 0) {
        std::cout << "can not find start point {H:touchEventDispatch}" << std::endl;
        LOGE("can not find start point {H:touchEventDispatch}");
    } else if (fullScreenLine.compare("") == 0) {
        std::cout << "can not find complete point {H:RSMainThread::WindowScene}" << std::endl;
        LOGE("can not find complete point {H:RSMainThread::WindowScene}");
    } else {
        double startTime = std::stod(SmartPerf::StartFrameTraceNoh::GetLineTime(startLine));
        double endTime = std::stod(SmartPerf::StartFrameTraceNoh::GetLineTime(fullScreenLine));
        for (size_t i = 0; i + 1 < startTimeArr.size(); ++i) {
            double result = 0;
            if (startTimeArr[i].times >= startTime && endTimeArr[i].times > startTimeArr[i].times) {
                size_t next = i + 1;
                result = startTimeArr[next].times - endTimeArr[i].times;
            }
            double mark = 0.05;
            if (result > mark) {
                std::cout << "NO." << startTimeArr[i].frameId << "fps Time: " << result << "s" << std::endl;
            }
        }
        return frameNum / (endTime - startTime);
    }
    return -1.0;
}
std::string StartFrameTraceNoh::GetLineTime(std::string line)
{
    size_t num = 7;
    size_t position1 = line.find("....");
    size_t position2 = line.find(":");
    return line.substr(position1 + num, position2 - position1 - num);
}
std::string StartFrameTraceNoh::CutString(std::string line, const std::string &start,
    const std::string &end, size_t offset)
{
    int position1 = static_cast<int>(line.find(start));
    int position2 = static_cast<int>(line.find(end));
    return line.substr(position1 + offset, position2 - position1 - static_cast<int>(offset));
}
std::vector<std::string> StartFrameTraceNoh::Split(std::string str, const std::string &pattern)
{
    std::vector<std::string> result;
    if (str.empty() || pattern.empty() || str.size() < pattern.size()) {
        return result;
    }
    size_t start = 0; // start index
    while (start < str.size()) {
        size_t pos = str.find(pattern, start);
        if (pos != std::string::npos) {
            result.push_back(str.substr(start, pos - start));
            start = pos + pattern.size(); // next start index
        } else {
            //the last start
            result.push_back(str.substr(start));
            break;
        }
    }
    return result;
}
}
}