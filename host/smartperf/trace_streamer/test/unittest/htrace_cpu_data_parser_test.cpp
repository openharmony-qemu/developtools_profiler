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

#include "cpu_plugin_result.pb.h"
#include "cpu_plugin_result.pbreader.h"
#include "htrace_cpu_data_parser.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HtraceCpuDataParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() const {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: ParseHtraceWithoutCpuDataData
 * @tc.desc: Parse a cpu that does not contain any cpudata
 * @tc.type: FUNC
 */
HWTEST_F(HtraceCpuDataParserTest, ParseHtraceWithoutCpuData, TestSize.Level1)
{
    TS_LOGI("test11-1");
    uint64_t ts = 100;
    auto cpuInfo = std::make_unique<CpuData>();
    std::string cpuData = "";
    cpuInfo->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    HtraceCpuDataParser htraceCpuDataParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceCpuDataParser.Parse(cpuInfoData, ts);
    auto size = stream_.traceDataCache_->GetConstCpuUsageInfoData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceWithOneCpuData
 * @tc.desc: Parse a cpu with one cpudata
 * @tc.type: FUNC
 */
HWTEST_F(HtraceCpuDataParserTest, ParseHtraceWithOneCpuData, TestSize.Level1)
{
    TS_LOGI("test11-2");
    uint64_t ts = 102;
    const uint64_t TOTAL_LOAD = 2;
    const uint64_t USER_LOAD = 42;
    const uint64_t SYSTEM_LOAD = 32;
    const uint64_t PROCESS_NUM = 202;

    auto cpuInfo(std::make_unique<CpuData>());
    cpuInfo->set_total_load(TOTAL_LOAD);
    cpuInfo->set_user_load(USER_LOAD);
    cpuInfo->set_sys_load(SYSTEM_LOAD);
    cpuInfo->set_process_num(PROCESS_NUM);

    std::string cpuData = "";
    cpuInfo->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    HtraceCpuDataParser htraceCpuDataParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceCpuDataParser.Parse(cpuInfoData, ts);
    htraceCpuDataParser.Finish();
    auto size = stream_.traceDataCache_->GetConstCpuUsageInfoData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceWithTwoCpuData
 * @tc.desc: Parse a cpu with two cpudata
 * @tc.type: FUNC
 */
HWTEST_F(HtraceCpuDataParserTest, ParseHtraceWithTwoCpuData, TestSize.Level1)
{
    TS_LOGI("test11-3");
    uint64_t ts = 103;
    const uint64_t TOTALLOAD_01 = 2;
    const uint64_t USERLOAD_01 = 42;
    const uint64_t SYSTEMLOAD_01 = 32;
    const uint64_t PROCESS_NUM_01 = 202;

    const uint64_t TOTALLOAD_02 = 3;
    const uint64_t USERLOAD_02 = 43;
    const uint64_t SYSTEMLOAD_02 = 33;
    const uint64_t PROCESS_NUM_02 = 203;

    CpuUsageInfo* cpuUsageInfo01 = new CpuUsageInfo();
    auto cpuDataInfo01(std::make_unique<CpuData>());
    cpuDataInfo01->set_allocated_cpu_usage_info(cpuUsageInfo01);
    cpuDataInfo01->set_total_load(TOTALLOAD_01);
    cpuDataInfo01->set_user_load(USERLOAD_01);
    cpuDataInfo01->set_sys_load(SYSTEMLOAD_01);
    cpuDataInfo01->set_process_num(PROCESS_NUM_01);

    std::string cpuData = "";
    cpuDataInfo01->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData01(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    HtraceCpuDataParser htraceCpuDataParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceCpuDataParser.Parse(cpuInfoData01, ts);

    CpuUsageInfo* cpuUsageInfo02 = new CpuUsageInfo();
    auto cpuDataInfo02(std::make_unique<CpuData>());
    cpuDataInfo02->set_allocated_cpu_usage_info(cpuUsageInfo02);
    cpuDataInfo02->set_total_load(TOTALLOAD_02);
    cpuDataInfo02->set_user_load(USERLOAD_02);
    cpuDataInfo02->set_sys_load(SYSTEMLOAD_02);
    cpuDataInfo02->set_process_num(PROCESS_NUM_02);

    cpuDataInfo02->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData02(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData02, ts);
    htraceCpuDataParser.Finish();

    auto size = stream_.traceDataCache_->GetConstCpuUsageInfoData().Size();
    EXPECT_EQ(1, size);

    auto totalLoad = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[0];
    EXPECT_EQ(totalLoad, TOTALLOAD_02);

    auto userLoad = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[0];
    EXPECT_EQ(userLoad, USERLOAD_02);

    auto systemLoad = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[0];
    EXPECT_EQ(systemLoad, SYSTEMLOAD_02);
}

/**
 * @tc.name: ParseHtraceWithThreeCpuData
 * @tc.desc: Parse a cpu with Three cpudata
 * @tc.type: FUNC
 */
HWTEST_F(HtraceCpuDataParserTest, ParseHtraceWithThreeCpuData, TestSize.Level1)
{
    TS_LOGI("test11-4");
    uint64_t ts = 104;
    const uint64_t TOTALLOAD_01 = 4;
    const uint64_t USERLOAD_01 = 44;
    const uint64_t SYSTEMLOAD_01 = 34;
    const uint64_t PROCESS_NUM_01 = 204;

    const uint64_t TOTALLOAD_02 = 5;
    const uint64_t USERLOAD_02 = 45;
    const uint64_t SYSTEMLOAD_02 = 35;
    const uint64_t PROCESS_NUM_02 = 205;

    const uint64_t TOTALLOAD_03 = 6;
    const uint64_t USERLOAD_03 = 46;
    const uint64_t SYSTEMLOAD_03 = 36;
    const uint64_t PROCESS_NUM_03 = 206;

    CpuUsageInfo* cpuUsageInfo01 = new CpuUsageInfo();
    auto cpuDataInfo01(std::make_unique<CpuData>());
    cpuDataInfo01->set_allocated_cpu_usage_info(cpuUsageInfo01);
    cpuDataInfo01->set_total_load(TOTALLOAD_01);
    cpuDataInfo01->set_user_load(USERLOAD_01);
    cpuDataInfo01->set_sys_load(SYSTEMLOAD_01);
    cpuDataInfo01->set_process_num(PROCESS_NUM_01);

    std::string cpuData = "";
    cpuDataInfo01->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData01(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    HtraceCpuDataParser htraceCpuDataParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceCpuDataParser.Parse(cpuInfoData01, ts);

    CpuUsageInfo* cpuUsageInfo02 = new CpuUsageInfo();
    auto cpuDataInfo02(std::make_unique<CpuData>());
    cpuDataInfo02->set_allocated_cpu_usage_info(cpuUsageInfo02);
    cpuDataInfo02->set_total_load(TOTALLOAD_02);
    cpuDataInfo02->set_user_load(USERLOAD_02);
    cpuDataInfo02->set_sys_load(SYSTEMLOAD_02);
    cpuDataInfo02->set_process_num(PROCESS_NUM_02);

    cpuDataInfo02->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData02(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData02, ts);

    CpuUsageInfo* cpuUsageInfo03 = new CpuUsageInfo();
    auto cpuDataInfo03(std::make_unique<CpuData>());
    cpuDataInfo03->set_allocated_cpu_usage_info(cpuUsageInfo03);
    cpuDataInfo03->set_total_load(TOTALLOAD_03);
    cpuDataInfo03->set_user_load(USERLOAD_03);
    cpuDataInfo03->set_sys_load(SYSTEMLOAD_03);
    cpuDataInfo03->set_process_num(PROCESS_NUM_03);

    cpuDataInfo03->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData03(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData03, ts);
    htraceCpuDataParser.Finish();

    auto size = stream_.traceDataCache_->GetConstCpuUsageInfoData().Size();
    EXPECT_EQ(2, size);

    auto totalLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[0];
    auto totalLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[1];
    EXPECT_EQ(totalLoadFirst, TOTALLOAD_02);
    EXPECT_EQ(totalLoadSecond, TOTALLOAD_03);

    auto userLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[0];
    auto userLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[1];
    EXPECT_EQ(userLoadFirst, USERLOAD_02);
    EXPECT_EQ(userLoadSecond, USERLOAD_03);

    auto systemLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[0];
    auto systemLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[1];
    EXPECT_EQ(systemLoadFirst, SYSTEMLOAD_02);
    EXPECT_EQ(systemLoadSecond, SYSTEMLOAD_03);
}

/**
 * @tc.name: ParseHtraceWithMultipleCpuData
 * @tc.desc: Parse a CpuData with Multiple CpuData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceCpuDataParserTest, ParseHtraceWithMultipleCpuData, TestSize.Level1)
{
    TS_LOGI("test11-5");
    uint64_t ts = 104;
    const uint64_t TOTALLOAD_01 = 4;
    const uint64_t USERLOAD_01 = 44;
    const uint64_t SYSTEMLOAD_01 = 34;
    const uint64_t PROCESS_NUM_01 = 204;

    const uint64_t TOTALLOAD_02 = 5;
    const uint64_t USERLOAD_02 = 45;
    const uint64_t SYSTEMLOAD_02 = 35;
    const uint64_t PROCESS_NUM_02 = 205;

    const uint64_t TOTALLOAD_03 = 6;
    const uint64_t USERLOAD_03 = 46;
    const uint64_t SYSTEMLOAD_03 = 36;
    const uint64_t PROCESS_NUM_03 = 206;

    const uint64_t TOTALLOAD_04 = 6;
    const uint64_t USERLOAD_04 = 46;
    const uint64_t SYSTEMLOAD_04 = 36;
    const uint64_t PROCESS_NUM_04 = 206;

    CpuUsageInfo* cpuUsageInfo01 = new CpuUsageInfo();
    auto cpuDataInfo01(std::make_unique<CpuData>());
    cpuDataInfo01->set_allocated_cpu_usage_info(cpuUsageInfo01);
    cpuDataInfo01->set_total_load(TOTALLOAD_01);
    cpuDataInfo01->set_user_load(USERLOAD_01);
    cpuDataInfo01->set_sys_load(SYSTEMLOAD_01);
    cpuDataInfo01->set_process_num(PROCESS_NUM_01);

    std::string cpuData = "";
    cpuDataInfo01->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData01(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    HtraceCpuDataParser htraceCpuDataParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceCpuDataParser.Parse(cpuInfoData01, ts);

    CpuUsageInfo* cpuUsageInfo02 = new CpuUsageInfo();
    auto cpuDataInfo02(std::make_unique<CpuData>());
    cpuDataInfo02->set_allocated_cpu_usage_info(cpuUsageInfo02);
    cpuDataInfo02->set_total_load(TOTALLOAD_02);
    cpuDataInfo02->set_user_load(USERLOAD_02);
    cpuDataInfo02->set_sys_load(SYSTEMLOAD_02);
    cpuDataInfo02->set_process_num(PROCESS_NUM_02);

    cpuDataInfo02->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData02(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData02, ts);

    CpuUsageInfo* cpuUsageInfo03 = new CpuUsageInfo();
    auto cpuDataInfo03(std::make_unique<CpuData>());
    cpuDataInfo03->set_allocated_cpu_usage_info(cpuUsageInfo03);
    cpuDataInfo03->set_total_load(TOTALLOAD_03);
    cpuDataInfo03->set_user_load(USERLOAD_03);
    cpuDataInfo03->set_sys_load(SYSTEMLOAD_03);
    cpuDataInfo03->set_process_num(PROCESS_NUM_03);

    cpuDataInfo03->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData03(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData03, ts);

    CpuUsageInfo* cpuUsageInfo04 = new CpuUsageInfo();
    auto cpuDataInfo04(std::make_unique<CpuData>());
    cpuDataInfo04->set_allocated_cpu_usage_info(cpuUsageInfo04);
    cpuDataInfo04->set_total_load(TOTALLOAD_04);
    cpuDataInfo04->set_user_load(USERLOAD_04);
    cpuDataInfo04->set_sys_load(SYSTEMLOAD_04);
    cpuDataInfo04->set_process_num(PROCESS_NUM_04);

    cpuDataInfo04->SerializeToString(&cpuData);
    ProtoReader::BytesView cpuInfoData04(reinterpret_cast<const uint8_t*>(cpuData.data()), cpuData.size());
    htraceCpuDataParser.Parse(cpuInfoData04, ts);
    htraceCpuDataParser.Finish();

    auto size = stream_.traceDataCache_->GetConstCpuUsageInfoData().Size();
    EXPECT_EQ(3, size);

    auto totalLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[0];
    auto totalLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[1];
    auto totalLoadThird = stream_.traceDataCache_->GetConstCpuUsageInfoData().TotalLoad()[2];
    EXPECT_EQ(totalLoadFirst, TOTALLOAD_02);
    EXPECT_EQ(totalLoadSecond, TOTALLOAD_03);
    EXPECT_EQ(totalLoadThird, TOTALLOAD_04);

    auto userLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[0];
    auto userLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[1];
    auto userLoadThird = stream_.traceDataCache_->GetConstCpuUsageInfoData().UserLoad()[2];
    EXPECT_EQ(userLoadFirst, USERLOAD_02);
    EXPECT_EQ(userLoadSecond, USERLOAD_03);
    EXPECT_EQ(userLoadThird, USERLOAD_04);

    auto systemLoadFirst = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[0];
    auto systemLoadSecond = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[1];
    auto systemLoadThird = stream_.traceDataCache_->GetConstCpuUsageInfoData().SystemLoad()[2];
    EXPECT_EQ(systemLoadFirst, SYSTEMLOAD_02);
    EXPECT_EQ(systemLoadSecond, SYSTEMLOAD_03);
    EXPECT_EQ(systemLoadThird, SYSTEMLOAD_04);
}
} // namespace TraceStreamer
} // namespace SysTuning
