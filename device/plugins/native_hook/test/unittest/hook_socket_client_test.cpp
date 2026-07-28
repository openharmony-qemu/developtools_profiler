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
static ClientConfig g_ClientConfigTest = {0};
constexpr unsigned int WAIT_THREAD_TIME = 5;

ClientConfigIpc MakeClientConfigIpc(const ClientConfig& config)
{
    ClientConfigIpc configIpc;
    EXPECT_TRUE(ConvertClientConfigToIpc(config, configIpc));
    return configIpc;
}
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
HWTEST_F(HookSocketClientTest, ProtocolProc, TestSize.Level0)
{
    ClientConfig config;
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(uint64_t);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    sleep(WAIT_THREAD_TIME);
}

/*
 * @tc.name: ProtocolProcWithNullBuffer
 * @tc.desc: test HookSocketClient::ProtocolProc returns false with null buffer.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, ProtocolProcWithNullBuffer, TestSize.Level0)
{
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    SocketContext socketContext;
    ASSERT_FALSE(hookClient.ProtocolProc(socketContext, 0, nullptr, sizeof(ClientConfigIpc)));
}

/*
 * @tc.name: ClientConfigIpcRoundTrip
 * @tc.desc: test ClientConfigIpc keeps long target so name and filter libs.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, ClientConfigIpcRoundTrip, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 4096;
    config.targetSoName = "lib" + std::string(64, 'a') + ".so, libtarget.so";
    config.filterLibList = {"libfilter_first.so", "lib" + std::string(64, 'b') + ".so"};
    ClientConfigIpc configIpc;
    ASSERT_TRUE(ConvertClientConfigToIpc(config, configIpc));
    ClientConfig output;
    ConvertClientConfigFromIpc(configIpc, output);
    EXPECT_EQ(output.shareMemorySize, config.shareMemorySize);
    EXPECT_EQ(output.targetSoName, config.targetSoName);
    EXPECT_EQ(output.filterLibList, config.filterLibList);
}

/*
 * @tc.name: ProtocolProcWithTargetSoAndNullRange
 * @tc.desc: test HookSocketClient::ProtocolProc returns false when target range is null.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, ProtocolProcWithTargetSoAndNullRange, TestSize.Level0)
{
    ClientConfig config;
    config.targetSoName = "lib" + std::string(64, 'a') + ".so";
    ClientConfigIpc configIpc = MakeClientConfigIpc(config);
    SocketContext socketContext;
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    ASSERT_FALSE(hookClient.ProtocolProc(socketContext, 0, ptr, sizeof(configIpc)));
}
/*
 * @tc.name: SendStack
 * @tc.desc: test HookSocketClient::SendStack with normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(HookSocketClientTest, SendStack, TestSize.Level0)
{
    ClientConfig config;
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(config);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
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
    sleep(WAIT_THREAD_TIME);
}
#endif

/*
 * @tc.name: FdListSize
 * @tc.desc: test fd list size with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, FdListSize, TestSize.Level0)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(clientConfig);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    ASSERT_EQ(hookClient.GetSmbFds().size(), 1);
    ASSERT_EQ(hookClient.GetEventFds().size(), 1);
    sleep(WAIT_THREAD_TIME);
}

/*
 * @tc.name: GetSmbFds
 * @tc.desc: test HookSocketClient::GetSmbFds with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetSmbFds, TestSize.Level0)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(clientConfig);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    std::vector<int> smbFds = hookClient.GetSmbFds();
    for (size_t i = 0; i < smbFds.size(); ++i) {
        ASSERT_EQ(smbFds[i], -1);
    }
    sleep(WAIT_THREAD_TIME);
}

/*
 * @tc.name: GetEventFds
 * @tc.desc: test HookSocketClient::GetEventFds with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, GetEventFds, TestSize.Level0)
{
    ClientConfig clientConfig;
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(clientConfig);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));
    std::vector<int> eventFds = hookClient.GetEventFds();
    for (size_t i = 0; i < eventFds.size(); ++i) {
        ASSERT_EQ(eventFds[i], -1);
    }
    sleep(WAIT_THREAD_TIME);
}

/*
 * @tc.name: SendNmdInfo
 * @tc.desc: test HookSocketClient::SendNmdInfo with normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(HookSocketClientTest, SendNmdInfo, TestSize.Level0)
{
    ClientConfig config;
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(config);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    ASSERT_FALSE(hookClient.SendNmdInfo());
    sleep(WAIT_THREAD_TIME);
}

/*
 * @tc.name: SendSimplifiedNmdInfo
 * @tc.desc: test HookSocketClient::SendSimplifiedNmdInfo with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, SendSimplifiedNmdInfo, TestSize.Level0)
{
    ClientConfig config;
    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr);
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(config);
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    auto size = sizeof(configIpc);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, size));

    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t metaSize = sizeof(ts);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(metaSize);
    if (memcpy_s(buffer.get(), metaSize, &ts, sizeof(ts)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s ts failed");
    }
    ASSERT_FALSE(hookClient.SendSimplifiedNmdInfo());
    sleep(WAIT_THREAD_TIME);
}
#endif

/*
 * @tc.name: DisableHookSkipInMmapMunmap
 * @tc.desc: test DisableHook skips when g_inMmapMunmap is true, callback not invoked.
 * @tc.type: FUNC
 */
HWTEST_F(HookSocketClientTest, DisableHookSkipInMmapMunmap, TestSize.Level0)
{
    static int callbackCount = 0;
    callbackCount = 0;
    auto callback = []() { callbackCount++; };

    HookSocketClient hookClient(1, &g_ClientConfigTest, nullptr, nullptr, callback);
    SocketContext socketContext;
    ClientConfigIpc configIpc = MakeClientConfigIpc(ClientConfig{});
    auto ptr = reinterpret_cast<const int8_t*>(&configIpc);
    ASSERT_TRUE(hookClient.ProtocolProc(socketContext, 0, ptr, sizeof(configIpc)));

    g_inMmapMunmap = true;
    hookClient.DisableHook();
    g_inMmapMunmap = false;
    EXPECT_EQ(callbackCount, 0);

    hookClient.DisableHook();
    EXPECT_EQ(callbackCount, 1);
}

} // namespace
