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
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include "include/profiler_fps.h"
#include "include/sp_log.h"
#include "include/sp_utils.h"


namespace OHOS {
namespace SmartPerf {

void ProfilerFPS::GetResultFPS(int sectionsNum)
{
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, nullptr);
    FpsInfoProfiler fpsInfoResult;
    unsigned long runTime;
    fpsInfoResult = GetFpsInfo();
    LOGI("result.fps====: %s", std::to_string(fpsInfoResult.fps).c_str());
    if (fpsInfoResult.fps == 0) {
        if (lastCurrTime == 0) {
            long long msStartTime = ((currRealTime / msClear) * msClear) - msClear;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, msStartTime / oneSec);
        } else {
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        }
    } else {
        long long two = 2;
        long long currTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + currTimeDiff;
        if ((lastCurrTime + two) == (currTime / oneSec)) {
            fpsInfoResult.fps = 0;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        } else {
            if (lastCurrTime < (currTime / oneSec)) {
                printf("fps:%d|%lld\n", fpsInfoResult.fps, currTime / oneSec);
                lastCurrTime = currTime / oneSec;
            } else {
                printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
                lastCurrTime = lastCurrTime + oneThousand;
            }
        }
    }
    time_t now = time(0);
    if (now == -1) {
        LOGI("Failed to get current time.");
        return;
    }
    char* dt = ctime(&now);
    LOGI("printf time is: %s", dt);
    if (sectionsNum == ten && fpsInfoResult.fps != 0) {
        GetSectionsFps(fpsInfoResult);
    }
    fflush(stdout);
    gettimeofday(&end, nullptr);
    runTime = end.tv_sec * 1e6 - start.tv_sec * 1e6 + end.tv_usec - start.tv_usec;
    if (runTime < oneSec) {
        usleep(oneSec - runTime);
    }
}

void ProfilerFPS::GetFPS(int argc, std::vector<std::string> v)
{
    int sectionsNum = 0;
    if (v[number] == "") {
        printf("the args of num must be not-null!\n");
    } else {
        num = atoi(v[number].c_str());
        if (num < 0) {
            printf("set num:%d not valid arg\n", num);
        }
        printf("set num:%d success\n", num);
        sectionsNum = atoi(v[four].c_str());
        for (int i = 0; i < num; i++) {
            GetResultFPS(sectionsNum);
        }
    }
    printf("SP_daemon exec finished!\n");
}

void ProfilerFPS::GetSectionsPrint(int printCount, long long msStartTime)
{
    long long msJiange = 100;
    if (printCount < ten) {
        for (int i = 0; i < ten - printCount; i++) {
            msStartTime += msJiange;
            printf("sectionsFps:%d|%lld\n", 0, msStartTime);
        }
    }
}

void ProfilerFPS::GetSectionsFps(FpsInfoProfiler &fpsInfo)
{
    int msCount = 0;
    long long msJiange = 100000000;
    long long msStartTime = (fpsInfo.currTimeStamps[0] / msClear) * msClear + msJiange;
    long long currTime = 0;
    long long currLastTime = lastCurrTime;
    long long harTime = 100;
    int printCount = 0;
    for (int i = 0; i < fpsInfo.currTimeStamps.size(); i++) {
        currTime = fpsInfo.currTimeStamps[i];
        if (currTime <= msStartTime) {
            msCount++;
        } else if (currTime > msStartTime && currTime <= (msStartTime + msJiange)) {
            printf("sectionsFps:%d|%lld\n", msCount * ten, currLastTime);
            msCount = 1;
            msStartTime += msJiange;
            currLastTime += harTime;
            printCount++;
        } else {
            printf("sectionsFps:%d|%lld\n", msCount * ten, currLastTime);
            printCount++;
            msCount = 0;
            msStartTime += msJiange;
            currLastTime += harTime;
            i--;
        }
        if (i == (fpsInfo.currTimeStamps.size() - 1)) {
            printf("sectionsFps:%d|%lld\n", msCount * ten, currLastTime);
            printCount++;
            GetSectionsPrint(printCount, currLastTime);
        }
    }
}

void ProfilerFPS::GetTimeDiff()
{
    long long clockRealTime = 0;
    long long clockMonotonicRaw = 0;
    int two = 2;
    std::string strRealTime;
    std::string cmd = "timestamps";
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
            clockRealTime = std::stoll(strRealTime);
            currRealTime = clockRealTime;
        } else if (params[0].find("CLOCK_MONOTONIC_RAW") != std::string::npos && clockMonotonicRaw == 0) {
            strRealTime = params[two];
            strRealTime.erase(strRealTime.find('.'), 1);
            clockMonotonicRaw = std::stoll(strRealTime);
        }
    }
    pclose(fd);
    currTimeDiff = clockRealTime - clockMonotonicRaw;
}

std::string ProfilerFPS::GetSurface()
{
    std::string cmdResult;
    std::string cmdString1 = "hidumper -s 10 -a sur";
    std::string cmdString2 = "face | grep sur";
    std::string cmdString3 = "face";
    SPUtils::LoadCmd(cmdString1 + cmdString2 + cmdString3, cmdResult);
    size_t position1 = cmdResult.find("[");
    size_t position2 = cmdResult.find("]");
    LOGI("cmdResult==: %s", (cmdResult.substr(position1 + 1, position2 - position1 - 1)).c_str());
    return cmdResult.substr(position1 + 1, position2 - position1 - 1);
}

std::string ProfilerFPS::CutLayerName(std::string layerName)
{
    std::string subLayerName;
    size_t twenty = 20;
    if (layerName.size() > twenty) {
        subLayerName = layerName.substr(0, twenty);
    } else {
        subLayerName = layerName;
    }
    return subLayerName;
}

FpsInfoProfiler ProfilerFPS::GetFpsInfoMax()
{
    int fpsValue = 0;
    if (fpsInfo.fps > uniteFpsInfo.fps) {
        fpsInfoMax = fpsInfo;
    } else {
        fpsInfoMax = uniteFpsInfo;
    }
    if (fpsInfoMax.fps < fpsValue) {
        fpsInfoMax.fps = fpsValue;
    }
    if (fpsInfoMax == prevFlagFpsInfo) {
        LOGI("fpsInfoMax == prevFlagFpsInfo");
        if (cntFpsInfo == lastFlagFpsInfo) {
            LOGI("cntFpsInfo == lastFlagFpsInfo");
            fpsInfoMax.fps = 0;
            fpsInfoMax.Clear();
            return fpsInfoMax;
        } else {
            LOGI("cntFpsInfo != lastFlagFpsInfo");
            if ((cntFpsInfo.currTimeStamps.size() > 0 && lastFlagFpsInfo.currTimeStamps.size() > 0) &&
                cntFpsInfo.currTimeStamps[0] == lastFlagFpsInfo.currTimeStamps[0]) {
                cntFpsInfo.fps = 0;
                cntFpsInfo.Clear();
            } else {
                lastFlagFpsInfo = cntFpsInfo;
            }
            return cntFpsInfo;
        }
    } else {
        LOGI("fpsInfoMax != prevFlagFpsInfo");
        if ((fpsInfoMax.currTimeStamps.size() > 0 && lastFlagFpsInfo.currTimeStamps.size() > 0) &&
            fpsInfoMax.currTimeStamps[0] == lastFlagFpsInfo.currTimeStamps[0]) {
            LOGI("fpsInfoMax == lastFlagFpsInfo");
            lastFlagFpsInfo = cntFpsInfo;
            prevFlagFpsInfo = fpsInfoMax;
            return cntFpsInfo;
        } else if ((fpsInfoMax.currTimeStamps.size() > 0 && prevFlagFpsInfo.currTimeStamps.size() > 0) &&
            fpsInfoMax.currTimeStamps[0] == prevFlagFpsInfo.currTimeStamps[0]) {
            prevFlagFpsInfo = fpsInfoMax;
            fpsInfoMax.fps = 0;
            fpsInfoMax.Clear();
            return fpsInfoMax;
        } else {
            LOGI("fpsInfoMax != lastFlagFpsInfo");
            prevFlagFpsInfo = fpsInfoMax;
            return fpsInfoMax;
        }
    }
}
FpsInfoProfiler ProfilerFPS::GetFpsInfo()
{
    fpsInfoMax.fps = 0;
    std::string tempLayerName;
    std::string uniteLayer = "DisplayNode";
    uniteLayer = GetSurface();
    tempLayerName = GetLayer();
    GetTimeDiff();
    uniteFpsInfo = GetSurfaceFrame(uniteLayer);
    fpsInfo = GetSurfaceFrame(tempLayerName);
    return GetFpsInfoMax();
}

FpsInfoProfiler ProfilerFPS::GetFpsInfoResult(FpsInfoProfiler &fpsInfo, long long &lastLineTime)
{
    const int maxZeroNum = 266;
    if (zeroNum >= maxZeroNum) {
        LOGI("zeroNum====: %s", std::to_string(zeroNum).c_str());
        while (!(fpsInfo.timeStampQ.empty())) {
            fpsInfo.timeStampQ.pop();
        }
        fpsInfo.fps = 0;
        fpsInfo.jitters.clear();
        LOGI("fpsInfo.fps0: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
    const int minPrintLine = 5;
    if (cnt < minPrintLine) {
        fpsInfo.fps = fpsInfo.preFps;
        LOGI("fpsInfo.fps1: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
    if (!fpsInfo.timeStampQ.empty() && fpsInfo.timeStampQ.back() == lastLineTime) {
        fpsInfo.fps = fpsGb;
        if (fpsGb == 0) {
            fpsInfo.jitters.clear();
        }
        LOGI("fpsInfo.fps2: %s", std::to_string(fpsInfo.fps).c_str());
        LOGI("lastLineTime: %s", std::to_string(lastLineTime).c_str());
        return fpsInfo;
    }
    if (fpsGb > 0) {
        fpsInfo.fps = fpsGb;
        fpsInfo.preFps = fpsGb;
        LOGI("fpsInfo.fps3: %s", std::to_string(fpsInfo.fps).c_str());
        LOGI("fpsInfo.preFps3: %s", std::to_string(fpsInfo.preFps).c_str());
        return fpsInfo;
    } else if (refresh && !jump) {
        fpsInfo.fps = fpsInfo.preFps;
        LOGI("fpsInfo.fps4: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    } else {
        fpsInfo.fps = 0;
        fpsInfo.jitters.clear();
        LOGI("fpsInfo.fps5: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
}

void ProfilerFPS::GetLastFpsInfo(FpsInfoProfiler &fpsInfo)
{
    int total = 266;
    if (cnt == total) {
        LOGI("cnt == total && fpsGb != 0");
        lastReadyTime = frameReadyTime;
        int fpsTmp = 0;
        cntFpsInfo.jitters.clear();
        cntFpsInfo.currTimeStamps.clear();
        while (!(fpsInfo.timeStampQ).empty()) {
            fpsTmp++;
            long long currFrame = (fpsInfo.timeStampQ.front());
            cntFpsInfo.currTimeStamps.push_back(currFrame);
            if (lastFrame != -1) {
                long long jitter = currFrame - lastFrame;
                cntFpsInfo.jitters.push_back(jitter);
            } else {
                long long jitter = currFrame - currFrame / mod * mod;
                cntFpsInfo.jitters.push_back(jitter);
            }
            lastFrame = currFrame;
            (fpsInfo.timeStampQ).pop();
        }
        cntFpsInfo.fps = fpsTmp;
        LOGI("cntFpsInfo.fps====: %s", std::to_string(cntFpsInfo.fps).c_str());
    }
}

void ProfilerFPS::GetPrevFpsInfo(FpsInfoProfiler &fpsInfo)
{
    refresh = true;
    long long tFrameReadyTime = frameReadyTime / mod;
    long long tLastReadyTime = lastReadyTime / mod;
    lastFrame = -1;
    if (tFrameReadyTime == tLastReadyTime) {
        (fpsInfo.timeStampQ).push(frameReadyTime);
    } else if (tFrameReadyTime >= tLastReadyTime + 1) {
        jump = true;
        lastReadyTime = frameReadyTime;
        int fpsTmp = 0;
        fpsInfo.jitters.clear();
        fpsInfo.currTimeStamps.clear();
        while (!(fpsInfo.timeStampQ).empty()) {
            fpsTmp++;
            long long currFrame = (fpsInfo.timeStampQ.front());
            fpsInfo.currTimeStamps.push_back(currFrame);
            if (lastFrame != -1) {
                long long jitter = currFrame - lastFrame;
                fpsInfo.jitters.push_back(jitter);
            } else {
                long long jitter = currFrame - currFrame / mod * mod;
                fpsInfo.jitters.push_back(jitter);
            }
            lastFrame = currFrame;
            (fpsInfo.timeStampQ).pop();
        }
        fpsGb = fpsTmp;
        LOGI("fpsGb====: %s", std::to_string(fpsGb).c_str());
        (fpsInfo.timeStampQ).push(frameReadyTime);
        fpsInfo.lastFrameReadyTime = lastFrame;
    }
}

void ProfilerFPS::InitParams(FpsInfoProfiler &fpsInfo, long long &lastLineTime)
{
    lastReadyTime = -1;
    fpsGb = 0;
    if (!(fpsInfo.timeStampQ).empty()) {
        lastReadyTime = (fpsInfo.timeStampQ).back();
        lastLineTime = (fpsInfo.timeStampQ).back();
    }
    jump = false;
    refresh = false;
    cnt = 0;
    zeroNum = 0;
}

FpsInfoProfiler ProfilerFPS::GetSurfaceFrame(std::string name)
{
    if (name == "") {
        return FpsInfoProfiler();
    }
    static std::map<std::string, FpsInfoProfiler> fpsMap;
    if (fpsMap.count(name) == 0) {
        FpsInfoProfiler tmp;
        tmp.fps = 0;
        tmp.preFps = 0;
        fpsMap[name] = tmp;
    }
    fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    FILE *fp;
    static char tmp[1024];
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
    LOGI("cmd=====: %s", cmd.c_str());
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return fpsInfo;
    }
    static long long lastLineTime;
    InitParams(fpsInfo, lastLineTime);
    LOGI("dump time: start!");
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        std::string str(tmp);
        LOGD("dump time: %s", str.c_str());
        frameReadyTime = 0;
        std::stringstream sstream;
        sstream << tmp;
        sstream >> frameReadyTime;
        cnt++;
        if (frameReadyTime == 0) {
            zeroNum++;
            continue;
        }
        if (lastReadyTime >= frameReadyTime) {
            lastReadyTime = -1;
            continue;
        }
        GetPrevFpsInfo(fpsInfo);
        GetLastFpsInfo(fpsInfo);
    }
    pclose(fp);
    return GetFpsInfoResult(fpsInfo, lastLineTime);
}

std::string ProfilerFPS::GetLayer()
{
    std::vector<DumpEntityProfiler> dumpEntityList;
    std::string curFocusId = "-1";
    const std::string cmd = "hidumper -s WindowManagerService -a -a";
    std::string focusWindowName = "NA";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return focusWindowName;
    }
    int lineNum = 0;
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line = buf;
        if (line[0] == '-' || line[0] == ' ') {
            continue;
        }
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params[windowNameIndex].find("WindowName") != std::string::npos &&
            params[windowIdIndex].find("WinId") != std::string::npos) {
            continue;
        }
        if (params.size() > paramFourteen) {
            DumpEntityProfiler dumpEntity { params[0], params[1], params[2], params[3], params[7]};
            dumpEntityList.push_back(dumpEntity);
        }
        if (params.size() == paramFourteen || params.size() == paramTwentyFour) {
            DumpEntityProfiler dumpEntity { params[0], params[2], params[2], params[3], params[6]};
            dumpEntityList.push_back(dumpEntity);
        }
        if (params.size() == paramThree) {
            curFocusId = params[focusNameIndex];
            break;
        }
        lineNum++;
    }
    pclose(fd);
    int curId = std::stoi(curFocusId);
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntityProfiler dumpItem = dumpEntityList[i];
        int curWinId = std::stoi(dumpItem.windId);
        if (curId == curWinId) {
            focusWindowName = dumpItem.windowName;
        }
    }
    return focusWindowName;
}
}
}