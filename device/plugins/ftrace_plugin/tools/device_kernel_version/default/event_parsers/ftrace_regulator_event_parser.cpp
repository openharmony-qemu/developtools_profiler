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
__attribute__((noinline)) void ParseRegulatorBypassDisableData(RegulatorBypassDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorBypassDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassDisableData eventData {};
    ParseRegulatorBypassDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorBypassDisableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassDisableData eventData {};
    ParseRegulatorBypassDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable, ParseRegulatorBypassDisable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable, ParseRegulatorBypassDisableEncoder);

struct RegulatorBypassDisableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassDisableCompleteData(RegulatorBypassDisableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorBypassDisableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassDisableCompleteData eventData {};
    ParseRegulatorBypassDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_complete_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorBypassDisableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassDisableCompleteData eventData {};
    ParseRegulatorBypassDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_disable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_disable_complete, ParseRegulatorBypassDisableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_disable_complete, ParseRegulatorBypassDisableCompleteEncoder);

struct RegulatorBypassEnableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassEnableData(RegulatorBypassEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorBypassEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassEnableData eventData {};
    ParseRegulatorBypassEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorBypassEnableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassEnableData eventData {};
    ParseRegulatorBypassEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable, ParseRegulatorBypassEnable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable, ParseRegulatorBypassEnableEncoder);

struct RegulatorBypassEnableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorBypassEnableCompleteData(RegulatorBypassEnableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorBypassEnableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassEnableCompleteData eventData {};
    ParseRegulatorBypassEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_complete_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorBypassEnableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorBypassEnableCompleteData eventData {};
    ParseRegulatorBypassEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_bypass_enable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_bypass_enable_complete, ParseRegulatorBypassEnableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_bypass_enable_complete, ParseRegulatorBypassEnableCompleteEncoder);

struct RegulatorDisableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorDisableData(RegulatorDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorDisableData eventData {};
    ParseRegulatorDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorDisableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorDisableData eventData {};
    ParseRegulatorDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable, ParseRegulatorDisable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable, ParseRegulatorDisableEncoder);

struct RegulatorDisableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorDisableCompleteData(RegulatorDisableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorDisableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorDisableCompleteData eventData {};
    ParseRegulatorDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_complete_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorDisableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorDisableCompleteData eventData {};
    ParseRegulatorDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_disable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_disable_complete, ParseRegulatorDisableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_disable_complete, ParseRegulatorDisableCompleteEncoder);

struct RegulatorEnableData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableData(RegulatorEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableData eventData {};
    ParseRegulatorEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorEnableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableData eventData {};
    ParseRegulatorEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable, ParseRegulatorEnable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable, ParseRegulatorEnableEncoder);

struct RegulatorEnableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableCompleteData(RegulatorEnableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorEnableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableCompleteData eventData {};
    ParseRegulatorEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_complete_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorEnableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableCompleteData eventData {};
    ParseRegulatorEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_complete, ParseRegulatorEnableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_complete, ParseRegulatorEnableCompleteEncoder);

struct RegulatorEnableDelayData {
    std::string name {};
};
__attribute__((noinline)) void ParseRegulatorEnableDelayData(RegulatorEnableDelayData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRegulatorEnableDelay(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableDelayData eventData {};
    ParseRegulatorEnableDelayData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_delay_format();
    msg->set_name(eventData.name);
}
void ParseRegulatorEnableDelayEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorEnableDelayData eventData {};
    ParseRegulatorEnableDelayData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_enable_delay_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_enable_delay, ParseRegulatorEnableDelay);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_enable_delay, ParseRegulatorEnableDelayEncoder);

struct RegulatorSetVoltageData {
    std::string name {};
    int32_t min {};
    int32_t max {};
};
__attribute__((noinline)) void ParseRegulatorSetVoltageData(RegulatorSetVoltageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.min = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.max = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseRegulatorSetVoltage(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorSetVoltageData eventData {};
    ParseRegulatorSetVoltageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_format();
    msg->set_name(eventData.name);
    msg->set_min(eventData.min);
    msg->set_max(eventData.max);
}
void ParseRegulatorSetVoltageEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorSetVoltageData eventData {};
    ParseRegulatorSetVoltageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_format();
    msg->set_name(eventData.name);
    msg->set_min(eventData.min);
    msg->set_max(eventData.max);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage, ParseRegulatorSetVoltage);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage, ParseRegulatorSetVoltageEncoder);

struct RegulatorSetVoltageCompleteData {
    std::string name {};
    uint32_t val {};
};
__attribute__((noinline)) void ParseRegulatorSetVoltageCompleteData(RegulatorSetVoltageCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.val = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseRegulatorSetVoltageComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorSetVoltageCompleteData eventData {};
    ParseRegulatorSetVoltageCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_complete_format();
    msg->set_name(eventData.name);
    msg->set_val(eventData.val);
}
void ParseRegulatorSetVoltageCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RegulatorSetVoltageCompleteData eventData {};
    ParseRegulatorSetVoltageCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_regulator_set_voltage_complete_format();
    msg->set_name(eventData.name);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(regulator_set_voltage_complete, ParseRegulatorSetVoltageComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(regulator_set_voltage_complete, ParseRegulatorSetVoltageCompleteEncoder);
} // namespace
FTRACE_NS_END
