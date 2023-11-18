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
class HtraceSysMemParserTest : public ::testing::Test {
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
 * @tc.name: ParseSysMemParseInputEmpty
 * @tc.desc: Kernel memory parsing test, input empty
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysMemParserTest, ParseSysMemParseInputEmpty, TestSize.Level1)
{
    TS_LOGI("test19-1");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    MemoryData tracePacket;
    SysMeminfo* mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    int32_t size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 1);
    mem->set_key(SysMeminfoType::PMEM_MEM_TOTAL);
    uint64_t value = random();
    mem->set_value(value);
    uint64_t zarm = 100;
    tracePacket.set_zram(zarm);

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

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_EQ(4, stream_.traceDataCache_->GetConstSysMeasureFilterData().Size());
    EXPECT_EQ(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0], static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysMemParseNormal
 * @tc.desc: Kernel memory parsing test normal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysMemParserTest, ParseSysMemParseNormal, TestSize.Level1)
{
    TS_LOGI("test19-2");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysMeminfo* mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    int32_t size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 1);
    mem->set_key(SysMeminfoType::PMEM_MEM_TOTAL);
    uint64_t value = random();
    mem->set_value(value);

    mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 2);
    mem->set_key(SysMeminfoType::PMEM_MEM_FREE);
    uint64_t value2 = random();
    mem->set_value(value2);
    uint64_t zarm = 100;
    tracePacket.set_zram(zarm);

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

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_EQ(5, stream_.traceDataCache_->GetConstSysMemMeasureData().Size());
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[1] == static_cast<int64_t>(value2));
}

/**
 * @tc.name: ParseSysMemParseAbnomal
 * @tc.desc: Kernel memory parsing test abnomal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysMemParserTest, ParseSysMemParseAbnomal, TestSize.Level1)
{
    TS_LOGI("test19-3");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysMeminfo* mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    int32_t size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 1);
    mem->set_key(SysMeminfoType::PMEM_MEM_TOTAL);
    uint64_t value = random();
    mem->set_value(value);

    mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 2);
    mem->set_key(static_cast<SysMeminfoType>(199999)); // invalid data
    uint64_t value2 = random();
    mem->set_value(value2);
    uint64_t zarm = 100;
    tracePacket.set_zram(zarm);

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

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_DATA_INVALID);
    EXPECT_TRUE(1 == eventCount);
    EXPECT_EQ(4, stream_.traceDataCache_->GetConstSysMemMeasureData().Size());
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysMemParseMutiNomal
 * @tc.desc: Kernel memory parsing test with muti nomal
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysMemParserTest, ParseSysMemParseMutiNomal, TestSize.Level1)
{
    TS_LOGI("test19-4");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;
    SysMeminfo* mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    int32_t size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 1);
    mem->set_key(SysMeminfoType::PMEM_KERNEL_RECLAIMABLE);
    uint64_t value = random();
    mem->set_value(value);

    mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 2);
    mem->set_key(SysMeminfoType::SysMeminfoType_INT_MIN_SENTINEL_DO_NOT_USE_);
    uint64_t value2 = random();
    mem->set_value(value2);

    mem = tracePacket.add_meminfo();
    EXPECT_TRUE(mem != nullptr);
    size = tracePacket.meminfo_size();
    EXPECT_TRUE(size == 3);
    mem->set_key(SysMeminfoType::SysMeminfoType_INT_MAX_SENTINEL_DO_NOT_USE_);
    uint64_t value3 = random();
    mem->set_value(value3);
    uint64_t zarm = 100;
    tracePacket.set_zram(zarm);

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

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_DATA_INVALID);
    EXPECT_TRUE(2 == eventCount);
    EXPECT_EQ(4, stream_.traceDataCache_->GetConstSysMemMeasureData().Size());
    EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[0] == static_cast<int64_t>(value));
}

/**
 * @tc.name: ParseSysMemParseWithRandomValue
 * @tc.desc: Kernel memory parsing test, input a random reasonable value
 * @tc.type: FUNC
 */
HWTEST_F(HtraceSysMemParserTest, ParseSysMemParseWithRandomValue, TestSize.Level1)
{
    TS_LOGI("test19-5");
    HtraceMemParser* memParser = new HtraceMemParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    MemoryData tracePacket;

    std::map<SysMeminfoType, int64_t> sysMemValueMap_ = {};
    for (auto i = 0; i < SysMeminfoType::PMEM_KERNEL_RECLAIMABLE + 1; i++) {
        uint64_t value = random();
        sysMemValueMap_.insert(std::make_pair(static_cast<SysMeminfoType>(i), value));
        SysMeminfo* mem = tracePacket.add_meminfo();
        EXPECT_TRUE(mem != nullptr);
        mem->set_key(static_cast<SysMeminfoType>(i));
        mem->set_value(value);
        int32_t size = tracePacket.meminfo_size();
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
    tracePacket.clear_meminfo();
    delete memParser;

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);

    for (auto i = 0; i < SysMeminfoType::PMEM_KERNEL_RECLAIMABLE + 1; i++) {
        EXPECT_TRUE(stream_.traceDataCache_->GetConstSysMemMeasureData().ValuesData()[i] ==
                    sysMemValueMap_.at(static_cast<SysMeminfoType>(i)));
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
