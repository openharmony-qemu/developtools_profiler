/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "preload_socket_client.h"
#include "hook_service.h"
#include "hook_common.h"
#include "service_entry.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "logging.h"

using namespace testing::ext;

namespace {
class PreloadSocketClientTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: ProtocolProc1
 * @tc.desc: test PreloadSocketClientTest::ProtocolProc with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadSocketClientTest, ProtocolProc1, TestSize.Level0)
{
    std::string diagPath = "/system/lib64/test/diagtest.so";
    PreloadSocketClient preloadClient("com.ohos.test");
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(diagPath.c_str());
    auto size = diagPath.size();
    ASSERT_TRUE(preloadClient.ProtocolProc(socketContext, 0, ptr, size));
}

/*
 * @tc.name: ProtocolProc2
 * @tc.desc: test PreloadSocketClientTest::ProtocolProc, input buf invalid.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadSocketClientTest, ProtocolProc2, TestSize.Level0)
{
    std::string diagPath = "/system/lib64/test/diagtest.so";
    PreloadSocketClient preloadClient("com.ohos.test");
    SocketContext socketContext;
    auto size = diagPath.size();
    ASSERT_FALSE(preloadClient.ProtocolProc(socketContext, 0, nullptr, size));
}

/*
 * @tc.name: ProtocolProc3
 * @tc.desc: test PreloadSocketClientTest::ProtocolProc, input size invalid.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadSocketClientTest, ProtocolProc3, TestSize.Level0)
{
    std::string diagPath = "/system/lib64/test/diagtest.so";
    PreloadSocketClient preloadClient("com.ohos.test");
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(diagPath.c_str());
    ASSERT_FALSE(preloadClient.ProtocolProc(socketContext, 0, ptr, 0));
}

/*
 * @tc.name: GetDiagPath
 * @tc.desc: test PreloadSocketClientTest::GetDiagPath normal case.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadSocketClientTest, GetDiagPath, TestSize.Level0)
{
    std::string diagPath = "/system/lib64/test/diagtest.so";
    PreloadSocketClient preloadClient("com.ohos.test");
    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(diagPath.c_str());
    auto size = diagPath.size();
    ASSERT_TRUE(preloadClient.ProtocolProc(socketContext, 0, ptr, size));
    ASSERT_TRUE(preloadClient.GetDiagPath() == diagPath);
}

} // namespace
