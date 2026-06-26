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
struct ClkDisableData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkDisableData(ClkDisableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkDisable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkDisableData eventData {};
    ParseClkDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_disable_format();
    msg->set_name(eventData.name);
}
void ParseClkDisableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkDisableData eventData {};
    ParseClkDisableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_disable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_disable, ParseClkDisable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_disable, ParseClkDisableEncoder);

struct ClkDisableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkDisableCompleteData(ClkDisableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkDisableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkDisableCompleteData eventData {};
    ParseClkDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_disable_complete_format();
    msg->set_name(eventData.name);
}
void ParseClkDisableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkDisableCompleteData eventData {};
    ParseClkDisableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_disable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_disable_complete, ParseClkDisableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_disable_complete, ParseClkDisableCompleteEncoder);

struct ClkEnableData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkEnableData(ClkEnableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkEnable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkEnableData eventData {};
    ParseClkEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_enable_format();
    msg->set_name(eventData.name);
}
void ParseClkEnableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkEnableData eventData {};
    ParseClkEnableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_enable_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_enable, ParseClkEnable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_enable, ParseClkEnableEncoder);

struct ClkEnableCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkEnableCompleteData(ClkEnableCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkEnableComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkEnableCompleteData eventData {};
    ParseClkEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_enable_complete_format();
    msg->set_name(eventData.name);
}
void ParseClkEnableCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkEnableCompleteData eventData {};
    ParseClkEnableCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_enable_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_enable_complete, ParseClkEnableComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_enable_complete, ParseClkEnableCompleteEncoder);

struct ClkPrepareData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkPrepareData(ClkPrepareData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkPrepare(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkPrepareData eventData {};
    ParseClkPrepareData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_prepare_format();
    msg->set_name(eventData.name);
}
void ParseClkPrepareEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkPrepareData eventData {};
    ParseClkPrepareData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_prepare_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_prepare, ParseClkPrepare);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_prepare, ParseClkPrepareEncoder);

struct ClkPrepareCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkPrepareCompleteData(ClkPrepareCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkPrepareComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkPrepareCompleteData eventData {};
    ParseClkPrepareCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_prepare_complete_format();
    msg->set_name(eventData.name);
}
void ParseClkPrepareCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkPrepareCompleteData eventData {};
    ParseClkPrepareCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_prepare_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_prepare_complete, ParseClkPrepareComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_prepare_complete, ParseClkPrepareCompleteEncoder);

struct ClkSetParentData {
    std::string name {};
    std::string pname {};
};
__attribute__((noinline)) void ParseClkSetParentData(ClkSetParentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pname = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkSetParent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetParentData eventData {};
    ParseClkSetParentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_parent_format();
    msg->set_name(eventData.name);
    msg->set_pname(eventData.pname);
}
void ParseClkSetParentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetParentData eventData {};
    ParseClkSetParentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_parent_format();
    msg->set_name(eventData.name);
    msg->set_pname(eventData.pname);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_parent, ParseClkSetParent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_parent, ParseClkSetParentEncoder);

struct ClkSetParentCompleteData {
    std::string name {};
    std::string pname {};
};
__attribute__((noinline)) void ParseClkSetParentCompleteData(ClkSetParentCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pname = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkSetParentComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetParentCompleteData eventData {};
    ParseClkSetParentCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_parent_complete_format();
    msg->set_name(eventData.name);
    msg->set_pname(eventData.pname);
}
void ParseClkSetParentCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetParentCompleteData eventData {};
    ParseClkSetParentCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_parent_complete_format();
    msg->set_name(eventData.name);
    msg->set_pname(eventData.pname);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_parent_complete, ParseClkSetParentComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_parent_complete, ParseClkSetParentCompleteEncoder);

struct ClkSetPhaseData {
    std::string name {};
    int32_t phase {};
};
__attribute__((noinline)) void ParseClkSetPhaseData(ClkSetPhaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.phase = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseClkSetPhase(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetPhaseData eventData {};
    ParseClkSetPhaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_phase_format();
    msg->set_name(eventData.name);
    msg->set_phase(eventData.phase);
}
void ParseClkSetPhaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetPhaseData eventData {};
    ParseClkSetPhaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_phase_format();
    msg->set_name(eventData.name);
    msg->set_phase(eventData.phase);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_phase, ParseClkSetPhase);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_phase, ParseClkSetPhaseEncoder);

struct ClkSetPhaseCompleteData {
    std::string name {};
    int32_t phase {};
};
__attribute__((noinline)) void ParseClkSetPhaseCompleteData(ClkSetPhaseCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.phase = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseClkSetPhaseComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetPhaseCompleteData eventData {};
    ParseClkSetPhaseCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_phase_complete_format();
    msg->set_name(eventData.name);
    msg->set_phase(eventData.phase);
}
void ParseClkSetPhaseCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetPhaseCompleteData eventData {};
    ParseClkSetPhaseCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_phase_complete_format();
    msg->set_name(eventData.name);
    msg->set_phase(eventData.phase);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_phase_complete, ParseClkSetPhaseComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_phase_complete, ParseClkSetPhaseCompleteEncoder);

struct ClkSetRateData {
    std::string name {};
    uint64_t rate {};
};
__attribute__((noinline)) void ParseClkSetRateData(ClkSetRateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.rate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseClkSetRate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetRateData eventData {};
    ParseClkSetRateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_rate_format();
    msg->set_name(eventData.name);
    msg->set_rate(eventData.rate);
}
void ParseClkSetRateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetRateData eventData {};
    ParseClkSetRateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_rate_format();
    msg->set_name(eventData.name);
    msg->set_rate(eventData.rate);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_rate, ParseClkSetRate);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_rate, ParseClkSetRateEncoder);

struct ClkSetRateCompleteData {
    std::string name {};
    uint64_t rate {};
};
__attribute__((noinline)) void ParseClkSetRateCompleteData(ClkSetRateCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.rate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseClkSetRateComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetRateCompleteData eventData {};
    ParseClkSetRateCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_rate_complete_format();
    msg->set_name(eventData.name);
    msg->set_rate(eventData.rate);
}
void ParseClkSetRateCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkSetRateCompleteData eventData {};
    ParseClkSetRateCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_set_rate_complete_format();
    msg->set_name(eventData.name);
    msg->set_rate(eventData.rate);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_set_rate_complete, ParseClkSetRateComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_set_rate_complete, ParseClkSetRateCompleteEncoder);

struct ClkUnprepareData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkUnprepareData(ClkUnprepareData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkUnprepare(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkUnprepareData eventData {};
    ParseClkUnprepareData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_unprepare_format();
    msg->set_name(eventData.name);
}
void ParseClkUnprepareEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkUnprepareData eventData {};
    ParseClkUnprepareData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_unprepare_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_unprepare, ParseClkUnprepare);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_unprepare, ParseClkUnprepareEncoder);

struct ClkUnprepareCompleteData {
    std::string name {};
};
__attribute__((noinline)) void ParseClkUnprepareCompleteData(ClkUnprepareCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseClkUnprepareComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkUnprepareCompleteData eventData {};
    ParseClkUnprepareCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_unprepare_complete_format();
    msg->set_name(eventData.name);
}
void ParseClkUnprepareCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ClkUnprepareCompleteData eventData {};
    ParseClkUnprepareCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_clk_unprepare_complete_format();
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(clk_unprepare_complete, ParseClkUnprepareComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(clk_unprepare_complete, ParseClkUnprepareCompleteEncoder);
} // namespace
FTRACE_NS_END
