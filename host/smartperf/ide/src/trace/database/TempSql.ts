/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

let temp_query_process = `create table temp_query_process as
select distinct process.pid  as pid,
                process.name as processName
from (select ipid, itid
      from sched_slice
      group by itid) the_tracks
         left join
     (select sched_slice.ipid, sum(dur) as total_dur from sched_slice group by ipid) using (ipid)
         left join
     process using (ipid)
where pid is not null
order by total_dur desc,
         the_tracks.ipid,
         processName,
         the_tracks.itid;
`;
let temp_query_cpu_data = `create table temp_query_cpu_data as
with list as (SELECT IP.name            as processName,
                     IP.name               processCmdLine,
                     IP.pid             as processId,
                     B.cpu,
                     A.name,
                     C.id               as schedId,
                     A.tid,
                     A.id,
                     A.type,
                     B.dur,
                     B.ts - TR.start_ts AS startTime,
                     C.priority,
                     C.end_state
              from thread_state AS B
                       left join thread as A on B.itid = A.id
                       left join sched_slice AS C on B.itid = C.itid and B.ts = C.ts
                       left join trace_range AS TR
                       left join process AS IP on A.ipid = IP.id
              where C.itid is not null
              order by B.id)
select *
from list;
create index temp_query_cpu_data_idx on temp_query_cpu_data (cpu, startTime);
`;

let temp_query_freq_data = `create table temp_query_freq_data as
select cpu, value, ts - tb.start_ts as startNS
from measure c,
     trace_range tb
         inner join cpu_measure_filter t on c.filter_id = t.id
where (name = 'cpufreq' or name = 'cpu_frequency')
order by ts;
create index temp_query_freq_data_idx on temp_query_freq_data (cpu);
`;

let temp_query_process_data = `create table temp_query_process_data as
select ta.id,
       type,
       ts,
       dur,
       ta.cpu,
       itid             as utid,
       state,
       ts - tb.start_ts as startTime,
       tc.tid,
       tc.pid,
       tc.process,
       tc.thread
from thread_state ta,
     trace_range tb
         left join (select it.id,
                           tid,
                           pid,
                           ip.name as process,
                           it.name as thread
                    from thread as it
                             left join process ip on it.ipid = ip.id) tc on ta.itid = tc.id
where ta.cpu is not null
order by startTime;
create index temp_query_process_data_idx on temp_query_process_data (pid, startTime);
`;
let temp_query_thread_function = `create table temp_query_thread_function as
select tid,
       A.name            as threadName,
       is_main_thread,
       c.callid          as track_id,
       c.ts - D.start_ts as startTs,
       c.dur,
       c.name            as funName,
       c.parent_id,
       c.id,
       c.depth,
       c.argsetid
from thread A,
     trace_range D
         left join callstack C on A.id = C.callid
where startTs not null and c.cookie is null;
create index temp_query_thread_function_idx on temp_query_thread_function (tid);
`;

let temp_query_thread_data = `create table temp_query_thread_data as
select A.id
     , A.type
     , A.tid
     , A.name
     , A.start_ts
     , A.end_ts
     , A.ipid             as upid
     , A.is_main_thread
     , B.cpu
     , B.ts - TR.start_ts AS startTime
     , B.dur
     , B.state
     , IP.pid
     , IP.name            as processName
from thread_state AS B
         left join thread as A on A.id = B.itid
         left join trace_range AS TR
         left join process AS IP on IP.id = A.ipid;
create index temp_query_thread_data_idx on temp_query_thread_data (tid);`;

let temp_view = `CREATE VIEW IF NOT EXISTS thread_view AS SELECT id as itid, * FROM thread;
CREATE VIEW IF NOT EXISTS process_view AS SELECT id as ipid, * FROM process;
CREATE VIEW IF NOT EXISTS sched_view AS SELECT *, ts + dur as ts_end FROM sched_slice;
CREATE VIEW IF NOT EXISTS instants_view AS SELECT *, 0.0 as value FROM instant;
CREATE VIEW IF NOT EXISTS trace_section AS select start_ts, end_ts from trace_range;`;

let temp_query_cpu_freq = `create table temp_query_cpu_freq as
select cpu
from cpu_measure_filter
where (name = 'cpufreq' or name = 'cpu_frequency')
order by cpu;`;
let temp_query_cpu_max_freq = `create table temp_query_cpu_max_freq as
select max(value) as maxFreq
from measure c
         inner join cpu_measure_filter t on c.filter_id = t.id
where (name = 'cpufreq' or name = 'cpu_frequency');`;

let temp_get_tab_states_group_by_process = `create table temp_get_tab_states_group_by_process as
select IP.name                 as process,
       IP.pid                  as processId,
       dur,
       Ip.id                   as id,
       (ts - B.start_ts + dur) as end_ts,
       (ts - B.start_ts)       as start_ts
from thread_state as A,
     trace_range as B
         left join thread as C on A.itid = C.id
         left join process AS IP on C.ipid = IP.id
where A.dur > 0
  and processId not null and (ts - B.start_ts)>0;
create index temp_get_tab_states_group_by_process_idx on temp_get_tab_states_group_by_process (end_ts, start_ts);
`;

let temp_get_process_thread_state_data = ` create table temp_get_process_thread_state_data as
select IP.name                      as process,
       IP.pid                       as processId,
       A.name                       as thread,
       B.state                      as state,
       A.tid                        as threadId,
       B.dur,
       (B.ts - TR.start_ts + B.dur) as end_ts,
       (B.ts - TR.start_ts)         as start_ts,
       B.cpu,
       C.priority,
       '-'                          as note
from thread_state as B
         left join thread as A on B.itid = A.id
         left join process as IP on A.ipid = IP.id
         left join trace_range as TR
         left join sched_slice as C on B.itid = C.itid and C.ts = B.ts
where B.dur > 0
  and IP.pid not null and (B.ts - TR.start_ts) >= 0;
create index temp_get_process_thread_state_data_idx on temp_get_process_thread_state_data (end_ts, start_ts);
`;

let temp_get_tab_states_group_by_state_pid_tid = ` create table temp_get_tab_states_group_by_state_pid_tid as
select IP.name                      as process,
       IP.pid                       as processId,
       A.name                       as thread,
       B.state                      as state,
       A.tid                        as threadId,
       B.dur                        as dur,
       A.tid                        as tid,
       (B.ts - TR.start_ts + B.dur) as end_ts,
       (B.ts - TR.start_ts)         as start_ts
from thread_state AS B
         left join thread as A on B.itid = A.id
         left join process AS IP on A.ipid = IP.id
         left join trace_range AS TR
where B.dur > 0
  and IP.pid not null and (B.ts - TR.start_ts > 0);
create index temp_get_tab_states_group_by_state_pid_tid_idx0 on temp_get_tab_states_group_by_state_pid_tid (process, processId, thread, threadId, state);
create index temp_get_tab_states_group_by_state_pid_tid_idx1 on temp_get_tab_states_group_by_state_pid_tid (end_ts, start_ts);
create index temp_get_tab_states_group_by_state_pid_tid_idx3 on temp_get_tab_states_group_by_state_pid_tid (end_ts, start_ts, process, processId, thread, threadId, state);
`;
let temp_get_tab_states_group_by_state_pid = `create table temp_get_tab_states_group_by_state_pid as
select IP.name                  as process,
       IP.pid                   as processId,
       B.state                  as state,
       B.dur                    as dur,
       A.tid                    as tid,
       (ts - TR.start_ts + dur) as end_ts,
       (ts - TR.start_ts)       as start_ts
from thread_state AS B
         left join thread as A on B.itid = A.id
         left join process AS IP on A.ipid = IP.id
         left join trace_range AS TR
where pid not null and
    B.dur > 0 and (ts - TR.start_ts > 0);
create index temp_get_tab_states_group_by_state_pid_idx0 on temp_get_tab_states_group_by_state_pid (process, processId, state);
create index temp_get_tab_states_group_by_state_pid_idx1 on temp_get_tab_states_group_by_state_pid (start_ts, end_ts);
`;
let temp_get_tab_states_group_by_state = `create table temp_get_tab_states_group_by_state as
select state,
       dur,
       (ts - B.start_ts + dur) as end_ts,
       (ts - B.start_ts)       as start_ts
from thread_state as A,
     trace_range as B
         left join thread as C on A.itid = C.id
         left join process AS IP on C.ipid = IP.id
where A.dur > 0
  and IP.pid not null and (ts - B.start_ts > 0);
create index temp_get_tab_states_group_by_state_idx0 on temp_get_tab_states_group_by_state (state);
create index temp_get_tab_states_group_by_state_idx1 on temp_get_tab_states_group_by_state (start_ts, end_ts);
`;
let temp_get_tab_states_group_by_process_thread = `create table temp_get_tab_states_group_by_process_thread as
select IP.name                  as process,
       IP.pid                   as processId,
       A.name                   as thread,
       a.tid                    as threadId,
       B.dur                    as dur,
       A.tid                    as tid,
       (ts - TR.start_ts + dur) as end_ts,
       (ts - TR.start_ts)       as start_ts
from thread_state AS B
         left join
     thread as A on B.itid = A.id
         left join
     process AS IP on A.ipid = IP.id
         left join
     trace_range AS TR
where pid not null
    and
      B.dur > 0
    and
      (ts - TR.start_ts)>0;
create index temp_get_tab_states_group_by_process_thread_idx0 on temp_get_tab_states_group_by_process_thread (process, processId, thread, threadId);
create index temp_get_tab_states_group_by_process_thread_idx1 on temp_get_tab_states_group_by_process_thread (start_ts, end_ts);
`;

let temp_get_cpu_rate = `create table temp_get_cpu_rate as
with cpu as (select cpu,
                    ts,
                    dur,
                    (case when ro < 99 then ro else 99 end)                               as ro,
                    (case when ro < 99 then stime + ro * cell else stime + 99 * cell end) as st,
                    (case when ro < 99 then stime + (ro + 1) * cell else etime end)       as et
             from (select cpu,
                          ts,
                          A.dur,
                          ((ts + A.dur) - D.start_ts) / ((D.end_ts - D.start_ts) / 100) as ro,
                          D.start_ts                                                    as stime,
                          D.end_ts                                                         etime,
                          (D.end_ts - D.start_ts) / 100                                 as cell
                   from sched_slice A
                            left join
                        trace_range D
                            left join
                        thread B on A.itid = B.id
                            left join
                        process C on B.ipid = C.id
                   where tid != 0
        and (A.ts)
          between D.start_ts and D.end_ts))
select cpu,
       ro,
       sum(case
               when ts <= st and ts + dur <= et then (ts + dur - st)
               when ts <= st and ts + dur > et then et - st
               when ts > st and ts + dur <= et then dur
               when ts > st and ts + dur > et then et - ts end) / cast(et - st as float) as rate
from cpu
group by cpu, ro;
`;

let temp_get_tab_thread_states = `create table temp_get_tab_thread_states as
select IP.name                                 as process,
       IP.pid                                  as pid,
       A.name                                  as thread,
       A.tid                                   as tid,
       B.state                                 as state,
       B.dur                                   as dur,
       (B.ts - TR.start_ts + ifnull(B.dur, 0)) as end_ts,
       (B.ts - TR.start_ts)                    as start_ts
from thread_state AS B
         left join
     thread as A
     on
         A.id = B.itid
         left join
     trace_range AS TR
         left join
     process AS IP
     on
         IP.id = A.ipid
where (B.ts - TR.start_ts > 0);
create index temp_get_tab_thread_states_idx0 on temp_get_tab_thread_states (process, pid, thread, tid, state);
create index temp_get_tab_thread_states_idx1 on temp_get_tab_thread_states (start_ts, end_ts);
`;

let temp_get_tab_slices = `create table temp_get_tab_slices as
select c.name                      as name,
       c.dur                       as dur,
       A.tid                       as tid,
       (C.ts - D.start_ts + C.dur) as end_ts,
       (C.ts - D.start_ts)         as start_ts
from thread A,
     trace_range D
         left join
     callstack C on A.id = C.callid
where C.ts not null
      and c.dur >= 0
      and (C.ts - D.start_ts > 0);
create index temp_get_tab_slices_idx0 on temp_get_tab_slices (name);
`;

let createProcessNoId = `
    insert into process(id, ipid, type, pid, name, start_ts)
    SELECT null, null, 'process' as type, tid as pid, t.name, t.start_ts
    from thread t
    where ipid is null
      and tid != 0;
    update process
    set id   = ROWID - 1,
        ipid = ROWID - 1
    where id is null;
    update thread
    set ipid = (select id from process where thread.tid = process.pid)
    where thread.ipid is null;
`;
let temp_create_cpu_freq_view = `CREATE VIEW cpu_freq_view AS SELECT B.cpu, A.ts, LEAD(A.ts, 1, (SELECT end_ts FROM trace_range)) OVER (PARTITION BY A.filter_id ORDER BY ts) AS end_ts,LEAD(A.ts, 1, (SELECT end_ts FROM trace_range)) OVER (PARTITION BY A.filter_id ORDER BY ts) - ts AS dur,value AS freq FROM measure AS A, cpu_measure_filter AS B WHERE B.name = 'cpu_frequency' AND A.filter_id = B.id`;
let temp_create_virtual_table = `CREATE VIRTUAL table result USING SPAN_JOIN(cpu_freq_view partitioned cpu, sched_slice partitioned cpu)`;

let queryThreadWakeUpFrom = `
    select TB.tid, TB.name as thread, TA.cpu, (TA.ts - TR.start_ts) as ts, TC.pid, TC.name as process
    from (select ts as wakeTs, wakeup_from as wakeupFromTid
          from instant,
               trace_range
          where name = 'sched_wakeup'
            and ref = $itid
            and ts > start_ts + $startTime
            and ts < start_ts + $startTime + $dur
          order by ts) TW
             left join thread_state TA
                       on TW.wakeupFromTid = TA.itid and TA.ts < TW.wakeTs and TA.ts + TA.dur >= TW.wakeTs
             left join thread TB on TA.itid = TB.id
             left join process TC on TB.ipid = TC.id
             left join trace_range TR
    where TB.ipid not null
    limit 1;
`;

let queryThreadWakeUp = `
    select TB.tid, TB.name as thread, min(TA.ts - TR.start_ts) as ts, TC.pid, TC.name as process
    from (select min(ts) as wakeTs, ref as itid
          from instant,
               trace_range
          where name = 'sched_wakeup'
            and wakeup_from = $itid
            and ts > start_ts + $startTime
            and ts < start_ts + $startTime + $dur
          group by ref) TW
             left join thread_state TA on TW.itid = TA.itid and TA.ts > TW.wakeTs
             left join thread TB on TA.itid = TB.id
             left join process TC on TB.ipid = TC.id
             left join trace_range TR
    where TB.ipid not null
    group by TB.tid, TB.name, TC.pid, TC.name;
`;

let delete_callstack_binder_data = `DELETE
                                    FROM callstack
                                    WHERE dur < -1
                                       or name = 'binder transaction async'
                                       or name = 'binder async rcv';`;

let temp_init_sql_list = [temp_query_process];