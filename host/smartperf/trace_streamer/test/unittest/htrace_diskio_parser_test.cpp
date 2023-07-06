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

#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>

#include "diskio_plugin_result.pb.h"
#include "diskio_plugin_result.pbreader.h"
#include "htrace_disk_io_parser.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HtracediskioParserTest : public ::testing::Test {
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
 * @tc.name: ParseHtracediskioWithoutDiskioData
 * @tc.desc: Parse a diskio that does not contain any DiskioData
 * @tc.type: FUNC
 */
HWTEST_F(HtracediskioParserTest, ParseHtracediskioWithoutDiskioData, TestSize.Level1)
{
    TS_LOGI("test13-1");
    uint64_t ts = 100;
    auto diskioInfo = std::make_unique<DiskioData>();
    std::string diskioData = "";
    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    HtraceDiskIOParser htraceDiskioParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceDiskioParser.Parse(diskioInfoData, ts);
    auto size = stream_.traceDataCache_->GetConstDiskIOData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtracediskioWithOneDiskioData
 * @tc.desc: Parse a diskio with one DiskioData
 * @tc.type: FUNC
 */
HWTEST_F(HtracediskioParserTest, ParseHtracediskioWithOneDiskioData, TestSize.Level1)
{
    TS_LOGI("test13-2");
    uint64_t ts = 100;
    const uint64_t RD = 100;
    const uint64_t WR = 101;
    const uint64_t RDPERSEC = 102;
    const uint64_t WRPERSEC = 103;
    StatsData* statsData = new StatsData();
    auto ioStatData = statsData->add_statsinfo();
    ioStatData->set_rd_kb(RD);
    ioStatData->set_wr_kb(WR);
    ioStatData->set_rd_per_sec(RDPERSEC);
    ioStatData->set_wr_per_sec(WRPERSEC);

    auto diskioInfo(std::make_unique<DiskioData>());
    diskioInfo->set_allocated_statsdata(statsData);

    std::string diskioData = "";
    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    HtraceDiskIOParser htraceDiskioParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceDiskioParser.Parse(diskioInfoData, ts);
    htraceDiskioParser.Finish();
    auto size = stream_.traceDataCache_->GetConstDiskIOData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtracediskioWithTwoDiskioData
 * @tc.desc: Parse a diskio with two DiskioData
 * @tc.type: FUNC
 */
HWTEST_F(HtracediskioParserTest, ParseHtracediskioWithTwoDiskioData, TestSize.Level1)
{
    TS_LOGI("test13-3");
    uint64_t ts = 100;
    const uint64_t RD_01 = 100;
    const uint64_t WR_01 = 101;
    const uint64_t RDPERSEC_01 = 102;
    const uint64_t WRPERSEC_01 = 103;

    const uint64_t RD_02 = 104;
    const uint64_t WR_02 = 105;
    const uint64_t RDPERSEC_02 = 106;
    const uint64_t WRPERSEC_02 = 107;

    StatsData* statsDataFirst = new StatsData();
    auto ioStatDatafirst = statsDataFirst->add_statsinfo();
    ioStatDatafirst->set_rd_kb(RD_01);
    ioStatDatafirst->set_wr_kb(WR_01);
    ioStatDatafirst->set_rd_per_sec(RDPERSEC_01);
    ioStatDatafirst->set_wr_per_sec(WRPERSEC_01);

    auto diskioInfo = std::make_unique<DiskioData>();
    diskioInfo->set_allocated_statsdata(statsDataFirst);

    std::string diskioData = "";
    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData01(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());

    HtraceDiskIOParser htraceDiskioParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceDiskioParser.Parse(diskioInfoData01, ts);

    StatsData* statsDataSecond = new StatsData();
    auto ioStatDataSecond = statsDataSecond->add_statsinfo();
    ioStatDataSecond->set_rd_kb(RD_02);
    ioStatDataSecond->set_wr_kb(WR_02);
    ioStatDataSecond->set_rd_per_sec(RDPERSEC_02);
    ioStatDataSecond->set_wr_per_sec(WRPERSEC_02);
    diskioInfo->set_allocated_statsdata(statsDataSecond);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData02(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData02, ts);
    htraceDiskioParser.Finish();
    auto size = stream_.traceDataCache_->GetConstDiskIOData().Size();
    EXPECT_EQ(1, size);

    auto rdCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[0];
    EXPECT_EQ(rdCountPerSecFirst, RDPERSEC_02);

    auto wrCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[0];
    EXPECT_EQ(wrCountPerSecFirst, WRPERSEC_02);

    auto rdCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[0];
    EXPECT_EQ(rdCountDatasFirst, RD_02);

    auto wrCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[0];
    EXPECT_EQ(wrCountDatasFirst, WR_02);
}

/**
 * @tc.name: ParseHtracediskioWithThreeDiskioData
 * @tc.desc: Parse a diskio with Three DiskioData
 * @tc.type: FUNC
 */
HWTEST_F(HtracediskioParserTest, ParseHtracediskioWithThreeDiskioData, TestSize.Level1)
{
    TS_LOGI("test13-4");
    uint64_t ts = 100;
    const uint64_t RD_01 = 100;
    const uint64_t WR_01 = 101;
    const uint64_t RDPERSEC_01 = 102;
    const uint64_t WRPERSEC_01 = 103;

    const uint64_t RD_02 = 104;
    const uint64_t WR_02 = 105;
    const uint64_t RDPERSEC_02 = 106;
    const uint64_t WRPERSEC_02 = 107;

    const uint64_t RD_03 = 108;
    const uint64_t WR_03 = 109;
    const uint64_t RDPERSEC_03 = 110;
    const uint64_t WRPERSEC_03 = 111;

    StatsData* statsDataFirst = new StatsData();
    auto ioStatDatafirst = statsDataFirst->add_statsinfo();
    ioStatDatafirst->set_rd_kb(RD_01);
    ioStatDatafirst->set_wr_kb(WR_01);
    ioStatDatafirst->set_rd_per_sec(RDPERSEC_01);
    ioStatDatafirst->set_wr_per_sec(WRPERSEC_01);

    auto diskioInfo = std::make_unique<DiskioData>();
    diskioInfo->set_allocated_statsdata(statsDataFirst);

    std::string diskioData = "";
    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData01(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());

    HtraceDiskIOParser htraceDiskioParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceDiskioParser.Parse(diskioInfoData01, ts);

    StatsData* statsDataSecond = new StatsData();
    auto ioStatDataSecond = statsDataSecond->add_statsinfo();
    ioStatDataSecond->set_rd_kb(RD_02);
    ioStatDataSecond->set_wr_kb(WR_02);
    ioStatDataSecond->set_rd_per_sec(RDPERSEC_02);
    ioStatDataSecond->set_wr_per_sec(WRPERSEC_02);
    diskioInfo->set_allocated_statsdata(statsDataSecond);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData02(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData02, ts);

    StatsData* statsDataThird = new StatsData();
    auto ioStatDataThird = statsDataThird->add_statsinfo();
    ioStatDataThird->set_rd_kb(RD_03);
    ioStatDataThird->set_wr_kb(WR_03);
    ioStatDataThird->set_rd_per_sec(RDPERSEC_03);
    ioStatDataThird->set_wr_per_sec(WRPERSEC_03);
    diskioInfo->set_allocated_statsdata(statsDataThird);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData03(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData03, ts);
    htraceDiskioParser.Finish();
    auto size = stream_.traceDataCache_->GetConstDiskIOData().Size();
    EXPECT_EQ(2, size);

    auto rdCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[0];
    auto rdCountPerSecSecond = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[1];
    EXPECT_EQ(rdCountPerSecFirst, RDPERSEC_02);
    EXPECT_EQ(rdCountPerSecSecond, RDPERSEC_03);

    auto wrCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[0];
    auto wrCountPerSecSecond = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[1];
    EXPECT_EQ(wrCountPerSecFirst, WRPERSEC_02);
    EXPECT_EQ(wrCountPerSecSecond, WRPERSEC_03);

    auto rdCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[0];
    auto rdCountDatasSecond = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[1];
    EXPECT_EQ(rdCountDatasFirst, RD_02);
    EXPECT_EQ(rdCountDatasSecond, RD_03);

    auto wrCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[0];
    auto wrCountDatasSecond = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[1];
    EXPECT_EQ(wrCountDatasFirst, WR_02);
    EXPECT_EQ(wrCountDatasSecond, WR_03);
}

/**
 * @tc.name: ParseHtracediskioWithMultipleDiskioData
 * @tc.desc: Parse a diskio with Multiple DiskioData
 * @tc.type: FUNC
 */
HWTEST_F(HtracediskioParserTest, ParseHtracediskioWithMultipleDiskioData, TestSize.Level1)
{
    TS_LOGI("test13-5");
    uint64_t ts = 100;
    const uint64_t RD_01 = 100;
    const uint64_t WR_01 = 101;
    const uint64_t RDPERSEC_01 = 102;
    const uint64_t WRPERSEC_01 = 103;

    const uint64_t RD_02 = 104;
    const uint64_t WR_02 = 105;
    const uint64_t RDPERSEC_02 = 106;
    const uint64_t WRPERSEC_02 = 107;

    const uint64_t RD_03 = 108;
    const uint64_t WR_03 = 109;
    const uint64_t RDPERSEC_03 = 110;
    const uint64_t WRPERSEC_03 = 111;

    const uint64_t RD_04 = 112;
    const uint64_t WR_04 = 113;
    const uint64_t RDPERSEC_04 = 114;
    const uint64_t WRPERSEC_04 = 115;

    StatsData* statsDataFirst = new StatsData();
    auto ioStatDatafirst = statsDataFirst->add_statsinfo();
    ioStatDatafirst->set_rd_kb(RD_01);
    ioStatDatafirst->set_wr_kb(WR_01);
    ioStatDatafirst->set_rd_per_sec(RDPERSEC_01);
    ioStatDatafirst->set_wr_per_sec(WRPERSEC_01);

    auto diskioInfo = std::make_unique<DiskioData>();
    diskioInfo->set_allocated_statsdata(statsDataFirst);

    std::string diskioData = "";
    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData01(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());

    HtraceDiskIOParser htraceDiskioParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceDiskioParser.Parse(diskioInfoData01, ts);

    StatsData* statsDataSecond = new StatsData();
    auto ioStatDataSecond = statsDataSecond->add_statsinfo();
    ioStatDataSecond->set_rd_kb(RD_02);
    ioStatDataSecond->set_wr_kb(WR_02);
    ioStatDataSecond->set_rd_per_sec(RDPERSEC_02);
    ioStatDataSecond->set_wr_per_sec(WRPERSEC_02);
    diskioInfo->set_allocated_statsdata(statsDataSecond);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData02(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData02, ts);

    StatsData* statsDataThird = new StatsData();
    auto ioStatDataThird = statsDataThird->add_statsinfo();
    ioStatDataThird->set_rd_kb(RD_03);
    ioStatDataThird->set_wr_kb(WR_03);
    ioStatDataThird->set_rd_per_sec(RDPERSEC_03);
    ioStatDataThird->set_wr_per_sec(WRPERSEC_03);
    diskioInfo->set_allocated_statsdata(statsDataThird);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData03(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData03, ts);

    StatsData* statsDataForth = new StatsData();
    auto ioStatDataForth = statsDataForth->add_statsinfo();
    ioStatDataForth->set_rd_kb(RD_04);
    ioStatDataForth->set_wr_kb(WR_04);
    ioStatDataForth->set_rd_per_sec(RDPERSEC_04);
    ioStatDataForth->set_wr_per_sec(WRPERSEC_04);
    diskioInfo->set_allocated_statsdata(statsDataForth);

    diskioInfo->SerializeToString(&diskioData);
    ProtoReader::BytesView diskioInfoData04(reinterpret_cast<const uint8_t*>(diskioData.data()), diskioData.size());
    htraceDiskioParser.Parse(diskioInfoData04, ts);
    htraceDiskioParser.Finish();

    auto size = stream_.traceDataCache_->GetConstDiskIOData().Size();
    EXPECT_EQ(3, size);

    auto rdCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[0];
    auto rdCountPerSecSecond = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[1];
    auto rdCountPerSecThird = stream_.traceDataCache_->GetConstDiskIOData().RdCountPerSecDatas()[2];
    EXPECT_EQ(rdCountPerSecFirst, RDPERSEC_02);
    EXPECT_EQ(rdCountPerSecSecond, RDPERSEC_03);
    EXPECT_EQ(rdCountPerSecThird, RDPERSEC_04);

    auto wrCountPerSecFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[0];
    auto wrCountPerSecSecond = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[1];
    auto wrCountPerSecThird = stream_.traceDataCache_->GetConstDiskIOData().WrCountPerSecDatas()[2];
    EXPECT_EQ(wrCountPerSecFirst, WRPERSEC_02);
    EXPECT_EQ(wrCountPerSecSecond, WRPERSEC_03);
    EXPECT_EQ(wrCountPerSecThird, WRPERSEC_04);

    auto rdCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[0];
    auto rdCountDatasSecond = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[1];
    auto rdCountDatasThird = stream_.traceDataCache_->GetConstDiskIOData().RdCountDatas()[2];
    EXPECT_EQ(rdCountDatasFirst, RD_02);
    EXPECT_EQ(rdCountDatasSecond, RD_03);
    EXPECT_EQ(rdCountDatasThird, RD_04);

    auto wrCountDatasFirst = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[0];
    auto wrCountDatasSecond = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[1];
    auto wrCountDatasThird = stream_.traceDataCache_->GetConstDiskIOData().WrCountDatas()[2];
    EXPECT_EQ(wrCountDatasFirst, WR_02);
    EXPECT_EQ(wrCountDatasSecond, WR_03);
    EXPECT_EQ(wrCountDatasThird, WR_04);
}
} // namespace TraceStreamer
} // namespace SysTuning