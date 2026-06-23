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
struct TaskNewtaskData {
    int32_t pid {};
    std::string comm {};
    uint64_t clone_flags {};
    int32_t oom_score_adj {};
};
__attribute__((noinline)) void ParseTaskNewtaskData(TaskNewtaskData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.clone_flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.oom_score_adj = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseTaskNewtask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TaskNewtaskData eventData {};
    ParseTaskNewtaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_task_newtask_format();
    msg->set_pid(eventData.pid);
    msg->set_comm(eventData.comm);
    msg->set_clone_flags(eventData.clone_flags);
    msg->set_oom_score_adj(eventData.oom_score_adj);
}
void ParseTaskNewtaskEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TaskNewtaskData eventData {};
    ParseTaskNewtaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_task_newtask_format();
    msg->set_pid(eventData.pid);
    msg->set_comm(eventData.comm);
    msg->set_clone_flags(eventData.clone_flags);
    msg->set_oom_score_adj(eventData.oom_score_adj);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(task_newtask, ParseTaskNewtask);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(task_newtask, ParseTaskNewtaskEncoder);

struct TaskRenameData {
    int32_t pid {};
    std::string oldcomm {};
    std::string newcomm {};
    int32_t oom_score_adj {};
};
__attribute__((noinline)) void ParseTaskRenameData(TaskRenameData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.oldcomm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.newcomm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.oom_score_adj = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseTaskRename(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TaskRenameData eventData {};
    ParseTaskRenameData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_task_rename_format();
    msg->set_pid(eventData.pid);
    msg->set_oldcomm(eventData.oldcomm);
    msg->set_newcomm(eventData.newcomm);
    msg->set_oom_score_adj(eventData.oom_score_adj);
}
void ParseTaskRenameEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TaskRenameData eventData {};
    ParseTaskRenameData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_task_rename_format();
    msg->set_pid(eventData.pid);
    msg->set_oldcomm(eventData.oldcomm);
    msg->set_newcomm(eventData.newcomm);
    msg->set_oom_score_adj(eventData.oom_score_adj);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(task_rename, ParseTaskRename);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(task_rename, ParseTaskRenameEncoder);
} // namespace
FTRACE_NS_END
