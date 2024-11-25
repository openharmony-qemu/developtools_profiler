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
void clk_disable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_disable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                     const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_disable_format();
    clk_disable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_disable, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                             size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_disable_format();
    clk_disable_func(msg, data, size, format);
});

template <typename T>
void clk_disable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_disable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_disable_complete_format();
    clk_disable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_disable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_clk_disable_complete_format();
                                                clk_disable_complete_func(msg, data, size, format);
                                            });

template <typename T>
void clk_enable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_enable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                    const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_enable_format();
    clk_enable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_enable, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                            size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_enable_format();
    clk_enable_func(msg, data, size, format);
});

template <typename T>
void clk_enable_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_enable_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_enable_complete_format();
    clk_enable_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_enable_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_clk_enable_complete_format();
                                                clk_enable_complete_func(msg, data, size, format);
                                            });

template <typename T>
void clk_prepare_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_prepare, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                     const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_prepare_format();
    clk_prepare_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_prepare, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                             size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_prepare_format();
    clk_prepare_func(msg, data, size, format);
});

template <typename T>
void clk_prepare_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_prepare_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_prepare_complete_format();
    clk_prepare_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_prepare_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_clk_prepare_complete_format();
                                                clk_prepare_complete_func(msg, data, size, format);
                                            });

template <typename T>
void clk_set_parent_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pname(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_parent, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_parent_format();
    clk_set_parent_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_parent, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_parent_format();
    clk_set_parent_func(msg, data, size, format);
});

template <typename T>
void clk_set_parent_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pname(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_parent_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_parent_complete_format();
    clk_set_parent_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_parent_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_parent_complete_format();
    clk_set_parent_complete_func(msg, data, size, format);
});

template <typename T>
void clk_set_phase_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_phase(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_phase, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_phase_format();
    clk_set_phase_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_phase, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_phase_format();
    clk_set_phase_func(msg, data, size, format);
});

template <typename T>
void clk_set_phase_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_phase(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_phase_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_phase_complete_format();
    clk_set_phase_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_phase_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_phase_complete_format();
    clk_set_phase_complete_func(msg, data, size, format);
});

template <typename T>
void clk_set_rate_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_rate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_rate, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_rate_format();
    clk_set_rate_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_rate, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_rate_format();
    clk_set_rate_func(msg, data, size, format);
});

template <typename T>
void clk_set_rate_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_rate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_rate_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_rate_complete_format();
    clk_set_rate_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_rate_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_set_rate_complete_format();
    clk_set_rate_complete_func(msg, data, size, format);
});

template <typename T>
void clk_unprepare_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_unprepare, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_unprepare_format();
    clk_unprepare_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_unprepare, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_unprepare_format();
    clk_unprepare_func(msg, data, size, format);
});

template <typename T>
void clk_unprepare_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_unprepare_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_unprepare_complete_format();
    clk_unprepare_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_unprepare_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clk_unprepare_complete_format();
    clk_unprepare_complete_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
