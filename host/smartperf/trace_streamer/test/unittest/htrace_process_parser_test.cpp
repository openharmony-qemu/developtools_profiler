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

#include "htrace_process_parser.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "process_plugin_result.pb.h"
#include "process_plugin_result.pbreader.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HtraceProcessParserTest : public ::testing::Test {
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
 * @tc.name: ParseHtraceProcessWithoutProcessData
 * @tc.desc: Parse a Process that does not contain any ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceProcessParserTest, ParseHtraceProcessWithoutProcessData, TestSize.Level1)
{
    TS_LOGI("test18-1");
    uint64_t ts = 100;
    auto processData = std::make_unique<ProcessData>();
    std::string processStrMsg = "";
    processData->SerializeToString(&processStrMsg);
    ProtoReader::BytesView processBytesView(reinterpret_cast<const uint8_t*>(processStrMsg.data()),
                                            processStrMsg.size());
    HtraceProcessParser htraceProcessParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceProcessParser.Parse(processBytesView, ts);
    auto size = stream_.traceDataCache_->GetConstLiveProcessData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceProcessWithProcessData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceProcessParserTest, ParseHtraceProcessWithProcessData, TestSize.Level1)
{
    TS_LOGI("test18-2");
    uint64_t ts = 100;
    const uint32_t PID = 312;
    const string NAME = "resource_schedu";
    const int32_t PPID = 22;
    const int32_t UID = 23;

    auto processData = std::make_unique<ProcessData>();
    ProcessInfo* processInfo = processData->add_processesinfo();
    processInfo->set_pid(PID);
    processInfo->set_name(NAME);
    processInfo->set_ppid(PPID);
    processInfo->set_uid(UID);

    std::string processStrMsg = "";
    processData->SerializeToString(&processStrMsg);
    ProtoReader::BytesView processBytesView(reinterpret_cast<const uint8_t*>(processStrMsg.data()),
                                            processStrMsg.size());
    HtraceProcessParser htraceProcessParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceProcessParser.Parse(processBytesView, ts);
    htraceProcessParser.Finish();

    auto size = stream_.traceDataCache_->GetConstLiveProcessData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceProcessWithTwoProcessData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceProcessParserTest, ParseHtraceProcessWithTwoProcessData, TestSize.Level1)
{
    TS_LOGI("test18-3");
    uint64_t ts = 100;
    const uint32_t PID_01 = 311;
    const string NAME_01 = "resource_schedu01";
    const int32_t PPID_01 = 21;
    const int32_t UID_01 = 1;

    const uint32_t PID_02 = 312;
    const string NAME_02 = "resource_schedu02";
    const int32_t PPID_02 = 22;
    const int32_t UID_02 = 2;

    auto processData = std::make_unique<ProcessData>();
    ProcessInfo* processInfoFirst = processData->add_processesinfo();
    processInfoFirst->set_pid(PID_01);
    processInfoFirst->set_name(NAME_01);
    processInfoFirst->set_ppid(PPID_01);
    processInfoFirst->set_uid(UID_01);

    ProcessInfo* processInfoSecond = processData->add_processesinfo();
    processInfoSecond->set_pid(PID_02);
    processInfoSecond->set_name(NAME_02);
    processInfoSecond->set_ppid(PPID_02);
    processInfoSecond->set_uid(UID_02);

    std::string processStrMsg = "";
    processData->SerializeToString(&processStrMsg);
    ProtoReader::BytesView processBytesView(reinterpret_cast<const uint8_t*>(processStrMsg.data()),
                                            processStrMsg.size());
    HtraceProcessParser htraceProcessParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceProcessParser.Parse(processBytesView, ts);
    htraceProcessParser.Finish();

    auto size = stream_.traceDataCache_->GetConstLiveProcessData().Size();
    EXPECT_EQ(1, size);

    auto pidFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[0];
    EXPECT_EQ(pidFirst, PID_02);
    auto processNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[0];
    EXPECT_EQ(processNameFirst, NAME_02);
    auto parentProcessIDFirst = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[0];
    EXPECT_EQ(parentProcessIDFirst, PPID_02);
    auto uidFirst = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[0];
    EXPECT_EQ(uidFirst, UID_02);
    auto userNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[0];
    EXPECT_EQ(userNameFirst, std::to_string(UID_02));
}

/**
 * @tc.name: ParseHtraceProcessWithThreeProcessData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceProcessParserTest, ParseHtraceProcessWithThreeProcessData, TestSize.Level1)
{
    TS_LOGI("test18-4");
    uint64_t ts = 100;
    const uint32_t PID_01 = 311;
    const string NAME_01 = "resource_schedu01";
    const int32_t PPID_01 = 21;
    const int32_t UID_01 = 1;

    const uint32_t PID_02 = 312;
    const string NAME_02 = "resource_schedu02";
    const int32_t PPID_02 = 22;
    const int32_t UID_02 = 2;

    const uint32_t PID_03 = 313;
    const string NAME_03 = "resource_schedu03";
    const int32_t PPID_03 = 23;
    const int32_t UID_03 = 3;

    auto processData = std::make_unique<ProcessData>();
    ProcessInfo* processInfoFirst = processData->add_processesinfo();
    processInfoFirst->set_pid(PID_01);
    processInfoFirst->set_name(NAME_01);
    processInfoFirst->set_ppid(PPID_01);
    processInfoFirst->set_uid(UID_01);

    ProcessInfo* processInfoSecond = processData->add_processesinfo();
    processInfoSecond->set_pid(PID_02);
    processInfoSecond->set_name(NAME_02);
    processInfoSecond->set_ppid(PPID_02);
    processInfoSecond->set_uid(UID_02);

    ProcessInfo* processInfoThird = processData->add_processesinfo();
    processInfoThird->set_pid(PID_03);
    processInfoThird->set_name(NAME_03);
    processInfoThird->set_ppid(PPID_03);
    processInfoThird->set_uid(UID_03);

    std::string processStrMsg = "";
    processData->SerializeToString(&processStrMsg);
    ProtoReader::BytesView processBytesView(reinterpret_cast<const uint8_t*>(processStrMsg.data()),
                                            processStrMsg.size());
    HtraceProcessParser htraceProcessParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceProcessParser.Parse(processBytesView, ts);
    htraceProcessParser.Finish();

    auto pidFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[0];
    auto pidSecond = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[1];
    EXPECT_EQ(pidFirst, PID_02);
    EXPECT_EQ(pidSecond, PID_03);
    auto processNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[0];
    auto processNameSecond = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[1];
    EXPECT_EQ(processNameFirst, NAME_02);
    EXPECT_EQ(processNameSecond, NAME_03);
    auto parentProcessIDFirst = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[0];
    auto parentProcessIDSecond = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[1];
    EXPECT_EQ(parentProcessIDFirst, PPID_02);
    EXPECT_EQ(parentProcessIDSecond, PPID_03);
    auto uidFirst = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[0];
    auto uidSecond = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[1];
    EXPECT_EQ(uidFirst, UID_02);
    EXPECT_EQ(uidSecond, UID_03);
    auto userNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[0];
    auto userNameSecond = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[1];
    EXPECT_EQ(userNameFirst, std::to_string(UID_02));
    EXPECT_EQ(userNameSecond, std::to_string(UID_03));
}

/**
 * @tc.name: ParseHtraceProcessWithMultipleProcessData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceProcessParserTest, ParseHtraceProcessWithMultipleProcessData, TestSize.Level1)
{
    TS_LOGI("test18-5");
    uint64_t ts = 100;
    const uint32_t PID_01 = 311;
    const string NAME_01 = "resource_schedu01";
    const int32_t PPID_01 = 21;
    const int32_t UID_01 = 1;

    const uint32_t PID_02 = 312;
    const string NAME_02 = "resource_schedu02";
    const int32_t PPID_02 = 22;
    const int32_t UID_02 = 2;

    const uint32_t PID_03 = 313;
    const string NAME_03 = "resource_schedu03";
    const int32_t PPID_03 = 23;
    const int32_t UID_03 = 3;

    const uint32_t PID_04 = 313;
    const string NAME_04 = "resource_schedu03";
    const int32_t PPID_04 = 23;
    const int32_t UID_04 = 3;

    auto processData = std::make_unique<ProcessData>();
    ProcessInfo* processInfoFirst = processData->add_processesinfo();
    processInfoFirst->set_pid(PID_01);
    processInfoFirst->set_name(NAME_01);
    processInfoFirst->set_ppid(PPID_01);
    processInfoFirst->set_uid(UID_01);

    ProcessInfo* processInfoSecond = processData->add_processesinfo();
    processInfoSecond->set_pid(PID_02);
    processInfoSecond->set_name(NAME_02);
    processInfoSecond->set_ppid(PPID_02);
    processInfoSecond->set_uid(UID_02);

    ProcessInfo* processInfoThird = processData->add_processesinfo();
    processInfoThird->set_pid(PID_03);
    processInfoThird->set_name(NAME_03);
    processInfoThird->set_ppid(PPID_03);
    processInfoThird->set_uid(UID_03);

    ProcessInfo* processInfoFour = processData->add_processesinfo();
    processInfoFour->set_pid(PID_04);
    processInfoFour->set_name(NAME_04);
    processInfoFour->set_ppid(PPID_04);
    processInfoFour->set_uid(UID_04);

    std::string processStrMsg = "";
    processData->SerializeToString(&processStrMsg);
    ProtoReader::BytesView processBytesView(reinterpret_cast<const uint8_t*>(processStrMsg.data()),
                                            processStrMsg.size());
    HtraceProcessParser htraceProcessParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceProcessParser.Parse(processBytesView, ts);
    htraceProcessParser.Finish();

    auto pidFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[0];
    auto pidSecond = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[1];
    auto pidThird = stream_.traceDataCache_->GetConstLiveProcessData().ProcessID()[2];
    EXPECT_EQ(pidFirst, PID_02);
    EXPECT_EQ(pidSecond, PID_03);
    EXPECT_EQ(pidThird, PID_04);
    auto processNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[0];
    auto processNameSecond = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[1];
    auto processNameThird = stream_.traceDataCache_->GetConstLiveProcessData().ProcessName()[2];
    EXPECT_EQ(processNameFirst, NAME_02);
    EXPECT_EQ(processNameSecond, NAME_03);
    EXPECT_EQ(processNameThird, NAME_04);
    auto parentProcessIDFirst = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[0];
    auto parentProcessIDSecond = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[1];
    auto parentProcessIDThird = stream_.traceDataCache_->GetConstLiveProcessData().ParentProcessID()[2];
    EXPECT_EQ(parentProcessIDFirst, PPID_02);
    EXPECT_EQ(parentProcessIDSecond, PPID_03);
    EXPECT_EQ(parentProcessIDThird, PPID_04);
    auto uidFirst = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[0];
    auto uidSecond = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[1];
    auto uidThird = stream_.traceDataCache_->GetConstLiveProcessData().Uid()[2];
    EXPECT_EQ(uidFirst, UID_02);
    EXPECT_EQ(uidSecond, UID_03);
    EXPECT_EQ(uidThird, UID_04);
    auto userNameFirst = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[0];
    auto userNameSecond = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[1];
    auto userNameThird = stream_.traceDataCache_->GetConstLiveProcessData().UserName()[2];
    EXPECT_EQ(userNameFirst, std::to_string(UID_02));
    EXPECT_EQ(userNameSecond, std::to_string(UID_03));
    EXPECT_EQ(userNameThird, std::to_string(UID_04));
}
} // namespace TraceStreamer
} // namespace SysTuning