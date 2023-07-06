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

#include "htrace_hilog_parser.h"
#include "hilog_plugin_result.pb.h"
#include "hilog_plugin_result.pbreader.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HilogParserTest : public ::testing::Test {
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
 * @tc.name: ParseHilogInfoWithoutHilogLine
 * @tc.desc: Parse a HilogInfo that does not contain any hiloglines
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoWithoutHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-1");
    HilogInfo* hilogInfo = new HilogInfo();
    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);
    auto size = stream_.traceDataCache_->GetConstHilogData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHilogInfoWithOneHilogLine
 * @tc.desc: Parse a HilogInfo with only one Hilogline
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoWithOneHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-2");
    const uint64_t TV_SEC = 1632675525;
    const uint64_t TV_NSEC = 996560700;
    const std::string LOG_TAG = "HwMSDPMovementService";
    const std::string LOG_CONTEXT = "handleGetSupportedModule";
    const uint32_t LOG_LEVEL_D = 68;
    const uint32_t PID = 2716;
    const uint32_t TID = 1532;
    const uint64_t LOG_ID = 1;

    HilogDetails* hilogDetails = new HilogDetails();
    hilogDetails->set_tv_sec(TV_SEC);
    hilogDetails->set_tv_nsec(TV_NSEC);
    hilogDetails->set_pid(PID);
    hilogDetails->set_tid(TID);
    hilogDetails->set_level(LOG_LEVEL_D);
    hilogDetails->set_tag(LOG_TAG);

    HilogInfo* hilogInfo = new HilogInfo();
    auto hilogLine = hilogInfo->add_info();
    hilogLine->set_allocated_detail(hilogDetails);
    hilogLine->set_context(LOG_CONTEXT);
    hilogLine->set_id(LOG_ID);

    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);

    auto seq = stream_.traceDataCache_->GetConstHilogData().HilogLineSeqs()[0];
    EXPECT_EQ(seq, LOG_ID);

    auto timeStamp = stream_.traceDataCache_->GetConstHilogData().TimeStampData()[0];
    EXPECT_EQ(timeStamp, (TV_NSEC + TV_SEC * SEC_TO_NS));

    auto pid = stream_.traceDataCache_->GetConstHilogData().Pids()[0];
    EXPECT_EQ(pid, PID);

    auto tid = stream_.traceDataCache_->GetConstHilogData().Tids()[0];
    EXPECT_EQ(tid, TID);

    auto level = stream_.traceDataCache_->GetConstHilogData().Levels()[0];
    auto iter = htraceHiLogParser.logLevelString_.find(LOG_LEVEL_D);
    if (iter == htraceHiLogParser.logLevelString_.end()) {
        EXPECT_FALSE(0);
    }
    DataIndex levelDIndex = stream_.traceDataCache_->dataDict_.GetStringIndex(iter->second.c_str());
    EXPECT_EQ(level, levelDIndex);

    auto readTagIndex = stream_.traceDataCache_->GetConstHilogData().Tags()[0];
    DataIndex writeTagIndex = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_TAG);
    EXPECT_EQ(readTagIndex, writeTagIndex);

    auto readContextIndex = stream_.traceDataCache_->GetConstHilogData().Contexts()[0];
    DataIndex writeContextIndex = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_CONTEXT);
    EXPECT_EQ(readContextIndex, writeContextIndex);

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseHilogInfoWithMultipleHilogLine
 * @tc.desc: Parse a HilogInfo with multiple Hiloglines
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoWithMultipleHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-3");
    const uint64_t TV_SEC_01 = 1632675525;
    const uint64_t TV_NSEC_01 = 996560700;
    const uint32_t PID_01 = 2716;
    const uint32_t TID_01 = 1532;
    const uint32_t LOG_LEVEL_D = 68;
    const std::string LOG_TAG_01 = "HwMSDPMovementService";
    const std::string LOG_CONTEXT_01 = "handleGetSupportedModule";
    const uint64_t LOG_ID_01 = 1;

    HilogDetails* hilogDetailsFirst = new HilogDetails();
    hilogDetailsFirst->set_tv_sec(TV_SEC_01);
    hilogDetailsFirst->set_tv_nsec(TV_NSEC_01);
    hilogDetailsFirst->set_pid(PID_01);
    hilogDetailsFirst->set_tid(TID_01);
    hilogDetailsFirst->set_level(LOG_LEVEL_D);
    hilogDetailsFirst->set_tag(LOG_TAG_01);

    const uint64_t TV_SEC_02 = 1632688888;
    const uint64_t TV_NSEC_02 = 996588888;
    const uint32_t PID_02 = 2532;
    const uint32_t TID_02 = 1716;
    const uint32_t LOG_LEVEL_E = 69;
    const std::string LOG_TAG_02 = "ProfilerService";
    const std::string LOG_CONTEXT_02 = "POST_RECV_MESSAGE method: /IProfilerService/CreateSession";
    const uint64_t LOG_ID_02 = 2;

    HilogDetails* hilogDetailsSecond = new HilogDetails();
    hilogDetailsSecond->set_tv_sec(TV_SEC_02);
    hilogDetailsSecond->set_tv_nsec(TV_NSEC_02);
    hilogDetailsSecond->set_pid(PID_02);
    hilogDetailsSecond->set_tid(TID_02);
    hilogDetailsSecond->set_level(LOG_LEVEL_E);
    hilogDetailsSecond->set_tag(LOG_TAG_02);

    HilogInfo* hilogInfo = new HilogInfo();
    auto hilogLineFirst = hilogInfo->add_info();
    hilogLineFirst->set_allocated_detail(hilogDetailsFirst);
    hilogLineFirst->set_context(LOG_CONTEXT_01);
    hilogLineFirst->set_id(LOG_ID_01);

    auto hilogLineSecond = hilogInfo->add_info();
    hilogLineSecond->set_allocated_detail(hilogDetailsSecond);
    hilogLineSecond->set_context(LOG_CONTEXT_02);
    hilogLineSecond->set_id(LOG_ID_02);

    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);

    auto seqFirst = stream_.traceDataCache_->GetConstHilogData().HilogLineSeqs()[0];
    auto seqSecond = stream_.traceDataCache_->GetConstHilogData().HilogLineSeqs()[1];
    EXPECT_EQ(seqFirst, LOG_ID_01);
    EXPECT_EQ(seqSecond, LOG_ID_02);

    auto timestampFirst = stream_.traceDataCache_->GetConstHilogData().TimeStampData()[0];
    auto timestampSecond = stream_.traceDataCache_->GetConstHilogData().TimeStampData()[1];
    EXPECT_EQ(timestampFirst, (TV_NSEC_01 + TV_SEC_01 * SEC_TO_NS));
    EXPECT_EQ(timestampSecond, (TV_NSEC_02 + TV_SEC_02 * SEC_TO_NS));

    auto pidFirst = stream_.traceDataCache_->GetConstHilogData().Pids()[0];
    auto pidSecond = stream_.traceDataCache_->GetConstHilogData().Pids()[1];
    EXPECT_EQ(pidFirst, PID_01);
    EXPECT_EQ(pidSecond, PID_02);

    auto tidFirst = stream_.traceDataCache_->GetConstHilogData().Tids()[0];
    auto tidSecond = stream_.traceDataCache_->GetConstHilogData().Tids()[1];
    EXPECT_EQ(tidFirst, TID_01);
    EXPECT_EQ(tidSecond, TID_02);

    auto levelFirst = stream_.traceDataCache_->GetConstHilogData().Levels()[0];
    auto iterFirst = htraceHiLogParser.logLevelString_.find(LOG_LEVEL_D);
    if (iterFirst == htraceHiLogParser.logLevelString_.end()) {
        EXPECT_FALSE(0);
    }
    DataIndex levelDIndex = stream_.traceDataCache_->dataDict_.GetStringIndex(iterFirst->second.c_str());
    EXPECT_EQ(levelFirst, levelDIndex);

    auto levelSecond = stream_.traceDataCache_->GetConstHilogData().Levels()[1];
    auto iterSecond = htraceHiLogParser.logLevelString_.find(LOG_LEVEL_E);
    if (iterSecond == htraceHiLogParser.logLevelString_.end()) {
        EXPECT_FALSE(0);
    }
    DataIndex levelEIndex = stream_.traceDataCache_->dataDict_.GetStringIndex(iterSecond->second.c_str());
    EXPECT_EQ(levelSecond, levelEIndex);

    auto readTagIndexFirst = stream_.traceDataCache_->GetConstHilogData().Tags()[0];
    auto readTagIndexSecond = stream_.traceDataCache_->GetConstHilogData().Tags()[1];
    DataIndex writeTagIndexFirst = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_TAG_01);
    DataIndex writeTagIndexSecond = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_TAG_02);
    EXPECT_EQ(readTagIndexFirst, writeTagIndexFirst);
    EXPECT_EQ(readTagIndexSecond, writeTagIndexSecond);

    auto readContextIndexFirst = stream_.traceDataCache_->GetConstHilogData().Contexts()[0];
    auto readContextIndexSecond = stream_.traceDataCache_->GetConstHilogData().Contexts()[1];
    DataIndex writeContextIndexFirst = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_CONTEXT_01);
    DataIndex writeContextIndexSecond = stream_.traceDataCache_->dataDict_.GetStringIndex(LOG_CONTEXT_02);
    EXPECT_EQ(readContextIndexFirst, writeContextIndexFirst);
    EXPECT_EQ(readContextIndexSecond, writeContextIndexSecond);

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseHilogInfoWithErrLevelHilogLine
 * @tc.desc: Parse a HilogInfo with error level Hiloglines
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoWithErrLevelHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-4");
    const uint64_t TV_SEC = 1632675525;
    const uint64_t TV_NSEC = 996560700;
    const std::string LOG_TAG = "HwMSDPMovementService";
    const std::string LOG_CONTEXT = "handleGetSupportedModule";
    const uint32_t LOG_LEVEL_ILLEGAL = 0;
    const uint32_t PID = 2716;
    const uint32_t TID = 1532;
    const uint64_t LOG_ID = 1;

    HilogDetails* hilogDetails = new HilogDetails();
    hilogDetails->set_tv_sec(TV_SEC);
    hilogDetails->set_tv_nsec(TV_NSEC);
    hilogDetails->set_pid(PID);
    hilogDetails->set_tid(TID);
    hilogDetails->set_level(LOG_LEVEL_ILLEGAL);
    hilogDetails->set_tag(LOG_TAG);

    HilogInfo* hilogInfo = new HilogInfo();
    auto hilogLine = hilogInfo->add_info();
    hilogLine->set_allocated_detail(hilogDetails);
    hilogLine->set_context(LOG_CONTEXT);
    hilogLine->set_id(LOG_ID);

    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_DATA_INVALID);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseHilogInfoLostHilogLine
 * @tc.desc: Parse a HilogInfo that lost a Hiloglines
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoLostHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-5");
    const uint64_t TV_SEC = 1632675525;
    const uint64_t TV_NSEC = 996560700;
    const std::string LOG_TAG = "HwMSDPMovementService";
    const std::string LOG_CONTEXT = "handleGetSupportedModule";
    const uint32_t LOG_LEVEL_D = 68;
    const uint32_t PID = 2716;
    const uint32_t TID = 1532;
    const uint64_t LOG_ID = 2;

    HilogDetails* hilogDetails = new HilogDetails();
    hilogDetails->set_tv_sec(TV_SEC);
    hilogDetails->set_tv_nsec(TV_NSEC);
    hilogDetails->set_pid(PID);
    hilogDetails->set_tid(TID);
    hilogDetails->set_level(LOG_LEVEL_D);
    hilogDetails->set_tag(LOG_TAG);

    HilogInfo* hilogInfo = new HilogInfo();
    auto hilogLine = hilogInfo->add_info();
    hilogLine->set_allocated_detail(hilogDetails);
    hilogLine->set_context(LOG_CONTEXT);
    hilogLine->set_id(LOG_ID);

    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_DATA_LOST);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseHilogInfoHasDuplicateHilogLine
 * @tc.desc: Parse a HilogInfo has duplicate HilogLine
 * @tc.type: FUNC
 */
HWTEST_F(HilogParserTest, ParseHilogInfoHasDuplicateHilogLine, TestSize.Level1)
{
    TS_LOGI("test8-6");
    const uint64_t TV_SEC = 1632675525;
    const uint64_t TV_NSEC = 996560700;
    const std::string LOG_TAG = "HwMSDPMovementService";
    const std::string LOG_CONTEXT = "handleGetSupportedModule";
    const uint32_t LOG_LEVEL_D = 68;
    const uint32_t PID = 2716;
    const uint32_t TID = 1532;
    const uint64_t LOG_ID = 1;

    HilogDetails* hilogDetails = new HilogDetails();
    hilogDetails->set_tv_sec(TV_SEC);
    hilogDetails->set_tv_nsec(TV_NSEC);
    hilogDetails->set_pid(PID);
    hilogDetails->set_tid(TID);
    hilogDetails->set_level(LOG_LEVEL_D);
    hilogDetails->set_tag(LOG_TAG);

    HilogInfo* hilogInfo = new HilogInfo();
    auto hilogLineFirst = hilogInfo->add_info();
    hilogLineFirst->set_allocated_detail(hilogDetails);
    hilogLineFirst->set_context(LOG_CONTEXT);
    hilogLineFirst->set_id(LOG_ID);
    auto hilogLineSecond = hilogInfo->add_info();
    hilogLineSecond->set_allocated_detail(hilogDetails);
    hilogLineSecond->set_context(LOG_CONTEXT);
    hilogLineSecond->set_id(LOG_ID);

    HtraceHiLogParser htraceHiLogParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hilogData = "";
    hilogInfo->SerializeToString(&hilogData);
    ProtoReader::BytesView hilogInfoData(reinterpret_cast<const uint8_t*>(hilogData.data()), hilogData.size());
    htraceHiLogParser.Parse(hilogInfoData);

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_HILOG, STAT_EVENT_NOTMATCH);
    EXPECT_TRUE(1 == eventCount);
}
} // namespace TraceStreamer
} // namespace SysTuning