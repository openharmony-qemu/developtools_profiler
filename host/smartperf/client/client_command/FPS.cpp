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
#include "include/sp_log.h"
#include "common.h"
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
        prevResultFpsInfo = fpsInfoResult;
    } else {
        fpsInfoResult = GetFpsInfo();
        prevResultFpsInfo = fpsInfoResult;
    }
    std::string screenInfo;
    SPUtils::LoadFile(screenPath, screenInfo);
    size_t pos = 0;
    std::string token;
    std::string value;
    while ((pos = screenInfo.find(";")) != std::string::npos) {
        token = screenInfo.substr(0, pos);
        screenInfo.erase(0, pos + 1);
        if (token.find("current_fps:") != std::string::npos) {
            value = token.substr(token.find(":") + 1);
            break;
        }
    }
    result["refreshrate"] = value;
    if (processFlag) {
        result["fps"] = "NA";
        result["fpsJitters"] = "NA";
    } else {
        int fullFrame = 120;
        if (fpsInfoResult.fps > fullFrame) {
            fpsInfoResult.fps = fullFrame;
        }
        result["fps"] = std::to_string(fpsInfoResult.fps);
        LOGI("result.fps====: %s", std::to_string(fpsInfoResult.fps).c_str());
        LOGI("result.curTime====: %s", std::to_string(fpsInfoResult.curTime).c_str());
        std::string jitterStr = "";
        std::string split = "";
        for (size_t i = 0; i < fpsInfoResult.jitters.size(); i++) {
            if (i > 0) {
                split = ";;";
            }
            jitterStr += split + std::to_string(fpsInfoResult.jitters[i]);
        }
        result["fpsJitters"] = jitterStr;
        LOGI("result.jitters====: %s", jitterStr.c_str());
        SetFpsCurrentFpsTime(fpsInfoResult);
    }
    LOGI("FPS::ItemData map size(%u)", result.size());
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
void FPS::SetLayerName(std::string sName)
{
    surfaceViewName = std::move(sName);
}
FpsInfo FPS::GetDiffLayersFpsInfo(const std::string &sName)
{
    GetCurrentTime();
    fpsInfoMax = GetSurfaceFrame(sName);
    return fpsInfoMax;
}

void FPS::GetCurrentTime()
{
    for (int i = 0; i < fifty; i++) {
        struct timespec time1 = { 0 };
        clock_gettime(CLOCK_MONOTONIC, &time1);
        int curTimeNow = static_cast<int>(time1.tv_sec - 1);
        if (curTimeNow == prevResultFpsInfo.curTime) {
            usleep(sleepNowTime);
        } else {
            break;
        }
    }
}

FpsInfo FPS::GetFpsInfo()
{
    processFlag = false;
    fpsInfoMax.fps = 0;
    if (pkgName.empty()) {
        return fpsInfoMax;
    }
    bool onTop = IsForeGround();
    if (onTop) {
        LOGI("onTop===========");
        std::string uniteLayer = "UniRender";
        GetCurrentTime();
        fpsInfoMax = GetSurfaceFrame(uniteLayer);
    } else {
        std::string processId = "";
        OHOS::SmartPerf::StartUpDelay sp;
        processId = sp.GetPidByPkg(pkgName);
        LOGI("FPS::processId -- %s", processId.c_str());
        if (processId.empty()) {
            processFlag = true;
            fpsInfoMax.Clear();
        } else {
            fpsInfoMax.Clear();
        }
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
    const char* args[] = { "hidumper", "-s", "10", "-a", command.c_str(), nullptr };
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
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        if (execvp(args[0], const_cast<char *const*>(args)) == -1) {
            LOGE("FPS::Failed to execute hidumper");
            return fpsInfo;
        }
    } else {
        close(pipefd[1]);
        ReadDataFromPipe(pipefd[0]);
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);
    }
    return fpsInfo;
}

void FPS::ReadDataFromPipe(int fd)
{
    char tmp[1024];
    fpsNum = 0;
    prevScreenTimestamp = -1;
    LOGI("FPS::dump time: start!");
    struct timespec time1 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    fpsInfo.curTime = static_cast<int>(time1.tv_sec - 1);
    fpsInfo.currTimeDump = (time1.tv_sec - 1) * mod + time1.tv_nsec;
    LOGI("FPS::time1.tv_sec: %s", std::to_string(time1.tv_sec).c_str());
    LOGI("FPS::time1.tv_nsec: %s", std::to_string(time1.tv_nsec).c_str());
    LOGI("FPS::fpsInfo.curTime: %s", std::to_string(fpsInfo.curTime).c_str());
    LOGI("FPS::psInfo.currTimeDump: %s", std::to_string(fpsInfo.currTimeDump).c_str());
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        LOGE("FPS::Failed to open file descriptor");
        return;
    }
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        std::string str(tmp);
        LOGD("FPS::dump time: %s", str.c_str());
        curScreenTimestamp = 0;
        std::stringstream sstream;
        sstream << tmp;
        sstream >> curScreenTimestamp;
        if (curScreenTimestamp == 0) {
            continue;
        }
        CalcFpsAndJitters();
    }
    if (fclose(fp) == EOF) {
        LOGE("FPS::Failed to close file descriptor");
    }
}

void FPS::CalcFpsAndJitters()
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

bool FPS::IsForeGround()
{
    const std::string cmd = "hidumper -s AbilityManagerService -a -l";
    char buf[1024] = {'\0'};
    std::string appLine = "app name [" + pkgName;
    std::string bundleLine = "bundle name [" + pkgName;
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return false;
    }
    bool tag = false;
    while (fgets(buf, sizeof(buf), fd) != nullptr) {
        std::string line = buf;
        if (line.find(appLine) != std::string::npos) {
            isFoundAppName = true;
        }
        if (line.find(bundleLine) != std::string::npos) {
            isFoundBundleName = true;
        }
        if (isFoundAppName || isFoundBundleName) {
            if (line.find("app state") != std::string::npos) {
                tag = IsFindForeGround(line);
                isFoundAppName = false;
                isFoundBundleName = false;
            }
        }
    }
    pclose(fd);
    return tag;
}
bool FPS::IsFindForeGround(std::string line) const
{
    std::string foreGroundTag = line.substr(line.find("#") + 1);
    if (foreGroundTag.find("FOREGROUND") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
}
}

