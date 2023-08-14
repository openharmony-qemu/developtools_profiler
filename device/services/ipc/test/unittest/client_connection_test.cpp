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

#include "client_connection.h"
#include "service_entry.h"

using namespace testing::ext;

namespace {
class ClientConnectionTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Service
 * @tc.desc: Client connection.
 * @tc.type: FUNC
 */
HWTEST_F(ClientConnectionTest, RawProtocolProc, TestSize.Level1)
{
    std::string serviceName="test_service_name";
    ServiceEntry serviceEntry;
    std::unique_ptr<ClientConnection> clientConnection = std::make_unique<ClientConnection>(0, serviceEntry);
    ASSERT_EQ(clientConnection->RawProtocolProc(RAW_PROTOCOL_POINTTO_SERVICE,
        (const int8_t *)serviceName.c_str(), sizeof(struct RawPointToService)), 1);
    ASSERT_EQ(clientConnection->RawProtocolProc(0,
        (const int8_t *)serviceName.c_str(), sizeof(struct RawPointToService)), -1);
    clientConnection.reset(nullptr);
}
} // namespace