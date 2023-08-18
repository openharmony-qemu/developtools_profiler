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
    sched_blocked_reason,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_blocked_reason_format();
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_caller(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_io_wait(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_kthread_stop,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_kthread_stop_ret,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
        msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_migrate_task,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_migrate_task_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_orig_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dest_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_move_numa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_move_numa_format();
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_tgid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ngid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_pi_setprio,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_oldprio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_newprio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_process_exec,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_process_exec_format();
        msg->set_filename(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_old_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_process_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_process_exit_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_process_fork,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_process_fork_format();
        msg->set_parent_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_parent_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_child_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_child_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_process_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_process_free_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_process_wait,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_process_wait_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stat_blocked,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stat_iowait,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stat_runtime,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_runtime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vruntime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stat_sleep,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stat_wait,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stat_wait_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_stick_numa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_stick_numa_format();
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_tgid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ngid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_swap_numa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_swap_numa_format();
        msg->set_src_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_tgid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_ngid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_src_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_tgid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_ngid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_switch,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_switch_format();
        msg->set_prev_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_prev_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prev_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prev_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_next_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_next_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_next_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_wait_task,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_wait_task_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_wake_idle_without_ipi,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
        msg->set_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_wakeup,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_wakeup_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_success(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_wakeup_new,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_success(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    sched_waking,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_sched_waking_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_success(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
