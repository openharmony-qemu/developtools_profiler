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

#ifndef NATIVE_MEMORY_PROFILER_SA_CONFIG_TEST_H
#define NATIVE_MEMORY_PROFILER_SA_CONFIG_TEST_H

#include <gtest/gtest.h>
#include <memory>

#include "native_memory_profiler_sa_config.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

class NativeMemoryProfilerSaConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

using ParcelFieldWriter = bool (*)(OHOS::Parcel&, size_t);

static bool WriteGpuRangeForTest(OHOS::Parcel& parcel)
{
    for (size_t i = 0; i < GPU_TYPE_SIZE * GPU_RANGE_SIZE * 2; ++i) {
        if (!parcel.WriteUint64(i + 1)) {
            return false;
        }
    }
    return true;
}

static bool WriteCoreField(OHOS::Parcel& parcel, size_t index)
{
    switch (index) {
        case 0: return parcel.WriteInt32(1);
        case 1: return parcel.WriteString("file_path");
        case 2: return parcel.WriteUint32(3);
        case 3: return parcel.WriteInt32(4);
        case 4: return parcel.WriteUint32(5);
        case 5: return parcel.WriteString("process_name");
        case 6: return parcel.WriteUint8(7);
        case 7: return parcel.WriteBool(true);
        case 8: return parcel.WriteBool(false);
        case 9: return parcel.WriteBool(true);
        case 10: return parcel.WriteBool(false);
        case 11: return parcel.WriteUint32(12);
        case 12: return parcel.WriteUint32(13);
        case 13: return parcel.WriteBool(true);
        case 14: return parcel.WriteBool(false);
        case 15: return parcel.WriteBool(true);
        case 16: return parcel.WriteBool(false);
        case 17: return parcel.WriteBool(true);
        case 18: return parcel.WriteBool(false);
        case 19: return parcel.WriteBool(true);
        case 20: return parcel.WriteUint64(21);
        case 21: return parcel.WriteBool(false);
        case 22: return parcel.WriteUint64(23);
        case 23: return WriteGpuRangeForTest(parcel);
        default: return false;
    }
}

static bool WriteExtField(OHOS::Parcel& parcel, size_t index)
{
    switch (index) {
        case 0: return parcel.WriteBool(true);
        case 1: return parcel.WriteBool(false);
        case 2: return parcel.WriteUint32(3);
        case 3: return parcel.WriteInt32(CLOCK_REALTIME);
        case 4: return parcel.WriteUint32(5);
        case 5: return parcel.WriteBool(false);
        case 6: return parcel.WriteBool(true);
        case 7: return parcel.WriteBool(false);
        case 8: return parcel.WriteBool(true);
        case 9: return parcel.WriteBool(false);
        case 10: return parcel.WriteBool(true);
        case 11: return parcel.WriteString("file_name");
        case 12: return parcel.WriteUint32(13);
        case 13: return parcel.WriteUint32(14);
        case 14: return parcel.WriteInt32(15);
        case 15: return parcel.WriteUint8(16);
        case 16: return parcel.WriteString("napi_name");
        case 17: return parcel.WriteString("libtarget.so");
        case 18: return parcel.WriteUint32(0);
        case 19: return parcel.WriteUint32(20);
        case 20: return parcel.WriteUint32(21);
        case 21: return parcel.WriteUint32(22);
        case 22: return parcel.WriteUint32(23);
        case 23: return parcel.WriteString("type_name");
        case 24: return parcel.WriteUint32(25);
        case 25: return parcel.WriteUint32(26);
        case 26: return parcel.WriteUint32(27);
        case 27: return parcel.WriteUint32(28);
        case 28: return parcel.WriteBool(true);
        case 29: return parcel.WriteBool(false);
        case 30: return parcel.WriteUint32(31);
        case 31: return parcel.WriteUint32(32);
        case 32: return parcel.WriteUint32(33);
        case 33: return parcel.WriteUint32(34);
        default: return false;
    }
}

static bool WriteFieldPrefix(OHOS::Parcel& parcel, size_t fieldCount, ParcelFieldWriter writer)
{
    for (size_t i = 0; i < fieldCount; ++i) {
        if (!writer(parcel, i)) {
            return false;
        }
    }
    return true;
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest001
 * @tc.desc: Test serialization and derivativeization
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->duration_ = 100;
    std::string filePath = "XXXXXXXXXXXX";
    config->filePath_ = filePath;
    config->fpUnwind_ = false;

    OHOS::Parcel parcel;
    EXPECT_EQ(config->Marshalling(parcel), true);

    std::shared_ptr<NativeMemoryProfilerSaConfig> result = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result), true);
    EXPECT_EQ(result->duration_, 100);
    EXPECT_EQ(result->filePath_, filePath);
    EXPECT_EQ(result->fpUnwind_, false);
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest002
 * @tc.desc: Test adding member variable serialization and derivativeization
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest002, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->largestSize_ = 100;
    config->secondLargestSize_ = 200;
    config->maxGrowthSize_ = 300;

    OHOS::Parcel parcel;
    EXPECT_EQ(config->Marshalling(parcel), true);

    std::shared_ptr<NativeMemoryProfilerSaConfig> result = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result), true);
    EXPECT_EQ(result->largestSize_, 100);
    EXPECT_EQ(result->secondLargestSize_, 200);
    EXPECT_EQ(result->maxGrowthSize_, 300);
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest003
 * @tc.desc: NMD and memtrace fields serialization (printNmd_, memtraceEnable_, nmdPid_, nmdType_, gpuRange_)
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest003, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->printNmd_ = true;
    config->printNmdOnly_ = true;
    config->nmdPid_ = 100;
    config->nmdType_ = 2;
    config->memtraceEnable_ = true;
    config->gpuRange_[0][0] = 1;
    config->gpuRange_[0][1] = 2;

    OHOS::Parcel parcel;
    EXPECT_EQ(config->Marshalling(parcel), true);

    std::shared_ptr<NativeMemoryProfilerSaConfig> result = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result), true);
    EXPECT_EQ(result->printNmd_, true);
    EXPECT_EQ(result->printNmdOnly_, true);
    EXPECT_EQ(result->nmdPid_, 100u);
    EXPECT_EQ(result->nmdType_, 2u);
    EXPECT_EQ(result->memtraceEnable_, true);
    EXPECT_EQ(result->gpuRange_[0][0], 1u);
    EXPECT_EQ(result->gpuRange_[0][1], 2u);
}

 /**
 * @tc.name: NativeMemoryProfilerSaConfigTest004
 * @tc.desc: GWP-ASan fields serialization and deserialization
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest004, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->gwpAsanHoldTime_ = 30;
    config->gwpAsanMaxEntries_ = 64;
    config->gwpAsanMaxStackDepth_ = 16;

    OHOS::Parcel parcel;
    EXPECT_EQ(config->Marshalling(parcel), true);

    auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result), true);
    EXPECT_EQ(result->gwpAsanHoldTime_, 30u);
    EXPECT_EQ(result->gwpAsanMaxEntries_, 64u);
    EXPECT_EQ(result->gwpAsanMaxStackDepth_, 16u);
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest005
 * @tc.desc: Test extended fields and multi-entry vectors serialization
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest005, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->filterLibList_ = {"liba.so", "libb.so"};
    config->targetSoName_ = "libtarget.so";
    config->filterNapiName_ = "napi_entry";
    config->allocatorSamplingEnable_ = true;
    config->nativeOrphanSnapshotsEnable_ = true;
    config->memLeakFormat_ = 7;
    config->memLeakLimit_ = 9;
    config->asyncNestingDepth_ = 5;
    config->asyncTaskStackDepth_ = 6;
    config->arkuiMemTraceEnable_ = true;
    config->discardDestroyedTraces_ = true;
    config->gpuRange_[GPU_TYPE_SIZE - 1][GPU_RANGE_SIZE * 2 - 1] = 99;

    OHOS::Parcel parcel;
    ASSERT_TRUE(config->Marshalling(parcel));
    auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result));
    ASSERT_EQ(result->filterLibList_.size(), 2u);
    EXPECT_EQ(result->filterLibList_[0], "liba.so");
    EXPECT_EQ(result->filterLibList_[1], "libb.so");
    EXPECT_EQ(result->targetSoName_, "libtarget.so");
    EXPECT_EQ(result->filterNapiName_, "napi_entry");
    EXPECT_TRUE(result->allocatorSamplingEnable_);
    EXPECT_TRUE(result->nativeOrphanSnapshotsEnable_);
    EXPECT_EQ(result->memLeakFormat_, 7u);
    EXPECT_EQ(result->memLeakLimit_, 9u);
    EXPECT_EQ(result->asyncNestingDepth_, 5u);
    EXPECT_EQ(result->asyncTaskStackDepth_, 6u);
    EXPECT_TRUE(result->arkuiMemTraceEnable_);
    EXPECT_TRUE(result->discardDestroyedTraces_);
    EXPECT_EQ(result->gpuRange_[GPU_TYPE_SIZE - 1][GPU_RANGE_SIZE * 2 - 1], 99u);
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest006
 * @tc.desc: Test Unmarshalling fails when filter library list payload is incomplete
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest006, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::Parcel parcel;
    ASSERT_TRUE(parcel.WriteBool(true));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteUint32(1));
    ASSERT_TRUE(parcel.WriteInt32(CLOCK_REALTIME));
    ASSERT_TRUE(parcel.WriteUint32(2));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteString("file_name"));
    ASSERT_TRUE(parcel.WriteUint32(3));
    ASSERT_TRUE(parcel.WriteUint32(4));
    ASSERT_TRUE(parcel.WriteInt32(5));
    ASSERT_TRUE(parcel.WriteUint8(6));
    ASSERT_TRUE(parcel.WriteString("napi_name"));
    ASSERT_TRUE(parcel.WriteString("libtarget.so"));
    ASSERT_TRUE(parcel.WriteUint32(1));

    auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(NativeMemoryProfilerSaConfig::UnmarshallingExt(parcel, result));
}

/**
 * @tc.name: NativeMemoryProfilerSaConfigTest007
 * @tc.desc: Test Unmarshalling fails when extended scalar tail is incomplete
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest007, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::Parcel parcel;
    ASSERT_TRUE(parcel.WriteBool(true));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteUint32(1));
    ASSERT_TRUE(parcel.WriteInt32(CLOCK_REALTIME));
    ASSERT_TRUE(parcel.WriteUint32(2));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteBool(false));
    ASSERT_TRUE(parcel.WriteString("file_name"));
    ASSERT_TRUE(parcel.WriteUint32(3));
    ASSERT_TRUE(parcel.WriteUint32(4));
    ASSERT_TRUE(parcel.WriteInt32(5));
    ASSERT_TRUE(parcel.WriteUint8(6));
    ASSERT_TRUE(parcel.WriteString("napi_name"));
    ASSERT_TRUE(parcel.WriteString("libtarget.so"));
    ASSERT_TRUE(parcel.WriteUint32(0));
    ASSERT_TRUE(parcel.WriteUint32(10));
    ASSERT_TRUE(parcel.WriteUint32(11));
    ASSERT_TRUE(parcel.WriteUint32(12));
    ASSERT_TRUE(parcel.WriteUint32(13));
    ASSERT_TRUE(parcel.WriteString("type_name"));

    auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(NativeMemoryProfilerSaConfig::UnmarshallingExt(parcel, result));
}

/**
 * @tc.name: UnmarshallingRejectsNullAndTruncatedCoreFields
 * @tc.desc: Unmarshalling rejects null config and truncated core fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, UnmarshallingRejectsNullAndTruncatedCoreFields, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::Parcel emptyParcel;
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig = nullptr;
    EXPECT_FALSE(NativeMemoryProfilerSaConfig::Unmarshalling(emptyParcel, nullConfig));

    constexpr size_t coreFieldCount = 24;
    for (size_t prefix = 0; prefix < coreFieldCount; ++prefix) {
        OHOS::Parcel parcel;
        ASSERT_TRUE(WriteFieldPrefix(parcel, prefix, WriteCoreField));
        auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
        ASSERT_NE(result, nullptr);
        EXPECT_FALSE(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result)) << prefix;
    }
}

/**
 * @tc.name: UnmarshallingExtRejectsTruncatedFields
 * @tc.desc: Unmarshalling rejects truncated extension fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, UnmarshallingExtRejectsTruncatedFields, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    constexpr size_t extFieldCount = 34;
    for (size_t prefix = 0; prefix < extFieldCount; ++prefix) {
        OHOS::Parcel parcel;
        ASSERT_TRUE(WriteFieldPrefix(parcel, prefix, WriteExtField));
        auto result = std::make_shared<NativeMemoryProfilerSaConfig>();
        ASSERT_NE(result, nullptr);
        EXPECT_FALSE(NativeMemoryProfilerSaConfig::UnmarshallingExt(parcel, result)) << prefix;
    }
}

#endif // NATIVE_MEMORY_PROFILER_SA_CONFIG_TEST_H
