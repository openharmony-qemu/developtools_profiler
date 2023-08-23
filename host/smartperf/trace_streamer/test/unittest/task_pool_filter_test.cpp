/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <unordered_map>

#include "task_pool_filter.h"
#include "trace_streamer_filters.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class TaskPoolFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: CheckTheSameTaskTest
 * @tc.desc: CheckTheSameTask function Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskPoolFilterTest, CheckTheSameTaskTest, TestSize.Level1)
{
    TS_LOGI("test37-1");
    std::unordered_map<int32_t, int32_t> executeMap;
    int32_t executeId = 0;
    uint32_t res = stream_.streamFilters_->taskPoolFilter_->CheckTheSameTask(executeId);
    EXPECT_EQ(res, INVALID_INT32);
}

/**
 * @tc.name: TaskPoolEventTest1
 * @tc.desc: TaskPoolEvent function Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskPoolFilterTest, TaskPoolEventTest1, TestSize.Level1)
{
    TS_LOGI("test37-2");
    std::string taskPoolStr = "H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    int32_t index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    taskPoolStr = "H:Task Perform: taskId : 1, executeId : 1";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    taskPoolStr = "H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: TaskPoolEventTest2
 * @tc.desc: TaskPoolEvent function Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskPoolFilterTest, TaskPoolEventTest2, TestSize.Level1)
{
    TS_LOGI("test37-3");
    std::string taskPoolStr = "H:Task Perform: taskId : 1, executeId : 1";
    int32_t index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    taskPoolStr = "H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    taskPoolStr = "H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: TaskPoolEventTest3
 * @tc.desc: TaskPoolEvent function Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskPoolFilterTest, TaskPoolEventTest3, TestSize.Level1)
{
    TS_LOGI("test37-4");
    std::string taskPoolStr = "H:Task PerformTask End: taskId : 1, executeId : 1, performResult : Successful";
    int32_t index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);

    taskPoolStr = "H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);

    taskPoolStr = "H:Task Perform: taskId : 1, executeId : 1";
    index = 1;
    stream_.streamFilters_->taskPoolFilter_->TaskPoolEvent(taskPoolStr, index);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnTaskIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);
}
} // namespace TraceStreamer
} // namespace SysTuning
