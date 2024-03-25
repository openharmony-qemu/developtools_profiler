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
#include <string>
#include <thread>
#include <ctime>
#include <sys/time.h>
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/Capture.h"
#include "include/FPS.h"
#include "include/startup_delay.h"
#include "include/sp_log.h"
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
    if (pkgName.length() > 0) {
        std::string processId = "";
        OHOS::SmartPerf::StartUpDelay sp;
        processId = sp.GetPidByPkg(pkgName);
        LOGI("FPS::processId -- %s", processId.c_str());
        if (processId.empty()) {
            result["fps"] = "NA";
            result["fpsJitters"] = "NA";
        }
    }
    if (isCatchTrace > 0) {
        ByTrace::GetInstance().CheckFpsJitters(fpsInfoResult.jitters, fpsInfoResult.fps);
    }
    if (isCapture > 0) {
        Capture::GetInstance().TriggerGetCatch(SPUtils::GetCurTime());
    }
    SetFpsCurrentFpsTime(fpsInfoResult);

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
void FPS::SetTraceCatch()
{
    isCatchTrace = 1;
}

void FPS::SetCaptureOn()
{
    isCapture = 1;
}

void FPS::SetPackageName(std::string pName)
{
    pkgName = std::move(pName);
}
void FPS::SetLayerName(std::string sName)
{
    surfaceViewName = std::move(sName);
}
FpsInfo FPS::GetDiffLayersFpsInfo(std::string sName)
{
    uniteFpsInfo = GetSurfaceFrame(sName);
    std::this_thread::sleep_for(std::chrono::milliseconds(threeHundred));
    secUniteFpsInfo = GetSurfaceFrame(sName);
    return GetFpsInfoMax();
}

FpsInfo FPS::GetFpsInfoMax()
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
        prevResultFpsInfo = uniteFpsInfo;
        prevSecFpsInfoMax = secUniteFpsInfo;
        prevFpsInfoMax = uniteFpsInfo;
        isFirstResult = true;
        return prevResultFpsInfo;
    }
}

FpsInfo FPS::GetFpsInfo()
{
    isFirstResult = false;
    fpsInfoMax.fps = 0;
    if (pkgName.empty()) {
        return fpsInfoMax;
    }
    std::string uniteLayer = "UniRender";
    bool onTop = IsForeGround();
    if (onTop) {
        LOGI("onTop===========");
        uniteFpsInfo = GetSurfaceFrame(uniteLayer);
        std::this_thread::sleep_for(std::chrono::milliseconds(threeHundred));
        secUniteFpsInfo = GetSurfaceFrame(uniteLayer);
    } else {
        LOGI("in the background==========");
        return fpsInfoMax;
    }
    return GetFpsInfoMax();
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
    FILE *fp;
    static char tmp[1024];
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return fpsInfo;
    }
    fpsNum = 0;
    refresh = true;
    lastTime = -1;
    LOGI("dump time: start!");
    struct timespec time1 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
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
        if (frameReadyTime == 0) {
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

void FPS::GetSameTimeNums()
{
    long long jitter;
    std::string onScreenTime = std::to_string(frameReadyTime / mod);
    std::string fpsCurTime = std::to_string(fpsInfo.curTime);
    if (onScreenTime.find(fpsCurTime) != std::string::npos) {
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

bool FPS::IsForeGround()
{
    const std::string cmd = "aa dump -l";
    char buf[1024] = {'\0'};
    std::string appLine = "app name [" + pkgName;
    std::string bundleLine = "bundle name [" + pkgName;
    isFoundAppName = false;
    isFoundBundleName = false;
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return false;
    }
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
                bool tag = IsFindForeGround(line);
                pclose(fd);
                return tag;
            }
        }
    }
    pclose(fd);
    return false;
}

bool FPS::IsFindForeGround(std::string line)
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
