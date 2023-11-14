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

#include "cpu_detail_parser.h"
#include <cinttypes>
#include <string_view>
#include "app_start_filter.h"
#include "binder_filter.h"
#include "common_types.h"
#include "cpu_filter.h"
#include "irq_filter.h"
#include "measure_filter.h"
#include "process_filter.h"
#include "slice_filter.h"
#include "stat_filter.h"
#include "symbols_filter.h"
#include "system_event_measure_filter.h"
#include "ftrace_event_processor.h"
#include "string_to_numerical.h"

namespace {
constexpr uint64_t FILTER_MAX_SIZE = 3000000;
}
namespace SysTuning {
namespace TraceStreamer {
CpuDetailParser::CpuDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : streamFilters_(ctx), traceDataCache_(dataCache), printEventParser_(dataCache, ctx)
{
    printEventParser_.SetTraceType(TRACE_FILETYPE_RAW_TRACE);
    printEventParser_.SetTraceClockId(clock_);
    eventToFunctionMap_ = {
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_SWITCH),
         std::bind(&CpuDetailParser::SchedSwitchEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_BLOCKED_REASON),
         std::bind(&CpuDetailParser::SchedBlockReasonEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP),
         std::bind(&CpuDetailParser::SchedWakeupEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKING),
         std::bind(&CpuDetailParser::SchedWakingEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP_NEW),
         std::bind(&CpuDetailParser::SchedWakeupNewEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_EXIT),
         std::bind(&CpuDetailParser::ProcessExitEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_FREE),
         std::bind(&CpuDetailParser::ProcessFreeEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION),
         std::bind(&CpuDetailParser::BinderTractionEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED),
         std::bind(&CpuDetailParser::BinderTractionReceivedEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF),
         std::bind(&CpuDetailParser::BinderTractionAllocBufEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCK),
         std::bind(&CpuDetailParser::BinderTractionLockEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCKED),
         std::bind(&CpuDetailParser::BinderTractionLockedEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK),
         std::bind(&CpuDetailParser::BinderTractionUnLockEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_RENAME),
         std::bind(&CpuDetailParser::TaskRenameEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_NEWTASK),
         std::bind(&CpuDetailParser::TaskNewtaskEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_PRINT),
         std::bind(&CpuDetailParser::ParseTracingMarkWriteOrPrintEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_TRACING_MARK_WRITE),
         std::bind(&CpuDetailParser::ParseTracingMarkWriteOrPrintEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_IDLE),
         std::bind(&CpuDetailParser::CpuIdleEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY),
         std::bind(&CpuDetailParser::CpuFrequencyEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY_LIMITS),
         std::bind(&CpuDetailParser::CpuFrequencyLimitsEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SUSPEND_RESUME),
         std::bind(&CpuDetailParser::SuspendResumeEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_START),
         std::bind(&CpuDetailParser::WorkqueueExecuteStartEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_END),
         std::bind(&CpuDetailParser::WorkqueueExecuteEndEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_ENTRY),
         std::bind(&CpuDetailParser::IrqHandlerEntryEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_EXIT),
         std::bind(&CpuDetailParser::IrqHandlerExitEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_ENTRY),
         std::bind(&CpuDetailParser::IpiHandlerEntryEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_EXIT),
         std::bind(&CpuDetailParser::IpiHandlerExitEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_ENTRY),
         std::bind(&CpuDetailParser::SoftIrqEntryEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_RAISE),
         std::bind(&CpuDetailParser::SoftIrqRaiseEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_EXIT),
         std::bind(&CpuDetailParser::SoftIrqExitEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_SET_RATE),
         std::bind(&CpuDetailParser::SetRateEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_ENABLE),
         std::bind(&CpuDetailParser::ClockEnableEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_DISABLE),
         std::bind(&CpuDetailParser::ClockDisableEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE),
         std::bind(&CpuDetailParser::RegulatorSetVoltageEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE),
         std::bind(&CpuDetailParser::RegulatorSetVoltageCompleteEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE),
         std::bind(&CpuDetailParser::RegulatorDisableEvent, this, std::placeholders::_1)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE),
         std::bind(&CpuDetailParser::RegulatorDisableCompleteEvent, this, std::placeholders::_1)},
    };
}
void CpuDetailParser::EventAppend(std::unique_ptr<RawTraceEventInfo> event)
{
    rawTraceEventList_.emplace_back(std::move(event));
}
bool CpuDetailParser::FilterAllEvents(FtraceCpuDetailMsg& cpuDetail, bool isFinished)
{
    if (cpuDetail.overwrite()) {
        if (!lastOverwrite_) {
            lastOverwrite_ = cpuDetail.overwrite();
        }
        if (lastOverwrite_ != cpuDetail.overwrite()) {
            TS_LOGW("lost events:%" PRIu64 "", cpuDetail.overwrite() - lastOverwrite_);
            lastOverwrite_ = cpuDetail.overwrite();
        }
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_LOST);
    }
    if (!isFinished && rawTraceEventList_.size() < FILTER_MAX_SIZE) {
        return false;
    }
    auto cmp = [](const std::unique_ptr<RawTraceEventInfo>& a, const std::unique_ptr<RawTraceEventInfo>& b) {
        return a->msgPtr->timestamp() < b->msgPtr->timestamp();
    };
#ifdef IS_WASM
    std::sort(rawTraceEventList_.begin(), rawTraceEventList_.end(), cmp);
#else
    std::stable_sort(rawTraceEventList_.begin(), rawTraceEventList_.end(), cmp);
#endif
    if (rawTraceEventList_.empty()) {
        return false;
    }
    traceDataCache_->UpdateTraceTime(rawTraceEventList_.front()->msgPtr->timestamp());
    traceDataCache_->UpdateTraceTime(rawTraceEventList_.back()->msgPtr->timestamp());
    for (size_t i = 0; i < rawTraceEventList_.size(); i++) {
        eventPid_ = rawTraceEventList_[i]->msgPtr->tgid();
        if (eventPid_ != INVALID_INT32) {
            streamFilters_->processFilter_->GetOrCreateThreadWithPid(eventPid_, eventPid_);
        }
        DealEvent(*rawTraceEventList_[i].get());
    }
    TS_LOGI("event_size=%d, rawTraceEventList_.size=%zu", cpuDetail.event().size(), rawTraceEventList_.size());
    rawTraceEventList_.clear();
    cpuDetail.Clear();
    TS_CHECK_TRUE_RET(isFinished, true);
    streamFilters_->cpuFilter_->Finish();
    traceDataCache_->dataDict_.Finish();
    traceDataCache_->UpdataZeroThreadInfo();
    if (traceDataCache_->AppStartTraceEnabled()) {
        streamFilters_->appStartupFilter_->FilterAllAPPStartupData();
    }
    Clear();
    return true;
}
void CpuDetailParser::Clear()
{
    const_cast<TraceStreamerFilters*>(streamFilters_)->FilterClear();
    streamFilters_->symbolsFilter_->Clear();
    streamFilters_->sysEventMemMeasureFilter_->Clear();
    streamFilters_->sysEventVMemMeasureFilter_->Clear();
    printEventParser_.Finish();
}
void CpuDetailParser::DealEvent(const RawTraceEventInfo& event)
{
    eventTid_ = event.msgPtr->common_fields().pid();
    if (eventTid_ != INVALID_INT32) {
        streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), eventTid_);
    }
    if (eventTid_ != INVALID_INT32 && eventPid_ != INVALID_INT32) {
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(eventTid_, eventPid_);
    }
    const auto& eventName = FtraceEventProcessor::GetInstance().GetEventNameById(event.eventId);
    auto iter = eventToFunctionMap_.find(eventName);
    if (iter != eventToFunctionMap_.end()) {
        iter->second(event);
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_NOTSUPPORTED);
    }
}
bool CpuDetailParser::SchedSwitchEvent(const RawTraceEventInfo& event)
{
    auto schedSwitchMsg = event.msgPtr->sched_switch_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_RECEIVED);

    auto schedSwitchPrevState = schedSwitchMsg.prev_state();
    auto nextInternalTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(
        event.msgPtr->timestamp(), schedSwitchMsg.next_pid(), schedSwitchMsg.next_comm());
    auto uprevtid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(
        event.msgPtr->timestamp(), schedSwitchMsg.prev_pid(), schedSwitchMsg.prev_comm());

    streamFilters_->cpuFilter_->InsertSwitchEvent(
        event.msgPtr->timestamp(), event.cpuId, uprevtid, static_cast<uint64_t>(schedSwitchMsg.prev_prio()),
        schedSwitchPrevState, nextInternalTid, static_cast<uint64_t>(schedSwitchMsg.next_prio()), INVALID_DATAINDEX);
    return true;
}
bool CpuDetailParser::SchedBlockReasonEvent(const RawTraceEventInfo& event)
{
    auto reasonMsg = event.msgPtr->sched_blocked_reason_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_RECEIVED);
    uint32_t ioWait = reasonMsg.io_wait();
    auto caller = traceDataCache_->GetDataIndex(
        std::string_view("0x" + SysTuning::base::number(reasonMsg.caller(), SysTuning::base::INTEGER_RADIX_TYPE_HEX)));
    auto itid = streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), reasonMsg.pid());
    if (!streamFilters_->cpuFilter_->InsertBlockedReasonEvent(event.msgPtr->timestamp(), event.cpuId, itid,
                                                              reasonMsg.io_wait(), caller, INVALID_UINT32)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_NOTMATCH);
    }
    return true;
}
bool CpuDetailParser::SchedWakeupEvent(const RawTraceEventInfo& event) const
{
    auto wakeupMsg = event.msgPtr->sched_wakeup_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_RECEIVED);
    auto instants = traceDataCache_->GetInstantsData();

    InternalTid internalTid =
        streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), wakeupMsg.pid());
    InternalTid wakeupFromPid =
        streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), eventTid_);
    instants->AppendInstantEventData(event.msgPtr->timestamp(), schedWakeupIndex_, internalTid, wakeupFromPid);
    streamFilters_->cpuFilter_->InsertWakeupEvent(event.msgPtr->timestamp(), internalTid);
    traceDataCache_->GetRawData()->AppendRawData(0, event.msgPtr->timestamp(), RAW_SCHED_WAKEUP, wakeupMsg.target_cpu(),
                                                 internalTid);
    return true;
}
bool CpuDetailParser::SchedWakingEvent(const RawTraceEventInfo& event) const
{
    auto wakeingMsg = event.msgPtr->sched_waking_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_RECEIVED);
    auto instants = traceDataCache_->GetInstantsData();
    auto internalTid =
        streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), wakeingMsg.pid());
    auto wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), eventTid_);
    streamFilters_->cpuFilter_->InsertWakeupEvent(event.msgPtr->timestamp(), internalTid, true);
    instants->AppendInstantEventData(event.msgPtr->timestamp(), schedWakingIndex_, internalTid, wakeupFromPid);
    traceDataCache_->GetRawData()->AppendRawData(0, event.msgPtr->timestamp(), RAW_SCHED_WAKING,
                                                 wakeingMsg.target_cpu(), wakeupFromPid);
    return true;
}
bool CpuDetailParser::SchedWakeupNewEvent(const RawTraceEventInfo& event) const
{
    auto wakeupNewMsg = event.msgPtr->sched_wakeup_new_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP_NEW, STAT_EVENT_RECEIVED);
    auto instants = traceDataCache_->GetInstantsData();
    auto internalTid =
        streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), wakeupNewMsg.pid());
    auto wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(event.msgPtr->timestamp(), eventTid_);
    instants->AppendInstantEventData(event.msgPtr->timestamp(), schedWakeupNewIndex_, internalTid, wakeupFromPid);
    streamFilters_->cpuFilter_->InsertWakeupEvent(event.msgPtr->timestamp(), internalTid);
    traceDataCache_->GetRawData()->AppendRawData(0, event.msgPtr->timestamp(), RAW_SCHED_WAKEUP,
                                                 wakeupNewMsg.target_cpu(), internalTid);
    return true;
}
bool CpuDetailParser::ProcessExitEvent(const RawTraceEventInfo& event) const
{
    auto procExitMsg = event.msgPtr->sched_process_exit_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_RECEIVED);
    uint32_t pidValue = procExitMsg.pid();
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    auto iTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(event.msgPtr->timestamp(), pidValue,
                                                                             procExitMsg.comm());
    if (streamFilters_->cpuFilter_->InsertProcessExitEvent(event.msgPtr->timestamp(), event.cpuId, iTid)) {
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_NOTMATCH);
        return false;
    }
}
bool CpuDetailParser::ProcessFreeEvent(const RawTraceEventInfo& event) const
{
    auto procFreeMsg = event.msgPtr->sched_process_exit_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_FREE, STAT_EVENT_RECEIVED);
    uint32_t pidValue = procFreeMsg.pid();
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    auto iTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(event.msgPtr->timestamp(), pidValue,
                                                                             procFreeMsg.comm());
    if (streamFilters_->cpuFilter_->InsertProcessFreeEvent(event.msgPtr->timestamp(), iTid)) {
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_FREE, STAT_EVENT_NOTMATCH);
        return false;
    }
}
bool CpuDetailParser::BinderTractionEvent(const RawTraceEventInfo& event) const
{
    auto transactionMsg = event.msgPtr->binder_transaction_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION, STAT_EVENT_RECEIVED);
    int32_t destNode = transactionMsg.target_node();
    int32_t destTgid = transactionMsg.to_proc();
    int32_t destTid = transactionMsg.to_thread();
    int32_t transactionId = transactionMsg.debug_id();
    bool isReply = transactionMsg.reply() == 1;
    uint32_t flags = transactionMsg.flags();
    TS_LOGD("destNode:%d, destTgid:%d, destTid:%d, transactionId:%d, isReply:%d flags:%d, code:%d", destNode, destTgid,
            destTid, transactionId, isReply, flags, transactionMsg.code());
    streamFilters_->binderFilter_->SendTraction(event.msgPtr->timestamp(), eventTid_, transactionId, destNode, destTgid,
                                                destTid, isReply, flags, transactionMsg.code());
    return true;
}
bool CpuDetailParser::BinderTractionAllocBufEvent(const RawTraceEventInfo& event) const
{
    auto allocBufMsg = event.msgPtr->binder_transaction_alloc_buf_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF, STAT_EVENT_RECEIVED);
    streamFilters_->binderFilter_->TransactionAllocBuf(event.msgPtr->timestamp(), eventTid_, allocBufMsg.data_size(),
                                                       allocBufMsg.offsets_size());
    TS_LOGD("dataSize:%lu, offsetSize:%lu", allocBufMsg.data_size(), allocBufMsg.offsets_size());
    return true;
}
bool CpuDetailParser::BinderTractionReceivedEvent(const RawTraceEventInfo& event) const
{
    auto recvedMsg = event.msgPtr->binder_transaction_received_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED, STAT_EVENT_RECEIVED);
    int32_t transactionId = recvedMsg.debug_id();
    streamFilters_->binderFilter_->ReceiveTraction(event.msgPtr->timestamp(), eventTid_, transactionId);
    TS_LOGD("transactionId:%d", transactionId);
    return true;
}
bool CpuDetailParser::BinderTractionLockEvent(const RawTraceEventInfo& event) const
{
    auto lockMsg = event.msgPtr->binder_lock_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_LOCK, STAT_EVENT_RECEIVED);
    streamFilters_->binderFilter_->TractionLock(event.msgPtr->timestamp(), eventTid_, lockMsg.tag());
    TS_LOGD("tag:%s", lockMsg.tag().c_str());
    return true;
}
bool CpuDetailParser::BinderTractionLockedEvent(const RawTraceEventInfo& event) const
{
    auto lockedMsg = event.msgPtr->binder_locked_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_LOCKED, STAT_EVENT_RECEIVED);
    streamFilters_->binderFilter_->TractionLocked(event.msgPtr->timestamp(), eventTid_, lockedMsg.tag());
    return true;
}
bool CpuDetailParser::BinderTractionUnLockEvent(const RawTraceEventInfo& event) const
{
    auto unlockMsg = event.msgPtr->binder_unlock_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK, STAT_EVENT_RECEIVED);
    streamFilters_->binderFilter_->TractionUnlock(event.msgPtr->timestamp(), eventTid_, unlockMsg.tag());
    return true;
}
bool CpuDetailParser::TaskRenameEvent(const RawTraceEventInfo& event) const
{
    auto renameMsg = event.msgPtr->task_rename_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_RENAME, STAT_EVENT_RECEIVED);
    streamFilters_->processFilter_->UpdateOrCreateThreadWithName(event.msgPtr->timestamp(), renameMsg.pid(),
                                                                 renameMsg.newcomm());
    return true;
}
bool CpuDetailParser::TaskNewtaskEvent(const RawTraceEventInfo& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_RECEIVED);
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool CpuDetailParser::ParseTracingMarkWriteOrPrintEvent(const RawTraceEventInfo& event)
{
    auto printMsg = event.msgPtr->print_format();
    BytraceLine line;
    line.tgid = eventPid_;
    line.pid = eventTid_;
    line.ts = event.msgPtr->timestamp();
    printEventParser_.ParsePrintEvent(event.msgPtr->comm(), line.ts, eventTid_, printMsg.buf(), line);
    return true;
}
bool CpuDetailParser::CpuIdleEvent(const RawTraceEventInfo& event) const
{
    auto idleMsg = event.msgPtr->cpu_idle_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_RECEIVED);
    std::optional<uint32_t> eventCpu = idleMsg.cpu_id();
    std::optional<uint64_t> newState = idleMsg.state();
    if (!eventCpu.has_value()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        TS_LOGW("Convert event cpu Failed");
        return false;
    }
    if (!newState.has_value()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        TS_LOGW("Convert event state Failed");
        return false;
    }

    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpu.value(), cpuIdleIndex_, event.msgPtr->timestamp(),
                                                            config_.GetStateValue(newState.value()));

    // Add cpu_idle event to raw_data_table
    traceDataCache_->GetRawData()->AppendRawData(0, event.msgPtr->timestamp(), RAW_CPU_IDLE, eventCpu.value(), 0);
    return true;
}
bool CpuDetailParser::CpuFrequencyEvent(const RawTraceEventInfo& event) const
{
    auto frequencyMsg = event.msgPtr->cpu_frequency_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_RECEIVED);
    std::optional<uint64_t> newState = frequencyMsg.state();
    std::optional<uint32_t> eventCpu = frequencyMsg.cpu_id();

    if (!newState.has_value()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        TS_LOGW("Convert event state Failed");
        return false;
    }
    if (!eventCpu.has_value()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        TS_LOGW("Convert event cpu Failed");
        return false;
    }

    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpu.value(), cpuFrequencyIndex_,
                                                            event.msgPtr->timestamp(), newState.value());
    return true;
}
bool CpuDetailParser::CpuFrequencyLimitsEvent(const RawTraceEventInfo& event) const
{
    auto limitsMsg = event.msgPtr->cpu_frequency_limits_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_RECEIVED);
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(limitsMsg.cpu_id(), cpuFrequencyLimitMaxIndex_,
                                                            event.msgPtr->timestamp(), limitsMsg.max_freq());
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(limitsMsg.cpu_id(), cpuFrequencyLimitMinIndex_,
                                                            event.msgPtr->timestamp(), limitsMsg.min_freq());
    return true;
}
bool CpuDetailParser::SuspendResumeEvent(const RawTraceEventInfo& event) const
{
    auto resumeMsg = event.msgPtr->suspend_resume_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SUSPEND_RESUME, STAT_EVENT_RECEIVED);
    int32_t val = resumeMsg.val();
    uint32_t start = resumeMsg.start();
    std::string action = resumeMsg.action();
    UNUSED(val);
    UNUSED(start);
    UNUSED(action);
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SUSPEND_RESUME, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool CpuDetailParser::WorkqueueExecuteStartEvent(const RawTraceEventInfo& event) const
{
    auto executeStartMsg = event.msgPtr->workqueue_execute_start_format();
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_START, STAT_EVENT_RECEIVED);
    auto funcNameIndex = streamFilters_->symbolsFilter_->GetFunc(executeStartMsg.function());
    size_t result = INVALID_UINT32;
    if (funcNameIndex == INVALID_UINT64) {
        std::string addrStr = "0x" + base::number(executeStartMsg.function(), base::INTEGER_RADIX_TYPE_HEX);
        auto addStrIndex = traceDataCache_->GetDataIndex(addrStr);
        result = streamFilters_->sliceFilter_->BeginSlice(event.msgPtr->comm(), event.msgPtr->timestamp(), eventPid_,
                                                          eventPid_, workQueueIndex_, addStrIndex);
    } else {
        result = streamFilters_->sliceFilter_->BeginSlice(event.msgPtr->comm(), event.msgPtr->timestamp(), eventPid_,
                                                          eventPid_, workQueueIndex_, funcNameIndex);
    }

    traceDataCache_->GetInternalSlicesData()->AppendDistributeInfo();
    if (result == INVALID_UINT32) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_START, STAT_EVENT_DATA_LOST);
    }
    return true;
}
bool CpuDetailParser::WorkqueueExecuteEndEvent(const RawTraceEventInfo& event) const
{
    auto executeEndMsg = event.msgPtr->workqueue_execute_end_format();
    if (!streamFilters_->sliceFilter_->EndSlice(event.msgPtr->timestamp(), eventPid_, eventPid_, workQueueIndex_)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_NOTMATCH);
        return false;
    }
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::IrqHandlerEntryEvent(const RawTraceEventInfo& event) const
{
    auto irqEntryMsg = event.msgPtr->irq_handler_entry_format();
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_RECEIVED);
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    streamFilters_->irqFilter_->IrqHandlerEntry(event.msgPtr->timestamp(), event.cpuId,
                                                traceDataCache_->GetDataIndex(irqEntryMsg.name()));
    return true;
}
bool CpuDetailParser::IrqHandlerExitEvent(const RawTraceEventInfo& event) const
{
    auto irqExitMsg = event.msgPtr->irq_handler_exit_format();
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IrqHandlerExit(event.msgPtr->timestamp(), event.cpuId, irqExitMsg.irq(),
                                               static_cast<uint32_t>(irqExitMsg.ret()));
    return true;
}
bool CpuDetailParser::IpiHandlerEntryEvent(const RawTraceEventInfo& event) const
{
    auto ipiEntryMsg = event.msgPtr->ipi_entry_format();
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_ENTRY, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IpiHandlerEntry(event.msgPtr->timestamp(), event.cpuId,
                                                traceDataCache_->GetDataIndex(ipiEntryMsg.reason()));
    return true;
}
bool CpuDetailParser::IpiHandlerExitEvent(const RawTraceEventInfo& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_EXIT, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IpiHandlerExit(event.msgPtr->timestamp(), event.cpuId);
    return true;
}
bool CpuDetailParser::SoftIrqEntryEvent(const RawTraceEventInfo& event) const
{
    auto softIrqEntryMsg = event.msgPtr->softirq_entry_format();
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->SoftIrqEntry(event.msgPtr->timestamp(), event.cpuId,
                                             static_cast<uint32_t>(softIrqEntryMsg.vec()));
    return true;
}
bool CpuDetailParser::SoftIrqRaiseEvent(const RawTraceEventInfo& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool CpuDetailParser::SoftIrqExitEvent(const RawTraceEventInfo& event) const
{
    auto softIrqExitMsg = event.msgPtr->softirq_exit_format();
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->SoftIrqExit(event.msgPtr->timestamp(), event.cpuId,
                                            static_cast<uint32_t>(softIrqExitMsg.vec()));
    return true;
}
bool CpuDetailParser::SetRateEvent(const RawTraceEventInfo& event) const
{
    auto clockSetRateMsg = event.msgPtr->clock_set_rate_format();
    DataIndex nameIndex = traceDataCache_->GetDataIndex(clockSetRateMsg.name());
    streamFilters_->clockRateFilter_->AppendNewMeasureData(clockSetRateMsg.cpu_id(), nameIndex,
                                                           event.msgPtr->timestamp(), clockSetRateMsg.state());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SET_RATE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::ClockEnableEvent(const RawTraceEventInfo& event) const
{
    auto clockEnableMsg = event.msgPtr->clock_enable_format();
    DataIndex nameIndex = traceDataCache_->GetDataIndex(clockEnableMsg.name());
    streamFilters_->clockEnableFilter_->AppendNewMeasureData(clockEnableMsg.cpu_id(), nameIndex,
                                                             event.msgPtr->timestamp(), clockEnableMsg.state());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_ENABLE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::ClockDisableEvent(const RawTraceEventInfo& event) const
{
    auto clockDisableMsg = event.msgPtr->clock_disable_format();
    DataIndex nameIndex = traceDataCache_->GetDataIndex(clockDisableMsg.name());
    streamFilters_->clockDisableFilter_->AppendNewMeasureData(clockDisableMsg.cpu_id(), nameIndex,
                                                              event.msgPtr->timestamp(), clockDisableMsg.state());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_DISABLE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::RegulatorSetVoltageEvent(const RawTraceEventInfo& event) const
{
    UNUSED(event);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE, STAT_EVENT_NOTSUPPORTED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::RegulatorSetVoltageCompleteEvent(const RawTraceEventInfo& event) const
{
    UNUSED(event);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE,
                                                    STAT_EVENT_NOTSUPPORTED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::RegulatorDisableEvent(const RawTraceEventInfo& event) const
{
    UNUSED(event);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE, STAT_EVENT_NOTSUPPORTED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE, STAT_EVENT_RECEIVED);
    return true;
}
bool CpuDetailParser::RegulatorDisableCompleteEvent(const RawTraceEventInfo& event) const
{
    UNUSED(event);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE, STAT_EVENT_NOTSUPPORTED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE, STAT_EVENT_RECEIVED);
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
