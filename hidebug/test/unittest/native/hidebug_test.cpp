/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include <filesystem>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "hidebug_base.h"
#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"

using namespace testing::ext;

namespace {
class HidebugTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override
    {
        system("param set hiviewdfx.debugenv.hidebug_test 0");
        system("param set libc.hook_mode 0");
    }
};

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam1, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = "hidebug_test";
    EXPECT_TRUE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for input is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam2, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = nullptr;
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for input is wrong
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam3, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = "hidebug_test/";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

#ifdef HIDEBUG_BUILD_VARIANT_ROOT
/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for param set wrong
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam4, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam5, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    system("param set libc.hook_mode startup:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_TRUE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode param set fail
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam6, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode fail
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam7, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode input error
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam8, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "error_input";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode param set wrong_proc
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam9, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode start_up:wrong_proc");
    const char* inputName = "hidebug";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}
#endif

/**
 * @tc.name: OH_HiDebug_GetAppCpuUsage1
 * @tc.desc: test OH_HiDebug_GetAppCpuUsage.get app cpu usage
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_GetAppCpuUsage1, TestSize.Level1)
{
    EXPECT_TRUE(OH_HiDebug_GetAppCpuUsage() >= 0);
}

/**
 * @tc.name: OH_HiDebug_GetAppThreadCpuUsage1
 * @tc.desc: test OH_HiDebug_GetAppThreadCpuUsage.get thread cpu usage of app
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_GetAppThreadCpuUsage1, TestSize.Level1)
{
    HiDebug_ThreadCpuUsagePtr threadCpuUsage = OH_HiDebug_GetAppThreadCpuUsage();
    HiDebug_ThreadCpuUsagePtr curThreadCpuUsage = threadCpuUsage;
    while (curThreadCpuUsage != nullptr) {
        curThreadCpuUsage = curThreadCpuUsage->next;
    }
    OH_HiDebug_FreeThreadCpuUsage(&threadCpuUsage);
    HiDebug_ThreadCpuUsagePtr threadCpuUsage1 = nullptr;
    OH_HiDebug_FreeThreadCpuUsage(&threadCpuUsage1);
    OH_HiDebug_FreeThreadCpuUsage(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetSystemCpuUsage
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode param set wrong_proc
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, GetSystemCpuUsage, TestSize.Level1)
{
    double systemCpuUsage = OH_HiDebug_GetSystemCpuUsage();
    ASSERT_GE(systemCpuUsage, 0);
    ASSERT_LE(systemCpuUsage, 1);
}

/**
 * @tc.name: GetAppMemoryLimit1
 * @tc.desc: test GetAppMemoryLimit1
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, GetAppMemoryLimit1, TestSize.Level1)
{
    OH_HiDebug_GetAppMemoryLimit(nullptr);
    HiDebug_MemoryLimit memoryLimit;
    OH_HiDebug_GetAppMemoryLimit(&memoryLimit);
    ASSERT_GE(memoryLimit.rssLimit, 0);
    ASSERT_GE(memoryLimit.vssLimit, 0);
}

/**
 * @tc.name: OH_HiDebug_GetAppNativeMemInfo1
 * @tc.desc: test OH_HiDebug_GetAppNativeMemInfo. get application process memory info
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_GetAppNativeMemInfo1, TestSize.Level1)
{
    HiDebug_NativeMemInfo nativeMemInfo;
    OH_HiDebug_GetAppNativeMemInfo(&nativeMemInfo);
    ASSERT_TRUE(nativeMemInfo.pss >= 0);
    ASSERT_TRUE(nativeMemInfo.vss >= 0);
    ASSERT_TRUE(nativeMemInfo.rss >= 0);
    ASSERT_TRUE(nativeMemInfo.sharedDirty >= 0);
    ASSERT_TRUE(nativeMemInfo.privateDirty >= 0);
    ASSERT_TRUE(nativeMemInfo.sharedClean >= 0);
    ASSERT_TRUE(nativeMemInfo.privateClean >= 0);
}

/**
 * @tc.name: OH_HiDebug_GetSystemMemInfo1
 * @tc.desc: test OH_HiDebug_GetSystemMemInfo. get system memory info
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_GetSystemMemInfo1, TestSize.Level1)
{
    OH_HiDebug_GetSystemMemInfo(nullptr);
    HiDebug_SystemMemInfo systemMemInfo;
    OH_HiDebug_GetSystemMemInfo(&systemMemInfo);
    ASSERT_GE(systemMemInfo.totalMem, 0);
    ASSERT_GE(systemMemInfo.freeMem, 0);
    ASSERT_GE(systemMemInfo.availableMem, 0);
}

/**
 * @tc.name: OH_HiDebug_StartAppTraceCapture1
 * @tc.desc: test OH_HiDebug_StartAppTraceCapture. start app capture trace
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_StartAppTraceCapture1, TestSize.Level1)
{
    uint32_t fileLength = 256;
    char fileName[256] = {0};
    HiDebug_TraceFlag flag = HIDEBUG_TRACE_FLAG_MAIN_THREAD;
    uint64_t tags = HIDEBUG_TRACE_TAG_COMMON_LIBRARY;
    uint32_t limitSize = 1024 * 1024;
    const char* targetPath = "/data/storage/el2/log";
    auto captureResult = OH_HiDebug_StartAppTraceCapture(flag, tags, limitSize, fileName, fileLength);
    if (std::filesystem::exists(targetPath)) {
        EXPECT_EQ(captureResult, HIDEBUG_SUCCESS);
        EXPECT_GT(sizeof(fileName) / sizeof(fileName[0]), 1);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_SUCCESS);
    } else {
        EXPECT_EQ(captureResult, HIDEBUG_NO_PERMISSION);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_NO_TRACE_RUNNING);
    }
}

/**
 * @tc.name: OH_HiDebug_StartAppTraceCapture2
 * @tc.desc: test OH_HiDebug_StartAppTraceCapture. repeat start app capture trace
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_StartAppTraceCapture2, TestSize.Level1)
{
    uint32_t fileLength = 256;
    char fileName[256] = {0};
    HiDebug_TraceFlag flag = HIDEBUG_TRACE_FLAG_MAIN_THREAD;
    uint64_t tags = HIDEBUG_TRACE_TAG_COMMON_LIBRARY;
    uint32_t limitSize = 1024 * 1024;
    const char* targetPath = "/data/storage/el2/log";
    auto captureResult = OH_HiDebug_StartAppTraceCapture(flag, tags, limitSize, fileName, fileLength);
    if (std::filesystem::exists(targetPath)) {
        EXPECT_EQ(captureResult, HIDEBUG_SUCCESS);
        auto captureResult2 = OH_HiDebug_StartAppTraceCapture(flag, tags, limitSize, fileName, fileLength);
        EXPECT_EQ(captureResult2, HIDEBUG_TRACE_CAPTURED_ALREADY);
        EXPECT_GT(sizeof(fileName) / sizeof(fileName[0]), 1);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_SUCCESS);
    } else {
        EXPECT_EQ(captureResult, HIDEBUG_NO_PERMISSION);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_NO_TRACE_RUNNING);
    }
}

/**
 * @tc.name: OH_HiDebug_StartAppTraceCapture3
 * @tc.desc: test OH_HiDebug_StartAppTraceCapture. repeat stop app capture trace
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_StartAppTraceCapture3, TestSize.Level1)
{
    uint32_t fileLength = 256;
    char fileName[256] = {0};
    HiDebug_TraceFlag flag = HIDEBUG_TRACE_FLAG_MAIN_THREAD;
    uint64_t tags = HIDEBUG_TRACE_TAG_COMMON_LIBRARY;
    uint32_t limitSize = 1024 * 1024;
    const char* targetPath = "/data/storage/el2/log";
    auto captureResult = OH_HiDebug_StartAppTraceCapture(flag, tags, limitSize, fileName, fileLength);
    if (std::filesystem::exists(targetPath)) {
        EXPECT_EQ(captureResult, HIDEBUG_SUCCESS);
        EXPECT_GT(sizeof(fileName) / sizeof(fileName[0]), 1);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_SUCCESS);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_NO_TRACE_RUNNING);
    } else {
        EXPECT_EQ(captureResult, HIDEBUG_NO_PERMISSION);
        EXPECT_EQ(OH_HiDebug_StopAppTraceCapture(), HIDEBUG_NO_TRACE_RUNNING);
    }
}

/**
 * @tc.name: OH_HiDebug_GetGraphicsMemory
 * @tc.desc: test OH_HiDebug_GetGraphicsMemory. get graphics memory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, OH_HiDebug_GetGraphicsMemory, TestSize.Level1)
{
    uint32_t value = 0;
    EXPECT_EQ(OH_HiDebug_GetGraphicsMemory(&value), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_GetGraphicsMemory(nullptr), HIDEBUG_INVALID_ARGUMENT);
    EXPECT_GE(value, 0);
}
} // namespace
