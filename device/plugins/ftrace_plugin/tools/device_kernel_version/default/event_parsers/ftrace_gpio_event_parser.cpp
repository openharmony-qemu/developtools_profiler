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
struct GpioDirectionData {
    uint32_t gpio {};
    int32_t in {};
    int32_t err {};
};
__attribute__((noinline)) void ParseGpioDirectionData(GpioDirectionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.gpio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.in = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGpioDirection(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    GpioDirectionData eventData {};
    ParseGpioDirectionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_gpio_direction_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_gpio(eventData.gpio);
    msg->set_in(eventData.in);
    msg->set_err(eventData.err);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(gpio_direction, ParseGpioDirection<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(gpio_direction, ParseGpioDirection<ProtoEncoder::FtraceEvent>);

struct GpioValueData {
    uint32_t gpio {};
    int32_t get {};
    int32_t value {};
};
__attribute__((noinline)) void ParseGpioValueData(GpioValueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.gpio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.get = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGpioValue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    GpioValueData eventData {};
    ParseGpioValueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_gpio_value_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_gpio(eventData.gpio);
    msg->set_get(eventData.get);
    msg->set_value(eventData.value);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(gpio_value, ParseGpioValue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(gpio_value, ParseGpioValue<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
