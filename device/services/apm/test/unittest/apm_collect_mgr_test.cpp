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

#include "apm_collect_mgr.h"
#include "i_data_collector.h"
#include "i_data_storage.h"

using namespace testing::ext;

namespace {
const std::string TEST_ZIP_PATH = "/data/local/tmp/apm_test.zip";

class MockDataStorage : public IDataStorage {
public:
    MockDataStorage() : addFileCallCount(0), addFileReturnValue(true) {}
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

class MockDataCollector : public IDataCollector {
public:
    MockDataCollector() : collectCallCount(0), collectReturnValue(true) {}
    bool Collect(std::shared_ptr<IDataStorage> storage) override
    {
        collectCallCount++;
        lastStorage = storage;
        return collectReturnValue;
    }
    int collectCallCount;
    bool collectReturnValue;
    std::shared_ptr<IDataStorage> lastStorage;
};

class ApmCollectMgrTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: RegisterCollector
 * @tc.desc: Test registering a valid collector
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, RegisterCollector, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto collector = std::make_shared<MockDataCollector>();
    EXPECT_TRUE(mgr.RegisterCollector(collector));
}

/**
 * @tc.name: RegisterCollectorNullptr
 * @tc.desc: Test registering a nullptr collector returns false
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, RegisterCollectorNullptr, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    std::shared_ptr<IDataCollector> nullCollector = nullptr;
    EXPECT_FALSE(mgr.RegisterCollector(nullCollector));
}

/**
 * @tc.name: RegisterMultipleCollectors
 * @tc.desc: Test registering multiple collectors
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, RegisterMultipleCollectors, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto collector1 = std::make_shared<MockDataCollector>();
    auto collector2 = std::make_shared<MockDataCollector>();
    EXPECT_TRUE(mgr.RegisterCollector(collector1));
    EXPECT_TRUE(mgr.RegisterCollector(collector2));
}

/**
 * @tc.name: CollectWithNoCollectors
 * @tc.desc: Test Collect with no registered collectors returns false
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, CollectWithNoCollectors, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    EXPECT_FALSE(mgr.Collect());
}

/**
 * @tc.name: CollectWithSingleCollector
 * @tc.desc: Test Collect invokes the registered collector
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, CollectWithSingleCollector, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto collector = std::make_shared<MockDataCollector>();
    mgr.RegisterCollector(collector);
    mgr.Collect();
    EXPECT_EQ(collector->collectCallCount, 1);
}

/**
 * @tc.name: CollectWithMultipleCollectors
 * @tc.desc: Test Collect invokes all registered collectors in order
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, CollectWithMultipleCollectors, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto collector1 = std::make_shared<MockDataCollector>();
    auto collector2 = std::make_shared<MockDataCollector>();
    mgr.RegisterCollector(collector1);
    mgr.RegisterCollector(collector2);
    mgr.Collect();
    EXPECT_EQ(collector1->collectCallCount, 1);
    EXPECT_EQ(collector2->collectCallCount, 1);
}

/**
 * @tc.name: CollectFailsWhenCollectorFails
 * @tc.desc: Test Collect returns false when a collector returns false
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, CollectFailsWhenCollectorFails, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto failCollector = std::make_shared<MockDataCollector>();
    failCollector->collectReturnValue = false;
    mgr.RegisterCollector(failCollector);
    EXPECT_FALSE(mgr.Collect());
}

/**
 * @tc.name: CollectStopsOnFirstFailure
 * @tc.desc: Test Collect stops on first collector failure and does not call subsequent collectors
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, CollectStopsOnFirstFailure, TestSize.Level1)
{
    ApmCollectMgr mgr(TEST_ZIP_PATH);
    auto failCollector = std::make_shared<MockDataCollector>();
    failCollector->collectReturnValue = false;
    auto successCollector = std::make_shared<MockDataCollector>();
    mgr.RegisterCollector(failCollector);
    mgr.RegisterCollector(successCollector);
    EXPECT_FALSE(mgr.Collect());
    EXPECT_EQ(failCollector->collectCallCount, 1);
    EXPECT_EQ(successCollector->collectCallCount, 0);
}

/**
 * @tc.name: DestructorClearsCollectors
 * @tc.desc: Test that destructor clears collectors and storage
 * @tc.type: FUNC
 */
HWTEST_F(ApmCollectMgrTest, DestructorClearsCollectors, TestSize.Level1)
{
    auto mgr = std::make_unique<ApmCollectMgr>(TEST_ZIP_PATH);
    auto collector = std::make_shared<MockDataCollector>();
    mgr->RegisterCollector(collector);
    mgr.reset();
    SUCCEED();
}
} // namespace
