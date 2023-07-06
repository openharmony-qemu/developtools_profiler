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

#include "clock_filter_ex.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class ClockFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.clockFilter_ = std::make_unique<ClockFilterEx>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerFilters streamFilters_;
    SysTuning::TraceStreamer::TraceDataCache traceDataCache_;
};

/**
 * @tc.name: ConvertBoottimeToMonitonicTime
 * @tc.desc: convert Boottime to MonitonicTime
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertBoottimeToMonitonicTime, TestSize.Level1)
{
    TS_LOGI("test3-1");
    uint64_t tsBoottime = 100;
    uint64_t tsMonotonicTime = 200;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    uint64_t time1 = 150;
    uint64_t expectTime1 = 250;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time1, TS_MONOTONIC), expectTime1);
}

/**
 * @tc.name: ConvertBoottimeToMonitonicTimeTwice
 * @tc.desc: convert twice Boottime to MonitonicTime
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertBoottimeToMonitonicTimeTwice, TestSize.Level1)
{
    TS_LOGI("test3-2");
    uint64_t tsBoottime = 100;
    uint64_t tsMonotonicTime = 200;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    uint64_t time1 = 150;
    uint64_t expectTime1 = 250;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time1, TS_MONOTONIC), expectTime1);
    uint64_t time2 = 200;
    uint64_t expectTime2 = 300;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time2, TS_MONOTONIC), expectTime2);
}

/**
 * @tc.name: ConvertTimestampInvalid
 * @tc.desc: Convert timeStamp invalid
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertTimestampInvalid, TestSize.Level1)
{
    TS_LOGI("test3-3");
    uint64_t tsBoottime = 100;
    uint64_t tsMonotonicTime = 200;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsMonotonicTime2 = 350;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_MONOTONIC, tsMonotonicTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);
    uint64_t time2 = 200;
    uint64_t expectTime2 = 200;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time2, TS_CLOCK_REALTIME), expectTime2);
}

/**
 * @tc.name: ConvertTimestampBoottimeToRealtime
 * @tc.desc: convert Boottime to Realtime
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertTimestampBoottimeToRealtime, TestSize.Level1)
{
    TS_LOGI("test3-4");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsRealTime2 = 400;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);
    uint64_t time3 = 101;
    uint64_t expectTime3 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
}

/**
 * @tc.name: ConvertBoottimeToRealtimeTwiceWithTwoSnapShot
 * @tc.desc: convert Boottime to Realtime twice with two snapShot
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertBoottimeToRealtimeTwiceWithTwoSnapShot, TestSize.Level1)
{
    TS_LOGI("test3-5");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsRealTime2 = 400;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);
    uint64_t time3 = 101;
    uint64_t expectTime3 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
    time3 = 201;
    expectTime3 = 401;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
}

/**
 * @tc.name: ConvertBoottimeToRealtimeWithSingleSnapShot
 * @tc.desc: convert Boottime to Realtime twice with single snapShot
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertBoottimeToRealtimeWithSingleSnapShot, TestSize.Level1)
{
    TS_LOGI("test3-6");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    uint64_t time3 = 101;
    uint64_t expectTime3 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
    time3 = 201;
    expectTime3 = 401;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
}

/**
 * @tc.name: ConvertRealtimeToBoottime
 * @tc.desc: convert Realtime to Boottime with single snapShot
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertRealtimeToBoottime, TestSize.Level1)
{
    TS_LOGI("test3-7");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);
    uint64_t time7 = 301;
    uint64_t expectTime7 = 101;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_CLOCK_BOOTTIME), expectTime7);
}

/**
 * @tc.name: ConvertRealtimeToBoottimeTwice
 * @tc.desc: convert Realtime to Boottime twice with single snapShot
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertRealtimeToBoottimeTwice, TestSize.Level1)
{
    TS_LOGI("test3-8");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);
    uint64_t time7 = 301;
    uint64_t expectTime7 = 101;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_CLOCK_BOOTTIME), expectTime7);
    time7 = 501;
    expectTime7 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_CLOCK_BOOTTIME), expectTime7);
}

/**
 * @tc.name: ConvertRealtimeToBoottimeTwiceWithTwoSnapshot
 * @tc.desc: convert Realtime to Boottime twice with two snapShot
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertRealtimeToBoottimeTwiceWithTwoSnapshot, TestSize.Level1)
{
    TS_LOGI("test3-9");
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsRealTime2 = 400;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);
    uint64_t time7 = 401;
    uint64_t expectTime7 = 201;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_CLOCK_BOOTTIME), expectTime7);
    time7 = 301;
    expectTime7 = 101;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_CLOCK_BOOTTIME), expectTime7);
}

/**
 * @tc.name: ConvertTimestamp
 * @tc.desc: muti type timeStamp convert
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertTimestamp, TestSize.Level1)
{
    TS_LOGI("test3-10");
    uint64_t tsBoottime = 100;
    uint64_t tsMonotonicTime = 200;
    uint64_t tsRealTime = 300;
    uint64_t tsRealTimeCoarseTime = 400;
    std::vector<SnapShot> snapShot0;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    snapShot0.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsMonotonicTime2 = 350;
    uint64_t tsRealTime2 = 400;
    uint64_t tsRealTimeCoarseTime2 = 800;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_MONOTONIC, tsMonotonicTime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    snapShot1.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);
    uint64_t time1 = 150;
    uint64_t expectTime1 = 250;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time1, TS_MONOTONIC), expectTime1);
    uint64_t time2 = 200;
    uint64_t expectTime2 = 350;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time2, TS_MONOTONIC), expectTime2);
    uint64_t time3 = 101;
    uint64_t expectTime3 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time3, TS_CLOCK_REALTIME), expectTime3);
    uint64_t time4 = 102;
    uint64_t expectTime4 = 402;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_BOOTTIME, time4, TS_CLOCK_REALTIME_COARSE), expectTime4);
    uint64_t time5 = 102;
    uint64_t expectTime5 = 202;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_MONOTONIC, time5, TS_CLOCK_REALTIME), expectTime5);
    uint64_t time6 = 351;
    uint64_t expectTime6 = 251;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time6, TS_MONOTONIC), expectTime6);
    uint64_t time7 = 401;
    uint64_t expectTime7 = 351;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_CLOCK_REALTIME, time7, TS_MONOTONIC), expectTime7);
    uint64_t time8 = 150;
    uint64_t expectTime8 = 50;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_MONOTONIC, time8, TS_CLOCK_BOOTTIME), expectTime8);
    uint64_t time9 = 250;
    uint64_t expectTime9 = 150;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_MONOTONIC, time9, TS_CLOCK_BOOTTIME), expectTime9);
    uint64_t time10 = 351;
    uint64_t expectTime10 = 201;
    EXPECT_EQ(streamFilters_.clockFilter_->Convert(TS_MONOTONIC, time10, TS_CLOCK_BOOTTIME), expectTime10);
}

/**
 * @tc.name: ConvertToPrimary
 * @tc.desc: set realtime as primary time type, and convert boottime to Primary time type
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertToPrimary, TestSize.Level1)
{
    TS_LOGI("test3-11");
    std::vector<SnapShot> snapShot0;
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsRealTime2 = 400;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_REALTIME);
    uint64_t time1 = 150;
    uint64_t expectTime1 = 350;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_BOOTTIME, time1), expectTime1);
}

/**
 * @tc.name: ConvertToPrimaryTwice
 * @tc.desc: set realtime as primary time type, and convert boottime to Primary time type twice
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertToPrimaryTwice, TestSize.Level1)
{
    TS_LOGI("test3-12");
    std::vector<SnapShot> snapShot0;
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsRealTime2 = 400;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_BOOTTIME);
    uint64_t time1 = 350;
    uint64_t expectTime1 = 150;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, time1), expectTime1);
}

/**
 * @tc.name: ConvertToPrimaryTimestampLessThanSnapShop
 * @tc.desc: convert realtime to primary time type, and timeStamp less than snapshop
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertToPrimaryTimestampLessThanSnapShop, TestSize.Level1)
{
    TS_LOGI("test3-13");
    std::vector<SnapShot> snapShot0;
    uint64_t tsBoottime = 100;
    uint64_t tsRealTime = 300;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_BOOTTIME);
    uint64_t time1 = 250;
    uint64_t expectTime1 = 50;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, time1), expectTime1);
}

/**
 * @tc.name: ConvertMonotonicTimeToPrimaryTwice
 * @tc.desc: convert TS_MONOTONIC to primary time type with single snapshop
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertMonotonicTimeToPrimaryTwice, TestSize.Level1)
{
    TS_LOGI("test3-14");
    std::vector<SnapShot> snapShot0;
    uint64_t tsMonotonicTime = 200;
    uint64_t tsRealTimeCoarseTime = 400;
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    snapShot0.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_REALTIME_COARSE);
    uint64_t time1 = 250;
    uint64_t expectTime1 = 450;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_MONOTONIC, time1), expectTime1);
    time1 = 550;
    expectTime1 = 750;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_MONOTONIC, time1), expectTime1);
}

/**
 * @tc.name: ConvertToPrimaryTwiceWithTwoSnapshop
 * @tc.desc: convert TS_MONOTONIC & TS_CLOCK_BOOTTIME to primary time type with two snapshop
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, ConvertToPrimaryTwiceWithTwoSnapshop, TestSize.Level1)
{
    TS_LOGI("test3-15");
    std::vector<SnapShot> snapShot0;
    uint64_t tsMonotonicTime = 200;
    uint64_t tsRealTimeCoarseTime = 400;
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    snapShot0.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBootTime2 = 350;
    uint64_t tsRealTimeCoarseTime2 = 800;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBootTime2});
    snapShot1.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_REALTIME_COARSE);

    uint64_t time1 = 250;
    uint64_t expectTime1 = 450;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_MONOTONIC, time1), expectTime1);

    uint64_t time2 = 450;
    uint64_t expectTime2 = 900;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_BOOTTIME, time2), expectTime2);
}

/**
 * @tc.name: MutiTimeTypeConvertWithMutiSnapshop
 * @tc.desc: convert muti time type to primary time type with muti snapshop
 * @tc.type: FUNC
 */
HWTEST_F(ClockFilterTest, MutiTimeTypeConvertWithMutiSnapshop, TestSize.Level1)
{
    TS_LOGI("test3-16");
    std::vector<SnapShot> snapShot0;
    uint64_t tsBoottime = 100;
    uint64_t tsMonotonicTime = 200;
    uint64_t tsRealTime = 300;
    uint64_t tsRealTimeCoarseTime = 400;
    snapShot0.push_back({TS_CLOCK_BOOTTIME, tsBoottime});
    snapShot0.push_back({TS_MONOTONIC, tsMonotonicTime});
    snapShot0.push_back({TS_CLOCK_REALTIME, tsRealTime});
    snapShot0.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot0);

    std::vector<SnapShot> snapShot1;
    uint64_t tsBoottime2 = 200;
    uint64_t tsMonotonicTime2 = 350;
    uint64_t tsRealTime2 = 400;
    uint64_t tsRealTimeCoarseTime2 = 800;
    snapShot1.push_back({TS_CLOCK_BOOTTIME, tsBoottime2});
    snapShot1.push_back({TS_MONOTONIC, tsMonotonicTime2});
    snapShot1.push_back({TS_CLOCK_REALTIME, tsRealTime2});
    snapShot1.push_back({TS_CLOCK_REALTIME_COARSE, tsRealTimeCoarseTime2});
    streamFilters_.clockFilter_->AddClockSnapshot(snapShot1);

    streamFilters_.clockFilter_->SetPrimaryClock(TS_CLOCK_REALTIME);
    uint64_t time1 = 150;
    uint64_t expectTime1 = 350;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_BOOTTIME, time1), expectTime1);
    uint64_t time2 = 101;
    uint64_t expectTime2 = 301;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_BOOTTIME, time2), expectTime2);
    uint64_t time3 = 101;
    uint64_t expectTime3 = 101;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, time3), expectTime3);
    uint64_t time4 = 351;
    uint64_t expectTime4 = 351;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, time4), expectTime4);
    uint64_t time5 = 350;
    uint64_t expectTime5 = 250;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME_COARSE, time5), expectTime5);
    uint64_t time6 = 420;
    uint64_t expectTime6 = 320;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME_COARSE, time6), expectTime6);
    uint64_t time7 = 801;
    uint64_t expectTime7 = 401;
    EXPECT_EQ(streamFilters_.clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME_COARSE, time7), expectTime7);
}
} // namespace TraceStreamer
} // namespace SysTuning
