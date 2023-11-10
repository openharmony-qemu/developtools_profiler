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

#include "filter_filter.h"
#include "measure_filter.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
constexpr int32_t CPU_ID_0 = 0;
constexpr int32_t CPU_ID_1 = 1;
constexpr std::string_view CPU_TYPE_0 = "cpu_idle";
constexpr std::string_view CPU_TYPE_1 = "cpu_frequency";
constexpr int32_t INTERNAL_THREAD_ID_0 = 1;
constexpr int32_t INTERNAL_THREAD_ID_1 = 2;
constexpr int32_t INTERNAL_PROCESS_ID_0 = 1;
constexpr int32_t INTERNAL_PROCESS_ID_1 = 2;
constexpr std::string_view TASK_NAME_0 = "softbus_server";
constexpr std::string_view TASK_NAME_1 = "hiprofilerd";

class MeasureFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: ThreadMeasureFilter
 * @tc.desc: Test whether the GetOrCreateFilterId interface generated filterid and threadmeasure Info is correct
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ThreadMeasureFilter, TestSize.Level1)
{
    TS_LOGI("test23-1");
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId =
        stream_.streamFilters_->threadMeasureFilter_->GetOrCreateFilterId(INTERNAL_THREAD_ID_0, nameIndex0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->threadMeasureFilter_->GetOrCreateFilterId(INTERNAL_THREAD_ID_1, nameIndex1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ThreadMeasureFilter* threadMeasureTable = stream_.traceDataCache_->GetThreadMeasureFilterData();
    EXPECT_TRUE(threadMeasureTable->Size() == 2);
    EXPECT_TRUE(threadMeasureTable->FilterIdData()[0] == 0);
    EXPECT_TRUE(threadMeasureTable->FilterIdData()[1] == 1);
    EXPECT_TRUE(threadMeasureTable->InternalTidData()[0] == INTERNAL_THREAD_ID_0);
    EXPECT_TRUE(threadMeasureTable->InternalTidData()[1] == INTERNAL_THREAD_ID_1);
}

/**
 * @tc.name: ThreadFilter
 * @tc.desc: Test whether the GetOrCreateFilterId interface generated filterid and thread Info is correct
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ThreadFilter, TestSize.Level1)
{
    TS_LOGI("test23-2");
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId = stream_.streamFilters_->threadFilter_->GetOrCreateFilterId(INTERNAL_THREAD_ID_0, nameIndex0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->threadFilter_->GetOrCreateFilterId(INTERNAL_THREAD_ID_1, nameIndex1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ThreadMeasureFilter* threadTable = stream_.traceDataCache_->GetThreadFilterData();
    EXPECT_TRUE(threadTable->Size() == 2);
    EXPECT_TRUE(threadTable->FilterIdData()[0] == 0);
    EXPECT_TRUE(threadTable->FilterIdData()[1] == 1);
    EXPECT_TRUE(threadTable->InternalTidData()[0] == INTERNAL_THREAD_ID_0);
    EXPECT_TRUE(threadTable->InternalTidData()[1] == INTERNAL_THREAD_ID_1);
}

/**
 * @tc.name: CpuFilter
 * @tc.desc: Test GetOrCreateFilterId interface of class CpuFilter
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, CpuFilter, TestSize.Level1)
{
    TS_LOGI("test23-3");
    auto nameIndex_0 = stream_.traceDataCache_->GetDataIndex(CPU_TYPE_0);
    uint32_t filterId = stream_.streamFilters_->cpuMeasureFilter_->GetOrCreateFilterId(CPU_ID_0, nameIndex_0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex_1 = stream_.traceDataCache_->GetDataIndex(CPU_TYPE_1);
    filterId = stream_.streamFilters_->cpuMeasureFilter_->GetOrCreateFilterId(CPU_ID_1, nameIndex_1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    CpuMeasureFilter* cpuMeasureTable = stream_.traceDataCache_->GetCpuMeasuresData();
    EXPECT_TRUE(cpuMeasureTable->Size() == 2);
    EXPECT_TRUE(cpuMeasureTable->IdsData()[0] == 0);
    EXPECT_TRUE(cpuMeasureTable->IdsData()[1] == 1);
    EXPECT_TRUE(cpuMeasureTable->CpuData()[0] == CPU_ID_0);
    EXPECT_TRUE(cpuMeasureTable->CpuData()[1] == CPU_ID_1);
}

/**
 * @tc.name: ProcessFilter
 * @tc.desc: Test GetOrCreateFilterId interface of class ProcessFilter
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ProcessFilter, TestSize.Level1)
{
    TS_LOGI("test23-4");
    auto nameIndex_0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId =
        stream_.streamFilters_->processFilterFilter_->GetOrCreateFilterId(INTERNAL_PROCESS_ID_0, nameIndex_0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex_1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->processFilterFilter_->GetOrCreateFilterId(INTERNAL_PROCESS_ID_1, nameIndex_1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ProcessMeasureFilter* processFilterTable = stream_.traceDataCache_->GetProcessFilterData();
    EXPECT_TRUE(processFilterTable->Size() == 2);
}

/**
 * @tc.name: ClockRateFilter
 * @tc.desc: Test GetOrCreateFilterId interface of class ClockRateFilter
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ClockRateFilter, TestSize.Level1)
{
    TS_LOGI("test23-5");
    auto nameIndex_0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId = stream_.streamFilters_->clockRateFilter_->GetOrCreateFilterId(CPU_ID_0, nameIndex_0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex_1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->clockRateFilter_->GetOrCreateFilterId(CPU_ID_1, nameIndex_1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ClockEventData* clockEventTable = stream_.traceDataCache_->GetClockEventFilterData();
    EXPECT_TRUE(clockEventTable->Size() == 2);
    EXPECT_TRUE(clockEventTable->CpusData()[0] == CPU_ID_0);
    EXPECT_TRUE(clockEventTable->CpusData()[1] == CPU_ID_1);
    EXPECT_TRUE(clockEventTable->NamesData()[0] == nameIndex_0);
    EXPECT_TRUE(clockEventTable->NamesData()[1] == nameIndex_1);
}

/**
 * @tc.name: ClockEnableFilter
 * @tc.desc: Test GetOrCreateFilterId interface of class ClockEnableFilter
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ClockEnableFilter, TestSize.Level1)
{
    TS_LOGI("test23-6");
    auto nameIndex_0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId = stream_.streamFilters_->clockEnableFilter_->GetOrCreateFilterId(CPU_ID_0, nameIndex_0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex_1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->clockEnableFilter_->GetOrCreateFilterId(CPU_ID_1, nameIndex_1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ClockEventData* clockEventTable = stream_.traceDataCache_->GetClockEventFilterData();
    EXPECT_TRUE(clockEventTable->Size() == 2);
    EXPECT_TRUE(clockEventTable->CpusData()[0] == CPU_ID_0);
    EXPECT_TRUE(clockEventTable->CpusData()[1] == CPU_ID_1);
    EXPECT_TRUE(clockEventTable->NamesData()[0] == nameIndex_0);
    EXPECT_TRUE(clockEventTable->NamesData()[1] == nameIndex_1);
}

/**
 * @tc.name: ClockDisableFilter
 * @tc.desc: Test GetOrCreateFilterId interface of class ClockDisableFilter
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, ClockDisableFilter, TestSize.Level1)
{
    TS_LOGI("test23-7");
    auto nameIndex_0 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_0);
    uint32_t filterId = stream_.streamFilters_->clockDisableFilter_->GetOrCreateFilterId(CPU_ID_0, nameIndex_0);
    EXPECT_TRUE(filterId == 0);

    auto nameIndex_1 = stream_.traceDataCache_->GetDataIndex(TASK_NAME_1);
    filterId = stream_.streamFilters_->clockDisableFilter_->GetOrCreateFilterId(CPU_ID_1, nameIndex_1);
    EXPECT_TRUE(filterId == 1);

    Filter* filterTable = stream_.traceDataCache_->GetFilterData();
    EXPECT_TRUE(filterTable->Size() == 2);

    ClockEventData* clockEventTable = stream_.traceDataCache_->GetClockEventFilterData();
    EXPECT_TRUE(clockEventTable->Size() == 2);
    EXPECT_TRUE(clockEventTable->CpusData()[0] == CPU_ID_0);
    EXPECT_TRUE(clockEventTable->CpusData()[1] == CPU_ID_1);
    EXPECT_TRUE(clockEventTable->NamesData()[0] == nameIndex_0);
    EXPECT_TRUE(clockEventTable->NamesData()[1] == nameIndex_1);
}

/**
 * @tc.name: MeasureFilterTest
 * @tc.desc: Test GetOrCreateFilterId interface of class MeasureFilterTest
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, MeasureFilterTest, TestSize.Level1)
{
    TS_LOGI("test23-8");
    uint64_t itid = 1;
    const std::string_view MEASURE_ITEM_NAME = "mem_rss";
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME);
    auto threadMeasureFilter = stream_.streamFilters_->processMeasureFilter_.get();
    threadMeasureFilter->AppendNewMeasureData(itid, nameIndex0, 168758682476000, 1200);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == 1);
}

/**
 * @tc.name: MeasureFilterAddMultiMemToSingleThread
 * @tc.desc: Test GetOrCreateFilterId interface of class MeasureFilterTest, Adding multiple memory information tests to
 * the same thread
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, MeasureFilterAddMultiMemToSingleThread, TestSize.Level1)
{
    TS_LOGI("test23-9");
    uint64_t itid = 1;
    auto threadMeasureFilter = stream_.streamFilters_->processMeasureFilter_.get();
    const std::string_view MEASURE_ITEM_NAME = "mem_rss";
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME);
    threadMeasureFilter->AppendNewMeasureData(itid, nameIndex0, 168758682476000, 1200);
    const std::string_view MEASURE_ITEM_NAME2 = "mem_vm";
    auto nameIndex1 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME2);
    threadMeasureFilter->AppendNewMeasureData(itid, nameIndex1, 168758682477000, 9200);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == 2);
}

/**
 * @tc.name: MeasureFilterAddMultiMemToMultiThread
 * @tc.desc: Test GetOrCreateFilterId interface of class MeasureFilterTest, Adding multiple memory information to multi
 * thread
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, MeasureFilterAddMultiMemToMultiThread, TestSize.Level1)
{
    TS_LOGI("test23-10");
    uint64_t itid = 1;
    uint64_t itid2 = 2;
    auto threadMeasureFilter = stream_.streamFilters_->processMeasureFilter_.get();
    const std::string_view MEASURE_ITEM_NAME = "mem_rss";
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME);
    threadMeasureFilter->AppendNewMeasureData(itid, nameIndex0, 168758682476000, 1200);
    const std::string_view MEASURE_ITEM_NAME2 = "mem_vm";
    auto nameIndex1 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME2);
    threadMeasureFilter->AppendNewMeasureData(itid2, nameIndex1, 168758682477000, 9200);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().ValuesData()[0] == 1200);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().ValuesData()[1] == 9200);
}

/**
 * @tc.name: MeasureFilterAddPerfclLfMux
 * @tc.desc: Add perfcl_ lf_mux status test
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, MeasureFilterAddPerfclLfMux, TestSize.Level1)
{
    TS_LOGI("test23-11");
    uint64_t cpuId = 1;
    int64_t state = 0;
    auto threadMeasureFilter = stream_.streamFilters_->clockDisableFilter_.get();
    const std::string_view MEASURE_ITEM_NAME = "perfcl_lf_mux";
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME);
    threadMeasureFilter->AppendNewMeasureData(cpuId, nameIndex0, 168758682476000, state);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstMeasureData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstMeasureData().ValuesData()[0] == state);
}

/**
 * @tc.name: MeasureFilterAddPerfclPll
 * @tc.desc: Add perfcl_pll status test
 * @tc.type: FUNC
 */
HWTEST_F(MeasureFilterTest, MeasureFilterAddPerfclPll, TestSize.Level1)
{
    TS_LOGI("test23-12");
    uint64_t cpuId = 1;
    int64_t state = 1747200000;
    auto threadMeasureFilter = stream_.streamFilters_->clockRateFilter_.get();
    const std::string_view MEASURE_ITEM_NAME = "perfcl_pll";
    auto nameIndex0 = stream_.traceDataCache_->GetDataIndex(MEASURE_ITEM_NAME);
    threadMeasureFilter->AppendNewMeasureData(cpuId, nameIndex0, 168758682476000, state);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstMeasureData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstMeasureData().ValuesData()[0] == state);
}
} // namespace TraceStreamer
} // namespace SysTuning
