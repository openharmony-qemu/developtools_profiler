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

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
template <typename T> void irq_handler_entry_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_irq(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(irq_handler_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_irq_handler_entry_format();
        irq_handler_entry_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(irq_handler_entry,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_irq_handler_entry_format();
        irq_handler_entry_func(msg, data, size, format);
    });

template <typename T> void irq_handler_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_irq(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(irq_handler_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_irq_handler_exit_format();
        irq_handler_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(irq_handler_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_irq_handler_exit_format();
        irq_handler_exit_func(msg, data, size, format);
    });

template <typename T> void softirq_entry_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_vec(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_entry_format();
        softirq_entry_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_entry,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_entry_format();
        softirq_entry_func(msg, data, size, format);
    });

template <typename T> void softirq_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_vec(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_exit_format();
        softirq_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_exit_format();
        softirq_exit_func(msg, data, size, format);
    });

template <typename T> void softirq_raise_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_vec(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(softirq_raise,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_raise_format();
        softirq_raise_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(softirq_raise,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_softirq_raise_format();
        softirq_raise_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
