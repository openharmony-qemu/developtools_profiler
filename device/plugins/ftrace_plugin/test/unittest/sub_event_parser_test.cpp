/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include <chrono>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>
#include <unistd.h>

#include "file_utils.h"
#include "ftrace_fs_ops.h"
#include "ftrace_parser.h"
#include "securec.h"
#include "sub_event_parser.h"

using FTRACE_NS::EventFormat;
using FTRACE_NS::FtraceFsOps;
using FTRACE_NS::FtraceParser;
using FTRACE_NS::SubEventParser;
using testing::ext::TestSize;

namespace {
#ifndef PAGE_SIZE
constexpr uint32_t PAGE_SIZE = 4096;
#endif
constexpr auto TEST_DELAY = std::chrono::milliseconds(10);
constexpr uint32_t SCHED_SWITCH_EVENT_ID = 189;
const std::string SCHED_SWITCH_FORMAT_DESC = R"(
name: sched_switch
ID: 189
format:
        field:unsigned short common_type;       offset:0;       size:2; signed:0;
        field:unsigned char common_flags;       offset:2;       size:1; signed:0;
        field:unsigned char common_preempt_count;       offset:3;       size:1; signed:0;
        field:int common_pid;   offset:4;       size:4; signed:1;

        field:char prev_comm[16];       offset:8;       size:16;        signed:0;
        field:pid_t prev_pid;   offset:24;      size:4; signed:1;
        field:int prev_prio;    offset:28;      size:4; signed:1;
        field:long prev_state;  offset:32;      size:4; signed:1;
        field:char next_comm[16];       offset:36;      size:16;        signed:0;
        field:pid_t next_pid;   offset:52;      size:4; signed:1;
        field:int next_prio;    offset:56;      size:4; signed:1;

print fmt: "prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d", ...
)";

class SubEventParserTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        SubEventParser::GetInstance().idToFunctions_.clear();
    }

    void TearDown() override
    {
        SubEventParser::GetInstance().idToFunctions_.clear();
    }
};

/*
 * @tc.name: IsSupportName
 * @tc.desc: test SubEventParser::IsSupport with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SubEventParserTest, IsSupportName, TestSize.Level1)
{
    EXPECT_FALSE(SubEventParser::GetInstance().IsSupport("xxx"));
    EXPECT_TRUE(SubEventParser::GetInstance().IsSupport("sched_switch"));
}

/*
 * @tc.name: SetupEvent
 * @tc.desc: test SubEventParser::SetupEvent with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SubEventParserTest, SetupEvent, TestSize.Level1)
{
    EventFormat format;
    FtraceParser ftraceParser;
    format.eventType = "sched";
    format.eventName = "sched_switch";
    EXPECT_TRUE(ftraceParser.ParseEventFormat(SCHED_SWITCH_FORMAT_DESC, format));

    EXPECT_TRUE(nullptr == SubEventParser::GetInstance().GetParseEventCtx(format.eventId));
    EXPECT_TRUE(SubEventParser::GetInstance().SetupEvent(format));
    EXPECT_TRUE(nullptr != SubEventParser::GetInstance().GetParseEventCtx(format.eventId));
}

/*
 * @tc.name: IsSupportId
 * @tc.desc: test SubEventParser::IsSupport with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SubEventParserTest, IsSupportId, TestSize.Level1)
{
    EventFormat format;
    FtraceParser ftraceParser;
    format.eventType = "sched";
    format.eventName = "sched_switch";
    EXPECT_TRUE(ftraceParser.ParseEventFormat(SCHED_SWITCH_FORMAT_DESC, format));
    SubEventParser::GetInstance().schedSwitchCtx = nullptr;
    EXPECT_TRUE(nullptr == SubEventParser::GetInstance().GetParseEventCtx(format.eventId));
    EXPECT_TRUE(SubEventParser::GetInstance().SetupEvent(format));
    EXPECT_TRUE(nullptr != SubEventParser::GetInstance().GetParseEventCtx(format.eventId));
}

/*
 * @tc.name: ParseEvent
 * @tc.desc: test SubEventParser::ParseEvent with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SubEventParserTest, ParseEvent, TestSize.Level1)
{
    EventFormat format;
    FtraceParser ftraceParser;
    format.eventType = "sched";
    format.eventName = "sched_switch";
    EXPECT_TRUE(ftraceParser.ParseEventFormat(SCHED_SWITCH_FORMAT_DESC, format));
    EXPECT_TRUE(SubEventParser::GetInstance().SetupEvent(format));
    EXPECT_EQ(format.eventId, SCHED_SWITCH_EVENT_ID);

    std::vector<uint8_t> buffer(PAGE_SIZE, 0);
    std::vector<uint8_t> zeros(PAGE_SIZE, 0);
    std::string traceRaw = FtraceFsOps::GetInstance().GetRawTracePath(0);
    int fd = open(traceRaw.c_str(), O_EXCL);
    EXPECT_NE(fd, -1);

    EXPECT_TRUE(FtraceFsOps::GetInstance().ClearTraceBuffer());
    EXPECT_TRUE(FtraceFsOps::GetInstance().EnableEvent("sched", "sched_switch"));
    EXPECT_TRUE(FtraceFsOps::GetInstance().EnableTracing());

    std::this_thread::sleep_for(TEST_DELAY);
    EXPECT_EQ(read(fd, buffer.data(), buffer.size()), static_cast<int>(PAGE_SIZE));
    EXPECT_TRUE(FtraceFsOps::GetInstance().DisableTracing());
    EXPECT_EQ(close(fd), 0);
    EXPECT_NE(buffer, zeros);

    FtraceEvent event = {};
    SubEventParser::ParseEventCtx* ctx = SubEventParser::GetInstance().GetParseEventCtx(format.eventId);
    EXPECT_TRUE(ctx != nullptr);
    SubEventParser::GetInstance().ParseEvent(event, buffer.data(), buffer.size(), ctx);
}
} // namespace
