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
#include <unordered_map>

#include "cpu_filter.h"
#include "htrace_cpu_detail_parser.h"
#include "parser/common_types.h"
#include "src/filter/symbols_filter.h"
#include "trace_plugin_result.pb.h"
#include "trace_plugin_result.pbreader.h"
#include "trace_streamer_filters.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
const uint64_t TIMESTAMP = 1616439852302;
const std::string THREAD_NAME_01 = "ACCS0";
const std::string THREAD_NAME_02 = "HeapTaskDaemon";
const uint32_t PRIORITY_01 = 120;
const uint32_t PRIORITY_02 = 124;
const uint32_t PID_01 = 2716;
const uint32_t PID_02 = 2532;
class HtraceEventParserTest : public ::testing::Test {
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
 * @tc.name: ParseSchedSwitchEvent
 * @tc.desc: Parse a schedswitch event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseSchedSwitchEvent, TestSize.Level1)
{
    TS_LOGI("test14-1");
    SchedSwitchFormat* event = new SchedSwitchFormat();
    event->set_prev_prio(PRIORITY_01);
    event->set_next_prio(PRIORITY_02);
    event->set_prev_pid(PID_01);
    event->set_next_pid(PID_02);
    event->set_prev_comm(THREAD_NAME_01);
    event->set_next_comm(THREAD_NAME_02);
    event->set_prev_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->unsafe_arena_set_allocated_sched_switch_format(event);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(1);
    auto realTimeStamp = stream_.traceDataCache_->GetConstSchedSliceData().TimeStampData()[0];
    EXPECT_TRUE(TIMESTAMP == realTimeStamp);
    auto realCpu = stream_.traceDataCache_->GetConstSchedSliceData().CpusData()[0];
    EXPECT_TRUE(0 == realCpu);
}

/**
 * @tc.name: ParseFtraceCpuDetailMsgHasNoEvent
 * @tc.desc: FtraceCpuDetailMsg has no ftrace event
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseFtraceCpuDetailMsgHasNoEvent, TestSize.Level1)
{
    TS_LOGI("test14-2");
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();

    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_OTHER, STAT_EVENT_DATA_LOST);
    EXPECT_TRUE(0 == eventCount);
}

/**
 * @tc.name: ParseFtraceCpuDetailMsgOverwriteTrue
 * @tc.desc: FtraceCpuDetailMsg overwrit is true
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseFtraceCpuDetailMsgOverwriteTrue, TestSize.Level1)
{
    TS_LOGI("test14-3");
    SchedSwitchFormat* event = new SchedSwitchFormat();
    event->set_prev_prio(PRIORITY_01);
    event->set_next_prio(PRIORITY_02);
    event->set_prev_pid(PID_01);
    event->set_next_pid(PID_02);
    event->set_prev_comm(THREAD_NAME_01);
    event->set_next_comm(THREAD_NAME_02);
    event->set_prev_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(1);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_sched_switch_format(event);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_OTHER, STAT_EVENT_DATA_LOST);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseTaskRenameEvent
 * @tc.desc: Parse a task_rename event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseTaskRenameEvent, TestSize.Level1)
{
    TS_LOGI("test14-4");
    TaskRenameFormat* taskRenameEvent = new TaskRenameFormat();
    taskRenameEvent->set_pid(PID_01);
    taskRenameEvent->set_oldcomm(THREAD_NAME_01);
    taskRenameEvent->set_newcomm(THREAD_NAME_02);
    taskRenameEvent->set_oom_score_adj(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_task_rename_format(taskRenameEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_TASK_RENAME, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseTaskNewtaskEvent
 * @tc.desc: Parse a task_newtask event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseTaskNewtaskEvent, TestSize.Level1)
{
    TS_LOGI("test14-5");
    TaskNewtaskFormat* newTaskEvent = new TaskNewtaskFormat();
    newTaskEvent->set_pid(PID_01);
    newTaskEvent->set_comm(THREAD_NAME_01);
    newTaskEvent->set_clone_flags(0);
    newTaskEvent->set_oom_score_adj(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_task_newtask_format(newTaskEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseSchedWakeupEvent
 * @tc.desc: Parse a sched_wakeup event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseSchedWakeupEvent, TestSize.Level1)
{
    TS_LOGI("test14-6");
    SchedWakeupFormat* wakeupEvent = new SchedWakeupFormat();
    wakeupEvent->set_comm(THREAD_NAME_01);
    wakeupEvent->set_pid(PRIORITY_02);
    wakeupEvent->set_prio(PID_01);
    wakeupEvent->set_target_cpu(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_sched_wakeup_format(wakeupEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseSchedWakingEvent
 * @tc.desc: Parse a sched_waking event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseSchedWakingEvent, TestSize.Level1)
{
    TS_LOGI("test14-7");
    SchedWakingFormat* wakingEvent = new SchedWakingFormat();
    wakingEvent->set_comm(THREAD_NAME_01);
    wakingEvent->set_pid(PRIORITY_02);
    wakingEvent->set_prio(PID_01);
    wakingEvent->set_target_cpu(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_sched_waking_format(wakingEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseCpuIdleEvent
 * @tc.desc: Parse a cpuIdle event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseCpuIdleEvent, TestSize.Level1)
{
    TS_LOGI("test14-8");
    CpuIdleFormat* cpuIdleEvent = new CpuIdleFormat();
    cpuIdleEvent->set_cpu_id(0);
    cpuIdleEvent->set_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_cpu_idle_format(cpuIdleEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CPU_IDLE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseCpuFrequencyEvent
 * @tc.desc: Parse a CpuFrequency event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseCpuFrequencyEvent, TestSize.Level1)
{
    TS_LOGI("test14-9");
    CpuFrequencyFormat* cpuFrequencyEvent = new CpuFrequencyFormat();
    cpuFrequencyEvent->set_cpu_id(0);
    cpuFrequencyEvent->set_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_cpu_frequency_format(cpuFrequencyEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseWorkqueueExecuteStartEvent
 * @tc.desc: Parse a WorkqueueExecuteStart event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseWorkqueueExecuteStartEvent, TestSize.Level1)
{
    TS_LOGI("test14-10");
    stream_.streamFilters_->symbolsFilter_->RegisterFunc(1, 1);
    auto funcNum = stream_.streamFilters_->symbolsFilter_->GetFunc(1);
    WorkqueueExecuteStartFormat* workqueueExecuteStartEvent = new WorkqueueExecuteStartFormat();
    workqueueExecuteStartEvent->set_work(0);
    workqueueExecuteStartEvent->set_function(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_workqueue_execute_start_format(workqueueExecuteStartEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_WORKQUEUE_EXECUTE_START,
                                                                              STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseWorkqueueExecuteEndEvent
 * @tc.desc: Parse a WorkqueueExecuteEnd event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseWorkqueueExecuteEndEvent, TestSize.Level1)
{
    TS_LOGI("test14-11");
    WorkqueueExecuteEndFormat* workqueueExecuteEndEvent = new WorkqueueExecuteEndFormat();
    workqueueExecuteEndEvent->set_work(0);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(1);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_workqueue_execute_end_format(workqueueExecuteEndEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClockDisableEvent
 * @tc.desc: Parse a clock_Disable event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClockDisableEvent, TestSize.Level1)
{
    TS_LOGI("test14-12");
    ClockDisableFormat* clockDisableEvent = new ClockDisableFormat();
    clockDisableEvent->set_name(THREAD_NAME_02);
    clockDisableEvent->set_cpu_id(0);
    clockDisableEvent->set_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clock_disable_format(clockDisableEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLOCK_DISABLE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClockEnableEvent
 * @tc.desc: Parse a clock_Enable event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClockEnableEvent, TestSize.Level1)
{
    TS_LOGI("test14-13");
    ClockEnableFormat* clockEnableEvent = new ClockEnableFormat();
    clockEnableEvent->set_name(THREAD_NAME_02);
    clockEnableEvent->set_cpu_id(0);
    clockEnableEvent->set_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clock_enable_format(clockEnableEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLOCK_ENABLE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClockSetRateEvent
 * @tc.desc: Parse a clock_set_rate event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClockSetRateEvent, TestSize.Level1)
{
    TS_LOGI("test14-14");
    ClockSetRateFormat* clockSetRateEvent = new ClockSetRateFormat();
    clockSetRateEvent->set_name(THREAD_NAME_02);
    clockSetRateEvent->set_cpu_id(0);
    clockSetRateEvent->set_state(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clock_set_rate_format(clockSetRateEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLOCK_SET_RATE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClkDisableEvent
 * @tc.desc: Parse a clk_Disable event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClkDisableEvent, TestSize.Level1)
{
    TS_LOGI("test14-15");
    ClkDisableFormat* clkDisableEvent = new ClkDisableFormat();
    clkDisableEvent->set_name(THREAD_NAME_02);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clk_disable_format(clkDisableEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLK_DISABLE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClkEnableEvent
 * @tc.desc: Parse a clk_Enable event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClkEnableEvent, TestSize.Level1)
{
    TS_LOGI("test14-16");
    ClkEnableFormat* clkEnableEvent = new ClkEnableFormat();
    clkEnableEvent->set_name(THREAD_NAME_02);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clk_enable_format(clkEnableEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLK_ENABLE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseClkSetRateEvent
 * @tc.desc: Parse a clk_set_rate event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseClkSetRateEvent, TestSize.Level1)
{
    TS_LOGI("test14-17");
    ClkSetRateFormat* clkSetRateEvent = new ClkSetRateFormat();
    clkSetRateEvent->set_name(THREAD_NAME_02);
    clkSetRateEvent->set_rate(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_clk_set_rate_format(clkSetRateEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_CLK_SET_RATE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseSysEnterEvent
 * @tc.desc: Parse a sysEnter event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseSysEnterEvent, TestSize.Level1)
{
    TS_LOGI("test14-18");
    SysEnterFormat* sysEnterEvent = new SysEnterFormat();
    sysEnterEvent->set_id(1);
    sysEnterEvent->set_args(THREAD_NAME_02);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_sys_enter_format(sysEnterEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SYS_ENTRY, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}
/**
 * @tc.name: ParseSystemExitEvent
 * @tc.desc: Parse a system_exit event in htrace format
 * @tc.type: FUNC
 */
HWTEST_F(HtraceEventParserTest, ParseSystemExitEvent, TestSize.Level1)
{
    TS_LOGI("test14-19");
    SysExitFormat* sysExitEvent = new SysExitFormat();
    sysExitEvent->set_id(1);
    sysExitEvent->set_ret(1);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(TIMESTAMP);
    ftraceEvent->set_tgid(2);
    ftraceEvent->set_comm(THREAD_NAME_02);
    ftraceEvent->set_allocated_sys_exit_format(sysExitEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SYS_EXIT, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}
} // namespace TraceStreamer
} // namespace SysTuning
