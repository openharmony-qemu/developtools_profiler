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

#include "bytrace_event_parser.h"
#include "app_start_filter.h"
#include "binder_filter.h"
#include "cpu_filter.h"
#include "filter_filter.h"
#include "irq_filter.h"
#include "measure_filter.h"
#include "parting_string.h"
#include "process_filter.h"
#include "slice_filter.h"
#include "stat_filter.h"
#include "string_to_numerical.h"
#include "thread_state_flag.h"
#include "ts_common.h"
namespace SysTuning {
namespace TraceStreamer {
namespace {
std::string GetFunctionName(const std::string_view& text, const std::string_view& delimiter)
{
    std::string str("");
    if (delimiter.empty()) {
        return str;
    }

    std::size_t foundIndex = text.find(delimiter);
    if (foundIndex != std::string::npos) {
        std::size_t funIndex = foundIndex + delimiter.size();
        str = std::string(text.substr(funIndex, text.size() - funIndex));
    }
    return str;
}
} // namespace

BytraceEventParser::BytraceEventParser(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : EventParserBase(dataCache, filter), printEventParser_(traceDataCache_, streamFilters_)
{
    printEventParser_.SetTraceType(TRACE_FILETYPE_BY_TRACE);
    eventToFunctionMap_ = {
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_SWITCH),
         bind(&BytraceEventParser::SchedSwitchEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_BLOCKED_REASON),
         bind(&BytraceEventParser::BlockedReason, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_RENAME),
         bind(&BytraceEventParser::TaskRenameEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_TASK_NEWTASK),
         bind(&BytraceEventParser::TaskNewtaskEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_TRACING_MARK_WRITE),
         bind(&BytraceEventParser::TracingMarkWriteOrPrintEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_PRINT),
         bind(&BytraceEventParser::TracingMarkWriteOrPrintEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP),
         bind(&BytraceEventParser::SchedWakeupEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKING),
         bind(&BytraceEventParser::SchedWakingEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_IDLE),
         bind(&BytraceEventParser::CpuIdleEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY),
         bind(&BytraceEventParser::CpuFrequencyEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CPU_FREQUENCY_LIMITS),
         bind(&BytraceEventParser::CpuFrequencyLimitsEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_START),
         bind(&BytraceEventParser::WorkqueueExecuteStartEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_WORKQUEUE_EXECUTE_END),
         bind(&BytraceEventParser::WorkqueueExecuteEndEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_SET_RATE),
         bind(&BytraceEventParser::SetRateEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_ENABLE),
         bind(&BytraceEventParser::ClockEnableEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_CLOCK_DISABLE),
         bind(&BytraceEventParser::ClockDisableEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE),
         bind(&BytraceEventParser::RegulatorSetVoltageEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE),
         bind(&BytraceEventParser::RegulatorSetVoltageCompleteEvent, this, std::placeholders::_1,
              std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE),
         bind(&BytraceEventParser::RegulatorDisableEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE),
         bind(&BytraceEventParser::RegulatorDisableCompleteEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_ENTRY),
         bind(&BytraceEventParser::IpiEntryEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_IPI_EXIT),
         bind(&BytraceEventParser::IpiExitEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_ENTRY),
         bind(&BytraceEventParser::IrqHandlerEntryEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_IRQ_HANDLER_EXIT),
         bind(&BytraceEventParser::IrqHandlerExitEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_RAISE),
         bind(&BytraceEventParser::SoftIrqRaiseEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_ENTRY),
         bind(&BytraceEventParser::SoftIrqEntryEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SOFTIRQ_EXIT),
         bind(&BytraceEventParser::SoftIrqExitEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION),
         bind(&BytraceEventParser::BinderTransaction, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED),
         bind(&BytraceEventParser::BinderTransactionReceived, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF),
         bind(&BytraceEventParser::BinderTransactionAllocBufEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_SCHED_WAKEUP_NEW),
         bind(&BytraceEventParser::SchedWakeupEvent, this, std::placeholders::_1, std::placeholders::_2)},
        {config_.eventNameMap_.at(TRACE_EVENT_PROCESS_EXIT),
         bind(&BytraceEventParser::ProcessExitEvent, this, std::placeholders::_1, std::placeholders::_2)}};
}

bool BytraceEventParser::SchedSwitchEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_SCHED_SWITCH_ARGS_COUNT) {
        TS_LOGD("Failed to parse sched_switch event, no args or args size < 6");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto prevCommStr = std::string_view(args.at("prev_comm"));
    auto nextCommStr = std::string_view(args.at("next_comm"));
    auto prevPrioValue = base::StrToInt<int32_t>(args.at("prev_prio"));
    auto nextPrioValue = base::StrToInt<int32_t>(args.at("next_prio"));
    auto prevPidValue = base::StrToInt<uint32_t>(args.at("prev_pid"));
    auto nextPidValue = base::StrToInt<uint32_t>(args.at("next_pid"));
    DataIndex nextInfo = INVALID_DATAINDEX;
    auto nextInfoIt = args.find("next_info");
    if (nextInfoIt != args.end()) {
        nextInfo = traceDataCache_->GetDataIndex(std::string_view(args.at("next_info")));
    }
    if (!(prevPidValue.has_value() && prevPrioValue.has_value() && nextPidValue.has_value() &&
          nextPrioValue.has_value())) {
        TS_LOGD("Failed to parse sched_switch event");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto prevStateStr = args.at("prev_state");
    auto threadState = ThreadStateFlag(prevStateStr.c_str());
    uint64_t prevState = threadState.State();
    if (!threadState.IsValid()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_DATA_INVALID);
    }
    uint32_t nextInternalTid = 0;
    uint32_t uprevtid = 0;
    nextInternalTid =
        streamFilters_->processFilter_->UpdateOrCreateThreadWithName(line.ts, nextPidValue.value(), nextCommStr);

    if (!prevCommStr.empty()) {
        uprevtid =
            streamFilters_->processFilter_->UpdateOrCreateThreadWithName(line.ts, prevPidValue.value(), prevCommStr);
    } else {
        uprevtid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, prevPidValue.value());
    }
    streamFilters_->cpuFilter_->InsertSwitchEvent(
        line.ts, line.cpu, uprevtid, static_cast<uint64_t>(prevPrioValue.value()), prevState, nextInternalTid,
        static_cast<uint64_t>(nextPrioValue.value()), nextInfo);
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_SWITCH, STAT_EVENT_RECEIVED);
    return true;
}
bool BytraceEventParser::BlockedReason(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_BLOCKED_REASON_ARGS_COUNT) {
        TS_LOGD("Failed to parse blocked_reason event, no args or args size < %d", MIN_BLOCKED_REASON_ARGS_COUNT);
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto tid = base::StrToInt<int32_t>(args.at("pid"));
    auto iowaitIt = args.find("iowait");
    if (iowaitIt == args.end()) {
        iowaitIt = args.find("io_wait");
    }
    auto iowait = base::StrToInt<int32_t>(iowaitIt->second);
    uint32_t delayValue = INVALID_UINT32;
    if (args.find("delay") != args.end()) {
        auto delay = base::StrToInt<int32_t>(args.at("delay"));
        delayValue = delay.has_value() ? delay.value() : INVALID_UINT32;
    }
    auto caller = traceDataCache_->GetDataIndex(std::string_view(args.at("caller")));
    if (!(tid.has_value() && iowait.has_value())) {
        TS_LOGD("Failed to parse blocked_reason event");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto iTid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, tid.value());

    if (streamFilters_->cpuFilter_->InsertBlockedReasonEvent(line.ts, line.cpu, iTid, iowait.value(), caller,
                                                             delayValue)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_RECEIVED);
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_BLOCKED_REASON, STAT_EVENT_NOTMATCH);
    }
    return true;
}

bool BytraceEventParser::TaskRenameEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_TASK_RENAME_ARGS_COUNT) {
        TS_LOGD("Failed to parse task_rename event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_RENAME, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto prevCommStr = std::string_view(args.at("newcomm"));
    auto pidValue = base::StrToInt<uint32_t>(args.at("pid"));
    streamFilters_->processFilter_->UpdateOrCreateThreadWithName(line.ts, pidValue.value(), prevCommStr);
    return true;
}

bool BytraceEventParser::TaskNewtaskEvent(const ArgsMap& args, const BytraceLine& line) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_RECEIVED);
    // the clone flag from txt trace from kernel original is HEX, but when it is converted from proto
    // based trace, it will be OCT number, it is not stable, so we decide to ignore it
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TASK_NEWTASK, STAT_EVENT_NOTSUPPORTED);
    return true;
}

bool BytraceEventParser::TracingMarkWriteOrPrintEvent(const ArgsMap& args, const BytraceLine& line)
{
    UNUSED(args);
    return printEventParser_.ParsePrintEvent(line.task, line.ts, line.pid, line.argsStr.c_str(), line);
}
// prefer to use waking, unless no waking, can use wakeup
bool BytraceEventParser::SchedWakeupEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.size() < MIN_SCHED_WAKEUP_ARGS_COUNT) {
        TS_LOGD("Failed to parse SchedWakeupEvent event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_DATA_INVALID);
        return false;
    }
    std::optional<uint32_t> wakePidValue = base::StrToInt<uint32_t>(args.at("pid"));
    if (!wakePidValue.has_value()) {
        TS_LOGD("Failed to convert wake_pid");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto instants = traceDataCache_->GetInstantsData();
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, wakePidValue.value_or(0));
    streamFilters_->cpuFilter_->InsertWakeupEvent(line.ts, internalTid);

    InternalTid wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, line.pid);

    instants->AppendInstantEventData(line.ts, schedWakeupName_, internalTid, wakeupFromPid);
    std::optional<uint32_t> targetCpu = base::StrToInt<uint32_t>(args.at("target_cpu"));
    if (targetCpu.has_value()) {
        traceDataCache_->GetRawData()->AppendRawData(0, line.ts, RAW_SCHED_WAKEUP, targetCpu.value(), wakeupFromPid);
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKEUP, STAT_EVENT_RECEIVED);
    }
    return true;
}

bool BytraceEventParser::SchedWakingEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_SCHED_WAKING_ARGS_COUNT) {
        TS_LOGD("Failed to parse sched_waking event, no args or args size < 4");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_DATA_INVALID);
        return false;
    }
    std::optional<uint32_t> wakePidValue = base::StrToInt<uint32_t>(args.at("pid"));
    if (!wakePidValue.has_value()) {
        TS_LOGD("Failed to convert wake_pid");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto instants = traceDataCache_->GetInstantsData();
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, wakePidValue.value());
    DataIndex wakeByPidStrIndex = traceDataCache_->GetDataIndex(line.task);
    InternalTid internalTidWakeup =
        streamFilters_->processFilter_->UpdateOrCreateThreadWithNameIndex(line.ts, line.pid, wakeByPidStrIndex);
    InternalTid wakeupFromPid = streamFilters_->processFilter_->UpdateOrCreateThread(line.ts, line.pid);
    streamFilters_->cpuFilter_->InsertWakeupEvent(line.ts, internalTid, true);
    instants->AppendInstantEventData(line.ts, schedWakingName_, internalTid, wakeupFromPid);
    std::optional<uint32_t> targetCpu = base::StrToInt<uint32_t>(args.at("target_cpu"));
    if (targetCpu.has_value()) {
        traceDataCache_->GetRawData()->AppendRawData(0, line.ts, RAW_SCHED_WAKING, targetCpu.value(),
                                                     internalTidWakeup);
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SCHED_WAKING, STAT_EVENT_RECEIVED);
    }

    return true;
}

bool BytraceEventParser::CpuIdleEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_CPU_IDLE_ARGS_COUNT) {
        TS_LOGD("Failed to parse cpu_idle event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    std::optional<uint32_t> eventCpuValue = base::StrToInt<uint32_t>(args.at("cpu_id"));
    std::optional<int64_t> newStateValue = base::StrToInt<int64_t>(args.at("state"));
    if (!eventCpuValue.has_value()) {
        TS_LOGD("Failed to convert event cpu");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!newStateValue.has_value()) {
        TS_LOGD("Failed to convert state");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    // Add cpu_idle event to raw_data_table
    auto cpuidleNameIndex = traceDataCache_->GetDataIndex(line.eventName.c_str());
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuidleNameIndex, line.ts,
                                                            config_.GetStateValue(newStateValue.value()));
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_IDLE, STAT_EVENT_RECEIVED);
    return true;
}

bool BytraceEventParser::CpuFrequencyEvent(const ArgsMap& args, const BytraceLine& line) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_RECEIVED);
    if (args.empty() || args.size() < MIN_CPU_FREQUENCY_ARGS_COUNT) {
        TS_LOGD("Failed to parse cpu_frequency event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        return false;
    }
    std::optional<uint32_t> eventCpuValue = base::StrToInt<uint32_t>(args.at("cpu_id"));
    std::optional<int64_t> newStateValue = base::StrToInt<int64_t>(args.at("state"));

    if (!newStateValue.has_value()) {
        TS_LOGD("Failed to convert state");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!eventCpuValue.has_value()) {
        TS_LOGD("Failed to convert event cpu");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY, STAT_EVENT_DATA_INVALID);
        return false;
    }

    auto cpuidleNameIndex = traceDataCache_->GetDataIndex(line.eventName.c_str());
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuidleNameIndex, line.ts,
                                                            newStateValue.value());
    return true;
}
bool BytraceEventParser::CpuFrequencyLimitsEvent(const ArgsMap& args, const BytraceLine& line) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_RECEIVED);
    if (args.empty() || args.size() < MIN_CPU_FREQUENCY_ARGS_COUNT) {
        TS_LOGD("Failed to parse cpu_frequency event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_DATA_INVALID);
        return false;
    }
    std::optional<uint32_t> eventCpuValue = base::StrToInt<uint32_t>(args.at("cpu_id"));

    auto minIt = args.find("min");
    if (minIt == args.end()) {
        minIt = args.find("min_freq");
    }
    auto maxIt = args.find("max");
    if (maxIt == args.end()) {
        maxIt = args.find("max_freq");
    }
    std::optional<int64_t> minValue = base::StrToInt<int64_t>(minIt->second);
    std::optional<int64_t> maxValue = base::StrToInt<int64_t>(maxIt->second);

    if (!minValue.has_value()) {
        TS_LOGD("Failed to get frequency minValue");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!maxValue.has_value()) {
        TS_LOGD("Failed to get frequency maxValue");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (!eventCpuValue.has_value()) {
        TS_LOGD("Failed to get frequency cpu");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CPU_FREQUENCY_LIMITS, STAT_EVENT_DATA_INVALID);
        return false;
    }

    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuFrequencyLimitMaxNameId, line.ts,
                                                            maxValue.value());
    streamFilters_->cpuMeasureFilter_->AppendNewMeasureData(eventCpuValue.value(), cpuFrequencyLimitMinNameId, line.ts,
                                                            minValue.value());
    return true;
}

bool BytraceEventParser::WorkqueueExecuteStartEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    auto splitStr = GetFunctionName(line.argsStr, "function ");
    auto splitStrIndex = traceDataCache_->GetDataIndex(splitStr);
    size_t result =
        streamFilters_->sliceFilter_->BeginSlice(line.task, line.ts, line.pid, 0, workQueueId_, splitStrIndex);
    traceDataCache_->GetInternalSlicesData()->AppendDistributeInfo();
    if (result != INVALID_UINT32) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_START, STAT_EVENT_RECEIVED);
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_START, STAT_EVENT_DATA_LOST);
        return false;
    }
}

bool BytraceEventParser::WorkqueueExecuteEndEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    if (streamFilters_->sliceFilter_->EndSlice(line.ts, line.pid, 0, workQueueId_)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_RECEIVED);
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_WORKQUEUE_EXECUTE_END, STAT_EVENT_NOTMATCH);
        return false;
    }
}

bool BytraceEventParser::ProcessExitEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_PROCESS_EXIT_ARGS_COUNT) {
        TS_LOGD("Failed to parse process_exit event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto comm = std::string_view(args.at("comm"));
    auto pid = base::StrToInt<uint32_t>(args.at("pid"));
    if (!pid.has_value()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto itid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(line.ts, pid.value(), comm);
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_RECEIVED);
    if (streamFilters_->cpuFilter_->InsertProcessExitEvent(line.ts, line.cpu, itid)) {
        return true;
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_PROCESS_EXIT, STAT_EVENT_NOTMATCH);
        return false;
    }
}

bool BytraceEventParser::SetRateEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_CLOCK_SET_RATE_ARGS_COUNT) {
        TS_LOGD("Failed to parse clock_set_rate event, no args or args size < 3");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SET_RATE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto name = std::string_view(args.at("name"));
    auto state = base::StrToInt<int64_t>(args.at("state"));
    auto cpu = base::StrToInt<uint64_t>(args.at("cpu_id"));
    DataIndex nameIndex = traceDataCache_->GetDataIndex(name);
    streamFilters_->clockRateFilter_->AppendNewMeasureData(cpu.value(), nameIndex, line.ts, state.value());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SET_RATE, STAT_EVENT_RECEIVED);
    return true;
}

bool BytraceEventParser::ClockEnableEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_CLOCK_ENABLE_ARGS_COUNT) {
        TS_LOGD("Failed to parse clock_enable event, no args or args size < 3");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_ENABLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto name = std::string_view(args.at("name"));
    auto state = base::StrToInt<int64_t>(args.at("state"));
    auto cpuId = base::StrToInt<uint64_t>(args.at("cpu_id"));
    DataIndex nameIndex = traceDataCache_->GetDataIndex(name);
    streamFilters_->clockEnableFilter_->AppendNewMeasureData(cpuId.value(), nameIndex, line.ts, state.value());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_ENABLE, STAT_EVENT_RECEIVED);
    return true;
}
bool BytraceEventParser::ClockDisableEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_CLOCK_DISABLE_ARGS_COUNT) {
        TS_LOGD("Failed to parse clock_disable event, no args or args size < 3");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_DISABLE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    auto name = std::string_view(args.at("name"));
    auto state = base::StrToInt<int64_t>(args.at("state"));
    auto cpuId = base::StrToInt<uint64_t>(args.at("cpu_id"));
    DataIndex nameIndex = traceDataCache_->GetDataIndex(name);
    streamFilters_->clockDisableFilter_->AppendNewMeasureData(cpuId.value(), nameIndex, line.ts, state.value());
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_DISABLE, STAT_EVENT_RECEIVED);
    return true;
}

bool BytraceEventParser::RegulatorSetVoltageEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    UNUSED(line);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool BytraceEventParser::RegulatorSetVoltageCompleteEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    UNUSED(line);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE,
                                                    STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool BytraceEventParser::RegulatorDisableEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    UNUSED(line);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool BytraceEventParser::RegulatorDisableCompleteEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    UNUSED(line);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_REGULATOR_DISABLE_COMPLETE, STAT_EVENT_NOTSUPPORTED);
    return true;
}

bool BytraceEventParser::IpiEntryEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_ENTRY, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IpiHandlerEntry(line.ts, line.cpu, traceDataCache_->GetDataIndex(line.argsStr));
    return true;
}
bool BytraceEventParser::IpiExitEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IPI_EXIT, STAT_EVENT_RECEIVED);
    streamFilters_->irqFilter_->IpiHandlerExit(line.ts, line.cpu);
    return true;
}
bool BytraceEventParser::IrqHandlerEntryEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_IRQ_HANDLER_ENTRY_ARGS_COUNT) {
        TS_LOGD("Failed to parse irq_handler_entry event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_DATA_INVALID);
        return false;
    }
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_RECEIVED);
    auto name = std::string_view(args.at("name"));
    streamFilters_->irqFilter_->IrqHandlerEntry(line.ts, line.cpu, traceDataCache_->GetDataIndex(name));
    return true;
}
bool BytraceEventParser::IrqHandlerExitEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_IRQ_HANDLER_EXIT_ARGS_COUNT) {
        TS_LOGD("Failed to parse irq_handler_exit event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_DATA_INVALID);
        return false;
    }
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_RECEIVED);
    uint32_t ret = (args.at("ret") == "handled") ? 1 : 0;
    auto irq = base::StrToInt<uint32_t>(args.at("irq"));
    streamFilters_->irqFilter_->IrqHandlerExit(line.ts, line.cpu, irq.value(), ret);
    return true;
}
bool BytraceEventParser::SoftIrqRaiseEvent(const ArgsMap& args, const BytraceLine& line) const
{
    UNUSED(args);
    UNUSED(line);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_RECEIVED);
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_RAISE, STAT_EVENT_NOTSUPPORTED);
    return true;
}
bool BytraceEventParser::SoftIrqEntryEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_SOFTIRQ_ENTRY_ARGS_COUNT) {
        TS_LOGD("Failed to parse softirq_entry event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_DATA_INVALID);
        return false;
    }
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_RECEIVED);
    auto vec = base::StrToInt<uint32_t>(args.at("vec"));
    streamFilters_->irqFilter_->SoftIrqEntry(line.ts, line.cpu, vec.value());
    return true;
}
bool BytraceEventParser::SoftIrqExitEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_SOFTIRQ_EXIT_ARGS_COUNT) {
        TS_LOGD("Failed to parse softirq_exit event, no args or args size < 2");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_INVALID);
        return false;
    }
    traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_RECEIVED);
    auto vec = base::StrToInt<uint32_t>(args.at("vec"));
    streamFilters_->irqFilter_->SoftIrqExit(line.ts, line.cpu, vec.value());
    return true;
}

bool BytraceEventParser::BinderTransaction(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_BINDER_TRANSACTION_ARGS_COUNT) {
        TS_LOGD("Failed to parse binder_transaction event, no args or args size < 7");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION, STAT_EVENT_DATA_INVALID);
        return false;
    }
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION, STAT_EVENT_RECEIVED);
    auto transactionId = base::StrToInt<int64_t>(args.at("transaction"));
    auto destNode = base::StrToInt<uint32_t>(args.at("dest_node"));
    auto destProc = base::StrToInt<uint32_t>(args.at("dest_proc"));
    auto destThread = base::StrToInt<uint32_t>(args.at("dest_thread"));
    auto isReply = base::StrToInt<uint32_t>(args.at("reply"));
    auto flags = base::StrToInt<uint32_t>(args.at("flags"), base::INTEGER_RADIX_TYPE_HEX);
    auto codeStr = base::StrToInt<uint32_t>(args.at("code"), base::INTEGER_RADIX_TYPE_HEX);
    TS_LOGD("ts:%lu, pid:%u, destNode:%u, destTgid:%u, destTid:%u, transactionId:%lu, isReply:%u flags:%u, code:%u",
            line.ts, line.pid, destNode.value(), destProc.value(), destThread.value(), transactionId.value(),
            isReply.value(), flags.value(), codeStr.value());
    streamFilters_->binderFilter_->SendTraction(line.ts, line.pid, transactionId.value(), destNode.value(),
                                                destProc.value(), destThread.value(), isReply.value(), flags.value(),
                                                codeStr.value());
    if (traceDataCache_->BinderRunnableTraceEnabled() && transactionId.has_value() && flags.has_value() &&
        !streamFilters_->binderFilter_->IsAsync(flags.value())) {
        streamFilters_->cpuFilter_->InsertRunnableBinderEvent(transactionId.value(),
                                                              streamFilters_->processFilter_->GetInternalTid(line.pid));
    }
    return true;
}
bool BytraceEventParser::BinderTransactionReceived(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_BINDER_TRANSACTION_RECEIVED_ARGS_COUNT) {
        TS_LOGD("Failed to parse binder_transaction_received event, no args or args size < 1");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED, STAT_EVENT_DATA_INVALID);
        return false;
    }
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_RECEIVED, STAT_EVENT_RECEIVED);
    auto transactionId = base::StrToInt<int64_t>(args.at("transaction"));
    streamFilters_->binderFilter_->ReceiveTraction(line.ts, line.pid, transactionId.value());
    if (traceDataCache_->BinderRunnableTraceEnabled() && transactionId.has_value()) {
        streamFilters_->cpuFilter_->InsertRunnableBinderRecvEvent(
            transactionId.value(), streamFilters_->processFilter_->GetInternalTid(line.pid));
    }
    TS_LOGD("ts:%lu, pid:%u, transactionId:%lu", line.ts, line.pid, transactionId.value());
    return true;
}
bool BytraceEventParser::BinderTransactionAllocBufEvent(const ArgsMap& args, const BytraceLine& line) const
{
    if (args.empty() || args.size() < MIN_BINDER_TRANSACTION_ALLOC_BUF_ARGS_COUNT) {
        TS_LOGD("Failed to parse binder_transaction_alloc_buf event, no args or args size < 3");
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF, STAT_EVENT_DATA_INVALID);
        return false;
    }
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF, STAT_EVENT_RECEIVED);
    auto dataSize = base::StrToInt<uint64_t>(args.at("data_size"));
    auto offsetsSize = base::StrToInt<uint64_t>(args.at("offsets_size"));
    streamFilters_->binderFilter_->TransactionAllocBuf(line.ts, line.pid, dataSize.value(), offsetsSize.value());
    TS_LOGD("dataSize:%lu, offsetSize:%lu", dataSize.value(), offsetsSize.value());
    return true;
}
void BytraceEventParser::ParseDataItem(const BytraceLine& line)
{
    eventList_.push_back(std::make_unique<EventInfo>(line.ts, std::move(line)));
    return;
}
void BytraceEventParser::GetDataSegArgs(BytraceLine& bufLine, ArgsMap& args, uint32_t& tgid) const
{
    if (bufLine.tGidStr.size() && bufLine.tGidStr.at(0) != '-') {
        tgid = base::StrToInt<uint32_t>(bufLine.tGidStr).value_or(0);
    } else {
        tgid = 0;
    }
    bufLine.tgid = tgid;

    for (base::PartingString ss(bufLine.argsStr, ' '); ss.Next();) {
        std::string key;
        std::string value;
        if (!(std::string(ss.GetCur()).find("=") != std::string::npos)) {
            key = "name";
            value = ss.GetCur();
            args.emplace(std::move(key), std::move(value));
            continue;
        }
        for (base::PartingString inner(ss.GetCur(), '='); inner.Next();) {
            if (key.empty()) {
                key = inner.GetCur();
            } else {
                value = inner.GetCur();
            }
        }
        args.emplace(std::move(key), std::move(value));
    }
}
void BytraceEventParser::FilterAllEventsTemp()
{
    size_t maxBuffSize = 1000 * 1000;
    size_t maxQueue = 2;
    if (eventList_.size() < maxBuffSize * maxQueue) {
        return;
    }
    auto cmp = [](const std::unique_ptr<EventInfo>& a, const std::unique_ptr<EventInfo>& b) {
        return a->eventTimestamp < b->eventTimestamp;
    };
#ifdef IS_WASM
    std::sort(eventList_.begin(), eventList_.end(), cmp);
#else
    std::stable_sort(eventList_.begin(), eventList_.end(), cmp);
#endif
    auto endOfList = eventList_.begin() + maxBuffSize;
    for (auto itor = eventList_.begin(); itor != endOfList; itor++) {
        EventInfo* event = itor->get();
        BeginFilterEvents(event);
        itor->reset();
    }
    eventList_.erase(eventList_.begin(), endOfList);
}

void BytraceEventParser::FilterAllEvents()
{
    auto cmp = [](const std::unique_ptr<EventInfo>& a, const std::unique_ptr<EventInfo>& b) {
        return a->eventTimestamp < b->eventTimestamp;
    };
#ifdef IS_WASM
    std::sort(eventList_.begin(), eventList_.end(), cmp);
#else
    std::stable_sort(eventList_.begin(), eventList_.end(), cmp);
#endif
    size_t maxBuffSize = 1000 * 1000;
    while (eventList_.size()) {
        int32_t size = std::min(maxBuffSize, eventList_.size());
        auto endOfList = eventList_.begin() + size;
        for (auto itor = eventList_.begin(); itor != endOfList; itor++) {
            EventInfo* event = itor->get();
            BeginFilterEvents(event);
            itor->reset();
        }
        eventList_.erase(eventList_.begin(), endOfList);
    }
    eventList_.clear();
    streamFilters_->cpuFilter_->Finish();
    traceDataCache_->dataDict_.Finish();
    traceDataCache_->UpdataZeroThreadInfo();
    if (traceDataCache_->AppStartTraceEnabled()) {
        streamFilters_->appStartupFilter_->FilterAllAPPStartupData();
    }
}

void BytraceEventParser::BeginFilterEvents(EventInfo* event)
{
    auto it = eventToFunctionMap_.find(event->line.eventName);
    if (it != eventToFunctionMap_.end()) {
        uint32_t tgid;
        ArgsMap args;
        GetDataSegArgs(event->line, args, tgid);
        if (tgid) {
            streamFilters_->processFilter_->UpdateOrCreateThreadWithPidAndName(event->line.pid, tgid, event->line.task);
        } else {
            // When tgid is zero, only use tid create thread
            streamFilters_->processFilter_->GetOrCreateThreadWithPid(event->line.pid, tgid);
        }
        if (it->second(args, event->line)) {
            traceDataCache_->UpdateTraceTime(event->line.ts);
        }
    } else {
        traceDataCache_->GetStatAndInfo()->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_NOTSUPPORTED);
        TS_LOGD("UnRecognizable event name:%s", event->line.eventName.c_str());
    }
}

void BytraceEventParser::Clear()
{
    const_cast<TraceStreamerFilters*>(streamFilters_)->FilterClear();
    printEventParser_.Finish();
}
} // namespace TraceStreamer
} // namespace SysTuning
