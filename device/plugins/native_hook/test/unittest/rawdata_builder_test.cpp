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
#include <unistd.h>
#include "rawdata_builder.h"
#include "hook_common.h"
#include "hook_client.h"

using namespace testing::ext;

namespace {
class RawDataBuilderTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        g_clientConfig.clockId = CLOCK_REALTIME;
    }

    void TearDown() override {}
};

/**
 * @tc.name: FluentAPITest
 * @tc.desc: Test builder fluent API
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, FluentAPITest, TestSize.Level0)
{
    RawDataBuilder builder;
    void* testAddr = reinterpret_cast<void*>(0x1000);

    builder.SetType(MALLOC_MSG)
           .SetAddr(testAddr)
           .SetSize(1024)
           .SetProcessInfo();

    StackRawData data = builder.Build();
    EXPECT_EQ(data.type, MALLOC_MSG);
    EXPECT_EQ(data.addr, testAddr);
    EXPECT_EQ(data.mallocSize, 1024U);
    EXPECT_GT(data.pid, 0U);
    EXPECT_GT(data.tid, 0U);
}

/**
 * @tc.name: SetTypeTest
 * @tc.desc: Test SetType method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetTypeTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetType(FREE_MSG);

    StackRawData data = builder.Build();
    EXPECT_EQ(data.type, FREE_MSG);
}

/**
 * @tc.name: SetAddrTest
 * @tc.desc: Test SetAddr method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetAddrTest, TestSize.Level0)
{
    RawDataBuilder builder;
    void* testAddr = reinterpret_cast<void*>(0x12345678);
    builder.SetAddr(testAddr);

    StackRawData data = builder.Build();
    EXPECT_EQ(data.addr, testAddr);
}

/**
 * @tc.name: SetNewAddrTest
 * @tc.desc: Test SetNewAddr method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetNewAddrTest, TestSize.Level0)
{
    RawDataBuilder builder;
    void* testNewAddr = reinterpret_cast<void*>(0xABCDEF00);
    builder.SetNewAddr(testNewAddr);

    StackRawData data = builder.Build();
    EXPECT_EQ(data.newAddr, testNewAddr);
}

/**
 * @tc.name: SetSizeTest
 * @tc.desc: Test SetSize method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetSizeTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetSize(2048);

    StackRawData data = builder.Build();
    EXPECT_EQ(data.mallocSize, 2048U);
}

/**
 * @tc.name: SetTagIdTest
 * @tc.desc: Test SetTagId method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetTagIdTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetTagId(42);

    StackRawData data = builder.Build();
    EXPECT_EQ(data.tagId, 42U);
}

/**
 * @tc.name: SetProcessInfoTest
 * @tc.desc: Test SetProcessInfo method
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, SetProcessInfoTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetProcessInfo();

    StackRawData data = builder.Build();
    EXPECT_GT(data.pid, 0U);
    EXPECT_GT(data.tid, 0U);
    EXPECT_EQ(data.pid, static_cast<uint32_t>(getpid()));
}

/**
 * @tc.name: TimestampTest
 * @tc.desc: Test timestamp setting
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, TimestampTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetTimestamp();

    StackRawData data = builder.Build();
    EXPECT_GT(data.ts.tv_sec, 0);
}

/**
 * @tc.name: CompleteBuilderTest
 * @tc.desc: Test building complete data structure
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, CompleteBuilderTest, TestSize.Level0)
{
    RawDataBuilder builder;
    void* testAddr = reinterpret_cast<void*>(0x1000);
    void* testNewAddr = reinterpret_cast<void*>(0x2000);

    builder.SetType(MALLOC_MSG)
           .SetAddr(testAddr)
           .SetNewAddr(testNewAddr)
           .SetSize(4096)
           .SetTagId(123)
           .SetProcessInfo()
           .SetTimestamp();

    StackRawData data = builder.Build();

    EXPECT_EQ(data.type, MALLOC_MSG);
    EXPECT_EQ(data.addr, testAddr);
    EXPECT_EQ(data.newAddr, testNewAddr);
    EXPECT_EQ(data.mallocSize, 4096U);
    EXPECT_EQ(data.tagId, 123U);
    EXPECT_GT(data.pid, 0U);
    EXPECT_GT(data.tid, 0U);
    EXPECT_GT(data.ts.tv_sec, 0);
}

/**
 * @tc.name: MultipleBuildsTest
 * @tc.desc: Test building multiple times produces independent results
 * @tc.type: FUNC
 */
HWTEST_F(RawDataBuilderTest, MultipleBuildsTest, TestSize.Level0)
{
    RawDataBuilder builder;
    builder.SetType(MALLOC_MSG).SetSize(1024);

    StackRawData data1 = builder.Build();
    StackRawData data2 = builder.Build();

    EXPECT_EQ(data1.type, data2.type);
    EXPECT_EQ(data1.mallocSize, data2.mallocSize);
}
}
