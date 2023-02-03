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

#include "htrace_mem_parser.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class SmapsParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown()
    {
        if (access(dbPath_.c_str(), F_OK) == 0) {
            remove(dbPath_.c_str());
        }
    }

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
    const std::string dbPath_ = "data/resource/out.db";
};
/**
 * @tc.name: ParseSmapsParse
 * @tc.desc: Parse SmapsData object and export database
 * @tc.type: FUNC
 */
HWTEST_F(SmapsParserTest, ParseSmapsParse, TestSize.Level1)
{
    TS_LOGI("test29-1");
    HtraceMemParser SmapsEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memInfo = tracePacket.add_processesinfo();
    SmapsInfo* smapsInfo = memInfo->add_smapinfo();
    EXPECT_TRUE(smapsInfo != nullptr);
    int size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 1);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock = TS_CLOCK_BOOTTIME;

    SmapsEvent.ParseProcessInfo(tracePacket, timeStamp);
    SmapsEvent.Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(),F_OK) == 0);
    memInfo->clear_smapinfo();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == MEM_MAX * 1);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 1);
}
/**
 * @tc.name: ParseSmapsParseTestMeasureDataSize
 * @tc.desc: Parse SmapsData object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(SmapsParserTest, ParseSmapsParseTestMeasureDataSize, TestSize.Level1)
{
    TS_LOGI("test29-2");
    HtraceMemParser SmapsEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memInfo = tracePacket.add_processesinfo();
    SmapsInfo* SmapsInfo = memInfo->add_smapinfo();
    EXPECT_TRUE(SmapsInfo != nullptr);
    int size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 1);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock = TS_CLOCK_BOOTTIME;
    std::string startAddr = "5589523000";
    std::string endAddr = "5589543000";
    std::string permission = "r--p";
    std::string path = "/system/bin/hiprofilerd";
    uint64_t vartualSize = 128;
    uint64_t rss = 112;
    uint64_t pss = 112;
    uint64_t reside = 87.5;
    SmapsInfo->set_start_addr(startAddr);
    SmapsInfo->set_end_addr(endAddr);
    SmapsInfo->set_permission(permission);
    SmapsInfo->set_path(path);
    SmapsInfo->set_size(vartualSize);
    SmapsInfo->set_rss(rss);
    SmapsInfo->set_pss(pss);
    SmapsInfo->set_reside(reside);

    SmapsEvent.ParseProcessInfo(tracePacket, timeStamp);
    SmapsEvent.Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(),F_OK) == 0);
    memInfo->clear_smapinfo();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == MEM_MAX * 1);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 1);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().StartAddrs()[0] == "0x5589523000");
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().EndAddrs()[0] == "0x5589543000");
    uint64_t protection = stream_.traceDataCache_->GetDataIndex(permission);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().ProtectionIds()[0] == protection);
    uint64_t pat = stream_.traceDataCache_->GetDataIndex(path);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().PathIds()[0] == pat);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Sizes()[0] == vartualSize);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Rss()[0] == rss);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Pss()[0] == pss);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Resides()[0] == reside);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 1);
}
/**
 * @tc.name: ParseSmapsParseTestMutiMeasureData
 * @tc.desc: Parse muti SmapsData object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(SmapsParserTest, ParseSmapsParseTestMutiMeasureData, TestSize.Level1)
{
    TS_LOGI("test29-3");
    HtraceMemParser SmapsEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memInfo = tracePacket.add_processesinfo();
    SmapsInfo* smapsInfo0 = memInfo->add_smapinfo();
    EXPECT_TRUE(smapsInfo0 != nullptr);
    int size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 1);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock0 = TS_CLOCK_BOOTTIME;
    std::string startAddr0 = "5589523000";
    std::string endAddr0 = "5589543000";
    std::string permission0 = "r--p";
    std::string path0 = "/system/bin/hiprofilerd";
    uint64_t vartualSize0 = 128;
    uint64_t rss0 = 112;
    uint64_t pss0 = 112;
    uint64_t reside0 = 87.5;
    smapsInfo0->set_start_addr(startAddr0);
    smapsInfo0->set_end_addr(endAddr0);
    smapsInfo0->set_permission(permission0);
    smapsInfo0->set_path(path0);
    smapsInfo0->set_size(vartualSize0);
    smapsInfo0->set_rss(rss0);
    smapsInfo0->set_pss(pss0);
    smapsInfo0->set_reside(reside0);

    SmapsInfo* smapsInfo1 = memInfo->add_smapinfo();
    EXPECT_TRUE(smapsInfo1 != nullptr);
    size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 2);
    timeStamp = 1616439852302;
    BuiltinClocks clock1 = TS_CLOCK_BOOTTIME;
    std::string startAddr1 = "5589543000";
    std::string endAddr1 = "5589589000";
    std::string permission1 = "r-xp";
    std::string path1 = "/system/bin/hiprofilerd";
    uint64_t vartualSize1 = 280;
    uint64_t rss1 = 280;
    uint64_t pss1 = 280;
    uint64_t reside1 = 100;
    smapsInfo1->set_start_addr(startAddr1);
    smapsInfo1->set_end_addr(endAddr1);
    smapsInfo1->set_permission(permission1);
    smapsInfo1->set_path(path1);
    smapsInfo1->set_size(vartualSize1);
    smapsInfo1->set_rss(rss1);
    smapsInfo1->set_pss(pss1);
    smapsInfo1->set_reside(reside1);

    SmapsEvent.ParseProcessInfo(tracePacket, timeStamp);
    SmapsEvent.Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(),F_OK) == 0);
    memInfo->clear_smapinfo();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().StartAddrs()[0] == "0x5589523000");
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().EndAddrs()[0] == "0x5589543000");
    uint64_t protection = stream_.traceDataCache_->GetDataIndex(permission0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().ProtectionIds()[0] == protection);
    uint64_t pathId = stream_.traceDataCache_->GetDataIndex(path0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().PathIds()[0] == pathId);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Sizes()[0] == vartualSize0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Rss()[0] == rss0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Pss()[0] == pss0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Resides()[0] == reside0);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().StartAddrs()[1] == "0x5589543000");
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().EndAddrs()[1] == "0x5589589000");
    protection = stream_.traceDataCache_->GetDataIndex(permission1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().ProtectionIds()[1] == protection);
    pathId = stream_.traceDataCache_->GetDataIndex(path1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().PathIds()[1] == pathId);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Sizes()[1] == vartualSize1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Rss()[1] == rss1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Pss()[1] == pss1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSmapsData().Resides()[1] == reside1);
}
/**
 * @tc.name: ParseMutiEmptySmapsDataAndCountStatInfo
 * @tc.desc: Parse muti Empty SmapsData object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(SmapsParserTest, ParseMutiEmptySmapsDataAndCountStatInfo, TestSize.Level1)
{
    TS_LOGI("test29-4");
    HtraceMemParser SmapsEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memInfo = tracePacket.add_processesinfo();
    SmapsInfo* smapsInfo0 = memInfo->add_smapinfo();
    EXPECT_TRUE(smapsInfo0 != nullptr);
    int size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 1);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock = TS_CLOCK_BOOTTIME;

    SmapsInfo* smapsInfo1 = memInfo->add_smapinfo();
    EXPECT_TRUE(smapsInfo1 != nullptr);
    size = memInfo->smapinfo_size();
    EXPECT_TRUE(size == 2);

    SmapsEvent.ParseProcessInfo(tracePacket, timeStamp);
    SmapsEvent.Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(),F_OK) == 0);
    memInfo->clear_smapinfo();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 1);
}
/**
 * @tc.name: ParseEmptySmapsData
 * @tc.desc: Parse Empty SmapsData
 * @tc.type: FUNC
 */
HWTEST_F(SmapsParserTest, ParseEmptySmapsData, TestSize.Level1)
{
    TS_LOGI("test29-5");
    HtraceMemParser SmapsEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    int size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 0);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock = TS_CLOCK_BOOTTIME;

    SmapsEvent.ParseProcessInfo(tracePacket, timeStamp);
    SmapsEvent.Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(),F_OK) == 0);
    tracePacket.clear_processesinfo();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    EXPECT_EQ(0, eventCount);
}
} // namespace TraceStreamer
} // namespace SysTuning
