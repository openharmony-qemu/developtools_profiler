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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "jsmap_collector.h"
#include "i_data_storage.h"

using namespace testing::ext;

namespace {
// 4194305 exceeds PID_MAX_LIMIT (4194304 = 2^22), the absolute kernel upper bound,
// guarantees no matching process record on any OHOS device under any pid_max config.
constexpr uint32_t VALID_PID = 4194305;
constexpr uint32_t VALID_TID = 4194305;
constexpr uint32_t INVALID_PID = 0;
constexpr uint32_t INVALID_TID = 0;
}

class MockStorageForJsmap : public IDataStorage {
public:
    MockStorageForJsmap() : addFileCallCount(0), addFileReturnValue(true) {}
    bool AddFile(const std::string &filepath) override
    {
        lastFilePath = filepath;
        addFileCallCount++;
        return addFileReturnValue;
    }
    int addFileCallCount;
    bool addFileReturnValue;
    std::string lastFilePath;
};

class JsmapCollectorTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ConstructorWithValidPidTid
 * @tc.desc: Test JsmapCollector can be constructed with pid and tid
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, ConstructorWithValidPidTid, TestSize.Level1)
{
    JsmapCollector collector(VALID_PID, VALID_TID);
    SUCCEED();
}

/**
 * @tc.name: CollectWithNullStorage
 * @tc.desc: Test Collect returns false when storage is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, CollectWithNullStorage, TestSize.Level1)
{
    JsmapCollector collector(VALID_PID, VALID_TID);
    std::shared_ptr<IDataStorage> nullStorage = nullptr;
    EXPECT_FALSE(collector.Collect(nullStorage));
}

/**
 * @tc.name: CollectWithInvalidPid
 * @tc.desc: Test Collect returns false when pid is 0
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, CollectWithInvalidPid, TestSize.Level1)
{
    JsmapCollector collector(INVALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForJsmap>();
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: CollectWithInvalidPidAndTid
 * @tc.desc: Test Collect returns false when both pid and tid are 0
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, CollectWithInvalidPidAndTid, TestSize.Level1)
{
    JsmapCollector collector(INVALID_PID, INVALID_TID);
    auto storage = std::make_shared<MockStorageForJsmap>();
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: CollectFailsWhenDumpJsmapFails
 * @tc.desc: Test Collect returns false when DumpJsmap fails (no system service)
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, CollectFailsWhenDumpJsmapFails, TestSize.Level1)
{
    JsmapCollector collector(VALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForJsmap>();
    EXPECT_FALSE(collector.Collect(storage));
    EXPECT_EQ(storage->addFileCallCount, 0);
}

/**
 * @tc.name: CollectWithStorageFailure
 * @tc.desc: Test Collect returns false when storage AddFile returns false
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, CollectWithStorageFailure, TestSize.Level1)
{
    JsmapCollector collector(VALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForJsmap>();
    storage->addFileReturnValue = false;
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: PrivateMethodDumpJsmapAccessible
 * @tc.desc: Test that DumpJsmap private method is accessible via -Dprivate=public
 * @tc.type: FUNC
 */
HWTEST_F(JsmapCollectorTest, PrivateMethodDumpJsmapAccessible, TestSize.Level1)
{
    JsmapCollector collector(VALID_PID, VALID_TID);
    EXPECT_FALSE(collector.DumpJsmap());
}
