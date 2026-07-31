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
#include <fstream>
#include <filesystem>

#include "rawheap_collector.h"
#include "i_data_storage.h"

using namespace testing::ext;

namespace {
constexpr uint32_t VALID_PID = 1000;
constexpr uint32_t VALID_TID = 1001;
constexpr uint32_t INVALID_PID = 0;
constexpr uint32_t INVALID_TID = 0;
const std::string TEST_RAWHEAP_FILE = "/data/local/tmp/rawheap_collector_test.rawheap";
const std::string TEST_METADATA_PATH = "/data/local/tmp/rawheap_collector_metadata.json";
}

class MockStorageForRawheap : public IDataStorage {
public:
    MockStorageForRawheap() : addFileCallCount(0), addFileReturnValue(true) {}
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

class RawheapCollectorTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ConstructorWithValidPidTid
 * @tc.desc: Test RawheapCollector can be constructed with pid and tid
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, ConstructorWithValidPidTid, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    SUCCEED();
}

/**
 * @tc.name: CollectWithNullStorage
 * @tc.desc: Test Collect returns false when storage is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, CollectWithNullStorage, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    std::shared_ptr<IDataStorage> nullStorage = nullptr;
    EXPECT_FALSE(collector.Collect(nullStorage));
}

/**
 * @tc.name: CollectWithInvalidPid
 * @tc.desc: Test Collect returns false when pid is 0
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, CollectWithInvalidPid, TestSize.Level1)
{
    RawheapCollector collector(INVALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForRawheap>();
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: CollectWithInvalidPidAndTid
 * @tc.desc: Test Collect returns false when both pid and tid are 0
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, CollectWithInvalidPidAndTid, TestSize.Level1)
{
    RawheapCollector collector(INVALID_PID, INVALID_TID);
    auto storage = std::make_shared<MockStorageForRawheap>();
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: CollectFailsWhenDumpRawheapFails
 * @tc.desc: Test Collect returns false when DumpRawheap fails (no system service)
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, CollectFailsWhenDumpRawheapFails, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForRawheap>();
    EXPECT_FALSE(collector.Collect(storage));
    EXPECT_EQ(storage->addFileCallCount, 0);
}

/**
 * @tc.name: CollectWithStorageFailure
 * @tc.desc: Test Collect returns false when storage AddFile returns false
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, CollectWithStorageFailure, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    auto storage = std::make_shared<MockStorageForRawheap>();
    storage->addFileReturnValue = false;
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: PrivateMethodsAccessible
 * @tc.desc: Test that DumpRawheap and AppendMetaData private methods are accessible via -Dprivate=public
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, PrivateMethodsAccessible, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    EXPECT_FALSE(collector.DumpRawheap());
}

/**
 * @tc.name: AppendMetaDataNonExistentFile
 * @tc.desc: Test AppendMetaData returns false for non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(RawheapCollectorTest, AppendMetaDataNonExistentFile, TestSize.Level1)
{
    RawheapCollector collector(VALID_PID, VALID_TID);
    EXPECT_FALSE(collector.AppendMetaData("/data/local/tmp/non_existent_rawheap_file.rawheap"));
}
