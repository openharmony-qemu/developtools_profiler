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
class TaskPoolData {
public:
    TaskPoolData(InternalTid expectAllocationItid,
                 InternalTid expectExecuteItid,
                 InternalTid expectReturnItid,
                 uint32_t executeId,
                 uint32_t priority,
                 uint32_t executeState,
                 uint32_t returnState)
        : expectAllocationItid_(expectAllocationItid),
          expectExecuteItid_(expectExecuteItid),
          expectReturnItid_(expectReturnItid),
          executeId_(executeId),
          priority_(priority),
          executeState_(executeState),
          returnState_(returnState){};
    TaskPoolData(size_t index, const TaskPoolInfo* taskpool)
        : expectAllocationItid_(taskpool->allocationItids_[index]),
          expectExecuteItid_(taskpool->executeItids_[index]),
          expectReturnItid_(taskpool->returnItids_[index]),
          executeId_(taskpool->executeIds_[index]),
          priority_(taskpool->prioritys_[index]),
          executeState_(taskpool->executeStates_[index]),
          returnState_(taskpool->returnStates_[index]){};
    friend bool operator==(const TaskPoolData& first, const TaskPoolData& second);

private:
    InternalTid expectAllocationItid_;
    InternalTid expectExecuteItid_;
    InternalTid expectReturnItid_;
    uint32_t executeId_;
    uint32_t priority_;
    uint32_t executeState_;
    uint32_t returnState_;
};

bool operator==(const TaskPoolData& first, const TaskPoolData& second)
{
    if (first.expectAllocationItid_ != second.expectAllocationItid_) {
        return false;
    }
    if (first.expectExecuteItid_ != second.expectExecuteItid_) {
        return false;
    }
    if (first.expectReturnItid_ != second.expectReturnItid_) {
        return false;
    }
    if (first.executeId_ != second.executeId_) {
        return false;
    }
    if (first.priority_ != second.priority_) {
        return false;
    }
    if (first.executeState_ != second.executeState_) {
        return false;
    }
    if (first.returnState_ != second.returnState_) {
        return false;
    }
    return true;
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

    TaskPoolData firstResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData firstExpect(1, INVALID_INT32, INVALID_INT32, 9, 1, 1, INVALID_INT32);
    EXPECT_TRUE(firstResult == firstExpect);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Perform: taskId : 1, executeId : 9";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);

    TaskPoolData secondResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData secondExpect(1, 1, INVALID_INT32, 9, 1, 1, INVALID_INT32);
    EXPECT_TRUE(secondResult == secondExpect);

    comm = "TaskWorkThread";
    taskPoolStr = "H:Task PerformTask End: taskId : 1, executeId : 9, performResult : IsCanceled";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    TaskPoolData thirdResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData thirdExpect(1, 1, INVALID_INT32, 9, 1, 1, INVALID_INT32);
    EXPECT_TRUE(thirdResult == thirdExpect);
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
    TaskPoolData firstResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData firstExpect(INVALID_INT32, 1, INVALID_INT32, 1, INVALID_INT32, INVALID_INT32, INVALID_INT32);
    EXPECT_TRUE(firstResult == firstExpect);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    TaskPoolData secondResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData secondExpect(1, 1, INVALID_INT32, 1, 1, 1, INVALID_INT32);
    EXPECT_TRUE(secondResult == secondExpect);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    TaskPoolData thirdResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData thirdExpect(1, 1, 1, 1, 1, 1, 0);
    EXPECT_TRUE(thirdResult == thirdExpect);
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
    TaskPoolData firstResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData firstExpect(INVALID_INT32, INVALID_INT32, 1, 1, INVALID_INT32, INVALID_INT32, 1);
    EXPECT_TRUE(firstResult == firstExpect);

    comm = "e.myapplication";
    taskPoolStr = "B|16502|H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    TaskPoolData secondResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData secondExpect(1, INVALID_INT32, 1, 1, 1, 1, 1);
    EXPECT_TRUE(secondResult == secondExpect);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Task Perform: taskId : 1, executeId : 1";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    TaskPoolData thirdResult(0, stream_.traceDataCache_->GetTaskPoolData());
    TaskPoolData thirdExpect(1, 1, 1, 1, 1, 1, 1);
    EXPECT_TRUE(thirdResult == thirdExpect);

    comm = "TaskWorkThread";
    taskPoolStr = "B|16502|H:Thread Timeout Exit";
    printEvent.ParsePrintEvent(comm, ts, pid, taskPoolStr, line);
    auto res = stream_.traceDataCache_->GetTaskPoolData()->timeoutRows_[0];
    EXPECT_EQ(res, 3);
}
} // namespace TraceStreamer
} // namespace SysTuning
