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
    gpio_direction,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_gpio_direction_format();
        msg->set_gpio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_in(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    gpio_value,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_gpio_value_format();
        msg->set_gpio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_get(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
