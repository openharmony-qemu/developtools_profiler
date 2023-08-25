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

#include "print_event_parser.h"
#include "task_pool_filter.h"
#include "trace_streamer_filters.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
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
    DoubleMap<InternalPid, uint32_t, uint32_t> executeMap(INVALID_INT32);
    int32_t executeId = 0;
    uint32_t index = 0;
    uint32_t res = stream_.streamFilters_->taskPoolFilter_->CheckTheSameTask(executeId, index);
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
    std::string comm("e.myapplication");
    uint64_t ts = 89227707307481;
    uint32_t pid = 16502;
    std::string taskPoolStr("B|16502|H:Task Allocation: taskId : 1, executeId : 9, priority : 1, executeState : 1");
    BytraceLine line;
    stream_.traceDataCache_->taskPoolTraceEnabled_ = true;
    PrintEventParser printEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 9);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Perform: taskId : 1, executeId : 9";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 9);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    comm = "TaskWorkThread";
    taskPoolStr = "H:Task PerformTask End: taskId : 1, executeId : 9, performResult : IsCanceled";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 9);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);
}

/**
 * @tc.name: TaskPoolEventTest2
 * @tc.desc: TaskPoolEvent function Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskPoolFilterTest, TaskPoolEventTest2, TestSize.Level1)
{
    TS_LOGI("test37-3");
    std::string comm("e.myapplication");
    uint64_t ts = 89227707307481;
    uint32_t pid = 16502;
    std::string taskPoolStr("B|16502|H:Task Perform: taskId : 1, executeId : 1");
    BytraceLine line;
    stream_.traceDataCache_->taskPoolTraceEnabled_ = true;
    PrintEventParser printEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, INVALID_INT32);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
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
    std::string comm("e.myapplication");
    uint64_t ts = 89227707307481;
    uint32_t pid = 16502;
    std::string taskPoolStr("B|16502|H:Task PerformTask End: taskId : 1, executeId : 1, performResult : Successful");
    BytraceLine line;
    stream_.traceDataCache_->taskPoolTraceEnabled_ = true;
    PrintEventParser printEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, INVALID_INT32);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Task Perform: taskId : 1, executeId : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->allocationItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnItids_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeIds_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->prioritys_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->executeStates_[0];
    EXPECT_EQ(res, 1);
    res = stream_.traceDataCache_->GetTaskPoolData()->returnStates_[0];
    EXPECT_EQ(res, 1);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Thread Timeout Exit";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    res = stream_.traceDataCache_->GetTaskPoolData()->timeoutRows_[0];
    EXPECT_EQ(res, 3);
}
} // namespace TraceStreamer
} // namespace SysTuning
