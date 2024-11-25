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
void clock_disable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_disable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_disable_format();
    clock_disable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_disable, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_disable_format();
    clock_disable_func(msg, data, size, format);
});

template <typename T>
void clock_enable_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_enable, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_enable_format();
    clock_enable_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_enable, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_enable_format();
    clock_enable_func(msg, data, size, format);
});

template <typename T>
void clock_set_rate_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_set_rate, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_set_rate_format();
    clock_set_rate_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_set_rate, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_clock_set_rate_format();
    clock_set_rate_func(msg, data, size, format);
});

template <typename T>
void cpu_frequency_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_frequency, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_cpu_frequency_format();
    cpu_frequency_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_frequency, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_cpu_frequency_format();
    cpu_frequency_func(msg, data, size, format);
});

template <typename T>
void cpu_frequency_limits_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_min_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_max_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_frequency_limits, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_cpu_frequency_limits_format();
    cpu_frequency_limits_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_frequency_limits,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_cpu_frequency_limits_format();
                                                cpu_frequency_limits_func(msg, data, size, format);
                                            });

template <typename T>
void cpu_idle_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_idle, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_cpu_idle_format();
    cpu_idle_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_idle, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                          size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_cpu_idle_format();
    cpu_idle_func(msg, data, size, format);
});

template <typename T>
void dev_pm_qos_add_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_add_request, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_dev_pm_qos_add_request_format();
    dev_pm_qos_add_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_add_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_dev_pm_qos_add_request_format();
                                                dev_pm_qos_add_request_func(msg, data, size, format);
                                            });

template <typename T>
void dev_pm_qos_remove_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_remove_request, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                   size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_dev_pm_qos_remove_request_format();
    dev_pm_qos_remove_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_remove_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_dev_pm_qos_remove_request_format();
                                                dev_pm_qos_remove_request_func(msg, data, size, format);
                                            });

template <typename T>
void dev_pm_qos_update_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_new_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_update_request, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                   size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_dev_pm_qos_update_request_format();
    dev_pm_qos_update_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_update_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_dev_pm_qos_update_request_format();
    dev_pm_qos_update_request_func(msg, data, size, format);
});

template <typename T>
void device_pm_callback_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_device(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(device_pm_callback_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_device_pm_callback_end_format();
    device_pm_callback_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(device_pm_callback_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_device_pm_callback_end_format();
                                                device_pm_callback_end_func(msg, data, size, format);
                                            });

template <typename T>
void device_pm_callback_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_device(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_parent(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pm_ops(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_event(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(device_pm_callback_start, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_device_pm_callback_start_format();
    device_pm_callback_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(device_pm_callback_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_device_pm_callback_start_format();
                                                device_pm_callback_start_func(msg, data, size, format);
                                            });

template <typename T>
void pm_qos_add_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_add_request, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_add_request_format();
    pm_qos_add_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_add_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_pm_qos_add_request_format();
                                                pm_qos_add_request_func(msg, data, size, format);
                                            });

template <typename T>
void pm_qos_remove_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_remove_request, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_remove_request_format();
    pm_qos_remove_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_remove_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_pm_qos_remove_request_format();
                                                pm_qos_remove_request_func(msg, data, size, format);
                                            });

template <typename T>
void pm_qos_update_flags_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_action(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_curr_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_flags, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_update_flags_format();
    pm_qos_update_flags_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_flags,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_pm_qos_update_flags_format();
                                                pm_qos_update_flags_func(msg, data, size, format);
                                            });

template <typename T>
void pm_qos_update_request_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_request, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_update_request_format();
    pm_qos_update_request_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_request,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_pm_qos_update_request_format();
                                                pm_qos_update_request_func(msg, data, size, format);
                                            });

template <typename T>
void pm_qos_update_target_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_action(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_curr_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_target, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_update_target_format();
    pm_qos_update_target_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_target,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pm_qos_update_target_format();
    pm_qos_update_target_func(msg, data, size, format);
});

template <typename T>
void power_domain_target_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_cpu_id(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(power_domain_target, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_power_domain_target_format();
    power_domain_target_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(power_domain_target,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_power_domain_target_format();
                                                power_domain_target_func(msg, data, size, format);
                                            });

template <typename T>
void pstate_sample_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_core_busy(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_scaled_busy(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_from(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_to(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_mperf(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_aperf(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_tsc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_freq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_io_boost(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pstate_sample, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pstate_sample_format();
    pstate_sample_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pstate_sample, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_pstate_sample_format();
    pstate_sample_func(msg, data, size, format);
});

template <typename T>
void suspend_resume_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_action(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_val(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(suspend_resume, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_suspend_resume_format();
    suspend_resume_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(suspend_resume, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_suspend_resume_format();
    suspend_resume_func(msg, data, size, format);
});

template <typename T>
void wakeup_source_activate_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup_source_activate, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_wakeup_source_activate_format();
    wakeup_source_activate_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup_source_activate,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_wakeup_source_activate_format();
                                                wakeup_source_activate_func(msg, data, size, format);
                                            });

template <typename T>
void wakeup_source_deactivate_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup_source_deactivate, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_wakeup_source_deactivate_format();
    wakeup_source_deactivate_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup_source_deactivate,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_wakeup_source_deactivate_format();
                                                wakeup_source_deactivate_func(msg, data, size, format);
                                            });
}  // namespace
FTRACE_NS_END
