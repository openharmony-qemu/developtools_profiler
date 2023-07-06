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

#include "htrace_cpu_detail_parser.h"
#include "htrace_event_parser.h"
#include "irq_filter.h"
#include "trace_plugin_result.pb.h"
#include "trace_plugin_result.pbreader.h"
#include "trace_streamer_selector.h"
#include "ts_common.h"
#include "types/plugins/ftrace_data/trace_plugin_result.pb.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class HtraceIrqEventTest : public ::testing::Test {
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
 * @tc.name: IrqHandlerEntryTest
 * @tc.desc: Binary formate IrqHandlerEntry Normal TEST
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, IrqHandlerEntryTest, TestSize.Level1)
{
    TS_LOGI("test15-1");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    int32_t irq = 12;
    IrqHandlerEntryFormat* irqHandlerEvent = new IrqHandlerEntryFormat();
    irqHandlerEvent->set_irq(irq);
    irqHandlerEvent->set_name("user_irq");
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_irq_handler_entry_format(irqHandlerEvent);

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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    eventParser.Clear();
}

/**
 * @tc.name: IrqHandlerEntryTestNotMatch
 * @tc.desc: Binary formate IrqHandlerEntry, only start, no end
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, IrqHandlerEntryTestNotMatch, TestSize.Level1)
{
    TS_LOGI("test15-2");
    int64_t ts1 = 120;
    uint32_t cpu1 = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    int32_t irq = 12;
    IrqHandlerEntryFormat* irqHandlerEvent = new IrqHandlerEntryFormat();
    irqHandlerEvent->set_irq(irq);
    irqHandlerEvent->set_name("user_irq");
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_irq_handler_entry_format(irqHandlerEvent);

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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    ts1 = 110;
    IrqHandlerEntryFormat* irqHandlerEvent2 = new IrqHandlerEntryFormat();
    irqHandlerEvent2->set_irq(irq);
    irqHandlerEvent2->set_name("user_irq");
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_irq_handler_entry_format(irqHandlerEvent2);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 2);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_DATA_LOST);
    EXPECT_TRUE(1 == eventCount);
    eventParser.Clear();
}

/**
 * @tc.name: IrqHandlerExitTestEmpty
 * @tc.desc: Binary formate IrqHandlerExit, Interrupt only ends, not starts
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, IrqHandlerExitTestEmpty, TestSize.Level1)
{
    TS_LOGI("test15-3");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t ret = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    int32_t irq = 12; // 1 for handled, else for unhandled

    IrqHandlerExitFormat* irqHandlerExitEvent = new IrqHandlerExitFormat();
    irqHandlerExitEvent->set_irq(irq);
    irqHandlerExitEvent->set_ret(ret);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_irq_handler_exit_format(irqHandlerExitEvent);

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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 0);
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_NOTMATCH);
    EXPECT_TRUE(1 == eventCount);
    eventParser.Clear();
}

/**
 * @tc.name: IrqHandlerEnterAndExitTest
 * @tc.desc: Binary formate IrqHandlerEnter, Interrupt normal start and end
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, IrqHandlerEnterAndExitTest, TestSize.Level1)
{
    TS_LOGI("test15-4");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    int32_t irq = 12;

    IrqHandlerEntryFormat* irqHandlerEvent = new IrqHandlerEntryFormat();
    irqHandlerEvent->set_irq(irq);
    irqHandlerEvent->set_name("user_irq");
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_irq_handler_entry_format(irqHandlerEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    uint32_t ret = 1; // 1 for handled, else for unhandled

    IrqHandlerExitFormat* irqHandlerExitEvent = new IrqHandlerExitFormat();
    irqHandlerExitEvent->set_irq(irq);
    irqHandlerExitEvent->set_ret(ret);
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_irq_handler_exit_format(irqHandlerExitEvent);

    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();

    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().ArgSetIdsData()[0] == 0);
    eventParser.Clear();
}

/**
 * @tc.name: IrqHandlerEnterAndExitTestTwice
 * @tc.desc: Binary formate IrqHandlerEnter and Exit, Interrupt normal start and end Twice
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, IrqHandlerEnterAndExitTestTwice, TestSize.Level1)
{
    TS_LOGI("test15-5");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    int32_t irq = 12;

    IrqHandlerEntryFormat* irqHandlerEvent = new IrqHandlerEntryFormat();
    irqHandlerEvent->set_irq(irq);
    irqHandlerEvent->set_name("user_irq");
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_irq_handler_entry_format(irqHandlerEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    uint32_t ret = 1; // 1 for handled, else for unhandled
    cpu1 = 2;
    ts1 = 150;

    IrqHandlerExitFormat* irqHandlerExitEvent = new IrqHandlerExitFormat();
    irqHandlerExitEvent->set_irq(irq);
    irqHandlerExitEvent->set_ret(ret);
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_irq_handler_exit_format(irqHandlerExitEvent);

    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_EXIT,
                                                                        STAT_EVENT_NOTMATCH) == 1);
    cpu1 = 1;
    ts1 = 200;

    IrqHandlerExitFormat* irqHandlerExitEvent2 = new IrqHandlerExitFormat();
    irqHandlerExitEvent2->set_irq(irq);
    irqHandlerExitEvent2->set_ret(ret);
    TracePluginResult tracePacket3;
    FtraceCpuDetailMsg* ftraceCpuDetail3 = tracePacket3.add_ftrace_cpu_detail();
    ftraceCpuDetail3->set_cpu(cpu1);
    ftraceCpuDetail3->set_overwrite(0);
    auto ftraceEvent3 = ftraceCpuDetail3->add_event();

    ftraceEvent3->set_timestamp(ts1);
    ftraceEvent3->set_tgid(tid1);
    ftraceEvent3->set_comm(appName);
    ftraceEvent3->set_allocated_irq_handler_exit_format(irqHandlerExitEvent2);

    HtraceDataSegment dataSeg3;
    dataSeg3.clockId = TS_CLOCK_BOOTTIME;
    tracePacket3.SerializeToString(&cpuDetailStrMsg);
    dataSeg3.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView3(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg3.protoData = cpuDetailBytesView3;
    eventParser.ParseDataItem(dataSeg3, dataSeg3.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().ArgSetIdsData()[0] == 0);
    eventParser.Clear();
}

/**
 * @tc.name: SoftIrqEntryTest
 * @tc.desc: Binary format Soft interrupt normal test
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, SoftIrqEntryTest, TestSize.Level1)
{
    TS_LOGI("test15-6");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;

    SoftirqEntryFormat* softirqEntryEvent = new SoftirqEntryFormat();
    softirqEntryEvent->set_vec(vec);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_softirq_entry_format(softirqEntryEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    eventParser.Clear();
}

/**
 * @tc.name: SoftIrqEntryNotMatch
 * @tc.desc: The binary format soft interrupts do not match. The two interrupts have only the beginning and no end
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, SoftIrqEntryNotMatch, TestSize.Level1)
{
    TS_LOGI("test15-7");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;

    SoftirqEntryFormat* softirqEntryEvent = new SoftirqEntryFormat();
    softirqEntryEvent->set_vec(vec);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_softirq_entry_format(softirqEntryEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    ts1 = 150;

    SoftirqEntryFormat* softirqEntryEvent2 = new SoftirqEntryFormat();
    softirqEntryEvent2->set_vec(vec);
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_softirq_entry_format(softirqEntryEvent2);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 2);
    EXPECT_TRUE(
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_DATA_LOST) == 1);
    eventParser.Clear();
}

/**
 * @tc.name: SoftIrqExitEmptyTest
 * @tc.desc: The binary format soft interrupt only ends without starting
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, SoftIrqExitEmptyTest, TestSize.Level1)
{
    TS_LOGI("test15-8");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;

    SoftirqExitFormat* softirqExitEvent = new SoftirqExitFormat();
    softirqExitEvent->set_vec(vec);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_softirq_exit_format(softirqExitEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 0);
    EXPECT_TRUE(
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
    eventParser.Clear();
}

/**
 * @tc.name: SoftIrqTest
 * @tc.desc: The binary format soft interrupt normal test
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, SoftIrqTest, TestSize.Level1)
{
    TS_LOGI("test15-9");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;

    SoftirqEntryFormat* softirqEntryEvent = new SoftirqEntryFormat();
    softirqEntryEvent->set_vec(vec);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_softirq_entry_format(softirqEntryEvent);
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

    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    eventParser.Clear();
    ts1 = 150;
    SoftirqExitFormat* softirqExitEvent = new SoftirqExitFormat();
    softirqExitEvent->set_vec(vec);
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_softirq_exit_format(softirqExitEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    eventParser.Clear();
}

/**
 * @tc.name: SoftIrqTestNotMatch
 * @tc.desc: The binary soft interrupt test not match
 * @tc.type: FUNC
 */
HWTEST_F(HtraceIrqEventTest, SoftIrqTestNotMatch, TestSize.Level1)
{
    TS_LOGI("test15-10");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    std::string appName = "app1";
    uint32_t tid1 = 1;

    SoftirqEntryFormat* softirqEntryEvent = new SoftirqEntryFormat();
    softirqEntryEvent->set_vec(vec);
    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(cpu1);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_softirq_entry_format(softirqEntryEvent);
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
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    eventParser.Clear();
    ts1 = 150;
    cpu1 = 2;

    SoftirqExitFormat* softirqExitEvent = new SoftirqExitFormat();
    softirqExitEvent->set_vec(vec);
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(cpu1);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_softirq_exit_format(softirqExitEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstIrqData().Size() == 1);
    EXPECT_TRUE(
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
    eventParser.Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
