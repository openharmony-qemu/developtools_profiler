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
struct CpuhpEnterData {
    uint32_t cpu {};
    int32_t target {};
    int32_t idx {};
    uint64_t fun {};
};
__attribute__((noinline)) void ParseCpuhpEnterData(CpuhpEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cpu = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.target = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.fun = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseCpuhpEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpEnterData eventData {};
    ParseCpuhpEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_enter_format();
    msg->set_cpu(eventData.cpu);
    msg->set_target(eventData.target);
    msg->set_idx(eventData.idx);
    msg->set_fun(eventData.fun);
}
void ParseCpuhpEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpEnterData eventData {};
    ParseCpuhpEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_enter_format();
    msg->set_cpu(eventData.cpu);
    msg->set_target(eventData.target);
    msg->set_idx(eventData.idx);
    msg->set_fun(eventData.fun);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_enter, ParseCpuhpEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_enter, ParseCpuhpEnterEncoder);

struct CpuhpExitData {
    uint32_t cpu {};
    int32_t state {};
    int32_t idx {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseCpuhpExitData(CpuhpExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cpu = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseCpuhpExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpExitData eventData {};
    ParseCpuhpExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_exit_format();
    msg->set_cpu(eventData.cpu);
    msg->set_state(eventData.state);
    msg->set_idx(eventData.idx);
    msg->set_ret(eventData.ret);
}
void ParseCpuhpExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpExitData eventData {};
    ParseCpuhpExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_exit_format();
    msg->set_cpu(eventData.cpu);
    msg->set_state(eventData.state);
    msg->set_idx(eventData.idx);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_exit, ParseCpuhpExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_exit, ParseCpuhpExitEncoder);

struct CpuhpMultiEnterData {
    uint32_t cpu {};
    int32_t target {};
    int32_t idx {};
    uint64_t fun {};
};
__attribute__((noinline)) void ParseCpuhpMultiEnterData(CpuhpMultiEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cpu = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.target = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.fun = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseCpuhpMultiEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpMultiEnterData eventData {};
    ParseCpuhpMultiEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_multi_enter_format();
    msg->set_cpu(eventData.cpu);
    msg->set_target(eventData.target);
    msg->set_idx(eventData.idx);
    msg->set_fun(eventData.fun);
}
void ParseCpuhpMultiEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CpuhpMultiEnterData eventData {};
    ParseCpuhpMultiEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cpuhp_multi_enter_format();
    msg->set_cpu(eventData.cpu);
    msg->set_target(eventData.target);
    msg->set_idx(eventData.idx);
    msg->set_fun(eventData.fun);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cpuhp_multi_enter, ParseCpuhpMultiEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cpuhp_multi_enter, ParseCpuhpMultiEnterEncoder);
}  // namespace
FTRACE_NS_END
