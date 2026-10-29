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
    hmfs_read_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hmfs_read_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hmfs_read_enter_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hmfs_read_enter: dev=%d,%d ino=%" PRIu64
                " offset=%" PRIu64 " size=%" PRIu64 " i_size=%" PRIu64 " entry_name=%s",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.ino(), msg.off(), msg.size(), msg.i_size(),
                ((char *)((void *)((char *)REC + (REC->__data_loc_name & 0xffff)))));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hmfs_read_enter) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE,
                "maybe, the contents of print event(hmfs_read_enter) msg snprintf_s error");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hmfs_read_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hmfs_read_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hmfs_read_exit_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hmfs_read_exit: dev=%d,%d ino=%" PRIu64
                " offset=%" PRIu64 " size=%" PRIu64 " res=%" PRId64 "", ((unsigned int)((msg.dev()) >> 20)),
                ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.off(), msg.size(), msg.res());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hmfs_read_exit) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE,
                "maybe, the contents of print event(hmfs_read_exit) msg snprintf_s error");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hmfs_write_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hmfs_write_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hmfs_write_enter_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hmfs_write_enter: dev=%d,%d ino=%" PRIu64
                " offset=%" PRIu64 " size=%" PRIu64 " i_size=%" PRIu64 " entry_name=%s",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.ino(), msg.off(), msg.size(), msg.i_size(),
                ((char *)((void *)((char *)REC + (REC->__data_loc_name & 0xffff)))));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hmfs_write_enter) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE,
                "maybe, the contents of print event(hmfs_write_enter) msg snprintf_s error");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hmfs_write_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hmfs_write_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hmfs_write_exit_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hmfs_write_exit: dev=%d,%d ino=%" PRIu64
                " offset=%" PRIu64 " size=%" PRIu64 " res=%" PRId64 "", ((unsigned int)((msg.dev()) >> 20)),
                ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.off(), msg.size(), msg.res());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hmfs_write_exit) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE,
                "maybe, the contents of print event(hmfs_write_exit) msg snprintf_s error");
            return "";
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
