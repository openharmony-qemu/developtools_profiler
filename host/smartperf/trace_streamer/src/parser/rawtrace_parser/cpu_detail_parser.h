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
#ifndef CPU_DETAIL_PARSER_H
#define CPU_DETAIL_PARSER_H
#include "print_event_parser.h"
#include "trace_data_cache.h"
#include "trace_plugin_result.pb.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
struct RawTraceEventInfo {
    uint8_t cpuId = INVALID_UINT8;
    uint32_t eventId = INVALID_UINT32;
    std::unique_ptr<FtraceEvent> msgPtr;
};
class CpuDetailParser {
public:
    CpuDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~CpuDetailParser() = default;
    void EventAppend(std::unique_ptr<RawTraceEventInfo> event);
    bool FilterAllEvents(FtraceCpuDetailMsg& cpuDetail, bool isFinished = false);
    void Clear();

private:
    void DealEvent(const RawTraceEventInfo& event);
    bool SchedSwitchEvent(const RawTraceEventInfo& event);
    bool SchedBlockReasonEvent(const RawTraceEventInfo& event);
    bool SchedWakeupEvent(const RawTraceEventInfo& event) const;
    bool SchedWakingEvent(const RawTraceEventInfo& event) const;
    bool SchedWakeupNewEvent(const RawTraceEventInfo& event) const;
    bool ProcessExitEvent(const RawTraceEventInfo& event) const;
    bool ProcessFreeEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionReceivedEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionAllocBufEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionLockEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionLockedEvent(const RawTraceEventInfo& event) const;
    bool BinderTractionUnLockEvent(const RawTraceEventInfo& event) const;
    bool TaskRenameEvent(const RawTraceEventInfo& event) const;
    bool TaskNewtaskEvent(const RawTraceEventInfo& event) const;
    bool ParseTracingMarkWriteOrPrintEvent(const RawTraceEventInfo& event);
    bool CpuIdleEvent(const RawTraceEventInfo& event) const;
    bool CpuFrequencyEvent(const RawTraceEventInfo& event) const;
    bool CpuFrequencyLimitsEvent(const RawTraceEventInfo& event) const;
    bool SuspendResumeEvent(const RawTraceEventInfo& event) const;
    bool WorkqueueExecuteStartEvent(const RawTraceEventInfo& event) const;
    bool WorkqueueExecuteEndEvent(const RawTraceEventInfo& event) const;
    bool IrqHandlerEntryEvent(const RawTraceEventInfo& event) const;
    bool IrqHandlerExitEvent(const RawTraceEventInfo& event) const;
    bool IpiHandlerEntryEvent(const RawTraceEventInfo& event) const;
    bool IpiHandlerExitEvent(const RawTraceEventInfo& event) const;
    bool SoftIrqEntryEvent(const RawTraceEventInfo& event) const;
    bool SoftIrqRaiseEvent(const RawTraceEventInfo& event) const;
    bool SoftIrqExitEvent(const RawTraceEventInfo& event) const;
    bool SetRateEvent(const RawTraceEventInfo& event) const;
    bool ClockEnableEvent(const RawTraceEventInfo& event) const;
    bool ClockDisableEvent(const RawTraceEventInfo& event) const;
    bool RegulatorSetVoltageEvent(const RawTraceEventInfo& event) const;
    bool RegulatorSetVoltageCompleteEvent(const RawTraceEventInfo& event) const;
    bool RegulatorDisableEvent(const RawTraceEventInfo& event) const;
    bool RegulatorDisableCompleteEvent(const RawTraceEventInfo& event) const;

private:
    using FuncCall = std::function<bool(const RawTraceEventInfo& event)>;
    uint32_t eventPid_ = INVALID_UINT32;
    uint32_t eventTid_ = INVALID_UINT32;
    uint64_t lastOverwrite_ = 0;
    PrintEventParser printEventParser_;
    std::deque<std::unique_ptr<RawTraceEventInfo>> rawTraceEventList_ = {};
    std::map<std::string, FuncCall> eventToFunctionMap_ = {};
    const TraceStreamerFilters* streamFilters_;
    TraceDataCache* traceDataCache_;
    TraceStreamerConfig config_{};
    const BuiltinClocks clock_ = TS_CLOCK_BOOTTIME;
    const DataIndex schedWakeupIndex_ = traceDataCache_->GetDataIndex("sched_wakeup");
    const DataIndex schedWakingIndex_ = traceDataCache_->GetDataIndex("sched_waking");
    const DataIndex schedWakeupNewIndex_ = traceDataCache_->GetDataIndex("sched_wakeup_new");
    const DataIndex cpuIdleIndex_ = traceDataCache_->GetDataIndex("cpu_idle");
    const DataIndex cpuFrequencyIndex_ = traceDataCache_->GetDataIndex("cpu_frequency");
    const DataIndex cpuFrequencyLimitMaxIndex_ = traceDataCache_->GetDataIndex("cpu_frequency_limits_max");
    const DataIndex cpuFrequencyLimitMinIndex_ = traceDataCache_->GetDataIndex("cpu_frequency_limits_min");
    const DataIndex workQueueIndex_ = traceDataCache_->GetDataIndex("workqueue");
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // CPU_DETAIL_PARSER_H_
