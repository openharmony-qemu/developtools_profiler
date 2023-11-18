/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required BY applicable law OR agreed to in writing, software
 * distributed under the License IS distributed ON an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express OR implied.
 * See the License for the specific language governing permissions AND
 * limitations under the License.
 */

const cpuSql: Array<DebugSql> = [
  {
    sql:
      'SELECT B.pid AS processId,B.cpu,B.tid,B.itid AS id,B.dur,B.ts - TR.start_ts AS startTime,B.arg_setid AS ' +
      'argSetID FROM thread_state AS B LEFT JOIN trace_range AS TR WHERE B.itid IS NOT NULL;',
    title: 'Cpu chart',
  },
  {
    sql:
      'SELECT cpu,value,IFNULL(dur,tb.end_ts - c.ts) dur,ts-tb.start_ts AS startNS FROM measure c,trace_range tb ' +
      "INNER JOIN cpu_measure_filter t ON c.filter_id = t.id WHERE (name = 'cpufreq' OR name='cpu_frequency');",
    title: 'Cpu Frequency chart',
  },
  {
    sql: 'SELECT (A.ts - B.start_ts) AS startTs,IFNULL(dur,B.end_ts - A.ts) dur,value FROM measure A,trace_range B;',
    title: 'Cpu State chart',
  },
  {
    sql:
      'SELECT ts - T.start_ts AS startNs,dur,max(value) AS max,min(value) AS min FROM measure,trace_range ' +
      'T GROUP BY ts;',
    title: 'Cpu Freq Limit chart',
  },
  {
    sql:
      'SELECT TS.pid AS pid,TS.tid AS tid,TS.cpu,SUM((TS.ts - TR.start_ts + TS.dur) - (TS.ts - TR.start_ts)) ' +
      'wallDuration,count(TS.tid) AS occurrences FROM thread_state AS TS LEFT JOIN trace_range AS TR GROUP BY TS.cpu,' +
      'TS.pid,TS.tid ORDER BY wallDuration DESC;',
    title: 'CPU By Thread table',
  },
  {
    sql:
      'SELECT B.pid AS pid, SUM(B.dur) AS wallDuration,AVG(B.dur) AS avgDuration,count(B.tid) AS occurrences FROM ' +
      'thread_state AS B LEFT JOIN trace_range AS TR GROUP BY B.pid ORDER BY wallDuration DESC;',
    title: 'CPU By Process table',
  },
  {
    sql:
      'SELECT cpu,SUM(A.dur) / CAST(B.end_ts - B.start_ts AS float) AS usage FROM thread_state A,trace_range B ' +
      'WHERE (A.ts - B.start_ts) > 0 AND A.dur > 0 GROUP BY cpu;',
    title: 'CPU Usage table',
  },
  {
    sql:
      'SELECT B.pid,B.tid,B.state,SUM(B.dur) AS wallDuration,AVG(IFNULL(B.dur,0)) AS ' +
      'avgDuration,count(B.tid) AS occurrences FROM thread_state AS B LEFT JOIN trace_range AS TR ' +
      'GROUP BY B.pid, B.tid, B.state ORDER BY wallDuration DESC;',
    title: 'Thread States table',
  },
  {
    sql:
      'SELECT value, filter_id AS filterId, ts, f.cpu FROM measure LEFT JOIN cpu_measure_filter AS f ON ' +
      'f.id=filter_id ORDER BY ts ASC;',
    title: 'Cpu State/Cpu Frequency table',
  },
];

const threadSql: Array<DebugSql> = [
  {
    sql:
      'SELECT  ta.cpu,dur, ts-TR.start_ts AS startTime FROM thread_state ta LEFT JOIN trace_range AS TR ' +
      'WHERE ta.cpu IS NOT NULL;',
    title: 'Process chart',
  },
];

const memory: Array<DebugSql> = [
  {
    sql:
      'SELECT startNs, SUM( value ) AS value FROM ( SELECT m.ts - tr.start_ts AS startNs,SUM( m.value ) AS value ' +
      'FROM sys_mem_measure m,trace_range tr LEFT JOIN sys_event_filter f ON f.id = m.filter_id WHERE m.ts < tr.end_ts ' +
      'GROUP BY m.ts UNION ALL SELECT a.ts - tr.start_ts AS startNs,SUM( a.size ) AS value FROM memory_ashmem a,' +
      'trace_range tr WHERE a.ts < tr.end_ts AND a.flag = 0 GROUP BY a.ts) GROUP BY startNs;',
    title: 'Ability Monitor purgeable chart',
  },
  {
    sql:
      'SELECT (A.ts - B.start_ts) AS startNs,SUM(A.size) AS value,E.data AS expTaskComm, A.flag AS flag ' +
      'FROM memory_dma A,trace_range B LEFT JOIN data_dict AS E ON E.id=A.exp_task_comm_id WHERE A.flag = 0 ' +
      'AND A.ts < B.end_ts GROUP BY A.ts;',
    title: 'Ability Monitor DMA chart',
  },
  {
    sql:
      'SELECT (A.ts - B.start_ts) AS startNs, SUM(A.used_gpu_size) AS value FROM memory_process_gpu A,trace_range B ' +
      'WHERE A.ts < B.end_ts GROUP BY A.ts;',
    title: 'Ability Monitor gpu memory chart',
  },
  {
    sql:
      "SELECT (A.timestamp - B.start_ts) AS startNs, SUM(dirty) * 1024 AS value, 'dirty' AS name " +
      'FROM smaps A,trace_range B WHERE A.timestamp < B.end_ts GROUP BY A.timestamp;',
    title: 'VM Tracker Smaps chart(dirty)',
  },
  {
    sql:
      "SELECT (A.timestamp - B.start_ts) AS startNs, SUM(swapped) * 1024 AS value, 'swapped' AS name " +
      'FROM smaps A,trace_range B WHERE A.timestamp < B.end_ts GROUP BY A.timestamp;',
    title: 'VM Tracker Smaps chart(swapped)',
  },
  {
    sql:
      "SELECT (A.timestamp - B.start_ts) AS startNs, SUM(resident_size) * 1024 AS value, 'resident_size' AS name " +
      'FROM smaps A,trace_range B WHERE A.timestamp < B.end_ts GROUP BY A.timestamp;',
    title: 'VM Tracker Smaps chart(resident_size)',
  },
  {
    sql:
      "SELECT (A.timestamp - B.start_ts) AS startNs, SUM(pss) * 1024 AS value, 'pss' AS name " +
      'FROM smaps A,trace_range B WHERE A.timestamp < B.end_ts GROUP BY A.timestamp;',
    title: 'VM Tracker Smaps chart(pss)',
  },
  {
    sql:
      'SELECT (A.timestamp - B.start_ts) AS startNs, SUM(private_clean + private_dirty) * 1024 AS value,' +
      "'private_clean + private_dirty' AS name FROM smaps A,trace_range B WHERE A.timestamp " +
      '< B.end_ts GROUP BY A.timestamp;',
    title: 'VM Tracker Smaps chart(private_clean + private_dirty)',
  },
  {
    sql:
      'SELECT (A.ts - B.start_ts) AS startNs,SUM(A.size) AS value FROM memory_ashmem A,trace_range B ' +
      'WHERE A.ts < B.end_ts AND flag = 0 GROUP BY A.ts;',
    title: 'VM Tracker SHM chart',
  },
  {
    sql:
      'SELECT (A.ts - B.start_ts) AS startNs,SUM(A.size) AS value,A.flag AS flag,A.ipid AS ipid,E.data AS ' +
      'expTaskComm FROM memory_dma A,trace_range B LEFT JOIN data_dict AS E ON E.id=A.exp_task_comm_id ' +
      'WHERE A.flag = 0 AND A.ts < B.end_ts GROUP BY A.ts;',
    title: 'VM Tracker Dma chart',
  },
  {
    sql:
      'SELECT startNs, SUM( value ) AS value FROM (SELECT m.ts - tr.start_ts AS startNs,SUM(m.value) AS value ' +
      'FROM process_measure m, trace_range tr LEFT JOIN process_measure_filter f ON f.id = m.filter_id ' +
      'WHERE m.ts < tr.end_ts GROUP BY m.ts UNION ALL SELECT a.ts - tr.start_ts AS startNs,SUM( a.pss ) AS value ' +
      'FROM memory_ashmem a,trace_range tr WHERE a.ts < tr.end_ts AND a.flag = 0 GROUP BY a.ts) GROUP BY startNs;',
    title: 'VM Tracker purgeable chart',
  },
  {
    sql:
      'SELECT (ts - start_ts) startNs,SUM(value) value FROM process_measure, trace_range ' +
      "WHERE filter_id = (SELECT id FROM process_measure_filter WHERE name = 'mem.gl_pss')AND ts " +
      'BETWEEN start_ts AND end_ts GROUP BY ts;',
    title: 'VM Tracker gpu GL chart',
  },
  {
    sql:
      'SELECT (A.ts - B.start_ts) AS startNs,SUM(A.used_gpu_size) AS value,A.ipid AS ipid ' +
      'FROM memory_process_gpu A,trace_range B WHERE A.ts < B.end_ts GROUP BY A.ts;',
    title: 'VM Tracker gpu memory chart',
  },
  {
    sql:
      'SELECT DISTINCT module_name_id id,data FROM memory_window_gpu A, trace_range TR LEFT JOIN data_dict B ' +
      'ON A.module_name_id = B.id WHERE window_name_id = 0 AND A.ts < TR.end_ts;',
    title: 'VM Tracker gpu total type chart',
  },
  {
    sql:
      'SELECT DISTINCT A.window_name_id AS id,B.data, NULL AS pid FROM memory_window_gpu A, trace_range tr ' +
      'LEFT JOIN data_dict B ON A.window_name_id = B.id WHERE window_name_id != 0 AND A.ts < tr.end_ts UNION ALL ' +
      'SELECT DISTINCT A.module_name_id id, B.data, A.window_name_id pid FROM memory_window_gpu A, trace_range TR ' +
      'LEFT JOIN data_dict B ON A.module_name_id = B.id WHERE window_name_id != 0 AND A.ts < TR.end_ts;',
    title: 'VM Tracker gpu window type chart',
  },
];

const bioSql: Array<DebugSql> = [
  {
    sql:
      'SELECT name,B.ipid,pid FROM (SELECT DISTINCT ipid FROM bio_latency_sample A,trace_range B WHERE A.start_ts ' +
      'BETWEEN B.start_ts AND B.end_ts) A LEFT JOIN process B ON A.ipid = B.ipid;',
    title: 'Disk IO process chart',
  },
  {
    sql:
      'SELECT (A.start_ts -B.start_ts) AS startNS,(A.start_ts - B.start_ts + A.latency_dur) AS endNS,latency_dur ' +
      'AS dur FROM bio_latency_sample A,trace_range B WHERE startNS > 0 ORDER BY A.start_ts;',
    title: 'Disk IO latency chart',
  },
];

const frameTimeSql: Array<DebugSql> = [
  {
    sql:
      "SELECT sf.id,'frameTime' AS frame_type,fs.ipid,fs.vsync AS name,fs.dur AS app_dur,(sf.ts + sf.dur - fs.ts) " +
      'AS dur,(fs.ts - TR.start_ts) AS ts,fs.type,fs.flag,pro.pid,pro.name AS cmdline,(sf.ts - TR.start_ts) AS rs_ts,' +
      'sf.vsync AS rs_vsync,sf.dur AS rs_dur,sf.ipid AS rs_ipid,proc.pid AS rs_pid,proc.name AS rs_name FROM ' +
      'frame_slice AS fs LEFT JOIN process AS pro ON pro.id = fs.ipid LEFT JOIN frame_slice AS sf ON fs.dst = sf.id ' +
      'LEFT JOIN process AS proc ON proc.id = sf.ipid LEFT JOIN trace_range TR WHERE fs.dst IS NOT NULL AND fs.type ' +
      "= 1 UNION SELECT -1 AS id,'frameTime' AS frame_type,fs.ipid,fs.vsync  AS name,fs.dur AS app_dur,fs.dur," +
      '(fs.ts - TR.start_ts) AS ts,fs.type,fs.flag, pro.pid,pro.name AS cmdline,NULL AS rs_ts,NULL AS rs_vsync,' +
      'NULL AS rs_dur,NULL AS rs_ipid,NULL AS rs_pid,NULL AS rs_name FROM frame_slice AS fs LEFT JOIN process AS pro' +
      " ON pro.id = fs.ipid LEFT JOIN trace_range TR WHERE fs.dst IS NULL AND pro.name NOT LIKE '%render_service%' " +
      'AND fs.type = 1 ORDER BY ts;',
    title: 'Expected timeline chart',
  },
  {
    sql:
      "SELECT sf.id,'frameTime' AS frame_type,fs.ipid,fs.vsync AS name,fs.dur AS app_dur,(sf.ts + sf.dur - fs.ts) " +
      'AS dur,(fs.ts - TR.start_ts) AS ts,fs.type,(CASE WHEN (sf.flag == 1 OR fs.flag == 1) THEN 1  WHEN (sf.flag == ' +
      '3 OR fs.flag == 3 ) THEN 3 ELSE 0 end) AS jank_tag,pro.pid,pro.name AS cmdline,(sf.ts - TR.start_ts) ' +
      'AS rs_ts,sf.vsync AS rs_vsync,sf.dur AS rs_dur,sf.ipid AS rs_ipid,proc.pid AS rs_pid, proc.name AS rs_name ' +
      'FROM frame_slice AS fs LEFT JOIN process AS pro ON pro.id = fs.ipid LEFT JOIN frame_slice AS sf ON fs.dst = ' +
      'sf.id LEFT JOIN process AS proc ON proc.id = sf.ipid LEFT JOIN trace_range TR WHERE fs.dst IS NOT NULL AND ' +
      "fs.type = 0 AND fs.flag <> 2 UNION SELECT -1 AS id,'frameTime' AS frame_type,fs.ipid,fs.vsync AS name," +
      'fs.dur AS app_dur,fs.dur,(fs.ts - TR.start_ts) AS ts,fs.type,fs.flag AS jank_tag,pro.pid,pro.name AS ' +
      'cmdline,NULL AS rs_ts,NULL AS rs_vsync,NULL AS rs_dur,NULL AS rs_ipid,NULL AS rs_pid,NULL AS rs_name FROM ' +
      'frame_slice AS fs LEFT JOIN process AS pro ON pro.id = fs.ipid LEFT JOIN trace_range TR WHERE fs.dst IS NULL ' +
      "AND pro.name NOT LIKE '%render_service%' AND fs.type = 0 AND fs.flag <> 2 ORDER BY ts;",
    title: 'Actual timeline chart',
  },
];

const appStartUpSql: Array<DebugSql> = [
  {
    sql:
      'SELECT P.pid,A.tid,A.call_id AS itid,(CASE WHEN A.start_time < B.start_ts THEN 0 ELSE (A.start_time - ' +
      'B.start_ts) end) AS startTs,(CASE WHEN A.start_time < B.start_ts THEN (A.end_time - B.start_ts) WHEN ' +
      'A.end_time = -1 THEN 0 ELSE (A.end_time - A.start_time) end) AS dur,A.start_name AS startName FROM ' +
      'app_startup A,trace_range B LEFT JOIN process P ON A.ipid = P.ipid ORDER BY start_name;',
    title: 'App start up chart',
  },
];

const animationSql: Array<DebugSql> = [
  {
    sql:
      'SELECT a.id AS animationId,(CASE WHEN a.input_time NOT NULL THEN (a.input_time - R.start_ts) ' +
      'ELSE (a.start_point- R.start_ts) END) AS ts,(a.start_point - R.start_ts) AS dynamicStartTs,(a.end_point ' +
      '- R.start_ts) AS dynamicEndTs FROM animation AS a,trace_range AS R ORDER BY ts;',
    title: 'Animation chart',
  },
  {
    sql:
      'SELECT d.id,d.x,d.y,d.width,d.height,d.alpha,d.name AS appName,(d.end_time - R.start_ts) AS ts ' +
      'FROM dynamic_frame AS d,trace_range AS R ORDER BY d.end_time;',
    title: 'Animation effect curve chart',
  },
  {
    sql:
      'SELECT d.id,d.width AS currentFrameWidth,d.height AS currentFrameHeight,d.name AS nameId,(d.end_time ' +
      '- R.start_ts) AS currentTs,d.x,d.y FROM dynamic_frame AS d,trace_range AS R ORDER BY d.end_time;',
    title: 'Frame spacing chart',
  },
];

export function getAllSql(): Array<DebugSql> {
  return [...cpuSql, ...threadSql, ...memory, ...bioSql, ...frameTimeSql, ...appStartUpSql, ...animationSql];
}

export interface DebugSql {
  sql: string;
  title: string;
}
