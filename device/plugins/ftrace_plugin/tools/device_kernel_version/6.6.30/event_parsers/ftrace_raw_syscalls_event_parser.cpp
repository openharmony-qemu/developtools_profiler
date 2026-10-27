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
struct SysEnterData {
    uint64_t id {};
    std::vector<uint64_t> args {};
};
__attribute__((noinline)) void ParseSysEnterData(SysEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.args = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSysEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    SysEnterData eventData {};
    ParseSysEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sys_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_id(eventData.id);
    for (size_t i = 0; i < eventData.args.size(); i++) {
        msg->add_args(eventData.args[i]);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sys_enter, ParseSysEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sys_enter, ParseSysEnter<ProtoEncoder::FtraceEvent>);

struct SysExitData {
    uint64_t id {};
    uint64_t ret {};
};
__attribute__((noinline)) void ParseSysExitData(SysExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.id = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSysExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    SysExitData eventData {};
    ParseSysExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sys_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_id(eventData.id);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sys_exit, ParseSysExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sys_exit, ParseSysExit<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
