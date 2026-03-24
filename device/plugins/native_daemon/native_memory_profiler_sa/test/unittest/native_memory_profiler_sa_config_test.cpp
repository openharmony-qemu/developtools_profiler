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

#endif // NATIVE_MEMORY_PROFILER_SA_CONFIG_TEST_H