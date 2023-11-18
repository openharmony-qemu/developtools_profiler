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

#include "memory_plugin_result.pb.h"
#include "htrace_mem_parser.h"
#include "memory_plugin_result.pbreader.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"
#include "process_filter.h"

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
    const std::string dbPath_ = "../../test/resource/out.db";
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
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessMeasureData().Size(), MEM_PURG_SUM * 1);
    EXPECT_EQ(stream_.traceDataCache_->GetConstProcessData().size(), 2);

    for (auto i = 0; i < MEM_PURG_SUM; i++) {
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

    EXPECT_TRUE(stream_.traceDataCache_->GetConstProcessMeasureData().Size() == MEM_PURG_SUM * 2);
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

/**
 * @tc.name: ParseAshmemInfo
 * @tc.desc: Parse Ashmem Info
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseAshmemInfo, TestSize.Level1)
{
    TS_LOGI("test16-6");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    AshmemInfo* ashmemInfo = tracePacket.add_ashmeminfo();
    EXPECT_TRUE(ashmemInfo != nullptr);
    int32_t size = tracePacket.ashmeminfo_size();
    EXPECT_TRUE(size == 1);

    int32_t id = 0;
    int64_t time = 1616439852302;
    int32_t pid = 6;
    int32_t adj = 6;
    int32_t fd = 6;
    int64_t purged = 6;
    uint64_t setSize = 6;
    int64_t refCount = 6;
    ashmemInfo->set_id(id);
    ashmemInfo->set_time(time);
    ashmemInfo->set_pid(pid);
    ashmemInfo->set_adj(adj);
    ashmemInfo->set_fd(fd);
    ashmemInfo->set_purged(purged);
    ashmemInfo->set_size(setSize);
    ashmemInfo->set_ref_count(refCount);

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
    EXPECT_EQ(stream_.traceDataCache_->GetConstAshMemData().ids_[0], id);
    EXPECT_EQ(stream_.traceDataCache_->GetConstAshMemData().adjs_[0], adj);
    EXPECT_EQ(stream_.traceDataCache_->GetConstAshMemData().times_[0], time);
}

/**
 * @tc.name: ParseDmaMemInfo
 * @tc.desc: Parse DmaMem Info
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseDmaMemInfo, TestSize.Level1)
{
    TS_LOGI("test16-7");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    DmaInfo* dmaInfo = tracePacket.add_dmainfo();
    EXPECT_TRUE(dmaInfo != nullptr);
    int32_t size = tracePacket.dmainfo_size();
    EXPECT_TRUE(size == 1);

    uint64_t setSize = 7;
    int32_t pid = 7;
    int32_t ino = 7;
    int32_t fd = 7;
    int64_t expPid = 7;
    dmaInfo->set_size(setSize);
    dmaInfo->set_pid(pid);
    dmaInfo->set_ino(ino);
    dmaInfo->set_fd(fd);
    dmaInfo->set_exp_pid(expPid);

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
    EXPECT_EQ(stream_.traceDataCache_->GetConstDmaMemData().inos_[0], ino);
    EXPECT_EQ(stream_.traceDataCache_->GetConstDmaMemData().fds_[0], fd);
    EXPECT_EQ(stream_.traceDataCache_->GetConstDmaMemData().expPids_[0], expPid);
}

/**
 * @tc.name: ParseGpuProcessMemInfo
 * @tc.desc: Parse GpuProcessMem Info
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseGpuProcessMemInfo, TestSize.Level1)
{
    TS_LOGI("test16-8");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    GpuMemoryInfo* gpuMemoryInfo = tracePacket.add_gpumemoryinfo();

    int32_t allGpuSize = 0;
    gpuMemoryInfo->set_all_gpu_size(allGpuSize);

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
    EXPECT_EQ(stream_.traceDataCache_->GetConstGpuProcessMemData().AllGpuSizes()[0], allGpuSize);
}

/**
 * @tc.name: ParseGpuWindowMemInfo
 * @tc.desc: Parse GpuWindowMem Info
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, ParseGpuWindowMemInfo, TestSize.Level1)
{
    TS_LOGI("test16-9");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    GpuDumpInfo* gpuDumpInfo = tracePacket.add_gpudumpinfo();

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
    EXPECT_EQ(stream_.traceDataCache_->GetConstGpuWindowMemData().Size(), 0);
}

/**
 * @tc.name: AshMemDeduplicateTest
 * @tc.desc: AshMem Deduplicate Test
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, AshMemDeduplicateTest, TestSize.Level1)
{
    TS_LOGI("test16-10");

    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    uint32_t adj = 6;
    uint32_t fd = 6;
    DataIndex ashmemNameId = stream_.traceDataCache_->GetDataIndex("xxx");
    uint64_t size = 222;
    uint64_t refCount = 3;
    uint64_t purged = 1;
    uint32_t flag = 0;
    uint64_t pss = 0;

    struct DeduplicateVar {
        uint64_t timeStamp;
        uint64_t pid;
        std::string_view pidName;
        uint32_t ashmemId;
        uint64_t time;
    };
    vector<DeduplicateVar> stubVars = {
        {1616439852302, 1, "aaa", 1, 1}, {1616439852302, 1, "aaa", 1, 1}, {1616439852302, 1, "aaa", 2, 2},
        {1616439852302, 2, "bbb", 1, 1}, {1616439852302, 2, "bbb", 2, 2}, {1616439852302, 3, "ccc", 1, 1},
        {1616439852302, 3, "ccc", 2, 2}, {1616439852302, 3, "ccc", 2, 2},

        {1616439855302, 1, "aaa", 1, 1}, {1616439855302, 1, "aaa", 1, 1}, {1616439855302, 2, "bbb", 2, 2},
        {1616439855302, 3, "ccc", 2, 2},
    };
    for (auto& m : stubVars) {
        auto ipid = stream_.streamFilters_->processFilter_->UpdateOrCreateProcessWithName(m.pid, m.pidName);
        stream_.traceDataCache_->GetAshMemData()->AppendNewData(ipid, m.timeStamp, adj, fd, ashmemNameId, size, pss,
                                                                m.ashmemId, m.time, refCount, purged, flag);
    }

    memParser->AshMemDeduplicate();

    auto ashMemData = stream_.traceDataCache_->GetConstAshMemData();
    EXPECT_EQ(ashMemData.Flags()[0], 0);
    EXPECT_EQ(ashMemData.Flags()[1], 1);
    EXPECT_EQ(ashMemData.Flags()[2], 0);
    EXPECT_EQ(ashMemData.Flags()[3], 2);
    EXPECT_EQ(ashMemData.Flags()[4], 2);
    EXPECT_EQ(ashMemData.Flags()[5], 2);
    EXPECT_EQ(ashMemData.Flags()[6], 2);
    EXPECT_EQ(ashMemData.Flags()[7], 1);
    EXPECT_EQ(ashMemData.Flags()[8], 0);
    EXPECT_EQ(ashMemData.Flags()[9], 1);
    EXPECT_EQ(ashMemData.Flags()[10], 0);
    EXPECT_EQ(ashMemData.Flags()[11], 2);
}

/**
 * @tc.name: DmaMemDeduplicateTest
 * @tc.desc: DmaMem Deduplicate Test
 * @tc.type: FUNC
 */
HWTEST_F(HtraceMemParserTest, DmaMemDeduplicateTest, TestSize.Level1)
{
    TS_LOGI("test16-11");

    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    uint32_t fd = 6;
    uint64_t size = 222;
    uint32_t flag = 0;
    uint64_t expPid = 5;
    DataIndex expTaskCommId = stream_.traceDataCache_->GetDataIndex("aaa");
    DataIndex bufNameId = stream_.traceDataCache_->GetDataIndex("bbb");
    DataIndex expNameId = stream_.traceDataCache_->GetDataIndex("ccc");

    struct DeduplicateVar {
        uint64_t timeStamp;
        uint64_t pid;
        std::string_view pidName;
        uint32_t ino;
    };
    vector<DeduplicateVar> stubVars = {
        {1616439852302, 1, "render_service", 1},
        {1616439852302, 1, "render_service", 1},
        {1616439852302, 1, "render_service", 2},
        {1616439852302, 2, "app", 1},
        {1616439852302, 2, "app", 2},
        {1616439852302, 3, "composer_host", 1},
        {1616439852302, 3, "composer_host", 2},
        {1616439852302, 3, "composer_host", 2},

        {1616439855302, 1, "render_service", 1},
        {1616439855302, 1, "render_service", 2},
        {1616439855302, 3, "composer_host", 2},
        {1616439855302, 3, "composer_host", 2},
    };
    for (auto& m : stubVars) {
        auto ipid = stream_.streamFilters_->processFilter_->UpdateOrCreateProcessWithName(m.pid, m.pidName);
        stream_.traceDataCache_->GetDmaMemData()->AppendNewData(ipid, m.timeStamp, fd, size, m.ino, expPid,
                                                                expTaskCommId, bufNameId, expNameId, flag);
    }

    memParser->DmaMemDeduplicate();

    auto DmaData = stream_.traceDataCache_->GetConstDmaMemData();
    EXPECT_EQ(DmaData.Flags()[0], 2);
    EXPECT_EQ(DmaData.Flags()[1], 1);
    EXPECT_EQ(DmaData.Flags()[2], 2);
    EXPECT_EQ(DmaData.Flags()[3], 0);
    EXPECT_EQ(DmaData.Flags()[4], 0);
    EXPECT_EQ(DmaData.Flags()[5], 2);
    EXPECT_EQ(DmaData.Flags()[6], 2);
    EXPECT_EQ(DmaData.Flags()[7], 1);
    EXPECT_EQ(DmaData.Flags()[8], 0);
    EXPECT_EQ(DmaData.Flags()[9], 0);
    EXPECT_EQ(DmaData.Flags()[10], 2);
    EXPECT_EQ(DmaData.Flags()[11], 1);
}

} // namespace TraceStreamer
} // namespace SysTuning
