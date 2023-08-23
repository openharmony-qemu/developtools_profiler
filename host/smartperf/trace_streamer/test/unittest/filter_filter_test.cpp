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
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class FilterFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.filterFilter_ = std::make_unique<FilterFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: AddCpuCounterFilter
 * @tc.desc: Add cpu_counter_filter through AddFilter interface
 * @tc.type: FUNC
 */
HWTEST_F(FilterFilterTest, AddCpuCounterFilter, TestSize.Level1)
{
    TS_LOGI("test6-1");
    uint32_t filterId = streamFilters_.filterFilter_->AddFilter("cpu_counter_filter", "cpu1", 1);
    EXPECT_EQ(filterId, static_cast<uint32_t>(0));

    filterId = streamFilters_.filterFilter_->AddFilter("cpu_counter_filter", "cpu2", 2);
    EXPECT_EQ(filterId, static_cast<uint32_t>(1));

    Filter* filterTable = traceDataCache_.GetFilterData();
    EXPECT_EQ(filterTable->Size(), static_cast<size_t>(2));
}

/**
 * @tc.name: AddThreadFilter
 * @tc.desc: Add thread_counter_filter & thread_filter through AddFilter interface
 * @tc.type: FUNC
 */
HWTEST_F(FilterFilterTest, AddThreadFilter, TestSize.Level1)
{
    TS_LOGI("test6-2");
    uint32_t threadFilterId = streamFilters_.filterFilter_->AddFilter("thread_counter_filter", "threadCount1", 1);
    EXPECT_EQ(threadFilterId, static_cast<uint32_t>(0));

    threadFilterId = streamFilters_.filterFilter_->AddFilter("thread_counter_filter", "threadCount2", 2);
    EXPECT_EQ(threadFilterId, static_cast<uint32_t>(1));

    Filter* filterTable = traceDataCache_.GetFilterData();
    EXPECT_EQ(filterTable->Size(), static_cast<size_t>(2));

    threadFilterId = streamFilters_.filterFilter_->AddFilter("thread_filter", "thread1", 1);
    EXPECT_EQ(threadFilterId, static_cast<uint32_t>(2));

    threadFilterId = streamFilters_.filterFilter_->AddFilter("thread_filter", "thread2", 2);
    EXPECT_EQ(threadFilterId, static_cast<uint32_t>(3));

    EXPECT_EQ(filterTable->Size(), static_cast<size_t>(4));
}
} // namespace TraceStreamer
} // namespace SysTuning
