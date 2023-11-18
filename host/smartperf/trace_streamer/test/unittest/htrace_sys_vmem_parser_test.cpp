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
class HtraceSysVMemParserTest : public ::testing::Test {
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
 * @tc.name: ParseSysVMemParse
 * @tc.desc: Virtual memory parsing test, input a random reasonable value
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysVMemParserTest, ParseSysVMemParse, TestSize.Level1)
{
    TS_LOGI("test20-1");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysVMeminfo* vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    int32_t size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 1);
    vmem->set_key(SysVMeminfoType::VMEMINFO_UNSPECIFIED);
    uint64_t value = random();
    vmem->set_value(value);
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
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_meminfo();
    delete memParser;

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysVMemNomal
 * @tc.desc: Virtual memory parsing test nomal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysVMemParserTest, ParseSysVMemNomal, TestSize.Level1)
{
    TS_LOGI("test20-2");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysVMeminfo* vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    int32_t size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 1);
    vmem->set_key(SysVMeminfoType::VMEMINFO_NR_FREE_PAGES);
    uint64_t value = random();
    vmem->set_value(value);

    vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 2);
    vmem->set_key(SysVMeminfoType::VMEMINFO_NR_ALLOC_BATCH);
    uint64_t value2 = random();
    vmem->set_value(value2);

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
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_meminfo();
    delete memParser;

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[1] == static_cast<int64_t>(value2));
}

/**
 * @tc.name: ParseSysVMemAbnomal
 * @tc.desc: Virtual memory parsing test abnomal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysVMemParserTest, ParseSysVMemAbnomal, TestSize.Level1)
{
    TS_LOGI("test20-3");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysVMeminfo* vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    int32_t size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 1);
    vmem->set_key(SysVMeminfoType::VMEMINFO_NR_FREE_PAGES);
    uint64_t value = random();
    vmem->set_value(value);

    vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 2);
    uint64_t value2 = random();
    vmem->set_value(value2);

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
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_meminfo();
    delete memParser;

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_DATA_INVALID);
    EXPECT_TRUE(0 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysVMemWithMutiNomal
 * @tc.desc: Virtual memory parsing test with muti nomal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysVMemParserTest, ParseSysVMemWithMutiNomal, TestSize.Level1)
{
    TS_LOGI("test20-4");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysVMeminfo* vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    int32_t size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 1);
    vmem->set_key(SysVMeminfoType::VMEMINFO_WORKINGSET_RESTORE);
    uint64_t value = random();
    vmem->set_value(value);

    vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 2);
    vmem->set_key(SysVMeminfoType::SysVMeminfoType_INT_MIN_SENTINEL_DO_NOT_USE_);
    uint64_t value2 = random();
    vmem->set_value(value2);

    vmem = tracePacket.add_vmeminfo();
    EXPECT_TRUE(vmem != nullptr);
    size = tracePacket.vmeminfo_size();
    EXPECT_TRUE(size == 3);
    vmem->set_key(SysVMeminfoType::SysVMeminfoType_INT_MAX_SENTINEL_DO_NOT_USE_);
    uint64_t value3 = random();
    vmem->set_value(value3);

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
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_vmeminfo();
    delete memParser;

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_DATA_INVALID);
    EXPECT_TRUE(2 == eventCount);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysVMemWithRandomValue
 * @tc.desc: Virtual memory parsing test, input a random reasonable value
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysVMemParserTest, ParseSysVMemWithRandomValue, TestSize.Level1)
{
    TS_LOGI("test20-5");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;

    std::map<SysVMeminfoType, int64_t> sysVMemValueMap_ = {};
    for (auto i = 0; i < SysVMeminfoType::VMEMINFO_WORKINGSET_RESTORE + 1; i++) {
        uint64_t value = random();
        sysVMemValueMap_.insert(std::make_pair(static_cast<SysVMeminfoType>(i), value));
        SysVMeminfo* vmem = tracePacket.add_vmeminfo();
        EXPECT_TRUE(vmem != nullptr);
        vmem->set_key(static_cast<SysVMeminfoType>(i));
        vmem->set_value(value);
        int32_t size = tracePacket.vmeminfo_size();
        EXPECT_TRUE(size == i + 1);
    }

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
    stream_.traceDataCache_->ExportDatabase(dbPath_);

    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    tracePacket.clear_vmeminfo();
    delete memParser;

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    for (auto i = 0; i < SysVMeminfoType::VMEMINFO_WORKINGSET_RESTORE + 1; i++) {
        EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[i] ==
                    sysVMemValueMap_.at(static_cast<SysVMeminfoType>(i)));
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
