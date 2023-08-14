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
    workqueue_activate_work,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_workqueue_activate_work_format();
        msg->set_work(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    workqueue_execute_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_workqueue_execute_end_format();
        msg->set_work(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    workqueue_execute_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_workqueue_execute_start_format();
        msg->set_work(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    workqueue_queue_work,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_workqueue_queue_work_format();
        msg->set_work(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_function(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_workqueue(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_req_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
