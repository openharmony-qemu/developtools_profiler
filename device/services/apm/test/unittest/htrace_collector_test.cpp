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

#include "htrace_collector.h"
#include "i_data_storage.h"

using namespace testing::ext;

namespace {
const std::string TEST_TRACE_FILE = "/data/local/tmp/htrace_test_trace.htrace";
}

class MockStorageForTrace : public IDataStorage {
public:
    MockStorageForTrace() : addFileCallCount(0), addFileReturnValue(true) {}
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

class TraceCollectorTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ConstructorWithValidPath
 * @tc.desc: Test TraceCollector can be constructed with a trace file path
 * @tc.type: FUNC
 */
HWTEST_F(TraceCollectorTest, ConstructorWithValidPath, TestSize.Level1)
{
    TraceCollector collector(TEST_TRACE_FILE);
    SUCCEED();
}

/**
 * @tc.name: CollectWithNullStorage
 * @tc.desc: Test Collect returns false when storage is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TraceCollectorTest, CollectWithNullStorage, TestSize.Level1)
{
    TraceCollector collector(TEST_TRACE_FILE);
    std::shared_ptr<IDataStorage> nullStorage = nullptr;
    EXPECT_FALSE(collector.Collect(nullStorage));
}

/**
 * @tc.name: CollectWithEmptyTracePath
 * @tc.desc: Test Collect returns false when trace file path is empty
 * @tc.type: FUNC
 */
HWTEST_F(TraceCollectorTest, CollectWithEmptyTracePath, TestSize.Level1)
{
    TraceCollector collector("");
    auto storage = std::make_shared<MockStorageForTrace>();
    EXPECT_FALSE(collector.Collect(storage));
}

/**
 * @tc.name: CollectDelegatesToStorage
 * @tc.desc: Test Collect delegates to storage->AddFile with the trace path
 * @tc.type: FUNC
 */
HWTEST_F(TraceCollectorTest, CollectDelegatesToStorage, TestSize.Level1)
{
    TraceCollector collector(TEST_TRACE_FILE);
    auto storage = std::make_shared<MockStorageForTrace>();
    collector.Collect(storage);
    EXPECT_EQ(storage->addFileCallCount, 1);
    EXPECT_EQ(storage->lastFilePath, TEST_TRACE_FILE);
}

/**
 * @tc.name: CollectReturnsStorageResult
 * @tc.desc: Test Collect returns false when storage->AddFile returns false
 * @tc.type: FUNC
 */
HWTEST_F(TraceCollectorTest, CollectReturnsStorageResult, TestSize.Level1)
{
    TraceCollector collector(TEST_TRACE_FILE);
    auto storage = std::make_shared<MockStorageForTrace>();
    storage->addFileReturnValue = false;
    EXPECT_FALSE(collector.Collect(storage));
}
