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
#include "ftrace_event_processor.h"
#include "log.h"

namespace SysTuning {
namespace TraceStreamer {
void SchedSwitch(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto schedSwitchMsg = ftraceEvent.mutable_sched_switch_format();
    schedSwitchMsg->set_prev_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    schedSwitchMsg->set_prev_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedSwitchMsg->set_prev_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedSwitchMsg->set_prev_state(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    schedSwitchMsg->set_next_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    schedSwitchMsg->set_next_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedSwitchMsg->set_next_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SchedBlockedReason(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto schedBlockedMsg = ftraceEvent.mutable_sched_blocked_reason_format();
    schedBlockedMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedBlockedMsg->set_caller(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    schedBlockedMsg->set_io_wait(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void SchedWakeup(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto schedWakeupMsg = ftraceEvent.mutable_sched_wakeup_format();
    schedWakeupMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    schedWakeupMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakeupMsg->set_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakeupMsg->set_success(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakeupMsg->set_target_cpu(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SchedWaking(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto schedWakingMsg = ftraceEvent.mutable_sched_waking_format();
    schedWakingMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    schedWakingMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakingMsg->set_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakingMsg->set_success(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    schedWakingMsg->set_target_cpu(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SchedWakeupNew(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto wakeupNewMsg = ftraceEvent.mutable_sched_wakeup_new_format();
    wakeupNewMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    wakeupNewMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    wakeupNewMsg->set_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    wakeupNewMsg->set_success(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    wakeupNewMsg->set_target_cpu(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SchedProcessExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto processExitMsg = ftraceEvent.mutable_sched_process_exit_format();
    processExitMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    processExitMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    processExitMsg->set_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SchedProcessFree(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto processFreeMsg = ftraceEvent.mutable_sched_process_free_format();
    processFreeMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    processFreeMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    processFreeMsg->set_prio(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}

void BinderTransaction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionMsg = ftraceEvent.mutable_binder_transaction_format();
    transactionMsg->set_debug_id(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionMsg->set_target_node(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionMsg->set_to_proc(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionMsg->set_to_thread(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionMsg->set_reply(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionMsg->set_code(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
    transactionMsg->set_flags(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void BinderTransactionReceived(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionReceivedMsg = ftraceEvent.mutable_binder_transaction_received_format();
    transactionReceivedMsg->set_debug_id(
        FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void BinderTransactionAllocBuf(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionAllocBufMsg = ftraceEvent.mutable_binder_transaction_alloc_buf_format();
    transactionAllocBufMsg->set_debug_id(
        FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    transactionAllocBufMsg->set_data_size(
        FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    transactionAllocBufMsg->set_offsets_size(
        FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    transactionAllocBufMsg->set_extra_buffers_size(
        FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}
void BinderTransactionAllocLock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionLockMsg = ftraceEvent.mutable_binder_lock_format();
    transactionLockMsg->set_tag(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void BinderTransactionAllocLocked(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionLockedMsg = ftraceEvent.mutable_binder_locked_format();
    transactionLockedMsg->set_tag(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void BinderTransactionAllocUnlock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto transactionUnlockMsg = ftraceEvent.mutable_binder_unlock_format();
    transactionUnlockMsg->set_tag(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}

void TaskRename(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto taskRenameMsg = ftraceEvent.mutable_task_rename_format();
    taskRenameMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    taskRenameMsg->set_oldcomm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    taskRenameMsg->set_newcomm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    taskRenameMsg->set_oom_score_adj(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void TaskNewtask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto newTaskMsg = ftraceEvent.mutable_task_newtask_format();
    newTaskMsg->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    newTaskMsg->set_comm(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    newTaskMsg->set_clone_flags(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    newTaskMsg->set_oom_score_adj(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}

void TracingMarkWriteOrPrintFormat(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto printMsg = ftraceEvent.mutable_print_format();
    printMsg->set_ip(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    printMsg->set_buf(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}

void CpuIdle(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto cpuIdleMsg = ftraceEvent.mutable_cpu_idle_format();
    cpuIdleMsg->set_state(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
    cpuIdleMsg->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void CpuFrequency(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto cpuFrequencyMsg = ftraceEvent.mutable_cpu_frequency_format();
    cpuFrequencyMsg->set_state(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
    cpuFrequencyMsg->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void CpuFrequencyLimits(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto frequencyLimitsMsg = ftraceEvent.mutable_cpu_frequency_limits_format();
    frequencyLimitsMsg->set_min_freq(
        FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
    frequencyLimitsMsg->set_max_freq(
        FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
    frequencyLimitsMsg->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}

void SuspendResume(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto resumeMsg = ftraceEvent.mutable_suspend_resume_format();
    resumeMsg->set_action(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    resumeMsg->set_val(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    resumeMsg->set_start(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void WorkqueueExecuteStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto executeStartMsg = ftraceEvent.mutable_workqueue_execute_start_format();
    executeStartMsg->set_work(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    executeStartMsg->set_function(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}
void WorkqueueExecuteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto executeEndMsg = ftraceEvent.mutable_workqueue_execute_end_format();
    executeEndMsg->set_work(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}

void IpiEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto ipiEntryMsg = ftraceEvent.mutable_ipi_entry_format();
    ipiEntryMsg->set_reason(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void IpiExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto ipiExitMsg = ftraceEvent.mutable_ipi_exit_format();
    ipiExitMsg->set_reason(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void IrqHandlerEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto handlerEntryMsg = ftraceEvent.mutable_irq_handler_entry_format();
    handlerEntryMsg->set_irq(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    handlerEntryMsg->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void IrqHandlerExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto handlerExitMsg = ftraceEvent.mutable_irq_handler_exit_format();
    handlerExitMsg->set_irq(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    handlerExitMsg->set_ret(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void SoftirqRaise(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto softirqRaiseMsg = ftraceEvent.mutable_softirq_raise_format();
    softirqRaiseMsg->set_vec(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void SoftirqEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto softirqEntryMsg = ftraceEvent.mutable_softirq_entry_format();
    softirqEntryMsg->set_vec(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void SoftirqExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto softirqExitMsg = ftraceEvent.mutable_softirq_exit_format();
    softirqExitMsg->set_vec(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void ClockSetRate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto clockSetRateMsg = ftraceEvent.mutable_clock_set_rate_format();
    clockSetRateMsg->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    clockSetRateMsg->set_state(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    clockSetRateMsg->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}
void ClockEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto clockEnable = ftraceEvent.mutable_clock_enable_format();
    clockEnable->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    clockEnable->set_state(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    clockEnable->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}
void ClockDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto clockDisable = ftraceEvent.mutable_clock_disable_format();
    clockDisable->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    clockDisable->set_state(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
    clockDisable->set_cpu_id(FtraceFieldProcessor::HandleIntField<uint64_t>(format.fields, index++, data, size));
}
void RegulatorSetVoltage(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto regulatorSetVoltage = ftraceEvent.mutable_regulator_set_voltage_format();
    regulatorSetVoltage->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    regulatorSetVoltage->set_min(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
    regulatorSetVoltage->set_max(FtraceFieldProcessor::HandleIntField<int32_t>(format.fields, index++, data, size));
}
void RegulatorSetVoltageComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto regulatorSetVoltage = ftraceEvent.mutable_regulator_set_voltage_complete_format();
    regulatorSetVoltage->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
    regulatorSetVoltage->set_val(FtraceFieldProcessor::HandleIntField<uint32_t>(format.fields, index++, data, size));
}
void RegulatorDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto regulatorDisable = ftraceEvent.mutable_regulator_disable_format();
    regulatorDisable->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}
void RegulatorDisableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format)
{
    uint8_t index = 0;
    auto regulatorDisableComplete = ftraceEvent.mutable_regulator_disable_complete_format();
    regulatorDisableComplete->set_name(FtraceFieldProcessor::HandleStrField(format.fields, index++, data, size));
}

FtraceEventProcessor& FtraceEventProcessor::GetInstance()
{
    static FtraceEventProcessor instance;
    return instance;
}

FtraceEventProcessor::FtraceEventProcessor()
{
    eventNameToFunctions_ = {
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_ENTRY), IpiEntry},
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_EXIT), IpiExit},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_ENTRY), IrqHandlerEntry},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_EXIT), IrqHandlerExit},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_RAISE), SoftirqRaise},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_ENTRY), SoftirqEntry},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_EXIT), SoftirqExit},
        {config_.eventNameMap_.at(TRACE_EVENT_SUSPEND_RESUME), SuspendResume},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_START), WorkqueueExecuteStart},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_END), WorkqueueExecuteEnd},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_IDLE), CpuIdle},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY), CpuFrequency},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY_LIMITS), CpuFrequencyLimits},
        {config_.eventNameMap_.at(TRACE_EVENT_PRINT), TracingMarkWriteOrPrintFormat},
        {config_.eventNameMap_.at(TRACE_EVENT_TRACING_MARK_WRITE), TracingMarkWriteOrPrintFormat},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_RENAME), TaskRename},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_NEWTASK), TaskNewtask},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION), BinderTransaction},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED), BinderTransactionReceived},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF), BinderTransactionAllocBuf},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCK), BinderTransactionAllocLock},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_LOCKED), BinderTransactionAllocLocked},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_UNLOCK), BinderTransactionAllocUnlock},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_SWITCH), SchedSwitch},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_BLOCKED_REASON), SchedBlockedReason},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP), SchedWakeup},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKING), SchedWaking},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP_NEW), SchedWakeupNew},
        {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_EXIT), SchedProcessExit},
        {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_FREE), SchedProcessFree},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_SET_RATE), ClockSetRate},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_ENABLE), ClockEnable},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_DISABLE), ClockDisable},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE), RegulatorSetVoltage},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE), RegulatorSetVoltageComplete},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE), RegulatorDisable},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE), RegulatorDisableComplete},
    };
}

FtraceEventProcessor::~FtraceEventProcessor()
{
    TS_LOGI("FtraceEventProcessor destroy!");
}

bool FtraceEventProcessor::IsSupported(uint32_t eventId) const
{
    return eventIdToFunctions_.count(eventId);
}

bool FtraceEventProcessor::IsSupported(const std::string& eventName) const
{
    return eventNameToFunctions_.count(eventName) > 0;
}

bool FtraceEventProcessor::HandleEvent(FtraceEvent& event, uint8_t data[], size_t size, const EventFormat& format) const
{
    auto iter = eventIdToFunctions_.find(format.eventId);
    TS_CHECK_TRUE_RET(iter != eventIdToFunctions_.end(), false);
    iter->second(event, data, size, format);
    return true;
}

bool FtraceEventProcessor::SetupEvent(const EventFormat& format)
{
    auto it = eventNameToFunctions_.find(format.eventName);
    TS_CHECK_TRUE_RET(it != eventNameToFunctions_.end(), false);
    eventIdToFunctions_[format.eventId] = it->second;
    eventIdToNames_[format.eventId] = format.eventName;
    return true;
}
const std::string& FtraceEventProcessor::GetEventNameById(uint32_t eventId)
{
    auto iter = eventIdToNames_.find(eventId);
    TS_CHECK_TRUE_RET(iter != eventIdToNames_.end(), INVALID_STRING);
    return iter->second;
}
} // namespace TraceStreamer
} // namespace SysTuning
