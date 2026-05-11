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

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "service_entry.h"
#include "socket_context.h"

using namespace testing::ext;

namespace {
class TestSocketContext : public SocketContext {
public:
    void SetSocketHandle(int fd)
    {
        socketHandle_ = fd;
    }
};

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


/**
 * @tc.name: SendHookConfigLargePayload
 * @tc.desc: Socket hook config should send complete head and payload.
 * @tc.type: FUNC
 */
HWTEST_F(SocketContextTest, SendHookConfigLargePayload, TestSize.Level1)
{
    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext socketContext;
    socketContext.SetSocketHandle(sockets[0]);
    std::vector<uint8_t> data(8192, 0x5A);
    ASSERT_TRUE(socketContext.SendHookConfig(data.data(), data.size()));

    ProtocolHead head = {};
    ASSERT_TRUE(SocketContext::ReceiveData(sockets[1], reinterpret_cast<uint8_t*>(&head), sizeof(head)));
    EXPECT_EQ(head.protoType, PROTOCOL_TYPE_PROTOBUF);
    EXPECT_EQ(head.protoSize, static_cast<uint32_t>(data.size() + sizeof(ProtocolHead)));
    std::vector<uint8_t> received(data.size(), 0);
    ASSERT_TRUE(SocketContext::ReceiveData(sockets[1], received.data(), received.size()));
    EXPECT_EQ(received, data);
    close(sockets[1]);
}
} // namespace
