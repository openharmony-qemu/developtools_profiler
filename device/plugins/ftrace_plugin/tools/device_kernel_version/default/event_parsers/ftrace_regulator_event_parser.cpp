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
struct RegulatorBypassDisableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassDisableData(
    RegulatorBypassDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorBypassDisable(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorBypassDisableData eventData {};
    ParseRegulatorBypassDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable, ParseRegulatorBypassDisable<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable,
    ParseRegulatorBypassDisable<ProtoEncoder::FtraceEvent>);

struct RegulatorBypassDisableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassDisableCompleteData(
    RegulatorBypassDisableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorBypassDisableComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorBypassDisableCompleteData eventData {};
    ParseRegulatorBypassDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable_complete,
    ParseRegulatorBypassDisableComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable_complete,
    ParseRegulatorBypassDisableComplete<ProtoEncoder::FtraceEvent>);

struct RegulatorBypassEnableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassEnableData(
    RegulatorBypassEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorBypassEnable(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorBypassEnableData eventData {};
    ParseRegulatorBypassEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable, ParseRegulatorBypassEnable<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable,
    ParseRegulatorBypassEnable<ProtoEncoder::FtraceEvent>);

struct RegulatorBypassEnableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassEnableCompleteData(
    RegulatorBypassEnableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorBypassEnableComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorBypassEnableCompleteData eventData {};
    ParseRegulatorBypassEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable_complete, ParseRegulatorBypassEnableComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable_complete,
    ParseRegulatorBypassEnableComplete<ProtoEncoder::FtraceEvent>);

struct RegulatorDisableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorDisableData(RegulatorDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorDisable(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorDisableData eventData {};
    ParseRegulatorDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable, ParseRegulatorDisable<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable, ParseRegulatorDisable<ProtoEncoder::FtraceEvent>);

struct RegulatorDisableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorDisableCompleteData(
    RegulatorDisableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorDisableComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorDisableCompleteData eventData {};
    ParseRegulatorDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable_complete, ParseRegulatorDisableComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable_complete,
    ParseRegulatorDisableComplete<ProtoEncoder::FtraceEvent>);

struct RegulatorEnableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableData(RegulatorEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorEnable(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorEnableData eventData {};
    ParseRegulatorEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable, ParseRegulatorEnable<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable, ParseRegulatorEnable<ProtoEncoder::FtraceEvent>);

struct RegulatorEnableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableCompleteData(
    RegulatorEnableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorEnableComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorEnableCompleteData eventData {};
    ParseRegulatorEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_complete, ParseRegulatorEnableComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_complete,
    ParseRegulatorEnableComplete<ProtoEncoder::FtraceEvent>);

struct RegulatorEnableDelayData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableDelayData(
    RegulatorEnableDelayData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorEnableDelay(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorEnableDelayData eventData {};
    ParseRegulatorEnableDelayData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_delay_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_delay, ParseRegulatorEnableDelay<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_delay,
    ParseRegulatorEnableDelay<ProtoEncoder::FtraceEvent>);

struct RegulatorSetVoltageData {
    std::string name {};
    int32_t min {};
    int32_t max {};
};
__attribute__((noinline)) void ParseRegulatorSetVoltageData(
    RegulatorSetVoltageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.min = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.max = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorSetVoltage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorSetVoltageData eventData {};
    ParseRegulatorSetVoltageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_min(eventData.min);
    msg->set_max(eventData.max);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage, ParseRegulatorSetVoltage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage,
    ParseRegulatorSetVoltage<ProtoEncoder::FtraceEvent>);

struct RegulatorSetVoltageCompleteData {
    std::string name {};
    uint32_t val {};
};
__attribute__((noinline)) void ParseRegulatorSetVoltageCompleteData(
    RegulatorSetVoltageCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.val = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRegulatorSetVoltageComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RegulatorSetVoltageCompleteData eventData {};
    ParseRegulatorSetVoltageCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage_complete, ParseRegulatorSetVoltageComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage_complete,
    ParseRegulatorSetVoltageComplete<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
