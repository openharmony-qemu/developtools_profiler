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
struct SignalDeliverData {
    int32_t sig {};
    int32_t error_code {};
    int32_t code {};
    uint64_t sig_handler {};
    uint64_t sig_flags {};
};
__attribute__((noinline)) void ParseSignalDeliverData(SignalDeliverData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.sig = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.error_code = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.code = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.sig_handler = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sig_flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSignalDeliver(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SignalDeliverData eventData {};
    ParseSignalDeliverData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_signal_deliver_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_sig(eventData.sig);
    msg->set_error_code(eventData.error_code);
    msg->set_code(eventData.code);
    msg->set_sig_handler(eventData.sig_handler);
    msg->set_sig_flags(eventData.sig_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(signal_deliver, ParseSignalDeliver<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(signal_deliver, ParseSignalDeliver<ProtoEncoder::FtraceEvent>);

struct SignalGenerateData {
    int32_t sig {};
    int32_t error_code {};
    int32_t code {};
    std::string comm {};
    int32_t pid {};
    int32_t group {};
    int32_t result {};
};
__attribute__((noinline)) void ParseSignalGenerateData(SignalGenerateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.sig = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.error_code = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.code = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSignalGenerate(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SignalGenerateData eventData {};
    ParseSignalGenerateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_signal_generate_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_sig(eventData.sig);
    msg->set_error_code(eventData.error_code);
    msg->set_code(eventData.code);
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_group(eventData.group);
    msg->set_result(eventData.result);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(signal_generate, ParseSignalGenerate<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(signal_generate, ParseSignalGenerate<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
