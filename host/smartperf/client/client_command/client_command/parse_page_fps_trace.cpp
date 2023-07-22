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
namespace OHOS {
    namespace SmartPerf {
        double PageFpsTrace::ParsePageFpsTrace(std::string file)
        {
            double fps = -1.0;
            infile.open(file);
            if (infile.fail()) {
                std::cout << "file open fail:" << file << std::endl;
                return fps;
            } else {
                fps = SmartPerf::PageFpsTrace::CalculateTime();
            }
            return fps;
        }
        double PageFpsTrace::CalculateTime()
        {
            std::string line;
            while (getline(infile, line)) {
                if (line.find("H:touchEventDispatch") != std::string::npos) {
                    startLine = line;
                    needUpdateResponseLine = true;
                    frameNum = 0;
                } else if (line.find("H:RSMainThread::DoComposition") != std::string::npos) {
                    updateCount = true;
                    count = 0;
                    pid = SmartPerf::PageFpsTrace::CutString(line, "B|", "|H:RSMainThread::DoComposition", 2);
                    frameNum++;
                    if(needUpdateResponseLine) {
                        responseLine = line;
                        needUpdateResponseLine = false;
                    }
                    frameStartTime = std::stod(SmartPerf::PageFpsTrace::getLineTime(line));
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
                        frameStartInterval = 0;
                        frameEndTime = std::stod(SmartPerf::PageFpsTrace::getLineTime(completeLine));
                        updateCount = false;
                    }
                }
                if (frameStartInterval != 0) {
                    double frameInterval = frameStartTime - frameStartInterval;
                    if (frameInterval > 0.05) {
                        std::cout<<"NO."<< frameNum <<"fps Time: "<< frameInterval << "s" <<std::endl;
                    }
                }
            }
            if (startLine.compare("") == 0) {
                std::cout << "can not find start point {H:touchEventDispatch}" << std::endl;
            } else if (completeLine.compare("") == 0) {
                std::cout << "can not find response and complete point {H:RSMainThread::DoComposition}" << std::endl;
            } else {
                double responseTime = std::stod(SmartPerf::PageFpsTrace::getLineTime(responseLine));
                double completeTime = std::stod(SmartPerf::PageFpsTrace::getLineTime(completeLine));
                return frameNum / (completeTime - responseTime);
            }
            return -1.0;
        }
        std::string PageFpsTrace::getLineTime(std::string line)
        {
            size_t num = 7;
            size_t position1 = line.find("....");
            size_t position2 = line.find(":");
            return line.substr(position1 + num, position2 - position1 -num);
        }
        std::string PageFpsTrace::CutString(std::string line, std::string start, std::string end, size_t offset)
        {
            size_t position1 = line.find(start);
            size_t position2 = line.find(end);
            return line.substr(position1 + offset, position2 - position1 -offset);
        }
    }
}