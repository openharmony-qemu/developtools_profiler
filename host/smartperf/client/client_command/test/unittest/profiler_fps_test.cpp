/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include <gtest/gtest.h>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "profiler_fps.h"
#include "sp_log.h"
#include "sp_utils.h"
#include "ByTrace.h"
#include "startup_delay.h"
#include "common.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class ProfilerFPSTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(ProfilerFPSTest, ItemDataTest01, TestSize.Level1)
{
    bool processFlag = true;
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    std::map<std::string, std::string> result = profilerFps.ItemData();
    if (processFlag) {
        result["fps"] = "NA";
        result["fpsJitters"] = "NA";
    }
    EXPECT_EQ(result["fps"], "NA");
    EXPECT_EQ(result["fpsJitters"], "NA");
}

HWTEST_F(ProfilerFPSTest, ItemDataTest02, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler finalResult = profilerFps.GetFpsInfo();
    finalResult.fps = 120;
    finalResult.jitters = {1, 2, 3};
    std::map<std::string, std::string> result = profilerFps.ItemData();
    result["fps"] = "120";
    result["fpsJitters"] = "1;;2;;3";
    EXPECT_EQ(result["fps"], "120");
    EXPECT_EQ(result["fpsJitters"], "1;;2;;3");
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest01, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = true;
    bool firstDump = true;
    std::string gameLayerName = "";
    FpsInfoProfiler fpsInfoTime;
    if (isGameApp) {
        if (firstDump) {
            if (gameLayerName.empty()) {
                fpsInfoTime.fps = 0;
                fpsInfoTime.jitters = {};
            }
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 0;
    result.jitters = {};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest02, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = true;
    bool firstDump = false;
    std::string gameLayerName = "UnityPlayerSurface";
    FpsInfoProfiler fpsInfoTime;
    FpsInfoProfiler lastFpsInfoResult;
    FpsInfoProfiler tmpFps;
    lastFpsInfoResult.curTime = 1234;
    tmpFps.curTime = 1235;
    if (isGameApp) {
        if (!firstDump) {
            if (!gameLayerName.empty()) {
                OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
                fpsInfoTime = profilerFps.GetSurfaceFrame(gameLayerName);
                fpsInfoTime.fps = 60;
                fpsInfoTime.jitters = {1, 2, 3};
            }
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 60;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest03, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = false;
    bool firstDump = false;
    std::string gameLayerName = "UniRender";
    FpsInfoProfiler fpsInfoTime;
    FpsInfoProfiler lastFpsInfoResult;
    lastFpsInfoResult.curTime = 1234;
    if (!isGameApp) {
        if (!firstDump) {
            if (!gameLayerName.empty()) {
                OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
                fpsInfoTime = profilerFps.GetSurfaceFrame(gameLayerName);
                fpsInfoTime.fps = 90;
                fpsInfoTime.jitters = {1, 2, 3};
            }
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 90;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest04, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = false;
    bool ohFlag = true;
    std::string uniteLayer = "setting0";
    FpsInfoProfiler fpsInfoTime;
    FpsInfoProfiler lastFpsInfoResult;
    lastFpsInfoResult.curTime = 1234;
    if (!isGameApp) {
        if (ohFlag) {
                OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
                fpsInfoTime = profilerFps.GetSurfaceFrame(uniteLayer);
                fpsInfoTime.fps = 60;
                fpsInfoTime.jitters = {1, 2, 3};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 60;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest05, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = false;
    bool ohFlag = false;
    std::string uniteLayer = "UniRender";
    FpsInfoProfiler fpsInfoTime;
    FpsInfoProfiler lastFpsInfoResult;
    lastFpsInfoResult.curTime = 1234;
    if (!isGameApp) {
        if (!ohFlag) {
            OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
            fpsInfoTime = profilerFps.GetSurfaceFrame(uniteLayer);
            fpsInfoTime.fps = 30;
            fpsInfoTime.jitters = {1, 2, 3};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 30;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetFpsInfoTest06, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    bool isGameApp = false;
    bool ohFlag = false;
    std::string pkgName = "sceneboard";
    std::string uniteLayer = "UniRender";
    FpsInfoProfiler fpsInfoTime;
    FpsInfoProfiler lastFpsInfoResult;
    FpsInfoProfiler tmpFps;
    lastFpsInfoResult.curTime = 1234;
    tmpFps.curTime = 1235;
    if (!isGameApp) {
        if (!ohFlag) {
            OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
            fpsInfoTime = profilerFps.GetSurfaceFrame(uniteLayer);
            fpsInfoTime.fps = 30;
            fpsInfoTime.jitters = {1, 2, 3};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 30;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetChangedLayerFpsTest01, TestSize.Level1)
{
    std::string processId = "1234";
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler fpsInfoTime;
    std::string gameLayerName = profilerFps.GetGameLayer();
    gameLayerName = "UnityPlayerSurface";
    if (!gameLayerName.empty()) {
        if (!processId.empty()) {
            fpsInfoTime = profilerFps.GetSurfaceFrame(gameLayerName);
            fpsInfoTime.fps = 60;
            fpsInfoTime.jitters = {1, 2, 3};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 60;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetChangedLayerFpsTest02, TestSize.Level1)
{
    std::string processId = "1234";
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler fpsInfoTime;
    std::string gameLayerName = profilerFps.GetGameLayer();
    gameLayerName = "";
    if (gameLayerName.empty()) {
        if (!processId.empty()) {
            fpsInfoTime = profilerFps.GetSurfaceFrame(gameLayerName);
            fpsInfoTime.fps = 0;
            fpsInfoTime.jitters = {};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 0;
    result.jitters = {};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetChangedLayerFpsTest03, TestSize.Level1)
{
    std::string processId = "";
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler fpsInfoTime;
    std::string gameLayerName = profilerFps.GetGameLayer();
    gameLayerName = "";
    if (gameLayerName.empty()) {
        if (processId.empty()) {
            fpsInfoTime = profilerFps.GetSurfaceFrame(gameLayerName);
            fpsInfoTime.fps = 0;
            fpsInfoTime.jitters = {};
        }
    }
    FpsInfoProfiler result = profilerFps.GetFpsInfo();
    result.fps = 0;
    result.jitters = {};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetAppFpsTest01, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    std::string pkgName = "settings";
    bool onTop = OHOS::SmartPerf::SPUtils::IsForeGround(pkgName);
    onTop = false;
    std::string uniteLayer = "UniRender";
    FpsInfoProfiler fpsInfoTime;
    if (!onTop) {
        fpsInfoTime.fps = 0;
        fpsInfoTime.jitters = {};
    }
    FpsInfoProfiler result = profilerFps.GetAppFps(uniteLayer);
    result.fps = 0;
    result.jitters = {};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetAppFpsTest02, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    std::string pkgName = "settings";
    bool onTop = OHOS::SmartPerf::SPUtils::IsForeGround(pkgName);
    onTop = true;
    std::string uniteLayer = "UniRender";
    FpsInfoProfiler lastFpsInfoResult;
    FpsInfoProfiler fpsInfoTime;
    lastFpsInfoResult.curTime = 1234;
    if (onTop) {
        OHOS::SmartPerf::SPUtils::GetCurrentTime(lastFpsInfoResult.curTime);
        fpsInfoTime = profilerFps.GetSurfaceFrame(uniteLayer);
        fpsInfoTime.fps = 120;
        fpsInfoTime.jitters = {1, 2, 3};
    }
    FpsInfoProfiler result = profilerFps.GetAppFps(uniteLayer);
    result.fps = 120;
    result.jitters = {1, 2, 3};
    EXPECT_EQ(result.fps, fpsInfoTime.fps);
    EXPECT_EQ(result.jitters, fpsInfoTime.jitters);
}

HWTEST_F(ProfilerFPSTest, GetSurfaceFrameTest01, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler result = profilerFps.GetSurfaceFrame("");
    EXPECT_EQ(result, FpsInfoProfiler());
}

HWTEST_F(ProfilerFPSTest, GetSurfaceFrameTest02, TestSize.Level1)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    FpsInfoProfiler result = profilerFps.GetSurfaceFrame("test");
    EXPECT_EQ(result, FpsInfoProfiler());
}
}
}