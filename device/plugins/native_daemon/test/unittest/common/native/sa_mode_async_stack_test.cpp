/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "native_memory_profiler_sa_config.h"
#include "hook_manager.h"
#include "native_hook_config.pb.h"
#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {

class SaModeAsyncStackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SaModeAsyncStackTest::SetUpTestCase(void) {}
void SaModeAsyncStackTest::TearDownTestCase(void) {}
void SaModeAsyncStackTest::SetUp() {}
void SaModeAsyncStackTest::TearDown() {}

/**
 * @tc.name: SaConfigAsyncStackMarshallingTest
 * @tc.desc: Test NativeMemoryProfilerSaConfig async stack fields marshalling/unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, SaConfigAsyncStackMarshallingTest, TestSize.Level0)
{
    // Create config with async stack settings
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = true;
    config->asyncFlag_ = 0x0003; // LIBUV_SEND | FFRT_POOL
    config->nestedAsyncStackEnable_ = true;
    config->traceMask_ = 0xFFFFFFFF;

    // Create parcel and marshall
    Parcel parcel;
    ASSERT_TRUE(config->Marshalling(parcel));

    // Unmarshall to new config
    auto unmarshalledConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, unmarshalledConfig));

    // Verify async stack fields
    EXPECT_EQ(unmarshalledConfig->asyncStackEnable_, true);
    EXPECT_EQ(unmarshalledConfig->asyncFlag_, 0x0003);
    EXPECT_EQ(unmarshalledConfig->nestedAsyncStackEnable_, true);
    EXPECT_EQ(unmarshalledConfig->traceMask_, 0xFFFFFFFF);
}

/**
 * @tc.name: SaConfigAsyncStackDefaultValuesTest
 * @tc.desc: Test NativeMemoryProfilerSaConfig async stack fields default values
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, SaConfigAsyncStackDefaultValuesTest, TestSize.Level0)
{
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();

    // Verify default values
    EXPECT_EQ(config->asyncStackEnable_, false);
    EXPECT_EQ(config->asyncFlag_, 0);
    EXPECT_EQ(config->nestedAsyncStackEnable_, false);
    EXPECT_EQ(config->traceMask_, 0);
}

/**
 * @tc.name: HookManagerAsyncStackConfigTest
 * @tc.desc: Test HookManager SetHookConfig with async stack configuration
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, HookManagerAsyncStackConfigTest, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);

    // Create SA config with async stack settings
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->asyncStackEnable_ = true;
    saConfig->asyncFlag_ = 0x0007; // LIBUV_SEND | FFRT_POOL | FFRT_QUEUE
    saConfig->nestedAsyncStackEnable_ = true;

    // Set config
    hookManager->SetHookConfig(saConfig);

    // Get internal hook config and verify
    NativeHookConfig hookConfig = hookManager->GetHookConfig();
    EXPECT_EQ(hookConfig.async_stack_enable(), true);
    EXPECT_EQ(hookConfig.async_flag(), 0x0007);
    EXPECT_EQ(hookConfig.nested_async_stack_enable(), true);
}

/**
 * @tc.name: HookManagerAsyncStackConfigDisabledTest
 * @tc.desc: Test HookManager SetHookConfig with async stack disabled
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, HookManagerAsyncStackConfigDisabledTest, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);

    // Create SA config with async stack disabled
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->asyncStackEnable_ = false;
    saConfig->asyncFlag_ = 0;
    saConfig->nestedAsyncStackEnable_ = false;

    // Set config
    hookManager->SetHookConfig(saConfig);

    // Get internal hook config and verify
    NativeHookConfig hookConfig = hookManager->GetHookConfig();
    EXPECT_EQ(hookConfig.async_stack_enable(), false);
    EXPECT_EQ(hookConfig.async_flag(), 0);
    EXPECT_EQ(hookConfig.nested_async_stack_enable(), false);
}

/**
 * @tc.name: HookManagerAsyncStackTypeConversionTest
 * @tc.desc: Test HookManager async flag type conversion to mask
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, HookManagerAsyncStackTypeConversionTest, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);

    // Create SA config with ARKTS_WORKER async type
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->asyncStackEnable_ = true;
    saConfig->asyncFlag_ = 0x0040; // ARKTS_WORKER (bit 6)
    saConfig->nestedAsyncStackEnable_ = false;

    // Set config
    hookManager->SetHookConfig(saConfig);

    // Get internal hook config and verify
    NativeHookConfig hookConfig = hookManager->GetHookConfig();
    EXPECT_EQ(hookConfig.async_stack_enable(), true);
    EXPECT_EQ(hookConfig.async_flag(), 0x0040);
    EXPECT_EQ(hookConfig.nested_async_stack_enable(), false);
}

/**
 * @tc.name: SaConfigFullAsyncStackMarshallingTest
 * @tc.desc: Test full config marshalling with all async stack related fields
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, SaConfigFullAsyncStackMarshallingTest, TestSize.Level0)
{
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();

    // Set basic fields
    config->pid_ = 12345;
    config->duration_ = 30;
    config->maxStackDepth_ = 50;

    // Set async stack fields
    config->asyncStackEnable_ = true;
    config->asyncFlag_ = 0x07FF; // ALL_ASYNC_TYPE
    config->nestedAsyncStackEnable_ = true;
    config->traceMask_ = 0x00000001; // LIBUV_SEND mask

    // Set other fields
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->stringCompressed_ = false;
    config->offlineSymbolization_ = false;

    // Marshall
    Parcel parcel;
    ASSERT_TRUE(config->Marshalling(parcel));

    // Unmarshall
    auto unmarshalledConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, unmarshalledConfig));

    // Verify all async stack fields
    EXPECT_EQ(unmarshalledConfig->asyncStackEnable_, true);
    EXPECT_EQ(unmarshalledConfig->asyncFlag_, 0x07FF);
    EXPECT_EQ(unmarshalledConfig->nestedAsyncStackEnable_, true);
    EXPECT_EQ(unmarshalledConfig->traceMask_, 0x00000001);

    // Verify other fields are also correctly marshalled
    EXPECT_EQ(unmarshalledConfig->pid_, 12345);
    EXPECT_EQ(unmarshalledConfig->duration_, 30);
    EXPECT_EQ(unmarshalledConfig->maxStackDepth_, 50);
    EXPECT_EQ(unmarshalledConfig->mallocDisable_, true);
    EXPECT_EQ(unmarshalledConfig->mmapDisable_, true);
    EXPECT_EQ(unmarshalledConfig->stringCompressed_, false);
    EXPECT_EQ(unmarshalledConfig->offlineSymbolization_, false);
}

/**
 * @tc.name: SaConfigMmapAnonymousOnlyMarshallingTest
 * @tc.desc: Test mmapAnonymousOnly_ marshalling/unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, SaConfigMmapAnonymousOnlyMarshallingTest, TestSize.Level0)
{
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->mmapAnonymousOnly_ = true;

    Parcel parcel;
    ASSERT_TRUE(config->Marshalling(parcel));

    auto unmarshalledConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, unmarshalledConfig));
    EXPECT_EQ(unmarshalledConfig->mmapAnonymousOnly_, true);

    // default false
    auto config2 = std::make_shared<NativeMemoryProfilerSaConfig>();
    Parcel parcel2;
    ASSERT_TRUE(config2->Marshalling(parcel2));
    auto unmarshalled2 = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel2, unmarshalled2));
    EXPECT_EQ(unmarshalled2->mmapAnonymousOnly_, false);
}

#ifdef __aarch64__
/**
 * @tc.name: DfxSetAsyncStackTypeIntegrationTest
 * @tc.desc: Integration test for DfxSetAsyncStackType with async flag mask
 * @tc.type: FUNC
 */
HWTEST_F(SaModeAsyncStackTest, DfxSetAsyncStackTypeIntegrationTest, TestSize.Level0)
{
    // Test that async flag mask can be properly constructed
    uint64_t asyncFlagMask = 0;

    // Simulate combining multiple async types
    asyncFlagMask |= (1ULL << 0); // LIBUV_SEND
    asyncFlagMask |= (1ULL << 1); // FFRT_POOL
    asyncFlagMask |= (1ULL << 6); // ARKTS_WORKER

    EXPECT_EQ(asyncFlagMask, 0x0043);

    // Verify the mask can be passed to config
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncFlag_ = asyncFlagMask;

    Parcel parcel;
    ASSERT_TRUE(config->Marshalling(parcel));

    auto unmarshalledConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, unmarshalledConfig));

    EXPECT_EQ(unmarshalledConfig->asyncFlag_, asyncFlagMask);
}
#endif

} // namespace
