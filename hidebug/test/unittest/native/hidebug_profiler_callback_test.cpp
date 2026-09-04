/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include <atomic>
#include <mutex>
#include <string>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
struct CallbackContext {
    std::mutex mtx;
    std::atomic<bool> called{false};
    OH_HiDebug_ResourceType resourceType{OH_RES_TYPE_FD};
    std::string filePath;
};

static CallbackContext g_ctx;

static void ResetContext()
{
    g_ctx.called = false;
    g_ctx.resourceType = OH_RES_TYPE_FD;
    g_ctx.filePath.clear();
}

static void ProfilingCallback(OH_HiDebug_ProfilingResult* result)
{
    if (result == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_ctx.mtx);
    g_ctx.resourceType = result->resourceType;
    if (result->filePath != nullptr) {
        g_ctx.filePath = result->filePath;
    }
    g_ctx.called = true;
}

static OH_HiDebug_ResProfilerConfig MakeDefaultConfig()
{
    OH_HiDebug_ResProfilerConfig config = {};
    config.maxDuration = 10;       // 10 seconds
    config.filterSize = 256;       // 256 bytes
    config.maxStackDepth = 30;     // 30 frames
    config.sampleInterval = 384;   // 384 bytes
    config.statisticsInterval = 5; // 5 seconds
    return config;
}

class HidebugProfilerCallbackTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ResetContext();
    }
    void TearDown() override
    {
        OH_HiDebug_StopProfiler();
        ResetContext();
    }
};

/**
 * @tc.name: NullCallbackTest
 * @tc.desc: StartProfiler with null callback should not crash.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, NullCallbackTest, TestSize.Level0)
{
    auto config = MakeDefaultConfig();
    HiDebug_ErrorCode ret = OH_HiDebug_StartProfiler(OH_RES_TYPE_NATIVE, &config, nullptr);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_FALSE(g_ctx.called.load());
}

/**
 * @tc.name: NullConfigTest
 * @tc.desc: StartProfiler with null config should not invoke callback.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, NullConfigTest, TestSize.Level0)
{
    HiDebug_ErrorCode ret = OH_HiDebug_StartProfiler(OH_RES_TYPE_NATIVE, nullptr, ProfilingCallback);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_FALSE(g_ctx.called.load());
    EXPECT_TRUE(g_ctx.filePath.empty());
}

/**
 * @tc.name: InvalidTypeStartTest
 * @tc.desc: StartProfiler with invalid type should not invoke callback.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, InvalidTypeStartTest, TestSize.Level0)
{
    auto config = MakeDefaultConfig();
    HiDebug_ErrorCode ret =
        OH_HiDebug_StartProfiler(static_cast<OH_HiDebug_ResourceType>(-1), &config, ProfilingCallback);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_RESOURCE_TYPE);
    EXPECT_FALSE(g_ctx.called.load());
    EXPECT_TRUE(g_ctx.filePath.empty());
}

/**
 * @tc.name: ProfilerOptionsCreateDestroyTest
 * @tc.desc: CreateProfilerOptions returns non-null, Destroy releases without crash.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsCreateDestroyTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    EXPECT_NE(opts, nullptr);
    OH_HiDebug_DestroyProfilerOptions(opts);
}

/**
 * @tc.name: ProfilerOptionsNullOptsTest
 * @tc.desc: StartProfilerWithOptions with null opts should return INVALID_ARG.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsNullOptsTest, TestSize.Level0)
{
    HiDebug_ErrorCode ret = OH_HiDebug_StartProfilerWithOptions(OH_RES_TYPE_NATIVE, nullptr, ProfilingCallback);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_FALSE(g_ctx.called.load());
}

/**
 * @tc.name: ProfilerOptionsNullCallbackTest
 * @tc.desc: StartProfilerWithOptions with null callback should return INVALID_ARG.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsNullCallbackTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    ASSERT_NE(opts, nullptr);
    HiDebug_ErrorCode ret = OH_HiDebug_StartProfilerWithOptions(OH_RES_TYPE_NATIVE, opts, nullptr);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_FALSE(g_ctx.called.load());
    OH_HiDebug_DestroyProfilerOptions(opts);
}

/**
 * @tc.name: ProfilerOptionsInvalidTypeTest
 * @tc.desc: StartProfilerWithOptions with invalid type returns INVALID_RESOURCE_TYPE.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsInvalidTypeTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    ASSERT_NE(opts, nullptr);
    OH_HiDebug_SetMaxDurationSec(opts, 10);
    OH_HiDebug_SetFilterSize(opts, 256);
    OH_HiDebug_SetMaxStackDepth(opts, 30);
    OH_HiDebug_SetSampleIntervalBytes(opts, 384);
    OH_HiDebug_SetStatisticsIntervalSec(opts, 5);
    HiDebug_ErrorCode ret =
        OH_HiDebug_StartProfilerWithOptions(static_cast<OH_HiDebug_ResourceType>(-1), opts, ProfilingCallback);
    EXPECT_EQ(ret, HIDEBUG_RES_PROF_INVALID_RESOURCE_TYPE);
    EXPECT_FALSE(g_ctx.called.load());
    OH_HiDebug_DestroyProfilerOptions(opts);
}

/**
 * @tc.name: ProfilerOptionsSetterNullSafeTest
 * @tc.desc: All setters with null opts should return HIDEBUG_RES_PROF_INVALID_ARG.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsSetterNullSafeTest, TestSize.Level0)
{
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(nullptr, 10), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetFilterSize(nullptr, 256), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(nullptr, 30), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(nullptr, 5), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetSampleIntervalBytes(nullptr, 384), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(nullptr, 1), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(nullptr, 16), HIDEBUG_RES_PROF_INVALID_ARG);
    EXPECT_EQ(OH_HiDebug_DestroyProfilerOptions(nullptr), HIDEBUG_RES_PROF_INVALID_ARG);
}

/**
 * @tc.name: ProfilerOptionsSetterInvalidValueTest
 * @tc.desc: Setters with out-of-range values should return corresponding RES_PROF error code.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsSetterInvalidValueTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    ASSERT_NE(opts, nullptr);
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(opts, 0), HIDEBUG_RES_PROF_INVALID_MAX_DURATION);
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(opts, 3601), HIDEBUG_RES_PROF_INVALID_MAX_DURATION);
    EXPECT_EQ(OH_HiDebug_SetFilterSize(opts, 0), HIDEBUG_RES_PROF_INVALID_FILTER_SIZE);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(opts, 0), HIDEBUG_RES_PROF_INVALID_MAX_STACK_DEPTH);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(opts, 31), HIDEBUG_RES_PROF_INVALID_MAX_STACK_DEPTH);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(opts, 0), HIDEBUG_RES_PROF_INVALID_STATISTICS_INTERVAL);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(opts, 3601), HIDEBUG_RES_PROF_INVALID_STATISTICS_INTERVAL);
    EXPECT_EQ(OH_HiDebug_SetSampleIntervalBytes(opts, 383), HIDEBUG_RES_PROF_INVALID_SAMPLE_INTERVAL);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(opts, 0), HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_NESTING_DEPTH);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(opts, 17), HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_NESTING_DEPTH);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(opts, 0), HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_TASK_STACK_DEPTH);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(opts, 257), HIDEBUG_RES_PROF_INVALID_MAX_ASYNC_TASK_STACK_DEPTH);
    OH_HiDebug_DestroyProfilerOptions(opts);
}

/**
 * @tc.name: ProfilerOptionsSetterValidValueTest
 * @tc.desc: Setters with valid values should return HIDEBUG_SUCCESS.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsSetterValidValueTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    ASSERT_NE(opts, nullptr);
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(opts, 10), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetFilterSize(opts, 256), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(opts, 30), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(opts, 5), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetSampleIntervalBytes(opts, 384), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(opts, 16), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_DestroyProfilerOptions(opts), HIDEBUG_SUCCESS);
}

/**
 * @tc.name: ProfilerOptionsSetterBoundaryValueTest
 * @tc.desc: Setters with valid boundary values should return HIDEBUG_SUCCESS.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugProfilerCallbackTest, ProfilerOptionsSetterBoundaryValueTest, TestSize.Level0)
{
    OH_HiDebug_ProfilerOptions* opts = OH_HiDebug_CreateProfilerOptions();
    ASSERT_NE(opts, nullptr);
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxDurationSec(opts, 3600), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetFilterSize(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxStackDepth(opts, 30), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetStatisticsIntervalSec(opts, 3600), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetSampleIntervalBytes(opts, 384), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncNestingDepth(opts, 16), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(opts, 1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_SetMaxAsyncTaskStackDepth(opts, 256), HIDEBUG_SUCCESS);
    OH_HiDebug_DestroyProfilerOptions(opts);
}

} // namespace HiviewDFX
} // namespace OHOS
