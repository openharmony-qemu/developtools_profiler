/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct IrqHandlerEntryData {
    int32_t irq {};
    std::string name {};
};
__attribute__((noinline)) void ParseIrqHandlerEntryData(IrqHandlerEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.irq = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseIrqHandlerEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    IrqHandlerEntryData eventData {};
    ParseIrqHandlerEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_irq_handler_entry_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_irq(eventData.irq);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(irq_handler_entry, ParseIrqHandlerEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(irq_handler_entry, ParseIrqHandlerEntry<ProtoEncoder::FtraceEvent>);

struct IrqHandlerExitData {
    int32_t irq {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseIrqHandlerExitData(IrqHandlerExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.irq = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseIrqHandlerExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    IrqHandlerExitData eventData {};
    ParseIrqHandlerExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_irq_handler_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_irq(eventData.irq);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(irq_handler_exit, ParseIrqHandlerExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(irq_handler_exit, ParseIrqHandlerExit<ProtoEncoder::FtraceEvent>);

struct SoftirqEntryData {
    uint32_t vec {};
};
__attribute__((noinline)) void ParseSoftirqEntryData(SoftirqEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.vec = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSoftirqEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SoftirqEntryData eventData {};
    ParseSoftirqEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_softirq_entry_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_vec(eventData.vec);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_entry, ParseSoftirqEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_entry, ParseSoftirqEntry<ProtoEncoder::FtraceEvent>);

struct SoftirqExitData {
    uint32_t vec {};
};
__attribute__((noinline)) void ParseSoftirqExitData(SoftirqExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.vec = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSoftirqExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SoftirqExitData eventData {};
    ParseSoftirqExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_softirq_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_vec(eventData.vec);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_exit, ParseSoftirqExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_exit, ParseSoftirqExit<ProtoEncoder::FtraceEvent>);

struct SoftirqRaiseData {
    uint32_t vec {};
};
__attribute__((noinline)) void ParseSoftirqRaiseData(SoftirqRaiseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.vec = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSoftirqRaise(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SoftirqRaiseData eventData {};
    ParseSoftirqRaiseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_softirq_raise_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_vec(eventData.vec);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_raise, ParseSoftirqRaise<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_raise, ParseSoftirqRaise<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
