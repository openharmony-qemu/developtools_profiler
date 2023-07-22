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

namespace OHOS {
    namespace SmartPerf {
        double ParseTrace::ParseTraceNoh(const std::string &fileNamePath, const std::string &appPid)
        {
            int conversion = 1000;
            double code = -1;
            infile.open(fileNamePath);
            if (infile.fail()) {
                std::cout << "File " << "open fail" << std::endl;
                infile.close();
                return code;
            } else {
                code = SmartPerf::ParseTrace::ParseNohTrace(fileNamePath, appPid);
            }
            infile.close();
            return code * conversion;
        }
        double ParseTrace::ParseNohTrace(const std::string &fileNamePath, const std::string &appPid)
        {
            std::string line;
            std::string::size_type positionPid;
            double codeTime = -1;
            while (getline(infile, line)) {
                startTime = SmartPerf::ParseTrace::GetStartTime(line, startTime);
                frameId = SmartPerf::ParseTrace::GetFrameId(line, appPid, frameId);
				
                positionPid = line.find("[" + appPid + "," + frameId + "]");
                if (positionPid != std::string::npos) {
                    size_t position1 = line.find("....");
                    size_t position2 = line.find(":");
                    size_t flagTimeSize = 5;
                    flagTime = line.substr(position1 + flagTimeSize, position2 - position1 - flagTimeSize);
                    if (std::stof(endTime) == 0) {
                        endTime = flagTime;
                    } else {
                        float dataThreshold = 0.3;
                        if ((std::stof(flagTime) - std::stof(endTime)) > dataThreshold) {
                            break;
                        } else {
                            endTime = flagTime;
                        }
                    }
                }
            }
            codeTime = SmartPerf::ParseTrace::GetTime(startTime, endTime);
            return codeTime;
        }
        std::string ParseTrace::GetFrameId(std::string line, std::string appPid, std::string fid)
        {
            std::string::size_type positionTransactionFlag = line.find("transactionFlag:[" + appPid);
            if (positionTransactionFlag != std::string::npos) {
                size_t positionFrame1 = line.rfind("[" + appPid + ",");
                size_t positionFrame2 = line.rfind("]");
                size_t subNum = 2;
                frameId = line.substr(positionFrame1 + subNum + appPid.length(), positionFrame2 - positionFrame1 - subNum - appPid.length());
            } else {
                frameId = fid;
            }
            return frameId;
        }
        std::string  ParseTrace::GetWindowTime(std::string line, std::string wt)
        {
            size_t positionWindow = line.find("H:RSUniRender::Process:[leashWindow");
            if (positionWindow != std::string::npos) {
                size_t positionWindow1 = line.rfind(")");
                size_t subNumSize = 4;
                std::string windowSizeFlag = line.substr(positionWindow1 - subNumSize, subNumSize);
                std::string windowSize = "0";
                if (std::stof(windowSize) == 0) {
                    const int windowSizeNum = 1344;
                    if (std::stof(windowSizeFlag) == windowSizeNum) {
                        windowSize = "0";
                    } else {
                        windowSize = windowSizeFlag;
                    }
                } else {
                    if (std::stof(windowSize) != std::stof(windowSizeFlag)) {
                        size_t subNum = 5;
                        size_t position1 = line.find("....");
                        size_t position2 = line.find(":");
                        windowTime = line.substr(position1 + subNum, position2 - position1 - subNum);
                        windowSize = windowSizeFlag;
                    } else {
                        windowTime = wt;
                    }
                }
            }
            return windowTime;
        }
        double ParseTrace::ParseTraceCold(const std::string &fileNamePath, const std::string &appPid)
        {
            int conversion = 1000;
            double code = -1;
            infile.open(fileNamePath);
            if (infile.fail()) {
                std::cout << "File " << "open fail" << std::endl;
                return 0;
            } else {
                code = SmartPerf::ParseTrace::ParseCodeTrace(fileNamePath, appPid);
            }
            infile.close();
            return code * conversion;
        }
        double ParseTrace::ParseTraceHot(const std::string &fileNamePath)
        {
            int conversion = 1000;
            double code = -1;
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
        double ParseTrace::ParseCodeTrace(const std::string &fileNamePath, const std::string &appPid)
        {
            std::string line;
            std::string::size_type tracingMarkWrite;
            std::string::size_type fourPoint;
            double codeTime = -1;
            while (getline(infile, line)) {
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
                    double interval = 0.3;
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
        double ParseTrace::ParseHotTrace(const std::string &fileNamePath)
        {
            std::string line;
            std::string::size_type doComposition;
            double codeTime = -1;
            while (getline(infile, line)) {
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
                    double interval = 0.3;
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
        double ParseTrace::GetTime(std::string start, std::string end)
        {
            double codeTime = -1;
            if (std::stof(end) == 0 || std::stof(start) == 0) {
                return codeTime;
            } else {
                double displayTime = 0.040;
                codeTime = std::stof(end) - std::stof(start) + displayTime;
            }
            return codeTime;
        }
        std::string  ParseTrace::GetStartTime(std::string line, const std::string &startTimeBefore)
        {
            std::string::size_type te = line.find("H:touchEventDispatch");
            std::string::size_type td = line.find("H:TouchEventDispatch");
            std::string::size_type pd = line.find("H:PointerEventDispatch");
            std::string::size_type kd = line.find("H:KeyEventDispatch");
            std::string::size_type nop = std::string::npos;
            if (te != nop || td != nop || pd != nop || kd != nop) {
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