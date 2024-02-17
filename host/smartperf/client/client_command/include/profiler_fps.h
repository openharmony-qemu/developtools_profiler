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
#ifndef PLUGNFPS_H
#define PLUGNFPS_H
#include <vector>
#include "sp_profiler.h"
#include <queue>
namespace OHOS {
namespace SmartPerf {
struct DumpEntityProfiler {
    const std::string windowName;
    const std::string displayId;
    const std::string pid;
    const std::string windId;
    const std::string zOrd;
};
struct FpsInfoProfiler {
    int fps;
    int preFps;
    std::vector<long long> jitters;
    std::queue<long long> timeStampQ;
    long long lastFrameReadyTime;
    long long currentFpsTime;
    std::vector<long long> currTimeStamps;
    void Clear()
    {
        fps = 0;
        jitters.clear();
    }
    bool operator==(const FpsInfoProfiler& other) const
    {
        if (fps != other.fps) {
            return false;
        }
        if (jitters.size() != other.jitters.size()) {
            return false;
        }
        for (size_t i = 0; i < jitters.size(); i++) {
            if (jitters[i] != other.jitters[i]) {
                return false;
            }
        }
        return true;
    }
    FpsInfoProfiler()
    {
        fps = 0;
        preFps = 0;
        lastFrameReadyTime = 0;
        currentFpsTime = 0;
    }
};
class ProfilerFPS {
public:
    void GetSectionsFps(FpsInfoProfiler &fpsInfo);
    void GetSectionsPrint(int printCount, long long msStartTime);
    void GetTimeDiff();
    std::string CutLayerName(std::string layerName);
    FpsInfoProfiler GetFpsInfo();
    FpsInfoProfiler GetFpsInfoMax();
    FpsInfoProfiler GetFpsInfoResult(FpsInfoProfiler &fpsInfo, long long &lastLineTime);
    void GetLastFpsInfo(FpsInfoProfiler &fpsInfo);
    void GetPrevFpsInfo(FpsInfoProfiler &fpsInfo);
    void InitParams(FpsInfoProfiler &fpsInfo, long long &lastLineTime);
    void GetResultFPS(int sectionsNum);
    std::string GetSurface();
    std::string GetLayer();
    FpsInfoProfiler fpsInfo;
    FpsInfoProfiler fpsInfoMax;
    FpsInfoProfiler uniteFpsInfo;
    FpsInfoProfiler mFpsInfo;
    FpsInfoProfiler cntFpsInfo;
    FpsInfoProfiler lastFlagFpsInfo;
    FpsInfoProfiler prevFlagFpsInfo;
    void GetFPS(int argc, std::vector<std::string> v);
private:
    std::string pkgName;
    std::string surfaceViewName;
    std::string curLayerName;
    bool jump = false;
    bool refresh = false;
    int cnt = 0;
    int zeroNum = 0;
    int fpsGb = 0;
    long long mod = 1e9;
    long long lastReadyTime = -1;
    long long frameReadyTime = 0;
    long long lastFrame = -1;
    long long currTimeDiff;
    long long currRealTime;
    const int paramFourteen = 14;
    const int paramTwentyFour = 24;
    const int paramThree = 3;
    const int windowNameIndex = 0;
    const int windowIdIndex = 3;
    const int focusNameIndex = 2;
    long long lastCurrTime = 0;
    int num = 1;
    int number = 2;
    int four = 4;
    unsigned long oneSec = 1000000;
    long long msClear = 1000000000;
    long long oneThousand = 1000;
    int ten = 10;
    FpsInfoProfiler GetSurfaceFrame(std::string name);
};
}
}
#endif