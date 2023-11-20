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
#ifndef FTRACE_EVENT_PROCESSOR_H
#define FTRACE_EVENT_PROCESSOR_H
#include <cstdint>
#include <functional>
#include <map>
#include "ftrace_field_processor.h"
#include "trace_plugin_result.pb.h"
#include "trace_streamer_config.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace TraceCfg;
class FtraceEventProcessor {
public:
    static FtraceEventProcessor& GetInstance();
    bool SetupEvent(const EventFormat& format);
    bool IsSupported(uint32_t eventId) const;
    bool IsSupported(const std::string& eventName) const;
    bool HandleEvent(FtraceEvent& event, uint8_t data[], size_t size, const EventFormat& format) const;
    const std::string& GetEventNameById(uint32_t eventId);

private:
    using HandleFunction = std::function<bool(FtraceEvent&, uint8_t[], size_t, const EventFormat&)>;
    bool IpiEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool IpiExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool IrqHandlerEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool IrqHandlerExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SoftirqRaise(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SoftirqEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SoftirqExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SuspendResume(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool WorkqueueExecuteStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool WorkqueueExecuteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool CpuIdle(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool CpuFrequency(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool CpuFrequencyLimits(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool TracingMarkWriteOrPrintFormat(FtraceEvent& ftraceEvent,
                                       uint8_t data[],
                                       size_t size,
                                       const EventFormat& format);
    bool TaskRename(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool TaskNewtask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransaction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransactionReceived(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransactionAllocBuf(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransactionAllocLock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransactionAllocLocked(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool BinderTransactionAllocUnlock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedSwitch(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedBlockedReason(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedWakeup(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedWaking(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedWakeupNew(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedProcessExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool SchedProcessFree(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool ClockSetRate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool ClockEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool ClockDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool RegulatorSetVoltage(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool RegulatorSetVoltageComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool RegulatorDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);
    bool RegulatorDisableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format);

private:
    FtraceEventProcessor();
    ~FtraceEventProcessor();
    std::map<uint32_t, HandleFunction> eventIdToFunctions_;
    std::map<uint32_t, std::string> eventIdToNames_;
    std::map<std::string, HandleFunction> eventNameToFunctions_;
    TraceStreamerConfig config_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // FTRACE_EVENT_PROCESSOR_H
