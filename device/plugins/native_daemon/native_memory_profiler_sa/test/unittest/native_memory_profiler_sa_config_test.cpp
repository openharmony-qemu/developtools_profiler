/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#define private public

#include <gtest/gtest.h>
#include <memory>
#include "message_parcel.h"
#include "native_memory_profiler_sa_config.h"
#include "hook_manager.h"
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

class NativeMemoryProfilerSaConfigTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

static std::shared_ptr<NativeMemoryProfilerSaConfig> MarshalAndUnmarshal(
    std::shared_ptr<NativeMemoryProfilerSaConfig> src)
{
    OHOS::MessageParcel parcel;
    if (!src->Marshalling(parcel)) {
        return nullptr;
    }
    auto dst = std::make_shared<NativeMemoryProfilerSaConfig>();
    if (!NativeMemoryProfilerSaConfig::Unmarshalling(parcel, dst)) {
        return nullptr;
    }
    return dst;
}

/**
 * @tc.name: AsyncTypeMarshallingWithValues
 * @tc.desc: asyncType_ with values survives Marshalling/Unmarshalling round-trip
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingWithValues, TestSize.Level0)
{
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    src->asyncStackEnable_ = true;
    src->asyncType_ = {0, 1, 3}; // LIBUV_SEND, FFRT_POOL, EVENTHANDLER

    auto dst = MarshalAndUnmarshal(src);
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->asyncStackEnable_, true);
    ASSERT_EQ(dst->asyncType_.size(), 3u);
    EXPECT_EQ(dst->asyncType_[0], 0);
    EXPECT_EQ(dst->asyncType_[1], 1);
    EXPECT_EQ(dst->asyncType_[2], 3);
}

/**
 * @tc.name: AsyncTypeMarshallingEmpty
 * @tc.desc: empty asyncType_ survives Marshalling/Unmarshalling round-trip
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingEmpty, TestSize.Level0)
{
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    src->asyncStackEnable_ = true;
    src->asyncType_ = {};

    auto dst = MarshalAndUnmarshal(src);
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->asyncStackEnable_, true);
    EXPECT_TRUE(dst->asyncType_.empty());
}

/**
 * @tc.name: AsyncTypeMarshallingBoundaryValue
 * @tc.desc: asyncType_ with ALL_ASYNC_TYPE (10) survives round-trip
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingBoundaryValue, TestSize.Level0)
{
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    src->asyncStackEnable_ = true;
    src->asyncType_ = {10}; // ALL_ASYNC_TYPE

    auto dst = MarshalAndUnmarshal(src);
    ASSERT_NE(dst, nullptr);
    ASSERT_EQ(dst->asyncType_.size(), 1u);
    EXPECT_EQ(dst->asyncType_[0], 10);
}

/**
 * @tc.name: SetHookConfigEmptyAsyncTypeDefaultsToAll
 * @tc.desc: When asyncStackEnable is true and asyncType_ is empty, ALL_ASYNC_TYPE is added
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetHookConfigEmptyAsyncTypeDefaultsToAll, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = true;
    config->asyncType_ = {}; // empty -> should default to ALL_ASYNC_TYPE

    hookManager.SetHookConfig(config);

    EXPECT_TRUE(hookManager.hookConfig_.async_stack_enable());
    ASSERT_EQ(hookManager.hookConfig_.async_type_size(), 1);
    EXPECT_EQ(hookManager.hookConfig_.async_type(0), AsyncFlagType::ALL_ASYNC_TYPE);
}

/**
 * @tc.name: SetHookConfigSpecificAsyncTypes
 * @tc.desc: When asyncType_ has specific values, they are forwarded to hookConfig_
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetHookConfigSpecificAsyncTypes, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = true;
    config->asyncType_ = {0, 4}; // LIBUV_SEND, PROMISE

    hookManager.SetHookConfig(config);

    EXPECT_TRUE(hookManager.hookConfig_.async_stack_enable());
    ASSERT_EQ(hookManager.hookConfig_.async_type_size(), 2);
    EXPECT_EQ(hookManager.hookConfig_.async_type(0), AsyncFlagType::LIBUV_SEND);
    EXPECT_EQ(hookManager.hookConfig_.async_type(1), AsyncFlagType::PROMISE);
}

/**
 * @tc.name: SetHookConfigAsyncDisabledNoAsyncType
 * @tc.desc: When asyncStackEnable is false, async_type should not be set
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetHookConfigAsyncDisabledNoAsyncType, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = false;
    config->asyncType_ = {0, 1};

    hookManager.SetHookConfig(config);

    EXPECT_FALSE(hookManager.hookConfig_.async_stack_enable());
    EXPECT_EQ(hookManager.hookConfig_.async_type_size(), 0);
}

// Forward declaration for ParseAsyncType defined in native_daemon_client.cpp
std::vector<int32_t> ParseAsyncType(const std::string& str);

/**
 * @tc.name: AsyncTypeMarshallingOutOfRangeFiltered
 * @tc.desc: asyncType_ with out-of-range values (negative / >10) are filtered by ReadAsyncType
 *           The values are stored as-is during marshalling; range check is in ParseAsyncType/SetAsyncTypeFromConfig.
 *           Verify that oversized size field causes Unmarshalling to return false.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingOversizeFails, TestSize.Level0)
{
    // Manually write a parcel with size > MAX_ASYNC_TYPE_SIZE (16) to trigger the guard
    OHOS::MessageParcel parcel;
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    // Fill all fields up to asyncStackEnable_ via normal Marshalling, then we'll craft the rest
    // Simpler: write a valid config but inject oversized asyncType size via raw parcel manipulation
    // We test the guard indirectly: create a config with 17 elements (exceeds MAX=16)
    src->asyncType_.resize(17, 0);
    // Marshalling writes size=17, ReadAsyncType should reject it
    OHOS::MessageParcel writeParcel;
    ASSERT_TRUE(src->Marshalling(writeParcel));
    // Unmarshalling should fail because size=17 > MAX_ASYNC_TYPE_SIZE=16
    auto dst = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_FALSE(NativeMemoryProfilerSaConfig::Unmarshalling(writeParcel, dst));
}

/**
 * @tc.name: ParseAsyncTypeFiltersOutOfRange
 * @tc.desc: ParseAsyncType skips negative values and values > ALL_ASYNC_TYPE(10)
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, ParseAsyncTypeFiltersOutOfRange, TestSize.Level0)
{
    // negative value -1 should be skipped
    auto result = ParseAsyncType("-1");
    EXPECT_TRUE(result.empty());

    // value 99 exceeds ALL_ASYNC_TYPE(10), should be skipped
    result = ParseAsyncType("99");
    EXPECT_TRUE(result.empty());

    // mixed: valid 0, invalid 11, valid 10
    result = ParseAsyncType("0,11,10");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 10);
}

/**
 * @tc.name: ParseAsyncTypeEmptyString
 * @tc.desc: ParseAsyncType with empty string returns empty vector
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, ParseAsyncTypeEmptyString, TestSize.Level0)
{
    auto result = ParseAsyncType("");
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ParseAsyncTypeNonNumericMixed
 * @tc.desc: ParseAsyncType with non-numeric tokens skips them
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, ParseAsyncTypeNonNumericMixed, TestSize.Level0)
{
    // "abc" is non-numeric, "3" is valid
    auto result = ParseAsyncType("abc,3,xyz");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 3);
}

/**
 * @tc.name: SetAsyncTypeFromConfigFiltersOutOfRange
 * @tc.desc: SetAsyncTypeFromConfig skips values outside [0, ALL_ASYNC_TYPE]
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetAsyncTypeFromConfigFiltersOutOfRange, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = true;
    config->asyncType_ = {-1, 0, 11, 99, 10}; // -1, 11, 99 should be filtered

    hookManager.SetHookConfig(config);

    ASSERT_EQ(hookManager.hookConfig_.async_type_size(), 2);
    EXPECT_EQ(hookManager.hookConfig_.async_type(0), AsyncFlagType::LIBUV_SEND); // 0
    EXPECT_EQ(hookManager.hookConfig_.async_type(1), AsyncFlagType::ALL_ASYNC_TYPE); // 10
}

/**
 * @tc.name: AsyncTypeMarshallingExactBoundary
 * @tc.desc: asyncType_ with exactly MAX_ASYNC_TYPE_SIZE(16) elements passes Marshalling/Unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingExactBoundary, TestSize.Level0)
{
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    src->asyncStackEnable_ = true;
    // Fill 16 elements (exactly MAX_ASYNC_TYPE_SIZE), values cycle through valid range [0,10]
    src->asyncType_.resize(16);
    for (int i = 0; i < 16; i++) {
        src->asyncType_[i] = i % 11; // 0..10
    }

    auto dst = MarshalAndUnmarshal(src);
    ASSERT_NE(dst, nullptr);
    ASSERT_EQ(dst->asyncType_.size(), 16u);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(dst->asyncType_[i], i % 11);
    }
}

/**
 * @tc.name: SetHookConfigAsyncDisabledIgnoresAsyncType
 * @tc.desc: When asyncStackEnable_ is false, asyncType_ is not forwarded to hookConfig_
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetHookConfigAsyncDisabledIgnoresAsyncType, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = false;
    config->asyncType_ = {0, 1, 2};

    hookManager.SetHookConfig(config);

    EXPECT_FALSE(hookManager.hookConfig_.async_stack_enable());
    EXPECT_EQ(hookManager.hookConfig_.async_type_size(), 0);
}

/**
 * @tc.name: AsyncTypeMarshallingOrderPreserved
 * @tc.desc: asyncType_ element order is preserved after Marshalling/Unmarshalling round-trip
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, AsyncTypeMarshallingOrderPreserved, TestSize.Level0)
{
    auto src = std::make_shared<NativeMemoryProfilerSaConfig>();
    src->asyncStackEnable_ = true;
    src->asyncType_ = {3, 1, 4, 2};

    auto dst = MarshalAndUnmarshal(src);
    ASSERT_NE(dst, nullptr);
    ASSERT_EQ(dst->asyncType_.size(), 4u);
    EXPECT_EQ(dst->asyncType_[0], 3);
    EXPECT_EQ(dst->asyncType_[1], 1);
    EXPECT_EQ(dst->asyncType_[2], 4);
    EXPECT_EQ(dst->asyncType_[3], 2);
}

/**
 * @tc.name: SetHookConfigDuplicateAsyncTypePreserved
 * @tc.desc: SetHookConfig does not deduplicate asyncType_; duplicate values are forwarded as-is
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, SetHookConfigDuplicateAsyncTypePreserved, TestSize.Level0)
{
    HookManager hookManager;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->asyncStackEnable_ = true;
    config->asyncType_ = {0, 0, 1, 1};

    hookManager.SetHookConfig(config);

    // Current implementation does not deduplicate; all 4 entries should be present
    EXPECT_TRUE(hookManager.hookConfig_.async_stack_enable());
    ASSERT_EQ(hookManager.hookConfig_.async_type_size(), 4);
    EXPECT_EQ(hookManager.hookConfig_.async_type(0), AsyncFlagType::LIBUV_SEND);
    EXPECT_EQ(hookManager.hookConfig_.async_type(1), AsyncFlagType::LIBUV_SEND);
    EXPECT_EQ(hookManager.hookConfig_.async_type(2), AsyncFlagType::FFRT_POOL);
    EXPECT_EQ(hookManager.hookConfig_.async_type(3), AsyncFlagType::FFRT_POOL);
}
