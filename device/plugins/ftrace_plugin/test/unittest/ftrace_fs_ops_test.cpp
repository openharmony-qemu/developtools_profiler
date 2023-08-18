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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "file_utils.h"
#include "flow_controller.h"
#include "ftrace_fs_ops.h"

namespace {
using FTRACE_NS::FlowController;
using FTRACE_NS::FtraceFsOps;
using testing::ext::TestSize;

constexpr uint32_t BUFFER_SIZE_KB = 256;
constexpr uint32_t FLUSH_INTERVAL_MS = 100;
constexpr uint32_t FLUSH_THRESHOLD_KB = 1024;
constexpr uint32_t TRACE_PERIOD_MS = 500;
using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
using ConstVoidPtr = std::unique_ptr<const void>::pointer;

long WriteFunc(WriterStructPtr writer, ConstVoidPtr data, size_t size);
bool FlushFunc(WriterStructPtr writer);

class FtraceFsOpsTest : public ::testing::Test {
protected:
    static void SetUpTestCase()
    {
        // start tracing sched_switch event
        FlowController controller;
        TracePluginConfig config;

        // set writer
        WriterStruct writer = {WriteFunc, FlushFunc};
        controller.SetWriter(static_cast<WriterStructPtr>(&writer));

        // set config
        config.add_ftrace_events("sched/sched_switch");
        config.set_buffer_size_kb(BUFFER_SIZE_KB);
        config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
        config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
        config.set_parse_ksyms(true);
        config.set_clock("global");
        config.set_trace_period_ms(TRACE_PERIOD_MS);
        config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
        std::vector<uint8_t> configData(config.ByteSizeLong());
        config.SerializeToArray(configData.data(), configData.size());
        controller.LoadConfig(configData.data(), configData.size());
        controller.StartCapture();
        sleep(1);
        controller.StopCapture();
    }

    void SetUp() override {}
    void TearDown() override {}
};

long WriteFunc(WriterStructPtr writer, ConstVoidPtr data, size_t size)
{
    if (writer == nullptr || data == nullptr || size <= 0) {
        return -1;
    }

    return 0;
}

bool FlushFunc(WriterStructPtr writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

/*
 * @tc.name: GetFtraceRoot
 * @tc.desc: test FtraceFsOps::GetFtraceRoot with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetFtraceRoot, TestSize.Level1)
{
    std::string path = FtraceFsOps::GetInstance().GetFtraceRoot();
    ASSERT_STRNE(path.c_str(), "");
    EXPECT_TRUE((strcmp(path.c_str(), "/sys/kernel/tracing") || strcmp(path.c_str(), "/sys/kernel/debug/tracing")));
}

/*
 * @tc.name: GetKernelSymbols
 * @tc.desc: test FtraceFsOps::GetKernelSymbols with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetKernelSymbols, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetKernelSymbols();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetPrintkFormatsNormal
 * @tc.desc: test FtraceFsOps::GetPrintkFormats with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPrintkFormatsNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetPrintkFormats();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetPrintkFormatsFalse
 * @tc.desc: test FtraceFsOps::GetPrintkFormats with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPrintkFormatsFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    std::string content = ftraceFsOps.GetPrintkFormats();
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetPrintkFormats();
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetProcessCommNormal
 * @tc.desc: test FtraceFsOps::GetProcessComm with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetProcessCommNormal, TestSize.Level1)
{
    int32_t pid = getpid();
    std::string content = FtraceFsOps::GetInstance().GetProcessComm(pid);
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetProcessCommFalse
 * @tc.desc: test FtraceFsOps::GetProcessComm with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetProcessCommFalse, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetProcessComm(-1);
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetThreadCommNormal
 * @tc.desc: test FtraceFsOps::GetThreadComm with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetThreadCommNormal, TestSize.Level1)
{
    int32_t pid = getpid();
    std::string content = FtraceFsOps::GetInstance().GetThreadComm(pid, pid);
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetThreadCommFalse
 * @tc.desc: test FtraceFsOps::GetThreadComm with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetThreadCommFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    std::string content = ftraceFsOps.GetThreadComm(-1, -1);
    EXPECT_STREQ(content.c_str(), "");

    int32_t pid = getpid();
    content = ftraceFsOps.GetThreadComm(pid, -1);
    EXPECT_STREQ(content.c_str(), "");

    content = ftraceFsOps.GetThreadComm(-1, pid);
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetSavedCmdLinesNormal
 * @tc.desc: test FtraceFsOps::GetSavedCmdLines with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetSavedCmdLinesNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetSavedCmdLines();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetSavedCmdLinesFalse
 * @tc.desc: test FtraceFsOps::GetSavedCmdLines with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetSavedCmdLinesFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    std::string content = ftraceFsOps.GetSavedCmdLines();
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetSavedCmdLines();
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetSavedTgidsNormal
 * @tc.desc: test FtraceFsOps::GetSavedTgids with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetSavedTgidsNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetSavedTgids();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetSavedTgidsFalse
 * @tc.desc: test FtraceFsOps::GetSavedTgids with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetSavedTgidsFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    std::string content = ftraceFsOps.GetSavedTgids();
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetSavedTgids();
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetPerCpuStatsNormal
 * @tc.desc: test FtraceFsOps::GetPerCpuStats with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPerCpuStatsNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetPerCpuStats(0);
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetPerCpuStatsFalse
 * @tc.desc: test FtraceFsOps::GetPerCpuStats with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPerCpuStatsFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    std::string content = ftraceFsOps.GetPerCpuStats(-1);
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("");
    content = ftraceFsOps.GetPerCpuStats(0);
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetPerCpuStats(0);
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetRawTracePath
 * @tc.desc: test FtraceFsOps::GetRawTracePath with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetRawTracePath, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetRawTracePath(0);
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetPageHeaderFormatNormal
 * @tc.desc: test FtraceFsOps::GetPageHeaderFormat with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPageHeaderFormatNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetPageHeaderFormat();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetPageHeaderFormatFalse
 * @tc.desc: test FtraceFsOps::GetPageHeaderFormat with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPageHeaderFormatFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    std::string content = ftraceFsOps.GetPageHeaderFormat();
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetPageHeaderFormat();
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetEventDataFormatNormal
 * @tc.desc: test FtraceFsOps::GetEventDataFormat with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetEventDataFormatNormal, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetEventDataFormat("irq", "softirq_entry");
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: GetEventDataFormatFalse
 * @tc.desc: test FtraceFsOps::GetEventDataFormat with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetEventDataFormatFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    std::string content = ftraceFsOps.GetEventDataFormat("test_type", "test_name");
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("");
    content = ftraceFsOps.GetEventDataFormat("irq", "softirq_entry");
    EXPECT_STREQ(content.c_str(), "");

    ftraceFsOps.SetFtraceRoot("/test_path");
    content = ftraceFsOps.GetEventDataFormat("irq", "softirq_entry");
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: GetPlatformEventsNormal
 * @tc.desc: test FtraceFsOps::GetPlatformEvents with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPlatformEventsNormal, TestSize.Level1)
{
    std::vector<std::pair<std::string, std::string>> event = FtraceFsOps::GetInstance().GetPlatformEvents();
    EXPECT_GT(event.size(), static_cast<size_t>(0));
}

/*
 * @tc.name: GetPlatformEventsFalse
 * @tc.desc: test FtraceFsOps::GetPlatformEvents with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetPlatformEventsFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    std::vector<std::pair<std::string, std::string>> event = ftraceFsOps.GetPlatformEvents();
    EXPECT_EQ(event.size(), static_cast<size_t>(0));

    ftraceFsOps.SetFtraceRoot("/test_path");
    event = ftraceFsOps.GetPlatformEvents();
    EXPECT_EQ(event.size(), static_cast<size_t>(0));
}

/*
 * @tc.name: ClearTraceBufferNormal
 * @tc.desc: test FtraceFsOps::ClearTraceBuffer with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, ClearTraceBufferNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().ClearTraceBuffer());
}

/*
 * @tc.name: ClearTraceBufferFalse
 * @tc.desc: test FtraceFsOps::ClearTraceBuffer with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, ClearTraceBufferFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.ClearTraceBuffer());

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.ClearTraceBuffer());
}

/*
 * @tc.name: SetRecordCmdOptionNormal
 * @tc.desc: test FtraceFsOps::SetRecordCmdOption with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetRecordCmdOptionNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().SetRecordCmdOption(true));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/options/record-cmd";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "1\n");

    EXPECT_TRUE(FtraceFsOps::GetInstance().SetRecordCmdOption(false));
    content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "0\n");
}

/*
 * @tc.name: SetRecordCmdOptionFalse
 * @tc.desc: test FtraceFsOps::SetRecordCmdOption with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetRecordCmdOptionFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.SetRecordCmdOption(true));
    EXPECT_FALSE(ftraceFsOps.SetRecordCmdOption(false));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.SetRecordCmdOption(true));
    EXPECT_FALSE(ftraceFsOps.SetRecordCmdOption(false));
}

/*
 * @tc.name: SetRecordTgidOptionNormal
 * @tc.desc: test FtraceFsOps::SetRecordTgidOption with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetRecordTgidOptionNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().SetRecordTgidOption(true));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/options/record-tgid";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "1\n");

    EXPECT_TRUE(FtraceFsOps::GetInstance().SetRecordTgidOption(false));
    content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "0\n");
}

/*
 * @tc.name: SetRecordTgidOptionFalse
 * @tc.desc: test FtraceFsOps::SetRecordTgidOption with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetRecordTgidOptionFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.SetRecordTgidOption(true));
    EXPECT_FALSE(ftraceFsOps.SetRecordTgidOption(false));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.SetRecordTgidOption(true));
    EXPECT_FALSE(ftraceFsOps.SetRecordTgidOption(false));
}

/*
 * @tc.name: SetBufferSizeKbNormal
 * @tc.desc: test FtraceFsOps::SetBufferSizeKb with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetBufferSizeKbNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().SetBufferSizeKb(1024));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/buffer_size_kb";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "1024\n");
}

/*
 * @tc.name: SetBufferSizeKbFalse
 * @tc.desc: test FtraceFsOps::SetBufferSizeKb with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetBufferSizeKbFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.SetBufferSizeKb(1024));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.SetBufferSizeKb(1024));
}

/*
 * @tc.name: SetTraceClockNormal
 * @tc.desc: test FtraceFsOps::SetTraceClock with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetTraceClockNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().SetTraceClock("uptime"));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/trace_clock";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: SetTraceClockFalse
 * @tc.desc: test FtraceFsOps::SetTraceClock with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, SetTraceClockFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.SetTraceClock("uptime"));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.SetTraceClock("uptime"));
}

/*
 * @tc.name: GetTraceClock
 * @tc.desc: test FtraceFsOps::GetTraceClock with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, GetTraceClock, TestSize.Level1)
{
    std::string content = FtraceFsOps::GetInstance().GetTraceClock();
    EXPECT_STRNE(content.c_str(), "");
}

/*
 * @tc.name: AppendSetEventNormal
 * @tc.desc: test FtraceFsOps::AppendSetEvent with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, AppendSetEventNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().ClearSetEvent());
    EXPECT_TRUE(FtraceFsOps::GetInstance().AppendSetEvent("sched", "sched_switch"));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/set_event";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "sched:sched_switch\n");
}

/*
 * @tc.name: AppendSetEventFalse
 * @tc.desc: test FtraceFsOps::AppendSetEvent with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, AppendSetEventFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.AppendSetEvent("sched", "sched_switch"));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.AppendSetEvent("sched", "sched_switch"));
}

/*
 * @tc.name: EnableEventNormal
 * @tc.desc: test FtraceFsOps::EnableEvent with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, EnableEventNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().EnableEvent("sched", "sched_switch"));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/events/sched/sched_switch/enable";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "1\n");
}

/*
 * @tc.name: EnableEventFalse
 * @tc.desc: test FtraceFsOps::EnableEvent with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, EnableEventFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.EnableEvent("sched", "sched_switch"));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.EnableEvent("sched", "sched_switch"));
}

/*
 * @tc.name: DisableEventNormal
 * @tc.desc: test FtraceFsOps::DisableEvent with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, DisableEventNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().DisableEvent("sched", "sched_switch"));
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/events/sched/sched_switch/enable";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "0\n");
}

/*
 * @tc.name: DisableEventFalse
 * @tc.desc: test FtraceFsOps::DisableEvent with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, DisableEventFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.DisableEvent("sched", "sched_switch"));

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.DisableEvent("sched", "sched_switch"));
}

/*
 * @tc.name: EnableTracingNormal
 * @tc.desc: test FtraceFsOps::EnableTracing with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, EnableTracingNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().EnableTracing());
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "/tracing_on";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "1\n");
}

/*
 * @tc.name: EnableTracingFalse
 * @tc.desc: test FtraceFsOps::EnableTracing with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, EnableTracingFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.EnableTracing());

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.EnableTracing());
}

/*
 * @tc.name: DisableTracingNormal
 * @tc.desc: test FtraceFsOps::DisableTracing with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, DisableTracingNormal, TestSize.Level1)
{
    EXPECT_TRUE(FtraceFsOps::GetInstance().DisableTracing());
    std::string path = FtraceFsOps::GetInstance().ftraceRoot_ + "tracing_on";
    std::string content = FileUtils::ReadFile(path);
    EXPECT_STREQ(content.c_str(), "");
}

/*
 * @tc.name: DisableTracingFalse
 * @tc.desc: test FtraceFsOps::DisableTracing with false case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFsOpsTest, DisableTracingFalse, TestSize.Level1)
{
    FtraceFsOps ftraceFsOps;
    ftraceFsOps.SetFtraceRoot("");
    EXPECT_FALSE(ftraceFsOps.DisableTracing());

    ftraceFsOps.SetFtraceRoot("/test_path");
    EXPECT_FALSE(ftraceFsOps.DisableTracing());
}
} // namespace