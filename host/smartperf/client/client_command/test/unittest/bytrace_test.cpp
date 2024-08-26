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
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <thread>
#include "unistd.h"
#include "sp_utils.h"
#include "ByTrace.h"

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
        cmdString = "hitrace --trace_clock mono -t 10 -b 102400 --overwrite idle ace app ohos ability graphic nweb ";
    } else {
        cmdString = "hitrace --trace_clock mono -t 10 -b 204800 --overwrite idle ace app ohos ability graphic nweb ";
    }
    std::string cmdStringEnd = "sched freq sync workq multimodalinput > ";
    std::string file = "/data/local/tmp/sptrace_";
    std::string time = std::to_string(SPUtils::GetCurTime());
    std::string traceFile = file + time + ".ftrace";
    std::string traceCmdExe = cmdString + cmdStringEnd + traceFile;
    auto ret = SPUtils::LoadCmd(traceCmdExe, result);
    byTrace.ThreadGetTrace();

    EXPECT_EQ(ret, true);
}

HWTEST_F(ByTraceTest, CheckFpsJittersTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    std::vector<long long> jitters = {1000000, 2000000, 3000000};
    int cfps = 30;
    TraceStatus result = byTrace.CheckFpsJitters(jitters, cfps);

    EXPECT_EQ(result, TraceStatus::TRACE_FINISH);
}

HWTEST_F(ByTraceTest, CheckHitraceIdTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    std::string result;
    std::string cmd = "ps -ef |grep hitrace |grep -v grep";
    SPUtils::LoadCmd(cmd, result);
    bool resultCheck = byTrace.CheckHitraceId();
    
    ASSERT_FALSE(resultCheck);
}
}
}