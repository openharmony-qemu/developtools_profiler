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
#include <string>
#include <unordered_map>

#include "htrace_mem_parser.h"
#include "memory_plugin_result.pb.h"
#include "memory_plugin_result.pbreader.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HtraceMemParserTest : public ::testing::Test {
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
 * @tc.name: ParseMemParse
 * @tc.desc: Parse MemoryData object and export database
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseMemParse, TestSize.Level1)
{
    TS_LOGI("test16-1");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memoryInfo = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo != nullptr);
    int32_t size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 1);

    HtraceDataSegment dataSeg;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
    dataSeg.timeStamp = 1616439852302;

    std::string memStrMsg = "";
    tracePacket.SerializeToString(&memStrMsg);
    ProtoReader::BytesView memBytesView(reinterpret_cast<const uint8_t*>(memStrMsg.data()), memStrMsg.size());
    dataSeg.protoData = memBytesView;

    memParser->Parse(dataSeg, dataSeg.timeStamp, dataSeg.clockId);
    memParser->Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_processesinfo();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseMemParseTestMeasureDataSize
 * @tc.desc: Parse MemoryData object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseMemParseTestMeasureDataSize, TestSize.Level1)
{
    TS_LOGI("test16-2");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memoryInfo = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo != nullptr);
    int32_t size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 1);
    uint32_t pid = 12;
    int64_t memKb = 1024;
    int64_t memRssKb = 512;
    int64_t memAnonKb = 128;
    int64_t memFileKb = 2048;
    memoryInfo->set_pid(pid);
    memoryInfo->set_name("Process1");
    memoryInfo->set_vm_size_kb(memKb);
    memoryInfo->set_vm_rss_kb(memRssKb);
    memoryInfo->set_rss_anon_kb(memAnonKb);
    memoryInfo->set_rss_file_kb(memFileKb);

    HtraceDataSegment dataSeg;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
    dataSeg.timeStamp = 1616439852302;

    std::string memStrMsg = "";
    tracePacket.SerializeToString(&memStrMsg);
    ProtoReader::BytesView memBytesView(reinterpret_cast<const uint8_t*>(memStrMsg.data()), memStrMsg.size());
    dataSeg.protoData = memBytesView;

    memParser->Parse(dataSeg, dataSeg.timeStamp, dataSeg.clockId);
    memParser->Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_processesinfo();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessData(1).pid_ == pid);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == MEM_MAX * 1);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 2);

    for (auto i = 0; i < MEM_MAX; i++) {
        if (stream_.traceDataCache_->GetConstProcessMeasureData().filterIdDeque_[i] ==
            memParser->memNameDictMap_.at(MEM_VM_SIZE)) {
            EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().valuesDeque_[i] == memKb);
        } else if (stream_.traceDataCache_->GetConstProcessMeasureData().filterIdDeque_[i] ==
                   memParser->memNameDictMap_.at(MEM_VM_RSS)) {
            EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().valuesDeque_[i] == memRssKb);
        } else if (stream_.traceDataCache_->GetConstProcessMeasureData().filterIdDeque_[i] ==
                   memParser->memNameDictMap_.at(MEM_VM_ANON)) {
            EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().valuesDeque_[i] == memAnonKb);
        } else if (stream_.traceDataCache_->GetConstProcessMeasureData().filterIdDeque_[i] ==
                   memParser->memNameDictMap_.at(MEM_RSS_FILE)) {
            EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().valuesDeque_[i] == memFileKb);
        }
    }
}

/**
 * @tc.name: ParseMemParseTestMutiMeasureData
 * @tc.desc: Parse muti MemoryData object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseMemParseTestMutiMeasureData, TestSize.Level1)
{
    TS_LOGI("test16-3");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memoryInfo = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo != nullptr);
    int32_t size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 1);
    uint32_t pid = 12;
    memoryInfo->set_pid(12);
    memoryInfo->set_name("Process1");
    memoryInfo->set_vm_size_kb(1024);
    memoryInfo->set_vm_rss_kb(512);
    memoryInfo->set_rss_anon_kb(128);
    memoryInfo->set_rss_file_kb(128);

    ProcessMemoryInfo* memoryInfo2 = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo2 != nullptr);
    size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 2);
    uint32_t pid2 = 13;
    memoryInfo2->set_pid(pid2);
    memoryInfo2->set_name("Process2");
    memoryInfo2->set_vm_size_kb(1024);
    memoryInfo2->set_vm_rss_kb(512);
    memoryInfo2->set_rss_anon_kb(128);
    memoryInfo2->set_rss_file_kb(128);

    HtraceDataSegment dataSeg;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
    dataSeg.timeStamp = 1616439852402;

    std::string memStrMsg = "";
    tracePacket.SerializeToString(&memStrMsg);
    ProtoReader::BytesView memBytesView(reinterpret_cast<const uint8_t*>(memStrMsg.data()), memStrMsg.size());
    dataSeg.protoData = memBytesView;

    memParser->Parse(dataSeg, dataSeg.timeStamp, dataSeg.clockId);
    memParser->Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_processesinfo();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessData(1).pid_ == pid);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessData(2).pid_ == pid2);
}

/**
 * @tc.name: ParseMultiEmptyProcessMemoryInfo
 * @tc.desc: Parse muti Empty ProcessMemoryInfo object and count StatInfo
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseMultiEmptyProcessMemoryInfo, TestSize.Level1)
{
    TS_LOGI("test16-4");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    ProcessMemoryInfo* memoryInfo = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo != nullptr);
    int32_t size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 1);

    ProcessMemoryInfo* memoryInfo2 = tracePacket.add_processesinfo();
    EXPECT_TRUE(memoryInfo2 != nullptr);
    size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 2);

    HtraceDataSegment dataSeg;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
    dataSeg.timeStamp = 1616439852302;

    std::string memStrMsg = "";
    tracePacket.SerializeToString(&memStrMsg);
    ProtoReader::BytesView memBytesView(reinterpret_cast<const uint8_t*>(memStrMsg.data()), memStrMsg.size());
    dataSeg.protoData = memBytesView;

    memParser->Parse(dataSeg, dataSeg.timeStamp, dataSeg.clockId);
    memParser->Finish();
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_processesinfo();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == MEM_MAX * 2);
}

/**
 * @tc.name: ParseEmptyMemoryData
 * @tc.desc: Parse Empty MemoryData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseEmptyMemoryData, TestSize.Level1)
{
    TS_LOGI("test16-5");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    int32_t size = tracePacket.processesinfo_size();
    EXPECT_TRUE(size == 0);
    uint64_t timeStamp = 1616439852302;
    BuiltinClocks clock = TS_CLOCK_REALTIME;

    HtraceDataSegment dataSeg;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
    dataSeg.timeStamp = 1616439852302;

    std::string memStrMsg = "";
    tracePacket.SerializeToString(&memStrMsg);
    ProtoReader::BytesView memBytesView(reinterpret_cast<const uint8_t*>(memStrMsg.data()), memStrMsg.size());
    dataSeg.protoData = memBytesView;

    memParser->Parse(dataSeg, dataSeg.timeStamp, dataSeg.clockId);
    memParser->Finish();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(0 == eventCount);
}
} // namespace TraceStreamer
} // namespace SysTuning
