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
#include <gtest/gtest.h>

#include "hook_socket_client.h"
#include "hook_service.h"
#include "hook_common.h"
#include "service_entry.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "logging.h"
#include "sampling.h"

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
    Sampling sampler;
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
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
#ifdef __aarch64__
HWTEST_F(HookSocketClientTest, SendStack, TestSize.Level1)
{
    uint64_t config = FILTER_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    Sampling sampler;
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(ClientConfig);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    metaSize = sizeof(ts);
    hookClient.unixSocketClient_ = nullptr;
    EXPECT_FALSE(hookClient.SendStack(buffer.get(), metaSize));
    EXPECT_FALSE(hookClient.SendStackWithPayload(buffer.get(), metaSize, buffer.get(), metaSize));
    hookClient.unixSocketClient_ = std::make_shared<UnixSocketClient>();
    EXPECT_FALSE(hookClient.SendStack(buffer.get(), metaSize));
    EXPECT_TRUE(hookClient.SendStackWithPayload(buffer.get(), metaSize, buffer.get(), metaSize));
}
#endif

/*
 * @tc.name: FdListSize
 * @tc.desc: test fd list size with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, FdListSize, TestSize.Level1)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    Sampling sampler;
    auto ptr = reinterpret_cast<const int8_t*>(&clientConfig);
    auto size = sizeof(clientConfig);
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    ASSERT_EQ(hookClient.GetSmbFds().size(), 1);
    ASSERT_EQ(hookClient.GetEventFds().size(), 1);
}

/*
 * @tc.name: GetSmbFds
 * @tc.desc: test HookSocketClient::GetSmbFds with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetSmbFds, TestSize.Level1)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    Sampling sampler;
    auto ptr = reinterpret_cast<const int8_t*>(&clientConfig);
    auto size = sizeof(clientConfig);
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    std::vector<int> smbFds = hookClient.GetSmbFds();
    for (size_t i = 0; i < smbFds.size(); ++i) {
        ASSERT_EQ(smbFds[i], -1);
    }
}

/*
 * @tc.name: GetEventFds
 * @tc.desc: test HookSocketClient::GetEventFds with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetEventFds, TestSize.Level1)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    Sampling sampler;
    auto ptr = reinterpret_cast<const int8_t*>(&clientConfig);
    auto size = sizeof(clientConfig);
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    std::vector<int> eventFds = hookClient.GetEventFds();
    for (size_t i = 0; i < eventFds.size(); ++i) {
        ASSERT_EQ(eventFds[i], -1);
    }
}

/*
 * @tc.name: SendNmdInfo
 * @tc.desc: test HookSocketClient::SendNmdInfo with normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(HookSocketClientTest, SendNmdInfo, TestSize.Level1)
{
    uint64_t config = FILTER_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    Sampling sampler;
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(ClientConfig);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    ASSERT_FALSE(hookClient.SendNmdInfo());
}

/*
 * @tc.name: SendSimplifiedNmdInfo
 * @tc.desc: test HookSocketClient::SendSimplifiedNmdInfo with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, SendSimplifiedNmdInfo, TestSize.Level1)
{
    uint64_t config = FILTER_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    Sampling sampler;
    HookSocketClient hookClient(1, &g_ClientConfigTest, &sampler, nullptr);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(ClientConfig);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    ASSERT_FALSE(hookClient.SendSimplifiedNmdInfo());
}
#endif
} // namespace
