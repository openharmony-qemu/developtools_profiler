/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include <string>

#include <gtest/gtest.h>

#include "ipc_unix_socket.h"

using namespace testing::ext;

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class IpcUnicSocketTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}

    const std::string TEST_PATH_NAME = "/data/test/unix_socket_hiebpf";
};

/**
 * @tc.name: ServerStartStop
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(IpcUnicSocketTest, ServerStartStop, TestSize.Level1)
{
    IpcUnixSocketServer server;
    ASSERT_TRUE(server.Start(TEST_PATH_NAME));
    sleep(2);
    ASSERT_TRUE(server.Stop());
}

HWTEST_F(IpcUnicSocketTest, ClientConnectFailed, TestSize.Level1)
{
    IpcUnixSocketClient client;
    ASSERT_FALSE(client.Connect(TEST_PATH_NAME));
}

HWTEST_F(IpcUnicSocketTest, ServerClientCommunicate, TestSize.Level1)
{
    IpcUnixSocketServer server;
    ASSERT_TRUE(server.Start(TEST_PATH_NAME));

    IpcUnixSocketClient client;
    ASSERT_TRUE(client.Connect(TEST_PATH_NAME));
    std::string cmd = "test command";
    ASSERT_TRUE(client.SendMessage(cmd.data(), cmd.size()));
    const int bufSize = 1024;
    std::string buf;
    size_t size = bufSize;
    buf.resize(size);
    const uint32_t timeout = 1000;
    ASSERT_TRUE(client.RecvMessage(buf.data(), size, timeout));
    ASSERT_EQ(size, 0);

    size = bufSize;
    server.SetHandleCallback([&](const void *buf, size_t size) {
        server.SendMessage(buf, size);
        });
    ASSERT_TRUE(client.SendMessage(cmd.data(), cmd.size()));
    ASSERT_TRUE(client.RecvMessage(buf.data(), size, timeout));
    buf.resize(size);
    ASSERT_EQ(buf, cmd);

    client.Disconnect();
    ASSERT_TRUE(server.Stop());
}

HWTEST_F(IpcUnicSocketTest, ServerClientException, TestSize.Level1)
{
    IpcUnixSocketServer server;
    std::string cmd = "test command";
    ASSERT_FALSE(server.SendMessage(cmd.data(), cmd.size()));

    ASSERT_TRUE(server.Start(TEST_PATH_NAME));
    ASSERT_FALSE(server.Start(TEST_PATH_NAME));

    IpcUnixSocketClient client;
    ASSERT_FALSE(client.SendMessage(cmd.data(), cmd.size()));
    const uint32_t timeout = 1000;
    const int bufSize = 1024;
    size_t size = bufSize;
    std::string buf;
    buf.resize(size);
    ASSERT_FALSE(client.RecvMessage(buf.data(), size, timeout));
    ASSERT_TRUE(client.Connect(TEST_PATH_NAME));
    ASSERT_FALSE(client.Connect(TEST_PATH_NAME));
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
