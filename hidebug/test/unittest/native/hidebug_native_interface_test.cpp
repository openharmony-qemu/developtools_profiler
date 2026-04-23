/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
#include <unistd.h>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "collect_result.h"
#include "hidebug_util.h"
#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "hidebug_native_interface.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
class HidebugNativeInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override
    {
        system("param set hiviewdfx.debugenv.hidebug_test 0");
        system("param set libc.hook_mode 0");
    }
};

/**
 * @tc.name: IsDebuggerConnectedTest
 * @tc.desc: test IsDebuggerConnectedTest.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, IsDebuggerConnectedTest, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    ASSERT_FALSE(interface.IsDebuggerConnected());
}

/**
 * @tc.name: GetVssTest
 * @tc.desc: test GetVssTest. get vss memory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetVssTest, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    auto vssInfo = interface.GetVss();
    ASSERT_TRUE(vssInfo);
    ASSERT_GE(vssInfo.value(), 0);
}

/**
 * @tc.name: GetRssInfoTest
 * @tc.desc: test GetRssInfoTest. get rss information.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetRssInfoTest, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    auto rssInfo = interface.GetRssInfo();
    ASSERT_TRUE(rssInfo.has_value());
    ASSERT_GT(rssInfo->rss, 0u);
    ASSERT_GE(rssInfo->swapRss, 0);
}

#ifdef __aarch64__
/**
 * @tc.name: OH_HiDebug_GetGraphicsMemory
 * @tc.desc: test OH_HiDebug_GetRealNanoSecondsTimestamp. get graphics memory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetMemoryLeakResourceTest, TestSize.Level1)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    ASSERT_EQ(interface.GetMemoryLeakResource("test", 10, false), NATIVE_SUCCESS);
}
#endif

/**
 * @tc.name: RequestTraceTest
 * @tc.desc: test RequestTrace.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, RequestTraceTest, TestSize.Level1)
{
    auto task = [](TraceErrorCode retCode, const std::string& tracePath) {
        EXPECT_EQ(retCode, TRACE_ABNORMAL);
        EXPECT_TRUE(tracePath.empty());
    };
    auto& interface = HidebugNativeInterface::GetInstance();
    std::string targetPath = "/data/storage/el2/log/";
    if (access(targetPath.c_str(), F_OK) == 0) {
        ASSERT_EQ(interface.RequestTrace("", 0, 0, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("", 1024, 1000, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("", 15 * 1024, 15 * 1000, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("", 15 * 1024 + 1, 15 * 1000 + 1, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("tracetest", 1024, 1000, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("trace12345_TRACE1234/trace", 1024, 1000, task), TRACE_SUCCESS);
        ASSERT_EQ(interface.RequestTrace("trace12345/TRACE1234trace", 1024, 1000, task), TRACE_SUCCESS);

        sleep(1);
        interface.SetTraceErrorCode(UCollect::UcError::TRACE_OVER_FLOW);
        ASSERT_EQ(interface.RequestTrace("tracetest", 1024, 1000, task), RESOURCE_UNAVAILABLE);
        interface.SetTraceErrorCode(0);
    } else {
        ASSERT_EQ(interface.RequestTrace("", 0, 0, task), TRACE_ABNORMAL);
    }
}

/**
 * @tc.name: ParseProcStatTest_Success
 * @tc.desc: test ParseProcStat success, utime and stime should be valid.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, ParseProcStatTest_Success, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    uint64_t utime = 0;
    uint64_t stime = 0;
    bool ret = interface.ParseProcStat(utime, stime);
    ASSERT_TRUE(ret);
    ASSERT_GE(utime + stime, 0ULL);
}

/**
 * @tc.name: ParseProcStatTest_ValueIncrease
 * @tc.desc: test ParseProcStat twice, cpu ticks should be non-decreasing after cpu work.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, ParseProcStatTest_ValueIncrease, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    uint64_t utime1 = 0, stime1 = 0;
    uint64_t utime2 = 0, stime2 = 0;
    ASSERT_TRUE(interface.ParseProcStat(utime1, stime1));

    volatile uint64_t x = 0;
    for (int i = 0; i < 100000000; i++) {
        x += i;
    }

    ASSERT_TRUE(interface.ParseProcStat(utime2, stime2));
    ASSERT_GE(utime2 + stime2, utime1 + stime1);
}

/**
 * @tc.name: GetCpuUsageTest_ReturnNonNegative
 * @tc.desc: test GetCpuUsage return value should be non-negative.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetCpuUsageTest_ReturnNonNegative, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    double usage = interface.GetCpuUsage();
    ASSERT_GE(usage, 0.0);
}

/**
 * @tc.name: GetCpuUsageTest_SecondCallReturnsCached
 * @tc.desc: test GetCpuUsage second call returns cached value without recalculation.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetCpuUsageTest_SecondCallReturnsCached, TestSize.Level0)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    double first  = interface.GetCpuUsage();
    double second = interface.GetCpuUsage();
    ASSERT_DOUBLE_EQ(first, second);
}
}
}
