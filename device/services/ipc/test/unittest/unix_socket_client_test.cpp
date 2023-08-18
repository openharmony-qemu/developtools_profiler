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

#include "service_base.h"
#include "unix_socket_client.h"

using namespace testing::ext;

namespace {
class UnixSocketClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Service
 * @tc.desc: Socket client connection.
 * @tc.type: FUNC
 */
HWTEST_F(UnixSocketClientTest, Connect, TestSize.Level1)
{
    UnixSocketClient unixSocketClient;
    ServiceBase serviceBase;
    ASSERT_TRUE(!unixSocketClient.Connect("asdf", serviceBase));
}
} // namespace