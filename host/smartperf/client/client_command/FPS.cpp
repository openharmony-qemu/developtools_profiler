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
#include <thread>
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/Capture.h"
#include "include/FPS.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> FPS::ItemData()
{
    std::map<std::string, std::string> result;
    FpsInfo fpsInfo = GetFpsInfo();
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
        ByTrace::GetInstance().CheckFpsJitters(fpsInfo.jitters);
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
std::string FPS::GetSurface()
{
    std::string cmdResult;
    std::string cmdString1 = "hidumper -s 10 -a sur";
    std::string cmdString2 = "face | grep sur";
    std::string cmdString3 = "face";
    SPUtils::LoadCmd(cmdString1 + cmdString2 + cmdString3, cmdResult);
    size_t position1 = cmdResult.find("[");
    size_t position2 = cmdResult.find("]");
    return cmdResult.substr(position1 + 1, position2 - position1 - 1);
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
    std::string addEndChar = "0";
    const uint64_t pNameLastPos = sps.size();
    std::string pkgSuffix = sps[pNameLastPos - 1];
    layerName = std::string(pkgSuffix.c_str() + addEndChar);
    std::string uniteLayer = "DisplayNode";
    uniteLayer = GetSurface();
    std::string spSurfacePrefix = "sp_";
    std::string line = GetLayer(layerName);
    std::vector<std::string> params;
    SPUtils::StrSplit(line, ":", params);
    std::string pkgZOrd = params[1];
    std::string zOrd = "-1";
    std::string focusSurface = params[0];
    FpsInfo uniteFpsInfo;
    if (focusSurface.find(layerName) != std::string::npos) {
        uniteFpsInfo = GetSurfaceFrame(uniteLayer);
    }
    if ((focusSurface.find(spSurfacePrefix) != std::string::npos) && (strcmp(pkgZOrd.c_str(), zOrd.c_str()) != 0)) {
        if (uniteFpsInfo.fps <= fpsValue) {
           uniteFpsInfo = GetSurfaceFrame(uniteLayer);
        }
    }
    FpsInfo fpsInfo = GetSurfaceFrame(layerName);
    if (fpsInfo.fps > uniteFpsInfo.fps) {
        fpsInfoMax = fpsInfo;
    } else {
        fpsInfoMax = uniteFpsInfo;
    }
    if (fpsInfoMax.fps < fpsValue) {
        fpsInfoMax.fps = fpsValue;
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
        tmp.preFps = 0;
        fpsMap[name] = tmp;
    }
    FpsInfo &fpsInfo = fpsMap[name];
    fpsInfo.fps = 0;
    FILE *fp;
    static char tmp[1024];
    std::string cmd = "hidumper -s 10 -a \"fps " + name + "\"";
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
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
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
                }
                lastFrame = currFrame;
                (fpsInfo.timeStampQ).pop();
            }

            fpsGb = fpsTmp;

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
    }

    pclose(fp);
    const int maxZeroNum = 120;
    if (zeroNum >= maxZeroNum) {
        while (!(fpsInfo.timeStampQ.empty())) {
            fpsInfo.timeStampQ.pop();
        }
        fpsInfo.fps = 0;
        return fpsInfo;
    }
    const int minPrintLine = 5;
    if (cnt < minPrintLine) {
        fpsInfo.fps = fpsInfo.preFps;
        return fpsInfo;
    }
    if (!fpsInfo.timeStampQ.empty() && fpsInfo.timeStampQ.back() == lastLineTime) {
        fpsInfo.fps = fpsGb;
        return fpsInfo;
    }
    if (fpsGb > 0) {
        fpsInfo.fps = fpsGb;
        fpsInfo.preFps = fpsGb;
        return fpsInfo;
    } else if (refresh && !jump) {
        fpsInfo.fps = fpsInfo.preFps;
        return fpsInfo;
    } else {
        fpsInfo.fps = 0;
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

        const int paramFifteen = 15;
        const int paramThree = 3;
        const int windowNameIndex = 0;
        const int windowIdIndex = 3;
        const int focusNameIndex = 2;

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
            if (params.size() == paramFifteen) {
                DumpEntity dumpEntity { params[0], params[1], params[2], params[3], params[7]};
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
    int curId = std::stoi(curFocusId);
    for (size_t i = 0; i < dumpEntityList.size(); i++) {
        DumpEntity dumpItem = dumpEntityList[i];
        int curWinId = std::stoi(dumpItem.windId);
        if (curId == curWinId) {
            focusWindowName = dumpItem.windowName;
        }
        if (dumpItem.windowName.find(pkgSurface) != std::string::npos)
        {
            pkgZOrd = dumpItem.zOrd;
        }
    }
    return focusWindowName + ":" + pkgZOrd;
}
}
}
