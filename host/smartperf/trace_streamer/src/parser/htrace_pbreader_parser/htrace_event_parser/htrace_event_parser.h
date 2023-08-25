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
#ifndef HTRACE_EVENT_PARSER_H
#define HTRACE_EVENT_PARSER_H
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "event_parser_base.h"
#include "ftrace_event.pbreader.h"
#include "google/protobuf/message_lite.h"
#include "log.h"
#include "print_event_parser.h"
#include "trace_data/trace_data_cache.h"
#include "trace_plugin_result.pbreader.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace google::protobuf;
class HtraceEventParser : private EventParserBase {
public:
    HtraceEventParser(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~HtraceEventParser();
    void ParseDataItem(HtraceDataSegment& tracePacket, BuiltinClocks clock);
    void FilterAllEventsReader();
    void FilterAllEvents();
    void Clear();

private:
    void DealEvent(const ProtoReader::FtraceEvent_Reader& event,
                   const ProtoReader::FtraceEvent_CommonFileds_Reader& comonFields);
    bool BinderTractionEvent(const ProtoReader::DataArea& event) const;
    bool BinderTractionReceivedEvent(const ProtoReader::DataArea& event) const;
    bool BinderTractionAllocBufEvent(const ProtoReader::DataArea& event) const;
    bool BinderTractionLockEvent(const ProtoReader::DataArea& event) const;
    bool BinderTractionLockedEvent(const ProtoReader::DataArea& event) const;
    bool BinderTractionUnLockEvent(const ProtoReader::DataArea& event) const;
    bool SchedSwitchEvent(const ProtoReader::DataArea& event);
    bool SchedBlockReasonEvent(const ProtoReader::DataArea& event);
    bool ProcessExitEvent(const ProtoReader::DataArea& event) const;
    bool ProcessFreeEvent(const ProtoReader::DataArea& event) const;
    bool TaskRenameEvent(const ProtoReader::DataArea& event) const;
    bool TaskNewtaskEvent(const ProtoReader::DataArea& event) const;
    bool ParsePrintEvent(const ProtoReader::DataArea& event);
    bool SchedWakeupEvent(const ProtoReader::DataArea& event) const;
    bool SchedWakeupNewEvent(const ProtoReader::DataArea& event) const;
    bool SchedWakingEvent(const ProtoReader::DataArea& event) const;
    bool CpuIdleEvent(const ProtoReader::DataArea& event) const;
    bool CpuFrequencyEvent(const ProtoReader::DataArea& event) const;
    bool CpuFrequencyLimitsEvent(const ProtoReader::DataArea& event) const;
    bool SuspendResumeEvent(const ProtoReader::DataArea& event) const;
    bool WorkqueueExecuteStartEvent(const ProtoReader::DataArea& event) const;
    bool WorkqueueExecuteEndEvent(const ProtoReader::DataArea& event) const;
    bool ClockSetRateEvent(const ProtoReader::DataArea& event) const;
    bool ClockEnableEvent(const ProtoReader::DataArea& event) const;
    bool ClockDisableEvent(const ProtoReader::DataArea& event) const;
    bool ClkSetRateEvent(const ProtoReader::DataArea& event) const;
    bool ClkEnableEvent(const ProtoReader::DataArea& event) const;
    bool ClkDisableEvent(const ProtoReader::DataArea& event) const;
    bool IrqHandlerEntryEvent(const ProtoReader::DataArea& event) const;
    bool IrqHandlerExitEvent(const ProtoReader::DataArea& event) const;
    bool IpiHandlerEntryEvent(const ProtoReader::DataArea& event) const;
    bool IpiHandlerExitEvent(const ProtoReader::DataArea& event) const;
    bool SoftIrqEntryEvent(const ProtoReader::DataArea& event) const;
    bool SoftIrqRaiseEvent(const ProtoReader::DataArea& event) const;
    bool SoftIrqExitEvent(const ProtoReader::DataArea& event) const;
    bool SysEnterEvent(const ProtoReader::DataArea& event) const;
    bool SysExitEvent(const ProtoReader::DataArea& event) const;
    bool OomScoreAdjUpdate(const ProtoReader::DataArea& event) const;
    bool SignalGenerateEvent(const ProtoReader::DataArea& event) const;
    bool SignalDeleverEvent(const ProtoReader::DataArea& event) const;
    bool InvokeFunc(const SupportedTraceEventType& eventType, const ProtoReader::DataArea& msgBase);
    class EventInfo {
    public:
        EventInfo(uint64_t eventTimestamp,
                  uint32_t eventCpu,
                  std::shared_ptr<std::string> cpuDetail,
                  ProtoReader::BytesView ftraceEventBytes)
            : eventTimeStamp_(eventTimestamp),
              eventCpu_(eventCpu),
              cpuDetail_(std::move(cpuDetail)),
              ftraceEventBytes_(ftraceEventBytes)
        {
        }
        uint64_t eventTimeStamp_;
        uint32_t eventCpu_;
        std::shared_ptr<std::string> cpuDetail_;
        ProtoReader::BytesView ftraceEventBytes_;
    };
    using FuncCall = std::function<bool(const ProtoReader::DataArea& event)>;
    uint32_t eventCpu_ = INVALID_UINT32;
    uint64_t eventTimeStamp_ = INVALID_UINT64;
    std::string comm_ = "";
    uint32_t eventPid_ = INVALID_UINT32;
    uint32_t eventTid_ = INVALID_UINT32;
    std::map<std::string, FuncCall> eventToFunctionMap_ = {};
    std::unordered_set<uint32_t> tids_ = {};
    std::unordered_set<uint32_t> pids_ = {};
    DataIndex workQueueId_ = 0;
    PrintEventParser printEventParser_;
    uint64_t lastOverwrite_ = 0;
    uint64_t ftraceStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t ftraceEndTime_ = 0;
    uint64_t ftraceOriginStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t ftraceOriginEndTime_ = 0;
    void ProtoReaderDealEvent(const ProtoReader::FtraceEvent_Reader& ftraceEvent,
                              const std::vector<std::unique_ptr<EventInfo>>::iterator& itor);
    std::vector<std::unique_ptr<EventInfo>> htraceEventList_ = {};
    const DataIndex signalGenerateId_ = traceDataCache_->GetDataIndex("signal_generate");
    const DataIndex signalDeliverId_ = traceDataCache_->GetDataIndex("signal_deliver");
    const DataIndex schedWakeupName_ = traceDataCache_->GetDataIndex("sched_wakeup");
    const DataIndex schedWakingName_ = traceDataCache_->GetDataIndex("sched_waking");
    const DataIndex schedWakeupNewName_ = traceDataCache_->GetDataIndex("sched_wakeup_new");
    const DataIndex cpuIdleName_ = traceDataCache_->GetDataIndex("cpu_idle");
    const DataIndex cpuFrequencyName_ = traceDataCache_->GetDataIndex("cpu_frequency");
    const DataIndex cpuFrequencyLimitMaxNameId = traceDataCache_->GetDataIndex("cpu_frequency_limits_max");
    const DataIndex cpuFrequencyLimitMinNameId = traceDataCache_->GetDataIndex("cpu_frequency_limits_min");
    const DataIndex sysEnterName_ = traceDataCache_->GetDataIndex("sys_enter");
    const DataIndex sysExitName_ = traceDataCache_->GetDataIndex("sys_exit");
    const DataIndex oomScoreAdjName_ = traceDataCache_->GetDataIndex("oom_score_adj");
    TraceStreamerConfig config_{};
    BuiltinClocks clock_ = TS_CLOCK_BOOTTIME;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_EVENT_PARSER_H_
