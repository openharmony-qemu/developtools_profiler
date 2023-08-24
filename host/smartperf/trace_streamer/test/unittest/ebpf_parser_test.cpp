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

#include "ebpf_data_parser.h"
#include "ebpf_stdtype.h"
#include "htrace_file_header.h"
#include "process_filter.h"
#include "trace_streamer_selector.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::EbpfStdtype;
namespace SysTuning {
namespace TraceStreamer {
const std::string COMMAND_LINE = "hiebpf --events ptrace --duration 50";
const uint64_t EPBF_ERROR_MAGIC = 0x12345678;
const uint32_t EPBF_ERROR_HEAD_SIZE = 0;
class EbpfParserTest : public ::testing::Test {
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
 * @tc.name: EbpfDataOnlyEbpfHeadWithErrorMagic
 * @tc.desc: Test parse Ebpf data with only ebpf head but no command line
 * @tc.type: FUNC
 */
HWTEST_F(EbpfParserTest, EbpfDataOnlyEbpfHeadWithErrorMagic, TestSize.Level1)
{
    TS_LOGI("test29-1");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.magic = EPBF_ERROR_MAGIC;
    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_FALSE(parser->Init(dequeBuffer, dequeBuffer.size()));
}

/**
 * @tc.name: EbpfDataOnlyEbpfHeadWithErrorSize
 * @tc.desc: Test parse Ebpf data with only ebpf head but no command line
 * @tc.type: FUNC
 */
HWTEST_F(EbpfParserTest, EbpfDataOnlyEbpfHeadWithErrorSize, TestSize.Level1)
{
    TS_LOGI("test29-2");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.headSize = EPBF_ERROR_HEAD_SIZE;
    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_FALSE(parser->Init(dequeBuffer, dequeBuffer.size()));
}

/**
 * @tc.name: EbpfDataEbpfHeadWithNormalData
 * @tc.desc: Test parse Ebpf data with normal data
 * @tc.type: FUNC
 */
HWTEST_F(EbpfParserTest, EbpfDataEbpfHeadWithNormalData, TestSize.Level1)
{
    TS_LOGI("test29-3");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = 0;
    ebpfHeader.header.headSize = EbpfDataHeader::EBPF_DATA_HEADER_SIZE;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    parser->Finish();
    EXPECT_EQ(parser->reader_->ebpfDataHeader_->header.clock, EBPF_CLOCK_BOOTTIME);
    EXPECT_EQ(parser->reader_->ebpfDataHeader_->header.cmdLineLen, 0);
    EXPECT_EQ(parser->reader_->ebpfDataHeader_->header.headSize, EbpfDataHeader::EBPF_DATA_HEADER_SIZE);
}

/**
 * @tc.name: EbpfDataWithOnlyEbpfHeadNoCommandLine
 * @tc.desc: Test parse Ebpf data with only ebpf head but no command line
 * @tc.type: FUNC
 */
HWTEST_F(EbpfParserTest, EbpfDataWithOnlyEbpfHeadNoCommandLine, TestSize.Level1)
{
    TS_LOGI("test29-4");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = 0;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
}

/**
 * @tc.name: EbpfDataEbpfHeadHasProcessName
 * @tc.desc: Test parse Ebpf data with only ebpf head
 * @tc.type: FUNC
 */
HWTEST_F(EbpfParserTest, EbpfDataEbpfHeadHasProcessName, TestSize.Level1)
{
    TS_LOGI("test29-5");
    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;
    ebpfHeader.header.cmdLineLen = COMMAND_LINE.length();
    strncpy_s(ebpfHeader.cmdline, EbpfDataHeader::EBPF_COMMAND_MAX_SIZE, COMMAND_LINE.c_str(),
              EbpfDataHeader::EBPF_COMMAND_MAX_SIZE);

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));
    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.cmdLineLen == COMMAND_LINE.length());
    EXPECT_STREQ(parser->reader_->ebpfDataHeader_->cmdline, COMMAND_LINE.c_str());
}

} // namespace TraceStreamer
} // namespace SysTuning
