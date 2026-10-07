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
#include "client_map.h"
#include "service_entry.h"

using namespace testing::ext;

namespace {
class ClientMapTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Service
 * @tc.desc: Client map performance.
 * @tc.type: FUNC
 */
HWTEST_F(ClientMapTest, ClientSocket, TestSize.Level1)
{
    ServiceEntry serviceEntry;
    ServiceEntry serviceTest;
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    int fds2[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds2), 0);
    ASSERT_EQ(ClientMap::GetInstance().PutClientSocket(fds[0], serviceEntry), 1);
    ClientMap::GetInstance().PutClientSocket(fds2[0], serviceTest);
    ASSERT_EQ(ClientMap::GetInstance().PutClientSocket(fds2[0], serviceTest), -1);
    ASSERT_EQ(ClientMap::GetInstance().AutoRelease(), 1);
    ClientMap::GetInstance().socketClients_[fds2[0]]->clientState_ = CLIENT_STAT_THREAD_EXITED;
    ASSERT_EQ(ClientMap::GetInstance().AutoRelease(), 1);
    ASSERT_EQ(ClientMap::GetInstance().ClearClientSocket(), 0);
    ASSERT_EQ(ClientMap::GetInstance().AutoRelease(), 1);
    close(fds[1]);
    close(fds2[1]);
}
} // namespace