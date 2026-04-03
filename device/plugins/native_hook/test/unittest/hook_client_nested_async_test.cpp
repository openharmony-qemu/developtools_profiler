/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
#include "hook_client.h"
#include "native_hook_config.pb.h"
#include <memory>

using namespace testing::ext;

namespace {
class HookClientNestedAsyncTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: nestedAsyncStackEnableConfigTest
 * @tc.desc: Test enable nested async stack configuration
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, nestedAsyncStackEnableConfigTest, TestSize.Level0)
{
    auto* config = static_cast<ClientConfig*>(ohos_malloc_hook_get_hook_config());
    ASSERT_NE(config, nullptr);
    
    config->nestedAsyncStackEnable = true;
    EXPECT_TRUE(config->nestedAsyncStackEnable);
    
    config->nestedAsyncStackEnable = false;
    EXPECT_FALSE(config->nestedAsyncStackEnable);
}

/*
 * @tc.name: AsyncStackEnableConfigTest
 * @tc.desc: Test async stack enable configuration
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, AsyncStackEnableConfigTest, TestSize.Level0)
{
    auto* config = static_cast<ClientConfig*>(ohos_malloc_hook_get_hook_config());
    ASSERT_NE(config, nullptr);
    
    config->asyncStackEnable = true;
    EXPECT_TRUE(config->asyncStackEnable);
    
    config->asyncStackEnable = false;
    EXPECT_FALSE(config->asyncStackEnable);
}

/*
 * @tc.name: MaxStackDepthConfigTest
 * @tc.desc: Test max stack depth configuration
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, MaxStackDepthConfigTest, TestSize.Level0)
{
    auto* config = static_cast<ClientConfig*>(ohos_malloc_hook_get_hook_config());
    ASSERT_NE(config, nullptr);
    
    config->maxStackDepth = 64;
    EXPECT_EQ(config->maxStackDepth, 64);
    
    config->maxStackDepth = 128;
    EXPECT_EQ(config->maxStackDepth, 128);
    
    config->maxStackDepth = 0;
    EXPECT_EQ(config->maxStackDepth, 0);
}

/*
 * @tc.name: FpUnwindConfigTest
 * @tc.desc: Test fp unwind configuration
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, FpUnwindConfigTest, TestSize.Level0)
{
    auto* config = static_cast<ClientConfig*>(ohos_malloc_hook_get_hook_config());
    ASSERT_NE(config, nullptr);
    
    config->fpunwind = true;
    EXPECT_TRUE(config->fpunwind);
    
    config->fpunwind = false;
    EXPECT_FALSE(config->fpunwind);
}

/*
 * @tc.name: NestedAsyncStackIntegrationTest
 * @tc.desc: Test nested async stack integration with multiple configs
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, NestedAsyncStackIntegrationTest, TestSize.Level0)
{
    auto* config = static_cast<ClientConfig*>(ohos_malloc_hook_get_hook_config());
    ASSERT_NE(config, nullptr);
    
    config->nestedAsyncStackEnable = true;
    config->asyncStackEnable = true;
    config->fpunwind = true;
    config->maxStackDepth = 128;
    
    EXPECT_TRUE(config->nestedAsyncStackEnable);
    EXPECT_TRUE(config->asyncStackEnable);
    EXPECT_TRUE(config->fpunwind);
    EXPECT_EQ(config->maxStackDepth, 128);
    
    config->nestedAsyncStackEnable = false;
    config->asyncStackEnable = false;
    config->fpunwind = false;
    config->maxStackDepth = 0;
    
    EXPECT_FALSE(config->nestedAsyncStackEnable);
    EXPECT_FALSE(config->asyncStackEnable);
    EXPECT_FALSE(config->fpunwind);
    EXPECT_EQ(config->maxStackDepth, 0);
}

/*
 * @tc.name: HookFlagOperationsTest
 * @tc.desc: Test hook flag operations
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, HookFlagOperationsTest, TestSize.Level0)
{
    bool flag = ohos_malloc_hook_get_hook_flag();
    EXPECT_TRUE(flag);
    
    bool oldFlag = ohos_malloc_hook_set_hook_flag(false);
    EXPECT_TRUE(oldFlag);
    EXPECT_FALSE(ohos_malloc_hook_get_hook_flag());
    
    oldFlag = ohos_malloc_hook_set_hook_flag(true);
    EXPECT_FALSE(oldFlag);
    EXPECT_TRUE(ohos_malloc_hook_get_hook_flag());
}

/*
 * @tc.name: FilterSizeTest
 * @tc.desc: Test filter size functionality
 * @tc.type: FUNC
 */
HWTEST_F(HookClientNestedAsyncTest, FilterSizeTest, TestSize.Level0)
{
    void* ptr = reinterpret_cast<void*>(0x1000);
    
    bool result = ohos_set_filter_size(1024, ptr);
    EXPECT_TRUE(result);
    
    result = ohos_set_filter_size(512, ptr);
    EXPECT_TRUE(result);
    
    result = ohos_set_filter_size(0, ptr);
    EXPECT_FALSE(result);
}
}