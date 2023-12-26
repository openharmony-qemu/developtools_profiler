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
    task_newtask,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_task_newtask_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.task_newtask_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "task_newtask: pid=%d comm=%s clone_flags=%" PRIx64 " oom_score_adj=%d", msg.pid(), msg.comm().c_str(),
            msg.clone_flags(), msg.oom_score_adj());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(task_newtask) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    task_rename,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_task_rename_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.task_rename_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "task_rename: pid=%d oldcomm=%s newcomm=%s oom_score_adj=%d", msg.pid(), msg.oldcomm().c_str(),
            msg.newcomm().c_str(), msg.oom_score_adj());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(task_rename) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
