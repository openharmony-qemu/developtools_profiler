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
using namespace OHOS::Developtools::Profiler;
template <typename T>
void sched_kthread_stop_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
    sched_kthread_stop_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
                                                sched_kthread_stop_func(msg, data, size, format);
                                            });

template <typename T>
void sched_kthread_stop_ret_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop_ret, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
    sched_kthread_stop_ret_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop_ret,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
                                                sched_kthread_stop_ret_func(msg, data, size, format);
                                            });

template <typename T>
void sched_migrate_task_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_orig_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dest_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_migrate_task, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_migrate_task_format();
    sched_migrate_task_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_migrate_task,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_migrate_task_format();
    sched_migrate_task_func(msg, data, size, format);
});

template <typename T>
void sched_move_numa_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_tgid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ngid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_src_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_src_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dst_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dst_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_move_numa, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_move_numa_format();
    sched_move_numa_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_move_numa, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_move_numa_format();
    sched_move_numa_func(msg, data, size, format);
});

template <typename T>
void sched_pi_setprio_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_oldprio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_newprio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_pi_setprio, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
    sched_pi_setprio_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_pi_setprio,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
    sched_pi_setprio_func(msg, data, size, format);
});

template <typename T>
void sched_process_exec_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_filename(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_old_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exec, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_exec_format();
    sched_process_exec_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exec,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_exec_format();
    sched_process_exec_func(msg, data, size, format);
});

template <typename T>
void sched_process_exit_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exit, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_exit_format();
    sched_process_exit_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exit,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_exit_format();
    sched_process_exit_func(msg, data, size, format);
});

template <typename T>
void sched_process_fork_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_parent_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_parent_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_child_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_child_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_fork, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_fork_format();
    sched_process_fork_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_fork,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_fork_format();
    sched_process_fork_func(msg, data, size, format);
});

template <typename T>
void sched_process_free_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_free, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_free_format();
    sched_process_free_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_free,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_free_format();
    sched_process_free_func(msg, data, size, format);
});

template <typename T>
void sched_process_wait_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_wait, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_wait_format();
    sched_process_wait_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_wait,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_process_wait_format();
    sched_process_wait_func(msg, data, size, format);
});

template <typename T>
void sched_stat_blocked_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_blocked, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
    sched_stat_blocked_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_blocked,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
    sched_stat_blocked_func(msg, data, size, format);
});

template <typename T>
void sched_stat_iowait_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_iowait, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
    sched_stat_iowait_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_iowait,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
    sched_stat_iowait_func(msg, data, size, format);
});

template <typename T>
void sched_stat_runtime_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_runtime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    if (format.osVersion.compare("6.6.76") < 0) {
        msg->set_vruntime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_runtime, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
    sched_stat_runtime_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_runtime,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
    sched_stat_runtime_func(msg, data, size, format);
});

template <typename T>
void sched_stat_sleep_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_sleep, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
    sched_stat_sleep_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_sleep,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
    sched_stat_sleep_func(msg, data, size, format);
});

template <typename T>
void sched_stat_wait_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_delay(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_wait, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_wait_format();
    sched_stat_wait_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_wait, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stat_wait_format();
    sched_stat_wait_func(msg, data, size, format);
});

template <typename T>
void sched_stick_numa_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
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
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stick_numa, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stick_numa_format();
    sched_stick_numa_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stick_numa,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_stick_numa_format();
    sched_stick_numa_func(msg, data, size, format);
});

template <typename T>
void sched_swap_numa_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
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
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_swap_numa, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_swap_numa_format();
    sched_swap_numa_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_swap_numa, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_swap_numa_format();
    sched_swap_numa_func(msg, data, size, format);
});

template <typename T>
void sched_switch_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_prev_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_prev_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prev_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prev_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_next_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_next_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_next_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_switch, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_switch_format();
    sched_switch_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_switch, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_switch_format();
    sched_switch_func(msg, data, size, format);
});

template <typename T>
void sched_wait_task_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wait_task, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wait_task_format();
    sched_wait_task_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wait_task, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wait_task_format();
    sched_wait_task_func(msg, data, size, format);
});

template <typename T>
void sched_wake_idle_without_ipi_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wake_idle_without_ipi, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                     size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
    sched_wake_idle_without_ipi_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wake_idle_without_ipi,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
    sched_wake_idle_without_ipi_func(msg, data, size, format);
});

template <typename T>
void sched_wakeup_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wakeup_format();
    sched_wakeup_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wakeup_format();
    sched_wakeup_func(msg, data, size, format);
});

template <typename T>
void sched_wakeup_new_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup_new, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
    sched_wakeup_new_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup_new,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
    sched_wakeup_new_func(msg, data, size, format);
});

template <typename T>
void sched_waking_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_target_cpu(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_waking, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_waking_format();
    sched_waking_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_waking, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_sched_waking_format();
    sched_waking_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
