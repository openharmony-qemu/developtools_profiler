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
#include "string_utils.h"

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct SchedBlockedReasonData {
    int32_t pid {};
    uint64_t caller {};
    uint32_t io_wait {};
    std::string caller_str {};
    bool has_caller_str {};
};
__attribute__((noinline)) void ParseSchedBlockedReasonData(SchedBlockedReasonData& eventData, uint8_t data[],
                                             size_t size, const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.caller = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.io_wait = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    uint64_t recOffset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, ++i, data, size);
    uint64_t recSize = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, ++i, data, size);
    std::string funcName = FtraceFieldParser::ParseStrField(format.fields, ++i, data, size);
    if (funcName.length() == 0) {
        return;
    }
    std::string modName = FtraceFieldParser::ParseStrField(format.fields, ++i, data, size);
    eventData.caller_str = funcName + "+0x" + StringUtils::ConvertToHex(recOffset) + "/0x"
                           + StringUtils::ConvertToHex(recSize) + "[" + modName + "]";
    eventData.has_caller_str = true;
}
void ParseSchedBlockedReason(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedBlockedReasonData eventData {};
    ParseSchedBlockedReasonData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_blocked_reason_format();
    msg->set_pid(eventData.pid);
    msg->set_caller(eventData.caller);
    msg->set_io_wait(eventData.io_wait);
    if (eventData.has_caller_str) {
        msg->set_caller_str(eventData.caller_str);
    }
}
void ParseSchedBlockedReasonEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedBlockedReasonData eventData {};
    ParseSchedBlockedReasonData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_blocked_reason_format();
    msg->set_pid(eventData.pid);
    msg->set_caller(eventData.caller);
    msg->set_io_wait(eventData.io_wait);
    if (eventData.has_caller_str) {
        msg->set_caller_str(eventData.caller_str);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_blocked_reason, ParseSchedBlockedReason);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_blocked_reason, ParseSchedBlockedReasonEncoder);

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
void ParseSchedKthreadStop(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedKthreadStopData eventData {};
    ParseSchedKthreadStopData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
}
void ParseSchedKthreadStopEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedKthreadStopData eventData {};
    ParseSchedKthreadStopData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop, ParseSchedKthreadStop);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop, ParseSchedKthreadStopEncoder);

struct SchedKthreadStopRetData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseSchedKthreadStopRetData(SchedKthreadStopRetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseSchedKthreadStopRet(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedKthreadStopRetData eventData {};
    ParseSchedKthreadStopRetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
    msg->set_ret(eventData.ret);
}
void ParseSchedKthreadStopRetEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedKthreadStopRetData eventData {};
    ParseSchedKthreadStopRetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_kthread_stop_ret_format();
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_kthread_stop_ret, ParseSchedKthreadStopRet);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_kthread_stop_ret, ParseSchedKthreadStopRetEncoder);

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
void ParseSchedMigrateTask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedMigrateTaskData eventData {};
    ParseSchedMigrateTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_migrate_task_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_orig_cpu(eventData.orig_cpu);
    msg->set_dest_cpu(eventData.dest_cpu);
}
void ParseSchedMigrateTaskEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedMigrateTaskData eventData {};
    ParseSchedMigrateTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_migrate_task_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_orig_cpu(eventData.orig_cpu);
    msg->set_dest_cpu(eventData.dest_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_migrate_task, ParseSchedMigrateTask);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_migrate_task, ParseSchedMigrateTaskEncoder);

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
void ParseSchedMoveNuma(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedMoveNumaData eventData {};
    ParseSchedMoveNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_move_numa_format();
    msg->set_pid(eventData.pid);
    msg->set_tgid(eventData.tgid);
    msg->set_ngid(eventData.ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
void ParseSchedMoveNumaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedMoveNumaData eventData {};
    ParseSchedMoveNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_move_numa_format();
    msg->set_pid(eventData.pid);
    msg->set_tgid(eventData.tgid);
    msg->set_ngid(eventData.ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_move_numa, ParseSchedMoveNuma);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_move_numa, ParseSchedMoveNumaEncoder);

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
void ParseSchedPiSetprio(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedPiSetprioData eventData {};
    ParseSchedPiSetprioData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_oldprio(eventData.oldprio);
    msg->set_newprio(eventData.newprio);
}
void ParseSchedPiSetprioEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedPiSetprioData eventData {};
    ParseSchedPiSetprioData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_pi_setprio_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_oldprio(eventData.oldprio);
    msg->set_newprio(eventData.newprio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_pi_setprio, ParseSchedPiSetprio);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_pi_setprio, ParseSchedPiSetprioEncoder);

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
void ParseSchedProcessExec(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessExecData eventData {};
    ParseSchedProcessExecData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exec_format();
    msg->set_filename(eventData.filename);
    msg->set_pid(eventData.pid);
    msg->set_old_pid(eventData.old_pid);
}
void ParseSchedProcessExecEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessExecData eventData {};
    ParseSchedProcessExecData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exec_format();
    msg->set_filename(eventData.filename);
    msg->set_pid(eventData.pid);
    msg->set_old_pid(eventData.old_pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exec, ParseSchedProcessExec);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exec, ParseSchedProcessExecEncoder);

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
void ParseSchedProcessExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessExitData eventData {};
    ParseSchedProcessExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exit_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
void ParseSchedProcessExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessExitData eventData {};
    ParseSchedProcessExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_exit_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_exit, ParseSchedProcessExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_exit, ParseSchedProcessExitEncoder);

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
void ParseSchedProcessFork(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessForkData eventData {};
    ParseSchedProcessForkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_fork_format();
    msg->set_parent_comm(eventData.parent_comm);
    msg->set_parent_pid(eventData.parent_pid);
    msg->set_child_comm(eventData.child_comm);
    msg->set_child_pid(eventData.child_pid);
}
void ParseSchedProcessForkEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessForkData eventData {};
    ParseSchedProcessForkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_fork_format();
    msg->set_parent_comm(eventData.parent_comm);
    msg->set_parent_pid(eventData.parent_pid);
    msg->set_child_comm(eventData.child_comm);
    msg->set_child_pid(eventData.child_pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_fork, ParseSchedProcessFork);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_fork, ParseSchedProcessForkEncoder);

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
void ParseSchedProcessFree(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessFreeData eventData {};
    ParseSchedProcessFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_free_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
void ParseSchedProcessFreeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessFreeData eventData {};
    ParseSchedProcessFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_free_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_free, ParseSchedProcessFree);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_free, ParseSchedProcessFreeEncoder);

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
void ParseSchedProcessWait(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessWaitData eventData {};
    ParseSchedProcessWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_wait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
void ParseSchedProcessWaitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedProcessWaitData eventData {};
    ParseSchedProcessWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_process_wait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_process_wait, ParseSchedProcessWait);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_process_wait, ParseSchedProcessWaitEncoder);

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
void ParseSchedStatBlocked(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatBlockedData eventData {};
    ParseSchedStatBlockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
void ParseSchedStatBlockedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatBlockedData eventData {};
    ParseSchedStatBlockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_blocked_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_blocked, ParseSchedStatBlocked);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_blocked, ParseSchedStatBlockedEncoder);

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
void ParseSchedStatIowait(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatIowaitData eventData {};
    ParseSchedStatIowaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
void ParseSchedStatIowaitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatIowaitData eventData {};
    ParseSchedStatIowaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_iowait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_iowait, ParseSchedStatIowait);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_iowait, ParseSchedStatIowaitEncoder);

struct SchedStatRuntimeData {
    std::string comm {};
    int32_t pid {};
    uint64_t runtime {};
    uint64_t vruntime {};
};
__attribute__((noinline)) void ParseSchedStatRuntimeData(SchedStatRuntimeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.runtime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vruntime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseSchedStatRuntime(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatRuntimeData eventData {};
    ParseSchedStatRuntimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_runtime(eventData.runtime);
    msg->set_vruntime(eventData.vruntime);
}
void ParseSchedStatRuntimeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatRuntimeData eventData {};
    ParseSchedStatRuntimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_runtime_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_runtime(eventData.runtime);
    msg->set_vruntime(eventData.vruntime);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_runtime, ParseSchedStatRuntime);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_runtime, ParseSchedStatRuntimeEncoder);

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
void ParseSchedStatSleep(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatSleepData eventData {};
    ParseSchedStatSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
void ParseSchedStatSleepEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatSleepData eventData {};
    ParseSchedStatSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_sleep_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_sleep, ParseSchedStatSleep);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_sleep, ParseSchedStatSleepEncoder);

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
void ParseSchedStatWait(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatWaitData eventData {};
    ParseSchedStatWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_wait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
void ParseSchedStatWaitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStatWaitData eventData {};
    ParseSchedStatWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stat_wait_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_delay(eventData.delay);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stat_wait, ParseSchedStatWait);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stat_wait, ParseSchedStatWaitEncoder);

struct SchedStickNumaData {
    int32_t pid {};
    int32_t tgid {};
    int32_t ngid {};
    int32_t src_cpu {};
    int32_t src_nid {};
    int32_t dst_cpu {};
    int32_t dst_nid {};
};
__attribute__((noinline)) void ParseSchedStickNumaData(SchedStickNumaData& eventData, uint8_t data[], size_t size,
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
void ParseSchedStickNuma(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStickNumaData eventData {};
    ParseSchedStickNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stick_numa_format();
    msg->set_pid(eventData.pid);
    msg->set_tgid(eventData.tgid);
    msg->set_ngid(eventData.ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
void ParseSchedStickNumaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedStickNumaData eventData {};
    ParseSchedStickNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_stick_numa_format();
    msg->set_pid(eventData.pid);
    msg->set_tgid(eventData.tgid);
    msg->set_ngid(eventData.ngid);
    msg->set_src_cpu(eventData.src_cpu);
    msg->set_src_nid(eventData.src_nid);
    msg->set_dst_cpu(eventData.dst_cpu);
    msg->set_dst_nid(eventData.dst_nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_stick_numa, ParseSchedStickNuma);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_stick_numa, ParseSchedStickNumaEncoder);

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
void ParseSchedSwapNuma(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedSwapNumaData eventData {};
    ParseSchedSwapNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_swap_numa_format();
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
void ParseSchedSwapNumaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedSwapNumaData eventData {};
    ParseSchedSwapNumaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_swap_numa_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_swap_numa, ParseSchedSwapNuma);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_swap_numa, ParseSchedSwapNumaEncoder);

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
    int16_t prevPrio = FtraceFieldParser::ParseIntField<int16_t>(format.fields, i++, data, size);
    eventData.prev_prio = static_cast<int32_t>(prevPrio);
    uint16_t prevState = FtraceFieldParser::ParseIntField<uint16_t>(format.fields, i++, data, size);
    eventData.prev_state = static_cast<uint64_t>(prevState);
    eventData.next_comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.next_pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    int16_t nextPrio = FtraceFieldParser::ParseIntField<int16_t>(format.fields, i++, data, size);
    eventData.next_prio = static_cast<int32_t>(nextPrio);
}
void ParseSchedSwitch(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedSwitchData eventData {};
    ParseSchedSwitchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_switch_format();
    msg->set_prev_comm(eventData.prev_comm);
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_comm(eventData.next_comm);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_prio(eventData.next_prio);
}
void ParseSchedSwitchEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedSwitchData eventData {};
    ParseSchedSwitchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_switch_format();
    msg->set_prev_comm(eventData.prev_comm);
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_comm(eventData.next_comm);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_prio(eventData.next_prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_switch, ParseSchedSwitch);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_switch, ParseSchedSwitchEncoder);

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
void ParseSchedWaitTask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWaitTaskData eventData {};
    ParseSchedWaitTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wait_task_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
void ParseSchedWaitTaskEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWaitTaskData eventData {};
    ParseSchedWaitTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wait_task_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wait_task, ParseSchedWaitTask);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wait_task, ParseSchedWaitTaskEncoder);

struct SchedWakeIdleWithoutIpiData {
    int32_t cpu {};
};
__attribute__((noinline)) void ParseSchedWakeIdleWithoutIpiData(SchedWakeIdleWithoutIpiData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cpu = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseSchedWakeIdleWithoutIpi(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeIdleWithoutIpiData eventData {};
    ParseSchedWakeIdleWithoutIpiData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
    msg->set_cpu(eventData.cpu);
}
void ParseSchedWakeIdleWithoutIpiEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeIdleWithoutIpiData eventData {};
    ParseSchedWakeIdleWithoutIpiData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wake_idle_without_ipi_format();
    msg->set_cpu(eventData.cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wake_idle_without_ipi, ParseSchedWakeIdleWithoutIpi);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wake_idle_without_ipi, ParseSchedWakeIdleWithoutIpiEncoder);

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
void ParseSchedWakeup(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeupData eventData {};
    ParseSchedWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
void ParseSchedWakeupEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeupData eventData {};
    ParseSchedWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup, ParseSchedWakeup);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup, ParseSchedWakeupEncoder);

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
    int16_t prio = FtraceFieldParser::ParseIntField<int16_t>(format.fields, i++, data, size);
    eventData.prio = static_cast<int32_t>(prio);
    int8_t targetCpu = FtraceFieldParser::ParseIntField<int8_t>(format.fields, i++, data, size);
    eventData.target_cpu = static_cast<int32_t>(targetCpu);
}
void ParseSchedWakeupNew(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeupNewData eventData {};
    ParseSchedWakeupNewData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
void ParseSchedWakeupNewEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakeupNewData eventData {};
    ParseSchedWakeupNewData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_wakeup_new_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_wakeup_new, ParseSchedWakeupNew);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_wakeup_new, ParseSchedWakeupNewEncoder);

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
void ParseSchedWaking(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakingData eventData {};
    ParseSchedWakingData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_waking_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
void ParseSchedWakingEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SchedWakingData eventData {};
    ParseSchedWakingData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_sched_waking_format();
    msg->set_comm(eventData.comm);
    msg->set_pid(eventData.pid);
    msg->set_prio(eventData.prio);
    msg->set_target_cpu(eventData.target_cpu);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(sched_waking, ParseSchedWaking);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(sched_waking, ParseSchedWakingEncoder);
} // namespace
FTRACE_NS_END
