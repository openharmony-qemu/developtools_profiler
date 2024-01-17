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
#include <unistd.h>
#include <sys/time.h>
#include "include/profiler_fps.h"


namespace OHOS {
namespace SmartPerf {
struct DumpEntityProfiler {
    const std::string windowName;
    const std::string displayId;
    const std::string pid;
    const std::string windId;
};
struct FpsInfo {
    int fps;
    int preFps;
    std::vector<long long> jitters;
    std::queue<long long> timeStampQ;
    long long lastFrameReadyTime;
    long long currentFpsTime;
    FpsInfo()
    {
        fps = 0;
        preFps = 0;
        lastFrameReadyTime = 0;
        currentFpsTime = 0;
    }
};
struct FpsConfig {
    const long long mod = 1e9;
    long long lastReadyTime;
    int fpsGb;
    bool jump;
    bool refresh;
    int cnt;
    int zeroNum;
    FpsConfig()
    {
        lastReadyTime = -1;
        fpsGb = 0;
        jump = false;
        refresh = false;
        cnt = 0;
        zeroNum = 0;
    }
};
static void StrSplit(const std::string &content, const std::string &sp, std::vector<std::string> &out)
{
    size_t index = 0;
    while (index != std::string::npos) {
        size_t tEnd = content.find_first_of(sp, index);
        std::string tmp = content.substr(index, tEnd - index);
        if (tmp != "" && tmp != " ") {
            out.push_back(tmp);
        }
        if (tEnd == std::string::npos) {
            break;
        }
        index = tEnd + 1;
    }
}

static std::string GetLayer()
{
    std::vector<DumpEntityProfiler> dumpEntityList;
    std::string curFocusId = "-1";
    const std::string cmd = "hidumper -s WindowManagerService -a -a";
    std::string cmdExc = cmd;
    FILE *fd = popen(cmdExc.c_str(), "r");
    if (fd != nullptr) {
        int lineNum = 0;
        std::string line;
        char buf[1024] = {'\0'};
        const int paramFifteen = 15;
        const int paramThree = 3;
        const int windowNameIndex = 0;
        const int windowIdIndex = 3;
        const int focusNameIndex = 2;
        while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
            line = buf;
            if (line[0] == '-' || line[0] == ' ') {
                continue;
            }
            std::vector<std::string> params;
            StrSplit(line, " ", params);
            if (params[windowNameIndex].find("WindowName") != std::string::npos &&
                params[windowIdIndex].find("WinId") != std::string::npos) {
                continue;
            }
            if (params.size() == paramFifteen) {
                DumpEntityProfiler dumpEntity { params[0], params[1], params[2], params[3] };
                dumpEntityList.push_back(dumpEntity);
            }
            if (params.size() == paramThree) {
                curFocusId = params[focusNameIndex];
                break;
            }
            lineNum++;
        }
        pclose(fd);
    }
    std::string resultWindowName = "NA";
    int curId = std::stoi(curFocusId);
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntityProfiler dumpItem = dumpEntityList[i];
        int curWinId = std::stoi(dumpItem.windId);
        if (curId == curWinId) {
            resultWindowName = dumpItem.windowName;
        }
    }
    return resultWindowName;
}
static void ProcessResult(FILE *fp, FpsConfig &fpsConfig, FpsInfo &fpsInfo)
{
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        long long frameReadyTime = 0;
        std::stringstream sstream;
        sstream << tmp;
        sstream >> frameReadyTime;
        fpsConfig.cnt++;
        if (frameReadyTime == 0) {
            fpsConfig.zeroNum++;
            continue;
        }
        if (fpsConfig.lastReadyTime >= frameReadyTime) {
            fpsConfig.lastReadyTime = -1;
            continue;
        }
        fpsConfig.refresh = true;
        long long tFrameReadyTime = frameReadyTime / fpsConfig.mod;
        long long tLastReadyTime = fpsConfig.lastReadyTime / fpsConfig.mod;
        long long lastFrame = -1;
        if (tFrameReadyTime == tLastReadyTime) {
            (fpsInfo.timeStampQ).push(frameReadyTime);
        } else if (tFrameReadyTime == tLastReadyTime + 1) {
            fpsConfig.jump = true;
            fpsConfig.lastReadyTime = frameReadyTime;
            int fpsTmp = 0;
            fpsInfo.jitters.clear();
            while (!(fpsInfo.timeStampQ).empty()) {
                fpsTmp++;
                long long currFrame = (fpsInfo.timeStampQ.front());
                if (lastFrame != -1) {
                    long long jitter = currFrame - lastFrame;
                    fpsInfo.jitters.push_back(jitter);
                } else {
                    long long jitter = currFrame - currFrame / fpsConfig.mod * fpsConfig.mod;
                    fpsInfo.jitters.push_back(jitter);
                }
                lastFrame = currFrame;
                (fpsInfo.timeStampQ).pop();
            }
            fpsConfig.fpsGb = fpsTmp;
            (fpsInfo.timeStampQ).push(frameReadyTime);
            fpsInfo.lastFrameReadyTime = lastFrame;
        } else if (tFrameReadyTime > tLastReadyTime + 1) {
            fpsConfig.jump = true;
            fpsConfig.lastReadyTime = frameReadyTime;
            while (!(fpsInfo.timeStampQ).empty()) {
                (fpsInfo.timeStampQ).pop();
            }
            (fpsInfo.timeStampQ).push(frameReadyTime);
        }
    }
}

static FpsInfo GetSurfaceFrame(std::string name, FpsConfig &fpsConfig)
{
    static std::map<std::string, FpsInfo> fpsMap;
    if (fpsMap.count(name) == 0) {
        FpsInfo tmp;
        tmp.fps = 0;
        tmp.preFps = 0;
        fpsMap[name] = tmp;
    }
    FpsInfo &fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    fpsInfo.currentFpsTime = tv.tv_sec * 1e3 + tv.tv_usec / 1e3;
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
    std::string cmdExc = cmd;
    FILE *fp = popen(cmdExc.c_str(), "r");
    if (fp == nullptr) {
        return fpsInfo;
    }
    static long long lastLineTime;
    if (!(fpsInfo.timeStampQ).empty()) {
        fpsConfig.lastReadyTime = (fpsInfo.timeStampQ).back();
        lastLineTime = (fpsInfo.timeStampQ).back();
    }
    ProcessResult(fp, fpsConfig, fpsInfo);
    pclose(fp);
    const int maxZeroNum = 120;
    const int minPrintLine = 5;
    if (fpsConfig.zeroNum >= maxZeroNum) {
        while (!(fpsInfo.timeStampQ.empty())) {
            fpsInfo.timeStampQ.pop();
        }
        fpsInfo.fps = 0;
        return fpsInfo;
    }
    if (fpsConfig.cnt < minPrintLine) {
        fpsInfo.fps = fpsInfo.preFps;
        return fpsInfo;
    }
    if (!fpsInfo.timeStampQ.empty() && fpsInfo.timeStampQ.back() == lastLineTime) {
        fpsInfo.fps = 0;
        return fpsInfo;
    }
    if (fpsConfig.fpsGb > 0) {
        fpsInfo.fps = fpsConfig.fpsGb;
        fpsInfo.preFps = fpsConfig.fpsGb;
        return fpsInfo;
    } else if (fpsConfig.refresh && !fpsConfig.jump) {
        fpsInfo.fps = fpsInfo.preFps;
        return fpsInfo;
    } else {
        fpsInfo.fps = 0;
        return fpsInfo;
    }
}

static void ReplaceString(std::string &res)
{
    std::string flagOne = "\r";
    std::string flagTwo = "\n";
    std::string::size_type ret = res.find(flagOne);
    while (ret != res.npos) {
        res.replace(ret, 1, "");
        ret = res.find(flagOne);
    }
    ret = res.find(flagTwo);
    while (ret != res.npos) {
        res.replace(ret, 1, "");
        ret = res.find(flagTwo);
    }
}

static bool LoadCmd(const std::string &cmd, std::string &result)
{
    std::string cmdExc = cmd;
    FILE *fd = popen(cmdExc.c_str(), "r");
    if (fd == nullptr) {
        return false;
    }
    char buf[1024] = {'\0'};
    int ret = fread(buf, sizeof(buf), 1, fd);
    if (ret >= 0) {
        result = buf;
    }
    if (pclose(fd) == -1) {
        std::cout << "" << std::endl;
    }
    ReplaceString(result);
    return ret >= 0 ? true : false;
}
static std::string GetSurFace()
{
    std::string cmdResult;
    std::string cmdString1 = "hidumper -s 10 -a sur";
    std::string cmdString2 = "face | grep sur";
    std::string cmdString3 = "face";
    LoadCmd(cmdString1 + cmdString2 + cmdString3, cmdResult);
    size_t position1 = cmdResult.find("[");
    size_t position2 = cmdResult.find("]");
    return cmdResult.substr(position1 + 1, position2 - position1 - 1);
}
static void GetSectionsPrint(int printCount, long long msStartTime)
{
    int ten  = 10;
    long long msJiange = 100;
    if (printCount < ten) {
        for (int i = 0; i < ten - printCount; i++) {
            msStartTime += msJiange;
            printf("sectionsFps:%d|%lld\n", 0, msStartTime);
        }
    }
}
static void GetSectionsFps(FpsInfo &fpsInfo)
{
    int msCount = 0;
    int ten = 10;
    long long msStartTime = fpsInfo.currentFpsTime;
    long long msAdd = 0;
    long long msJiange = 100000000;
    int printCount = 0;
    for (int i = 0; i < fpsInfo.jitters.size(); i++) {
        msAdd += fpsInfo.jitters[i];
        if (msAdd <= msJiange) {
            msCount++;
            if (i == (fpsInfo.jitters.size() - 1)) {
                printf("sectionsFps:%d|%lld\n", msCount * ten, msStartTime);
                printCount++;
                GetSectionsPrint(printCount, msStartTime);
            }
        } else if (msJiange < msAdd && msAdd <= (msJiange + 1e8)) {
            printf("sectionsFps:%d|%lld\n", msCount * ten, msStartTime);
            printCount++;
            msStartTime = fpsInfo.currentFpsTime + (msAdd / 1e6);
            msCount = 1;
            msJiange += 1e8;
        } else {
            printf("sectionsFps:%d|%lld\n", msCount * ten, msStartTime);
            printCount++;
            msStartTime += 1e2;
            msCount = 0;
            msAdd -= fpsInfo.jitters[i];
            i--;
            msJiange += 1e8;
        }
    }
}
static void CalculateFPS(int num, int sectionsNum)
{
        FpsInfo gfpsInfo;
        FpsInfo gfpsUniteInfo;
        std::string layerName;
        std::string tempLayerName;
        struct timeval start;
        struct timeval end;
        std::string uniteLayer = "DisplayNode";
        uniteLayer = GetSurFace();
        unsigned long oneSec = 1000000;
        int ten = 10;
        std::string cmdResult;
        for (int i = 0; i < num; i++) {
            unsigned long runTime;
            gettimeofday(&start, nullptr);
            tempLayerName = GetLayer();
            if (i == 0) {
                layerName = tempLayerName;
                LoadCmd("hidumper -s 10 -a \"fpsClear DisplayNode\"", cmdResult);
                LoadCmd("hidumper -s 10 -a \"fpsClear" + layerName + "\"", cmdResult);
            } else {
                if (layerName.compare(tempLayerName) != 0) {
                    layerName = tempLayerName;
                    LoadCmd("hidumper -s 10 -a \"fpsClear" + layerName + "\"", cmdResult);
                }
            }
            FpsConfig fpsConfig;
            FpsConfig fpsUniteConfig;
            gfpsInfo = GetSurfaceFrame(layerName, fpsConfig);
            gfpsUniteInfo = GetSurfaceFrame(uniteLayer, fpsUniteConfig);
            if (gfpsUniteInfo.fps > gfpsInfo.fps) {
                printf("fps:%d|%lld\n", gfpsUniteInfo.fps, gfpsUniteInfo.currentFpsTime);
                if (sectionsNum == ten) {
                    GetSectionsFps(gfpsUniteInfo);
                }
            } else {
                printf("fps:%d|%lld\n", gfpsInfo.fps, gfpsInfo.currentFpsTime);
                if (sectionsNum == ten) {
                    GetSectionsFps(gfpsInfo);
                }
            }
            fflush(stdout);
            gettimeofday(&end, nullptr);
            runTime = end.tv_sec * 1e6 - start.tv_sec * 1e6 + end.tv_usec - start.tv_usec;
            if (runTime < oneSec) {
                usleep(oneSec - runTime);
            }
        }
}
void ProfilerFPS::GetFPS(int argc, std::vector<std::string> v)
{
    int num = 1;
    int number = 2;
    int ten = 10;
    int four = 4;
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
        if (sectionsNum == ten) {
            CalculateFPS(num, sectionsNum);
        } else {
            CalculateFPS(num, 0);
        }
    }
    printf("SP_daemon exec finished!\n");
}
}
}