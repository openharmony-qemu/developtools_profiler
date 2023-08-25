
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
#include "paged_memory_data_parser.h"
#include "cpu_filter.h"
#include "ebpf_data_parser.h"
#include "ebpf_stdtype.h"
#include "process_filter.h"
#include "trace_streamer_selector.h"
#include "ts_common.h"
using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::EbpfStdtype;
namespace SysTuning ::TraceStreamer {
const std::string COMMAND_LINE = "hiebpf --events ptrace --duration 50";
class EbpfPagedMemoryParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
};
const uint64_t START_TIME = 1725645867369;
const uint64_t END_TIME = 1725645967369;
const uint64_t PAGEED_MEM_ADDR = 46549876;
const uint64_t IPS_01 = 548606407208;
const uint64_t IPS_02 = 548607407208;
const uint64_t EBPF_COMMAND_MAX_SIZE = 1000;
/**
 * @tc.name: EbpfPagedMemoryParserCorrectWithoutCallback
 * @tc.desc: Test parse PagedMem data without callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfPagedMemoryParserTest, EbpfPagedMemoryParserCorrectWithoutCallback, TestSize.Level1)
{
    TS_LOGI("test31-1");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfHeader))[EbpfDataHeader::EBPF_DATA_HEADER_SIZE]);
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(PagedMemoryFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_VM;
    PagedMemoryFixedHeader pagedMemoryFixedHeader;
    pagedMemoryFixedHeader.pid = 32;
    pagedMemoryFixedHeader.tid = 32;
    memcpy_s(pagedMemoryFixedHeader.comm, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    pagedMemoryFixedHeader.startTime = START_TIME;
    pagedMemoryFixedHeader.endTime = END_TIME;
    pagedMemoryFixedHeader.addr = PAGEED_MEM_ADDR;
    pagedMemoryFixedHeader.size = 1;
    pagedMemoryFixedHeader.nips = 0;
    pagedMemoryFixedHeader.type = 2;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[sizeof(EbpfTypeAndLength)]);
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader))[sizeof(PagedMemoryFixedHeader)]);

    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetPagedMemoryMap().size());
    ebpfDataParser->ParsePagedMemoryEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstPagedMemorySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstPagedMemorySampleData().Types()[0];
    EXPECT_EQ(type, 2);

    auto startTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstPagedMemorySampleData().Durs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto size = stream_.traceDataCache_->GetConstPagedMemorySampleData().Sizes()[0];
    EXPECT_EQ(size, 1);
    auto ExpectAddr = ebpfDataParser->ConvertToHexTextIndex(pagedMemoryFixedHeader.addr);
    auto addr = stream_.traceDataCache_->GetConstPagedMemorySampleData().Addr()[0];
    EXPECT_EQ(addr, ExpectAddr);
}

/**
 * @tc.name: EbpfPagedMemoryParserwrongWithoutCallback
 * @tc.desc: Test parse pagedMem data without callback and startTs > endTs
 * @tc.type: FUNC
 */
HWTEST_F(EbpfPagedMemoryParserTest, EbpfPagedMemoryParserwrongWithoutCallback, TestSize.Level1)
{
    TS_LOGI("test31-2");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfHeader))[EbpfDataHeader::EBPF_DATA_HEADER_SIZE]);
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(PagedMemoryFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_VM;
    PagedMemoryFixedHeader pagedMemoryFixedHeader;
    pagedMemoryFixedHeader.pid = 32;
    pagedMemoryFixedHeader.tid = 32;
    memcpy_s(pagedMemoryFixedHeader.comm, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    pagedMemoryFixedHeader.startTime = END_TIME;
    pagedMemoryFixedHeader.endTime = START_TIME;
    pagedMemoryFixedHeader.addr = PAGEED_MEM_ADDR;
    pagedMemoryFixedHeader.size = 1;
    pagedMemoryFixedHeader.nips = 0;
    pagedMemoryFixedHeader.type = 2;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[sizeof(EbpfTypeAndLength)]);
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader))[sizeof(PagedMemoryFixedHeader)]);

    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetPagedMemoryMap().size());
    ebpfDataParser->ParsePagedMemoryEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstPagedMemorySampleData().CallChainIds()[0];
    EXPECT_FALSE(callChainId == INVALID_UINT64);
    auto type = stream_.traceDataCache_->GetConstPagedMemorySampleData().Types()[0];
    EXPECT_FALSE(type == 2);

    auto startTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().StartTs()[0];
    EXPECT_FALSE(startTs == pagedMemoryFixedHeader.startTime);
    auto endTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().EndTs()[0];
    EXPECT_FALSE(endTs == pagedMemoryFixedHeader.endTime);
    auto dur = stream_.traceDataCache_->GetConstPagedMemorySampleData().Durs()[0];
    EXPECT_FALSE(dur == endTs - startTs);
    auto size = stream_.traceDataCache_->GetConstPagedMemorySampleData().Sizes()[0];
    EXPECT_FALSE(size == 1);
    auto ExpectAddr = ebpfDataParser->ConvertToHexTextIndex(pagedMemoryFixedHeader.addr);
    auto addr = stream_.traceDataCache_->GetConstPagedMemorySampleData().Addr()[0];
    EXPECT_FALSE(addr == ExpectAddr);
}

/**
 * @tc.name: EbpfPagedMemoryParserCorrectWithOneCallback
 * @tc.desc: Test parse PagedMem data with one callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfPagedMemoryParserTest, EbpfPagedMemoryParserCorrectWithOneCallback, TestSize.Level1)
{
    TS_LOGI("test31-3");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(PagedMemoryFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_VM;
    PagedMemoryFixedHeader pagedMemoryFixedHeader;
    pagedMemoryFixedHeader.pid = 32;
    pagedMemoryFixedHeader.tid = 32;
    memcpy_s(pagedMemoryFixedHeader.comm, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    pagedMemoryFixedHeader.startTime = START_TIME;
    pagedMemoryFixedHeader.endTime = END_TIME;
    pagedMemoryFixedHeader.addr = PAGEED_MEM_ADDR;
    pagedMemoryFixedHeader.size = 1;
    pagedMemoryFixedHeader.nips = 1;
    pagedMemoryFixedHeader.type = 2;
    const uint64_t ips[1] = {IPS_01};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader),
                       reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<const uint8_t*>(ips),
                       reinterpret_cast<const uint8_t*>(&ips + 1));
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetPagedMemoryMap().size());
    ebpfDataParser->ParsePagedMemoryEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstPagedMemorySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, 0);
    auto type = stream_.traceDataCache_->GetConstPagedMemorySampleData().Types()[0];
    EXPECT_EQ(type, 2);
    auto startTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstPagedMemorySampleData().Durs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto size = stream_.traceDataCache_->GetConstPagedMemorySampleData().Sizes()[0];
    EXPECT_EQ(size, 1);
    auto ExpectAddr = ebpfDataParser->ConvertToHexTextIndex(pagedMemoryFixedHeader.addr);
    auto addr = stream_.traceDataCache_->GetConstPagedMemorySampleData().Addr()[0];
    EXPECT_EQ(addr, ExpectAddr);
    auto ExpectIps0 = ebpfDataParser->ConvertToHexTextIndex(ips[0]);
    auto ips0 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[0];
    EXPECT_EQ(ips0, ExpectIps0);
}

/**
 * @tc.name: EbpfPagedMemoryParserCorrectWithMultipleCallback
 * @tc.desc: Test parse PagedMem data with Multiple callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfPagedMemoryParserTest, EbpfPagedMemoryParserCorrectWithMultipleCallback, TestSize.Level1)
{
    TS_LOGI("test31-4");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(PagedMemoryFixedHeader) + 2 * sizeof(uint64_t);
    ebpfTypeAndLength.type = ITEM_EVENT_VM;
    PagedMemoryFixedHeader pagedMemoryFixedHeader;
    pagedMemoryFixedHeader.pid = 32;
    pagedMemoryFixedHeader.tid = 32;
    memcpy_s(pagedMemoryFixedHeader.comm, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    pagedMemoryFixedHeader.startTime = START_TIME;
    pagedMemoryFixedHeader.endTime = END_TIME;
    pagedMemoryFixedHeader.addr = PAGEED_MEM_ADDR;
    pagedMemoryFixedHeader.size = 1;
    pagedMemoryFixedHeader.nips = 2;
    pagedMemoryFixedHeader.type = 2;
    const uint64_t ips[2] = {IPS_01, IPS_02};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader),
                       reinterpret_cast<uint8_t*>(&pagedMemoryFixedHeader + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<const uint8_t*>(ips),
                       reinterpret_cast<const uint8_t*>(&ips + 1));
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetPagedMemoryMap().size());
    ebpfDataParser->ParsePagedMemoryEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstPagedMemorySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, 0);
    auto type = stream_.traceDataCache_->GetConstPagedMemorySampleData().Types()[0];
    EXPECT_EQ(type, 2);
    auto startTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstPagedMemorySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstPagedMemorySampleData().Durs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto size = stream_.traceDataCache_->GetConstPagedMemorySampleData().Sizes()[0];
    EXPECT_EQ(size, 1);
    auto ExpectAddr = ebpfDataParser->ConvertToHexTextIndex(pagedMemoryFixedHeader.addr);
    auto addr = stream_.traceDataCache_->GetConstPagedMemorySampleData().Addr()[0];
    EXPECT_EQ(addr, ExpectAddr);
    auto ExpectIps0 = ebpfDataParser->ConvertToHexTextIndex(ips[0]);
    auto ips0 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[1];
    EXPECT_EQ(ips0, ExpectIps0);
    auto ExpectIps1 = ebpfDataParser->ConvertToHexTextIndex(ips[1]);
    auto ips1 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[0];
    EXPECT_EQ(ips1, ExpectIps1);
}
} // namespace SysTuning::TraceStreamer
