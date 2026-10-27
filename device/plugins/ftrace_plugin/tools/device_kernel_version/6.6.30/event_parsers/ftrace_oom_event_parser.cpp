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
struct OomScoreAdjUpdateData {
    int32_t pid {};
    std::string comm {};
    int32_t oom_score_adj {};
};
__attribute__((noinline)) void ParseOomScoreAdjUpdateData(OomScoreAdjUpdateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.oom_score_adj = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseOomScoreAdjUpdate(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    OomScoreAdjUpdateData eventData {};
    ParseOomScoreAdjUpdateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_oom_score_adj_update_format();
    msg->set_pid(eventData.pid);
    msg->set_comm(eventData.comm);
    msg->set_oom_score_adj(eventData.oom_score_adj);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(oom_score_adj_update, ParseOomScoreAdjUpdate<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(oom_score_adj_update, ParseOomScoreAdjUpdate<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
