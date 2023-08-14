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
    clock_disable,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_clock_disable_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    clock_enable,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_clock_enable_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    clock_set_rate,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_clock_set_rate_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cpu_frequency,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cpu_frequency_format();
        msg->set_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cpu_frequency_limits,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cpu_frequency_limits_format();
        msg->set_min_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_max_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cpu_idle,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cpu_idle_format();
        msg->set_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    dev_pm_qos_add_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_dev_pm_qos_add_request_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    dev_pm_qos_remove_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_dev_pm_qos_remove_request_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    dev_pm_qos_update_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_dev_pm_qos_update_request_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    device_pm_callback_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_device_pm_callback_end_format();
        msg->set_device(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    device_pm_callback_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_device_pm_callback_start_format();
        msg->set_device(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_parent(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pm_ops(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_event(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_add_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_add_request_format();
        msg->set_pm_qos_class(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_remove_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_remove_request_format();
        msg->set_pm_qos_class(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_update_flags,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_update_flags_format();
        msg->set_action(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_curr_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_update_request,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_update_request_format();
        msg->set_pm_qos_class(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_update_request_timeout,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_update_request_timeout_format();
        msg->set_pm_qos_class(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_timeout_us(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pm_qos_update_target,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pm_qos_update_target_format();
        msg->set_action(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_curr_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    power_domain_target,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_power_domain_target_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    pstate_sample,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_pstate_sample_format();
        msg->set_core_busy(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_scaled_busy(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_from(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_to(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mperf(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_aperf(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_tsc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_io_boost(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    suspend_resume,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_suspend_resume_format();
        msg->set_action(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_val(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_start(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    wakeup_source_activate,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_wakeup_source_activate_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    wakeup_source_deactivate,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_wakeup_source_deactivate_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
