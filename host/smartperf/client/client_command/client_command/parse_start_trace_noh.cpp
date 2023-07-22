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
#include "include/parse_start_trace_noh.h"

namespace OHOS {
    namespace SmartPerf {
        double StartTraceNoh::ParseStartTraceNohe(std::string file)
        {
            double codeTime = -1.0;
            infile.open(file);
            if (infile.fail()) {
                std::cout << "file open fail:" << file << std::endl;
                return codeTime;
            } else {
                codeTime = SmartPerf::StartTraceNoh::CalculateTime();
            }
            return codeTime;
        }
        double StartTraceNoh::CalculateTime()
        {
            std::string startLine = "";
            std::string endLine = "";
            bool findTouch = false;
            std::string line;
            while (getline(infile, line)) {
                if (line.find("H:touchEventDispatch") != std::string::npos) {
                    startLine = line;
                    findTouch = true;
                } else if (findTouch && line.find("H:RSUniRender::Process:[leashWindow") != std::string::npos) {
                    std::string tmpStr = Split(line, "H:RSUniRender::Process:[leashWindow")[1];
                    std::string numsStr = CutString(tmpStr, "(", ")", 1);
                    std::vector<std::string> nums = Split(numsStr, ",");
                    std::string alpha = Split(tmpStr, "Alpha:")[1];
                    bool full = stof(nums[0]) == 0 && stof(nums[1]) == 0 && stof(nums[2]) == 1344 && stof(nums[3]) == 2772  ? true : false;
                    if (full && stof(alpha) == 1) {
                        endLine = line;
                        break;
                    }
                }
            }
            if (startLine.compare("") == 0) {
                std::cout << "can not find start point {H:touchEventDispatch}" << std::endl;
            } else if (endLine.compare("") == 0) {
                std::cout << "can not find complete point {H:RSMainThread::leashWindow}" << std::endl;
            } else {
                double startTime = std::stod(SmartPerf::StartTraceNoh::GetLineTime(startLine));
                double endTime = std::stod(SmartPerf::StartTraceNoh::GetLineTime(endLine));
                return endTime - startTime;
            }
            return -1.0;
        }
        std::string StartTraceNoh::GetLineTime(std::string line)
        {
            size_t num = 7;
            size_t position1 = line.find("....");
            size_t position2 = line.find(":");
            return line.substr(position1 + num, position2 - position1 - num);
        }
        std::string StartTraceNoh::CutString(std::string line, std::string start, std::string end, size_t offset)
        {
            size_t position1 = line.find(start);
            size_t position2 = line.find(end);
            return line.substr(position1 + offset, position2 - position1 - offset);
        }
        std::vector<std::string> StartTraceNoh::Split(std::string str, std::string pattern)
        {
            std::string::size_type pos;
            std::vector<std::string> result;
            str += pattern;
            size_t size = str.size();
            for (size_t i = 0; i < size; i++) {
                pos = str.find(pattern, i);
                if (pos < size) {
                    std::string s = str.substr(i, pos - i);
                    result.push_back(s);
                    i = pos + pattern.size() - 1;
                }
            }
            return result;
        }
    }
}