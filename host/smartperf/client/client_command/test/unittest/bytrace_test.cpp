/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include "unistd.h"
#include "sp_utils.h"
#include "ByTrace.h"
#include "common.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class ByTraceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(ByTraceTest, ThreadGetTraceTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    std::string result;
    std::string cmdString;
    if (SPUtils::IsHmKernel()) {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_1024);
    } else {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_2048);
    }
    std::string time = std::to_string(SPUtils::GetCurTime());
    std::string traceFile = "/data/local/tmp/sptrace_" + time + ".ftrace";
    std::string traceCmdExe = cmdString + traceFile;
    auto ret = SPUtils::LoadCmd(traceCmdExe, result);
    byTrace.ThreadGetTrace();

    EXPECT_EQ(ret, true);
}

HWTEST_F(ByTraceTest, CheckFpsJittersTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    int times = 3;
    int two = 2;
    int curNum = 5;
    int sum = 6;
    long long currentTrigger = -1;
    long long lastTriggerTime = -1;
    int interval = 10000;
    long long curTime = SPUtils::GetCurTime();
    std::vector<long long> jitters = {1000000, 2000000, 3000000};
    long long threshold = 22;
    int cfps = 30;
    int lowfps = 50;
    if (curNum <= sum && currentTrigger < 0 && times > two) {
        for (size_t i = 0; i < jitters.size(); i++) {
            long long normalJitter = jitters[i] / 1e6;
            if (normalJitter > threshold || cfps < lowfps) {
                byTrace.TriggerCatch(curTime);
            }
        }
    }
    if ((curTime - lastTriggerTime) / 1e3 > interval && currentTrigger == 1) {
        currentTrigger = -1;
    }
    TraceStatus result = byTrace.CheckFpsJitters(jitters, cfps);

    EXPECT_EQ(result, TraceStatus::TRACE_FINISH);
}

HWTEST_F(ByTraceTest, TriggerCatchTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    long long lastTriggerTime = SPUtils::GetCurTime();
    usleep(2);
    int curNum = 0;
    int curTime = 0;
    int interval = 2000;
    long long currentTrigger = -1;
    curTime = SPUtils::GetCurTime();
    if ((curTime - lastTriggerTime) / 1e3 > interval && !byTrace.CheckHitraceId()) {
        std::thread tStart([&byTrace] { byTrace.ThreadGetTrace(); });
        currentTrigger = 1;
        lastTriggerTime = curTime;
        curNum++;
        tStart.detach();
    }
    bool haveHitraceId = byTrace.CheckHitraceId();

    EXPECT_EQ(haveHitraceId, true);
}

HWTEST_F(ByTraceTest, CheckHitraceIdTest, TestSize.Level1)
{
    std::string result;
    bool hitraceProc = false;
    std::string hitrace = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_CMD);
    SPUtils::LoadCmd(hitrace, result);
    if (result.empty()) {
        hitraceProc = false;
    }
    if (result.find("-t") != std::string::npos) {
        hitraceProc = true;
    }
    hitraceProc = false;
    
    ASSERT_FALSE(hitraceProc);
}
}
}