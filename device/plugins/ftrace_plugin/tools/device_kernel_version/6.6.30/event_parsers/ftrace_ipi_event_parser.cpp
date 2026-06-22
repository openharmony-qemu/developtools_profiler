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
struct IpiEntryData {
    std::string reason {};
};
__attribute__((noinline)) void ParseIpiEntryData(IpiEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.reason = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseIpiEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiEntryData eventData {};
    ParseIpiEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_entry_format();
    msg->set_reason(eventData.reason);
}
void ParseIpiEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiEntryData eventData {};
    ParseIpiEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_entry_format();
    msg->set_reason(eventData.reason);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ipi_entry, ParseIpiEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ipi_entry, ParseIpiEntryEncoder);

struct IpiExitData {
    std::string reason {};
};
__attribute__((noinline)) void ParseIpiExitData(IpiExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.reason = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseIpiExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiExitData eventData {};
    ParseIpiExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_exit_format();
    msg->set_reason(eventData.reason);
}
void ParseIpiExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiExitData eventData {};
    ParseIpiExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_exit_format();
    msg->set_reason(eventData.reason);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ipi_exit, ParseIpiExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ipi_exit, ParseIpiExitEncoder);

struct IpiRaiseData {
    uint64_t target_cpus {};
    std::string reason {};
};
__attribute__((noinline)) void ParseIpiRaiseData(IpiRaiseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.target_cpus = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseIpiRaise(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiRaiseData eventData {};
    ParseIpiRaiseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_raise_format();
    msg->set_target_cpus(eventData.target_cpus);
    msg->set_reason(eventData.reason);
}
void ParseIpiRaiseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    IpiRaiseData eventData {};
    ParseIpiRaiseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ipi_raise_format();
    msg->set_target_cpus(eventData.target_cpus);
    msg->set_reason(eventData.reason);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ipi_raise, ParseIpiRaise);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ipi_raise, ParseIpiRaiseEncoder);
}  // namespace
FTRACE_NS_END
