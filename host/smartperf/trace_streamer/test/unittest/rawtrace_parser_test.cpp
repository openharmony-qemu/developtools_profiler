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

#include <fstream>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>
#include <string>

#include "file.h"
#include "parser/rawtrace_parser/rawtrace_parser.h"
#include "parser/common_types.h"
#include "string_help.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::base;

namespace SysTuning {
namespace TraceStreamer {
class RawTraceParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        selector_.InitFilter();
        selector_.EnableMetaTable(false);
        if (access(rawTraceBinPath_.c_str(), F_OK) == 0) {
            binFs_.open(rawTraceBinPath_, std::ios::binary | std::ios::in);
        }
        if (access(rawTraceDataPath_.c_str(), F_OK) == 0) {
            dataFs_.open(rawTraceDataPath_, std::ios::binary | std::ios::in);
        }
        parser_ = std::make_unique<RawTraceParser>(selector_.traceDataCache_.get(), selector_.streamFilters_.get());
    }

    void TearDown()
    {
        binFs_.close();
        dataFs_.close();
    }

    std::string ChunkToString(std::ifstream& fs)
    {
        std::string line_;
        std::stringstream ss;
        while (std::getline(fs, line_)) {
            if (StartWith(line_, chunkEndCmd_)) {
                break;
            }
            ss << line_ << '\n';
        }
        return ss.str();
    }

public:
    SysTuning::TraceStreamer::TraceStreamerSelector selector_ = {};
    std::unique_ptr<RawTraceParser> parser_;
    const std::string fileHeaderCmd_ = "file_header:";
    const std::string headPageFormatsCmd_ = "headPageFormats:";
    const std::string printkFormatsCmd_ = "printkFormats:";
    const std::string kAllSymsCmd_ = "kAllSyms:";
    const std::string cmdlinesCmd_ = "cmdlines:";
    const std::string tgidsCmd_ = "tgids:";
    const std::string cpuDataCmd_ = "cpuData:";
    const std::string chunkEndCmd_ = "-end-";
    const std::string rawTraceBinPath_ = "../../test/resource/rawtrace.bin";
    const std::string rawTraceDataPath_ = "../../test/resource/rawtrace.data";
    std::string line_;
    std::string chunckStr_;
    const size_t bufferSize_ = 1024 * 1024;
    std::ifstream binFs_;
    std::ifstream dataFs_;
};

/**
 * @tc.name: ParseAllData
 * @tc.desc: Test ParseTraceDataSegment interface Parse all data
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, ParseAllData, TestSize.Level1)
{
    TS_LOGI("test38-1");
    EXPECT_TRUE(binFs_.is_open() && parser_ != nullptr);
    while (true) {
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(bufferSize_);
        binFs_.read((char*)buf.get(), bufferSize_);
        auto readSize = binFs_.gcount();
        if (readSize == 0) {
            break;
        }
        if (readSize < 0) {
            TS_LOGD("Reading trace file failed (errno: %d, %s)", errno, strerror(errno));
            break;
        }
        if (!selector_.ParseTraceDataSegment(std::move(buf), readSize, false, false)) {
            break;
        };
    }
    selector_.WaitForParserEnd();
}
/**
 * @tc.name: ParseFileHeader
 * @tc.desc: Test ParseTraceDataSegment interface Parse file header
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, ParseFileHeader, TestSize.Level1)
{
    TS_LOGI("test38-2");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, fileHeaderCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            auto& packagesBuffer = parser_->packagesBuffer_;
            auto packagesCurIter = packagesBuffer.begin();
            EXPECT_FALSE(parser_->InitRawTraceFileHeader(packagesCurIter));
            const uint8_t* data = reinterpret_cast<const uint8_t*>(chunckStr_.c_str());
            packagesBuffer.insert(packagesBuffer.end(), data, data + chunckStr_.size());
            EXPECT_TRUE(parser_->InitRawTraceFileHeader(packagesCurIter));
            break;
        }
    }
}
/**
 * @tc.name: HandleHeadPage
 * @tc.desc: Test ParseTraceDataSegment interface Handle head page format
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, HandleHeadPage, TestSize.Level1)
{
    TS_LOGI("test38-3");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    // empty parse
    EXPECT_FALSE(parser_->ftraceProcessor_->HandleHeaderPageFormat(chunckStr_));
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, headPageFormatsCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ftraceProcessor_->HandleHeaderPageFormat(chunckStr_));
            break;
        }
    }
    std::string noCommitField = R"(field: u64 timestamp;	offset:0;	size:8;	signed:0;
	field: int overwrite;	offset:8;	size:1;	signed:1;
	field: char data;	offset:16;	size:4080;	signed:0;)";
    EXPECT_FALSE(parser_->ftraceProcessor_->HandleHeaderPageFormat(noCommitField));
}
/**
 * @tc.name: HandlePrintkFormats
 * @tc.desc: Test ParseTraceDataSegment interface Handle printk formats
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, HandlePrintkFormats, TestSize.Level1)
{
    TS_LOGI("test38-4");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, printkFormatsCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(PrintkFormatsProcessor::GetInstance().HandlePrintkSyms(chunckStr_));
            break;
        }
    }
    PrintkFormatsProcessor::GetInstance().Clear();
    std::string errPrintkFormats = R"(ffffffc010001578 T __entry_text_start
    1409 HitraceDumpTest
    250 239
    )";
    EXPECT_FALSE(PrintkFormatsProcessor::GetInstance().HandlePrintkSyms(errPrintkFormats));
}
/**
 * @tc.name: HandleKallSyms
 * @tc.desc: Test ParseTraceDataSegment interface Handle kAllSyms formats
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, HandleKallSyms, TestSize.Level1)
{
    TS_LOGI("test38-5");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    // empty parse
    EXPECT_FALSE(parser_->ksymsProcessor_->HandleKallSyms(chunckStr_));
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, kAllSymsCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ksymsProcessor_->HandleKallSyms(chunckStr_));
            break;
        }
    }
}
/**
 * @tc.name: HandleCmdlines
 * @tc.desc: Test ParseTraceDataSegment interface Handle cmdlines
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, HandleCmdlines, TestSize.Level1)
{
    TS_LOGI("test38-6");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    // empty parse
    EXPECT_FALSE(parser_->ftraceProcessor_->HandleCmdlines(chunckStr_));
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, cmdlinesCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ftraceProcessor_->HandleCmdlines(chunckStr_));
            break;
        }
    }
    std::string errCmdlineFormats = "162,kworker/u8:2\n1409:HitraceDumpTest\n250-239";
    EXPECT_FALSE(parser_->ftraceProcessor_->HandleCmdlines(errCmdlineFormats));
}
/**
 * @tc.name: HandleTgids
 * @tc.desc: Test ParseTraceDataSegment interface Handle tgids
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, HandleTgids, TestSize.Level1)
{
    TS_LOGI("test38-7");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    // empty parse
    EXPECT_FALSE(parser_->ftraceProcessor_->HandleTgids(chunckStr_));
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, tgidsCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ftraceProcessor_->HandleTgids(chunckStr_));
            break;
        }
    }
}
/**
 * @tc.name: ParseCpuData
 * @tc.desc: Test ParseTraceDataSegment interface Parse cpuData
 * @tc.type: FUNC
 */
HWTEST_F(RawTraceParserTest, ParseCpuData, TestSize.Level1)
{
    TS_LOGI("test38-8");
    EXPECT_TRUE(dataFs_.is_open() && parser_ != nullptr);
    auto cpuId = 2;
    EXPECT_FALSE(parser_->ParseCpuRawData(cpuId, chunckStr_));
    while (std::getline(dataFs_, line_)) {
        if (StartWith(line_, cpuDataCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ParseCpuRawData(cpuId, chunckStr_));
        } else if (StartWith(line_, headPageFormatsCmd_)) {
            chunckStr_ = ChunkToString(dataFs_);
            EXPECT_TRUE(parser_->ftraceProcessor_->HandleHeaderPageFormat(chunckStr_));
        }
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
