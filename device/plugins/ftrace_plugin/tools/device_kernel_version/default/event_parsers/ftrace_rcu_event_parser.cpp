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
struct RcuUtilizationData {
    std::string s {};
};
__attribute__((noinline)) void ParseRcuUtilizationData(RcuUtilizationData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.s = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRcuUtilization(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RcuUtilizationData eventData {};
    ParseRcuUtilizationData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rcu_utilization_format();
    msg->set_s(eventData.s);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rcu_utilization, ParseRcuUtilization<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rcu_utilization, ParseRcuUtilization<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
