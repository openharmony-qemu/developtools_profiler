
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
#include "bio_latency_data_parser.h"
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
const uint64_t EPBF_ERROR_MAGIC = 0x12345678;
const uint32_t EPBF_ERROR_HEAD_SIZE = 0;
class EbpfBioParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
};
const char PROCESS_NAME_01[MAX_PROCESS_NAME_SZIE] = "process01";
const uint64_t START_TIME = 1725645867369;
const uint64_t END_TIME = 1725645967369;
const uint64_t BLKCNT = 7829248;
const uint64_t IPS_01 = 548606407208;
const uint64_t IPS_02 = 548607407208;
const uint64_t EBPF_COMMAND_MAX_SIZE = 1000;
const uint32_t DURPER4K = 4096;
/**
 * @tc.name: EbpfBioParserCorrectWithoutCallback
 * @tc.desc: Test parse BIO data without callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfBioParserTest, EbpfBioParserCorrectWithoutCallback, TestSize.Level1)
{
    TS_LOGI("test32-1");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    strcpy(ebpfHeader.cmdline, COMMAND_LINE.c_str());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(BIOFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_BIO;
    BIOFixedHeader bioFixedHeader;
    bioFixedHeader.pid = 32;
    bioFixedHeader.tid = 32;
    memcpy_s(bioFixedHeader.processName, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    bioFixedHeader.startTime = START_TIME;
    bioFixedHeader.endTime = END_TIME;
    bioFixedHeader.prio = 0;
    bioFixedHeader.size = DURPER4K;
    bioFixedHeader.blkcnt = BLKCNT;
    bioFixedHeader.nips = 0;
    bioFixedHeader.type = 2;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfTypeAndLength))[sizeof(EbpfTypeAndLength)]);
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&bioFixedHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&bioFixedHeader))[sizeof(BIOFixedHeader)]);
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetBIOSampleMap().size());
    ebpfDataParser->ParseBioLatencyEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstBioLatencySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstBioLatencySampleData().Types()[0];
    EXPECT_EQ(type, 2);
    auto startTs = stream_.traceDataCache_->GetConstBioLatencySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstBioLatencySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstBioLatencySampleData().LatencyDurs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto tier = stream_.traceDataCache_->GetConstBioLatencySampleData().Tiers()[0];
    EXPECT_EQ(tier, 0);
    auto size = stream_.traceDataCache_->GetConstBioLatencySampleData().Sizes()[0];
    EXPECT_EQ(size, DURPER4K);
    auto Expectblk = ebpfDataParser->ConvertToHexTextIndex(BLKCNT);
    auto blk = stream_.traceDataCache_->GetConstBioLatencySampleData().BlockNumbers()[0];
    EXPECT_EQ(blk, Expectblk);
    auto durPer4K = stream_.traceDataCache_->GetConstBioLatencySampleData().DurPer4k()[0];
    EXPECT_EQ(durPer4K, dur / (size / DURPER4K));
}

/**
 * @tc.name: EbpfBioParserwrongWithoutCallback
 * @tc.desc: Test parse BIO data without callback and startTs > endTs
 * @tc.type: FUNC
 */
HWTEST_F(EbpfBioParserTest, EbpfBioParserwrongWithoutCallback, TestSize.Level1)
{
    TS_LOGI("test32-2");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    strcpy(ebpfHeader.cmdline, COMMAND_LINE.c_str());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(BIOFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_BIO;
    BIOFixedHeader bioFixedHeader;
    bioFixedHeader.pid = 32;
    bioFixedHeader.tid = 32;
    memcpy_s(bioFixedHeader.processName, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    bioFixedHeader.startTime = END_TIME;
    bioFixedHeader.endTime = START_TIME;
    bioFixedHeader.prio = 0;
    bioFixedHeader.size = DURPER4K;
    bioFixedHeader.blkcnt = BLKCNT;
    bioFixedHeader.nips = 0;
    bioFixedHeader.type = 2;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&bioFixedHeader),
                       reinterpret_cast<uint8_t*>(&bioFixedHeader + 1));
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetBIOSampleMap().size());
    ebpfDataParser->ParseBioLatencyEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstBioLatencySampleData().CallChainIds()[0];
    EXPECT_FALSE(callChainId == INVALID_UINT64);
    auto type = stream_.traceDataCache_->GetConstBioLatencySampleData().Types()[0];
    EXPECT_FALSE(type == 2);
    auto startTs = stream_.traceDataCache_->GetConstBioLatencySampleData().StartTs()[0];
    EXPECT_FALSE(startTs == END_TIME);
    auto endTs = stream_.traceDataCache_->GetConstBioLatencySampleData().EndTs()[0];
    EXPECT_FALSE(endTs == START_TIME);
    auto dur = stream_.traceDataCache_->GetConstBioLatencySampleData().LatencyDurs()[0];
    EXPECT_FALSE(dur == endTs - startTs);
    auto tier = stream_.traceDataCache_->GetConstBioLatencySampleData().Tiers()[0];
    EXPECT_FALSE(tier == 0);
    auto size = stream_.traceDataCache_->GetConstBioLatencySampleData().Sizes()[0];
    EXPECT_FALSE(size == DURPER4K);
    auto Expectblk = ebpfDataParser->ConvertToHexTextIndex(BLKCNT);
    auto blk = stream_.traceDataCache_->GetConstBioLatencySampleData().BlockNumbers()[0];
    EXPECT_FALSE(blk == Expectblk);
    auto durPer4K = stream_.traceDataCache_->GetConstBioLatencySampleData().DurPer4k()[0];
    EXPECT_FALSE(durPer4K == dur / (size / DURPER4K));
}

/**
 * @tc.name: EbpfBioParserCorrectWithOneCallback
 * @tc.desc: Test parse BIO data with one callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfBioParserTest, EbpfBioParserCorrectWithOneCallback, TestSize.Level1)
{
    TS_LOGI("test32-3");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(BIOFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_BIO;
    BIOFixedHeader bioFixedHeader;
    bioFixedHeader.pid = 32;
    bioFixedHeader.tid = 32;
    memcpy_s(bioFixedHeader.processName, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    bioFixedHeader.startTime = START_TIME;
    bioFixedHeader.endTime = END_TIME;
    bioFixedHeader.prio = 0;
    bioFixedHeader.size = DURPER4K;
    bioFixedHeader.blkcnt = BLKCNT;
    bioFixedHeader.nips = 1;
    bioFixedHeader.type = 2;
    const uint64_t ips[1] = {IPS_01};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&bioFixedHeader),
                       reinterpret_cast<uint8_t*>(&bioFixedHeader + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<const uint8_t*>(ips),
                       reinterpret_cast<const uint8_t*>(&ips + 1));
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetBIOSampleMap().size());
    ebpfDataParser->ParseBioLatencyEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstBioLatencySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, 0);
    auto type = stream_.traceDataCache_->GetConstBioLatencySampleData().Types()[0];
    EXPECT_EQ(type, 2);
    auto ipid = stream_.traceDataCache_->GetConstBioLatencySampleData().Ipids()[0];
    EXPECT_EQ(ipid, 1);
    auto itid = stream_.traceDataCache_->GetConstBioLatencySampleData().Itids()[0];
    EXPECT_EQ(itid, 1);
    auto startTs = stream_.traceDataCache_->GetConstBioLatencySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstBioLatencySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstBioLatencySampleData().LatencyDurs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto tier = stream_.traceDataCache_->GetConstBioLatencySampleData().Tiers()[0];
    EXPECT_EQ(tier, 0);
    auto size = stream_.traceDataCache_->GetConstBioLatencySampleData().Sizes()[0];
    EXPECT_EQ(size, DURPER4K);
    auto Expectblk = ebpfDataParser->ConvertToHexTextIndex(BLKCNT);
    auto blk = stream_.traceDataCache_->GetConstBioLatencySampleData().BlockNumbers()[0];
    EXPECT_EQ(blk, Expectblk);
    auto durPer4K = stream_.traceDataCache_->GetConstBioLatencySampleData().DurPer4k()[0];
    EXPECT_EQ(durPer4K, dur / (size / DURPER4K));
    auto ExpectIps0 = ebpfDataParser->ConvertToHexTextIndex(ips[0]);
    auto ips0 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[0];
    EXPECT_EQ(ips0, ExpectIps0);
}

/**
 * @tc.name: EbpfBioParserCorrectWithMultipleCallback
 * @tc.desc: Test parse BIO data with multiple callback
 * @tc.type: FUNC
 */
HWTEST_F(EbpfBioParserTest, EbpfBioParserCorrectWithMultipleCallback, TestSize.Level1)
{
    TS_LOGI("test32-4");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    memcpy_s(ebpfHeader.cmdline, EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(), COMMAND_LINE.length());
    std::deque<uint8_t> dequeBuffer;
    dequeBuffer.insert(dequeBuffer.end(), &(reinterpret_cast<uint8_t*>(&ebpfHeader))[0],
                       &(reinterpret_cast<uint8_t*>(&ebpfHeader))[EbpfDataHeader::EBPF_DATA_HEADER_SIZE]);
    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(BIOFixedHeader) + 2 * sizeof(uint64_t);
    ebpfTypeAndLength.type = ITEM_EVENT_BIO;
    BIOFixedHeader bioFixedHeader;
    bioFixedHeader.pid = 32;
    bioFixedHeader.tid = 32;
    memcpy_s(bioFixedHeader.processName, MAX_PROCESS_NAME_SZIE, "process", MAX_PROCESS_NAME_SZIE);
    bioFixedHeader.startTime = START_TIME;
    bioFixedHeader.endTime = END_TIME;
    bioFixedHeader.prio = 0;
    bioFixedHeader.size = DURPER4K;
    bioFixedHeader.blkcnt = BLKCNT;
    bioFixedHeader.nips = 2;
    bioFixedHeader.type = 2;
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&bioFixedHeader),
                       reinterpret_cast<uint8_t*>(&bioFixedHeader + 1));
    const uint64_t ips[2] = {IPS_01, IPS_02};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<const uint8_t*>(ips),
                       reinterpret_cast<const uint8_t*>(&ips + 1));
    std::unique_ptr<EbpfDataParser> ebpfDataParser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(ebpfDataParser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(ebpfDataParser->reader_->GetBIOSampleMap().size());
    ebpfDataParser->ParseBioLatencyEvent();
    ebpfDataParser->Finish();
    EXPECT_TRUE(ebpfDataParser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstBioLatencySampleData().CallChainIds()[0];
    EXPECT_EQ(callChainId, 0);
    auto type = stream_.traceDataCache_->GetConstBioLatencySampleData().Types()[0];
    EXPECT_EQ(type, 2);
    auto ipid = stream_.traceDataCache_->GetConstBioLatencySampleData().Ipids()[0];
    EXPECT_EQ(ipid, 1);
    auto itid = stream_.traceDataCache_->GetConstBioLatencySampleData().Itids()[0];
    EXPECT_EQ(itid, 1);
    auto startTs = stream_.traceDataCache_->GetConstBioLatencySampleData().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME);
    auto endTs = stream_.traceDataCache_->GetConstBioLatencySampleData().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME);
    auto dur = stream_.traceDataCache_->GetConstBioLatencySampleData().LatencyDurs()[0];
    EXPECT_EQ(dur, END_TIME - START_TIME);
    auto tier = stream_.traceDataCache_->GetConstBioLatencySampleData().Tiers()[0];
    EXPECT_EQ(tier, 0);
    auto size = stream_.traceDataCache_->GetConstBioLatencySampleData().Sizes()[0];
    EXPECT_EQ(size, DURPER4K);
    auto Expectblk = ebpfDataParser->ConvertToHexTextIndex(BLKCNT);
    auto blk = stream_.traceDataCache_->GetConstBioLatencySampleData().BlockNumbers()[0];
    EXPECT_EQ(blk, Expectblk);
    auto durPer4K = stream_.traceDataCache_->GetConstBioLatencySampleData().DurPer4k()[0];
    EXPECT_EQ(durPer4K, dur / (size / DURPER4K));
    auto ExpectIps0 = ebpfDataParser->ConvertToHexTextIndex(ips[0]);
    auto ips0 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[1];
    EXPECT_EQ(ips0, ExpectIps0);
    auto ExpectIps1 = ebpfDataParser->ConvertToHexTextIndex(ips[1]);
    auto ips1 = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[0];
    EXPECT_EQ(ips1, ExpectIps1);
}
} // namespace SysTuning::TraceStreamer
