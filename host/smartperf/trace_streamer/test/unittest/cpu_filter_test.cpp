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

#include "cpu_filter.h"
#include "process_filter.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class CpuFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.cpuFilter_ = std::make_unique<CpuFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.processFilter_ = std::make_unique<ProcessFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: CpufilterInsertWakeupTest
 * @tc.desc: Test CpuFilter insert WakeupEvent
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertWakeupTest, TestSize.Level1)
{
    TS_LOGI("test4-1");
    /* InsertWakeupEvent ts, internalTid */
    uint64_t ts1 = 168758662877000;
    uint64_t itid = 1;
    streamFilters_.cpuFilter_->InsertWakeupEvent(ts1, itid); // 1st waking

    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itid) == INVALID_UINT64);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itid) == TASK_INVALID);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 0); // 0 thread state only
}

/**
 * @tc.name: CpufilterInsertSwitchTest
 * @tc.desc: Test CpuFilter insert SwitchEvent
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchTest, TestSize.Level1)
{
    TS_LOGI("test4-2");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 2;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 3;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch
    printf("state of pre itid: %llu\n", streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre));

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 0);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 2); // 2 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchFromZeroThread
 * @tc.desc: Test CpuFilter insert SwitchEvent switch from zero thread
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchFromZeroThread, TestSize.Level1)
{
    TS_LOGI("test4-3");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 0;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 3;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch
    printf("state of pre itid: %llu\n", streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre));

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INVALID);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 0);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 1); // 1 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchToZeroThread
 * @tc.desc: Test CpuFilter insert SwitchEvent switch to zero thread
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchToZeroThread, TestSize.Level1)
{
    TS_LOGI("test4-4");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 2;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 0;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch
    printf("state of pre itid: %llu\n", streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre));

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_INVALID);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == INVALID_UINT64);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 1); // 1 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchDoubleThread
 * @tc.desc: Test CpuFilter insert SwitchEvent, A switch to B and B switch to A
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchDoubleThread, TestSize.Level1)
{
    TS_LOGI("test4-5");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 2;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 3;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 0);

    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 2); // 2 thread state

    ts1 = 168758663017000;
    itidPre = 3;
    prePior = 120;
    cpu = 0;
    itidNext = 4;
    nextPior = 120;
    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 2nd switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 2);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidPre) == 3);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 4); // 4 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchThreeThread
 * @tc.desc: Test CpuFilter insert SwitchEvent, A switch to B and B switch to C
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchThreeThread, TestSize.Level1)
{
    TS_LOGI("test4-6");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 2;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 3;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 0);

    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 2); // 2 thread state

    ts1 = 168758663017000;
    itidPre = 3;
    prePior = 120;
    cpu = 0;
    itidNext = 2;
    nextPior = 120;
    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_RUNNABLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 2nd switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 2);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidPre) == 3);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 4); // 4 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchTestZero
 * @tc.desc: Test CpuFilter insert SwitchEvent, A switch to B and B switch to zero thread
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchTestZero, TestSize.Level1)
{
    TS_LOGI("test4-7");
    uint64_t ts1 = 168758662919000;
    uint64_t itidPre = 2;
    uint64_t prePior = 120;
    uint64_t cpu = 0;
    uint64_t itidNext = 3;
    uint64_t nextPior = 124;

    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_INTERRUPTIBLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 1st switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidPre) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 0);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 2); // 2 thread state

    ts1 = 168758663017000;
    itidPre = 0;
    prePior = 120;
    cpu = 0;
    itidNext = 4;
    nextPior = 120;
    streamFilters_.cpuFilter_->InsertSwitchEvent(ts1, cpu, itidPre, prePior, TASK_RUNNABLE, itidNext, nextPior,
                                                 INVALID_DATAINDEX); // 2nd switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itidNext) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itidNext) == 2);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 3); // 4 thread state
}

/**
 * @tc.name: CpufiltertWakeingTest
 * @tc.desc: Test CpuFilter insert Waking Event
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufiltertWakeingTest, TestSize.Level1)
{
    TS_LOGI("test4-8");
    uint64_t ts1 = 168758662919000;
    uint64_t itid = 2;

    streamFilters_.cpuFilter_->InsertWakeupEvent(ts1, itid);

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itid) == TASK_INVALID);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itid) == INVALID_UINT64);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 0); // 0 thread state
}

/**
 * @tc.name: CpufiltertWakingTwice
 * @tc.desc: Test CpuFilter insert Waking Event, one thread waking twice
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufiltertWakingTwice, TestSize.Level1)
{
    TS_LOGI("test4-9");
    uint64_t ts1 = 168758662919000;
    uint64_t itid = 2;

    streamFilters_.cpuFilter_->InsertWakeupEvent(ts1, itid);
    ts1 = 168758662929000;
    itid = 4;
    streamFilters_.cpuFilter_->InsertWakeupEvent(ts1, itid);

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(itid) == TASK_INVALID);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(itid) == INVALID_UINT64);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 0); // 0 thread state
}

/**
 * @tc.name: CpufilterInsertSwitchTestFull
 * @tc.desc: Parsing multiple switch and wakeup alternates
 * @tc.type: FUNC
 */
HWTEST_F(CpuFilterTest, CpufilterInsertSwitchTestFull, TestSize.Level1)
{
    TS_LOGI("test4-10");
    /* InsertWakeupEvent ts, internalTid */
    /* InsertSwitchEvent                         ts,             cpu, prevPid, prevPior, prevState, nextPid, nextPior */
    streamFilters_.cpuFilter_->InsertWakeupEvent(168758662877000, 1); // 1st waking

    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(1) == INVALID_UINT64);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(1) == TASK_INVALID);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 0); // 0 thread state only

    streamFilters_.cpuFilter_->InsertSwitchEvent(168758662919000, 0, 1, 120, TASK_INTERRUPTIBLE, 2, 124,
                                                 INVALID_DATAINDEX); // 1st switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(1) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(2) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(2) == 0);
    // 2 thread state, the waking event add a runnable state
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 2);
    streamFilters_.cpuFilter_->InsertSwitchEvent(168758663017000, 0, 0, 120, TASK_RUNNABLE, 4, 120,
                                                 INVALID_DATAINDEX); // 2nd switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(4) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(4) == 2);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 3); // 4 thread state

    streamFilters_.cpuFilter_->InsertWakeupEvent(168758663078000, 0); // 2nd waking

    streamFilters_.cpuFilter_->InsertWakeupEvent(168758663092000, 0); // 3rd waking

    streamFilters_.cpuFilter_->InsertSwitchEvent(168758663107000, 0, 2, 124, TASK_RUNNABLE, 5, 98,
                                                 INVALID_DATAINDEX); // 3rd switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(5) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(2) == TASK_RUNNABLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(5) == 3);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(2) == 4);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 5); // 6 thread state

    streamFilters_.cpuFilter_->InsertSwitchEvent(168758663126000, 0, 5, 98, TASK_INTERRUPTIBLE, 2, 124,
                                                 INVALID_DATAINDEX); // 4th switch
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(2) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(5) == TASK_INTERRUPTIBLE);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(2) == 5);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(5) == 6);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 7); // 8 thread state

    streamFilters_.cpuFilter_->InsertSwitchEvent(168758663136000, 3, 5, 120, TASK_RUNNABLE, 6, 120,
                                                 INVALID_DATAINDEX); // 5th switch

    EXPECT_TRUE(streamFilters_.cpuFilter_->StateOfInternalTidInStateTable(6) == TASK_RUNNING);
    EXPECT_TRUE(streamFilters_.cpuFilter_->RowOfInternalTidInStateTable(6) == 7);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->Size() == 9); // 10 thread state

    // after 3rd switch
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->DursData()[1] == INVALID_UINT64);
    // after 4th switch
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->DursData()[6] == 168758663136000 - 168758663126000);
    EXPECT_TRUE(traceDataCache_.GetThreadStateData()->DursData()[7] == INVALID_UINT64);
}
} // namespace TraceStreamer
} // namespace SysTuning
