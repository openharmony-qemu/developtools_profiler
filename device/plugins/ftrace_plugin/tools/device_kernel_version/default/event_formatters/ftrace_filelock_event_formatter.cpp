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
    break_lease_block,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_break_lease_block_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.break_lease_block_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "break_lease_block: fl=%p dev=0x%x:0x%x ino=0x%" PRIx64
            " fl_blocker=%p fl_owner=%p fl_flags=%s fl_type=%s fl_break_time=%" PRIu64 " fl_downgrade_time=%" PRIu64 "",
            msg.fl(), ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))),
            msg.i_ino(), msg.fl_next(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}), msg.fl_break_time(),
            msg.fl_downgrade_time());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(break_lease_block) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    break_lease_noblock,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_break_lease_noblock_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.break_lease_noblock_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "break_lease_noblock: fl=%p dev=0x%x:0x%x ino=0x%" PRIx64
            " fl_blocker=%p fl_owner=%p fl_flags=%s fl_type=%s fl_break_time=%" PRIu64 " fl_downgrade_time=%" PRIu64 "",
            msg.fl(), ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))),
            msg.i_ino(), msg.fl_next(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}), msg.fl_break_time(),
            msg.fl_downgrade_time());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(break_lease_noblock) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    break_lease_unblock,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_break_lease_unblock_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.break_lease_unblock_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "break_lease_unblock: fl=%p dev=0x%x:0x%x ino=0x%" PRIx64
            " fl_blocker=%p fl_owner=%p fl_flags=%s fl_type=%s fl_break_time=%" PRIu64 " fl_downgrade_time=%" PRIu64 "",
            msg.fl(), ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))),
            msg.i_ino(), msg.fl_next(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}), msg.fl_break_time(),
            msg.fl_downgrade_time());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(break_lease_unblock) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    generic_add_lease,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_generic_add_lease_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.generic_add_lease_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "generic_add_lease: dev=0x%x:0x%x ino=0x%" PRIx64
            " wcount=%d rcount=%d icount=%d fl_owner=%p fl_flags=%s fl_type=%s",
            ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(),
            msg.wcount(), msg.dcount(), msg.icount(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(generic_add_lease) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    generic_delete_lease,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_generic_delete_lease_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.generic_delete_lease_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "generic_delete_lease: fl=%p dev=0x%x:0x%x ino=0x%" PRIx64
            " fl_blocker=%p fl_owner=%p fl_flags=%s fl_type=%s fl_break_time=%" PRIu64 " fl_downgrade_time=%" PRIu64 "",
            msg.fl(), ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))),
            msg.i_ino(), msg.fl_next(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}), msg.fl_break_time(),
            msg.fl_downgrade_time());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(generic_delete_lease) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    time_out_leases,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_time_out_leases_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.time_out_leases_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "time_out_leases: fl=%p dev=0x%x:0x%x ino=0x%" PRIx64
            " fl_blocker=%p fl_owner=%p fl_flags=%s fl_type=%s fl_break_time=%" PRIu64 " fl_downgrade_time=%" PRIu64 "",
            msg.fl(), ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))),
            msg.i_ino(), msg.fl_next(), msg.fl_owner(),
            __print_flags(msg.fl_flags(), "|", {1, "FL_POSIX"}, {2, "FL_FLOCK"}, {4, "FL_DELEG"}, {8, "FL_ACCESS"},
                {16, "FL_EXISTS"}, {32, "FL_LEASE"}, {64, "FL_CLOSE"}, {128, "FL_SLEEP"}, {256, "FL_DOWNGRADE_PENDING"},
                {512, "FL_UNLOCK_PENDING"}, {1024, "FL_OFDLCK"}),
            __print_symbolic(msg.fl_type(), {0, "F_RDLCK"}, {1, "F_WRLCK"}, {2, "F_UNLCK"}), msg.fl_break_time(),
            msg.fl_downgrade_time());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(time_out_leases) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
