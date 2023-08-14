/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "socket_context.h"

using namespace testing::ext;

namespace {
class SocketContextTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Service
 * @tc.desc: Socket send file descriptor test.
 * @tc.type: FUNC
 */
HWTEST_F(SocketContextTest, SendFileDescriptor, TestSize.Level1)
{
    SocketContext socketContext;
    ASSERT_TRUE(!socketContext.SendFileDescriptor(-1));
    ASSERT_TRUE(!socketContext.SendFileDescriptor(1));
}

/**
 * @tc.name: Service
 * @tc.desc: Abnormal socket detection.
 * @tc.type: FUNC
 */
HWTEST_F(SocketContextTest, RawProtocolProc, TestSize.Level1)
{
    std::string s="abc";
    SocketContext socketContext;
    std::vector<unsigned char> buf(64); // 64: the buf size
    ASSERT_EQ(socketContext.RawProtocolProc(2, (const int8_t *)s.c_str(), s.size()), -1);
    EXPECT_FALSE(socketContext.ReceiveData(-1, buf.data(), buf.size()));
    EXPECT_FALSE(socketContext.ReceiveData(1, nullptr, 0));
    EXPECT_FALSE(socketContext.SendRaw(1, (const int8_t *)s.c_str(), s.size(), -1));
}

/**
 * @tc.name: Service
 * @tc.desc: Socket receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(SocketContextTest, ReceiveFileDiscriptor, TestSize.Level1)
{
    SocketContext socketContext;
    ASSERT_EQ(socketContext.ReceiveFileDiscriptor(), -1);
}
/**
 * @tc.name: Service
 * @tc.desc: Socket send heart beat test.
 * @tc.type: FUNC
 */
HWTEST_F(SocketContextTest, SendHeartBeat, TestSize.Level1)
{
    SocketContext socketContext;
    EXPECT_FALSE(socketContext.SendHeartBeat());
    EXPECT_FALSE(socketContext.SendHookConfig(nullptr, 0));
}

} // namespace