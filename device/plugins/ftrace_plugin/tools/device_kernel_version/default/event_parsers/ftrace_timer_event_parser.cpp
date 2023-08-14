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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    hrtimer_cancel,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_hrtimer_cancel_format();
        msg->set_hrtimer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    hrtimer_expire_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_hrtimer_expire_entry_format();
        msg->set_hrtimer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_now(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    hrtimer_expire_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_hrtimer_expire_exit_format();
        msg->set_hrtimer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    hrtimer_init,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_hrtimer_init_format();
        msg->set_hrtimer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_clockid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    hrtimer_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_hrtimer_start_format();
        msg->set_hrtimer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_expires(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
        msg->set_softexpires(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    itimer_expire,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_itimer_expire_format();
        msg->set_which(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_now(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    itimer_state,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_itimer_state_format();
        msg->set_which(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_expires(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_value_sec(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_value_usec(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_interval_sec(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_interval_usec(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    timer_cancel,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_timer_cancel_format();
        msg->set_timer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    timer_expire_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_timer_expire_entry_format();
        msg->set_timer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_now(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    timer_expire_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_timer_expire_exit_format();
        msg->set_timer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    timer_init,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_timer_init_format();
        msg->set_timer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    timer_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_timer_start_format();
        msg->set_timer(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_expires(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_now(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
