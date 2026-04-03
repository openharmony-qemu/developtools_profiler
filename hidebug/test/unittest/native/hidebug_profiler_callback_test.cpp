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

} // namespace HiviewDFX
} // namespace OHOS
