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
#include "include/parse_trace.h"
#include "include/sp_utils.h"
namespace OHOS {
    namespace SmartPerf {
        float ParseTrace::ParseTraceCold(const std::string &fileNamePath)
        {
            int conversion = 1000;
            float code = -1;
            infile.open(fileNamePath);
            if (infile.fail()) {
                std::cout << "File " << "open fail" << std::endl;
                return 0;
            } else {
                code = SmartPerf::ParseTrace::ParseCodeTrace(fileNamePath);
            }
            infile.close();
            return code * conversion;
        }
        float ParseTrace::ParseTraceHot(const std::string &fileNamePath)
        {
            int conversion = 1000;
            float code = -1;
            infile.open(fileNamePath);
            if (infile.fail()) {
                std::cout << "File " << "open fail" << std::endl;
                return 0;
            } else {
                code = SmartPerf::ParseTrace::ParseHotTrace(fileNamePath);
            }
            infile.close();
            return code * conversion;
        }
        float ParseTrace::ParseCodeTrace(const std::string &fileNamePath)
        {
            std::string line;
            std::string::size_type tracingMarkWrite;
            std::string::size_type fourPoint;
            float codeTime = -1;
            while (getline(infile, line)) {
                appPid = SmartPerf::ParseTrace::GetPid(line, "pid", appPid);
                startTime = SmartPerf::ParseTrace::GetStartTime(line, startTime);
                tracingMarkWrite = line.find("tracing_mark_write: B|"+ appPid + "|H:RSRenderThread DrawFrame:");
                fourPoint = line.find("....");
                if (tracingMarkWrite != std::string::npos && fourPoint != std::string::npos) {
                    size_t p1 = line.find("....");
                    size_t p2 = line.find(":");
                    size_t subNum = 5;
                    endTime = line.substr(p1 + subNum, p2 - p1 - subNum);
                    int endNum = std::stof(endTime);
                    int endFlagNum = std::stof(endTimeFlag);
                    int startNum = std::stof(startTime);
                    int timeNum = endNum - endFlagNum;
                    float interval = 0.3;
                    if (timeNum < interval) {
                            endTimeFlag = endTime;
                    } else {
                        if (std::stof(endTimeFlag) == 0) {
                            endTimeFlag = endTime;
                        } else if (endFlagNum != 0 && startNum != 0 && timeNum > interval) {
                            break;
                        } else {
                            endTimeFlag = endTime;
                        }
                    }
                }
            }
            codeTime = SmartPerf::ParseTrace::GetTime(startTime, endTime);
            return codeTime;
        }
        float ParseTrace::ParseHotTrace(const std::string &fileNamePath)
        {
            std::string line;
            std::string::size_type doComposition;
            float codeTime = -1;
            while (getline(infile, line)) {
                appPid=SmartPerf::ParseTrace::GetPid(line, "pid", appPid);
                startTime=SmartPerf::ParseTrace::GetStartTime(line, startTime);
                doComposition = line.find("H:RSMainThread::DoComposition");
                if (doComposition != std::string::npos) {
                    size_t position1 = line.find("....");
                    size_t position2 = line.find(":");
                    int subNum = 5;
                    endTime = line.substr(position1 + subNum, position2 - position1 - subNum);
                    int endNum = std::stof(endTime);
                    int endFlagNum = std::stof(endTimeFlag);
                    int startNum = std::stof(startTime);
                    int timeNum = endNum - endFlagNum;
                    float interval = 0.3;
                    if (timeNum < interval) {
                            endTimeFlag = endTime;
                    } else {
                        if (std::stof(endTimeFlag) == 0) {
                            endTimeFlag = endTime;
                        } else if (endFlagNum != 0 && startNum != 0 && timeNum > interval) {
                            break;
                        } else {
                            endTimeFlag = endTime;
                        }
                    }
                }
            }
            codeTime = SmartPerf::ParseTrace::GetTime(startTime, endTime);
            return codeTime;
        }
        float ParseTrace::GetTime(std::string start, std::string end)
        {
            size_t point = end.find(".");
            float codeTime = -1;
            size_t subNum = 2;
            end = end.substr(point - subNum);
            start = start.substr(point - subNum);
            if (std::stof(end) == 0 || std::stof(start) == 0) {
            } else {
                float displayTime = 0.040;
                codeTime = std::stof(end) - std::stof(start) + displayTime;
            }
            return codeTime;
        }
        std::string  ParseTrace::GetPid(std::string line, const std::string &strPackgeName, const std::string &appPidBefore)
        {
            std::string::size_type positionPackgeName;
            std::string::size_type positionAppspawn;
            if (appPidnum == 0) {
                size_t packageNameNumSize = 5;
                if (strPackgeName.length() < packageNameNumSize) {
                    positionPackgeName = line.find("task_newtask: pid=");
                    positionAppspawn = line.find("comm=appspawn");
                    if (positionPackgeName != std::string::npos && positionAppspawn != std::string::npos) {
                        size_t position1 = line.find("pid=");
                        size_t position2 = line.find(" comm=appspawn");
                        size_t subNum = 4;
                        appPid = line.substr(position1 + subNum, position2 - position1 - subNum);
                        appPidnum++;
                    } else {
                        appPid = appPidBefore;
                    }
                } else {
                    positionPackgeName = line.find(strPackgeName);
                    if (positionPackgeName != std::string::npos) {
                        size_t p1 = line.find(strPackgeName);
                        size_t p2 = line.find(" prio");
                        appPid = line.substr(p1 + strPackgeName.length(), p2 - p1 - strPackgeName.length());
                        appPidnum++;
                    } else {
                        appPid = appPidBefore;
                    }
                }
            } else {
                appPid = appPidBefore;
            }
            return appPid;
        }
        std::string  ParseTrace::GetStartTime(std::string line, const std::string &startTimeBefore)
        {
            std::string::size_type touchEventDisPos = line.find("H:touchEventDispatch");
            std::string::size_type mTouchEventDisPos = line.find("H:TouchEventDispatch");
            if (mTouchEventDisPos != std::string::npos || touchEventDisPos != std::string::npos) {
                size_t touchNum = 3;
                if (flagTouch <= touchNum) {
                size_t position1 = line.find("....");
                size_t position2 = line.find(":");
                size_t subNum = 5;
                startTime = line.substr(position1 + subNum, position2 - position1 - subNum);
                flagTime = "0";
                flagTouch++;
                } else {
                startTime = startTimeBefore;
                }
            } else {
                startTime = startTimeBefore;
            }
            return startTime;
        }
    }
}