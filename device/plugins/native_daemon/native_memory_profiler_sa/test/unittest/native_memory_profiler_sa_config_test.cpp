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

class NativeMemoryProfilerSaConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

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
    config->discardDestoryedTraces_ = true;
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
    EXPECT_TRUE(result->discardDestoryedTraces_);
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
 * @tc.name: NativeMemoryProfilerSaConfigTest008
 * @tc.desc: bundleName_ serialization roundtrip
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaConfigTest, NativeMemoryProfilerSaConfigTest008, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->bundleName_ = "test_bundle";
    OHOS::Parcel parcel;
    EXPECT_EQ(config->Marshalling(parcel), true);

    std::shared_ptr<NativeMemoryProfilerSaConfig> result = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaConfig::Unmarshalling(parcel, result), true);
    EXPECT_EQ(result->bundleName_, "test_bundle");
}

#endif // NATIVE_MEMORY_PROFILER_SA_CONFIG_TEST_H
