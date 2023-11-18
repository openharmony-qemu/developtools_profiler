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
#include <string>

#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "string_help.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::base;

namespace SysTuning {
namespace TraceStreamer {
class BytraceParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
    const std::string dbPath_ = "../../test/resource/out.db";
};

/**
 * @tc.name: ParseNoData
 * @tc.desc: Test ParseTraceDataSegment interface Parse empty memory
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseNoData, TestSize.Level1)
{
    TS_LOGI("test2-1");
    auto buf = std::make_unique<uint8_t[]>(1);
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), 1);
    bytraceParser.WaitForParserEnd();
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 0);
}

/**
 * @tc.name: ParseNoDataWhithLineFlag
 * @tc.desc: Test ParseTraceDataSegment interface Parse "\n"
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseNoDataWhithLineFlag, TestSize.Level1)
{
    TS_LOGI("test2-2");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    auto realBufSize = strlen(" \n");
    if (memcpy_s(buf.get(), bufSize, " \n", realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: ParseInvalidData
 * @tc.desc: Test ParseTraceDataSegment interface Parse invalid string
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseInvalidData, TestSize.Level1)
{
    TS_LOGI("test2-3");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    auto realBufSize = strlen("0123456789\n");
    if (memcpy_s(buf.get(), bufSize, "0123456789\n", realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: ParseComment
 * @tc.desc: Test ParseTraceDataSegment interface Parse Multiline data
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseComment, TestSize.Level1)
{
    TS_LOGI("test2-4");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    auto realBufSize = strlen("TRACE: \n# tracer: nop \n# \n");
    if (memcpy_s(buf.get(), bufSize, "TRACE: \n# tracer: nop \n# \n", realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 2);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: ParseInvalidLines
 * @tc.desc: Test ParseTraceDataSegment interface Parse Multiline Invalid data
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseInvalidLines, TestSize.Level1)
{
    TS_LOGI("test2-5");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    auto realBufSize = strlen(" \nafafda\n");
    if (memcpy_s(buf.get(), bufSize, " \nafafda\n", realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 2);
}

/**
 * @tc.name: ParseNormal
 * @tc.desc: Test ParseTraceDataItem interface Parse normal data
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, ParseNormal, TestSize.Level1)
{
    TS_LOGI("test2-6");
    std::string str(
        "ACCS0-2716  ( 2519) [000] ...1 168758.662861: binder_transaction: \
        transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \n");
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataItem(str);
    bytraceParser.WaitForParserEnd();

    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 1);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 0);
}

/**
 * @tc.name: LineParser_abnormal_pid_err
 * @tc.desc: Test ParseTraceDataItem interface Parse data with error pid
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, LineParser_abnormal_pid_err, TestSize.Level1)
{
    TS_LOGI("test2-7");
    std::string str(
        "ACCS0-27X6  ( 2519) [000] ...1 168758.662861: binder_transaction: \
        transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \n");
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataItem(str);
    bytraceParser.WaitForParserEnd();

    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: LineParserWithInvalidCpu
 * @tc.desc: Test ParseTraceDataItem interface Parse data with invalid cpu
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, LineParserWithInvalidCpu, TestSize.Level1)
{
    TS_LOGI("test2-8");
    std::string str(
        "ACCS0-2716  ( 2519) [00X] ...1 168758.662861: binder_transaction: \
        transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \n");
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataItem(str);
    bytraceParser.WaitForParserEnd();

    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: LineParserWithInvalidTs
 * @tc.desc: Test ParseTraceDataItem interface Parse data with invalid ts
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, LineParserWithInvalidTs, TestSize.Level1)
{
    TS_LOGI("test2-9");
    std::string str(
        "ACCS0-2716  ( 2519) [000] ...1 168758.662X61: binder_transaction: \
        transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \n");
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataItem(str);
    bytraceParser.WaitForParserEnd();

    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

/**
 * @tc.name: NomalHtmlBytraceFile
 * @tc.desc: Test ParseTraceDataItem interface Parse normal html format bytrace data
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, NomalHtmlBytraceFile, TestSize.Level1)
{
    TS_LOGI("test2-10");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    char realBuf[] =
        "<!DOCTYPE html>\r\n<html>\r\n<script class=\"trace-data\" type=\"application/text\">\r\n"
        "ACCS0-2716  ( 2519) [000] ...1 168758.662861: binder_transaction: "
        "transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \r\n"
        "</script>\r\n</html>\n";
    auto realBufSize = sizeof(realBuf);

    if (memcpy_s(buf.get(), bufSize, realBuf, realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 1);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 0);
}

/**
 * @tc.name: MultiScriptHtmlBytraceFile
 * @tc.desc: Test ParseTraceDataItem interface Parse html format bytrace data with multiple script section
 * @tc.type: FUNC
 */
HWTEST_F(BytraceParserTest, MultiScriptHtmlBytraceFile, TestSize.Level1)
{
    TS_LOGI("test2-11");
    constexpr uint32_t bufSize = 1024;
    auto buf = std::make_unique<uint8_t[]>(bufSize);
    char realBuf[] =
        "<!DOCTYPE html>\r\n<html>\r\n<script class=\"trace-data\" type=\"application/text\">\r\n"
        "ACCS0-2716  ( 2519) [000] ...1 168758.662861: binder_transaction: "
        "transaction=25137708 dest_node=4336 dest_proc=924 dest_thread=0 reply=0 flags=0x10 code=0x3 \r\n"
        "</script>\r\n<script class=\"trace-data\" type=\"application/text\">\r\n"
        "{\"traceEvents\": [{\"category\": \"process_argv\", \"name\": \"process_argv\", \"args\": "
        "{\"argv\": [\"c:\\\\platform-tools\\\\systrace\\\\systrace.py\", \"--from-file\", \"d:\\\\trace_output\", "
        "\"-o\", \"output.html\"]}, "
        "\"pid\": 18892, \"ts\": 13988802989.6, \"tid\": 4464, \"ph\": \"M\"}], \"metadata\": {\"clock-domain\": "
        "\"SYSTRACE\"}}  </script>"
        "</html>\n";
    auto realBufSize = sizeof(realBuf);
    if (memcpy_s(buf.get(), bufSize, realBuf, realBufSize)) {
        EXPECT_TRUE(false);
        return;
    }
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    bytraceParser.ParseTraceDataSegment(std::move(buf), realBufSize);
    bytraceParser.WaitForParserEnd();
    stream_.traceDataCache_->ExportDatabase(dbPath_);
    EXPECT_TRUE(access(dbPath_.c_str(), F_OK) == 0);
    EXPECT_TRUE(bytraceParser.TraceCommentLines() == 0);
    EXPECT_TRUE(bytraceParser.ParsedTraceValidLines() == 1);
    EXPECT_TRUE(bytraceParser.ParsedTraceInvalidLines() == 1);
}

} // namespace TraceStreamer
} // namespace SysTuning
