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
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/Capture.h"
#include "include/FPS.h"
#include "include/sp_log.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> FPS::ItemData()
{
    std::map<std::string, std::string> result;
    FpsInfo fpsInfo;
    if (surfaceViewName.length() > 0) {
        fpsInfo = GetDiffLayersFpsInfo(surfaceViewName);
    } else {
        fpsInfo = GetFpsInfo();
    }
    result["fps"] = std::to_string(fpsInfo.fps);
    std::string jitterStr = "";
    std::string split = "";
    for (size_t i = 0; i < fpsInfo.jitters.size(); i++) {
        if (i > 0) {
            split = ";;";
        }
        jitterStr += split + std::to_string(fpsInfo.jitters[i]);
    }
    result["fpsJitters"] = jitterStr;
    if (isCatchTrace > 0) {
        ByTrace::GetInstance().CheckFpsJitters(fpsInfo.jitters, fpsInfo.fps);
    }
    if (isCapture > 0) {
        Capture::GetInstance().TriggerGetCatch(SPUtils::GetCurTime());
    }
    return result;
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
    FpsInfo fpsInfo = GetSurfaceFrame(sName);
    return fpsInfo;
}
std::string FPS::GetSurface()
{
    std::string cmdResult;
    std::string cmdString1 = "hidumper -s 10 -a sur";
    std::string cmdString2 = "face | grep sur";
    std::string cmdString3 = "face";
    SPUtils::LoadCmd(cmdString1 + cmdString2 + cmdString3, cmdResult);
    size_t position1 = cmdResult.find("[");
    size_t position2 = cmdResult.find("]");
    LOGE("cmdResult==: %s", (cmdResult.substr(position1 + 1, position2 - position1 - 1)).c_str());
    return cmdResult.substr(position1 + 1, position2 - position1 - 1);
}
static void PrintFpsInfo(FpsInfo &fpsInfo, std::string Tag, std::string Type)
{
    LOGE("Print fps: %s", std::to_string(fpsInfo.fps).c_str());
    LOGE("Print Tag: %s", Tag.c_str());
    LOGE("Print Type: %s", Type.c_str());
    std::string jit
    std::string split = "";
    for (size_t i = 0; i < fpsInfo.jitters.size(); i++) {
        if (i > 0) {
            split = ";;";
        }
        jitterStr += split + std::to_string(fpsInfo.jitters[i]);
    }
    LOGE("Print Tag: %s", Tag.c_str());
}
std::string FPS::CutLayerName(std::string layerName)
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

FpsInfo FPS::GetFpsInfo()
{
    FpsInfo fpsInfoMax;
    fpsInfoMax.fps = 0;
    int fpsValue = 0;
    if (pkgName.empty()) {
        return fpsInfoMax;
    }
    std::string layerName;
    std::vector<std::string> sps;
    SPUtils::StrSplit(this->pkgName, ".", sps);
    const uint64_t pNameLastPos = sps.size();
    std::string pkgSuffix = sps[pNameLastPos - 1];
    layerName = std::string(pkgSuffix.c_str());
    std::string uniteLayer = "DisplayNode";
    uniteLayer = GetSurface();
    std::string line = GetLayer(layerName);
    std::vector<std::string> params;
    SPUtils::StrSplit(line, ":", params);
    std::string pkgZOrd = params[1];
    std::string zOrd = "-1";
    std::string focusSurface = params[0];
    FpsInfo uniteFpsInfo;
    std::string subStrLayerName = CutLayerName(layerName);
    LOGE("layerName: %s", layerName.c_str(), "uniteLayer: %s", uniteLayer.c_str(), "line: %s", line.c_str(),
        "focusSurface: %s", focusSurface.c_str(), "subStrLayerName: %s", subStrLayerName.c_str());
    if ((focusSurface.find(subStrLayerName) != std::string::npos) && (strcmp(pkgZOrd.c_str(), zOrd.c_str()) != 0)) {
        uniteFpsInfo = GetSurfaceFrame(uniteLayer);
        PrintFpsInfo(uniteFpsInfo, uniteLayer, "one");
    }
    FpsInfo fpsInfo = GetSurfaceFrame(focusSurface);
    PrintFpsInfo(fpsInfo, focusSurface, "two");

    if (fpsInfo.fps > uniteFpsInfo.fps) {
        fpsInfoMax = fpsInfo;
    } else {
        fpsInfoMax = uniteFpsInfo;
    }
    if (fpsInfoMax.fps < fpsValue) {
        fpsInfoMax.fps = fpsValue;
    }
    if (flagFpsInfo == fpsInfoMax) {
        if (cntFpsInfo.fps != 0) {
            fpsInfoMax = cntFpsInfo;
        } else {
            flagFpsInfo = fpsInfoMax;
            fpsInfoMax.Clear();
        }
        return fpsInfoMax;
    } else {
        flagFpsInfo = fpsInfoMax;
        return fpsInfoMax;
    }
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
        tmp.preFps = 0;
        fpsMap[name] = tmp;
    }
    FpsInfo &fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    FILE *fp;
    static char tmp[1024];
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
    LOGE("cmd=====: %s", cmd.c_str());
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return fpsInfo;
    }
    long long mod = 1e9;
    long long lastReadyTime = -1;
    int fpsGb = 0;
    static long long lastLineTime;
    if (!(fpsInfo.timeStampQ).empty()) {
        lastReadyTime = (fpsInfo.timeStampQ).back();
        lastLineTime = (fpsInfo.timeStampQ).back();
    }
    bool jump = false;
    bool refresh = false;
    int cnt = 0;
    int zeroNum = 0;
    LOGE("dump time: start!");
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        std::string str(tmp);
        LOGE("dump time: %s", str.c_str());
        long long frameReadyTime = 0;
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
        refresh = true;
        long long tFrameReadyTime = frameReadyTime / mod;
        long long tLastReadyTime = lastReadyTime / mod;
        long long lastFrame = -1;
        if (tFrameReadyTime == tLastReadyTime) {
            (fpsInfo.timeStampQ).push(frameReadyTime);
        } else if (tFrameReadyTime == tLastReadyTime + 1) {
            jump = true;
            lastReadyTime = frameReadyTime;
            int fpsTmp = 0;
            fpsInfo.jitters.clear();
            while (!(fpsInfo.timeStampQ).empty()) {
                fpsTmp++;
                long long currFrame = (fpsInfo.timeStampQ.front());
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
            LOGE("fpsGb====: %s", std::to_string(fpsGb).c_str());
            (fpsInfo.timeStampQ).push(frameReadyTime);
            fpsInfo.lastFrameReadyTime = lastFrame;
        } else if (tFrameReadyTime > tLastReadyTime + 1) {
            jump = true;
            lastReadyTime = frameReadyTime;
            while (!(fpsInfo.timeStampQ).empty()) {
                (fpsInfo.timeStampQ).pop();
            }
            (fpsInfo.timeStampQ).push(frameReadyTime);
        }
        int total = 138;
        if (cnt == total && fpsGb != 0) {
            lastReadyTime = frameReadyTime;
            int fpsTmp = 0;
            cntFpsInfo.jitters.clear();
            while (!(fpsInfo.timeStampQ).empty()) {
                fpsTmp++;
                long long currFrame = (fpsInfo.timeStampQ.front());
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
            LOGE("cntFpsInfo.fps====: %s", std::to_string(cntFpsInfo.fps).c_str());
        }
    }
    pclose(fp);
    const int maxZeroNum = 130;
    if (zeroNum >= maxZeroNum) {
        LOGE("zeroNum====: %s", std::to_string(zeroNum).c_str());
        while (!(fpsInfo.timeStampQ.empty())) {
            fpsInfo.timeStampQ.pop();
        }
        fpsInfo.fps = 0;
        fpsInfo.jitters.clear();
        LOGE("fpsInfo.fps0: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
    const int minPrintLine = 5;
    if (cnt < minPrintLine) {
        fpsInfo.fps = fpsInfo.preFps;
        LOGE("fpsInfo.fps1: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
    if (!fpsInfo.timeStampQ.empty() && fpsInfo.timeStampQ.back() == lastLineTime) {
        fpsInfo.fps = fpsGb;
        if (fpsGb == 0) {
            fpsInfo.jitters.clear();
        }
        LOGE("fpsInfo.fps2: %s", std::to_string(fpsInfo.fps).c_str());
        LOGE("lastLineTime: %s", std::to_string(lastLineTime).c_str());
        return fpsInfo;
    }
    if (fpsGb > 0) {
        fpsInfo.fps = fpsGb;
        fpsInfo.preFps = fpsGb;
        LOGE("fpsInfo.fps3: %s", std::to_string(fpsInfo.fps).c_str());
        LOGE("fpsInfo.preFps3: %s", std::to_string(fpsInfo.preFps).c_str());
        return fpsInfo;
    } else if (refresh && !jump) {
        fpsInfo.fps = fpsInfo.preFps;
        LOGE("fpsInfo.fps4: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    } else {
        fpsInfo.fps = 0;
        fpsInfo.jitters.clear();
        LOGE("fpsInfo.fps5: %s", std::to_string(fpsInfo.fps).c_str());
        return fpsInfo;
    }
}

std::string FPS::GetLayer(std::string pkgSurface)
{
    std::vector<DumpEntity> dumpEntityList;
    std::string curFocusId = "-1";
    const std::string cmd = "hidumper -s WindowManagerService -a -a";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd != nullptr) {
        int lineNum = 0;
        char buf[1024] = {'\0'};
        const int paramFourteen = 14;
        const int paramTwentyFour = 14;
        const int paramThree = 3;
        const int windowNameIndex = 0;
        const int windowIdIndex = 3;
        const int focusNameIndex = 2;
        while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
            std::string line = buf;
            LOGE("hidumperline: %s", line.c_str());
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
                DumpEntity dumpEntity { params[0], params[1], params[2], params[3], params[7]};
                dumpEntityList.push_back(dumpEntity);
            }
            if (params.size() == paramFourteen || params.size() == paramTwentyFour) {
                DumpEntity dumpEntity { params[0], params[2], params[2], params[3], params[6]};
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

    std::string focusWindowName = "NA";
    std::string pkgZOrd = "-1";
    std::string spSurfacePrefix = "sp_";
    std::string floatWindow = "floatWindow";
    int curId = std::stoi(curFocusId);
    LOGE("getLayerCurId====: %s", std::to_string(curId).c_str());
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntity dumpItem = dumpEntityList[i];
        int curWinId = std::stoi(dumpItem.windId);
        if (curId == curWinId) {
            LOGE("curId == curWinId");
            if ((dumpItem.windowName.find(spSurfacePrefix) != std::string::npos) ||
                (dumpItem.windowName.find(floatWindow) != std::string::npos) && dumpItem.zOrd != "-1") {
                continue;
            }
            focusWindowName = dumpItem.windowName;
            LOGE("focusWindowName: %s", focusWindowName.c_str());
            LOGE("dumpItem.windowName: %s", dumpItem.windowName.c_str());
        }
        if ((dumpItem.windowName.find(pkgSurface) != std::string::npos) && dumpItem.zOrd != "-1") {
            focusWindowName = dumpItem.windowName;
            pkgZOrd = dumpItem.zOrd;
            LOGE("pkgZOrd: %s", pkgZOrd.c_str());
            LOGE("dumpItem.zOrd: %s", dumpItem.zOrd.c_str());
            LOGE("focusWindowName2: %s", focusWindowName.c_str());
            LOGE("dumpItem.windowName2: %s", dumpItem.windowName.c_str());
        }
    }
    return focusWindowName + ":" + pkgZOrd;
}
}
}
