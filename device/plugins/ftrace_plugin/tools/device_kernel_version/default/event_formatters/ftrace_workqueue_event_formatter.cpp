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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    workqueue_activate_work,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_workqueue_activate_work_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.workqueue_activate_work_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "workqueue_activate_work: work struct %p", msg.work());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(workqueue_activate_work) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    workqueue_execute_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_workqueue_execute_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.workqueue_execute_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "workqueue_execute_end: work struct %p", msg.work());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(workqueue_execute_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    workqueue_execute_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_workqueue_execute_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.workqueue_execute_start_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "workqueue_execute_start: work struct %p: function %s", msg.work(), functionStr.c_str());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "workqueue_execute_start: work struct %p: function %p", msg.work(), msg.function());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(workqueue_execute_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    workqueue_queue_work,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_workqueue_queue_work_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.workqueue_queue_work_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "workqueue_queue_work: work struct=%p function=%s workqueue=%p req_cpu=%u cpu=%u", msg.work(),
                functionStr.c_str(), msg.workqueue(), msg.req_cpu(), msg.cpu());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "workqueue_queue_work: work struct=%p function=%p workqueue=%p req_cpu=%u cpu=%u", msg.work(),
                msg.function(), msg.workqueue(), msg.req_cpu(), msg.cpu());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(workqueue_queue_work) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
