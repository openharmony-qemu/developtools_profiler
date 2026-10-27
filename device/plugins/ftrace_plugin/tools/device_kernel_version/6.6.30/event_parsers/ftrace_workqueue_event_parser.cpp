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
struct WorkqueueActivateWorkData {
    uint64_t work {};
};
__attribute__((noinline)) void ParseWorkqueueActivateWorkData(WorkqueueActivateWorkData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.work = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWorkqueueActivateWork(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WorkqueueActivateWorkData eventData {};
    ParseWorkqueueActivateWorkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_workqueue_activate_work_format();
    msg->set_work(eventData.work);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(workqueue_activate_work, ParseWorkqueueActivateWork<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(workqueue_activate_work, ParseWorkqueueActivateWork<ProtoEncoder::FtraceEvent>);

struct WorkqueueExecuteEndData {
    uint64_t work {};
    uint64_t function {};
};
__attribute__((noinline)) void ParseWorkqueueExecuteEndData(WorkqueueExecuteEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.work = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWorkqueueExecuteEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WorkqueueExecuteEndData eventData {};
    ParseWorkqueueExecuteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_workqueue_execute_end_format();
    msg->set_work(eventData.work);
    msg->set_function(eventData.function);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(workqueue_execute_end, ParseWorkqueueExecuteEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(workqueue_execute_end, ParseWorkqueueExecuteEnd<ProtoEncoder::FtraceEvent>);

struct WorkqueueExecuteStartData {
    uint64_t work {};
    uint64_t function {};
};
__attribute__((noinline)) void ParseWorkqueueExecuteStartData(WorkqueueExecuteStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.work = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWorkqueueExecuteStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WorkqueueExecuteStartData eventData {};
    ParseWorkqueueExecuteStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_workqueue_execute_start_format();
    msg->set_work(eventData.work);
    msg->set_function(eventData.function);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(workqueue_execute_start, ParseWorkqueueExecuteStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(workqueue_execute_start, ParseWorkqueueExecuteStart<ProtoEncoder::FtraceEvent>);

struct WorkqueueQueueWorkData {
    uint64_t work {};
    uint64_t function {};
    std::string workqueue_new {};
    int32_t req_cpu {};
    int32_t cpu {};
};
__attribute__((noinline)) void ParseWorkqueueQueueWorkData(WorkqueueQueueWorkData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.work = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.workqueue_new = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.req_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWorkqueueQueueWork(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WorkqueueQueueWorkData eventData {};
    ParseWorkqueueQueueWorkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_workqueue_queue_work_format();
    msg->set_work(eventData.work);
    msg->set_function(eventData.function);
    msg->set_workqueue_new(eventData.workqueue_new);
    msg->set_req_cpu(eventData.req_cpu);
    msg->set_cpu(eventData.cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(workqueue_queue_work, ParseWorkqueueQueueWork<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(workqueue_queue_work, ParseWorkqueueQueueWork<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
