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
struct SchedKthreadStopData {
    std::string comm {};
    int32_t pid {};
};
__attribute__((noinline)) void ParseSchedKthreadStopData(SchedKthreadStopData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedKthreadStop(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedKthreadStopData eventData {};
    ParseSchedKthreadStopData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop, ParseSchedKthreadStop<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop, ParseSchedKthreadStop<ProtoEncoder::FtraceEvent>);

struct SchedKthreadStopRetData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseSchedKthreadStopRetData(SchedKthreadStopRetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedKthreadStopRet(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedKthreadStopRetData eventData {};
    ParseSchedKthreadStopRetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop_ret, ParseSchedKthreadStopRet<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop_ret, ParseSchedKthreadStopRet<ProtoEncoder::FtraceEvent>);

struct SchedMigrateTaskData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
    int32_t orig_cpu {};
    int32_t dest_cpu {};
};
__attribute__((noinline)) void ParseSchedMigrateTaskData(SchedMigrateTaskData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.orig_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dest_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedMigrateTask(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedMigrateTaskData eventData {};
    ParseSchedMigrateTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_migrate_task_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_orig_cpu(eventData.orig_cpu);
    msg->set_dest_cpu(eventData.dest_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_migrate_task, ParseSchedMigrateTask<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_migrate_task, ParseSchedMigrateTask<ProtoEncoder::FtraceEvent>);

struct SchedMoveNumaData {
    int32_t pid {};
    int32_t tgid {};
    int32_t ngid {};
    int32_t src_cpu {};
    int32_t src_nid {};
    int32_t dst_cpu {};
    int32_t dst_nid {};
};
__attribute__((noinline)) void ParseSchedMoveNumaData(SchedMoveNumaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.tgid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ngid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedMoveNuma(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedMoveNumaData eventData {};
    ParseSchedMoveNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_move_numa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pid(eventData.pid);
    msg->set_tgid(eventData.tgid);
    msg->set_ngid(eventData.ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_move_numa, ParseSchedMoveNuma<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_move_numa, ParseSchedMoveNuma<ProtoEncoder::FtraceEvent>);

struct SchedPiSetprioData {
    std::string comm {};
    int32_t pid {};
    int32_t oldprio {};
    int32_t newprio {};
};
__attribute__((noinline)) void ParseSchedPiSetprioData(SchedPiSetprioData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.oldprio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.newprio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedPiSetprio(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedPiSetprioData eventData {};
    ParseSchedPiSetprioData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_oldprio(eventData.oldprio);
    msg->set_newprio(eventData.newprio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_pi_setprio, ParseSchedPiSetprio<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_pi_setprio, ParseSchedPiSetprio<ProtoEncoder::FtraceEvent>);

struct SchedProcessExecData {
    std::string filename {};
    int32_t pid {};
    int32_t old_pid {};
};
__attribute__((noinline)) void ParseSchedProcessExecData(SchedProcessExecData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.filename = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.old_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedProcessExec(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedProcessExecData eventData {};
    ParseSchedProcessExecData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exec_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_filename(eventData.filename);
    msg->set_pid(eventData.pid);
    msg->set_old_pid(eventData.old_pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exec, ParseSchedProcessExec<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exec, ParseSchedProcessExec<ProtoEncoder::FtraceEvent>);

struct SchedProcessExitData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseSchedProcessExitData(SchedProcessExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedProcessExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedProcessExitData eventData {};
    ParseSchedProcessExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exit, ParseSchedProcessExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exit, ParseSchedProcessExit<ProtoEncoder::FtraceEvent>);

struct SchedProcessForkData {
    std::string parent_comm {};
    int32_t parent_pid {};
    std::string child_comm {};
    int32_t child_pid {};
};
__attribute__((noinline)) void ParseSchedProcessForkData(SchedProcessForkData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.parent_comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.parent_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.child_comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.child_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedProcessFork(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedProcessForkData eventData {};
    ParseSchedProcessForkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_fork_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_parent_comm(eventData.parent_comm);
    msg->set_parent_pid(eventData.parent_pid);
    msg->set_child_comm(eventData.child_comm);
    msg->set_child_pid(eventData.child_pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_fork, ParseSchedProcessFork<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_fork, ParseSchedProcessFork<ProtoEncoder::FtraceEvent>);

struct SchedProcessFreeData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseSchedProcessFreeData(SchedProcessFreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedProcessFree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedProcessFreeData eventData {};
    ParseSchedProcessFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_free_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_free, ParseSchedProcessFree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_free, ParseSchedProcessFree<ProtoEncoder::FtraceEvent>);

struct SchedProcessWaitData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseSchedProcessWaitData(SchedProcessWaitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedProcessWait(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedProcessWaitData eventData {};
    ParseSchedProcessWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_wait_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_wait, ParseSchedProcessWait<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_wait, ParseSchedProcessWait<ProtoEncoder::FtraceEvent>);

struct SchedStatBlockedData {
    std::string comm {};
    int32_t pid {};
    uint64_t delay {};
};
__attribute__((noinline)) void ParseSchedStatBlockedData(SchedStatBlockedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.delay = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedStatBlocked(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStatBlockedData eventData {};
    ParseSchedStatBlockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_blocked, ParseSchedStatBlocked<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_blocked, ParseSchedStatBlocked<ProtoEncoder::FtraceEvent>);

struct SchedStatIowaitData {
    std::string comm {};
    int32_t pid {};
    uint64_t delay {};
};
__attribute__((noinline)) void ParseSchedStatIowaitData(SchedStatIowaitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.delay = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedStatIowait(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStatIowaitData eventData {};
    ParseSchedStatIowaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_iowait, ParseSchedStatIowait<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_iowait, ParseSchedStatIowait<ProtoEncoder::FtraceEvent>);

struct SchedStatRuntimeData {
    std::string comm {};
    int32_t pid {};
    uint64_t runtime {};
    uint64_t vruntime {};
    bool has_vruntime {};
};
__attribute__((noinline)) void ParseSchedStatRuntimeData(SchedStatRuntimeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.runtime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    if (format.osVersion.compare("6.6.76") < 0) {
        eventData.vruntime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
        eventData.has_vruntime = true;
    }
}
template <typename FtraceEventType>
void ParseSchedStatRuntime(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStatRuntimeData eventData {};
    ParseSchedStatRuntimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_runtime(eventData.runtime);
    if (eventData.has_vruntime) {
        msg->set_vruntime(eventData.vruntime);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_runtime, ParseSchedStatRuntime<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_runtime, ParseSchedStatRuntime<ProtoEncoder::FtraceEvent>);

struct SchedStatSleepData {
    std::string comm {};
    int32_t pid {};
    uint64_t delay {};
};
__attribute__((noinline)) void ParseSchedStatSleepData(SchedStatSleepData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.delay = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedStatSleep(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStatSleepData eventData {};
    ParseSchedStatSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_sleep, ParseSchedStatSleep<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_sleep, ParseSchedStatSleep<ProtoEncoder::FtraceEvent>);

struct SchedStatWaitData {
    std::string comm {};
    int32_t pid {};
    uint64_t delay {};
};
__attribute__((noinline)) void ParseSchedStatWaitData(SchedStatWaitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.delay = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedStatWait(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStatWaitData eventData {};
    ParseSchedStatWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_wait_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_wait, ParseSchedStatWait<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_wait, ParseSchedStatWait<ProtoEncoder::FtraceEvent>);

struct SchedStickNumaData {
    int32_t src_pid {};
    int32_t src_tgid {};
    int32_t src_ngid {};
    int32_t src_cpu {};
    int32_t src_nid {};
    int32_t dst_pid {};
    int32_t dst_tgid {};
    int32_t dst_ngid {};
    int32_t dst_cpu {};
    int32_t dst_nid {};
};
__attribute__((noinline)) void ParseSchedStickNumaData(SchedStickNumaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.src_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_tgid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_ngid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_tgid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_ngid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedStickNuma(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedStickNumaData eventData {};
    ParseSchedStickNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stick_numa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_src_pid(eventData.src_pid);
    msg->set_src_tgid(eventData.src_tgid);
    msg->set_src_ngid(eventData.src_ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_pid(eventData.dst_pid);
    msg->set_dst_tgid(eventData.dst_tgid);
    msg->set_dst_ngid(eventData.dst_ngid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stick_numa, ParseSchedStickNuma<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stick_numa, ParseSchedStickNuma<ProtoEncoder::FtraceEvent>);

struct SchedSwapNumaData {
    int32_t src_pid {};
    int32_t src_tgid {};
    int32_t src_ngid {};
    int32_t src_cpu {};
    int32_t src_nid {};
    int32_t dst_pid {};
    int32_t dst_tgid {};
    int32_t dst_ngid {};
    int32_t dst_cpu {};
    int32_t dst_nid {};
};
__attribute__((noinline)) void ParseSchedSwapNumaData(SchedSwapNumaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.src_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_tgid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_ngid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_tgid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_ngid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedSwapNuma(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedSwapNumaData eventData {};
    ParseSchedSwapNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_swap_numa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_src_pid(eventData.src_pid);
    msg->set_src_tgid(eventData.src_tgid);
    msg->set_src_ngid(eventData.src_ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_pid(eventData.dst_pid);
    msg->set_dst_tgid(eventData.dst_tgid);
    msg->set_dst_ngid(eventData.dst_ngid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_swap_numa, ParseSchedSwapNuma<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_swap_numa, ParseSchedSwapNuma<ProtoEncoder::FtraceEvent>);

struct SchedSwitchData {
    std::string prev_comm {};
    int32_t prev_pid {};
    int32_t prev_prio {};
    uint64_t prev_state {};
    std::string next_comm {};
    int32_t next_pid {};
    int32_t next_prio {};
};
__attribute__((noinline)) void ParseSchedSwitchData(SchedSwitchData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.prev_comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.prev_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prev_prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prev_state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.next_comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.next_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.next_prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedSwitch(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedSwitchData eventData {};
    ParseSchedSwitchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_switch_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_prev_comm(eventData.prev_comm);
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_comm(eventData.next_comm);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_prio(eventData.next_prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_switch, ParseSchedSwitch<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_switch, ParseSchedSwitch<ProtoEncoder::FtraceEvent>);

struct SchedWaitTaskData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseSchedWaitTaskData(SchedWaitTaskData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedWaitTask(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedWaitTaskData eventData {};
    ParseSchedWaitTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wait_task_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wait_task, ParseSchedWaitTask<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wait_task, ParseSchedWaitTask<ProtoEncoder::FtraceEvent>);

struct SchedWakeIdleWithoutIpiData {
    int32_t cpu {};
};
__attribute__((noinline)) void ParseSchedWakeIdleWithoutIpiData(SchedWakeIdleWithoutIpiData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedWakeIdleWithoutIpi(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedWakeIdleWithoutIpiData eventData {};
    ParseSchedWakeIdleWithoutIpiData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cpu(eventData.cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wake_idle_without_ipi, ParseSchedWakeIdleWithoutIpi<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wake_idle_without_ipi, ParseSchedWakeIdleWithoutIpi<ProtoEncoder::FtraceEvent>);

struct SchedWakeupData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
    int32_t target_cpu {};
};
__attribute__((noinline)) void ParseSchedWakeupData(SchedWakeupData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.target_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedWakeup(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedWakeupData eventData {};
    ParseSchedWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup, ParseSchedWakeup<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup, ParseSchedWakeup<ProtoEncoder::FtraceEvent>);

struct SchedWakeupNewData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
    int32_t target_cpu {};
};
__attribute__((noinline)) void ParseSchedWakeupNewData(SchedWakeupNewData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.target_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedWakeupNew(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedWakeupNewData eventData {};
    ParseSchedWakeupNewData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup_new, ParseSchedWakeupNew<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup_new, ParseSchedWakeupNew<ProtoEncoder::FtraceEvent>);

struct SchedWakingData {
    std::string comm {};
    int32_t pid {};
    int32_t prio {};
    int32_t target_cpu {};
};
__attribute__((noinline)) void ParseSchedWakingData(SchedWakingData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.target_cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSchedWaking(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SchedWakingData eventData {};
    ParseSchedWakingData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_waking_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_waking, ParseSchedWaking<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_waking, ParseSchedWaking<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
