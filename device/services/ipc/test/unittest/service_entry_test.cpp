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
#include "client_connection.h"
#include "plugin_service.ipc.h"
#include "service_entry.h"
#include "unix_socket_server.h"

using namespace testing::ext;
namespace {
class ServiceEntryTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

namespace {
const int MS_PER_S = 1000;
const int US_PER_S = 1000000;
const int NS_PER_US = 1000;
const int NS_PER_S = 1000000000;
const int NS_PER_MS = 1000000;
} // namespace

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in milliseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeMS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1 = ts.tv_sec * MS_PER_S + ts.tv_nsec / NS_PER_MS;
    long t2 = GetTimeMS();

    ASSERT_TRUE(t2 - t1 >= 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in microseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeUS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1= ts.tv_sec * US_PER_S + ts.tv_nsec / NS_PER_US;
    long t2 = GetTimeUS();

    ASSERT_TRUE(t2 - t1 >= 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in nanoseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeNS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1 = ts.tv_sec * NS_PER_S + ts.tv_nsec;
    long t2 = GetTimeNS();
    ASSERT_TRUE(t2 - t1 >= 0);
}

/**
 * @tc.name: GetClientConnectionNoServerStarted
 * @tc.desc: GetClientConnection returns nullptr when server is not started.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetClientConnectionNoServerStarted, TestSize.Level1)
{
    ServiceEntry entry;
    EXPECT_EQ(entry.GetClientConnection(123), nullptr);
}

/**
 * @tc.name: GetClientConnectionNotFoundFd
 * @tc.desc: GetClientConnection returns nullptr when fd is not in the client map.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetClientConnectionNotFoundFd, TestSize.Level1)
{
    ServiceEntry entry;
    entry.unixSocketServer_ = std::make_shared<UnixSocketServer>();
    EXPECT_EQ(entry.GetClientConnection(999), nullptr);
}

/**
 * @tc.name: GetClientConnectionFoundFd
 * @tc.desc: GetClientConnection returns the connection for a registered fd.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetClientConnectionFoundFd, TestSize.Level1)
{
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    ServiceEntry entry;
    entry.unixSocketServer_ = std::make_shared<UnixSocketServer>();
    auto conn = std::make_shared<ClientConnection>(fds[0], entry);
    entry.unixSocketServer_->socketClients_[fds[0]] = conn;
    auto result = entry.GetClientConnection(fds[0]);
    EXPECT_EQ(result, conn);
    entry.unixSocketServer_->socketClients_.clear();
    close(fds[0]);
    close(fds[1]);
}
} // namespace