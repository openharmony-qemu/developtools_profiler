/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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
#include <atomic>
#include <unistd.h>
#include "hook_guard.h"
#include "hook_common.h"

using namespace testing::ext;

extern std::atomic<bool> g_hookReady;
extern std::atomic<pid_t> g_hookPid;
extern std::atomic<bool> g_isPidChanged;
extern struct ClientConfig g_ClientConfig;

namespace {
class HookGuardTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        g_hookReady = false;
        g_hookPid = getpid();
        g_isPidChanged = false;
        g_ClientConfig.mallocDisable = false;
        g_ClientConfig.mmapDisable = false;
        g_ClientConfig.largestSize = 0;
        g_ClientConfig.secondLargestSize = 0;
        g_ClientConfig.sampleInterval = 0;
    }

    void TearDown() override {}
};

/**
 * @tc.name: IsReadyTest
 * @tc.desc: Test HookGuard::IsReady function
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, IsReadyTest, TestSize.Level0)
{
    g_hookReady = true;
    EXPECT_TRUE(HookGuard::IsReady());

    g_hookReady = false;
    EXPECT_FALSE(HookGuard::IsReady());
}

/**
 * @tc.name: IsPidChangedTest
 * @tc.desc: Test HookGuard::IsPidChanged function
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, IsPidChangedTest, TestSize.Level0)
{
    g_isPidChanged = false;
    g_hookPid = getpid();
    EXPECT_FALSE(HookGuard::IsPidChanged());

    g_isPidChanged = true;
    EXPECT_TRUE(HookGuard::IsPidChanged());

    g_isPidChanged = false;
    g_hookPid = 99999;
    EXPECT_TRUE(HookGuard::IsPidChanged());
    EXPECT_TRUE(g_isPidChanged.load());
}

/**
 * @tc.name: ShouldSkipMallocTest
 * @tc.desc: Test HookGuard::ShouldSkipMalloc function
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldSkipMallocTest, TestSize.Level0)
{
    g_ClientConfig.mallocDisable = false;
    g_isPidChanged = false;
    g_hookPid = getpid();
    EXPECT_FALSE(HookGuard::ShouldSkipMalloc());

    g_ClientConfig.mallocDisable = true;
    EXPECT_TRUE(HookGuard::ShouldSkipMalloc());

    g_ClientConfig.mallocDisable = false;
    g_isPidChanged = true;
    EXPECT_TRUE(HookGuard::ShouldSkipMalloc());
}

/**
 * @tc.name: ShouldSkipMmapTest
 * @tc.desc: Test HookGuard::ShouldSkipMmap function
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldSkipMmapTest, TestSize.Level0)
{
    g_ClientConfig.mmapDisable = false;
    g_isPidChanged = false;
    g_hookPid = getpid();
    EXPECT_FALSE(HookGuard::ShouldSkipMmap());

    g_ClientConfig.mmapDisable = true;
    EXPECT_TRUE(HookGuard::ShouldSkipMmap());

    g_ClientConfig.mmapDisable = false;
    g_isPidChanged = true;
    EXPECT_TRUE(HookGuard::ShouldSkipMmap());
}

/**
 * @tc.name: ShouldSkipMemtraceTest
 * @tc.desc: Test HookGuard::ShouldSkipMemtrace function
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldSkipMemtraceTest, TestSize.Level0)
{
    g_ClientConfig.memtraceEnable = true;
    g_isPidChanged = false;
    g_hookPid = getpid();
    EXPECT_FALSE(HookGuard::ShouldSkipMemtrace());

    g_ClientConfig.memtraceEnable = false;
    EXPECT_TRUE(HookGuard::ShouldSkipMemtrace());

    g_ClientConfig.memtraceEnable = true;
    g_isPidChanged = true;
    EXPECT_TRUE(HookGuard::ShouldSkipMemtrace());
}

/**
 * @tc.name: ShouldFilterBySizeBasicTest
 * @tc.desc: Test HookGuard::ShouldFilterBySize basic cases
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldFilterBySizeBasicTest, TestSize.Level0)
{
    g_ClientConfig.largestSize = 0;
    g_ClientConfig.secondLargestSize = 0;

    void* testPtr = malloc(256);
    ASSERT_NE(testPtr, nullptr);
    EXPECT_FALSE(HookGuard::ShouldFilterBySize(testPtr, 256));
    free(testPtr);
}

/**
 * @tc.name: ShouldFilterBySizeWithConfigTest
 * @tc.desc: Test HookGuard::ShouldFilterBySize with configurations
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldFilterBySizeWithConfigTest, TestSize.Level0)
{
    g_ClientConfig.largestSize = 1024;
    g_ClientConfig.secondLargestSize = 512;
    g_ClientConfig.sampleInterval = 2048;

    void* testPtr1 = malloc(256);
    ASSERT_NE(testPtr1, nullptr);
    EXPECT_TRUE(HookGuard::ShouldFilterBySize(testPtr1, 256));
    free(testPtr1);

    void* testPtr2 = malloc(4096);
    ASSERT_NE(testPtr2, nullptr);
    EXPECT_FALSE(HookGuard::ShouldFilterBySize(testPtr2, 4096));
    free(testPtr2);
}

/**
 * @tc.name: ShouldSampleBasicTest
 * @tc.desc: Test HookGuard::ShouldSample basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, ShouldSampleBasicTest, TestSize.Level0)
{
    g_ClientConfig.sampleInterval = 0;
    EXPECT_FALSE(HookGuard::ShouldSample(1024));

    g_ClientConfig.sampleInterval = 1;
    EXPECT_FALSE(HookGuard::ShouldSample(1024));
}

/**
 * @tc.name: CalculateRealSizeBasicTest
 * @tc.desc: Test HookGuard::CalculateRealSize basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(HookGuardTest, CalculateRealSizeBasicTest, TestSize.Level0)
{
    g_ClientConfig.fpunwind = true;
    int fpStackDepth = 5;
    EXPECT_EQ(HookGuard::CalculateRealSize(fpStackDepth), 96);

    g_ClientConfig.fpunwind = false;
    EXPECT_EQ(HookGuard::CalculateRealSize(fpStackDepth), 320);
}
}
