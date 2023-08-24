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
#include "process_filter.h"
#include "slice_filter.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class SliceFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    TraceStreamerSelector stream_;
};

/**
 * @tc.name: SliceTestOnceCall
 * @tc.desc: Parse once method call stack
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, SliceTestOnceCall, TestSize.Level1)
{
    TS_LOGI("test28-1");
    uint64_t ts = 168758662957000;
    uint64_t ts2 = 168758663011000;

    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("call_function_one");
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts2, pid1, threadGroupId1);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 1);
    EXPECT_TRUE(slices->DursData()[0] == ts2 - ts);
}

/**
 * @tc.name: SliceTestNestedTwoMethod
 * @tc.desc: Parse Nested call stack of two methods
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, SliceTestNestedTwoMethod, TestSize.Level1)
{
    TS_LOGI("test28-2");
    uint64_t ts1 = 168758670506000;
    uint32_t pid1 = 1298;
    uint32_t threadGroupId1 = 1298;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("call_function_one");
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, splitStrIndex);
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("call_function_two");
    uint64_t ts2 = 168758670523000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts2, pid1, threadGroupId1, cat, splitStrIndex);
    uint64_t ts3 = 168758670720000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts3, pid1, threadGroupId1);
    uint64_t ts4 = 168758670732000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts4, pid1, threadGroupId1);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 2);
    EXPECT_TRUE(slices->DursData()[0] == ts4 - ts1);
    EXPECT_TRUE(slices->DursData()[1] == ts3 - ts2);
    EXPECT_TRUE(slices->Depths()[1] == 1);
}

/**
 * @tc.name: SliceTestNestedTwoMethodStackAndOneMethodStack
 * @tc.desc: Parse Nested call stack of two methods and one method call stack
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, SliceTestNestedTwoMethodStackAndOneMethodStack, TestSize.Level1)
{
    TS_LOGI("test28-3");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_one");
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_one");
    uint64_t ts2 = 168758663028000;
    uint32_t pid2 = 2533;
    uint32_t threadGroupId2 = 2529;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts2, pid2, threadGroupId2, cat, splitStrIndex);
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_two");
    uint64_t ts3 = 168758679303000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts3, pid1, threadGroupId1, cat, splitStrIndex); // slice 2
    // end thread_one_call_function_two
    uint64_t ts4 = 168758682466000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts4, pid1, threadGroupId1);
    // end thread_one_call_function_one
    uint64_t ts5 = 168758682476000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts5, pid1, threadGroupId1);
    // end thread_two_call_function_one slice 1
    uint64_t ts6 = 168758689323000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts6, pid2, threadGroupId2);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 3);
    EXPECT_TRUE(slices->DursData()[0] == ts5 - ts1); // slice 0
    EXPECT_TRUE(slices->Depths()[0] == 0);
    EXPECT_TRUE(slices->DursData()[1] == ts6 - ts2); // slice 1
    EXPECT_TRUE(slices->Depths()[1] == 0);
    EXPECT_TRUE(slices->DursData()[2] == ts4 - ts3); // slice 2
    EXPECT_TRUE(slices->Depths()[2] == 1);
}

/**
 * @tc.name: SliceTestWithoutBeginSlice
 * @tc.desc: Test EndSlice without BeginSlice
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, SliceTestWithoutBeginSlice, TestSize.Level1)
{
    TS_LOGI("test28-4");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts1, pid1, threadGroupId1);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 0);
}

/**
 * @tc.name: SliceTestWithMultiNestedCall
 * @tc.desc: Parse multi nested call stack
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, SliceTestWithMultiNestedCall, TestSize.Level1)
{
    TS_LOGI("test28-5");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_one");
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_one");
    uint64_t ts2 = 168758663028000;
    uint32_t pid2 = 2533;
    uint32_t threadGroupId2 = 2529;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts2, pid2, threadGroupId2, cat, splitStrIndex);
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_two");
    uint64_t ts3 = 168758679303000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts3, pid1, threadGroupId1, cat, splitStrIndex); // slice 2
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_two");
    uint64_t ts4 = 168758679312000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts4, pid2, threadGroupId2, cat, splitStrIndex);
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_three");
    uint64_t ts5 = 168758679313000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts5, pid1, threadGroupId1, cat, splitStrIndex); // slice 4
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_three");
    uint64_t ts6 = 168758679323000;
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts6, pid2, threadGroupId2, cat, splitStrIndex);
    // end thread_one_call_function_three
    uint64_t ts7 = 168758682456000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts7, pid1, threadGroupId1);
    // end thread_one_call_function_two
    uint64_t ts8 = 168758682466000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts8, pid1, threadGroupId1);
    // end thread_one_call_function_one
    uint64_t ts9 = 168758682476000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts9, pid1, threadGroupId1);
    // end thread_two_call_function_three slice 5
    uint64_t ts10 = 168758679343000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts10, pid2, threadGroupId2);
    // end thread_two_call_function_two slice 3
    uint64_t ts11 = 168758679344000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts11, pid2, threadGroupId2);
    // end thread_two_call_function_one slice 1
    uint64_t ts12 = 168758689323000;
    stream_.streamFilters_->sliceFilter_->EndSlice(ts12, pid2, threadGroupId2);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 6);
    EXPECT_TRUE(slices->DursData()[0] == ts9 - ts1); // slice 0
    EXPECT_TRUE(slices->Depths()[0] == 0);
    EXPECT_TRUE(slices->DursData()[1] == ts12 - ts2); // slice 1
    EXPECT_TRUE(slices->Depths()[1] == 0);
    EXPECT_TRUE(slices->DursData()[2] == ts8 - ts3); // slice 2
    EXPECT_TRUE(slices->Depths()[2] == 1);
    EXPECT_TRUE(slices->DursData()[3] == ts11 - ts4); // slice 3
    EXPECT_TRUE(slices->Depths()[3] == 1);
    EXPECT_TRUE(slices->DursData()[4] == ts7 - ts5);  // slice 4
    EXPECT_TRUE(slices->DursData()[5] == ts10 - ts6); // slice 5
}

/**
 * @tc.name: AsyncTest
 * @tc.desc: Test once asynchronous method call stack
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, AsyncTest, TestSize.Level1)
{
    TS_LOGI("test28-6");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("async_call_function_one");
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0
    splitStrIndex = stream_.traceDataCache_->GetDataIndex("async_call_function_one");
    // end thread_one_call_function_three
    uint64_t ts2 = 168758682456000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts2, pid1, threadGroupId1, cat, splitStrIndex);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 1);
    EXPECT_TRUE(slices->DursData()[0] == ts2 - ts1); // slice 0
}

/**
 * @tc.name: FinishAsyncSliceWithoutStart
 * @tc.desc: Finish async slice without start
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, FinishAsyncSliceWithoutStart, TestSize.Level1)
{
    TS_LOGI("test28-7");
    uint64_t ts = 100;
    uint32_t pid = 2532;
    uint32_t threadGroupId = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("async_call_function_one");
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts, pid, threadGroupId, cat, splitStrIndex);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 0);
}

/**
 * @tc.name: AsyncTestTwiceCallStack
 * @tc.desc: Test Twice asynchronous call stack
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, AsyncTestTwiceCallStack, TestSize.Level1)
{
    TS_LOGI("test28-8");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_one");
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0
    DataIndex splitStrIndex2 = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_one");
    uint64_t ts2 = 168758663028000;
    uint32_t pid2 = 2533;
    uint32_t threadGroupId2 = 2529;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts2, pid2, threadGroupId2, cat, splitStrIndex2);
    // end thread_one_call_function_three
    uint64_t ts3 = 168758682456000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts3, pid1, threadGroupId1, cat, splitStrIndex);
    // end thread_two_call_function_three slice 5
    uint64_t ts4 = 168758679343000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts4, pid2, threadGroupId2, cat, splitStrIndex2);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 2);
    EXPECT_TRUE(slices->DursData()[0] == ts3 - ts1); // slice 0
    EXPECT_TRUE(slices->Depths()[0] == 0);
    EXPECT_TRUE(slices->DursData()[1] == ts4 - ts2); // slice 1
}

/**
 * @tc.name: BeginAsyncSliceThreeTimes
 * @tc.desc: Test asynchronous call three times
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, BeginAsyncSliceThreeTimes, TestSize.Level1)
{
    TS_LOGI("test28-9");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_one");
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0
    DataIndex splitStrIndex2 = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_one");
    uint64_t ts2 = 168758663028000;
    uint32_t pid2 = 2533;
    uint32_t threadGroupId2 = 2529;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts2, pid2, threadGroupId2, cat, splitStrIndex2);
    DataIndex splitStrIndex3 = stream_.traceDataCache_->GetDataIndex("thread_three_call_function_two");
    DataIndex cat2 = stream_.traceDataCache_->GetDataIndex("Catalog2");
    uint64_t ts3 = 168758679303000;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts3, pid1, threadGroupId1, cat2, splitStrIndex3); // slice 2
    // end thread_one_call_function_three
    uint64_t ts4 = 168758682456000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts4, pid1, threadGroupId1, cat, splitStrIndex);
    // end thread_one_call_function_two
    uint64_t ts5 = 168758682466000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts5, pid1, threadGroupId1, cat2, splitStrIndex3);
    // end thread_two_call_function_three slice 5
    uint64_t ts6 = 168758679343000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts6, pid2, threadGroupId2, cat, splitStrIndex2);
    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 3);
    EXPECT_TRUE(slices->DursData()[0] == ts4 - ts1); // slice 0
    EXPECT_TRUE(slices->Depths()[0] == 0);
    EXPECT_TRUE(slices->DursData()[1] == ts6 - ts2); // slice 1
    EXPECT_TRUE(slices->Depths()[1] == 0);
    EXPECT_TRUE(slices->DursData()[2] == ts5 - ts3); // slice 2
}

/**
 * @tc.name: BeginSliceMultiTimes
 * @tc.desc: Test asynchronous call muti times
 * @tc.type: FUNC
 */
HWTEST_F(SliceFilterTest, BeginSliceMultiTimes, TestSize.Level1)
{
    TS_LOGI("test28-10");
    uint64_t ts1 = 168758663018000;
    uint32_t pid1 = 2532;
    uint32_t threadGroupId1 = 2519;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_one");
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts1, pid1, threadGroupId1, cat, splitStrIndex); // slice 0

    DataIndex splitStrIndex2 = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_one");
    uint64_t ts2 = 168758663028000;
    uint32_t pid2 = 2533;
    uint32_t threadGroupId2 = 2529;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts2, pid2, threadGroupId2, cat, splitStrIndex2); // slice 1

    DataIndex splitStrIndex3 = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_two");
    DataIndex cat2 = stream_.traceDataCache_->GetDataIndex("Catalog2");
    uint64_t ts3 = 168758679303000;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts3, pid1, threadGroupId1, cat2, splitStrIndex3); // slice 2

    DataIndex splitStrIndex4 = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_two");
    uint64_t ts4 = 168758679312000;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts4, pid2, threadGroupId2, cat2, splitStrIndex4); // slice 3

    DataIndex splitStrIndex5 = stream_.traceDataCache_->GetDataIndex("thread_one_call_function_three");
    uint64_t ts5 = 168758679313000;
    DataIndex cat3 = stream_.traceDataCache_->GetDataIndex("Catalog3");
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts5, pid1, threadGroupId1, cat3, splitStrIndex5); // slice 4

    DataIndex splitStrIndex6 = stream_.traceDataCache_->GetDataIndex("thread_two_call_function_three");
    uint64_t ts6 = 168758679323000;
    stream_.streamFilters_->sliceFilter_->StartAsyncSlice(ts6, pid2, threadGroupId2, cat3, splitStrIndex6); // slice 5

    // end thread_one_call_function_three
    uint64_t ts7 = 168758682456000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts7, pid1, threadGroupId1, cat,
                                                           splitStrIndex); // end slice 0

    // end thread_one_call_function_two
    uint64_t ts8 = 168758682466000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts8, pid2, threadGroupId2, cat,
                                                           splitStrIndex2); // end slice 1

    // end thread_one_call_function_one
    uint64_t ts9 = 168758682476000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts9, pid1, threadGroupId1, cat2,
                                                           splitStrIndex3); // end slice 2

    // end thread_two_call_function_three slice 5
    uint64_t ts10 = 168758679343000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts10, pid2, threadGroupId2, cat2,
                                                           splitStrIndex4); // end slice 3

    // end thread_two_call_function_two slice 3
    uint64_t ts11 = 168758679344000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts11, pid1, threadGroupId1, cat3,
                                                           splitStrIndex5); // end slice 4

    // end thread_two_call_function_one slice 1
    uint64_t ts12 = 168758689323000;
    stream_.streamFilters_->sliceFilter_->FinishAsyncSlice(ts12, pid2, threadGroupId2, cat3,
                                                           splitStrIndex6); // end slice 5

    auto slices = stream_.traceDataCache_->GetInternalSlicesData();
    EXPECT_TRUE(slices->Size() == 6);
    EXPECT_TRUE(slices->DursData()[0] == ts7 - ts1); // slice 0
    EXPECT_TRUE(slices->Depths()[0] == 0);

    EXPECT_TRUE(slices->DursData()[1] == ts8 - ts2); // slice 1
    EXPECT_TRUE(slices->Depths()[1] == 0);

    EXPECT_TRUE(slices->DursData()[2] == ts9 - ts3); // slice 2
    EXPECT_TRUE(slices->Depths()[2] == 0);

    EXPECT_TRUE(slices->DursData()[3] == ts10 - ts4); // slice 3
    EXPECT_TRUE(slices->Depths()[3] == 0);

    EXPECT_TRUE(slices->DursData()[4] == ts11 - ts5); // slice 4

    EXPECT_TRUE(slices->DursData()[5] == ts12 - ts6); // slice 5
}
} // namespace TraceStreamer
} // namespace SysTuning
