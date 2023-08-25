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
#include "htrace_event_parser.h"
#include <string>
#include <cinttypes>
#include "app_start_filter.h"
#include "binder_filter.h"
#include "binder.pbreader.h"
#include "clk.pbreader.h"
#include "clock_filter_ex.h"
#include "cpu_filter.h"
#include "ftrace.pbreader.h"
#include "ftrace_event.pbreader.h"
#include "sched.pbreader.h"
#include "ipi.pbreader.h"
#include "irq_filter.h"
#include "irq.pbreader.h"
#include "log.h"
#include "measure_filter.h"
#include "oom.pbreader.h"
#include "power.pbreader.h"
#include "process_filter.h"
#include "raw_syscalls.pbreader.h"
#include "signal.pbreader.h"
#include "slice_filter.h"
#include "stat_filter.h"
#include "symbols_filter.h"
#include "system_event_measure_filter.h"
#include "task.pbreader.h"
#include "thread_state_flag.h"
#include "trace_plugin_result.pbreader.h"
#include "workqueue.pbreader.h"
namespace SysTuning {
namespace TraceStreamer {
const int32_t MIN_DATA_AREA = 10;
const int32_t DATA_AREA_START = 1;
const int32_t DATA_AREA_END = 11;

HtraceEventParser::HtraceEventParser(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : EventParserBase(dataCache, filter),
      workQueueId_(dataCache->dataDict_.GetStringIndex("workqueue")),
      printEventParser_(traceDataCache_, streamFilters_)
{
    eventToFunctionMap_ = {{config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION),
                            std::bind(&HtraceEventParser::BinderTractionEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED),
                            std::bind(&HtraceEventParser::BinderTractionReceivedEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF),
                            std::bind(&HtraceEventParser::BinderTractionAllocBufEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCK),
                            std::bind(&HtraceEventParser::BinderTractionLockEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCKED),
                            std::bind(&HtraceEventParser::BinderTractionLockedEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK),
                            std::bind(&HtraceEventParser::BinderTractionUnLockEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SCHED_SWITCH),
                            std::bind(&HtraceEventParser::SchedSwitchEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SCHED_BLOCKED_REASON),
                            std::bind(&HtraceEventParser::SchedBlockReasonEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_TASK_RENAME),
                            std::bind(&HtraceEventParser::TaskRenameEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_TASK_NEWTASK),
                            std::bind(&HtraceEventParser::TaskNewtaskEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_PRINT),
                            std::bind(&HtraceEventParser::ParsePrintEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP),
                            std::bind(&HtraceEventParser::SchedWakeupEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP_NEW),
                            std::bind(&HtraceEventParser::SchedWakeupNewEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_EXIT),
                            std::bind(&HtraceEventParser::ProcessExitEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_FREE),
                            std::bind(&HtraceEventParser::ProcessFreeEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKING),
                            std::bind(&HtraceEventParser::SchedWakingEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CPU_IDLE),
                            std::bind(&HtraceEventParser::CpuIdleEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY),
                            std::bind(&HtraceEventParser::CpuFrequencyEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY_LIMITS),
                            std::bind(&HtraceEventParser::CpuFrequencyLimitsEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SUSPEND_RESUME),
                            std::bind(&HtraceEventParser::SuspendResumeEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_START),
                            std::bind(&HtraceEventParser::WorkqueueExecuteStartEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_END),
                            std::bind(&HtraceEventParser::WorkqueueExecuteEndEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_SET_RATE),
                            std::bind(&HtraceEventParser::ClockSetRateEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_ENABLE),
                            std::bind(&HtraceEventParser::ClockEnableEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_DISABLE),
                            std::bind(&HtraceEventParser::ClockDisableEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLK_SET_RATE),
                            std::bind(&HtraceEventParser::ClkSetRateEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLK_ENABLE),
                            std::bind(&HtraceEventParser::ClkEnableEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_CLK_DISABLE),
                            std::bind(&HtraceEventParser::ClkDisableEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_ENTRY),
                            std::bind(&HtraceEventParser::IrqHandlerEntryEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_EXIT),
                            std::bind(&HtraceEventParser::IrqHandlerExitEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_IPI_ENTRY),
                            std::bind(&HtraceEventParser::IpiHandlerEntryEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_IPI_EXIT),
                            std::bind(&HtraceEventParser::IpiHandlerExitEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_ENTRY),
                            std::bind(&HtraceEventParser::SoftIrqEntryEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_RAISE),
                            std::bind(&HtraceEventParser::SoftIrqRaiseEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_EXIT),
                            std::bind(&HtraceEventParser::SoftIrqExitEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SYS_ENTRY),
                            std::bind(&HtraceEventParser::SysEnterEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_SYS_EXIT),
                            std::bind(&HtraceEventParser::SysExitEvent, this, std::placeholders::_1)},
                           {config_.eventNameMap_.at(TRACE_EVENT_OOM_SCORE_ADJ_UPDATE),
                            std::bind(&HtraceEventParser::OomScoreAdjUpdate, this, std::placeholders::_1)}};
}

HtraceEventParser::~HtraceEventParser()
{
    TS_LOGI("thread count:%u", static_cast<uint32_t>(tids_.size()));
    TS_LOGI("process count:%u", static_cast<uint32_t>(pids_.size()));
    TS_LOGI("ftrace ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(ftraceStartTime_),
            static_cast<unsigned long long>(ftraceEndTime_));
    TS_LOGI("ftrace origin ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(ftraceOriginStartTime_),
            static_cast<unsigned long long>(ftraceOriginEndTime_));
}

void HtraceEventParser::ParseDataItem(HtraceDataSegment& tracePacket, BuiltinClocks clock)
{
    if (clock != clock_) {
        clock_ = clock;
        printEventParser_.SetTraceType(TRACE_FILETYPE_H_TRACE);
        printEventParser_.SetTraceClockId(clock);
    }
    ProtoReader::TracePluginResult_Reader tracePluginResult(tracePacket.protoData);
    if (!tracePluginResult.has_ftrace_cpu_detail()) {
        return;
    }

    for (auto it = tracePluginResult.ftrace_cpu_detail(); it; ++it) {
        ProtoReader::FtraceCpuDetailMsg_Reader msg(it->ToBytes());
        eventCpu_ = msg.cpu();
        auto events = msg.event();
        if (!msg.has_event()) {
            return;
        }
        if (msg.overwrite()) {
            if (!lastOverwrite_) {
                lastOverwrite_ = msg.overwrite();
            }
            if (lastOverwrite_ != msg.overwrite()) {
                TS_LOGW("lost events:%" PRIu64 "", msg.overwrite() - lastOverwrite_);
                lastOverwrite_ = msg.overwrite();
            }
            streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_LOST);
        }
        // parser cpu event
        auto kTimestampDataAreaNumber = ProtoReader::FtraceEvent_Reader::kTimestampDataAreaNumber;
        auto tsTag = CreateTagVarInt(kTimestampDataAreaNumber);
        for (auto i = events; i; i++) {
            ProtoReader::BytesView event(i->ToBytes());
            uint64_t timeStamp = 0;
            if (event.size_ > MIN_DATA_AREA && event.data_[0] == tsTag) {
                (void)ProtoReader::VarIntDecode(event.data_ + DATA_AREA_START, event.data_ + DATA_AREA_END, &timeStamp);
            }
            eventTimeStamp_ = timeStamp;
            ftraceOriginStartTime_ = std::min(ftraceOriginStartTime_, eventTimeStamp_);
            ftraceOriginEndTime_ = std::max(ftraceOriginEndTime_, eventTimeStamp_);
            eventTimeStamp_ = streamFilters_->clockFilter_->ToPrimaryTraceTime(clock, eventTimeStamp_);
            ftraceStartTime_ = std::min(ftraceStartTime_, eventTimeStamp_);
            ftraceEndTime_ = std::max(ftraceEndTime_, eventTimeStamp_);
            traceDataCache_->UpdateTraceTime(eventTimeStamp_);
            ProtoReader::BytesView commonField;
            htraceEventList_.push_back(
                std::make_unique<EventInfo>(eventTimeStamp_, eventCpu_, tracePacket.seg, i->ToBytes()));
            FilterAllEventsReader();
        }
    }
}
void HtraceEventParser::DealEvent(const ProtoReader::FtraceEvent_Reader& event,
                                  const ProtoReader::FtraceEvent_CommonFileds_Reader& comonFields)
{
    if (comonFields.pid() != INVALID_INT32) {
        streamFilters_->processFilter_->UpdateOrCreateThreadWithPidAndName(comonFields.pid(), event.tgid(),
                                                                           event.comm().ToStdString());
    }
    if (event.has_sched_switch_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_SWITCH,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedSwitchFormatDataAreaNumber>());
    } else if (event.has_wakeup_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_WAKEUP, event.at<ProtoReader::FtraceEvent_Reader::kWakeupFormatDataAreaNumber>());
    } else if (event.has_binder_alloc_lru_end_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_SWITCH,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderAllocLruEndFormatDataAreaNumber>());
    } else if (event.has_task_rename_format()) {
        InvokeFunc(TRACE_EVENT_TASK_RENAME,
                   event.at<ProtoReader::FtraceEvent_Reader::kTaskRenameFormatDataAreaNumber>());
    } else if (event.has_sched_blocked_reason_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_BLOCKED_REASON,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedBlockedReasonFormatDataAreaNumber>());
    } else if (event.has_task_newtask_format()) {
        InvokeFunc(TRACE_EVENT_TASK_NEWTASK,
                   event.at<ProtoReader::FtraceEvent_Reader::kTaskNewtaskFormatDataAreaNumber>());
    } else if (event.has_sched_wakeup_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_WAKEUP,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedWakeupFormatDataAreaNumber>());
    } else if (event.has_sched_wakeup_new_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_WAKEUP,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedWakeupNewFormatDataAreaNumber>());
    } else if (event.has_sched_process_exit_format()) {
        InvokeFunc(TRACE_EVENT_PROCESS_EXIT,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedProcessExitFormatDataAreaNumber>());
    } else if (event.has_sched_process_free_format()) {
        InvokeFunc(TRACE_EVENT_PROCESS_FREE,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedProcessFreeFormatDataAreaNumber>());
    } else if (event.has_sched_waking_format()) {
        InvokeFunc(TRACE_EVENT_SCHED_WAKING,
                   event.at<ProtoReader::FtraceEvent_Reader::kSchedWakingFormatDataAreaNumber>());
    } else if (event.has_cpu_idle_format()) {
        InvokeFunc(TRACE_EVENT_CPU_IDLE, event.at<ProtoReader::FtraceEvent_Reader::kCpuIdleFormatDataAreaNumber>());
    } else if (event.has_cpu_frequency_format()) {
        InvokeFunc(TRACE_EVENT_CPU_FREQUENCY,
                   event.at<ProtoReader::FtraceEvent_Reader::kCpuFrequencyFormatDataAreaNumber>());
    } else if (event.has_cpu_frequency_limits_format()) {
        InvokeFunc(TRACE_EVENT_CPU_FREQUENCY_LIMITS,
                   event.at<ProtoReader::FtraceEvent_Reader::kCpuFrequencyLimitsFormatDataAreaNumber>());
    } else if (event.has_print_format()) {
        InvokeFunc(TRACE_EVENT_PRINT, event.at<ProtoReader::FtraceEvent_Reader::kPrintFormatDataAreaNumber>());
    } else if (event.has_suspend_resume_format()) {
        InvokeFunc(TRACE_EVENT_SUSPEND_RESUME,
                   event.at<ProtoReader::FtraceEvent_Reader::kSuspendResumeFormatDataAreaNumber>());
    } else if (event.has_workqueue_execute_start_format()) {
        InvokeFunc(TRACE_EVENT_WORKQUEUE_EXECUTE_START,
                   event.at<ProtoReader::FtraceEvent_Reader::kWorkqueueExecuteStartFormatDataAreaNumber>());
    } else if (event.has_workqueue_execute_end_format()) {
        InvokeFunc(TRACE_EVENT_WORKQUEUE_EXECUTE_END,
                   event.at<ProtoReader::FtraceEvent_Reader::kWorkqueueExecuteEndFormatDataAreaNumber>());
    } else if (event.has_clock_disable_format()) {
        InvokeFunc(TRACE_EVENT_CLOCK_DISABLE,
                   event.at<ProtoReader::FtraceEvent_Reader::kClockDisableFormatDataAreaNumber>());
    } else if (event.has_clock_enable_format()) {
        InvokeFunc(TRACE_EVENT_CLOCK_ENABLE,
                   event.at<ProtoReader::FtraceEvent_Reader::kClockEnableFormatDataAreaNumber>());
    } else if (event.has_clock_set_rate_format()) {
        InvokeFunc(TRACE_EVENT_CLOCK_SET_RATE,
                   event.at<ProtoReader::FtraceEvent_Reader::kClockSetRateFormatDataAreaNumber>());
    } else if (event.has_clk_disable_format()) {
        InvokeFunc(TRACE_EVENT_CLK_DISABLE,
                   event.at<ProtoReader::FtraceEvent_Reader::kClkDisableFormatDataAreaNumber>());
    } else if (event.has_clk_enable_format()) {
        InvokeFunc(TRACE_EVENT_CLK_ENABLE, event.at<ProtoReader::FtraceEvent_Reader::kClkEnableFormatDataAreaNumber>());
    } else if (event.has_clk_set_rate_format()) {
        InvokeFunc(TRACE_EVENT_CLK_SET_RATE,
                   event.at<ProtoReader::FtraceEvent_Reader::kClkSetRateFormatDataAreaNumber>());
    } else if (event.has_sys_enter_format()) {
        InvokeFunc(TRACE_EVENT_SYS_ENTRY, event.at<ProtoReader::FtraceEvent_Reader::kSysEnterFormatDataAreaNumber>());
    } else if (event.has_sys_exit_format()) {
        InvokeFunc(TRACE_EVENT_SYS_EXIT, event.at<ProtoReader::FtraceEvent_Reader::kSysExitFormatDataAreaNumber>());
    } else if (event.has_binder_transaction_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderTransactionFormatDataAreaNumber>());
    } else if (event.has_binder_transaction_received_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderTransactionReceivedFormatDataAreaNumber>());
    } else if (event.has_binder_transaction_alloc_buf_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderTransactionAllocBufFormatDataAreaNumber>());
    } else if (event.has_binder_lock_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION_LOCK,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderLockFormatDataAreaNumber>());
    } else if (event.has_binder_unlock_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderUnlockFormatDataAreaNumber>());
    } else if (event.has_binder_locked_format()) {
        InvokeFunc(TRACE_EVENT_BINDER_TRANSACTION_LOCKED,
                   event.at<ProtoReader::FtraceEvent_Reader::kBinderLockedFormatDataAreaNumber>());
    } else if (event.has_irq_handler_entry_format()) {
        InvokeFunc(TRACE_EVENT_IRQ_HANDLER_ENTRY,
                   event.at<ProtoReader::FtraceEvent_Reader::kIrqHandlerEntryFormatDataAreaNumber>());
    } else if (event.has_irq_handler_exit_format()) {
        InvokeFunc(TRACE_EVENT_IRQ_HANDLER_EXIT,
                   event.at<ProtoReader::FtraceEvent_Reader::kIrqHandlerExitFormatDataAreaNumber>());
    } else if (event.has_softirq_entry_format()) {
        InvokeFunc(TRACE_EVENT_SOFTIRQ_ENTRY,
                   event.at<ProtoReader::FtraceEvent_Reader::kSoftirqEntryFormatDataAreaNumber>());
    } else if (event.has_softirq_exit_format()) {
        InvokeFunc(TRACE_EVENT_SOFTIRQ_EXIT,
                   event.at<ProtoReader::FtraceEvent_Reader::kSoftirqExitFormatDataAreaNumber>());
    } else if (event.has_oom_score_adj_update_format()) {
        InvokeFunc(TRACE_EVENT_OOM_SCORE_ADJ_UPDATE,
                   event.at<ProtoReader::FtraceEvent_Reader::kOomScoreAdjUpdateFormatDataAreaNumber>());
    } else if (event.has_signal_generate_format()) {
        InvokeFunc(TRACE_EVENT_SIGNAL_GENERATE,
                   event.at<ProtoReader::FtraceEvent_Reader::kSignalGenerateFormatDataAreaNumber>());
    } else if (event.has_signal_deliver_format()) {
        InvokeFunc(TRACE_EVENT_SIGNAL_DELIVER,
                   event.at<ProtoReader::FtraceEvent_Reader::kSignalDeliverFormatDataAreaNumber>());
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_NOTSUPPORTED);
    }
}
bool HtraceEventParser::BinderTractionAllocBufEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF, STAT_EVENT_RECEIVED);
    ProtoReader::BinderTransactionAllocBufFormat_Reader msg(event.Data(), event.Size());
    uint64_t dataSize = msg.data_size();
    uint64_t offsetsSize = msg.offsets_size();
    streamFilters_->binderFilter_->TransactionAllocBuf(eventTimeStamp_, eventTid_, dataSize, offsetsSize);
    TS_LOGD("dataSize:%lu, offsetSize:%lu", dataSize, offsetsSize);
    return true;
}
bool HtraceEventParser::BinderTractionEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION, STAT_EVENT_RECEIVED);
    ProtoReader::BinderTransactionFormat_Reader msg(event.Data(), event.Size());
    int32_t destNode = msg.target_node();
    int32_t destTgid = msg.to_proc();
    int32_t destTid = msg.to_thread();
    int32_t transactionId = msg.debug_id();
    bool isReply = msg.reply() == 1;
    uint32_t flags = msg.flags();
    TS_LOGD("destNode:%d, destTgid:%d, destTid:%d, transactionId:%d, isReply:%d flags:%d, code:%d", destNode, destTgid,
            destTid, transactionId, isReply, flags, msg.code());
    streamFilters_->binderFilter_->SendTraction(eventTimeStamp_, eventTid_, transactionId, destNode, destTgid, destTid,
                                                isReply, flags, msg.code());
    if (traceDataCache_->BinderRunnableTraceEnabled() && !streamFilters_->binderFilter_->IsAsync(flags)) {
        streamFilters_->cpuFilter_->InsertRunnableBinderEvent(
            transactionId, streamFilters_->processFilter_->GetInternalTid(eventTid_));
    }
    return true;
}
bool HtraceEventParser::BinderTractionReceivedEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED, STAT_EVENT_RECEIVED);
    ProtoReader::BinderTransactionReceivedFormat_Reader msg(event.Data(), event.Size());
    int32_t transactionId = msg.debug_id();
    streamFilters_->binderFilter_->ReceiveTraction(eventTimeStamp_, eventTid_, transactionId);
    if (traceDataCache_->BinderRunnableTraceEnabled()) {
        streamFilters_->cpuFilter_->InsertRunnableBinderRecvEvent(
            transactionId, streamFilters_->processFilter_->GetInternalTid(eventTid_));
    }
    TS_LOGD("transactionId:%d", transactionId);
    return true;
}
bool HtraceEventParser::BinderTractionLockEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_LOCK, STAT_EVENT_RECEIVED);
    ProtoReader::BinderLockFormat_Reader msg(event.Data(), event.Size());
    std::string tag = msg.tag().ToStdString();
    streamFilters_->binderFilter_->TractionLock(eventTimeStamp_, eventTid_, tag);
    TS_LOGD("tag:%s", tag.c_str());
    return true;
}
bool HtraceEventParser::BinderTractionLockedEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_LOCKED, STAT_EVENT_RECEIVED);
    ProtoReader::BinderLockedFormat_Reader msg(event.Data(), event.Size());
    std::string tag = msg.tag().ToStdString();
    streamFilters_->binderFilter_->TractionLocked(eventTimeStamp_, eventTid_, tag);
    return true;
}
bool HtraceEventParser::BinderTractionUnLockEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK, STAT_EVENT_RECEIVED);
    ProtoReader::BinderUnlockFormat_Reader msg(event.Data(), event.Size());
    std::string tag = msg.tag().ToStdString();
    streamFilters_->binderFilter_->TractionUnlock(eventTimeStamp_, eventTid_, tag);
    return true;
}
bool HtraceEventParser::SchedSwitchEvent(const ProtoReader::DataArea& event)
{
    ProtoReader::SchedSwitchFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_RECEIVED);
    uint32_t prevPrioValue = msg.prev_prio();
    uint32_t nextPrioValue = msg.next_prio();
    uint32_t prevPidValue = msg.prev_pid();
    uint32_t nextPidValue = msg.next_pid();
    if (!tids_.count(prevPidValue)) {
        tids_.insert(prevPidValue);
    }
    if (!tids_.count(nextPidValue)) {
        tids_.insert(nextPidValue);
    }
    std::string prevCommStr = msg.prev_comm().ToStdString();
    std::string nextCommStr = msg.next_comm().ToStdString();
    auto prevState = msg.prev_state();

    auto nextInternalTid =
        streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, nextPidValue, nextCommStr);
    auto uprevtid =
        streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, prevPidValue, prevCommStr);
    streamFilters_->cpuFilter_->InsertSwitchEvent(eventTimeStamp_, eventCpu_, uprevtid,
                                                  static_cast<uint64_t>(prevPrioValue), prevState, nextInternalTid,
                                                  static_cast<uint64_t>(nextPrioValue), INVALID_DATAINDEX);
    return true;
}
bool HtraceEventParser::SchedBlockReasonEvent(const ProtoReader::DataArea& event)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_RECEIVED);
    ProtoReader::SchedBlockedReasonFormat_Reader msg(event.Data(), event.Size());
    uint32_t pid = msg.pid();
    uint32_t ioWait = msg.io_wait();
    auto caller = traceDataCache_->GetDataIndex(
        std::string_view("0x" + SysTuning::base::number(msg.caller(), SysTuning::base::INTEGER_RADIX_TYPE_HEX)));
    auto itid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, pid);
    if (!streamFilters_->cpuFilter_->InsertBlockedReasonEvent(eventTimeStamp_, eventCpu_, itid, ioWait, caller,
                                                              INVALID_UINT32)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_NOTMATCH);
    }
    return true;
}
bool HtraceEventParser::ProcessExitEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_RECEIVED);
    ProtoReader::SchedProcessExitFormat_Reader msg(event.Data(), event.Size());
    uint32_t pidValue = msg.pid();
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    auto iTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, pidValue,
                                                                             msg.comm().ToStdString());
    if (streamFilters_->cpuFilter_->InsertProcessExitEvent(eventTimeStamp_, eventCpu_, iTid)) {
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_NOTMATCH);
        return false;
    }
}
bool HtraceEventParser::ProcessFreeEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_FREE, STAT_EVENT_RECEIVED);
    ProtoReader::SchedProcessFreeFormat_Reader msg(event.Data(), event.Size());
    uint32_t pidValue = msg.pid();
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    auto iTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, pidValue,
                                                                             msg.comm().ToStdString());
    if (streamFilters_->cpuFilter_->InsertProcessFreeEvent(eventTimeStamp_, iTid)) {
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_FREE, STAT_EVENT_NOTMATCH);
        return false;
    }
}
bool HtraceEventParser::TaskRenameEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_RENAME, STAT_EVENT_RECEIVED);
    ProtoReader::TaskRenameFormat_Reader msg(event.Data(), event.Size());
    auto commStr = msg.newcomm();
    auto pidValue = msg.pid();
    streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, pidValue, commStr.ToStdString());
    return true;
}
bool HtraceEventParser::TaskNewtaskEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_RECEIVED);
    // the clone flag from txt trace from kernel original is HEX, but when it is converted from proto
    // based trace, it will be OCT number, it is not stable, so we decide to ignore it
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool HtraceEventParser::ParsePrintEvent(const ProtoReader::DataArea& event)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PRINT, STAT_EVENT_RECEIVED);
    ProtoReader::PrintFormat_Reader msg(event.Data(), event.Size());
    BytraceLine line;
    line.tgid = eventPid_;
    line.pid = eventTid_;
    line.ts = eventTimeStamp_;
    printEventParser_.ParsePrintEvent(comm_, eventTimeStamp_, eventTid_, msg.buf().ToStdString(), line);
    if (!tids_.count(eventTid_)) {
        tids_.insert(eventTid_);
    }
    return true;
}
bool HtraceEventParser::SchedWakeupEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::SchedWakeupFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_RECEIVED);
    auto instants = traceDataCache_->GetInstantsData();

    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, msg.pid());
    InternalTid wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventTid_);
    instants->AppendInstantEventData(eventTimeStamp_, schedWakeupName_, internalTid, wakeupFromPid);
    streamFilters_->cpuFilter_->InsertWakeupEvent(eventTimeStamp_, internalTid);
    uint32_t targetCpu = msg.target_cpu();
    traceDataCache_->GetRawData()->AppendRawData(0, eventTimeStamp_, RAW_SCHED_WAKEUP, targetCpu, internalTid);
    return true;
}
bool HtraceEventParser::SchedWakeupNewEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP_NEW, STAT_EVENT_RECEIVED);
    ProtoReader::SchedWakeupNewFormat_Reader msg(event.Data(), event.Size());
    auto instants = traceDataCache_->GetInstantsData();

    auto internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, msg.pid());
    auto wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventTid_);
    instants->AppendInstantEventData(eventTimeStamp_, schedWakeupNewName_, internalTid, wakeupFromPid);
    streamFilters_->cpuFilter_->InsertWakeupEvent(eventTimeStamp_, internalTid);
    uint32_t targetCpu = msg.target_cpu();
    traceDataCache_->GetRawData()->AppendRawData(0, eventTimeStamp_, RAW_SCHED_WAKEUP, targetCpu, internalTid);
    return true;
}
bool HtraceEventParser::SchedWakingEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::SchedWakingFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_RECEIVED);
    uint32_t wakePidValue = msg.pid();
    auto instants = traceDataCache_->GetInstantsData();
    auto internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, wakePidValue);
    auto wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventTid_);
    streamFilters_->cpuFilter_->InsertWakeupEvent(eventTimeStamp_, internalTid);
    instants->AppendInstantEventData(eventTimeStamp_, schedWakingName_, internalTid, wakeupFromPid);
    uint32_t targetCpu = msg.target_cpu();
    traceDataCache_->GetRawData()->AppendRawData(0, eventTimeStamp_, RAW_SCHED_WAKING, targetCpu, wakeupFromPid);
    return true;
}
bool HtraceEventParser::CpuIdleEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_RECEIVED);
    ProtoReader::CpuIdleFormat_Reader msg(event.Data(), event.Size());
    std::optional<uint32_t> eventCpuValue = msg.cpu_id();
    std::optional<uint64_t> newStateValue = msg.state();
    if (!eventCpuValue.has_value()) {
        TS_LOGW("Failed to convert event cpu");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!newStateValue.has_value()) {
        TS_LOGW("Failed to convert state");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        return false;
    }

    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuIdleName_, eventTimeStamp_,
                                                            config_.GetStateValue(newStateValue.value()));

    // Add cpu_idle event to raw_data_table
    traceDataCache_->GetRawData()->AppendRawData(0, eventTimeStamp_, RAW_CPU_IDLE, eventCpuValue.value(), 0);
    return true;
}
bool HtraceEventParser::CpuFrequencyEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_RECEIVED);
    ProtoReader::CpuFrequencyFormat_Reader msg(event.Data(), event.Size());
    std::optional<uint64_t> newStateValue = msg.state();
    std::optional<uint32_t> eventCpuValue = msg.cpu_id();

    if (!newStateValue.has_value()) {
        TS_LOGW("Failed to convert state");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!eventCpuValue.has_value()) {
        TS_LOGW("Failed to convert event cpu");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        return false;
    }

    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuFrequencyName_, eventTimeStamp_,
                                                            newStateValue.value());
    return true;
}
bool HtraceEventParser::CpuFrequencyLimitsEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_RECEIVED);
    ProtoReader::CpuFrequencyLimitsFormat_Reader msg(event.Data(), event.Size());
    uint32_t maxFreq = msg.max_freq();
    uint32_t minFreq = msg.min_freq();
    uint32_t eventCpuValue = msg.cpu_id();
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue, cpuFrequencyLimitMaxNameId, eventTimeStamp_,
                                                            maxFreq);
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue, cpuFrequencyLimitMinNameId, eventTimeStamp_,
                                                            minFreq);
    return true;
}
bool HtraceEventParser::SuspendResumeEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SUSPEND_RESUME, STAT_EVENT_RECEIVED);
    ProtoReader::SuspendResumeFormat_Reader msg(event.Data(), event.Size());
    int32_t val = msg.val();
    uint32_t start = msg.start();
    std::string action = msg.action().ToStdString();
    UNUSED(val);
    UNUSED(start);
    UNUSED(action);
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SUSPEND_RESUME, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool HtraceEventParser::WorkqueueExecuteStartEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::WorkqueueExecuteStartFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_START, STAT_EVENT_RECEIVED);
    auto funcNameIndex = streamFilters_->symbolsFilter_->GetFunc(msg.function());
    size_t result = INVALID_UINT32;
    if (funcNameIndex == INVALID_UINT64) {
        std::string addrStr = "0x" + base::number(msg.function(), base::INTEGER_RADIX_TYPE_HEX);
        auto addStrIndex = traceDataCache_->GetDataIndex(addrStr);
        result = streamFilters_->sliceFilter_->BeginSlice(comm_, eventTimeStamp_, eventPid_, eventPid_, workQueueId_,
                                                          addStrIndex);
    } else {
        result = streamFilters_->sliceFilter_->BeginSlice(comm_, eventTimeStamp_, eventPid_, eventPid_, workQueueId_,
                                                          funcNameIndex);
    }

    traceDataCache_->GetInternalSlicesData()->AppendDistributeInfo();
    if (result == INVALID_UINT32) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TRACING_MARK_WRITE, STAT_EVENT_DATA_LOST);
    }
    return true;
}
bool HtraceEventParser::WorkqueueExecuteEndEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::WorkqueueExecuteEndFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_RECEIVED);
    if (streamFilters_->sliceFilter_->EndSlice(eventTimeStamp_, eventPid_, eventPid_, workQueueId_)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_NOTMATCH);
    }
    return true;
}
bool HtraceEventParser::ClockSetRateEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::ClockSetRateFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SET_RATE, STAT_EVENT_RECEIVED);
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clockRateFilter_->AppendNewMeasureData(msg.cpu_id(), nameIndex, eventTimeStamp_, msg.state());
    return true;
}
bool HtraceEventParser::ClockEnableEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_ENABLE, STAT_EVENT_RECEIVED);
    ProtoReader::ClockEnableFormat_Reader msg(event.Data(), event.Size());
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clockEnableFilter_->AppendNewMeasureData(msg.cpu_id(), nameIndex, eventTimeStamp_, msg.state());
    return true;
}
bool HtraceEventParser::ClockDisableEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_DISABLE, STAT_EVENT_RECEIVED);
    ProtoReader::ClockDisableFormat_Reader msg(event.Data(), event.Size());
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clockDisableFilter_->AppendNewMeasureData(msg.cpu_id(), nameIndex, eventTimeStamp_, msg.state());
    return true;
}
bool HtraceEventParser::ClkSetRateEvent(const ProtoReader::DataArea& event) const
{
    ProtoReader::ClkSetRateFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLK_SET_RATE, STAT_EVENT_RECEIVED);
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clkRateFilter_->AppendNewMeasureData(eventCpu_, nameIndex, eventTimeStamp_, msg.rate());
    return true;
}
bool HtraceEventParser::ClkEnableEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLK_ENABLE, STAT_EVENT_RECEIVED);
    ProtoReader::ClkEnableFormat_Reader msg(event.Data(), event.Size());
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clkEnableFilter_->AppendNewMeasureData(eventCpu_, nameIndex, eventTimeStamp_, 1);
    return true;
}
bool HtraceEventParser::ClkDisableEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLK_DISABLE, STAT_EVENT_RECEIVED);
    ProtoReader::ClkDisableFormat_Reader msg(event.Data(), event.Size());
    DataIndex nameIndex = traceDataCache_->GetDataIndex(msg.name().ToStdString());
    streamFilters_->clkDisableFilter_->AppendNewMeasureData(eventCpu_, nameIndex, eventTimeStamp_, 0);
    return true;
}

bool HtraceEventParser::IrqHandlerEntryEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_RECEIVED);
    ProtoReader::IrqHandlerEntryFormat_Reader msg(event.Data(), event.Size());
    // The tostdstring() here cannot use temporary variables, which will cause occasional garbled characters under wasm
    streamFilters_->irqFilter_->IrqHandlerEntry(eventTimeStamp_, eventCpu_,
                                                traceDataCache_->GetDataIndex(msg.name().ToStdString()));
    return true;
}
bool HtraceEventParser::IrqHandlerExitEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_RECEIVED);
    ProtoReader::IrqHandlerExitFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->irqFilter_->IrqHandlerExit(eventTimeStamp_, eventCpu_, msg.irq(), static_cast<uint32_t>(msg.ret()));
    return true;
}
bool HtraceEventParser::IpiHandlerEntryEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_ENTRY, STAT_EVENT_RECEIVED);
    ProtoReader::IpiEntryFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->irqFilter_->IpiHandlerEntry(eventTimeStamp_, eventCpu_,
                                                traceDataCache_->GetDataIndex(msg.reason().ToStdString()));
    return true;
}
bool HtraceEventParser::IpiHandlerExitEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_EXIT, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IpiHandlerExit(eventTimeStamp_, eventCpu_);
    return true;
}
bool HtraceEventParser::SoftIrqEntryEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_RECEIVED);
    ProtoReader::SoftirqEntryFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->irqFilter_->SoftIrqEntry(eventTimeStamp_, eventCpu_, static_cast<uint32_t>(msg.vec()));
    return true;
}
bool HtraceEventParser::SoftIrqRaiseEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool HtraceEventParser::SoftIrqExitEvent(const ProtoReader::DataArea& event) const
{
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_RECEIVED);
    ProtoReader::SoftirqExitFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->irqFilter_->SoftIrqExit(eventTimeStamp_, eventCpu_, static_cast<uint32_t>(msg.vec()));
    return true;
}
bool HtraceEventParser::SysEnterEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SYS_ENTRY, STAT_EVENT_RECEIVED);
    ProtoReader::SysEnterFormat_Reader msg(event.Data(), event.Size());
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventPid_);
    traceDataCache_->GetSysCallData()->AppendSysCallData(msg.id(), sysEnterName_, ipid, eventTimeStamp_, 0);
    return true;
}
bool HtraceEventParser::SysExitEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SYS_EXIT, STAT_EVENT_RECEIVED);
    ProtoReader::SysExitFormat_Reader msg(event.Data(), event.Size());
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventPid_);
    traceDataCache_->GetSysCallData()->AppendSysCallData(msg.id(), sysExitName_, ipid, eventTimeStamp_, msg.ret());
    return true;
}

bool HtraceEventParser::OomScoreAdjUpdate(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OOM_SCORE_ADJ_UPDATE, STAT_EVENT_RECEIVED);
    ProtoReader::OomScoreAdjUpdateFormat_Reader msg(event.Data(), event.Size());
    streamFilters_->processMeasureFilter_->AppendNewMeasureData(msg.pid(), oomScoreAdjName_, eventTimeStamp_,
                                                                msg.oom_score_adj());
    return true;
}

bool HtraceEventParser::SignalGenerateEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BLOCK_BIO_BACKMERGE, STAT_EVENT_RECEIVED);
    ProtoReader::SignalGenerateFormat_Reader msg(event.Data(), event.Size());
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(eventTimeStamp_, msg.pid(),
                                                                                           msg.comm().ToStdString());
    streamFilters_->threadFilter_->AppendNewMeasureData(internalTid, signalGenerateId_, eventTimeStamp_, msg.sig());
    return true;
}
bool HtraceEventParser::SignalDeleverEvent(const ProtoReader::DataArea& event) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BLOCK_BIO_BACKMERGE, STAT_EVENT_RECEIVED);
    ProtoReader::SignalDeliverFormat_Reader msg(event.Data(), event.Size());
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventPid_);
    streamFilters_->threadFilter_->AppendNewMeasureData(internalTid, signalDeliverId_, eventTimeStamp_, msg.sig());
    return true;
}
bool HtraceEventParser::InvokeFunc(const SupportedTraceEventType& eventType, const ProtoReader::DataArea& msgBase)
{
    auto eventName = config_.eventNameMap_.find(eventType);
    if (eventName == config_.eventNameMap_.end()) {
        // log warn
        streamFilters_->statFilter_->IncreaseStat(eventType, STAT_EVENT_NOTSUPPORTED);
        return false;
    }
    auto it = eventToFunctionMap_.find(eventName->second);
    if (it == eventToFunctionMap_.end()) {
        // log warn
        streamFilters_->statFilter_->IncreaseStat(eventType, STAT_EVENT_NOTSUPPORTED);
        return false;
    }
    it->second(msgBase);
    return true;
}
void HtraceEventParser::FilterAllEventsReader()
{
    size_t maxBuffSize = 1000 * 1000;
    size_t maxQueue = 2;

    if (htraceEventList_.size() < maxBuffSize * maxQueue) {
        return;
    }
    auto cmp = [](const std::unique_ptr<EventInfo>& a, const std::unique_ptr<EventInfo>& b) {
        return a->eventTimeStamp_ < b->eventTimeStamp_;
    };
#ifdef IS_WASM
    std::sort(htraceEventList_.begin(), htraceEventList_.end(), cmp);
#else
    std::stable_sort(htraceEventList_.begin(), htraceEventList_.end(), cmp);
#endif

    auto endOfList = htraceEventList_.begin() + maxBuffSize;
    for (auto itor = htraceEventList_.begin(); itor != endOfList; ++itor) {
        EventInfo* event = itor->get();
        ProtoReader::FtraceEvent_Reader ftraceEvent(event->ftraceEventBytes_);
        eventTimeStamp_ = event->eventTimeStamp_;
        eventCpu_ = event->eventCpu_;
        if (ftraceEvent.tgid() != INVALID_INT32) {
            eventPid_ = ftraceEvent.tgid();
            if (!pids_.count(eventPid_)) {
                pids_.insert(eventPid_);
            }
            streamFilters_->processFilter_->GetOrCreateThreadWithPid(eventPid_, eventPid_);
        }
        comm_ = ftraceEvent.comm().ToStdString();
        ProtoReaderDealEvent(ftraceEvent, itor);
    }
    htraceEventList_.erase(htraceEventList_.begin(), endOfList);
}
void HtraceEventParser::FilterAllEvents()
{
    auto cmp = [](const std::unique_ptr<EventInfo>& a, const std::unique_ptr<EventInfo>& b) {
        return a->eventTimeStamp_ < b->eventTimeStamp_;
    };
#ifdef IS_WASM
    std::sort(htraceEventList_.begin(), htraceEventList_.end(), cmp);
#else
    std::stable_sort(htraceEventList_.begin(), htraceEventList_.end(), cmp);
#endif
    size_t maxBuffSize = 1000 * 1000;

    while (htraceEventList_.size()) {
        int32_t size = std::min(maxBuffSize, htraceEventList_.size());
        auto endOfList = htraceEventList_.begin() + size;
        for (auto itor = htraceEventList_.begin(); itor != endOfList; itor++) {
            EventInfo* event = itor->get();
            ProtoReader::FtraceEvent_Reader ftraceEvent(event->ftraceEventBytes_);
            eventTimeStamp_ = event->eventTimeStamp_;
            eventCpu_ = event->eventCpu_;
            eventPid_ = ftraceEvent.tgid();
            comm_ = ftraceEvent.comm().ToStdString();
            ProtoReaderDealEvent(ftraceEvent, itor);
        }
        htraceEventList_.erase(htraceEventList_.begin(), endOfList);
    }
    htraceEventList_.clear();
    streamFilters_->cpuFilter_->Finish();
    traceDataCache_->dataDict_.Finish();
    traceDataCache_->UpdataZeroThreadInfo();
    if (traceDataCache_->AppStartTraceEnabled()) {
        streamFilters_->appStartupFilter_->FilterAllAPPStartupData();
    }
}

void HtraceEventParser::ProtoReaderDealEvent(const ProtoReader::FtraceEvent_Reader& ftraceEvent,
                                             const std::vector<std::unique_ptr<EventInfo>>::iterator& itor)
{
    ProtoReader::FtraceEvent_CommonFileds_Reader comonFields(ftraceEvent.common_fields().data_,
                                                             ftraceEvent.common_fields().size_);
    if (comonFields.pid() != INVALID_INT32) {
        eventTid_ = comonFields.pid();
        if (!tids_.count(eventTid_)) {
            tids_.insert(eventTid_);
        }
        streamFilters_->processFilter_->UpdateOrCreateThread(eventTimeStamp_, eventTid_);
    }
    if (eventTid_ != INVALID_INT32 && eventPid_ != INVALID_INT32) {
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(eventTid_, eventPid_);
    }
    DealEvent(ftraceEvent, comonFields);
    itor->reset();
}

void HtraceEventParser::Clear()
{
    const_cast<TraceStreamerFilters*>(streamFilters_)->FilterClear();
    streamFilters_->symbolsFilter_->Clear();
    streamFilters_->sysEventMemMeasureFilter_->Clear();
    streamFilters_->sysEventVMemMeasureFilter_->Clear();
    printEventParser_.Finish();
}
} // namespace TraceStreamer
} // namespace SysTuning
