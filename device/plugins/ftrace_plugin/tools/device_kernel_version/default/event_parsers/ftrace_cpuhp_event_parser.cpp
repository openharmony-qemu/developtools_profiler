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
template <typename T> void cpuhp_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_target(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_fun(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_enter_format();
        cpuhp_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_enter_format();
        cpuhp_enter_func(msg, data, size, format);
    });

template <typename T> void cpuhp_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_exit_format();
        cpuhp_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_exit_format();
        cpuhp_exit_func(msg, data, size, format);
    });

template <typename T> void cpuhp_multi_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_target(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_fun(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_multi_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_multi_enter_format();
        cpuhp_multi_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_multi_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_cpuhp_multi_enter_format();
        cpuhp_multi_enter_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
