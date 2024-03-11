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
            long long currTime = (fpsInfoResult.curTimeDump / msClear) * msClear + fpsInfoResult.curTimeDiff;
            lastCurrTime = currTime / oneSec;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, currTime / oneSec);
        } else {
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        }
    } else {
        if (lastCurrTime == 0) {
            long long currTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + fpsInfoResult.curTimeDiff;
            lastCurrTime = currTime / oneSec;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime);
        } else {
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        }
    }
    if (sectionsNum == ten && fpsInfoResult.fps != 0) {
        GetSectionsFps(fpsInfoResult);
    }
    time_t now = time(nullptr);
    if (now == -1) {
        LOGI("Failed to get current time.");
        return;
    }
    char* dt = ctime(&now);
    LOGI("printf time is: %s", dt);

    fflush(stdout);
    gettimeofday(&end, nullptr);
    runTime = end.tv_sec * 1e6 - start.tv_sec * 1e6 + end.tv_usec - start.tv_usec;
    LOGI("printf time is---runTime: %s", std::to_string(runTime).c_str());
    if (runTime < sleepTime) {
        usleep(sleepTime - runTime);
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
    if (fd != nullptr) {
        return false;
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
    fpsTnfo.currTimeDiff = clockRealTime - clockMonotonicRaw;
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

void ProfilerFPS::GetSectionsFps()
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
    if (uniteFpsInfo.curTime == (prevResultFpsInfo.curTime + 1)) {
        LOGI("uniteFpsInfo.curTime == prevResultFpsInfo.curTime + 1");
        prevResultFpsInfo = uniteFpsInfo;
        prevSecFpsInfoMax = secUniteFpsInfo;
        prevFpsInfoMax = uniteFpsInfo;
        return prevResultFpsInfo;
    } else if (uniteFpsInfo.curTime == prevResultFpsInfo.curTime) {
        LOGI("uniteFpsInfo.curTime == prevResultFpsInfo.curTime");
        prevResultFpsInfo = secUniteFpsInfo;
        prevSecFpsInfoMax = secUniteFpsInfo;
        prevFpsInfoMax = uniteFpsInfo;
        return prevResultFpsInfo;
    } else if (uniteFpsInfo.curTime > (prevResultFpsInfo.curTime + 1) && isFirstResult) {
        LOGI("uniteFpsInfo.curTime > prevResultFpsInfo.curTime");
        prevResultFpsInfo = prevSecFpsInfoMax;
        prevSecFpsInfoMax = secUniteFpsInfo;
        prevFpsInfoMax = uniteFpsInfo;
        return prevResultFpsInfo;
    } else {
        LOGI("8888888888888888888888888"); 
        prevResultFpsInfo = uniteFpsInfo;
        prevSecFpsInfoMax = secUniteFpsInfo;
        prevFpsInfoMax = uniteFpsInfo;
        isFirstResult = true;
        return prevResultFpsInfo;
    }
}
FpsInfoProfiler ProfilerFPS::GetFpsInfo()
{
    isFirstResult = false;
    int fourHundred = 400;
    fpsInfoMax.fps = 0;
    std::string uniteLayer = "UniRender";
    uniteFpsInfo = GetSurfaceFrame(uniteLayer);
    std::this_thread::sleep_for(std::chrono::milliseconds(threeHundred));
    secUniteFpsInfo = GetSurfaceFrame(uniteLayer); 
    return GetFpsInfoMax();
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
    fpsNum = 0;
    refresh = true;
    lastTime = -1;
    LOGI("dump time: start!");
    struct timespec time1 = {0};
    clock_gittime(CLOCK_MONOTONIC, &time1);
    LOGI("Time-------time1.tv_sec: %s", std::to_string(time1.tv_sec).c_str());
    LOGI("Time-------time1.tv_nsec: %s", std::to_string(time1.tv_nsec).c_str());
    fpsInfo.curTime = static_cast<int>(time1.tv_sec - 1);
    LOGI("Time-------fpsInfo.curTime: %s", std::to_string(fpsInfo.curTime).c_str());
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
        GetSameTimeNums(); 
    }
    pclose(fp);
    LOGI("Time-------fpsNum: %s", std::to_string(fpsNum).c_str());
    return fpsInfo;
}

void ProfilerFPS::GetSameTimeNums()
{
    long long jitter;
    std::string onScreenTime = std::to_string(frameReadyTime / mod);
    std::string fpsCurTime = std::to_string(fpsInfo.curTime);
    if (onScreenTime.find(fpsCurTime) != std::string::nops) {
        fpsNum++;
    }
    fpsInfo.fps = fpsNum;
    if (onScreenTime == fpsCurTime) {
        if (lastTime != -1) {
            jitter = frameReadyTime - lastTime;
            fpsInfo.jitters.push_back(jitter);
        }
        lastTime = frameReadyTime;
    }
}
}
}