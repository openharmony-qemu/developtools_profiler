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
#ifndef FPS_H
#define FPS_H
#include <vector>
#include <queue>
#include "sp_profiler.h"
namespace OHOS {
namespace SmartPerf {
struct FpsInfo {
    int fps;
    int preFps;
    std::vector<long long> jitters;
    std::queue<long long> timeStampQ;
    long long lastFrameReadyTime;
    long long currentFpsTime;
    void Clear()
    {
        fps = 0;
        jitters.clear();
    }
    bool operator==(const FpsInfo& other) const
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
    FpsInfo()
    {
        fps = 0;
        preFps = 0;
        lastFrameReadyTime = 0;
        currentFpsTime = 0;
    }
};
class FPS : public SpProfiler {
public:
    void SetPackageName(std::string pName);
    void SetLayerName(std::string sName);
    void SetCaptureOn();
    void SetTraceCatch();
    std::string CutLayerName(std::string layerName);
    FpsInfo GetFpsInfo();
    FpsInfo GetFpsInfoMax();
    FpsInfo GetFpsInfoResult(FpsInfo &fpsInfo, long long &lastLineTime);
    void GetLastFpsInfo(FpsInfo &fpsInfo);
    void GetPrevFpsInfo(FpsInfo &fpsInfo);
    void InitParams(FpsInfo &fpsInfo, long long &lastLineTime);
    FpsInfo GetDiffLayersFpsInfo(std::string sName);
    std::string GetSurface();
    std::string GetLayer(std::string pkgSurface);
    FpsInfo fpsInfo;
    FpsInfo fpsInfoMax;
    FpsInfo uniteFpsInfo;
    FpsInfo mFpsInfo;
    FpsInfo cntFpsInfo;
    FpsInfo lastFlagFpsInfo; // 倒数第一个flag
    FpsInfo prevFlagFpsInfo; // 倒数第二个flag
    static FPS &GetInstance()
    {
        static FPS instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
private:
    FPS() {};
    FPS(const FPS &);
    FPS &operator = (const FPS &);

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
    int isCatchTrace = 0;
    int isCapture = 0;
    FpsInfo GetSurfaceFrame(std::string name);
};

struct DumpEntity {
    const std::string windowName;
    const std::string displayId;
    const std::string pid;
    const std::string windId;
    const std::string zOrd;
};

}
}
#endif
