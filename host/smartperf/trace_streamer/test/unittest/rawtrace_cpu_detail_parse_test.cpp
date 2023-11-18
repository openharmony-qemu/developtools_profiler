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

#include "cpu_detail_parser.h"
#include "ftrace_event_processor.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class RawtraceCpuDetailParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        selector_.InitFilter();
        selector_.EnableMetaTable(false);
        cpuDetailParser_ =
            std::make_unique<CpuDetailParser>(selector_.traceDataCache_.get(), selector_.streamFilters_.get());
        ftraceEvent_ = std::make_unique<FtraceEvent>();
    }

    void TearDown() const {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector selector_ = {};
    std::unique_ptr<CpuDetailParser> cpuDetailParser_ = {};
    std::unique_ptr<FtraceEvent> ftraceEvent_ = {};
    std::vector<uint8_t> data_;
    EventFormat format_;
    RawTraceEventInfo eventInfo_;
};

/**
 * @tc.name: ParseSchedSwitchEvent
 * @tc.desc: Test CpuDetailParser: Parse SchedSwitchEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSchedSwitchEvent, TestSize.Level1)
{
    TS_LOGI("test39-1");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SchedSwitch(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SchedSwitchEvent(eventInfo_));
}
/**
 * @tc.name: ParseSchedBlockReasonEvent
 * @tc.desc: Test CpuDetailParser: Parse SchedBlockReasonEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSchedBlockReasonEvent, TestSize.Level1)
{
    TS_LOGI("test39-2");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().SchedBlockedReason(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SchedBlockReasonEvent(eventInfo_));
}
/**
 * @tc.name: ParseSchedWakeupEvent
 * @tc.desc: Test CpuDetailParser: Parse SchedWakeupEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSchedWakeupEvent, TestSize.Level1)
{
    TS_LOGI("test39-3");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SchedWakeup(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SchedWakeupEvent(eventInfo_));
}
/**
 * @tc.name: ParseSchedWakingEvent
 * @tc.desc: Test CpuDetailParser: Parse SchedWakingEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSchedWakingEvent, TestSize.Level1)
{
    TS_LOGI("test39-4");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SchedWaking(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SchedWakingEvent(eventInfo_));
}
/**
 * @tc.name: ParseSchedWakeupNewEvent
 * @tc.desc: Test CpuDetailParser: Parse SchedWakeupNewEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSchedWakeupNewEvent, TestSize.Level1)
{
    TS_LOGI("test39-5");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SchedWakeupNew(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SchedWakeupNewEvent(eventInfo_));
}
/**
 * @tc.name: ParseProcessExitEvent
 * @tc.desc: Test CpuDetailParser: Parse ProcessExitEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseProcessExitEvent, TestSize.Level1)
{
    TS_LOGI("test39-6");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().SchedProcessExit(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->ProcessExitEvent(eventInfo_));
}
/**
 * @tc.name: ParseProcessFreeEvent
 * @tc.desc: Test CpuDetailParser: Parse ProcessFreeEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseProcessFreeEvent, TestSize.Level1)
{
    TS_LOGI("test39-7");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().SchedProcessFree(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->ProcessFreeEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionEvent, TestSize.Level1)
{
    TS_LOGI("test39-8");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().BinderTransaction(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionReceivedEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionReceivedEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionReceivedEvent, TestSize.Level1)
{
    TS_LOGI("test39-9");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().BinderTransactionReceived(*ftraceEvent_, data_.data(), data_.size(),
                                                                              format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionReceivedEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionAllocBufEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionAllocBufEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionAllocBufEvent, TestSize.Level1)
{
    TS_LOGI("test39-10");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().BinderTransactionAllocBuf(*ftraceEvent_, data_.data(), data_.size(),
                                                                              format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionAllocBufEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionLockEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionLockEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionLockEvent, TestSize.Level1)
{
    TS_LOGI("test39-11");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().BinderTransactionAllocLock(*ftraceEvent_, data_.data(),
                                                                               data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionLockEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionLockedEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionLockedEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionLockedEvent, TestSize.Level1)
{
    TS_LOGI("test39-12");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().BinderTransactionAllocLocked(*ftraceEvent_, data_.data(),
                                                                                 data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionLockedEvent(eventInfo_));
}
/**
 * @tc.name: ParseBinderTractionUnLockEvent
 * @tc.desc: Test CpuDetailParser: Parse BinderTractionUnLockEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseBinderTractionUnLockEvent, TestSize.Level1)
{
    TS_LOGI("test39-13");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().BinderTransactionAllocUnlock(*ftraceEvent_, data_.data(),
                                                                                 data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->BinderTractionUnLockEvent(eventInfo_));
}
/**
 * @tc.name: ParseTaskRenameEvent
 * @tc.desc: Test CpuDetailParser: Parse TaskRenameEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseTaskRenameEvent, TestSize.Level1)
{
    TS_LOGI("test39-14");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().TaskRename(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->TaskRenameEvent(eventInfo_));
}
/**
 * @tc.name: ParseTaskNewtaskEvent
 * @tc.desc: Test CpuDetailParser: Parse TaskNewtaskEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseTaskNewtaskEvent, TestSize.Level1)
{
    TS_LOGI("test39-15");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().TaskNewtask(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->TaskNewtaskEvent(eventInfo_));
}
/**
 * @tc.name: ParseTracingMarkWriteOrPrintEvent
 * @tc.desc: Test CpuDetailParser: Parse TracingMarkWriteOrPrintEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseTracingMarkWriteOrPrintEvent, TestSize.Level1)
{
    TS_LOGI("test39-16");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().TracingMarkWriteOrPrintFormat(*ftraceEvent_, data_.data(),
                                                                                  data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->ParseTracingMarkWriteOrPrintEvent(eventInfo_));
}
/**
 * @tc.name: ParseCpuIdleEvent
 * @tc.desc: Test CpuDetailParser: Parse CpuIdleEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseCpuIdleEvent, TestSize.Level1)
{
    TS_LOGI("test39-17");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().CpuIdle(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->CpuIdleEvent(eventInfo_));
}
/**
 * @tc.name: ParseCpuFrequencyEvent
 * @tc.desc: Test CpuDetailParser: Parse CpuFrequencyEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseCpuFrequencyEvent, TestSize.Level1)
{
    TS_LOGI("test39-18");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().CpuFrequency(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->CpuFrequencyEvent(eventInfo_));
}
/**
 * @tc.name: ParseCpuFrequencyLimitsEvent
 * @tc.desc: Test CpuDetailParser: Parse CpuFrequencyLimitsEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseCpuFrequencyLimitsEvent, TestSize.Level1)
{
    TS_LOGI("test39-19");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().CpuFrequencyLimits(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->CpuFrequencyLimitsEvent(eventInfo_));
}
/**
 * @tc.name: ParseSuspendResumeEvent
 * @tc.desc: Test CpuDetailParser: Parse SuspendResumeEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSuspendResumeEvent, TestSize.Level1)
{
    TS_LOGI("test39-20");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SuspendResume(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SuspendResumeEvent(eventInfo_));
}
/**
 * @tc.name: ParseWorkqueueExecuteStartEvent
 * @tc.desc: Test CpuDetailParser: Parse WorkqueueExecuteStartEvents
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseWorkqueueExecuteStartEvent, TestSize.Level1)
{
    TS_LOGI("test39-21");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().WorkqueueExecuteStart(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->WorkqueueExecuteStartEvent(eventInfo_));
}
/**
 * @tc.name: ParseWorkqueueExecuteEndEvent
 * @tc.desc: Test CpuDetailParser: Parse WorkqueueExecuteEndEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseWorkqueueExecuteEndEvent, TestSize.Level1)
{
    TS_LOGI("test39-22");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().WorkqueueExecuteEnd(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->WorkqueueExecuteEndEvent(eventInfo_));
}
/**
 * @tc.name: ParseIrqHandlerEntryEvent
 * @tc.desc: Test CpuDetailParser: Parse IrqHandlerEntryEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseIrqHandlerEntryEvent, TestSize.Level1)
{
    TS_LOGI("test39-23");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().IrqHandlerEntry(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->IrqHandlerEntryEvent(eventInfo_));
}
/**
 * @tc.name: ParseIrqHandlerExitEvent
 * @tc.desc: Test CpuDetailParser: Parse IrqHandlerExitEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseIrqHandlerExitEvent, TestSize.Level1)
{
    TS_LOGI("test39-24");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().IrqHandlerExit(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->IrqHandlerExitEvent(eventInfo_));
}
/**
 * @tc.name: ParseIpiHandlerEntryEvent
 * @tc.desc: Test CpuDetailParser: Parse IpiHandlerEntryEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseIpiHandlerEntryEvent, TestSize.Level1)
{
    TS_LOGI("test39-25");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().IpiEntry(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->IpiHandlerEntryEvent(eventInfo_));
}
/**
 * @tc.name: ParseIpiHandlerExitEvent
 * @tc.desc: Test CpuDetailParser: Parse IpiHandlerExitEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseIpiHandlerExitEvent, TestSize.Level1)
{
    TS_LOGI("test39-26");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().IpiExit(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->IpiHandlerExitEvent(eventInfo_));
}
/**
 * @tc.name: ParseSoftIrqEntryEvent
 * @tc.desc: Test CpuDetailParser: Parse SoftIrqEntryEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSoftIrqEntryEvent, TestSize.Level1)
{
    TS_LOGI("test39-27");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SoftirqEntry(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SoftIrqEntryEvent(eventInfo_));
}
/**
 * @tc.name: ParseSoftIrqRaiseEvent
 * @tc.desc: Test CpuDetailParser: Parse SoftIrqRaiseEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSoftIrqRaiseEvent, TestSize.Level1)
{
    TS_LOGI("test39-28");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SoftirqRaise(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SoftIrqRaiseEvent(eventInfo_));
}
/**
 * @tc.name: ParseSoftIrqExitEvent
 * @tc.desc: Test CpuDetailParser: Parse SoftIrqExitEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSoftIrqExitEvent, TestSize.Level1)
{
    TS_LOGI("test39-29");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().SoftirqExit(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SoftIrqExitEvent(eventInfo_));
}
/**
 * @tc.name: ParseSetRateEvent
 * @tc.desc: Test CpuDetailParser: Parse SetRateEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseSetRateEvent, TestSize.Level1)
{
    TS_LOGI("test39-30");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().ClockSetRate(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->SetRateEvent(eventInfo_));
}
/**
 * @tc.name: ParseClockEnableEvent
 * @tc.desc: Test CpuDetailParser: Parse ClockEnableEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseClockEnableEvent, TestSize.Level1)
{
    TS_LOGI("test39-31");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().ClockEnable(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->ClockEnableEvent(eventInfo_));
}
/**
 * @tc.name: ParseClockDisableEvent
 * @tc.desc: Test CpuDetailParser: Parse ClockDisableEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseClockDisableEvent, TestSize.Level1)
{
    TS_LOGI("test39-32");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().ClockDisable(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->ClockDisableEvent(eventInfo_));
}
/**
 * @tc.name: ParseRegulatorSetVoltageEvent
 * @tc.desc: Test CpuDetailParser: Parse RegulatorSetVoltageEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseRegulatorSetVoltageEvent, TestSize.Level1)
{
    TS_LOGI("test39-33");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().RegulatorSetVoltage(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->RegulatorSetVoltageEvent(eventInfo_));
}
/**
 * @tc.name: ParseRegulatorSetVoltageCompleteEvent
 * @tc.desc: Test CpuDetailParser: Parse RegulatorSetVoltageCompleteEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseRegulatorSetVoltageCompleteEvent, TestSize.Level1)
{
    TS_LOGI("test39-34");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().RegulatorSetVoltageComplete(*ftraceEvent_, data_.data(),
                                                                                data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->RegulatorSetVoltageCompleteEvent(eventInfo_));
}
/**
 * @tc.name: ParseRegulatorDisableEvent
 * @tc.desc: Test CpuDetailParser: Parse RegulatorDisableEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseRegulatorDisableEvent, TestSize.Level1)
{
    TS_LOGI("test39-35");
    EXPECT_TRUE(
        FtraceEventProcessor::GetInstance().RegulatorDisable(*ftraceEvent_, data_.data(), data_.size(), format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->RegulatorDisableEvent(eventInfo_));
}
/**
 * @tc.name: ParseRegulatorDisableCompleteEvent
 * @tc.desc: Test CpuDetailParser: Parse RegulatorDisableCompleteEvent
 * @tc.type: FUNC
 */
HWTEST_F(RawtraceCpuDetailParserTest, ParseRegulatorDisableCompleteEvent, TestSize.Level1)
{
    TS_LOGI("test39-36");
    EXPECT_TRUE(FtraceEventProcessor::GetInstance().RegulatorDisableComplete(*ftraceEvent_, data_.data(), data_.size(),
                                                                             format_));
    eventInfo_.msgPtr = std::move(ftraceEvent_);
    EXPECT_TRUE(cpuDetailParser_->RegulatorDisableCompleteEvent(eventInfo_));
}
} // namespace TraceStreamer
} // namespace SysTuning