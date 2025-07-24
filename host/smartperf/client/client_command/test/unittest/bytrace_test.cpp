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

HWTEST_F(ByTraceTest, SetTraceConfigTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    long long threshold = 200;
    int lowfps = 10;
    byTrace.SetTraceConfig(threshold, lowfps);
    EXPECT_EQ(byTrace.threshold, threshold);
    EXPECT_EQ(byTrace.lowfps, lowfps);
}

HWTEST_F(ByTraceTest, SetByTraceTest01, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    byTrace.jittersAndLowFps = "";
    byTrace.SetByTrace();
    EXPECT_EQ(byTrace.threshold, 0);
    EXPECT_EQ(byTrace.lowfps, 0);
}

HWTEST_F(ByTraceTest, SetByTraceTest02, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    byTrace.jittersAndLowFps = "fpsJitterTime=100||lowFps=10";
    byTrace.SetByTrace();
    EXPECT_EQ(byTrace.threshold, 100);
    EXPECT_EQ(byTrace.lowfps, 10);
}

HWTEST_F(ByTraceTest, ClearTraceFilesTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    byTrace.traceCpPath_ = "/data/local/tmp/hitrace";
    std::filesystem::remove_all(byTrace.traceCpPath_);
    byTrace.ClearTraceFiles();
    EXPECT_TRUE(std::filesystem::exists(byTrace.traceCpPath_));
}

HWTEST_F(ByTraceTest, RemoveTraceFiles01, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    byTrace.traceCpPath_ = "/data/local/tmp/hitrace";
    std::filesystem::directory_iterator(traceCpPath_);
    std::ofstream file(byTrace.traceCpPath_ + "/test_file.txt");
    file << "test content";
    file.close();
    byTrace.RemoveTraceFiles();
    EXPECT_FALSE(std::filesystem::exists(byTrace.traceCpPath_ + "/test_file.txt"));
}

HWTEST_F(ByTraceTest, RemoveTraceFiles02, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    byTrace.traceCpPath_ = "/data/local/tmp/hitrace";
    byTrace.RemoveTraceFiles();
    EXPECT_TRUE(std::filesystem::exists(byTrace.traceCpPath_));
}

HWTEST_F(ByTraceTest, CheckFpsJittersTest, TestSize.Level1)
{
    ByTrace &byTrace = ByTrace::GetInstance();
    long long jitters = 101;
    int cfps = 10;
    byTrace.SetTraceConfig(100, 20);
    byTrace.lastEnableTime = 1000;
    byTrace.nowTime = 500;
    byTrace.CheckFpsJitters(jitters, cfps);
    EXPECT_EQ(byTrace.lastEnableTime, 1000);
}
}
}