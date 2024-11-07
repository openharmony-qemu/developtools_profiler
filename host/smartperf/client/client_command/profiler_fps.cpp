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
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "include/profiler_fps.h"
#include "include/sp_log.h"
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/startup_delay.h"
#include "include/common.h"

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> ProfilerFPS::ItemData()
{
    std::map<std::string, std::string> result;
    FpsInfoProfiler finalResult = GetFpsInfo();
    lastFpsInfoResult = finalResult;
    if (processFlag) {
        result["fps"] = "NA";
        result["fpsJitters"] = "NA";
    } else {
        const int fullFrame = 120;
        const int maxFullFrame = 123;
        if (finalResult.fps > fullFrame && finalResult.fps < maxFullFrame) {
            finalResult.fps = fullFrame;
        }
        result["fps"] = std::to_string(finalResult.fps);
        LOGI("ProfilerFPS.result.fps: %s", std::to_string(finalResult.fps).c_str());
        LOGI("ProfilerFPS.result.curTime: %s", std::to_string(finalResult.curTime).c_str());
        std::string jitterStr = "";
        std::string split = "";
        for (size_t i = 0; i < finalResult.jitters.size(); i++) {
            if (i > 0) {
                split = ";;";
            }
            jitterStr += split + std::to_string(finalResult.jitters[i]);
        }
        result["fpsJitters"] = jitterStr;
        LOGI("ProfilerFPS.result.jitters: %s", jitterStr.c_str());
        if (isCatchTrace > 0) {
            ByTrace::GetInstance().CheckFpsJitters(finalResult.jitters, finalResult.fps);
        }
    }
    return result;
}

void ProfilerFPS::SetTraceCatch()
{
    isCatchTrace = 1;
}

void ProfilerFPS::SetPackageName(std::string pName)
{
    pkgName = std::move(pName);
}

void ProfilerFPS::SetProcessId(const std::string &pid)
{
    processId = pid;
}

void ProfilerFPS::GetResultFPS(int sectionsNum)
{
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, nullptr);
    FpsInfoProfiler fpsInfoResult;
    unsigned long runTime;
    fpsInfoResult = GetFpsInfo();
    if (fpsInfoResult.fps == 0) {
        if (lastCurrTime == 0) {
            long long currTime = (fpsInfoResult.currTimeDump / msClear) * msClear + fpsInfoResult.currTimeDiff;
            lastCurrTime = currTime / oneSec;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, currTime / oneSec);
        } else {
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        }
    } else {
        long long currTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + fpsInfoResult.currTimeDiff;
        lastCurrTime = currTime / oneSec;
        printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime);
    }
    lastFpsInfoResult = fpsInfoResult;
    if (sectionsNum != 0 && fpsInfoResult.fps != 0) {
        GetSectionsFps(fpsInfoResult, sectionsNum);
    }
    time_t now = time(nullptr);
    if (now == -1) {
        LOGI("Failed to get current time.");
        return;
    }
    char *dt = ctime(&now);
    LOGI("printf time is: %s", dt);
    fflush(stdout);
    gettimeofday(&end, nullptr);
    runTime = end.tv_sec * 1e6 - start.tv_sec * 1e6 + end.tv_usec - start.tv_usec;
    LOGI("printf time is runTime: %s", std::to_string(runTime).c_str());
    if (runTime < sleepTime) {
        usleep(sleepTime - runTime);
    }
    OHOS::SmartPerf::SPUtils::GetCurrentTime(ten, lastFpsInfoResult.curTime);
}

void ProfilerFPS::GetTimeDiff()
{
    long long clockRealTime = 0;
    long long clockMonotonicRaw = 0;
    int two = 2;
    std::string strRealTime;
    const std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::TIMESTAMPS);
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return;
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params[0].find("CLOCK_REALTIME") != std::string::npos && clockRealTime == 0) {
            strRealTime = params[two];
            strRealTime.erase(strRealTime.find('.'), 1);
            clockRealTime = SPUtilesTye::StringToSometype<long long>(strRealTime);
            currRealTime = clockRealTime;
        } else if (params[0].find("CLOCK_MONOTONIC_RAW") != std::string::npos && clockMonotonicRaw == 0) {
            strRealTime = params[two];
            strRealTime.erase(strRealTime.find('.'), 1);
            clockMonotonicRaw = SPUtilesTye::StringToSometype<long long>(strRealTime);
        }
    }
    if (pclose(fd) == -1) {
        LOGE("Error: Failed to close file");
        return;
    }
    fpsInfo.currTimeDiff = clockRealTime - clockMonotonicRaw;
}

void ProfilerFPS::GetSectionsPrint(int printCount, long long msStartTime, int numb, long long harTime) const
{
    if (printCount < numb) {
        for (int i = 0; i < numb - printCount; i++) {
            msStartTime += harTime;
            printf("sectionsFps:%d|%lld\n", 0, msStartTime);
        }
    }
}

void ProfilerFPS::PrintSections(int msCount, long long currTimeLast,
                                long long currTimeStart, long long currLastTime) const
{
    int conversionFps = 1000000;
    int conversionTime = 1000;
    long long times = 120;
    int fpsNums = 0;
    if (msCount == 0) {
        fpsNums = 0;
    } else {
        fpsNums = msCount - 1;
    }
    double timeN = (currTimeLast - currTimeStart) * 1.0 / conversionTime;
    if (timeN == 0) {
        printf("sectionsFps:%d|%lld\n", 0, currLastTime);
        return;
    }
    double fpsSections = (fpsNums * conversionFps) / timeN;
    int fpsSectionsInt = round(fpsSections);
    if (fpsSectionsInt > static_cast<int>(times)) {
        fpsSectionsInt = static_cast<int>(times);
    }
    printf("sectionsFps:%d|%lld\n", fpsSectionsInt, currLastTime);
}

void ProfilerFPS::GetSectionsFps(FpsInfoProfiler &fpsInfoResult, int nums) const
{
    int msCount = 0;
    long long msJiange = 0;
    if (nums != 0) {
        msJiange = msClear / nums;
    }
    long long msStartTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + msJiange;
    long long currLastTime = lastCurrTime;
    long long harTime = msJiange / 1000000;
    int printCount = 0;
    long long currTimeStart = 0;
    long long currTimeLast = 0;
    for (size_t i = 0; i < fpsInfoResult.currTimeStamps.size(); i++) {
        long long currTime = fpsInfoResult.currTimeStamps[i];
        if (currTime <= msStartTime) {
            if (msCount == 0) {
                currTimeStart = currTime;
            }
            currTimeLast = currTime;
            msCount++;
        } else {
            while (currTime > msStartTime) {
                PrintSections(msCount, currTimeLast, currTimeStart, currLastTime);
                printCount++;
                msCount = 1;
                msStartTime += msJiange;
                currLastTime += harTime;
                currTimeLast = currTime;
                currTimeStart = currTime;
            }
        }
        if (i == (static_cast<size_t>(fpsInfoResult.currTimeStamps.size()) - 1)) {
            PrintSections(msCount, currTimeLast, currTimeStart, currLastTime);
            currTimeLast = currTime;
            printCount++;
            GetSectionsPrint(printCount, currLastTime, nums, harTime);
        }
    }
}

void ProfilerFPS::GetFPS(std::vector<std::string> v)
{
    if (v[number] == "") {
        printf("the args of num must be not-null!\n");
    } else {
        this->num = SPUtilesTye::StringToSometype<int>(v[number].c_str());
        if (this->num < 0) {
            printf("set num:%d not valid arg\n", this->num);
        }
        printf("set num:%d success\n", this->num);
        int sectionsNum = (static_cast<int>(v.size()) >= four) ?
                            SPUtilesTye::StringToSometype<int>(v[four].c_str()) : 0;
        if (sectionsNum > ten) {
            printf("set sectionsNum:%d not valid arg \n", sectionsNum);
        } else {
            for (int i = 0; i < this->num; i++) {
                GetResultFPS(sectionsNum);
            }
        }
    }
    printf("SP_daemon exec finished!\n");
}

std::string ProfilerFPS::GetSurface()
{
    std::string cmdResult;
    std::string dumperSurface = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SURFACE);
    SPUtils::LoadCmd(dumperSurface, cmdResult);
    size_t positionLeft = cmdResult.find("[");
    size_t positionRight = cmdResult.find("]");
    size_t positionNum = 1;
    return cmdResult.substr(positionLeft + positionNum, positionRight - positionLeft - positionNum);
}

FpsInfoProfiler ProfilerFPS::GetFpsInfo()
{
    processFlag = false;
    fpsInfoMax.fps = 0;
    std::string uniteLayer;
    bool isGameSurface = false;
    std::string gameLayerName;
    if (isGameLayer == "1") {
        gameLayerName = GetGameLayer();
    }
    if (!gameLayerName.empty()) {
        uniteLayer = gameLayerName;
        isGameSurface = true;
    } else if (!rkFlag) {
        LOGI("uniteLayer is UniRender");
        uniteLayer = "UniRender";
    } else {
        ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
        uniteLayer = profilerFps.GetSurface();
    }
    if (ohFlag) {
        uniteLayer = GetSurface();
    }
    if (pkgName.empty() || pkgName.find("sceneboard") != std::string::npos || isGameSurface) {
        LOGI("ProfilerFPS.pkgName: %s", pkgName.c_str());
        OHOS::SmartPerf::SPUtils::GetCurrentTime(fifty, lastFpsInfoResult.curTime);
        fpsInfoMax = GetSurfaceFrame(uniteLayer);
    } else {
        bool onTop = OHOS::SmartPerf::SPUtils::IsForeGround(pkgName);
        if (onTop) {
            OHOS::SmartPerf::SPUtils::GetCurrentTime(fifty, lastFpsInfoResult.curTime);
            fpsInfoMax = GetSurfaceFrame(uniteLayer);
        } else {
            LOGI("ProfilerFPS::app is in the background");
            if (processId.empty()) {
                processFlag = true;
                fpsInfoMax.Clear();
            } else {
                fpsInfoMax.Clear();
            }
        }
    }
    return fpsInfoMax;
}

FpsInfoProfiler ProfilerFPS::GetSurfaceFrame(std::string name)
{
    if (name == "") {
        return FpsInfoProfiler();
    }
    return GetFrameInfoFromMap(name);
}

FpsInfoProfiler ProfilerFPS::GetFrameInfoFromMap(std::string name)
{
    static std::map<std::string, FpsInfoProfiler> fpsMap;
    if (fpsMap.count(name) == 0) {
        FpsInfoProfiler tmp;
        tmp.fps = 0;
        fpsMap[name] = tmp;
    }
    fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    FILE *fp;
    static char tmp[1024];
    GetTimeDiff();
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
    if (cmd.empty()) {
        LOGE("cmd is null");
        return fpsInfo;
    }
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        LOGE("Failed to open hidumper file");
        return fpsInfo;
    }
    fpsNum = 0;
    prevScreenTimestamp = -1;
    LOGI("ProfilerFPS dump time: start!");
    struct timespec time1 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    fpsInfo.curTime = static_cast<int>(time1.tv_sec - 1);
    fpsInfo.currTimeDump = (time1.tv_sec - 1) * mod + time1.tv_nsec;
    LOGI("ProfilerFPS fpsInfo.curTime: %d", fpsInfo.curTime);
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        std::string str(tmp);
        LOGD("ProfilerFPS dump time: %s", str.c_str());
        curScreenTimestamp = 0;
        std::stringstream sstream;
        sstream << tmp;
        sstream >> curScreenTimestamp;
        if (curScreenTimestamp == 0) {
            continue;
        }
        CalcFpsAndJitters();
    }
    if (pclose(fp) == -1) {
        LOGE("Error: Failed to close file");
        return fpsInfo;
    }
    LOGI("ProfilerFPS fpsNum: %d", fpsNum);
    return fpsInfo;
}

void ProfilerFPS::CalcFpsAndJitters()
{
    std::string onScreenTime = std::to_string(curScreenTimestamp / mod);
    std::string fpsCurTime = std::to_string(fpsInfo.curTime);
    if (onScreenTime.find(fpsCurTime) != std::string::npos) {
        fpsNum++;
        fpsInfo.currTimeStamps.push_back(curScreenTimestamp);
    }
    fpsInfo.fps = fpsNum;
    if (onScreenTime == fpsCurTime) {
        long long jitter;
        if (prevScreenTimestamp != -1) {
            jitter = curScreenTimestamp - prevScreenTimestamp;
            fpsInfo.jitters.push_back(jitter);
        } else {
            if (prevlastScreenTimestamp != 0 && (curScreenTimestamp - prevlastScreenTimestamp) < mod) {
                jitter = curScreenTimestamp - prevlastScreenTimestamp;
                fpsInfo.jitters.push_back(jitter);
            } else {
                jitter = curScreenTimestamp - curScreenTimestamp / mod * mod;
                fpsInfo.jitters.push_back(jitter);
            }
        }
        prevScreenTimestamp = curScreenTimestamp;
        prevlastScreenTimestamp = curScreenTimestamp;
    }
}

void ProfilerFPS::GetOhFps(std::vector<std::string> v)
{
    if (v[number] == "") {
        printf("the args of num must be not-null!\n");
    } else {
        this->num = SPUtilesTye::StringToSometype<int>(v[number].c_str());
        if (this->num < 0) {
            printf("set num:%d not vaild arg\n", this->num);
        }
        printf("set num:%d success\n", this->num);
        ohFlag = true;
        int sectionsNum;
        if (static_cast<int>(v.size()) < four) {
            sectionsNum = 0;
        } else {
            sectionsNum = SPUtilesTye::StringToSometype<int>(v[four].c_str());
        }
        for (int i = 0; i < this->num; i++) {
            GetResultFPS(sectionsNum);
        }
    }
    printf("SP_daemon exec finished!\n");
}

void ProfilerFPS::SetGameLayer(std::string isGameView)
{
    isGameLayer = std::move(isGameView);
}

FpsInfoProfiler ProfilerFPS::selfRerderLayers(const std::string &gameLayer)
{
    OHOS::SmartPerf::SPUtils::GetCurrentTime(fifty, lastFpsInfoResult.curTime);
    fpsInfoMax = GetSurfaceFrame(gameLayer);
    return fpsInfoMax;
}

std::string ProfilerFPS::GetGameLayer()
{
    std::string gameLayer = "";
    if (processId.empty()) {
        return gameLayer;
    }
    std::string cmdResult;
    const std::string dumperSurface = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SURFACE);
    char buf[1024] = {'\0'};
    std::string start = "NodeId[";
    std::string end = "] LayerId";
    std::string nodeIdStr;
    uint64_t nodeId;
    if (dumperSurface.empty()) {
        LOGI("ProfilerFPS::DUMPER_SURFACE failed");
        return gameLayer;
    }
    FILE *fd = popen(dumperSurface.c_str(), "r");
    if (fd == nullptr) {
        return gameLayer;
    }
    while (fgets(buf, sizeof(buf), fd) != nullptr) {
        std::string line = buf;
        size_t startPos = line.find(start);
        size_t endPos = line.find(end);
        if (startPos != std::string::npos && endPos != std::string::npos) {
            nodeIdStr = line.substr(startPos + start.length(), endPos - startPos - start.length());
            LOGI("ProfilerFPS::nodeIdStr: (%s)", nodeIdStr.c_str());
        }
        const int kShiftAmount = 32;
        if (!nodeIdStr.empty()) {
            std::stringstream ss(nodeIdStr);
            ss >> nodeId;
            if (ss.fail() || !ss.eof()) {
                return gameLayer;
            }
            nodeId = nodeId >> kShiftAmount;
            LOGI("ProfilerFPS::nodeId: (%d)", nodeId);
            GetLayerName(gameLayer, nodeId, line, endPos);
        }
    }
    if (pclose(fd) == -1) {
        LOGE("Error: Failed to close file");
        return gameLayer;
    }
    LOGI("ProfilerFPS::gameLayer: (%s)", gameLayer.c_str());
    return gameLayer;
}

std::string ProfilerFPS::GetLayerName(std::string &gameLayer, uint64_t &nodeId, std::string &line, size_t &endPos)
{
    if (std::to_string(nodeId) == processId) {
        size_t layerStartPos = line.find("[");
        size_t layerEndPos = line.find("]");
        if (layerEndPos - layerStartPos <= 1 && layerEndPos > endPos) {
            return gameLayer;
        }
        layerStartPos += 1;
        gameLayer = line.substr(layerStartPos, layerEndPos - layerStartPos);
    }
    return gameLayer;
}

void ProfilerFPS::SetRkFlag()
{
    rkFlag = true;
}
}
}