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

#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "sp_utils.h"
#include "RAM.h"
#include "GPU.h"
#include "CPU.h"
#include "FPS.h"
#include "Temperature.h"
#include "Power.h"
#include "Capture.h"
#include "Network.h"
#include "profiler_fps.h"
#include "parse_page_fps_trace.h"
#include "parse_click_complete_trace.h"
#include "parse_click_response_trace.h"
#include "parse_slide_fps_trace.h"
#include "parse_start_frame_trace.h"
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: CpuTestCase
 * @tc.desc: Test CPU
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, CpuTestCase, TestSize.Level1)
{
    CPU &cpu = CPU::GetInstance();
    std::string packName = "ohos.samples.ecg";

    std::map<std::string, std::string> cpuItemData = cpu.ItemData();
    cpu.SetPackageName(packName);
    std::vector<CpuFreqs> cpuFreqs = cpu.GetCpuFreq();
    std::vector<CpuUsageInfos> getCpuUsage = cpu.GetCpuUsage();
    std::map<std::string, std::string> getSysProcessCpuLoad = cpu.GetSysProcessCpuLoad();

    std::string cmd = "SP_daemon -N 1 -PKG ohos.samples.ecg -c";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("cpu0Usage");
    std::string::size_type strTwo = result.find("cpu0idleUsage");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: GpuTestCase
 * @tc.desc: Test GPU
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, GpuTestCase, TestSize.Level1)
{
    GPU &gpu = GPU::GetInstance();
    int getGpuFreq = 0;
    float getGpuLoad = 0.0;
    std::map<std::string, std::string> gpuItemData = gpu.ItemData();
    getGpuFreq = gpu.GetGpuFreq();
    getGpuLoad = gpu.GetGpuLoad();

    std::string cmd = "SP_daemon -N 1 -g";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("gpuFrequency");
    std::string::size_type strTwo = result.find("gpuLoad");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: FpsTestCase
 * @tc.desc: Test FPS
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, FpsTestCase, TestSize.Level1)
{
    FPS &fps = FPS::GetInstance();
    std::string packName = "ohos.samples.ecg";
    std::string surfaceViewName;
    FpsInfo fpsInfoResult;
    
    fps.SetFpsCurrentFpsTime(fpsInfoResult);
    fps.SetPackageName(packName);
    fps.SetLayerName(surfaceViewName);
    fps.GetCurrentTime();
    fps.CalcFpsAndJitters();

    std::string cmd = "SP_daemon -N 1 -PKG ohos.samples.ecg -f";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("fpsJitters");
    std::string::size_type strTwo = result.find("timestamp");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: TemperatureTestCase
 * @tc.desc: Test Temperature
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, TemperatureTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 1 -t";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("timestamp");
    std::string::size_type strTwo = result.find("system_h");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }


    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: PowerTestCase
 * @tc.desc: Test Power
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, PowerTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 1 -p";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("currentNow");
    std::string::size_type strTwo = result.find("voltageNow");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: RamTestCase
 * @tc.desc: Test RAM
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, RamTestCase, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    std::string packName = "ohos.samples.ecg";

    ram.SetFirstFlag();
    ram.SetPackageName(packName);
    ram.ThreadGetPss();
    ram.TriggerGetPss();

    std::string cmd = "SP_daemon -N 1 -PKG ohos.samples.ecg -r";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("memAvailable");
    std::string::size_type strTwo = result.find("memTotal");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: SnapShotTestCase
 * @tc.desc: Test SnapShot
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, SnapShotTestCase, TestSize.Level1)
{
    Capture &capture = Capture::GetInstance();
    long long catTime = 0;
    std::string curTime = "";
    
    capture.SocketMessage();
    capture.ThreadGetCatch();
    capture.ThreadGetCatchSocket(curTime);
    capture.TriggerGetCatch();
    capture.TriggerGetCatchSocket(catTime);

    std::string cmd = "SP_daemon -N 1 -snapshot";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("capture");
    std::string::size_type strTwo = result.find(".png");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: NetWorkTestCase
 * @tc.desc: Test NetWork
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, NetWorkTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 1 -net";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("networkDown");
    std::string::size_type strTwo = result.find("networkUp");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: StartTestCase
 * @tc.desc: Test Start
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, StartTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -start -c";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("Collection");
    std::string::size_type strTwo = result.find("begins");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: StopTestCase
 * @tc.desc: Test Stop
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, StopTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -stop";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("Collection");
    std::string::size_type strTwo = result.find("ended");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: ProfilerFpsTestCase
 * @tc.desc: Test ProfilerFps
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, ProfilerFpsTestCase, TestSize.Level1)
{
    ProfilerFPS &profiler = ProfilerFPS::GetInstance();
    std::string packName = "ohos.samples.ecg";
    int sleepNum = 100;
    int sectionsNum = 10;
    FpsInfoProfiler fpsInfoResult;
    int nums = 10;
    int printCount = 10;
    long long msStartTime = 0;
    int numb = 20;
    long long harTime = 0;
    
    profiler.GetCurrentTime(sleepNum);
    profiler.GetResultFPS(sectionsNum);
    profiler.CalcFpsAndJitters();
    profiler.GetSectionsFps(fpsInfoResult, nums);
    profiler.GetSectionsPrint(printCount, msStartTime, numb, harTime);

    std::string cmd = "SP_daemon -profilerfps 10";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("set");
    std::string::size_type strTwo = result.find("success");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: ScreenTestCase
 * @tc.desc: Test Screen
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, ScreenTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -screen";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("activeMode");
    std::string::size_type strTwo = result.find("refreshrate");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

/**
 * @tc.name: FrameLossTestCase
 * @tc.desc: Test FrameLoss
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, FrameLossTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -editor frameLoss";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("BUNDLE_NAME");
    std::string::size_type strTwo = result.find("TOTAL_APP_MISSED_FRAMES");
    if ((strOne != result.npos) && (strTwo != result.npos)) {
        flag = true;
    }

    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

HWTEST_F(SPdaemonTest, ParsePageFpsTraceTest, TestSize.Level1)
{
    ParsePageFpsTrace parsePageFpsTrace;
    double calculateTime = 0.0;
    double calculateTimeEnd = 0.0;
    std::string line = "";
    std::string curString = "";
    std::string start = "";
    std::string end = "";
    size_t offset = 0;

    curString = parsePageFpsTrace.CutString(line, start, end, offset);
    calculateTime = parsePageFpsTrace.CalculateTime();
    calculateTimeEnd = parsePageFpsTrace.CalculateTimeEnd();

    std::string cmd = "SP_daemon -editor pagefps ohos.samples.ecg 设置 ohtest";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("FPS");
    if ((strOne != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

HWTEST_F(SPdaemonTest, ParseClickCompleteTraceTest, TestSize.Level1)
{
    ParseClickCompleteTrace parseClickCompleteTrace;
    double parseClickCompleteName = 0.0;
    double getLineTime = 0.0;
    std::string line = "";
    std::string getStartTime = "";
    std::string startTimeBefore = "";
    std::string file = "";

    parseClickCompleteName = parseClickCompleteTrace.ParseCompleteTrace(file);
    getStartTime = parseClickCompleteTrace.GetStartTime(line, startTimeBefore);
    getLineTime = parseClickCompleteTrace.GetLineTime();

    std::string cmd = "SP_daemon -editor completeTime ohos.samples.ecg 设置 ohtest";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("time");
    if ((strOne != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

HWTEST_F(SPdaemonTest, ParseClickResponseTraceTest, TestSize.Level1)
{
    ParseClickResponseTrace parseClickResponseTrace;
    double parseResponseTrace = 0.0;
    double getLineTime = 0.0;
    std::string line = "";
    std::string getStartTime = "";
    std::string startTimeBefore = "";
    std::string file = "";

    parseResponseTrace = parseClickResponseTrace.ParseResponseTrace(file);
    getStartTime = parseClickResponseTrace.GetStartTime(line, startTimeBefore);
    getLineTime = parseClickResponseTrace.GetLineTime();

    std::string cmd = "SP_daemon -editor responseTime ohos.samples.ecg 设置 ohtest";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("time");
    if ((strOne != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}

HWTEST_F(SPdaemonTest, StartFrameTraceNohTest, TestSize.Level1)
{
    StartFrameTraceNoh startFrameTraceNoh;
    double parseStartFrameTraceNoh = 0.0;
    double calculateTime = 0.0;
    double getFps = 0.0;
    std::vector<std::string> split;
    std::string str = "";
    std::string  curString = "";
    std::string pattern = "";
    std::string file = "";
    std::string start = "";
    std::string end = "";
    size_t offset = 0;

    curString = startFrameTraceNoh.CutString(str, start, end, offset);
    calculateTime = startFrameTraceNoh.CalculateTime();
    parseStartFrameTraceNoh = startFrameTraceNoh.ParseStartFrameTraceNoh(file);
    getFps = startFrameTraceNoh.GetFps();
    split = startFrameTraceNoh.Split(str, pattern);

    std::string cmd = "SP_daemon -editor startFrame ohos.samples.ecg 设置 ohtest";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("FPS");
    if ((strOne != result.npos)) {
        flag = true;
    }
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}
} // namespace OHOS
} // namespace SmartPerf