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
    std::vector<long long> jitters;
    std::vector<long long> currTimeStamps;
    int curTime;
    long long currTimeDump;
    void Clear()
    {
        fps = 0;
        curTime = 0;
        jitters.clear();
    }
    bool operator == (const FpsInfo &other) const
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
        curTime = 0;
        currTimeDump = 0;
    }
};
struct FpsCurrentFpsTime {
    int fps = 0;
    long long currentFpsTime = 0;
};

class FPS : public SpProfiler {
public:
    void SetPackageName(std::string pName);
    void SetLayerName(std::string sName);
    FpsInfo GetFpsInfo();
    FpsInfo GetDiffLayersFpsInfo(const std::string &sName);
    bool IsForeGround();
    bool IsFindForeGround(std::string line) const;
    void CalcFpsAndJitters();
    void GetCurrentTime();
    FpsInfo fpsInfo;
    FpsInfo fpsInfoMax;
    FpsInfo prevResultFpsInfo;
    static FPS &GetInstance()
    {
        static FPS instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    void SetFpsCurrentFpsTime(FpsInfo fpsInfoResult);
    FpsCurrentFpsTime GetFpsCurrentFpsTime();
    void ReadDataFromPipe(int fd);
private:
    FPS() {};
    FPS(const FPS &);
    FPS &operator = (const FPS &);

    std::string pkgName;
    std::string surfaceViewName;
    bool refresh = false;
    long long mod = 1e9;
    long long curScreenTimestamp = 0;
    long long prevScreenTimestamp = -1;
    long long prevlastScreenTimestamp = 0;
    int fpsNum = 0;
    FpsInfo GetSurfaceFrame(std::string name);
    unsigned long sleepNowTime = 10000;
    bool isFoundAppName = false;
    bool isFoundBundleName = false;
    int fifty = 50;
    FpsCurrentFpsTime ffTime;
    bool processFlag = false;
    const std::string screenPath = "/sys/class/graphics/fb0/lcd_fps_scence";
};
}
}
#endif
