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
    std::vector<long long> currDumpTimeStamps;
    int curTime;
    long long currTimeDiff;
    long long currTimeDump;
    void Clear()
    {
        fps = 0;
        curTime = 0;
        currTimeDiff = 0;
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
        currTimeDiff = 0;
        currTimeDump = 0;
    }
};
struct FpsCurrentFpsTime {
    int fps = 0;
    long long currentFpsTime = 0;
};

class FPS : public SpProfiler {
public:
    void SetPackageName(const std::string& pName);
    void SetLayerName(const std::string& sName);
    FpsInfo GetFpsInfo();
    FpsInfo GetDiffLayersFpsInfo(const std::string &sName);
    FpsInfo GetChangedLayerFps();
    bool CalcFpsAndJitters(bool isBreak);
    long long CalculateJitter() const;
    FpsInfo fpsInfo;
    FpsInfo fpsInfoData;
    FpsInfo prevResultFpsInfo;
    static FPS &GetInstance()
    {
        static FPS instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    void StartExecutionOnce(bool isPause) override;
    void SetFpsCurrentFpsTime(FpsInfo fpsInfoResult);
    FpsCurrentFpsTime GetFpsCurrentFpsTime();
    void ReadDataFromPipe(int fd);
    void SetProcessId(const std::string &pid);
    void SetRkFlag();
    std::string FindFpsRefreshrate();
    std::string GetHardenRefreshrate(std::string &screenInfo) const;
    void CalcJitters();
    static inline bool isGameApp = false;
    static inline bool isNeedDump = false;
    static inline bool isPreset = false;
    static inline bool isLowCurFps = false;
    static inline bool isHistoryHap = false;
    bool SetOtherDeviceFlag();
    bool hapLowFpsFlag = false;
    bool hapNeedCatonInfo = false;
    int catonNum = 0;
    void CalcFatalCaton(std::vector<long long>& jitters);
    void GetGameScenStatus();
    FpsInfo GetFpsInfoByDump(const std::string& name);
    FpsInfo GetFpsInfoByRs(const std::string& name);
    std::map<std::string, std::string> GetFpsAndJitters(FpsInfo &fpsInfoResult,
        std::map<std::string, std::string> &result);
    void SetTraceCatch();
    std::string GetGameLayer();
    std::string GetLayerName(std::string &gameLayer, uint64_t &nodeId, const std::string& line, size_t &endPos);
    std::string GetSurfaceId(std::string &gameLayer, uint64_t &nodeId, const std::string &surfaceId,
        const std::string& line, size_t &endPos);
    FpsInfo GetAppFps(std::string &uniteLayer);

    // sections
    void GetOhFps(std::vector<std::string>& v);
    void GetTimeDiff();
    void GetResultFPS(int sectionsNum);
    void GetSectionsFps(FpsInfo &fpsInfoResult, int nums) const;
    void PrintSections(int msCount, long long currTimeLast, long long currTimeStart, long long currLastTime) const;
    void GetSectionsPrint(int printCount, long long msStartTime, int numb, long long harTime) const;
    void GetFPS(std::vector<std::string>& v);
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
    FpsInfo GetSurfaceFrame(const std::string& name);
    int fifty = 50;
    int hundred = 100;
    FpsCurrentFpsTime ffTime;
    bool processFlag = false;
    bool rkFlag = false;
    const std::string screenPath = "/sys/class/graphics/fb0/lcd_fps_scence";
    std::string processId = "";
    std::string gameLayerName;
    bool isOtherDevice = false;
    int inGameSceneNum = 0;
    int eleven = 11;

    int num = 1;
    int number = 2;
    bool ohFlag = false;
    int four = 4;
    int ten = 10;
    long long lastCurrTime = 0;
    long long msClear = 1000000000;
    long oneSec = 1000000;
    long long oneThousand = 1000;
    long long currRealTime = 0;
    unsigned long sleepTime = 950000;
    unsigned long sleepNowTime = 10000;

    bool isSections = false;
    int isCatchTrace = 0;
    std::string nodeIdStr = "";
};
}
}
#endif
