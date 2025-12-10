/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#include <gtest/gtest.h>
#include <memory>
#include "hook_client_holder.h"
#include "hook_socket_client.h"
#include "hook_common.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

extern struct ClientConfig g_ClientConfig;

namespace {
class HookClientHolderTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ValidSharedPtrTest
 * @tc.desc: Test valid shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, ValidSharedPtrTest, TestSize.Level0)
{
    g_ClientConfig.shareMemorySize = 1024 * 1024;
    auto client = std::make_shared<HookSocketClient>(getpid(), &g_ClientConfig, nullptr);

    HookClientHolder holder(client);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_NE(holder.Get(), nullptr);
}

/**
 * @tc.name: NullSharedPtrTest
 * @tc.desc: Test null shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, NullSharedPtrTest, TestSize.Level0)
{
    std::shared_ptr<HookSocketClient> nullClient;

    HookClientHolder holder(nullClient);
    EXPECT_FALSE(holder.IsValid());
    EXPECT_EQ(holder.Get(), nullptr);
}

/**
 * @tc.name: SharedPtrLifetimeTest
 * @tc.desc: Test holder keeps shared_ptr alive
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SharedPtrLifetimeTest, TestSize.Level0)
{
    g_ClientConfig.shareMemorySize = 1024 * 1024;
    std::shared_ptr<HookSocketClient> client = std::make_shared<HookSocketClient>(
        getpid(), &g_ClientConfig, nullptr);
    HookClientHolder holder(client);

    EXPECT_TRUE(holder.IsValid());
    EXPECT_NE(holder.Get(), nullptr);
}

/**
 * @tc.name: MultipleHoldersTest
 * @tc.desc: Test multiple holders can access the same client
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, MultipleHoldersTest, TestSize.Level0)
{
    g_ClientConfig.shareMemorySize = 1024 * 1024;
    auto client = std::make_shared<HookSocketClient>(getpid(), &g_ClientConfig, nullptr);

    HookClientHolder holder1(client);
    HookClientHolder holder2(client);

    EXPECT_TRUE(holder1.IsValid());
    EXPECT_TRUE(holder2.IsValid());
    EXPECT_EQ(holder1.Get(), holder2.Get());
}
}
