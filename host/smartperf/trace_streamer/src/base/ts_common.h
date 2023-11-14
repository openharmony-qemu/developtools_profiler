/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SRC_TRACE_BASE_TS_COMMON_H
#define SRC_TRACE_BASE_TS_COMMON_H

#include <cstdint>
#include <limits>
#include <map>
#include <string>
using ClockId = uint32_t;
const std::string INVALID_STRING = "INVALID_STRING";
const uint64_t INVALID_ITID = std::numeric_limits<uint32_t>::max();
const uint64_t INVALID_IPID = std::numeric_limits<uint32_t>::max();
const uint64_t INVALID_UINT64 = std::numeric_limits<uint64_t>::max();
const uint64_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
const uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();
const uint32_t INVALID_UINT8 = std::numeric_limits<uint8_t>::max();
const uint32_t INVALID_UINT16 = std::numeric_limits<uint16_t>::max();
const uint32_t INVALID_UINT32 = std::numeric_limits<uint32_t>::max();
const uint32_t INVALID_INT32 = std::numeric_limits<int32_t>::max();
const int64_t INVALID_INT64 = std::numeric_limits<int64_t>::max();
const uint64_t INVALID_DATAINDEX = std::numeric_limits<uint64_t>::max();
const uint64_t INVALID_CALL_CHAIN_ID = std::numeric_limits<uint64_t>::max();
const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();
const uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
const uint64_t SEC_TO_NS = 1000 * 1000 * 1000;
const int32_t STR_DEFAULT_LEN = -1;
const auto INVALID_CPU = INVALID_UINT32;
const auto INVALID_TIME = INVALID_UINT64;
const std::string memQuery =
    "select max(value) as maxNum, min(value) as minNum, avg(value) as avgNum, filter.name as name, p.name as "
    "processName from process_measure left join process_measure_filter as filter on filter.id= filter_id left join "
    "process as p on p.id = filter.ipid where filter_id > 0 and filter.name = 'mem.rss.anon' group by filter_id order "
    "by avgNum desc;";
const std::string memTopQuery =
    "select max(value) as maxNum, min(value) as minNum, avg(value) as avgNum, f.name as name, p.name as processName "
    "from process_measure left join process_measure_filter as f on f.id= filter_id left join process as p on p.id = "
    "f.ipid where filter_id > 0 and f.name = 'mem.rss.anon' group by filter_id order by avgNum desc limit 10;";
const std::string cpuSqlQuery =
    "SELECT itid AS tid, ipid AS pid, group_concat(cpu, ',') AS cpu, group_concat(dur, ',') AS dur, "
    "group_concat(min_freq, ',') AS min_freq, group_concat(max_freq, ',') AS max_freq, group_concat(avg_frequency, "
    "',') AS avg_frequency FROM (SELECT itid, ipid, cpu, CAST (SUM(dur) AS INT) AS dur, CAST (MIN(freq) AS INT) AS "
    "min_freq, CAST (MAX(freq) AS INT) AS max_freq, CAST ( (SUM(dur * freq) / SUM(dur) ) AS INT) AS avg_frequency from "
    "result group by itid, cpu)GROUP BY ipid, itid ORDER BY ipid;";
const std::string cpuTopTenSqlQuery =
    "SELECT itid AS tid, ipid AS pid, group_concat(cpu, ',') AS cpu,group_concat(dur, ',') AS dur, "
    "group_concat(min_freq, ',') AS min_freq, group_concat(max_freq, ',') AS max_freq, group_concat(avg_frequency, "
    "',') AS avg_frequency, sum(dur * avg_frequency) AS sumNum FROM (SELECT itid, ipid, cpu, CAST (SUM(dur) AS INT) AS "
    "dur,CAST (MIN(freq) AS INT) AS min_freq, CAST (MAX(freq) AS INT) AS max_freq,CAST ( (SUM(dur * freq) / SUM(dur) ) "
    "AS INT) AS avg_frequency from result group by itid, cpu) GROUP BY ipid, itid ORDER BY sumNum DESC LIMIT 10";
const std::string distributedTermQuery =
    "select group_concat(thread.id,',') as threadId, group_concat(thread.name,',') as threadName, "
    "group_concat(process.id,',') as processId, group_concat(process.name,',') as processName, "
    "group_concat(callstack.name,',') as funName, group_concat(callstack.dur,',') as dur, "
    "group_concat(callstack.ts,',') as ts, cast(callstack.chainId as varchar) as chainId, callstack.spanId as spanId, "
    "callstack.parentSpanId as parentSpanId, group_concat(callstack.flag,',') as flag, (select value from meta where "
    "name='source_name') as trace_name from callstack inner join thread on callstack.callid = thread.id inner join "
    "process on process.id = thread.ipid where (callstack.flag='S' or callstack.flag='C') group by "
    "callstack.chainId,callstack.spanId,callstack.parentSpanId;";
const std::string memUnaggQuery =
    "select p.name as processName, group_concat(filter.name) as name, cast(group_concat(value) as varchar) as value, "
    "cast(group_concat(ts) as varchar) as ts from process_measure m left join process_measure_filter as filter on "
    "filter.id= m.filter_id left join process as p on p.id = filter.ipid where filter.name = 'mem.rss.anon' or "
    "filter.name = 'mem.rss.file' or filter.name = 'mem.swap' or filter.name = 'oom_score_adj' group by "
    "p.name,filter.ipid order by filter.ipid;";
const std::string metaDataQuery =
    "select cast(name as varchar) as name, cast(value as varchar) as valueText from meta UNION select "
    "'start_ts',cast(start_ts as varchar) from trace_range UNION select 'end_ts',cast(end_ts as varchar) from "
    "trace_range;";
const std::string sysCallsTopQuery =
    "SELECT cpu.tid AS tid, cpu.pid AS pid, callstack.name AS funName, count(callstack.name) AS frequency, "
    "min(callstack.dur) AS minDur, max(callstack.dur) AS maxDur, round(avg(callstack.dur)) AS avgDur FROM callstack "
    "INNER JOIN (SELECT itid AS tid, ipid AS pid, group_concat(cpu, ',') AS cpu, group_concat(dur, ',') AS dur, "
    "group_concat(min_freq, ',') AS min_freq, group_concat(max_freq, ',') AS max_freq, group_concat(avg_frequency, "
    "',') AS avg_frequency, sum(dur * avg_frequency) AS sumNum FROM (SELECT itid, ipid, cpu, CAST (SUM(dur) AS INT) AS "
    "dur, CAST (MIN(freq) AS INT) AS min_freq, CAST (MAX(freq) AS INT) AS max_freq, CAST ( (SUM(dur * freq) / SUM(dur) "
    ") AS INT) AS avg_frequency FROM result GROUP BY itid, cpu) GROUP BY ipid, itid ORDER BY sumNum DESC LIMIT 10) AS "
    "cpu ON callstack.callid = cpu.tid GROUP BY callstack.name ORDER BY frequency DESC LIMIT 10;";
const std::string sysCallQuery =
    "select count(*) as frequency, min(dur) as minDur, max(dur) as maxDur, avg(dur) as avgDur, name as funName from "
    "callstack group by name order by frequency desc limit 100;";
const std::string traceStateQuery = "select event_name,stat_type,count,source,serverity from stat;";
const std::string traceTaskName =
    "select P.id as id, P.pid as pid, P.name as process_name, group_concat(T.name,',') as thread_name from process as "
    "P left join thread as T where P.id = T.ipid group by pid;";
enum BuiltinClocks {
    TS_CLOCK_UNKNOW = 0,
    TS_CLOCK_BOOTTIME = 1,
    TS_CLOCK_REALTIME = 2,
    TS_CLOCK_REALTIME_COARSE = 3,
    TS_MONOTONIC = 4,
    TS_MONOTONIC_COARSE = 5,
    TS_MONOTONIC_RAW = 6,
};
extern BuiltinClocks g_primaryClockId;
enum RefType {
    K_REF_NO_REF = 0,
    K_REF_ITID = 1,
    K_REF_CPUID = 2,
    K_REF_IRQ = 3,
    K_REF_SOFT_IRQ = 4,
    K_REF_IPID = 5,
    K_REF_ITID_LOOKUP_IPID = 6,
    K_REF_MAX
};

enum TraceFileType {
    TRACE_FILETYPE_BY_TRACE,
    TRACE_FILETYPE_H_TRACE,
    TRACE_FILETYPE_RAW_TRACE,
    TRACE_FILETYPE_SYSEVENT,
    TRACE_FILETYPE_PERF,
    TRACE_FILETYPE_HILOG,
    TRACE_FILETYPE_UN_KNOW
};

enum EndState {
    // (R) ready state or running state, the process is ready to run, but not necessarily occupying the CPU
    TASK_RUNNABLE = 0,
    // (S) Indicates that the process is in light sleep, waiting for the resource state, and can respond to the signal.
    // Generally, the process actively sleeps into 'S' state.
    TASK_INTERRUPTIBLE = 1,
    // (D) Indicates that the process is in deep sleep, waiting for resources, and does not respond to signals.
    // Typical scenario: process acquisition semaphore blocking.
    TASK_UNINTERRUPTIBLE = 2,
    // (D-IO)
    TASK_UNINTERRUPTIBLE_IO = 21,
    // (D-NIO)
    TASK_UNINTERRUPTIBLE_NIO = 22,
    // (Running) Indicates that the thread is running
    TASK_RUNNING = 3,
    // (T) Thread in interrupt state
    TASK_STOPPED = 4,
    // (t) Task being traced
    TASK_TRACED = 8,
    // (X) Exit status, the process is about to be destroyed.
    TASK_EXIT_DEAD = 16,
    // (Z) Zombie state
    TASK_ZOMBIE = 32,
    // (P)
    TASK_PARKED = 64,
    // (I) Process killed
    TASK_DEAD = 128,
    // (DK)
    TASK_DK = 130,
    // (DK-IO)
    TASK_DK_IO = 131,
    // (DK-NIO)
    TASK_DK_NIO = 132,
    // (tK)the process is being debug now
    TASK_TRACED_KILL = 136,
    // (R+) The process is in a deep sleep state and will be killed directly after waking up
    TASK_WAKEKILL = 256,
    // TASK_WAKING  = 512 (R) waking dont use for Runable state
    // TASK_NOLOAD = 1024
    // (R+) Process groups in the foreground
    TASK_NEW = 2048,
    TASK_RUNNABLE_BINDER = 2049,
    TASK_MAX = 4096,
    TASK_INVALID = 0x8000
};
enum TSLogLevel {
    TS_DEBUG = 68,   // Debug
    TS_ERROR = 69,   // Error
    TS_INFO = 73,    // Info
    TS_VERBOSE = 86, // Verbose
    TS_WARN = 87     // Warn
};
enum SchedWakeType {
    SCHED_WAKING = 0, // sched_waking
    SCHED_WAKEUP = 1, // sched_wakeup
};
enum DataSourceType {
    DATA_SOURCE_TYPE_TRACE,
    DATA_SOURCE_TYPE_MEM,
    DATA_SOURCE_TYPE_HILOG,
    DATA_SOURCE_TYPE_NATIVEHOOK,
    DATA_SOURCE_TYPE_NATIVEHOOK_CONFIG,
    DATA_SOURCE_TYPE_FPS,
    DATA_SOURCE_TYPE_NETWORK,
    DATA_SOURCE_TYPE_DISKIO,
    DATA_SOURCE_TYPE_CPU,
    DATA_SOURCE_TYPE_PROCESS,
    DATA_SOURCE_TYPE_HISYSEVENT,
    DATA_SOURCE_TYPE_HISYSEVENT_CONFIG,
    DATA_SOURCE_TYPE_JSMEMORY,
    DATA_SOURCE_TYPE_JSMEMORY_CONFIG,
    DATA_SOURCE_TYPE_MEM_CONFIG
};
using DataIndex = uint64_t;
using TableRowId = int32_t;
using InternalPid = uint32_t;
using InternalTid = uint32_t;
using InternalTime = uint64_t;
using FilterId = uint32_t;
using InternalCpu = uint32_t; // how many cpus? could change to int8_t?

enum BaseDataType { BASE_DATA_TYPE_INT, BASE_DATA_TYPE_STRING, BASE_DATA_TYPE_DOUBLE, BASE_DATA_TYPE_BOOLEAN };
namespace SysTuning {
namespace TraceStreamer {
struct ArgsData {
    BaseDataType type;
    int64_t value;
};
struct TraceTimeSnap {
    uint64_t startTime;
    uint64_t endTime;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
