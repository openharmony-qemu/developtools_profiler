/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include <dlfcn.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "hook_socket_client.h"
#include "hook_service.h"
#include "service_entry.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "logging.h"

using namespace testing::ext;

namespace {
constexpr int MOBILE_BIT = 32;
constexpr int32_t FILTER_SIZE = 100;
constexpr int32_t SMB_SIZE = 409600;
static ClientConfig g_ClientConfigTest = {0};

class HookSocketClientTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: ProtocolProc
 * @tc.desc: test HookSocketClient::ProtocolProc with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, ProtocolProc, TestSize.Level1)
{
    uint64_t config = FILTER_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    HookSocketClient hookClient(1, &g_ClientConfigTest);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(uint64_t);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
}

/*
 * @tc.name: SendStack
 * @tc.desc: test HookSocketClient::SendStack with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, SendStack, TestSize.Level1)
{
    ClientConfig config = {0};
    config.shareMemroySize = SMB_SIZE;
    HookSocketClient hookClient(1, &g_ClientConfigTest);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(ClientConfig);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        HILOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    metaSize = sizeof(ts);
    EXPECT_FALSE(hookClient.SendStack(buffer.get(), metaSize));
    EXPECT_FALSE(hookClient.SendStackWithPayload(buffer.get(), metaSize, buffer.get(), metaSize));
    hookClient.unixSocketClient_ = std::make_shared<UnixSocketClient>();
    EXPECT_FALSE(hookClient.SendStack(buffer.get(), metaSize));
    EXPECT_TRUE(hookClient.SendStackWithPayload(buffer.get(), metaSize, buffer.get(), metaSize));
}

/*
 * @tc.name: GetSmbFd
 * @tc.desc: test HookSocketClient::GetSmbFd with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetSmbFd, TestSize.Level1)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&clientConfig);
    auto size = sizeof(clientConfig);
    HookSocketClient hookClient(1, &g_ClientConfigTest);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    ASSERT_EQ(hookClient.GetSmbFd(), -1);
}

/*
 * @tc.name: GetEventFd
 * @tc.desc: test HookSocketClient::GetEventFd with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetEventFd, TestSize.Level1)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&clientConfig);
    auto size = sizeof(clientConfig);
    HookSocketClient hookClient(1, &g_ClientConfigTest);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    ASSERT_EQ(hookClient.GetEventFd(), -1);
}
} // namespace
