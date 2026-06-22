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
struct ClockDisableData {
    std::string name {};
    uint64_t state {};
    uint64_t cpu_id {};
};
__attribute__((noinline)) void ParseClockDisableData(ClockDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseClockDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockDisableData eventData {};
    ParseClockDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_disable_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseClockDisableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockDisableData eventData {};
    ParseClockDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_disable_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_disable, ParseClockDisable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_disable, ParseClockDisableEncoder);

struct ClockEnableData {
    std::string name {};
    uint64_t state {};
    uint64_t cpu_id {};
};
__attribute__((noinline)) void ParseClockEnableData(ClockEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseClockEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockEnableData eventData {};
    ParseClockEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_enable_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseClockEnableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockEnableData eventData {};
    ParseClockEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_enable_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_enable, ParseClockEnable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_enable, ParseClockEnableEncoder);

struct ClockSetRateData {
    std::string name {};
    uint64_t state {};
    uint64_t cpu_id {};
};
__attribute__((noinline)) void ParseClockSetRateData(ClockSetRateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseClockSetRate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockSetRateData eventData {};
    ParseClockSetRateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_set_rate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseClockSetRateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClockSetRateData eventData {};
    ParseClockSetRateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clock_set_rate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clock_set_rate, ParseClockSetRate);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clock_set_rate, ParseClockSetRateEncoder);

struct CpuFrequencyData {
    uint32_t state {};
    uint32_t cpu_id {};
};
__attribute__((noinline)) void ParseCpuFrequencyData(CpuFrequencyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseCpuFrequency(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuFrequencyData eventData {};
    ParseCpuFrequencyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_frequency_format();
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseCpuFrequencyEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuFrequencyData eventData {};
    ParseCpuFrequencyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_frequency_format();
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_frequency, ParseCpuFrequency);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_frequency, ParseCpuFrequencyEncoder);

struct CpuFrequencyLimitsData {
    uint32_t min_freq {};
    uint32_t max_freq {};
    uint32_t cpu_id {};
};
__attribute__((noinline)) void ParseCpuFrequencyLimitsData(CpuFrequencyLimitsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.min_freq = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.max_freq = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseCpuFrequencyLimits(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuFrequencyLimitsData eventData {};
    ParseCpuFrequencyLimitsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_frequency_limits_format();
    msg->set_min_freq(eventData.min_freq);
    msg->set_max_freq(eventData.max_freq);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseCpuFrequencyLimitsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuFrequencyLimitsData eventData {};
    ParseCpuFrequencyLimitsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_frequency_limits_format();
    msg->set_min_freq(eventData.min_freq);
    msg->set_max_freq(eventData.max_freq);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_frequency_limits, ParseCpuFrequencyLimits);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_frequency_limits, ParseCpuFrequencyLimitsEncoder);

struct CpuIdleData {
    uint32_t state {};
    uint32_t cpu_id {};
};
__attribute__((noinline)) void ParseCpuIdleData(CpuIdleData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseCpuIdle(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuIdleData eventData {};
    ParseCpuIdleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_idle_format();
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParseCpuIdleEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuIdleData eventData {};
    ParseCpuIdleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpu_idle_format();
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpu_idle, ParseCpuIdle);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpu_idle, ParseCpuIdleEncoder);

struct DevPmQosAddRequestData {
    std::string name {};
    uint32_t type {};
    int32_t new_value {};
};
__attribute__((noinline)) void ParseDevPmQosAddRequestData(DevPmQosAddRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.new_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseDevPmQosAddRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosAddRequestData eventData {};
    ParseDevPmQosAddRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_add_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
void ParseDevPmQosAddRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosAddRequestData eventData {};
    ParseDevPmQosAddRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_add_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_add_request, ParseDevPmQosAddRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_add_request, ParseDevPmQosAddRequestEncoder);

struct DevPmQosRemoveRequestData {
    std::string name {};
    uint32_t type {};
    int32_t new_value {};
};
__attribute__((noinline)) void ParseDevPmQosRemoveRequestData(DevPmQosRemoveRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.new_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseDevPmQosRemoveRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosRemoveRequestData eventData {};
    ParseDevPmQosRemoveRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_remove_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
void ParseDevPmQosRemoveRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosRemoveRequestData eventData {};
    ParseDevPmQosRemoveRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_remove_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_remove_request, ParseDevPmQosRemoveRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_remove_request, ParseDevPmQosRemoveRequestEncoder);

struct DevPmQosUpdateRequestData {
    std::string name {};
    uint32_t type {};
    int32_t new_value {};
};
__attribute__((noinline)) void ParseDevPmQosUpdateRequestData(DevPmQosUpdateRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.new_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseDevPmQosUpdateRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosUpdateRequestData eventData {};
    ParseDevPmQosUpdateRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_update_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
void ParseDevPmQosUpdateRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevPmQosUpdateRequestData eventData {};
    ParseDevPmQosUpdateRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dev_pm_qos_update_request_format();
    msg->set_name(eventData.name);
    msg->set_type(eventData.type);
    msg->set_new_value(eventData.new_value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dev_pm_qos_update_request, ParseDevPmQosUpdateRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dev_pm_qos_update_request, ParseDevPmQosUpdateRequestEncoder);

struct DevicePmCallbackEndData {
    std::string device {};
    std::string driver {};
    int32_t error {};
};
__attribute__((noinline)) void ParseDevicePmCallbackEndData(DevicePmCallbackEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.device = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseDevicePmCallbackEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevicePmCallbackEndData eventData {};
    ParseDevicePmCallbackEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_device_pm_callback_end_format();
    msg->set_device(eventData.device);
    msg->set_driver(eventData.driver);
    msg->set_error(eventData.error);
}
void ParseDevicePmCallbackEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevicePmCallbackEndData eventData {};
    ParseDevicePmCallbackEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_device_pm_callback_end_format();
    msg->set_device(eventData.device);
    msg->set_driver(eventData.driver);
    msg->set_error(eventData.error);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(device_pm_callback_end, ParseDevicePmCallbackEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(device_pm_callback_end, ParseDevicePmCallbackEndEncoder);

struct DevicePmCallbackStartData {
    std::string device {};
    std::string driver {};
    std::string parent {};
    std::string pm_ops {};
    int32_t event {};
};
__attribute__((noinline)) void ParseDevicePmCallbackStartData(DevicePmCallbackStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.device = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.parent = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pm_ops = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.event = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseDevicePmCallbackStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevicePmCallbackStartData eventData {};
    ParseDevicePmCallbackStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_device_pm_callback_start_format();
    msg->set_device(eventData.device);
    msg->set_driver(eventData.driver);
    msg->set_parent(eventData.parent);
    msg->set_pm_ops(eventData.pm_ops);
    msg->set_event(eventData.event);
}
void ParseDevicePmCallbackStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DevicePmCallbackStartData eventData {};
    ParseDevicePmCallbackStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_device_pm_callback_start_format();
    msg->set_device(eventData.device);
    msg->set_driver(eventData.driver);
    msg->set_parent(eventData.parent);
    msg->set_pm_ops(eventData.pm_ops);
    msg->set_event(eventData.event);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(device_pm_callback_start, ParseDevicePmCallbackStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(device_pm_callback_start, ParseDevicePmCallbackStartEncoder);

struct PmQosAddRequestData {
    int32_t pm_qos_class {};
    int32_t value {};
};
__attribute__((noinline)) void ParsePmQosAddRequestData(PmQosAddRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pm_qos_class = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParsePmQosAddRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosAddRequestData eventData {};
    ParsePmQosAddRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_add_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
void ParsePmQosAddRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosAddRequestData eventData {};
    ParsePmQosAddRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_add_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_add_request, ParsePmQosAddRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_add_request, ParsePmQosAddRequestEncoder);

struct PmQosRemoveRequestData {
    int32_t pm_qos_class {};
    int32_t value {};
};
__attribute__((noinline)) void ParsePmQosRemoveRequestData(PmQosRemoveRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pm_qos_class = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParsePmQosRemoveRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosRemoveRequestData eventData {};
    ParsePmQosRemoveRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_remove_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
void ParsePmQosRemoveRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosRemoveRequestData eventData {};
    ParsePmQosRemoveRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_remove_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_remove_request, ParsePmQosRemoveRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_remove_request, ParsePmQosRemoveRequestEncoder);

struct PmQosUpdateFlagsData {
    uint32_t action {};
    int32_t prev_value {};
    int32_t curr_value {};
};
__attribute__((noinline)) void ParsePmQosUpdateFlagsData(PmQosUpdateFlagsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.action = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.curr_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParsePmQosUpdateFlags(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateFlagsData eventData {};
    ParsePmQosUpdateFlagsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_flags_format();
    msg->set_action(eventData.action);
    msg->set_prev_value(eventData.prev_value);
    msg->set_curr_value(eventData.curr_value);
}
void ParsePmQosUpdateFlagsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateFlagsData eventData {};
    ParsePmQosUpdateFlagsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_flags_format();
    msg->set_action(eventData.action);
    msg->set_prev_value(eventData.prev_value);
    msg->set_curr_value(eventData.curr_value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_flags, ParsePmQosUpdateFlags);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_flags, ParsePmQosUpdateFlagsEncoder);

struct PmQosUpdateRequestData {
    int32_t pm_qos_class {};
    int32_t value {};
};
__attribute__((noinline)) void ParsePmQosUpdateRequestData(PmQosUpdateRequestData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pm_qos_class = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParsePmQosUpdateRequest(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateRequestData eventData {};
    ParsePmQosUpdateRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
void ParsePmQosUpdateRequestEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateRequestData eventData {};
    ParsePmQosUpdateRequestData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_request_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_request, ParsePmQosUpdateRequest);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_request, ParsePmQosUpdateRequestEncoder);

struct PmQosUpdateRequestTimeoutData {
    int32_t pm_qos_class {};
    int32_t value {};
    uint64_t timeout_us {};
};
__attribute__((noinline)) void ParsePmQosUpdateRequestTimeoutData(PmQosUpdateRequestTimeoutData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pm_qos_class = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.timeout_us = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParsePmQosUpdateRequestTimeout(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateRequestTimeoutData eventData {};
    ParsePmQosUpdateRequestTimeoutData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_request_timeout_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
    msg->set_timeout_us(eventData.timeout_us);
}
void ParsePmQosUpdateRequestTimeoutEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateRequestTimeoutData eventData {};
    ParsePmQosUpdateRequestTimeoutData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_request_timeout_format();
    msg->set_pm_qos_class(eventData.pm_qos_class);
    msg->set_value(eventData.value);
    msg->set_timeout_us(eventData.timeout_us);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_request_timeout, ParsePmQosUpdateRequestTimeout);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_request_timeout, ParsePmQosUpdateRequestTimeoutEncoder);

struct PmQosUpdateTargetData {
    uint32_t action {};
    int32_t prev_value {};
    int32_t curr_value {};
};
__attribute__((noinline)) void ParsePmQosUpdateTargetData(PmQosUpdateTargetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.action = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.curr_value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParsePmQosUpdateTarget(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateTargetData eventData {};
    ParsePmQosUpdateTargetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_target_format();
    msg->set_action(eventData.action);
    msg->set_prev_value(eventData.prev_value);
    msg->set_curr_value(eventData.curr_value);
}
void ParsePmQosUpdateTargetEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PmQosUpdateTargetData eventData {};
    ParsePmQosUpdateTargetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pm_qos_update_target_format();
    msg->set_action(eventData.action);
    msg->set_prev_value(eventData.prev_value);
    msg->set_curr_value(eventData.curr_value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pm_qos_update_target, ParsePmQosUpdateTarget);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pm_qos_update_target, ParsePmQosUpdateTargetEncoder);

struct PowerDomainTargetData {
    std::string name {};
    uint64_t state {};
    uint64_t cpu_id {};
};
__attribute__((noinline)) void ParsePowerDomainTargetData(PowerDomainTargetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cpu_id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParsePowerDomainTarget(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PowerDomainTargetData eventData {};
    ParsePowerDomainTargetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_power_domain_target_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
void ParsePowerDomainTargetEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PowerDomainTargetData eventData {};
    ParsePowerDomainTargetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_power_domain_target_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
    msg->set_cpu_id(eventData.cpu_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(power_domain_target, ParsePowerDomainTarget);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(power_domain_target, ParsePowerDomainTargetEncoder);

struct PstateSampleData {
    uint32_t core_busy {};
    uint32_t scaled_busy {};
    uint32_t from {};
    uint32_t to {};
    uint64_t mperf {};
    uint64_t aperf {};
    uint64_t tsc {};
    uint32_t freq {};
    uint32_t io_boost {};
};
__attribute__((noinline)) void ParsePstateSampleData(PstateSampleData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.core_busy = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.scaled_busy = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.from = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.to = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mperf = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.aperf = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.tsc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.freq = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.io_boost = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParsePstateSample(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PstateSampleData eventData {};
    ParsePstateSampleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pstate_sample_format();
    msg->set_core_busy(eventData.core_busy);
    msg->set_scaled_busy(eventData.scaled_busy);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_mperf(eventData.mperf);
    msg->set_aperf(eventData.aperf);
    msg->set_tsc(eventData.tsc);
    msg->set_freq(eventData.freq);
    msg->set_io_boost(eventData.io_boost);
}
void ParsePstateSampleEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PstateSampleData eventData {};
    ParsePstateSampleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_pstate_sample_format();
    msg->set_core_busy(eventData.core_busy);
    msg->set_scaled_busy(eventData.scaled_busy);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_mperf(eventData.mperf);
    msg->set_aperf(eventData.aperf);
    msg->set_tsc(eventData.tsc);
    msg->set_freq(eventData.freq);
    msg->set_io_boost(eventData.io_boost);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(pstate_sample, ParsePstateSample);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(pstate_sample, ParsePstateSampleEncoder);

struct SuspendResumeData {
    std::string action {};
    int32_t val {};
    uint32_t start {};
};
__attribute__((noinline)) void ParseSuspendResumeData(SuspendResumeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.action = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.val = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseSuspendResume(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SuspendResumeData eventData {};
    ParseSuspendResumeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_suspend_resume_format();
    msg->set_action(eventData.action);
    msg->set_val(eventData.val);
    msg->set_start(eventData.start);
}
void ParseSuspendResumeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SuspendResumeData eventData {};
    ParseSuspendResumeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_suspend_resume_format();
    msg->set_action(eventData.action);
    msg->set_val(eventData.val);
    msg->set_start(eventData.start);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(suspend_resume, ParseSuspendResume);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(suspend_resume, ParseSuspendResumeEncoder);

struct WakeupSourceActivateData {
    std::string name {};
    uint64_t state {};
};
__attribute__((noinline)) void ParseWakeupSourceActivateData(WakeupSourceActivateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseWakeupSourceActivate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupSourceActivateData eventData {};
    ParseWakeupSourceActivateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_source_activate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
}
void ParseWakeupSourceActivateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupSourceActivateData eventData {};
    ParseWakeupSourceActivateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_source_activate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup_source_activate, ParseWakeupSourceActivate);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup_source_activate, ParseWakeupSourceActivateEncoder);

struct WakeupSourceDeactivateData {
    std::string name {};
    uint64_t state {};
};
__attribute__((noinline)) void ParseWakeupSourceDeactivateData(WakeupSourceDeactivateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseWakeupSourceDeactivate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupSourceDeactivateData eventData {};
    ParseWakeupSourceDeactivateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_source_deactivate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
}
void ParseWakeupSourceDeactivateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupSourceDeactivateData eventData {};
    ParseWakeupSourceDeactivateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_source_deactivate_format();
    msg->set_name(eventData.name);
    msg->set_state(eventData.state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup_source_deactivate, ParseWakeupSourceDeactivate);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup_source_deactivate, ParseWakeupSourceDeactivateEncoder);
} // namespace
FTRACE_NS_END
