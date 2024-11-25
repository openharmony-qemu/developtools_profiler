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
template <typename T>
void regulator_bypass_disable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_format();
    regulator_bypass_disable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_regulator_bypass_disable_format();
                                                regulator_bypass_disable_func(msg, data, size, format);
                                            });

template <typename T>
void regulator_bypass_disable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                           size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_complete_format();
    regulator_bypass_disable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_complete_format();
    regulator_bypass_disable_complete_func(msg, data, size, format);
});

template <typename T>
void regulator_bypass_enable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_format();
    regulator_bypass_enable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_regulator_bypass_enable_format();
                                                regulator_bypass_enable_func(msg, data, size, format);
                                            });

template <typename T>
void regulator_bypass_enable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                          size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_complete_format();
    regulator_bypass_enable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_complete_format();
    regulator_bypass_enable_complete_func(msg, data, size, format);
});

template <typename T>
void regulator_disable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_disable_format();
    regulator_disable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_disable_format();
    regulator_disable_func(msg, data, size, format);
});

template <typename T>
void regulator_disable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                    size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_disable_complete_format();
    regulator_disable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_disable_complete_format();
    regulator_disable_complete_func(msg, data, size, format);
});

template <typename T>
void regulator_enable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_enable_format();
    regulator_enable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_enable_format();
    regulator_enable_func(msg, data, size, format);
});

template <typename T>
void regulator_enable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                   size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_enable_complete_format();
    regulator_enable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_enable_complete_format();
    regulator_enable_complete_func(msg, data, size, format);
});

template <typename T>
void regulator_enable_delay_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_delay, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_enable_delay_format();
    regulator_enable_delay_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_delay,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_regulator_enable_delay_format();
                                                regulator_enable_delay_func(msg, data, size, format);
                                            });

template <typename T>
void regulator_set_voltage_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_min(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_max(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_set_voltage_format();
    regulator_set_voltage_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_regulator_set_voltage_format();
                                                regulator_set_voltage_func(msg, data, size, format);
                                            });

template <typename T>
void regulator_set_voltage_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_val(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage_complete, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                        size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_regulator_set_voltage_complete_format();
    regulator_set_voltage_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_regulator_set_voltage_complete_format();
                                                regulator_set_voltage_complete_func(msg, data, size, format);
                                            });
}  // namespace
FTRACE_NS_END
