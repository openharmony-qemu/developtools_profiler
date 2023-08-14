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
    sched_blocked_reason,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_blocked_reason_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_blocked_reason_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_blocked_reason: pid=%d iowait=%d caller=%p",
            msg.pid(), msg.io_wait(), msg.caller());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_blocked_reason) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_kthread_stop,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_kthread_stop_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_kthread_stop_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_kthread_stop: comm=%s pid=%d", msg.comm().c_str(), msg.pid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_kthread_stop) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_kthread_stop_ret,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_kthread_stop_ret_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_kthread_stop_ret_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_kthread_stop_ret: ret=%d", msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_kthread_stop_ret) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_migrate_task,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_migrate_task_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_migrate_task_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_migrate_task: comm=%s pid=%d prio=%d orig_cpu=%d dest_cpu=%d", msg.comm().c_str(), msg.pid(),
            msg.prio(), msg.orig_cpu(), msg.dest_cpu());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_migrate_task) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_move_numa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_move_numa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_move_numa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_move_numa: pid=%d tgid=%d ngid=%d src_cpu=%d src_nid=%d dst_cpu=%d dst_nid=%d", msg.pid(),
            msg.tgid(), msg.ngid(), msg.src_cpu(), msg.src_nid(), msg.dst_cpu(), msg.dst_nid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_move_numa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_pi_setprio,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_pi_setprio_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_pi_setprio_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_pi_setprio: comm=%s pid=%d oldprio=%d newprio=%d",
                msg.comm().c_str(), msg.pid(), msg.oldprio(), msg.newprio());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_pi_setprio) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_process_exec,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_process_exec_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_process_exec_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_process_exec: filename=%s pid=%d old_pid=%d",
            msg.filename().c_str(), msg.pid(), msg.old_pid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_process_exec) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_process_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_process_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_process_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_process_exit: comm=%s pid=%d prio=%d",
            msg.comm().c_str(), msg.pid(), msg.prio());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_process_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_process_fork,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_process_fork_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_process_fork_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_process_fork: comm=%s pid=%d child_comm=%s child_pid=%d", msg.parent_comm().c_str(),
            msg.parent_pid(), msg.child_comm().c_str(), msg.child_pid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_process_fork) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_process_free,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_process_free_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_process_free_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_process_free: comm=%s pid=%d prio=%d",
            msg.comm().c_str(), msg.pid(), msg.prio());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_process_free) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_process_wait,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_process_wait_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_process_wait_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_process_wait: comm=%s pid=%d prio=%d",
            msg.comm().c_str(), msg.pid(), msg.prio());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_process_wait) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stat_blocked,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stat_blocked_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stat_blocked_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stat_blocked: comm=%s pid=%d delay=%" PRIu64 " [ns]", msg.comm().c_str(), msg.pid(), msg.delay());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_stat_blocked) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stat_iowait,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stat_iowait_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stat_iowait_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stat_iowait: comm=%s pid=%d delay=%" PRIu64 " [ns]", msg.comm().c_str(), msg.pid(), msg.delay());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_stat_iowait) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stat_runtime,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stat_runtime_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stat_runtime_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stat_runtime: comm=%s pid=%d runtime=%" PRIu64 " [ns] vruntime=%" PRIu64 " [ns]", msg.comm().c_str(),
            msg.pid(), msg.runtime(), msg.vruntime());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(sched_stat_runtime) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stat_sleep,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stat_sleep_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stat_sleep_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stat_sleep: comm=%s pid=%d delay=%" PRIu64 " [ns]", msg.comm().c_str(), msg.pid(), msg.delay());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_stat_sleep) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stat_wait,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stat_wait_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stat_wait_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stat_wait: comm=%s pid=%d delay=%" PRIu64 " [ns]", msg.comm().c_str(), msg.pid(), msg.delay());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_stat_wait) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_stick_numa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_stick_numa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_stick_numa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_stick_numa: src_pid=%d src_tgid=%d src_ngid=%d src_cpu=%d src_nid=%d dst_pid=%d dst_tgid=%d "
            "dst_ngid=%d dst_cpu=%d dst_nid=%d",
            msg.pid(), msg.tgid(), msg.ngid(), msg.src_cpu(), msg.src_nid(), msg.pid(), msg.tgid(), msg.ngid(),
            msg.dst_cpu(), msg.dst_nid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_stick_numa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_swap_numa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_swap_numa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_swap_numa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "sched_swap_numa: src_pid=%d src_tgid=%d src_ngid=%d src_cpu=%d src_nid=%d dst_pid=%d dst_tgid=%d "
            "dst_ngid=%d dst_cpu=%d dst_nid=%d",
            msg.src_pid(), msg.src_tgid(), msg.src_ngid(), msg.src_cpu(), msg.src_nid(), msg.dst_pid(), msg.dst_tgid(),
            msg.dst_ngid(), msg.dst_cpu(), msg.dst_nid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_swap_numa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_switch,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_switch_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_switch_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        if (msg.prev_state() > 0x0400) {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "sched_switch: prev_comm=%s prev_pid=%d prev_prio=%d prev_state=? ==> next_comm=%s next_pid=%d "
                "next_prio=%d",
                msg.prev_comm().c_str(), msg.prev_pid(), msg.prev_prio(), msg.next_comm().c_str(), msg.next_pid(),
                msg.next_prio());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "sched_switch: prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d "
                "next_prio=%d",
                msg.prev_comm().c_str(), msg.prev_pid(), msg.prev_prio(),
                (msg.prev_state() &
                    ((((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1) - 1))
                    ? __print_flags(
                          msg.prev_state() &
                              ((((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1) -
                                  1),
                          "|", {0x0001, "S"}, {0x0002, "D"}, {0x0004, "T"}, {0x0008, "t"}, {0x0010, "X"}, {0x0020, "Z"},
                          {0x0040, "I"}, {0x0080, "K"}, {0x0100, "W"}, {0x0200, "P"}, {0x0400, "N"})
                    : "R",
                msg.prev_state() & (((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1)
                    ? "+"
                    : "",
                msg.next_comm().c_str(), msg.next_pid(), msg.next_prio());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_switch) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_wait_task,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_wait_task_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_wait_task_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_wait_task: comm=%s pid=%d prio=%d",
            msg.comm().c_str(), msg.pid(), msg.prio());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_wait_task) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_wake_idle_without_ipi,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_wake_idle_without_ipi_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_wake_idle_without_ipi_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_wake_idle_without_ipi: cpu=%d", msg.cpu());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(sched_wake_idle_without_ipi) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_wakeup,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_wakeup_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_wakeup_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_wakeup: comm=%s pid=%d prio=%d target_cpu=%03d",
                msg.comm().c_str(), msg.pid(), msg.prio(), msg.target_cpu());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_wakeup) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_wakeup_new,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_wakeup_new_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_wakeup_new_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_wakeup_new: comm=%s pid=%d prio=%d target_cpu=%03d",
                msg.comm().c_str(), msg.pid(), msg.prio(), msg.target_cpu());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_wakeup_new) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    sched_waking,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_sched_waking_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.sched_waking_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_waking: comm=%s pid=%d prio=%d target_cpu=%03d",
                msg.comm().c_str(), msg.pid(), msg.prio(), msg.target_cpu());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(sched_waking) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
