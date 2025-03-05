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
#include <string>
#include <thread>
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/Capture.h"
#include "include/FPS.h"
#include "include/startup_delay.h"
#include "include/profiler_fps.h"
#include "include/sp_log.h"
#include "include/common.h"
#include <sys/wait.h>
#include <sys/types.h>
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> FPS::ItemData()
{
    std::map<std::string, std::string> result;
    FpsInfo fpsInfoResult;
    if (surfaceViewName.length() > 0) {
        fpsInfoResult = GetDiffLayersFpsInfo(surfaceViewName);
    } else {
        fpsInfoResult = GetFpsInfo();
    }
    prevResultFpsInfo = fpsInfoResult;
    std::string value = FindFpsRefreshrate();
    result["refreshrate"] = value;
    if (processFlag) {
        result["fps"] = "NA";
        result["fpsJitters"] = "NA";
    } else {
        const int fullFrame = 120;
        const int maxFullFrame = 123;
        if (fpsInfoResult.fps > fullFrame && fpsInfoResult.fps < maxFullFrame) {
            fpsInfoResult.fps = fullFrame;
        }
        result["fps"] = std::to_string(fpsInfoResult.fps);
        std::string jitterStr = "";
        std::string split = "";
        for (size_t i = 0; i < fpsInfoResult.jitters.size(); i++) {
            if (i > 0) {
                split = ";;";
            }
            jitterStr += split + std::to_string(fpsInfoResult.jitters[i]);
        }
        result["fpsJitters"] = jitterStr;
        LOGD("result.fps: %s, result.curTime: %s, result.jitters: %s",
            std::to_string(fpsInfoResult.fps).c_str(),
            std::to_string(fpsInfoResult.curTime).c_str(),
            jitterStr.c_str());
        SetFpsCurrentFpsTime(fpsInfoResult);
    }
    return result;
}

void FPS::SetFpsCurrentFpsTime(FpsInfo fpsInfoResult)
{
    ffTime.fps = fpsInfoResult.fps;
    if (!fpsInfoResult.jitters.empty()) {
        auto maxElement = std::max_element(fpsInfoResult.jitters.begin(), fpsInfoResult.jitters.end());
        ffTime.currentFpsTime = *maxElement;
    }
}

FpsCurrentFpsTime FPS::GetFpsCurrentFpsTime()
{
    return ffTime;
}

void FPS::SetPackageName(std::string pName)
{
    pkgName = std::move(pName);
}

void FPS::SetProcessId(const std::string &pid)
{
    processId = pid;
}

void FPS::SetLayerName(std::string sName)
{
    surfaceViewName = std::move(sName);
}
FpsInfo FPS::GetDiffLayersFpsInfo(const std::string &sName)
{
    OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
    fpsInfoMax = GetSurfaceFrame(sName);
    return fpsInfoMax;
}

FpsInfo FPS::GetFpsInfo()
{
    processFlag = false;
    fpsInfoMax.fps = 0;
    if (pkgName.empty()) {
        return fpsInfoMax;
    }
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    if (isGameApp) {
        if (firstDump) {
            gameLayerName = profilerFps.GetGameLayer();
            if (gameLayerName.empty()) {
                firstDump = true;
                fpsInfoMax.fps = 0;
                fpsInfoMax.jitters.clear();
                return fpsInfoMax;
            } else {
                firstDump = false;
            }
        }
        OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
        fpsInfoMax = GetSurfaceFrame(gameLayerName);
        if (fpsInfoMax.fps == 0) {
            return GetChangedLayerFps();
        } else {
            return fpsInfoMax;
        }
    } else {
        bool onTop = OHOS::SmartPerf::SPUtils::IsForeGround(pkgName);
        if (onTop) {
            std::string uniteLayer;
            if (!rkFlag && !isOtherDevice) {
                uniteLayer = "UniRender";
            } else {
                uniteLayer = profilerFps.GetSurface();
            }
            OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
            fpsInfoMax = GetSurfaceFrame(uniteLayer);
        } else {
            LOGE("FPS:app is in the background");
            if (processId.empty()) {
                processFlag = true;
            }
            fpsInfoMax.fps = 0;
            fpsInfoMax.jitters.clear();
        }
    }
    return fpsInfoMax;
}

bool FPS::SetOtherDeviceFlag()
{
    isOtherDevice = true;
    return isOtherDevice;
}

FpsInfo FPS::GetChangedLayerFps()
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    gameLayerName = profilerFps.GetGameLayer();
    if (gameLayerName.empty()) {
        if (processId.empty()) {
            processFlag = true;
        }
        fpsInfoMax.fps = 0;
        fpsInfoMax.jitters.clear();
    } else {
        fpsInfoMax = GetSurfaceFrame(gameLayerName);
    }
    return fpsInfoMax;
}

FpsInfo FPS::GetSurfaceFrame(std::string name)
{
    if (name == "") {
        return FpsInfo();
    }
    static std::map<std::string, FpsInfo> fpsMap;
    if (fpsMap.count(name) == 0) {
        FpsInfo tmp;
        tmp.fps = 0;
        fpsMap[name] = tmp;
    }
    fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    std::string command = "fps " + name;
    const std::vector<const char*> args = { "hidumper", "-s", "10", "-a", command.c_str(), nullptr };
    int pipefd[2];
    pid_t pid;
    if (pipe(pipefd) == -1) {
        LOGE("FPS::Failed to create pipe");
        return fpsInfo;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("FPS::Failed to fork");
        return fpsInfo;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp(args[0], const_cast<char *const*>(args.data()));
        LOGE("FPS::Failed to execute hidumper");
        _exit(EXIT_FAILURE);
    }
    close(pipefd[1]);
    ReadDataFromPipe(pipefd[0]);
    close(pipefd[0]);
    waitpid(pid, nullptr, 0);
    return fpsInfo;
}

void FPS::ReadDataFromPipe(int fd)
{
    fpsInfo.currTimeStamps.clear();
    char tmp[1024];
    fpsNum = 0;
    prevScreenTimestamp = -1;
    bool isBreak = false;
    struct timespec time1 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    fpsInfo.curTime = static_cast<int>(time1.tv_sec - 1);
    fpsInfo.currTimeDump = (time1.tv_sec - 1) * mod + time1.tv_nsec;
    LOGD("FPS:fpsInfo.curTime: %d, FPS:psInfo.currTimeDump: %lld",
        fpsInfo.curTime, fpsInfo.currTimeDump);
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        LOGE("FPS::Failed to open file descriptor");
        return;
    }
    std::stringstream sstream;
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        LOGD("FPS::ReadDataFromPipe::dump time: %s", tmp);
        std::string tmpStr(tmp);
        curScreenTimestamp = 0;
        sstream.clear();
        sstream.str(tmpStr);
        sstream >> curScreenTimestamp;
        if (curScreenTimestamp == 0) {
            continue;
        }
        if (CalcFpsAndJitters(isBreak)) {
            break;
        }
    }
    CalcJitters();
}

bool FPS::CalcFpsAndJitters(bool isBreak)
{
    long long onScreenTime = curScreenTimestamp / mod;
    bool findFpsCurTime = (onScreenTime == fpsInfo.curTime);
    if (findFpsCurTime) {
        isBreak = true;
        fpsNum++;
        fpsInfo.fps = fpsNum;
        fpsInfo.currTimeStamps.push_back(curScreenTimestamp);
    } else {
        findFpsCurTime = false;
    }
    return isBreak && !findFpsCurTime;
}

void FPS::CalcJitters()
{
    bool isOrder = true;
    if (fpsInfo.currTimeStamps.size() > 1) {
        isOrder = fpsInfo.currTimeStamps[1] - fpsInfo.currTimeStamps[0] > 0;
    }
    if (isOrder) {
        for (size_t i = 0; i < fpsInfo.currTimeStamps.size(); i++) {
            curScreenTimestamp = fpsInfo.currTimeStamps[i];
            long long jitter = CalculateJitter();
            fpsInfo.jitters.push_back(jitter);
            prevlastScreenTimestamp = curScreenTimestamp;
            prevScreenTimestamp = curScreenTimestamp;
        }
    } else {
        for (size_t i = fpsInfo.currTimeStamps.size(); i > 0; i--) {
            curScreenTimestamp = fpsInfo.currTimeStamps[i - 1];
            long long jitter = CalculateJitter();
            fpsInfo.jitters.push_back(jitter);
            prevlastScreenTimestamp = curScreenTimestamp;
            prevScreenTimestamp = curScreenTimestamp;
        }
    }
}
        
long long FPS::CalculateJitter() const
{
    long long jitter;
    if (prevScreenTimestamp == -1) {
        if (prevlastScreenTimestamp != 0 && (curScreenTimestamp - prevlastScreenTimestamp) < mod) {
            jitter = curScreenTimestamp - prevlastScreenTimestamp;
        } else {
            jitter = curScreenTimestamp % mod;
        }
    } else {
        jitter = curScreenTimestamp - prevScreenTimestamp;
    }
    return jitter;
}

void FPS::SetRkFlag()
{
    rkFlag = true;
}

std::string FPS::FindFpsRefreshrate()
{
    std::string value;
    std::string screenInfo;
    SPUtils::LoadFile(screenPath, screenInfo);
    value = GetHardenRefreshrate(screenInfo);
    size_t pos = 0;
    std::string token;
    if (!rkFlag) {
        while ((pos = screenInfo.find(";")) != std::string::npos) {
            token = screenInfo.substr(0, pos);
            screenInfo.erase(0, pos + 1);
            if (token.find("current_fps:") != std::string::npos) {
                value = token.substr(token.find(":") + 1);
                break;
            }
        }
    } else {
            std::string screen = OHOS::SmartPerf::SPUtils::GetScreen();
            std::string start = "refreshrate=";
            size_t startPos = screen.find(start) + start.length();
            size_t endPos = screen.length();
            value = screen.substr(startPos, endPos - startPos);
        }
    return value;
}

std::string FPS::GetHardenRefreshrate(std::string &screenInfo) const
{
    if (screenInfo.empty()) {
        SPUtils::LoadCmd(HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SCREEN), screenInfo);
    }
    std::string value = "";
    std::string refreshrate = "refreshrate=";
    size_t activeModePos = screenInfo.find("activeMode:");
    if (activeModePos != std::string::npos) {
        size_t refreshRatePos = screenInfo.find(refreshrate, activeModePos);
        if (refreshRatePos != std::string::npos) {
            size_t endPos = screenInfo.find(" ", refreshRatePos);
            if (endPos != std::string::npos) {
                value = screenInfo.substr(refreshRatePos + refreshrate.length(),
                endPos - refreshRatePos - refreshrate.length());
            }
        }
    }
    return value;
}
}
}

