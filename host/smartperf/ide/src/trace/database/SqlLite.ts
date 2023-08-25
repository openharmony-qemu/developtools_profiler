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

import './sql-wasm.js';

import { Counter, Fps, SelectionData } from '../bean/BoxSelection.js';
import { WakeupBean } from '../bean/WakeupBean.js';
import { BinderArgBean } from '../bean/BinderArgBean.js';
import { SPT, SPTChild } from '../bean/StateProcessThread.js';
import { CpuUsage, Freq } from '../bean/CpuUsage.js';

import {
  NativeEvent,
  NativeEventHeap,
  NativeHookMalloc,
  NativeHookProcess,
  NativeHookSampleQueryInfo,
  NativeHookStatistics,
} from '../bean/NativeHook.js';
import {
  Dma,
  DmaComparison,
  GpuMemory,
  GpuMemoryComparison,
  LiveProcess,
  ProcessHistory,
  SystemCpuSummary,
  SystemDiskIOSummary,
  SystemNetworkSummary,
} from '../bean/AbilityMonitor.js';

import {
  PerfCall,
  PerfCallChain,
  PerfCmdLine,
  PerfFile,
  PerfSample,
  PerfStack,
  PerfThread,
} from '../bean/PerfProfile.js';
import { SearchFuncBean } from '../bean/SearchFuncBean.js';
import { CounterSummary, SdkSliceSummary } from '../bean/SdkSummary.js';
import { Smaps } from '../bean/SmapsStruct.js';
import { CpuFreqRowLimit } from '../component/chart/SpFreqChart.js';
import { CpuFreqLimitsStruct } from './ui-worker/ProcedureWorkerCpuFreqLimits.js';
import { CpuStruct } from './ui-worker/ProcedureWorkerCPU.js';
import { CpuFreqStruct } from './ui-worker/ProcedureWorkerFreq.js';
import { ThreadStruct } from './ui-worker/ProcedureWorkerThread.js';
import { FuncStruct } from './ui-worker/ProcedureWorkerFunc.js';
import { ProcessMemStruct } from './ui-worker/ProcedureWorkerMem.js';
import { FpsStruct } from './ui-worker/ProcedureWorkerFPS.js';
import { CpuAbilityMonitorStruct } from './ui-worker/ProcedureWorkerCpuAbility.js';
import { MemoryAbilityMonitorStruct } from './ui-worker/ProcedureWorkerMemoryAbility.js';
import { DiskAbilityMonitorStruct } from './ui-worker/ProcedureWorkerDiskIoAbility.js';
import { NetworkAbilityMonitorStruct } from './ui-worker/ProcedureWorkerNetworkAbility.js';
import { EnergyAnomalyStruct } from './ui-worker/ProcedureWorkerEnergyAnomaly.js';
import { EnergyStateStruct } from './ui-worker/ProcedureWorkerEnergyState.js';
import { CounterStruct } from './ui-worker/ProduceWorkerSdkCounter.js';
import { SdkSliceStruct } from './ui-worker/ProduceWorkerSdkSlice.js';
import { SystemDetailsEnergy } from '../bean/EnergyStruct.js';
import { ClockStruct } from './ui-worker/ProcedureWorkerClock.js';
import { IrqStruct } from './ui-worker/ProcedureWorkerIrq.js';
import {
  HeapEdge,
  HeapLocation,
  HeapNode,
  HeapSample,
  HeapTraceFunctionInfo,
} from '../../js-heap/model/DatabaseStruct';
import { FileInfo } from '../../js-heap/model/UiStruct.js';
import { AppStartupStruct } from './ui-worker/ProcedureWorkerAppStartup.js';
import { SoStruct } from './ui-worker/ProcedureWorkerSoInit.js';
import { HeapTreeDataBean } from './logic-worker/ProcedureLogicWorkerCommon.js';
import { TaskTabStruct } from '../component/trace/sheet/task/TabPaneTaskFrames.js';
import { DeviceStruct } from '../bean/FrameComponentBean.js';
import { FrameSpacingStruct } from './ui-worker/ProcedureWorkerFrameSpacing.js';
import { FrameDynamicStruct } from './ui-worker/ProcedureWorkerFrameDynamic.js';
import { FrameAnimationStruct } from './ui-worker/ProcedureWorkerFrameAnimation.js';
import { SnapshotStruct } from './ui-worker/ProcedureWorkerSnapshot.js';
import { MemoryConfig } from '../bean/MemoryConfig.js';

class DataWorkerThread extends Worker {
  taskMap: any = {};

  uuid(): string {
    // @ts-ignore
    return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c: any) =>
      (c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
    );
  }

  //发送方法名 参数 回调
  queryFunc(action: string, args: any, handler: Function) {
    let id = this.uuid();
    this.taskMap[id] = handler;
    let msg = {
      id: id,
      action: action,
      args: args,
    };
    this.postMessage(msg);
  }
}

class DbThread extends Worker {
  busy: boolean = false;
  isCancelled: boolean = false;
  id: number = -1;
  taskMap: any = {};
  cacheArray: Array<any> = [];

  uuid(): string {
    // @ts-ignore
    return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c: any) =>
      (c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
    );
  }

  queryFunc(name: string, sql: string, args: any, handler: Function, action: string | null) {
    this.busy = true;
    let id = this.uuid();
    this.taskMap[id] = handler;
    let msg = {
      id: id,
      name: name,
      action: action || 'exec',
      sql: sql,
      params: args,
    };
    this.postMessage(msg);
  }

  dbOpen = async (
    parseConfig: string,
    sdkWasmConfig?: string
  ): Promise<{
    status: boolean;
    msg: string;
    buffer: ArrayBuffer;
    sdkConfigMap: any;
  }> => {
    return new Promise<any>((resolve, reject) => {
      let id = this.uuid();
      this.taskMap[id] = (res: any) => {
        if (res.init) {
          resolve({
            status: res.init,
            msg: res.msg,
            sdkConfigMap: res.configSqlMap,
            buffer: res.buffer,
          });
        } else {
          resolve({ status: res.init, msg: res.msg });
        }
      };
      this.postMessage(
        {
          id: id,
          action: 'open',
          parseConfig: parseConfig,
          wasmConfig: sdkWasmConfig,
          buffer: DbPool.sharedBuffer! /*Optional. An ArrayBuffer representing an SQLite Database file*/,
        },
        [DbPool.sharedBuffer!]
      );
    });
  };

  resetWASM() {
    this.postMessage({
      id: this.uuid(),
      action: 'reset',
    });
  }
}

export class DbPool {
  static sharedBuffer: ArrayBuffer | null = null;
  maxThreadNumber: number = 0;
  works: Array<DbThread> = [];
  progress: Function | undefined | null;
  num = Math.floor(Math.random() * 10 + 1) + 20;
  cutDownTimer: any | undefined;
  dataWorker: DataWorkerThread | undefined | null;
  currentWasmThread: DbThread | undefined = undefined;

  init = async (type: string, threadBuild: (() => DbThread) | undefined = undefined) => {
    // wasm | server | sqlite
    if (this.currentWasmThread) {
      this.currentWasmThread.resetWASM();
      this.currentWasmThread = undefined;
    }
    await this.close();
    const { port1, port2 } = new MessageChannel();
    if (type === 'wasm') {
      this.maxThreadNumber = 1;
    } else if (type === 'server') {
      this.maxThreadNumber = 1;
    } else if (type === 'sqlite') {
      this.maxThreadNumber = 1;
    } else if (type === 'duck') {
      this.maxThreadNumber = 1;
    }
    for (let i = 0; i < this.maxThreadNumber; i++) {
      let thread: DbThread | undefined;
      if (threadBuild) {
        thread = threadBuild();
      } else {
        if (type === 'wasm') {
          thread = new DbThread('trace/database/TraceWorker.js');
        } else if (type === 'server') {
          thread = new DbThread('trace/database/SqlLiteWorker.js');
        } else if (type === 'sqlite') {
          thread = new DbThread('trace/database/SqlLiteWorker.js');
        }
      }
      if (thread) {
        this.currentWasmThread = thread;
        thread!.onmessage = (event: MessageEvent) => {
          thread!.busy = false;
          if (Reflect.has(thread!.taskMap, event.data.id)) {
            if (event.data.results) {
              let fun = thread!.taskMap[event.data.id];
              if (fun) {
                fun(event.data.results);
              }
              Reflect.deleteProperty(thread!.taskMap, event.data.id);
            } else if (Reflect.has(event.data, 'ready')) {
              this.progress!('database opened', this.num + event.data.index);
              this.progressTimer(this.num + event.data.index, this.progress!);
            } else if (Reflect.has(event.data, 'init')) {
              if (this.cutDownTimer != undefined) {
                clearInterval(this.cutDownTimer);
              }
              let fun = thread!.taskMap[event.data.id];
              if (!event.data.init && !event.data.status) {
                if (fun) {
                  fun(['error', event.data.msg]);
                }
              } else {
                this.progress!('database ready', 40);
                if (fun) {
                  fun(event.data);
                }
              }
              Reflect.deleteProperty(thread!.taskMap, event.data.id);
            } else {
              let fun = thread!.taskMap[event.data.id];
              if (fun) {
                fun([]);
              }
              Reflect.deleteProperty(thread!.taskMap, event.data.id);
            }
          }
        };
        thread!.onmessageerror = (e) => {};
        thread!.onerror = (e) => {};
        thread!.id = i;
        thread!.busy = false;
        this.works?.push(thread!);
      }
    }
  };

  initServer = async (url: string, progress: Function) => {
    this.progress = progress;
    progress('database loaded', 15);
    DbPool.sharedBuffer = await fetch(url).then((res) => res.arrayBuffer());
    progress('open database', 20);
    for (let i = 0; i < this.works.length; i++) {
      let thread = this.works[i];
      let { status, msg } = await thread.dbOpen('');
      if (!status) {
        DbPool.sharedBuffer = null;
        return { status, msg };
      }
    }
    return { status: true, msg: 'ok' };
  };
  initSqlite = async (buf: ArrayBuffer, parseConfig: string, sdkWasmConfig: string, progress: Function) => {
    this.progress = progress;
    progress('database loaded', 15);
    DbPool.sharedBuffer = buf;
    progress('parse database', 20);
    let configMap;
    for (let i = 0; i < this.works.length; i++) {
      let thread = this.works[i];
      let { status, msg, buffer, sdkConfigMap } = await thread.dbOpen(parseConfig, sdkWasmConfig);
      if (!status) {
        DbPool.sharedBuffer = null;
        return { status, msg };
      } else {
        configMap = sdkConfigMap;
        DbPool.sharedBuffer = buffer;
      }
    }
    return { status: true, msg: 'ok', sdkConfigMap: configMap };
  };

  close = async () => {
    clearInterval(this.cutDownTimer);
    for (let i = 0; i < this.works.length; i++) {
      let thread = this.works[i];
      thread.terminate();
    }
    this.works.length = 0;
  };

  submit(name: string, sql: string, args: any, handler: Function, action: string | null) {
    let noBusyThreads = this.works.filter((it) => !it.busy);
    let thread: DbThread;
    if (noBusyThreads.length > 0) {
      //取第一个空闲的线程进行任务
      thread = noBusyThreads[0];
      thread.queryFunc(name, sql, args, handler, action);
    } else {
      // 随机插入一个线程中
      thread = this.works[Math.floor(Math.random() * this.works.length)];
      thread.queryFunc(name, sql, args, handler, action);
    }
  }

  //new method replace submit() method
  submitTask(action: string, args: any, handler: Function) {
    this.dataWorker?.queryFunc(action, args, handler);
  }

  progressTimer(num: number, progress: Function) {
    let currentNum = num;
    clearInterval(this.cutDownTimer);
    this.cutDownTimer = setInterval(() => {
      currentNum += Math.floor(Math.random() * 3);
      if (currentNum >= 50) {
        progress('database opened', 40);
        clearInterval(this.cutDownTimer);
      } else {
        progress('database opened', currentNum);
      }
    }, Math.floor(Math.random() * 2500 + 1000));
  }
}

export const threadPool = new DbPool();

export function query<T extends any>(
  name: string,
  sql: string,
  args: any = null,
  action: string | null = null
): Promise<Array<T>> {
  return new Promise<Array<T>>((resolve, reject) => {
    threadPool.submit(
      name,
      sql,
      args,
      (res: any) => {
        if (res[0] && res[0] === 'error') {
          window.publish(window.SmartEvent.UI.Error, res[1]);
          reject(res);
        } else {
          resolve(res);
        }
      },
      action
    );
  });
}

export const queryEventCountMap = (): Promise<
  Array<{
    eventName: string;
    count: number;
  }>
> => query('queryEventCountMap', `select event_name as eventName,count from stat where stat_type = 'received';`);

export const queryProcess = (): Promise<
  Array<{
    pid: number | null;
    processName: string | null;
  }>
> =>
  query(
    'queryProcess',
    `
    SELECT
      pid, processName
    FROM
      temp_query_process where pid != 0`
  );

export const queryProcessByTable = (): Promise<
  Array<{
    pid: number | null;
    processName: string | null;
  }>
> =>
  query(
    'queryProcessByTable',
    `
    SELECT
      pid, name as processName
    FROM
      process where pid != 0`
  );

export const queryProcessAsyncFunc = (_funName?: string): Promise<Array<any>> =>
  query(
    'queryProcessAsyncFunc',
    `
select tid,
    P.pid,
    A.name as threadName,
    is_main_thread,
    c.callid as track_id,
    c.ts-D.start_ts as startTs,
    c.dur,
    c.name as funName,
    c.parent_id,
    c.id,
    c.cookie,
    c.depth,
    c.argsetid
from thread A,trace_range D
left join callstack C on A.id = C.callid
left join process P on P.id = A.ipid
where startTs not null and cookie not null ${_funName ? 'funName=$funName' : ''};`,
    {
      funName: _funName,
    }
  );

export const queryTotalTime = (): Promise<Array<{ total: number; recordStartNS: number; recordEndNS: number }>> =>
  query(
    'queryTotalTime',
    `
    select
      start_ts as recordStartNS,end_ts as recordEndNS,end_ts-start_ts as total
    from
      trace_range;`
  );

export const getAsyncEvents = (): Promise<Array<any>> =>
  query(
    'getAsyncEvents',
    `
    select
      *,
      p.pid as pid,
      c.ts - t.start_ts as "startTime"
    from
      callstack c,trace_range t
    left join
      process p
    on
      c.callid = p.id
    where
      cookie is not null;`
  );

export const getCpuUtilizationRate = (
  startNS: number,
  endNS: number
): Promise<
  Array<{
    cpu: number;
    ro: number;
    rate: number;
  }>
> =>
  query(
    'getCpuUtilizationRate',
    `
    with cpu as (
    select
      cpu,
      ts,
      dur,
      (case when ro < 99 then ro else 99 end) as ro ,
      (case when ro < 99 then stime+ro*cell else stime + 99 * cell end) as st,
      (case when ro < 99 then stime + (ro+1)*cell else etime end) as et
    from (
        select
          cpu,
          ts,
          A.dur,
          ((ts+A.dur)-D.start_ts)/((D.end_ts-D.start_ts)/100) as ro,
          D.start_ts as stime,
          D.end_ts etime,
          (D.end_ts-D.start_ts)/100 as cell
        from
          sched_slice A
        left join
          trace_range D
        left join
          thread B on A.itid = B.id
        where
          tid != 0
        and (A.ts)
          between D.start_ts and D.end_ts))
    select cpu,ro,
       sum(case
               when ts <= st and ts + dur <= et then (ts + dur - st)
               when ts <= st and ts + dur > et then et-st
               when ts > st and ts + dur <= et then dur
               when ts > st and ts + dur > et then et - ts end)/cast(et-st as float) as rate
    from cpu
    group by cpu,ro;`,
    {}
  );

export const getFps = () =>
  query<FpsStruct>(
    'getFps',
    `
    select
      distinct(ts-tb.start_ts) as startNS, fps
    from
      hidump c ,trace_range tb
    where
      startNS >= 0
    --order by startNS;
    `,
    {}
  );

export const getFunDataByTid = (tid: number, ipid: number): Promise<Array<FuncStruct>> =>
  query(
    'getFunDataByTid',
    `
    select 
    c.ts-D.start_ts as startTs,
    c.dur,
    c.name as funName,
    c.argsetid,
    c.depth,
    c.id as id,
    A.itid as itid,
    A.ipid as ipid
from thread A,trace_range D
left join callstack C on A.id = C.callid
where startTs not null and c.cookie is null and tid = $tid and A.ipid = $ipid`,
    { $tid: tid, $ipid: ipid }
  );

export const getMaxDepthByTid = (): Promise<Array<any>> =>
  query(
    'getMaxDepthByTid',
    `
    select
tid,
ipid,
    MAX(c.depth + 1) as maxDepth
from thread A
left join callstack C on A.id = C.callid
where c.ts not null and c.cookie is null group by tid,ipid`,
    {}
  );

export const getStatesProcessThreadDataByRange = (leftNs: number, rightNs: number): Promise<Array<SPT>> =>
  query<SPT>(
    'getStatesProcessThreadDataByRange',
    `
    select
      IP.name as process,
      IP.pid as processId,
      A.name as thread,
      B.state as state,
      A.tid as threadId,
      B.dur,
      (B.ts - TR.start_ts + B.dur) as end_ts,
      (B.ts - TR.start_ts) as start_ts,
      B.cpu
    from
      thread_state as B 
    left join thread as A on B.itid = A.id
    left join process as IP on A.ipid = IP.id
    left join trace_range as TR
    where B.dur > 0
    and IP.pid not null
    and (B.ts - TR.start_ts) >= $leftNs 
    and (B.ts - TR.start_ts + B.dur) <= $rightNs
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabBoxChildData = (
  leftNs: number,
  rightNs: number,
  state: string | undefined,
  processId: number | undefined,
  threadId: number | undefined
): Promise<Array<SPTChild>> =>
  query<SPTChild>(
    'getTabBoxChildData',
    `
    select
      IP.name as process,
      IP.pid as processId,
      A.name as thread,
      B.state as state,
      A.tid as threadId,
      B.dur as duration,
      B.ts - TR.start_ts as startNs,
      B.cpu,
      C.priority
    from
      thread_state AS B
    left join
      thread as A
    on
      B.itid = A.id
    left join
      process AS IP
    on
      A.ipid = IP.id
    left join
      trace_range AS TR
    left join
      sched_slice as C
    on
      B.itid = C.itid
    and
      C.ts = B.ts
    where
      B.dur > 0
    and
      IP.pid not null
    and
      not ((B.ts - TR.start_ts + B.dur < $leftNS) or (B.ts - TR.start_ts > $rightNS))
      ${state != undefined && state != '' ? 'and B.state = $state' : ''}
      ${processId != undefined && processId != -1 ? 'and IP.pid = $processID' : ''}
      ${threadId != undefined && threadId != -1 ? 'and A.tid = $threadID' : ''}
    `,
    {
      $leftNS: leftNs,
      $rightNS: rightNs,
      $state: state,
      $processID: processId,
      $threadID: threadId,
    }
  );

export const getTabCpuUsage = (cpus: Array<number>, leftNs: number, rightNs: number): Promise<Array<CpuUsage>> =>
  query<CpuUsage>(
    'getTabCpuUsage',
    `
    select
      cpu,
      sum(case
        when (A.ts - B.start_ts) < $leftNS
          then (A.ts - B.start_ts + A.dur - $leftNS)
        when (A.ts - B.start_ts) >= $leftNS
          and (A.ts - B.start_ts + A.dur) <= $rightNS
          then A.dur
        when (A.ts - B.start_ts + A.dur) > $rightNS
          then ($rightNS - (A.ts - B.start_ts)) end) / cast($rightNS - $leftNS as float) as usage
    from
      thread_state A,
      trace_range B
    where
      (A.ts - B.start_ts) > 0 and A.dur > 0
    and
      cpu in (${cpus.join(',')})
    and
      (A.ts - B.start_ts + A.dur) > $leftNS
    and
      (A.ts - B.start_ts) < $rightNS
    group by
      cpu`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabCpuFreq = (cpus: Array<number>, leftNs: number, rightNs: number): Promise<Array<Freq>> =>
  query<Freq>(
    'getTabCpuFreq',
    `
    select
      cpu,
      value,
      (ts - tr.start_ts) as startNs
    from
      measure m,
      trace_range tr
    inner join
      cpu_measure_filter t
    on
      m.filter_id = t.id
    where
      (name = 'cpufreq' or name='cpu_frequency')
    and
      cpu in (${cpus.join(',')})
    and
      startNs > 0
    and
      startNs < $rightNS
    --order by startNs
    `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabFps = (leftNs: number, rightNs: number): Promise<Array<Fps>> =>
  query<Fps>(
    'getTabFps',
    `
    select
      distinct(ts-tb.start_ts) as startNS,
      fps
    from
      hidump c,
      trace_range tb
    where
      startNS <= $rightNS
    and
      startNS >= 0
    --order by startNS;
    `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabCounters = (processFilterIds: Array<number>, virtualFilterIds: Array<number>, startTime: number) =>
  query<Counter>(
    'getTabCounters',
    `
    select
      t1.filter_id as trackId,
      t2.name,
      value,
      t1.ts - t3.start_ts as startTime
    from
      process_measure t1
    left join
      process_measure_filter t2
    on
      t1.filter_id = t2.id
    left join
      trace_range t3
    where
      filter_id in (${processFilterIds.join(',')})
    and
      startTime <= $startTime
union
 select
      t1.filter_id as trackId,
      t2.name,
      value,
      t1.ts - t3.start_ts as startTime
    from
      sys_mem_measure t1
    left join
      sys_event_filter t2
    on
      t1.filter_id = t2.id
    left join
      trace_range t3
    where
      filter_id in (${virtualFilterIds.join(',')})
    and
      startTime <= $startTime
    `,
    { $startTime: startTime }
  );

export const getTabVirtualCounters = (virtualFilterIds: Array<number>, startTime: number) =>
  query<Counter>(
    'getTabVirtualCounters',
    `
    select
      table1.filter_id as trackId,
      table2.name,
      value,
      table1.ts - table3.start_ts as startTime
    from
      sys_mem_measure table1
    left join
      sys_event_filter table2
    on
      table1.filter_id = table2.id
    left join
      trace_range table3
    where
      filter_id in (${virtualFilterIds.join(',')})
    and
      startTime <= $startTime
    `,
    { $startTime: startTime }
  );

export const getTabCpuByProcess = (cpus: Array<number>, leftNS: number, rightNS: number) =>
  query<any>(
    'getTabCpuByProcess',
    `
    select
      B.pid as pid,
      sum(B.dur) as wallDuration,
      avg(B.dur) as avgDuration,
      count(B.tid) as occurrences
    from
      thread_state AS B
    left join
      trace_range AS TR
    where
      B.cpu in (${cpus.join(',')})
    and
      not ((B.ts - TR.start_ts + B.dur < $leftNS) or (B.ts - TR.start_ts > $rightNS ))
    group by
      B.pid
    order by
      wallDuration desc;`,
    { $rightNS: rightNS, $leftNS: leftNS }
  );

export const getTabCpuByThread = (cpus: Array<number>, leftNS: number, rightNS: number) =>
  query<any>(
    'getTabCpuByThread',
    `
    select
      TS.pid as pid,
      TS.tid as tid,
      TS.cpu,
      sum( min(${rightNS},(TS.ts - TR.start_ts + TS.dur)) - max(${leftNS},TS.ts - TR.start_ts)) wallDuration,
      count(TS.tid) as occurrences
    from
      thread_state AS TS
    left join
      trace_range AS TR
    where
      TS.cpu in (${cpus.join(',')})
    and
      not ((TS.ts - TR.start_ts + TS.dur < $leftNS) or (TS.ts - TR.start_ts > $rightNS))
    group by
      TS.cpu,
      TS.pid,
      TS.tid
    order by
      wallDuration desc;`,
    { $rightNS: rightNS, $leftNS: leftNS }
  );

export const getTabSlices = (
  funTids: Array<number>,
  pids: Array<number>,
  leftNS: number,
  rightNS: number
): Promise<Array<any>> =>
  query<SelectionData>(
    'getTabSlices',
    `
    select
      c.name as name,
      sum(c.dur) as wallDuration,
      avg(c.dur) as avgDuration,
      count(c.name) as occurrences
    from
      thread T, trace_range TR
      left join process P on T.ipid = P.id
    left join
      callstack C
    on
      T.id = C.callid
    where
      C.ts not null
    and
      c.dur >= 0
    and
      T.tid in (${funTids.join(',')})
    and
      P.pid in (${pids.join(',')})
    and
      c.name != 'binder transaction async'
    and
      c.name != 'binder async rcv'
    and
      c.cookie is null
    and
      not ((C.ts - TR.start_ts + C.dur < $leftNS) or (C.ts - TR.start_ts > $rightNS))
    group by
      c.name
    order by
      wallDuration desc;`,
    { $leftNS: leftNS, $rightNS: rightNS }
  );

export const getTabSlicesAsyncFunc = (
  asyncNames: Array<string>,
  asyncPid: Array<number>,
  leftNS: number,
  rightNS: number
): Promise<Array<any>> =>
  query<SelectionData>(
    'getTabSlicesAsyncFunc',
    `
    select
      c.name as name,
      sum(c.dur) as wallDuration,
      avg(c.dur) as avgDuration,
      count(c.name) as occurrences
    from
      thread A, trace_range D
    left join
      callstack C
    on
      A.id = C.callid
    left join process P on P.id = A.ipid
    where
      C.ts not null
    and
      c.dur >= -1
    and 
      c.cookie not null
    and
      P.pid in (${asyncPid.join(',')})
    and
      c.name in (${asyncNames.map((it) => "'" + it + "'").join(',')})
    and
      not ((C.ts - D.start_ts + C.dur < $leftNS) or (C.ts - D.start_ts > $rightNS))
    group by
      c.name
    order by
      wallDuration desc;`,
    { $leftNS: leftNS, $rightNS: rightNS }
  );

export const getTabThreadStates = (tIds: Array<number>, leftNS: number, rightNS: number): Promise<Array<any>> =>
  query<SelectionData>(
    'getTabThreadStates',
    `
    select
      B.pid,
      B.tid,
      B.state,
      sum(B.dur) as wallDuration,
      avg(ifnull(B.dur,0)) as avgDuration,
      count(B.tid) as occurrences
    from
      thread_state AS B
    left join
      trace_range AS TR
    where
      B.tid in (${tIds.join(',')})
    and
      not ((B.ts - TR.start_ts + ifnull(B.dur,0) < $leftNS) or (B.ts - TR.start_ts > $rightNS))
    group by
      B.pid, B.tid, B.state
    order by
      wallDuration desc;`,
    { $leftNS: leftNS, $rightNS: rightNS }
  );

export const getTabThreadStatesCpu = (tIds: Array<number>, leftNS: number, rightNS: number): Promise<Array<any>> => {
  let sql = `
select 
       B.pid,
       B.tid,
       B.cpu,
       sum( min(${rightNS},(B.ts - TR.start_ts + B.dur)) - max(${leftNS},B.ts - TR.start_ts)) wallDuration
from thread_state as B
left join trace_range as TR
where cpu notnull
    and B.tid in (${tIds.join(',')})
    and not ((B.ts - TR.start_ts + ifnull(B.dur,0) < ${leftNS}) or (B.ts - TR.start_ts > ${rightNS}))
group by B.tid, B.pid, B.cpu;`;
  return query<SelectionData>('getTabThreadStatesCpu', sql, {
    $leftNS: leftNS,
    $rightNS: rightNS,
  });
};

export const getTabStartups = (ids: Array<number>, leftNS: number, rightNS: number): Promise<Array<any>> => {
  let sql = `
select
    P.pid,
    P.name as process,
    (A.start_time - B.start_ts) as startTs,
    (case when A.end_time = -1 then 0 else (A.end_time - A.start_time) end) as dur,
    A.start_name as startName
from app_startup A,trace_range B
left join process P on A.ipid = P.ipid
where P.pid in (${ids.join(',')}) 
and not ((startTs + dur < ${leftNS}) or (startTs > ${rightNS}))
order by start_name;`;
  return query('getTabStartups', sql, {});
};

export const getTabStaticInit = (ids: Array<number>, leftNS: number, rightNS: number): Promise<Array<any>> => {
  let sql = `
select
    P.pid,
    P.name as process,
    (A.start_time - B.start_ts) as startTs,
    (case when A.end_time = -1 then 0 else (A.end_time - A.start_time) end) as dur,
    A.so_name as soName
from static_initalize A,trace_range B
left join process P on A.ipid = P.ipid
where P.pid in (${ids.join(',')}) 
and not ((startTs + dur < ${leftNS}) or (startTs > ${rightNS}))
order by dur desc;`;
  return query('getTabStaticInit', sql, {});
};

export const queryBinderArgsByArgset = (argset: number): Promise<Array<BinderArgBean>> =>
  query(
    'queryBinderArgsByArgset',
    `
    select
      *
    from
      args_view
    where
      argset = $argset;`,
    { $argset: argset }
  );

export const queryCpuData = (cpu: number, startNS: number, endNS: number): Promise<Array<CpuStruct>> =>
  query(
    'queryCpuData',
    `
    SELECT
    B.pid as processId,
    B.cpu,
    B.tid,
    B.itid as id,
    B.dur,
    B.ts - TR.start_ts AS startTime,
    B.arg_setid as argSetID
from thread_state AS B
    left join trace_range AS TR
where B.itid is not null
    and
      B.cpu = $cpu
    and
      startTime between $startNS and $endNS;`,
    {
      $cpu: cpu,
      $startNS: startNS,
      $endNS: endNS,
    }
  );

export const queryCpuFreq = (): Promise<Array<{ cpu: number; filterId: number }>> =>
  query(
    'queryCpuFreq',
    `
    select
      cpu,id as filterId
    from
      cpu_measure_filter
    where
      (name='cpufreq' or name='cpu_frequency')
    order by cpu;
    `
  );

export const queryCpuFreqData = (cpu: number): Promise<Array<CpuFreqStruct>> =>
  query<CpuFreqStruct>(
    'queryCpuFreqData',
    `
    select
      cpu,
      value,
      ifnull(dur,tb.end_ts - c.ts) dur,
      ts-tb.start_ts as startNS
    from
      measure c,
      trace_range tb
    inner join
      cpu_measure_filter t
    on
      c.filter_id = t.id
    where
      (name = 'cpufreq' or name='cpu_frequency')
    and
      cpu= $cpu
    --order by ts;
    `,
    { $cpu: cpu }
  );

export const queryCpuMax = (): Promise<Array<any>> =>
  query(
    'queryCpuMax',
    `
    select
      cpu
    from
      sched_slice
    order by
      cpu
    desc limit 1;`
  );

export const queryCpuDataCount = () =>
  query('queryCpuDataCount', 'select count(1) as count,cpu from thread_state where cpu not null group by cpu');

export const queryCpuCount = (): Promise<Array<any>> =>
  query(
    'queryCpuCount',
    `
   select max(cpuCount) cpuCount from
(select ifnull((max(cpu) + 1),0) cpuCount  from cpu_measure_filter where name in ('cpu_frequency','cpu_idle')
 union all
 select ifnull((max(callid)+1),0) cpuCount from irq
) A;`
  );

export const queryCpuSchedSlice = (): Promise<Array<any>> =>
  query(
    'queryCpuSchedSlice',
    `
   select (ts - start_ts) as ts,
       itid,
       end_state as endState,
       priority
   from sched_slice,trace_range;`
  );

export const queryCpuStateFilter = (): Promise<Array<any>> =>
  query(
    'queryCpuStateFilter',
    `select cpu,id as filterId from cpu_measure_filter where name = 'cpu_idle' order by cpu;`,
    {}
  );

export const queryCpuState = (cpuFilterId: number): Promise<Array<any>> =>
  query(
    'queryCpuState',
    `
        select (A.ts - B.start_ts) as startTs,ifnull(dur,B.end_ts - A.ts) dur,
            value
        from measure A,trace_range B
        where filter_id = $filterId;`,
    { $filterId: cpuFilterId }
  );

export const queryCpuMaxFreq = (): Promise<Array<any>> =>
  query(
    'queryCpuMaxFreq',
    `
    select
      max(value) as maxFreq
    from
      measure c
    inner join
      cpu_measure_filter t
    on
      c.filter_id = t.id
    where
      (name = 'cpufreq' or name='cpu_frequency');`
  );

export const queryProcessData = (pid: number, startNS: number, endNS: number): Promise<Array<any>> =>
  query(
    'queryProcessData',
    `
    select  ta.cpu,
        dur, 
        ts-${(window as any).recordStartNS} as startTime
from thread_state ta
where ta.cpu is not null and pid=$pid and startTime between $startNS and $endNS;`,
    {
      $pid: pid,
      $startNS: startNS,
      $endNS: endNS,
    }
  );

export const queryProcessMem = (): Promise<Array<any>> =>
  query(
    'queryProcessMem',
    `
    select
      process_measure_filter.id as trackId,
      process_measure_filter.name as trackName,
      ipid as upid,
      process.pid,
      process.name as processName
    from
      process_measure_filter
    join
      process using (ipid)
    order by trackName;`
  );

export const queryProcessThreadDataCount = (): Promise<Array<any>> =>
  query(
    `queryProcessThreadDataCount`,
    `select pid,count(id) as count 
    from thread_state 
    where ts between ${(window as any).recordStartNS} and ${(window as any).recordEndNS} group by pid;`,
    {}
  );

export const queryProcessFuncDataCount = (): Promise<Array<any>> =>
  query(
    `queryProcessFuncDataCount`,
    `select
        P.pid,
        count(tid) as count
    from callstack C
    left join thread A on A.id = C.callid
    left join process AS P on P.id = A.ipid
    where  C.ts between ${(window as any).recordStartNS} and ${(window as any).recordEndNS} 
    group by pid;`,
    {}
  );

export const queryProcessMemDataCount = (): Promise<Array<any>> =>
  query(
    `queryProcessMemDataCount`,
    `select
      p.pid as pid, count(value) count
    from process_measure c
    left join process_measure_filter f on f.id = c.filter_id
    left join process p on p.ipid = f.ipid
where f.id not NULL and value>0 
 and c.ts between ${(window as any).recordStartNS} and ${(window as any).recordEndNS}
group by p.pid`,
    {}
  );

export const queryProcessMemData = (trackId: number): Promise<Array<ProcessMemStruct>> =>
  query(
    'queryProcessMemData',
    `
    select
      c.type,
      ts,
      value,
      filter_id as track_id,
      c.ts-tb.start_ts startTime
    from
      process_measure c,
      trace_range tb
    where
      filter_id = $id;`,
    { $id: trackId }
  );

export const queryThreads = (): Promise<Array<any>> =>
  query('queryThreads', `select id,tid,(ifnull(name,'Thread') || '(' || tid || ')') name from thread where id != 0;`);

export const queryDataDICT = (): Promise<Array<any>> => query('queryDataDICT', `select * from data_dict;`);

export const queryAppStartupProcessIds = (): Promise<Array<{ pid: number }>> => query('queryAppStartupProcessIds', `
  SELECT pid FROM process 
  WHERE ipid IN (
    SELECT ipid FROM app_startup 
    UNION
    SELECT t.ipid FROM app_startup a LEFT JOIN thread t ON a.call_id = t.itid 
);`);
export const queryProcessContentCount = (): Promise<Array<any>> =>
  query(`queryProcessContentCount`, `select pid,switch_count,thread_count,slice_count,mem_count from process;`);
export const queryProcessThreadsByTable = (): Promise<Array<ThreadStruct>> =>
  query(
    'queryProcessThreadsByTable',
    `
        select p.pid as pid,p.ipid as upid,t.tid as tid,p.name as processName,t.name as threadName from thread t left join process  p on t.ipid = p.id where t.tid != 0;
    `
  );
export const queryVirtualMemory = (): Promise<Array<any>> =>
  query('queryVirtualMemory', `select id,name from sys_event_filter where type='sys_virtual_memory_filter'`);
export const queryVirtualMemoryData = (filterId: number): Promise<Array<any>> =>
  query(
    'queryVirtualMemoryData',
    `select ts-${
      (window as any).recordStartNS
    } as startTime,value,filter_id as filterID from sys_mem_measure where filter_id=$filter_id`,
    { $filter_id: filterId }
  );
export const queryProcessThreads = (): Promise<Array<ThreadStruct>> =>
  query(
    'queryProcessThreads',
    `
    select
      the_tracks.ipid as upid,
      the_tracks.itid as utid,
      total_dur as hasSched,
      process.pid as pid,
      thread.tid as tid,
      process.name as processName,
      thread.name as threadName
    from (
      select ipid,itid from sched_slice group by itid
    ) the_tracks
    left join (select itid,sum(dur) as total_dur from thread_state where state != 'S' group by itid) using(itid)
    left join thread using(itid)
    left join process using(ipid)
    order by total_dur desc,the_tracks.ipid,the_tracks.itid;`,
    {}
  );

export const queryThreadData = (tid: number, pid: number): Promise<Array<ThreadStruct>> =>
  query(
    'queryThreadData',
    `
    select 
      B.itid as id
     , B.tid
     , B.cpu
     , B.ts - TR.start_ts AS startTime
     , B.dur
     , B.state
     , B.pid
     , B.arg_setid as argSetID
from thread_state AS B
    left join trace_range AS TR
where B.tid = $tid and B.pid = $pid;`,
    { $tid: tid, $pid: pid }
  );

export const queryStartupPidArray = (): Promise<Array<{ pid: number }>> =>
  query(
    'queryStartupPidArray',
    `
    select distinct pid 
from app_startup A,trace_range B left join process P on A.ipid = p.ipid
where A.start_time between B.start_ts and B.end_ts;`,
    {}
  );

export const queryProcessStartup = (pid: number): Promise<Array<AppStartupStruct>> =>
  query(
    'queryProcessStartup',
    `
    select
    P.pid,
    A.tid,
    A.call_id as itid,
    (case when A.start_time < B.start_ts then 0 else (A.start_time - B.start_ts) end) as startTs,
    (case 
        when A.start_time < B.start_ts then (A.end_time - B.start_ts) 
        when A.end_time = -1 then 0
        else (A.end_time - A.start_time) end) as dur,
    A.start_name as startName
from app_startup A,trace_range B
left join process P on A.ipid = P.ipid
where P.pid = $pid
order by start_name;`,
    { $pid: pid }
  );

export const queryProcessSoMaxDepth = (): Promise<Array<{ pid: number; maxDepth: number }>> =>
  query(
    'queryProcessSoMaxDepth',
    `select p.pid,max(depth) maxDepth 
from static_initalize S,trace_range B left join process p on S.ipid = p.ipid 
where S.start_time between B.start_ts and B.end_ts
group by p.pid;`,
    {}
  );

export const queryProcessSoInitData = (pid: number): Promise<Array<SoStruct>> =>
  query(
    'queryProcessSoInitData',
    `
    select
    P.pid,
    T.tid,
    A.call_id as itid,
    (A.start_time - B.start_ts) as startTs,
    (A.end_time - A.start_time) as dur,
    A.so_name as soName,
    A.depth
from static_initalize A,trace_range B
left join process P on A.ipid = P.ipid
left join thread T on A.call_id = T.itid
where P.pid = $pid;`,
    { $pid: pid }
  );

export const queryThreadAndProcessName = (): Promise<Array<any>> =>
  query(
    'queryThreadAndProcessName',
    `
    select tid id,name,'t' type from thread
union all
select pid id,name,'p' type from process;`,
    {}
  );

export const queryThreadStateArgs = (argset: number): Promise<Array<BinderArgBean>> =>
  query('queryThreadStateArgs', ` select args_view.* from args_view where argset = ${argset}`, {});

export const queryWakeUpThread_Desc = (): Promise<Array<any>> =>
  query(
    'queryWakeUpThread_Desc',
    `This is the interval from when the task became eligible to run
(e.g.because of notifying a wait queue it was a suspended on) to when it started running.`
  );

export const queryThreadWakeUp = (itid: number, startTime: number, dur: number): Promise<Array<WakeupBean>> =>
  query(
    'queryThreadWakeUp',
    `
select TA.tid,min(TA.ts - TR.start_ts) as ts,TA.pid
from
  (select min(ts) as wakeTs,ref as itid from instant,trace_range
       where name = 'sched_wakeup'
       and wakeup_from = $itid
       and ts > start_ts + $startTime
       and ts < start_ts + $startTime + $dur
      group by ref
       ) TW
left join thread_state TA on TW.itid = TA.itid
left join trace_range TR
where TA.ts > TW.wakeTs
group by TA.tid,TA.pid;
    `,
    { $itid: itid, $startTime: startTime, $dur: dur }
  );

export const queryRunnableTimeByRunning = (tid: number, startTime: number): Promise<Array<WakeupBean>> => {
  let sql = `
select ts from thread_state,trace_range where ts + dur -start_ts = ${startTime} and state = 'R' and tid=${tid} limit 1
    `;
  return query('queryRunnableTimeByRunning', sql, {});
};

export const queryThreadWakeUpFrom = (itid: number, startTime: number): Promise<Array<WakeupBean>> => {
  let sql = `
select (A.ts - B.start_ts) as ts,
       A.tid,
       A.itid,
       A.pid,
       A.cpu,
       A.dur
from thread_state A,trace_range B
where A.state = 'Running'
and A.itid = (select wakeup_from from instant where ts = ${startTime} and ref = ${itid} limit 1)
and (A.ts - B.start_ts) < (${startTime} - B.start_ts)
order by ts desc limit 1
    `;
  return query('queryThreadWakeUpFrom', sql, {});
};
/*-------------------------------------------------------------------------------------*/

export const queryHeapGroupByEvent = (type: string): Promise<Array<NativeEventHeap>> => {
  let sql1 = `
        select
            event_type as eventType,
            sum(heap_size) as sumHeapSize
        from native_hook
        where event_type = 'AllocEvent' or event_type = 'MmapEvent'
        group by event_type
    `;
  let sql2 = `
        select (case when type = 0 then 'AllocEvent' else 'MmapEvent' end) eventType,
            sum(apply_size) sumHeapSize
        from native_hook_statistic
        group by eventType;
    `;
  return query('queryHeapGroupByEvent', type === 'native_hook' ? sql1 : sql2, {});
};

export const queryAllHeapByEvent = (): Promise<Array<NativeEvent>> =>
  query(
    'queryAllHeapByEvent',
    `
    select * from (
      select h.start_ts - t.start_ts as startTime,
       h.heap_size as heapSize,
       h.event_type as eventType
from native_hook h ,trace_range t
where h.start_ts >= t.start_ts and h.start_ts <= t.end_ts
and (h.event_type = 'AllocEvent' or h.event_type = 'MmapEvent')
union
select h.end_ts - t.start_ts as startTime,
       h.heap_size as heapSize,
       (case when h.event_type = 'AllocEvent' then 'FreeEvent' else 'MunmapEvent' end) as eventType
from native_hook h ,trace_range t
where h.start_ts >= t.start_ts and h.start_ts <= t.end_ts
and (h.event_type = 'AllocEvent' or h.event_type = 'MmapEvent')
and h.end_ts not null ) order by startTime;
`,
    {}
  );

export const queryHeapAllData = (
  startTs: number,
  endTs: number,
  ipids: Array<number>
): Promise<Array<HeapTreeDataBean>> =>
  query(
    'queryHeapAllData',
    `
    select
      h.start_ts - t.start_ts as startTs,
      h.end_ts - t.start_ts as endTs,
      h.heap_size as heapSize,
      h.event_type as eventType,
      h.callchain_id as eventId
    from
      native_hook h
    inner join
      trace_range  t
    where
      event_type = 'AllocEvent'
    and
      ipid in (${ipids.join(',')})
    and
      (h.start_ts - t.start_ts between ${startTs} and ${endTs} or h.end_ts - t.start_ts between ${startTs} and ${endTs})`,
    { ipids: ipids, $startTs: startTs, $endTs: endTs }
  );

export const queryNativeHookStatistics = (leftNs: number, rightNs: number): Promise<Array<NativeHookMalloc>> =>
  query(
    'queryNativeHookStatistics',
    `
    select
      event_type as eventType,
      sub_type_id as subTypeId,
      max(heap_size) as max,
      sum(case when ((A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}) then heap_size else 0 end) as allocByte,
      sum(case when ((A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}) then 1 else 0 end) as allocCount,
      sum(case when ((A.end_ts - B.start_ts) between ${leftNs} and ${rightNs} ) then heap_size else 0 end) as freeByte,
      sum(case when ((A.end_ts - B.start_ts) between ${leftNs} and ${rightNs} ) then 1 else 0 end) as freeCount
    from
      native_hook A,
      trace_range B
    where
      (A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}
     and (event_type = 'AllocEvent' or event_type = 'MmapEvent')
    group by event_type;`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const queryNativeHookStatisticsMalloc = (leftNs: number, rightNs: number): Promise<Array<NativeHookMalloc>> =>
  query(
    'queryNativeHookStatisticsMalloc',
    `
    select
      event_type as eventType,
      heap_size as heapSize,
      sum(case when ((A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}) then heap_size else 0 end) as allocByte,
      sum(case when ((A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}) then 1 else 0 end) as allocCount,
      sum(case when ((A.end_ts - B.start_ts) between ${leftNs} and ${rightNs} ) then heap_size else 0 end) as freeByte,
      sum(case when ((A.end_ts - B.start_ts) between ${leftNs} and ${rightNs} ) then 1 else 0 end) as freeCount
    from
      native_hook A,
      trace_range B
    where
      (A.start_ts - B.start_ts) between ${leftNs} and ${rightNs}
    and
      (event_type = 'AllocEvent' or event_type = 'MmapEvent')
    and 
      sub_type_id is null
    group by
      event_type,
      heap_size
    order by heap_size desc
    `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const queryNativeHookStatisticsSubType = (leftNs: number, rightNs: number): Promise<Array<NativeHookMalloc>> =>
  query(
    'queryNativeHookStatisticsSubType',
    `
    select
      event_type as eventType,
      sub_type_id as subTypeId,
      max(heap_size) as max,
      sum(case when ((NH.start_ts - TR.start_ts) between ${leftNs} and ${rightNs}) then heap_size else 0 end) as allocByte,
      sum(case when ((NH.start_ts - TR.start_ts) between ${leftNs} and ${rightNs}) then 1 else 0 end) as allocCount,
      sum(case when ((NH.end_ts - TR.start_ts) between ${leftNs} and ${rightNs} ) then heap_size else 0 end) as freeByte,
      sum(case when ((NH.end_ts - TR.start_ts) between ${leftNs} and ${rightNs} ) then 1 else 0 end) as freeCount
    from
      native_hook NH,
      trace_range TR
    where
      (NH.start_ts - TR.start_ts) between ${leftNs} and ${rightNs}
    and
      (event_type = 'MmapEvent')
    group by
      event_type,sub_type_id;
        `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const queryNativeHookSubType = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'queryNativeHookSubType',
    `select distinct sub_type_id as subTypeId, DD.data as subType
      from
        native_hook NH,
        trace_range TR
      left join data_dict DD on NH.sub_type_id = DD.id
      where
        NH.sub_type_id not null and
        (NH.start_ts - TR.start_ts) between ${leftNs} and ${rightNs}
        `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const queryNativeHookStatisticSubType = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'queryNativeHookStatisticSubType',
    `SELECT DISTINCT
      CASE
        WHEN type = 3 AND sub_type_id NOT NULL THEN sub_type_id
        ELSE type
      END AS subTypeId,
      CASE
        WHEN type = 2 THEN 'FILE_PAGE_MSG'
        WHEN type = 3 AND sub_type_id NOT NULL THEN D.data
        WHEN type = 3 THEN 'MEMORY_USING_MSG'
        ELSE 'MmapEvent'
      END AS subType
      FROM
        native_hook_statistic NHS
        LEFT JOIN data_dict D ON NHS.sub_type_id = D.id,
        trace_range TR
      WHERE
        NHS.type > 1 AND
        (NHS.ts - TR.start_ts) between ${leftNs} and ${rightNs}
      `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const queryNativeHookStatisticsCount = (): Promise<Array<NativeHookProcess>> =>
  query('queryNativeHookStatisticsCount', `select count(1) num from native_hook_statistic`, {});

export const queryNativeHookProcess = (table: string): Promise<Array<NativeHookProcess>> => {
  let sql = `
    select
      distinct ${table}.ipid,
      pid,
      name
    from
      ${table}
    left join
      process p
    on
      ${table}.ipid = p.id
    `;
  return query('queryNativeHookProcess', sql, {});
};

export const queryNativeHookSnapshotTypes = (): Promise<Array<NativeHookSampleQueryInfo>> =>
  query(
    'queryNativeHookSnapshotTypes',
    `
select
      event_type as eventType,
      data as subType
    from
      native_hook left join data_dict on native_hook.sub_type_id = data_dict.id
    where
      (event_type = 'AllocEvent' or event_type = 'MmapEvent')
    group by
      event_type,data;`,
    {}
  );

export const queryAllHookData = (rightNs: number): Promise<Array<NativeHookSampleQueryInfo>> =>
  query(
    'queryAllHookData',
    `
    select
      callchain_id as eventId,
      event_type as eventType,
      data as subType,
      addr,
      heap_size as growth,
      (n.start_ts - t.start_ts) as startTs,
      (n.end_ts - t.start_ts) as endTs
    from
      native_hook n left join data_dict on n.sub_type_id = data_dict.id,
      trace_range t
    where
      (event_type = 'AllocEvent' or event_type = 'MmapEvent')
    and
      n.start_ts between t.start_ts and ${rightNs} + t.start_ts`,
    { $rightNs: rightNs }
  );

export const queryNativeHookResponseTypes = (
  leftNs: number,
  rightNs: number,
  types: Array<string>
): Promise<Array<any>> =>
  query(
    'queryNativeHookResponseTypes',
    `
        select 
          distinct last_lib_id as lastLibId,
          data_dict.data as value 
        from 
          native_hook A ,trace_range B
          left join data_dict on A.last_lib_id = data_dict.id 
        where
        A.start_ts - B.start_ts
        between ${leftNs} and ${rightNs} and A.event_type in (${types.join(',')});
    `,
    { $leftNs: leftNs, $rightNs: rightNs, $types: types }
  );
/**
 * HiPerf
 */
export const queryHiPerfEventList = (): Promise<Array<any>> =>
  query('queryHiPerfEventList', `select id,report_value from perf_report where report_type='config_name'`, {});
export const queryHiPerfEventListData = (eventTypeId: number): Promise<Array<any>> =>
  query(
    'queryHiPerfEventListData',
    `
        select s.callchain_id,
               (s.timestamp_trace-t.start_ts) startNS 
        from perf_sample s,trace_range t 
        where 
            event_type_id=${eventTypeId} 
            and s.thread_id != 0
            and s.callchain_id != -1;
`,
    { $eventTypeId: eventTypeId }
  );
export const queryHiPerfEventData = (eventTypeId: number, cpu: number): Promise<Array<any>> =>
  query(
    'queryHiPerfEventList',
    `
    select s.callchain_id,
        (s.timestamp_trace-t.start_ts) startNS 
    from perf_sample s,trace_range t 
    where
        event_type_id=${eventTypeId} 
        and cpu_id=${cpu} 
        and s.thread_id != 0
        and s.callchain_id != -1;
`,
    { $eventTypeId: eventTypeId, $cpu: cpu }
  );
export const queryHiPerfCpuData = (cpu: number): Promise<Array<any>> =>
  query(
    'queryHiPerfCpuData',
    `
    select s.callchain_id,
        (s.timestamp_trace-t.start_ts) startNS 
    from perf_sample s,trace_range t 
    where 
        cpu_id=${cpu} 
        and s.thread_id != 0;`,
    { $cpu: cpu }
  );
export const queryHiPerfCpuMergeData = (): Promise<Array<any>> =>
  query(
    'queryHiPerfCpuData',
    `select s.callchain_id,(s.timestamp_trace-t.start_ts) startNS from perf_sample s,trace_range t 
where s.thread_id != 0;`,
    {}
  );
export const queryHiPerfCpuMergeData2 = (): Promise<Array<any>> =>
  query(
    'queryHiPerfCpuData2',
    `select distinct cpu_id from perf_sample where thread_id != 0 order by cpu_id desc;`,
    {}
  );

export const queryHiPerfProcessData = (pid: number): Promise<Array<any>> =>
  query(
    'queryHiPerfProcessData',
    `
SELECT sp.callchain_id,
       th.thread_name,
       th.thread_id                     tid,
       th.process_id                    pid,
       sp.timestamp_trace - tr.start_ts startNS
from perf_sample sp,
     trace_range tr
         left join perf_thread th on th.thread_id = sp.thread_id
where pid = ${pid} and sp.thread_id != 0 `,
    { $pid: pid }
  );

export const queryHiPerfThreadData = (tid: number): Promise<Array<any>> =>
  query(
    'queryHiPerfThreadData',
    `
SELECT sp.callchain_id,
       th.thread_name,
       th.thread_id                     tid,
       th.process_id                    pid,
       sp.timestamp_trace - tr.start_ts startNS
from perf_sample sp,
     trace_range tr
         left join perf_thread th on th.thread_id = sp.thread_id
where tid = ${tid} and sp.thread_id != 0 ;`,
    { $tid: tid }
  );

export const querySelectTraceStats = (): Promise<
  Array<{
    event_name: string;
    stat_type: string;
    count: number;
    source: string;
    serverity: string;
  }>
> => query('querySelectTraceStats', 'select event_name,stat_type,count,source,serverity from stat');

export const queryCustomizeSelect = (sql: string): Promise<Array<any>> => query('queryCustomizeSelect', sql);

export const queryDistributedTerm = (): Promise<
  Array<{
    threadId: string;
    threadName: string;
    processId: string;
    processName: string;
    funName: string;
    dur: string;
    ts: string;
    chainId: string;
    spanId: string;
    parentSpanId: string;
    flag: string;
    trace_name: string;
  }>
> =>
  query(
    'queryDistributedTerm',
    `
    select
      group_concat(thread.id,',') as threadId,
      group_concat(thread.name,',') as threadName,
      group_concat(process.id,',') as processId,
      group_concat(process.name,',') as processName,
      group_concat(callstack.name,',') as funName,
      group_concat(callstack.dur,',') as dur,
      group_concat(callstack.ts,',') as ts,
      cast(callstack.chainId as varchar) as chainId,
      callstack.spanId as spanId,
      callstack.parentSpanId as parentSpanId,
      group_concat(callstack.flag,',') as flag,
      (select
        value
      from
        meta
      where
        name='source_name') as trace_name
      from
        callstack
      inner join thread on callstack.callid = thread.id
      inner join process on process.id = thread.ipid
      where (callstack.flag='S' or callstack.flag='C')
      group by callstack.chainId,callstack.spanId,callstack.parentSpanId`
  );

export const queryTraceCpu = (): Promise<
  Array<{
    tid: string;
    pid: string;
    cpu: string;
    dur: string;
    min_freq: string;
    max_freq: string;
    avg_frequency: string;
  }>
> =>
  query(
    'queryTraceCpu',
    `SELECT 
        itid AS tid, 
        ipid AS pid, 
        group_concat(cpu, ',') AS cpu, 
        group_concat(dur, ',') AS dur, 
        group_concat(min_freq, ',') AS min_freq, 
        group_concat(max_freq, ',') AS max_freq, 
        group_concat(avg_frequency, ',') AS avg_frequency 
        FROM 
        (SELECT 
            itid, 
            ipid, 
            cpu, 
            CAST (SUM(dur) AS INT) AS dur, 
            CAST (MIN(freq) AS INT) AS min_freq, 
            CAST (MAX(freq) AS INT) AS max_freq, 
            CAST ( (SUM(dur * freq) / SUM(dur) ) AS INT) AS avg_frequency 
            from 
            result 
            group by 
            itid, cpu
        ) 
        GROUP BY 
        ipid, itid 
        ORDER BY 
        ipid
    `
  );

export const queryTraceCpuTop = (): Promise<
  Array<{
    tid: string;
    pid: string;
    cpu: string;
    duration: string;
    min_freq: string;
    max_freq: string;
    avg_frequency: string;
    sumNum: string;
  }>
> =>
  query(
    'queryTraceCpuTop',
    `SELECT
         ipid AS pid,
         itid AS tid, 
        group_concat(cpu, ',') AS cpu, 
        group_concat(dur, ',') AS dur,
        group_concat(avg_frequency, ',') AS avg_frequency,
        group_concat(min_freq, ',') AS min_freq, 
        group_concat(max_freq, ',') AS max_freq, 
        sum(dur * avg_frequency) AS sumNum 
        FROM 
        (SELECT 
            itid, 
            ipid, 
            cpu, 
            CAST (SUM(dur) AS INT) AS dur, 
            CAST (MIN(freq) AS INT) AS min_freq, 
            CAST (MAX(freq) AS INT) AS max_freq, 
            CAST ( (SUM(dur * freq) / SUM(dur) ) AS INT) AS avg_frequency 
            from result group by itid, cpu
        ) 
        GROUP BY 
        ipid, itid 
        ORDER BY 
        sumNum 
        DESC 
        LIMIT 10;
    `
  );

export const queryTraceMemory = (): Promise<
  Array<{
    maxNum: string;
    minNum: string;
    avgNum: string;
    name: string;
    processName: string;
  }>
> =>
  query(
    'queryTraceMemory',
    `
    select
        max(value) as maxNum,
        min(value) as minNum,
        avg(value) as avgNum,
        filter.name as name,
        p.name as processName
        from process_measure
        left join process_measure_filter as filter on filter.id= filter_id
        left join process as p on p.id = filter.ipid
    where 
    filter_id > 0 
    and 
    filter.name = 'mem.rss.anon' 
    group by 
    filter_id 
    order by 
    avgNum desc`
  );

export const queryTraceMemoryTop = (): Promise<
  Array<{
    maxNum: string;
    minNum: string;
    avgNum: string;
    name: string;
    processName: string;
  }>
> =>
  query(
    'queryTraceMemoryTop',
    `
    select
        max(value) as maxNum,
        min(value) as minNum,
        avg(value) as avgNum,
        f.name as name,
        p.name as processName
        from process_measure
        left join process_measure_filter as f on f.id= filter_id
        left join process as p on p.id = f.ipid
    where
    filter_id > 0
    and
    f.name = 'mem.rss.anon'
    group by
    filter_id
    order by
    avgNum desc limit 10`
  );

export const queryTraceMemoryUnAgg = (): Promise<
  Array<{
    processName: string;
    name: string;
    value: string;
    ts: string;
  }>
> =>
  query(
    'queryTraceMemoryUnAgg',
    `
    select
        p.name as processName,
        group_concat(filter.name) as name,
        cast(group_concat(value) as varchar) as value,
        cast(group_concat(ts) as varchar) as ts
        from process_measure m
        left join process_measure_filter as filter on filter.id= m.filter_id
        left join process as p on p.id = filter.ipid
        where 
        filter.name = 'mem.rss.anon' 
        or 
        filter.name = 'mem.rss.file' 
        or 
        filter.name = 'mem.swap' 
        or 
        filter.name = 'oom_score_adj'
    group by 
    p.name,filter.ipid 
    order by 
    filter.ipid`
  );

export const queryTraceTaskName = (): Promise<
  Array<{
    id: string;
    pid: string;
    process_name: string;
    thread_name: string;
  }>
> =>
  query(
    'queryTraceTaskName',
    `
    select
        P.id as id,
        P.pid as pid,
        P.name as process_name,
        group_concat(T.name,',') as thread_name
    from process as P left join thread as T where P.id = T.ipid
    group by pid`
  );

export const queryTraceMetaData = (): Promise<
  Array<{
    name: string;
    valueText: string;
  }>
> =>
  query(
    'queryTraceMetaData',
    `
    select
        cast(name as varchar) as name,
        cast(value as varchar) as valueText 
        from meta
        UNION
        select 'start_ts',cast(start_ts as varchar) from trace_range
        UNION
        select 'end_ts',cast(end_ts as varchar) from trace_range`
  );

export const querySystemCalls = (): Promise<
  Array<{
    frequency: string;
    minDur: string;
    maxDur: string;
    avgDur: string;
    funName: string;
  }>
> =>
  query(
    'querySystemCalls',
    `
    select
      count(*) as frequency,
      min(dur) as minDur,
      max(dur) as maxDur,
      avg(dur) as avgDur,
      name as funName
    from
      callstack
      group by name
      order by
    frequency desc limit 100`
  );

export const querySystemCallsTop = (): Promise<
  Array<{
    tid: string;
    pid: string;
    funName: string;
    frequency: string;
    minDur: string;
    maxDur: string;
    avgDur: string;
  }>
> =>
  query(
    'querySystemCallsTop',
    `SELECT 
        cpu.tid AS tid, 
        cpu.pid AS pid, 
        callstack.name AS funName, 
        count(callstack.name) AS frequency, 
        min(callstack.dur) AS minDur, 
        max(callstack.dur) AS maxDur, 
        round(avg(callstack.dur)) AS avgDur 
        FROM 
        callstack 
        INNER JOIN 
        (SELECT 
            itid AS tid, 
            ipid AS pid, 
            group_concat(cpu, ',') AS cpu, 
            group_concat(dur, ',') AS dur, 
            group_concat(min_freq, ',') AS min_freq, 
            group_concat(max_freq, ',') AS max_freq, 
            group_concat(avg_frequency, ',') AS avg_frequency, 
            sum(dur * avg_frequency) AS sumNum 
            FROM 
            (SELECT 
                itid, 
                ipid, 
                cpu, 
                CAST (SUM(dur) AS INT) AS dur, 
                CAST (MIN(freq) AS INT) AS min_freq, 
                CAST (MAX(freq) AS INT) AS max_freq, 
                CAST ( (SUM(dur * freq) / SUM(dur) ) AS INT) AS avg_frequency 
                FROM 
                result 
                GROUP BY 
                itid, cpu
            ) 
            GROUP BY 
            ipid, itid 
            ORDER BY 
            sumNum 
            DESC 
            LIMIT 10
        ) AS cpu 
        ON 
        callstack.callid = cpu.tid 
        GROUP BY 
        callstack.name 
        ORDER BY 
        frequency 
        DESC
    LIMIT 10`
  );

export const getTabLiveProcessData = (leftNs: number, rightNs: number): Promise<Array<LiveProcess>> =>
  query<LiveProcess>(
    'getTabLiveProcessData',
    `SELECT
        process.id as processId,
        process.name as processName,
        process.ppid as responsibleProcess,
        process.uud as userName,
        process.usag as cpu,
        process.threadN as threads,
        process.pss as memory,
        process.cpu_time as cpuTime,
        process.disk_reads as diskReads,
        process.disk_writes as diskWrite
        FROM
        (
        SELECT
        tt.process_id AS id,
        tt.process_name AS name,
        tt.parent_process_id AS ppid,
        tt.uid as uud,
        tt.cpu_usage as usag,
        tt.thread_num AS threadN,
        mt.maxTT - TR.start_ts as endTs,
        tt.pss_info as pss,
        tt.cpu_time,
        tt.disk_reads,
        tt.disk_writes
        FROM
        live_process tt
        LEFT JOIN trace_range AS TR 
        LEFT JOIN (select re.process_id as idd, max(re.ts) as maxTT, min(re.ts) as minTT 
        from live_process re GROUP BY re.process_name, re.process_id ) mt
        on mt.idd = tt.process_id where endTs >= $rightNS
        GROUP BY
        tt.process_name,
        tt.process_id 
        ) process ;`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabProcessHistoryData = (
  leftNs: number,
  rightNs: number,
  processId: number | undefined,
  threadId: number | undefined
): Promise<Array<ProcessHistory>> =>
  query<ProcessHistory>(
    'getTabProcessHistoryData',
    `SELECT
        process.id as processId,
        process.isD as alive,
        process.startTS as firstSeen,
        process.endTs as lastSeen,
        process.name as processName,
        process.ppid as responsibleProcess,
        process.uuid as userName,
        process.cpu_time as cpuTime,
        0 as pss
        FROM
        (
        SELECT
        tt.process_id AS id,
        tt.process_name AS name,
        tt.parent_process_id AS ppid,
        tt.uid AS uuid,
        tt.cpu_time,
        (mt.minTT - TR.start_ts ) AS startTS,
        mt.maxTT - TR.start_ts as endTs,
        (mt.maxTT - TR.start_ts - $rightNS) > 0 as isD
        FROM
        live_process tt
        LEFT JOIN trace_range AS TR 
        LEFT JOIN (select re.process_id as idd, max(re.ts) as maxTT, min(re.ts) as minTT 
        from live_process re GROUP BY re.process_name, re.process_id ) mt
        on mt.idd = tt.process_id 
        GROUP BY
        tt.process_name,
        tt.process_id 
        ) process;`,
    {
      $leftNS: leftNs,
      $rightNS: rightNs,
      $processID: processId,
      $threadID: threadId,
    }
  );

export const getTabCpuAbilityData = (leftNs: number, rightNs: number): Promise<Array<SystemCpuSummary>> =>
  query<SystemCpuSummary>(
    'getTabCpuAbilityData',
    `SELECT
        ( n.ts - TR.start_ts ) AS startTime,
        n.dur AS duration,
        n.total_load AS totalLoad,
        n.user_load AS userLoad,
        n.system_load AS systemLoad,
        n.process_num AS threads 
        FROM
        cpu_usage AS n,
        trace_range AS TR 
        WHERE
        ( n.ts - TR.start_ts ) >= ifnull((
        SELECT
        ( usage.ts - TR.start_ts ) 
        FROM
        cpu_usage usage,
        trace_range TR 
        WHERE
        ( usage.ts - TR.start_ts ) <= $leftNS 
        ORDER BY
        usage.ts DESC 
        LIMIT 1 
        ),0)
        AND ( n.ts - TR.start_ts ) <= $rightNS 
        ORDER BY
        startTime ASC;
    `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabMemoryAbilityData = (
  leftNs: number,
  rightNs: number
): Promise<
  Array<{
    startTime: number;
    value: string;
    name: string;
  }>
> =>
  query(
    'getTabMemoryAbilityData',
    `SELECT
        m.ts AS startTime,
        GROUP_CONCAT( IFNULL( m.value, 0 ) ) AS value,
        GROUP_CONCAT( f.name ) AS name 
        FROM
        sys_mem_measure AS m
        INNER JOIN sys_event_filter AS f ON m.filter_id = f.id 
        AND (f.name = 'sys.mem.total' 
         or f.name = 'sys.mem.free'
         or f.name = 'sys.mem.buffers'
         or f.name = 'sys.mem.cached' 
         or f.name = 'sys.mem.shmem'
         or f.name = 'sys.mem.slab'
         or f.name = 'sys.mem.swap.total'
         or f.name = 'sys.mem.swap.free'
         or f.name = 'sys.mem.mapped'
         or f.name = 'sys.mem.vmalloc.used'
         or f.name = 'sys.mem.page.tables'
         or f.name = 'sys.mem.kernel.stack'
         or f.name = 'sys.mem.active'
         or f.name = 'sys.mem.inactive'
         or f.name = 'sys.mem.unevictable'
         or f.name = 'sys.mem.vmalloc.total'
         or f.name = 'sys.mem.slab.unreclaimable'
         or f.name = 'sys.mem.cma.total'
         or f.name = 'sys.mem.cma.free'
         or f.name = 'sys.mem.kernel.reclaimable'
         or f.name = 'sys.mem.zram'
         ) 
        AND m.ts >= ifnull((
        SELECT
        m.ts AS startTime 
        FROM
        sys_mem_measure AS m
        INNER JOIN sys_event_filter AS f ON m.filter_id = f.id 
        AND m.ts <= $leftNS 
        AND (f.name = 'sys.mem.total'
         or f.name = 'sys.mem.kernel.stack'
         or f.name = 'sys.mem.free'
         or f.name = 'sys.mem.swap.free'
         or f.name = 'sys.mem.cma.free'
         or f.name = 'sys.mem.inactive'
         or f.name = 'sys.mem.buffers'
         or f.name = 'sys.mem.cached' 
         or f.name = 'sys.mem.shmem'
         or f.name = 'sys.mem.slab'
         or f.name = 'sys.mem.swap.total'
         or f.name = 'sys.mem.vmalloc.used'
         or f.name = 'sys.mem.page.tables'
         or f.name = 'sys.mem.active'
         or f.name = 'sys.mem.unevictable'
         or f.name = 'sys.mem.vmalloc.total'
         or f.name = 'sys.mem.slab.unreclaimable'
         or f.name = 'sys.mem.cma.total'
         or f.name = 'sys.mem.mapped'
         or f.name = 'sys.mem.kernel.reclaimable'
         or f.name = 'sys.mem.zram'
         ) 
        ORDER BY
        m.ts DESC 
        LIMIT 1 
        ),0)
        AND m.ts <= $rightNS GROUP BY m.ts;`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabNetworkAbilityData = (leftNs: number, rightNs: number): Promise<Array<SystemNetworkSummary>> =>
  query<SystemNetworkSummary>(
    'getTabNetworkAbilityData',
    `SELECT
            ( n.ts - TR.start_ts ) AS startTime,
            n.dur AS duration,
            n.rx AS dataReceived,
            n.tx_speed AS dataReceivedSec,
            n.tx AS dataSend,
            n.rx_speed AS dataSendSec,
            n.packet_in AS packetsIn,
            n.packet_in_sec AS packetsInSec,
            n.packet_out AS packetsOut,
            n.packet_out_sec AS packetsOutSec 
            FROM
            network AS n,
            trace_range AS TR 
            WHERE
            ( n.ts - TR.start_ts ) >= ifnull((
            SELECT
            ( nn.ts - T.start_ts ) AS startTime 
            FROM
            network nn,
            trace_range T 
            WHERE
            ( nn.ts - T.start_ts ) <= $leftNS
            ORDER BY
            nn.ts DESC 
            LIMIT 1 
            ),0)  
            AND ( n.ts - TR.start_ts ) <= $rightNS 
            ORDER BY
            startTime ASC`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabDiskAbilityData = (leftNs: number, rightNs: number): Promise<Array<SystemDiskIOSummary>> =>
  query<SystemDiskIOSummary>(
    'getTabDiskAbilityData',
    `SELECT
        ( n.ts - TR.start_ts ) AS startTime,
        n.dur AS duration,
        n.rd AS dataRead,
        n.rd_speed AS dataReadSec,
        n.wr AS dataWrite,
        n.wr_speed AS dataWriteSec,
        n.rd_count AS readsIn,
        n.rd_count_speed AS readsInSec,
        n.wr_count AS writeOut,
        n.wr_count_speed AS writeOutSec 
        FROM
        diskio AS n,
        trace_range AS TR 
        WHERE
        ( n.ts - TR.start_ts ) >= ifnull((
        SELECT
        ( nn.ts - T.start_ts ) AS startTime 
        FROM
        diskio AS nn,
        trace_range AS T 
        WHERE
        ( nn.ts - T.start_ts ) <= $leftNS 
        ORDER BY
        nn.ts DESC 
        LIMIT 1 
        ),0)
        AND ( n.ts - TR.start_ts ) <= $rightNS 
        ORDER BY
        startTime ASC;
    `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const queryCpuAbilityData = (): Promise<Array<CpuAbilityMonitorStruct>> =>
  query(
    'queryCpuAbilityData',
    `select 
        (t.total_load) as value,
        (t.ts - TR.start_ts) as startNS
        from cpu_usage t, trace_range AS TR;`
  );

export const queryCpuAbilityUserData = (): Promise<Array<CpuAbilityMonitorStruct>> =>
  query(
    'queryCpuAbilityUserData',
    `select 
        t.user_load as value,
        (t.ts - TR.start_ts) as startNS
        from cpu_usage t, trace_range AS TR;`
  );

export const queryCpuAbilitySystemData = (): Promise<Array<CpuAbilityMonitorStruct>> =>
  query(
    'queryCpuAbilitySystemData',
    `select 
        t.system_load as value,
        (t.ts - TR.start_ts) as startNS
        from cpu_usage t, trace_range AS TR;`
  );

export const queryMemoryUsedAbilityData = (id: string): Promise<Array<MemoryAbilityMonitorStruct>> =>
  query(
    'queryMemoryUsedAbilityData',
    `select 
        t.value as value,
        (t.ts - TR.start_ts) as startNS
        from sys_mem_measure t, trace_range AS TR where t.filter_id = $id;`,
    { $id: id }
  );

export const queryCachedFilesAbilityData = (id: string): Promise<Array<MemoryAbilityMonitorStruct>> =>
  query(
    'queryCachedFilesAbilityData',
    `select 
        t.value as value,
        (t.ts - TR.start_ts) as startNS
        from sys_mem_measure t, trace_range AS TR where t.filter_id = $id;`,
    { $id: id }
  );

export const queryCompressedAbilityData = (id: string): Promise<Array<MemoryAbilityMonitorStruct>> =>
  query(
    'queryCompressedAbilityData',
    `select 
        t.value as value,
        (t.ts - TR.start_ts) as startNS
        from sys_mem_measure t, trace_range AS TR where t.filter_id = $id;`,
    { $id: id }
  );

export const querySwapUsedAbilityData = (id: string): Promise<Array<MemoryAbilityMonitorStruct>> =>
  query(
    'querySwapUsedAbilityData',
    `select 
        t.value as value,
        (t.ts - TR.start_ts) as startNS
        from sys_mem_measure t, trace_range AS TR where t.filter_id = $id;`,
    { $id: id }
  );

export const queryBytesReadAbilityData = (): Promise<Array<DiskAbilityMonitorStruct>> =>
  query(
    'queryBytesReadAbilityData',
    `select 
        t.rd_speed as value,
        (t.ts - TR.start_ts) as startNS
        from diskio t, trace_range AS TR;`
  );

export const queryBytesWrittenAbilityData = (): Promise<Array<DiskAbilityMonitorStruct>> =>
  query(
    'queryBytesWrittenAbilityData',
    `select 
        t.wr_speed as value,
        (t.ts - TR.start_ts) as startNS
        from diskio t, trace_range AS TR;`
  );

export const queryReadAbilityData = (): Promise<Array<DiskAbilityMonitorStruct>> =>
  query(
    'queryReadAbilityData',
    `select 
        t.rd_count_speed as value,
        (t.ts - TR.start_ts) as startNS
        from diskio t, trace_range AS TR;`
  );

export const queryWrittenAbilityData = (): Promise<Array<DiskAbilityMonitorStruct>> =>
  query(
    'queryWrittenAbilityData',
    `select 
        t.wr_count_speed as value,
        (t.ts - TR.start_ts) as startNS
        from diskio t, trace_range AS TR;`
  );

export const queryBytesInAbilityData = (): Promise<Array<NetworkAbilityMonitorStruct>> =>
  query(
    'queryBytesInAbilityData',
    `select 
        t.tx_speed as value,
        (t.ts - TR.start_ts) as startNS
        from network t, trace_range AS TR;`
  );

export const queryBytesOutAbilityData = (): Promise<Array<NetworkAbilityMonitorStruct>> =>
  query(
    'queryBytesOutAbilityData',
    `select 
        t.rx_speed as value,
        (t.ts - TR.start_ts) as startNS
        from network t, trace_range AS TR;`
  );

export const queryPacketsInAbilityData = (): Promise<Array<NetworkAbilityMonitorStruct>> =>
  query(
    'queryPacketsInAbilityData',
    `select 
        t.packet_in_sec as value,
        (t.ts - TR.start_ts) as startNS
        from network t, trace_range AS TR;`
  );

export const queryPacketsOutAbilityData = (): Promise<Array<NetworkAbilityMonitorStruct>> =>
  query(
    'queryPacketsOutAbilityData',
    `select 
        t.packet_out_sec as value,
        (t.ts - TR.start_ts) as startNS
        from network t, trace_range AS TR;`
  );

export const queryNetWorkMaxData = (): Promise<Array<any>> =>
  query(
    'queryNetWorkMaxData',
    `select 
     ifnull(max(tx_speed),0) as maxIn, 
     ifnull(max(rx_speed),0) as maxOut,
     ifnull(max(packet_in_sec),0) as maxPacketIn,
     ifnull(max(packet_in_sec),0) as maxPacketOut
     from network`
  );

export const queryMemoryMaxData = (memoryName: string): Promise<Array<any>> =>
  query(
    'queryMemoryMaxData',
    `SELECT ifnull(max(m.value),0) as maxValue,
            filter_id 
            from sys_mem_measure m 
            WHERE m.filter_id =
            (SELECT id FROM sys_event_filter WHERE name = $memoryName)
`,
    { $memoryName: memoryName }
  );

export const queryDiskIoMaxData = (): Promise<Array<any>> =>
  query(
    'queryDiskIoMaxData',
    `select
    ifnull(max(rd_speed),0) as bytesRead, 
    ifnull(max(wr_speed),0) as bytesWrite,
    ifnull(max(rd_count_speed),0) as readOps,
    ifnull(max(wr_count_speed),0)  as writeOps
    from diskio`
  );

export const queryAbilityExits = (): Promise<Array<any>> =>
  query(
    'queryAbilityExits',
    `select 
      event_name 
      from stat s 
      where s.event_name in ('trace_diskio','trace_network', 'trace_cpu_usage','sys_memory') 
      and s.stat_type ='received' and s.count > 0`
  );

export const queryStartTime = (): Promise<Array<any>> => query('queryStartTime', `SELECT start_ts FROM trace_range`);

export const queryPerfFiles = (): Promise<Array<PerfFile>> =>
  query('queryPerfFiles', `select file_id as fileId,symbol,path from perf_files`, {});

export const queryPerfProcess = (): Promise<Array<PerfThread>> =>
  query(
    'queryPerfThread',
    `select process_id as pid,thread_name as processName from perf_thread where process_id = thread_id`,
    {}
  );

export const queryPerfThread = (): Promise<Array<PerfThread>> =>
  query(
    'queryPerfThread',
    `select a.thread_id as tid,a.thread_name as threadName,a.process_id as pid,b.thread_name as processName from perf_thread a left join (select * from perf_thread where thread_id = process_id) b on a.process_id = b.thread_id`,
    {}
  );

export const queryPerfSampleListByTimeRange = (
  leftNs: number,
  rightNs: number,
  cpus: Array<number>,
  processes: Array<number>,
  threads: Array<number>
): Promise<Array<PerfSample>> => {
  let sql = `
select A.callchain_id as sampleId,
       A.thread_id as tid,
       C.thread_name as threadName,
       A.thread_state as state,
       C.process_id as pid,
       (timestamp_trace - R.start_ts) as time,
       cpu_id as core
from perf_sample A,trace_range R
left join perf_thread C on A.thread_id = C.thread_id
where time >= $leftNs and time <= $rightNs and A.thread_id != 0
    `;
  if (cpus.length != 0 || processes.length != 0 || threads.length != 0) {
    let arg1 = cpus.length > 0 ? `or core in (${cpus.join(',')}) ` : '';
    let arg2 = processes.length > 0 ? `or pid in (${processes.join(',')}) ` : '';
    let arg3 = threads.length > 0 ? `or tid in (${threads.join(',')})` : '';
    let arg = `${arg1}${arg2}${arg3}`.substring(3);
    sql = `${sql} and (${arg})`;
  }
  return query('queryPerfSampleListByTimeRange', sql, {
    $leftNs: leftNs,
    $rightNs: rightNs,
  });
};

export const queryPerfSampleIdsByTimeRange = (
  leftNs: number,
  rightNs: number,
  cpus: Array<number>,
  processes: Array<number>,
  threads: Array<number>
): Promise<Array<PerfSample>> => {
  let sql = `
select A.callchain_id as sampleId 
from perf_sample A,trace_range R
left join perf_thread C on A.thread_id = C.thread_id
where (timestamp_trace - R.start_ts) >= $leftNs and (timestamp_trace - R.start_ts) <= $rightNs and A.thread_id != 0 
    `;
  if (cpus.length != 0 || processes.length != 0 || threads.length != 0) {
    let arg1 = cpus.length > 0 ? `or A.cpu_id in (${cpus.join(',')}) ` : '';
    let arg2 = processes.length > 0 ? `or C.process_id in (${processes.join(',')}) ` : '';
    let arg3 = threads.length > 0 ? `or A.thread_id in (${threads.join(',')})` : '';
    let arg = `${arg1}${arg2}${arg3}`.substring(3);
    sql = `${sql} and (${arg})`;
  }
  return query('queryPerfSampleIdsByTimeRange', sql, {
    $leftNs: leftNs,
    $rightNs: rightNs,
  });
};

export const queryPerfSampleCallChain = (sampleId: number): Promise<Array<PerfStack>> =>
  query(
    'queryPerfSampleCallChain',
    `
    select
    callchain_id as callChainId,
    callchain_id as sampleId,
    file_id as fileId,
    symbol_id as symbolId,
    vaddr_in_file as vaddrInFile,
    name as symbol
from perf_callchain where callchain_id = $sampleId and symbol_id != -1 and vaddr_in_file != 0;
    `,
    { $sampleId: sampleId }
  );

export const queryPerfCmdline = (): Promise<Array<PerfCmdLine>> =>
  query(
    'queryPerfCmdline',
    `
    select report_value from perf_report  where report_type = 'cmdline'
    `,
    {}
  );

export const queryCPuAbilityMaxData = (): Promise<Array<any>> =>
  query(
    'queryCPuAbilityMaxData',
    `select ifnull(max(total_load),0) as totalLoad, 
                ifnull(max(user_load),0) as userLoad,
                ifnull(max(system_load),0) as systemLoad
                from cpu_usage`
  );

export const querySearchFunc = (search: string): Promise<Array<SearchFuncBean>> =>
  query(
    'querySearchFunc',
    `
   select c.cookie,c.id,c.name as funName,c.ts - r.start_ts as startTime,c.dur,c.depth,t.tid,t.name as threadName
   ,p.pid ,'func' as type from callstack c left join thread t on c.callid = t.id left join process p on t.ipid = p.id
   left join trace_range r 
   where c.name like '%${search}%' and startTime > 0;
    `,
    { $search: search }
  );

export const querySceneSearchFunc = (search: string, processList: Array<string>): Promise<Array<SearchFuncBean>> =>
  query(
    'querySearchFunc',
    `
   select c.cookie,c.id,c.name as funName,c.ts - r.start_ts as startTime,c.dur,c.depth,t.tid,t.name as threadName
   ,p.pid ,'func' as type from callstack c left join thread t on c.callid = t.id left join process p on t.ipid = p.id
   left join trace_range r
   where c.name like '%${search}%' and startTime > 0 and p.pid in (${processList.join(',')});
    `,
    { $search: search }
  );

export const queryBinderBySliceId = (id: number): Promise<Array<any>> =>
  query(
    'queryBinderBySliceId',
    `select c.ts-D.start_ts as startTime,
    c.dur,
    t.tid,p.pid,c.depth 
    from callstack c,trace_range D
    left join thread t on c.callid = t.id
    left join process p on p.id = t.ipid
where cat = 'binder' and c.id = $id;`,
    { $id: id }
  );

export const queryThreadByItid = (itid: number, ts: number): Promise<Array<any>> =>
  query(
    'queryThreadByItid',
    `select tid,pid,c.dur,c.depth,c.name 
from thread t left join process p on t.ipid = p.ipid
left join callstack c on t.itid = c.callid
where itid = $itid and c.ts = $ts;`,
    { $itid: itid, $ts: ts }
  );

export const queryBinderByArgsId = (id: number, startTime: number, isNext: boolean): Promise<Array<any>> => {
  let sql = `select c.ts - D.start_ts as startTime,
    c.dur,
    t.tid,p.pid,c.depth 
    from callstack c,trace_range D
    left join thread t on c.callid = t.id
    left join process p on p.id = t.ipid
where cat = 'binder' and  c.argsetid = $id`;
  if (isNext) {
    sql += ' and c.ts > $startTime +  D.start_ts';
  } else {
    sql += ' and c.ts < $startTime +  D.start_ts';
  }
  return query('queryBinderByArgsId', sql, {
    $id: id,
    $startTime: startTime,
  });
};

export const getTabPaneFilesystemStatisticsFather = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'getTabPaneFilesystemStatisticsFather',
    `
    select SUM(dur) as allDuration,
    count(f.type) as count,
    min(dur) as minDuration,
    max(dur) as maxDuration,
    round(avg(dur),2) as avgDuration,
    p.name,
    f.type,
    p.pid,
    sum(ifnull(size,0)) as size
    from file_system_sample as f
    left join process as p on f.ipid=p.ipid
    where f.start_ts >= $leftNs
    and end_ts <= $rightNs
    group by f.type;
    `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabPaneFilesystemStatisticsChild = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'getTabPaneFilesystemStatisticsChild',
    `
    select SUM(dur)    as allDuration,
        count(f.type) as count,
        min(dur)    as minDuration,
        max(dur)    as maxDuration,
        round(avg(dur),2)    as avgDuration,
        p.name,
        p.pid,
        f.type,
        sum(ifnull(size,0))    as size
        from file_system_sample as f left join process as p on f.ipid=p.ipid
        where f.start_ts >= $leftNs
        and end_ts <= $rightNs
        group by f.type, f.ipid;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabPaneFilesystemStatisticsAll = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'getTabPaneFilesystemStatisticsAll',
    `
    select SUM(dur)    as allDuration,
       count(type) as count,
       min(dur)    as minDuration,
       max(dur)    as maxDuration,
       round(avg(dur),2)    as avgDuration,
       type
    from file_system_sample
    where start_ts >= $leftNs
    and end_ts <= $rightNs;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabPaneFilesystemStatistics = (leftNs: number, rightNs: number, types: number[]): Promise<Array<any>> =>
  query(
    'getTabPaneFilesystemStatistics',
    `
    select p.pid,
       ifnull(p.name,'Process') as name,
       f.type,
       count(f.ipid) as count,
       sum(ifnull(size,0)) as size,
       sum(case when f.type = 2 then ifnull(size,0) else 0 end) as logicalReads,
       sum(case when f.type = 3 then ifnull(size,0) else 0 end) as logicalWrites,
       sum(case when f.type != 2 and f.type != 3 then ifnull(size,0) else 0 end) as otherFile,
       sum(dur) as allDuration,
       min(dur) as minDuration,
       max(dur) as maxDuration,
       avg(dur) as avgDuration
    from file_system_sample as f left join process as p on f.ipid=p.ipid
    where end_ts >= $leftNs
    and end_ts <= $rightNs
    and f.type in (${types.join(',')})
    group by f.type,f.ipid
    order by f.type;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabPaneVirtualMemoryStatisticsData = (leftNs: number, rightNs: number): Promise<Array<any>> =>
  query(
    'getTabPaneVirtualMemoryStatisticsData',
    `
    select p.pid,
       t.tid,
       ifnull(p.name,'Process') as pname,
       ifnull(t.name,'Thread') as tname,
       f.type,
       f.ipid,
       f.itid,
       count(f.ipid) as count,
       sum(dur) as allDuration,
       min(dur) as minDuration,
       max(dur) as maxDuration,
       avg(dur) as avgDuration
    from paged_memory_sample as f left join process as p on f.ipid=p.ipid left join thread as t on f.itid=t.itid
    where f.end_ts >= $leftNs
    and f.end_ts <= $rightNs
    group by f.type,f.ipid,f.itid
    order by f.type;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );

export const getTabPaneIOTierStatisticsData = (
  leftNs: number,
  rightNs: number,
  diskIOipids: Array<number>
): Promise<Array<any>> => {
  let str = '';
  if (diskIOipids.length > 0) {
    str = ` and i.ipid in (${diskIOipids.join(',')})`;
  }
  return query(
    'getTabPaneIOTierStatisticsData',
    `
    select p.pid,
       ifnull(p.name,'Process') as pname,
       i.tier,
       i.ipid,
       path_id as path,
       count(i.ipid) as count,
       sum(latency_dur) as allDuration,
       min(latency_dur) as minDuration,
       max(latency_dur) as maxDuration,
       avg(latency_dur) as avgDuration
    from bio_latency_sample as i left join process as p on i.ipid=p.ipid
    where i.start_ts+latency_dur >= $leftNs
    and i.start_ts+latency_dur <= $rightNs
    ${str}
    group by i.tier,i.ipid,i.path_id
    order by i.tier;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );
};

export const getTabPaneCounterSampleData = (
  leftNs: number,
  rightNs: number,
  cpuStateFilterIds: Array<number>
): Promise<Array<any>> => {
  let str = '';
  if (cpuStateFilterIds.length > 0) {
    str = ` and filter_id in (${cpuStateFilterIds.join(',')})`;
  }
  return query(
    'getTabPaneCounterSampleData',
    `
    select value, filter_id as filterId, ts, f.cpu
    from measure left join cpu_measure_filter as f on f.id=filter_id
    where
    ts <= $rightNs${str} order by ts asc;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );
};

export const getTabPaneFrequencySampleData = (
  leftNs: number,
  rightNs: number,
  cpuFreqFilterIds: Array<number>
): Promise<Array<any>> => {
  let str = '';
  if (cpuFreqFilterIds.length > 0) {
    str = ` and filter_id in (${cpuFreqFilterIds.join(',')})`;
  }
  return query(
    'getTabPaneFrequencySampleData',
    `
    select value, filter_id as filterId, ts, f.cpu
    from measure left join cpu_measure_filter as f on f.id=filter_id
    where
    ts <= $rightNs${str} order by ts asc;
`,
    { $leftNs: leftNs, $rightNs: rightNs }
  );
};

export const hasFileSysData = (): Promise<Array<any>> =>
  query(
    'hasFileSysData',
    `
    select 
        fsCount,
        vmCount,
        ioCount from
        (select count(1) as fsCount from file_system_sample s,trace_range t where (s.start_ts between t.start_ts and t.end_ts) or (s.end_ts between t.start_ts and t.end_ts) )
        ,(select count(1) as vmCount from paged_memory_sample s,trace_range t where (s.start_ts between t.start_ts and t.end_ts) or (s.end_ts between t.start_ts and t.end_ts) )
        ,(select count(1) as ioCount from bio_latency_sample s,trace_range t where (s.start_ts between t.start_ts and t.end_ts) or (s.end_ts between t.start_ts and t.end_ts) );
    `,
    {}
  );

export const getFileSysChartDataByType = (type: number): Promise<Array<any>> =>
  query(
    'getFileSysChartData',
    `
    select
       (A.start_ts -B.start_ts) as startNS,
       (A.end_ts - B.start_ts) as endNS,
       dur
    from file_system_sample A,trace_range B
    where type = $type and startNS > 0;`,
    { $type: type },
    'exec'
  );

export const getFileSysVirtualMemoryChartData = (): Promise<Array<any>> =>
  query(
    'getFileSysVirtualMemoryChartData',
    `
    select
       (A.start_ts -B.start_ts) as startNS,
       (A.end_ts - B.start_ts) as endNS,
       dur as dur
    from paged_memory_sample A,trace_range B
    where startNS > 0
    order by A.start_ts;`,
    {},
    'exec'
  );

export const getDiskIOProcess = (): Promise<Array<any>> =>
  query(
    'getDiskIOProcess',
    `
    select name,B.ipid,pid
    from (select distinct ipid from bio_latency_sample A,trace_range B where A.start_ts between B.start_ts and B.end_ts) A
    left join process B on A.ipid = B.ipid;`,
    {}
  );

export const getDiskIOLatencyChartDataByProcess = (
  all: boolean,
  ipid: number,
  typeArr: Array<number>
): Promise<Array<any>> =>
  query(
    'getDiskIOLatencyChartDataByProcess',
    `
    select
       (A.start_ts -B.start_ts) as startNS,
       (A.start_ts - B.start_ts + A.latency_dur) as endNS,
       latency_dur as dur
    from bio_latency_sample A,trace_range B
    where type in (${typeArr.join(',')}) and startNS > 0
        ${all ? '' : 'and ipid = ' + ipid}
    order by A.start_ts;`,
    {},
    'exec'
  );

export const querySdkCount = (sql: string, componentId: number, args?: any): Promise<Array<any>> =>
  query('querySdkCount', sql, args, 'exec-sdk-' + componentId);

export const querySdkCounterData = (
  sql: string,
  counter_id: number,
  componentId: number
): Promise<Array<CounterStruct>> =>
  query('querySdkCounterData', sql, { $counter_id: counter_id }, 'exec-sdk-' + componentId);

export const getTabSdkCounterData = (
  sqlStr: string,
  startTime: number,
  leftNs: number,
  rightNs: number,
  counters: Array<string>,
  componentId: number
): Promise<Array<CounterSummary>> =>
  query<CounterSummary>(
    'getTabSdkCounterData',
    sqlStr,
    {
      $startTime: startTime,
      $leftNs: leftNs,
      $rightNs: rightNs,
      $counters: counters,
    },
    'exec-sdk-' + componentId
  );

export const getTabSdkCounterLeftData = (
  sqlStr: string,
  leftNs: number,
  counters: Array<string>,
  componentId: number
): Promise<Array<any>> =>
  query<any>(
    'getTabSdkCounterLeftData',
    sqlStr,
    {
      $leftNs: leftNs,
      $counters: counters,
    },
    'exec-sdk-' + componentId
  );

export const getTabSdkSliceData = (
  sqlStr: string,
  startTime: number,
  leftNs: number,
  rightNs: number,
  slices: Array<string>,
  componentId: number
): Promise<Array<SdkSliceSummary>> =>
  query<SdkSliceSummary>(
    'getTabSdkSliceData',
    sqlStr,
    {
      $startTime: startTime,
      $leftNs: leftNs,
      $rightNs: rightNs,
      $slices: slices,
    },
    'exec-sdk-' + componentId
  );

export const querySdkSliceData = (
  sqlStr: string,
  column_id: number,
  startNS: number,
  endNS: number,
  componentId: number
): Promise<Array<SdkSliceStruct>> =>
  query(
    'querySdkSliceData',
    sqlStr,
    { $column_id: column_id, $startNS: startNS, $endNS: endNS },
    'exec-sdk-' + componentId
  );

export const queryCounterMax = (sqlStr: string, counter_id: number, componentId: number): Promise<Array<any>> =>
  query('queryCounterMax', sqlStr, { $counter_id: counter_id }, 'exec-sdk-' + componentId);

export const queryAnomalyData = (): Promise<Array<EnergyAnomalyStruct>> =>
  query(
    'queryAnomalyData',
    `select 
      (S.ts - TR.start_ts) as startNS,
      D.data as eventName,
      D2.data as appKey, 
      (case when S.type==1 then group_concat(S.string_value,',') else group_concat(S.int_value,',') end) as Value
      from trace_range AS TR,hisys_event_measure as S 
      left join data_dict as D on D.id=S.name_id 
      left join app_name as APP on APP.id=S.key_id 
      left join data_dict as D2 on D2.id=APP.app_key
      where D.data in ('ANOMALY_SCREEN_OFF_ENERGY','ANOMALY_KERNEL_WAKELOCK','ANOMALY_CPU_HIGH_FREQUENCY','ANOMALY_WAKEUP')
     or (D.data in ('ANOMALY_RUNNINGLOCK','ANORMALY_APP_ENERGY','ANOMALY_GNSS_ENERGY','ANOMALY_CPU_ENERGY','ANOMALY_ALARM_WAKEUP') and D2.data in ("APPNAME")) 
      group by S.serial,D.data`
  );

export const querySystemLocationData = (): Promise<
  Array<{
    startNs: string;
    eventName: string;
    type: string;
    state: string;
  }>
> =>
  query(
    'querySystemLocationData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS Value 
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key 
        WHERE
        D.data = 'GNSS_STATE' AND D2.data = 'STATE'
        GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data;`
  );

export const querySystemLockData = (): Promise<
  Array<{
    startNs: string;
    eventName: string;
    type: string;
    state: string;
  }>
> =>
  query(
    'querySystemLockData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat(( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS Value 
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key 
        WHERE
        ( D.data = 'POWER_RUNNINGLOCK' AND D2.data in ('TAG','MESSAGE')) 
        GROUP BY
        S.serial;`
  );

export const querySystemSchedulerData = (): Promise<
  Array<{
    startNs: string;
    eventName: string;
    appKey: string;
    Value: string;
  }>
> =>
  query(
    'querySystemSchedulerData',
    `SELECT
        ( S.ts - TR.start_ts ) AS startNs,
        D.data AS eventName,
        group_concat(D2.data, ',') AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS Value 
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key 
        WHERE
        D.data IN ( 'WORK_REMOVE', 'WORK_STOP', 'WORK_ADD' ) AND D2.data in ('NAME','TYPE','WORKID') 
        GROUP BY
        S.serial;`
  );

export const querySystemDetailsData = (rightNs: number, eventName: string): Promise<Array<SystemDetailsEnergy>> =>
  query(
    'querySystemDetailsData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS appValue
    FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key
    WHERE
        D.data in ($eventName)
    AND
        D2.data in ('UID', 'TYPE', 'WORKID', 'NAME', 'INTERVAL', 'TAG', 'STATE', 'STACK', 'APPNAME', 'MESSAGE', 'PID', 'LOG_LEVEL')
    AND
        (S.ts - TR.start_ts) <= $rightNS
    GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data;`,
    { $rightNS: rightNs, $eventName: eventName }
  );

export const querySystemWorkData = (rightNs: number): Promise<Array<SystemDetailsEnergy>> =>
  query(
    'querySystemWorkData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS appValue
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D
        ON D.id = S.name_id
        LEFT JOIN app_name AS APP
        ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2
        ON D2.id = APP.app_key
        WHERE
        D.data in ("WORK_REMOVE", "WORK_STOP", "WORK_ADD", "WORK_START")
        and
        D2.data in ('UID', 'TYPE', 'WORKID', 'NAME', 'INTERVAL', 'TAG', 'STATE', 'STACK', 'APPNAME', 'MESSAGE', 'PID', 'LOG_LEVEL')
        and (S.ts - TR.start_ts) <= $rightNS
        GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data;`,
    { $rightNS: rightNs }
  );

export const queryMaxPowerValue = (
  appName: string
): Promise<
  Array<{
    maxValue: number;
  }>
> =>
  query(
    'queryMaxPowerValue',
    `SELECT
        max( item ) AS maxValue 
        FROM
        (
            SELECT 
            sum( energy + background_energy + screen_on_energy + screen_off_energy + foreground_energy ) AS item 
            FROM 
            energy 
            WHERE 
            app_name = $appName 
            GROUP BY 
            startNs);`,
    { $appName: appName }
  );

export const queryPowerData = (): Promise<
  Array<{
    startNS: number;
    eventName: string;
    appKey: string;
    eventValue: string;
  }>
> =>
  query(
    'queryPowerData',
    `SELECT
        ( S.ts - TR.start_ts ) AS startNS,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS eventValue
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D
        ON D.id = S.name_id
        LEFT JOIN app_name AS APP
        ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2
        ON D2.id = APP.app_key
        where
        D.data in ('POWER_IDE_CPU','POWER_IDE_LOCATION','POWER_IDE_GPU','POWER_IDE_DISPLAY','POWER_IDE_CAMERA','POWER_IDE_BLUETOOTH','POWER_IDE_FLASHLIGHT','POWER_IDE_AUDIO','POWER_IDE_WIFISCAN')
        and
        D2.data in ('BACKGROUND_ENERGY','FOREGROUND_ENERGY','SCREEN_ON_ENERGY','SCREEN_OFF_ENERGY','ENERGY','APPNAME')
        GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data
        ORDER BY
        eventName;`,
    {}
  );

export const getTabPowerDetailsData = (
  leftNs: number,
  rightNs: number
): Promise<
  Array<{
    startNS: number;
    eventName: string;
    appKey: string;
    eventValue: string;
  }>
> =>
  query(
    'getTabPowerDetailsData',
    `SELECT
        ( S.ts - TR.start_ts ) AS startNS,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS eventValue
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key
        where
        D.data in ('POWER_IDE_CPU','POWER_IDE_LOCATION','POWER_IDE_GPU','POWER_IDE_DISPLAY','POWER_IDE_CAMERA','POWER_IDE_BLUETOOTH','POWER_IDE_FLASHLIGHT','POWER_IDE_AUDIO','POWER_IDE_WIFISCAN')
        and
        D2.data in ('APPNAME')
        GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data
        UNION
        SELECT
        ( S.ts - TR.start_ts ) AS startNS,
        D1.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS eventValue
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D1 ON D1.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key
        where
        D1.data in ('POWER_IDE_CPU','POWER_IDE_LOCATION','POWER_IDE_GPU','POWER_IDE_DISPLAY','POWER_IDE_CAMERA','POWER_IDE_BLUETOOTH','POWER_IDE_FLASHLIGHT','POWER_IDE_AUDIO','POWER_IDE_WIFISCAN')
        and
        D2.data in ('CHARGE','BACKGROUND_TIME','SCREEN_ON_TIME','SCREEN_OFF_TIME','LOAD','USAGE','DURATION','CAMERA_ID',
        'FOREGROUND_COUNT','BACKGROUND_COUNT','SCREEN_ON_COUNT','SCREEN_OFF_COUNT','COUNT','UID','FOREGROUND_DURATION',
        'FOREGROUND_ENERGY','BACKGROUND_DURATION','BACKGROUND_ENERGY','SCREEN_ON_DURATION','SCREEN_ON_ENERGY',
        'SCREEN_OFF_DURATION','SCREEN_OFF_ENERGY','ENERGY')
        and
        (S.ts - TR.start_ts) >= $leftNS
        and (S.ts - TR.start_ts) <= $rightNS
        GROUP BY
        S.serial,
        APP.app_key,
        D1.data,
        D2.data
        ORDER BY
        eventName;`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const getTabPowerBatteryData = (
  rightNs: number
): Promise<
  Array<{
    ts: number;
    eventName: string;
    appKey: string;
    eventValue: string;
  }>
> =>
  query(
    'getTabPowerBatteryData',
    `select
      MAX(S.ts) as ts,
      D.data as eventName,
      D2.data as appKey, 
      group_concat((case when S.type==1 then S.string_value else S.int_value end), ',') as eventValue 
      from 
      trace_range AS TR,
      hisys_event_measure as S 
      left join 
      data_dict as D 
      on 
      D.id=S.name_id 
      left join 
      app_name as APP 
      on 
      APP.id=S.key_id 
      left join 
      data_dict as D2 
      on 
      D2.id=APP.app_key
      where 
      D.data = 'POWER_IDE_BATTERY'
      and D2.data in ('GAS_GAUGE','CHARGE','SCREEN','LEVEL','CURRENT','CAPACITY','UID')
      and (S.ts - TR.start_ts) >= 0
      and (S.ts - TR.start_ts) <= $rightNS 
      group by APP.app_key,D.data,D2.data;`,
    { $rightNS: rightNs }
  );

export const queryMaxStateValue = (
  eventName: string
): Promise<
  Array<{
    type: string;
    maxValue: number;
  }>
> =>
  query(
    'queryMaxStateValue',
    `select 
  D.data as type,
  max(S.int_value) as maxValue 
  from trace_range AS TR,hisys_event_measure as S 
  left join data_dict as D on D.id=S.name_id 
  left join app_name as APP on APP.id=S.key_id 
  left join data_dict as D2 on D2.id=APP.app_key
  where (case when 'SENSOR_STATE'==$eventName then D.data like '%SENSOR%' else D.data = $eventName end)
  and D2.data in ('BRIGHTNESS','STATE','VALUE','LEVEL','VOLUME','OPER_TYPE','VOLUME')
  group by APP.app_key,D.data,D2.data;`,
    { $eventName: eventName }
  );

export const queryStateData = (eventName: string): Promise<Array<EnergyStateStruct>> =>
  query(
    'queryStateData',
    `select
  (S.ts-TR.start_ts) as startNs,
  D.data as type,
  D2.data as appKey, 
  S.int_value as value 
  from trace_range AS TR,hisys_event_measure as S 
  left join data_dict as D on D.id=S.name_id 
  left join app_name as APP on APP.id=S.key_id 
  left join data_dict as D2 on D2.id=APP.app_key
  where (case when 'SENSOR_STATE'==$eventName then D.data like '%SENSOR%' else D.data = $eventName end)
  and D2.data in ('BRIGHTNESS','STATE','VALUE','LEVEL','VOLUME','OPER_TYPE','VOLUME')
  group by S.serial,APP.app_key,D.data,D2.data;`,
    { $eventName: eventName }
  );

export const querySyseventAppName = (): Promise<
  Array<{
    string_value: string | null;
  }>
> =>
  query(
    'querySyseventAppName',
    `
    SELECT
    DISTINCT hisys_event_measure.string_value from data_dict 
    left join app_name on app_name.app_key=data_dict.id 
    left join hisys_event_measure on hisys_event_measure.key_id = app_name.id
    where data_dict.data = "APPNAME"`
  );

export const queryAnomalyDetailedData = (leftNs: number, rightNs: number): Promise<Array<EnergyAnomalyStruct>> =>
  query<EnergyAnomalyStruct>(
    'queryAnomalyDetailedData',
    `select
  S.ts,
  D.data as eventName,
  D2.data as appKey,
  group_concat((case when S.type==1 then S.string_value else S.int_value end), ',') as Value
  from trace_range AS TR,hisys_event_measure as S
  left join data_dict as D on D.id=S.name_id
  left join app_name as APP on APP.id=S.key_id
  left join data_dict as D2 on D2.id=APP.app_key
  where D.data in ('ANOMALY_SCREEN_OFF_ENERGY','ANOMALY_ALARM_WAKEUP','ANOMALY_KERNEL_WAKELOCK',
  'ANOMALY_RUNNINGLOCK','ANORMALY_APP_ENERGY','ANOMALY_GNSS_ENERGY','ANOMALY_CPU_HIGH_FREQUENCY','ANOMALY_CPU_ENERGY','ANOMALY_WAKEUP')
  and D2.data in ('APPNAME')
  and (S.ts - TR.start_ts) >= $leftNS
   and (S.ts - TR.start_ts) <= $rightNS
  group by S.serial,APP.app_key,D.data,D2.data
  union
  select
  S.ts,
  D.data as eventName,
  D2.data as appKey,
  group_concat((case when S.type == 1 then S.string_value else S.int_value end), ',') as Value
  from trace_range AS TR,hisys_event_measure as S
  left join data_dict as D on D.id = S.name_id
  left join app_name as APP on APP.id = S.key_id
  left join data_dict as D2 on D2.id = APP.app_key
  where D.data in ('ANOMALY_SCREEN_OFF_ENERGY', 'ANOMALY_ALARM_WAKEUP', 'ANOMALY_KERNEL_WAKELOCK',
  'ANOMALY_RUNNINGLOCK', 'ANORMALY_APP_ENERGY', 'ANOMALY_GNSS_ENERGY', 'ANOMALY_CPU_HIGH_FREQUENCY', 'ANOMALY_CPU_ENERGY', 'ANOMALY_WAKEUP')
  and D2.data not in ('pid_', 'tid_', 'type_', 'tz_', 'uid_', 'domain_', 'id_', 'level_', 'info_', 'tag_', 'APPNAME')
  and (S.ts - TR.start_ts) >= $leftNS
  and (S.ts - TR.start_ts) <= $rightNS
  group by S.serial, APP.app_key, D.data, D2.data;`,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

export const queryGpuTotalType = (): Promise<Array<{ id: number; data: string }>> =>
  query(
    'queryGpuTotalType',
    `
  select distinct module_name_id id,data
    from memory_window_gpu A, trace_range TR left join data_dict B on A.module_name_id = B.id
    where window_name_id = 0
    and A.ts < TR.end_ts;
  `
  );

export const queryGpuDataByTs = (
  ts: number,
  window: number,
  module: number | null
): Promise<
  Array<{
    windowNameId: number;
    windowId: number;
    moduleId: number;
    categoryId: number;
    size: number;
  }>
> => {
  let condition =
    module === null
      ? `and window_name_id = ${window}`
      : `and window_name_id = ${window} and module_name_id = ${module}`;
  let sql = `select window_name_id as windowNameId,
       window_id as windowId,
       module_name_id as moduleId,
       category_name_id as categoryId,
       size
       from memory_window_gpu, trace_range
       where ts - start_ts = ${ts} ${condition};`;
  return query('queryGpuDataByTs', sql);
};

export const queryGpuTotalData = (moduleId: number | null): Promise<Array<{ startNs: number; value: number }>> => {
  let moduleCondition = moduleId === null ? '' : `and module_name_id = ${moduleId}`;
  let sql = `
  select (ts - start_ts) startNs, sum(size) value
    from memory_window_gpu,trace_range
    where window_name_id = 0 ${moduleCondition}
    and ts< end_ts
    group by ts;
  `;
  return query('queryGpuTotalData', sql);
};

export const queryGpuGLData = (ipid: number): Promise<Array<{ startNs: number; value: number }>> => {
  let sql = `
  select (ts - start_ts) startNs,sum(value) value
from process_measure, trace_range
where filter_id = (
    select id
    from process_measure_filter
    where name = 'mem.gl_pss' and ipid = ${ipid}
    )
and ts between start_ts and end_ts
group by ts;
  `;
  return query('queryGpuGLData', sql);
};

export const queryGpuGLDataByRange = (
  ipid: number,
  leftNs: number,
  rightNs: number,
  interval: number
): Promise<Array<{ startTs: number; size: number }>> => {
  let sql = `
  select (ts - start_ts) startTs,sum(value) size
from process_measure, trace_range
where filter_id = (
    select id
    from process_measure_filter
    where name = 'mem.gl_pss' and ipid = ${ipid}
    )
and not ((startTs + ${interval} < ${leftNs}) or (startTs > ${rightNs}))
group by ts;
  `;
  return query('queryGpuGLDataByRange', sql);
};

export const queryGpuDataByRange = (
  leftNs: number,
  rightNs: number,
  interval: number
): Promise<
  Array<{
    startTs: number;
    windowId: number;
    moduleId: number;
    categoryId: number;
    avgSize: number;
    maxSize: number;
    minSize: number;
  }>
> => {
  let sql = `select (ts - start_ts) startTs,
    window_name_id windowId,
    module_name_id moduleId,
    category_name_id categoryId,
    avg(size) avgSize,
    max(size) maxSize,
    min(size) minSize
  from memory_window_gpu,trace_range
  where not ((startTs + ${interval} < ${leftNs}) or (startTs > ${rightNs}))
  group by window_name_id,module_name_id,category_name_id
  order by avgSize DESC;
  `;
  return query('queryGpuWindowData', sql);
};

export const queryGpuWindowData = (
  windowId: number,
  moduleId: number | null
): Promise<Array<{ startNs: number; value: number }>> => {
  let moduleCondition = moduleId === null ? '' : `and module_name_id = ${moduleId}`;
  let sql = `
  select (ts - start_ts) startNs, sum(size) value
    from memory_window_gpu,trace_range
    where window_name_id = ${windowId} ${moduleCondition}
    and ts < end_ts
    group by ts;
  `;
  return query('queryGpuWindowData', sql);
};

export const queryGpuWindowType = (): Promise<Array<{ id: number; data: string; pid: number }>> =>
  query(
    'queryGpuWindowType',
    `
  select distinct A.window_name_id as id,B.data, null as pid
from memory_window_gpu A, trace_range tr left join data_dict B on A.window_name_id = B.id
where window_name_id != 0
and A.ts < tr.end_ts
union all
select distinct A.module_name_id id, B.data, A.window_name_id pid
from memory_window_gpu A, trace_range TR left join data_dict B on A.module_name_id = B.id
where window_name_id != 0
and A.ts < TR.end_ts
  `
  );

export const querySmapsExits = (): Promise<Array<any>> =>
  query(
    'querySmapsExits',
    `select
      event_name
      from stat s
      where s.event_name = 'trace_smaps'
      and s.stat_type ='received' and s.count > 0`
  );

export const querySmapsData = (columnName: string): Promise<Array<any>> =>
  query(
    'querySmapsCounterData',
    `SELECT (A.timestamp - B.start_ts) as startNs, sum(${columnName}) * 1024 as value, $columnName as name FROM smaps A,trace_range B WHERE A.timestamp < B.end_ts GROUP by A.timestamp;`,
    { $columnName: columnName }
  );

export const querySmapsDataMax = (columnName: string): Promise<Array<any>> =>
  query(
    'querySmapsDataMax',
    `
   SELECT (A.timestamp - B.start_ts) as startNS,sum(${columnName}) as max_value FROM smaps A,trace_range B GROUP by A.timestamp order by max_value desc LIMIT 1`
  );

export const getTabSmapsMaxSize = (leftNs: number, rightNs: number, dur: number): Promise<Array<any>> =>
  query<Smaps>(
    'getTabSmapsMaxRss',
    `
SELECT (A.timestamp - B.start_ts) as startNS, sum(virtaul_size) *1024 as max_value FROM smaps A,trace_range B where startNS <= $rightNs and (startNS+$dur)>=$leftNs`,
    { $rightNs: rightNs, $leftNs: leftNs, $dur: dur }
  );

export const getTabSmapsData = (leftNs: number, rightNs: number, dur: number): Promise<Array<Smaps>> =>
  query<Smaps>(
    'getTabSmapsData',
    `
    SELECT
     (A.timestamp - t.start_ts) AS startNs,
     start_addr as startAddr,
     end_addr as endAddr,
     A.type,
     resident_size * 1024 AS rss,
     protection_id as pid,
     pss * 1024 as pss,virtaul_size * 1024 AS size,reside,A.path_id AS path,
     shared_clean * 1024 as sharedClean,shared_dirty * 1024 as sharedDirty,private_clean * 1024 as privateClean,
     private_dirty * 1024 as privateDirty,swap * 1024 as swap,swap_pss * 1024 as swapPss
     FROM smaps A,
     trace_range AS t
     WHERE (startNs) <= $rightNs and (startNs+$dur) >=$leftNs`,
    { $rightNs: rightNs, $leftNs: leftNs, $dur: dur },
    'exec'
  );

export const getTabVirtualMemoryType = (startTime: number, endTime: number): Promise<Array<string>> =>
  query(
    'getTabVirtualMemoryType',
    `
    SELECT type from paged_memory_sample s,trace_range t
     WHERE s.end_ts between $startTime + t.start_ts and $endTime + t.start_ts group by type`,
    { $startTime: startTime, $endTime: endTime },
    'exec'
  );

export const getTabIoCompletionTimesType = (startTime: number, endTime: number): Promise<Array<string>> =>
  query(
    'getTabIoCompletionTimesType',
    `
    SELECT tier from bio_latency_sample s,trace_range t
     WHERE s.start_ts + s.latency_dur between $startTime + t.start_ts and $endTime + t.start_ts group by tier`,
    { $startTime: startTime, $endTime: endTime },
    'exec'
  );

export const getCpuLimitFreqId = (): Promise<Array<CpuFreqRowLimit>> =>
  query(
    'getCpuMaxMinFreqId',
    `
    select cpu,MAX(iif(name = 'cpu_frequency_limits_max',id,0)) as maxFilterId,MAX(iif(name = 'cpu_frequency_limits_min',id,0)) as minFilterId from cpu_measure_filter where name in ('cpu_frequency_limits_max','cpu_frequency_limits_min') group by cpu
`,
    {}
  );

export const getCpuLimitFreqMax = (filterIds: string): Promise<Array<any>> => {
  return query(
    'getCpuLimitFreqMax',
    `
    select max(value) as maxValue,filter_id as filterId from measure where filter_id in (${filterIds}) group by filter_id
`,
    {}
  );
};

export const getCpuLimitFreq = (maxId: number, minId: number, cpu: number): Promise<Array<CpuFreqLimitsStruct>> =>
  query(
    'getCpuLimitFreq',
    `
    select ts - T.start_ts as startNs,dur,max(value) as max,min(value) as min,$cpu as cpu from measure,trace_range T where filter_id in ($maxId,$minId) group by ts
`,
    { $maxId: maxId, $minId: minId, $cpu: cpu }
  );

export const queryHisystemEventExits = (): Promise<Array<any>> =>
  query(
    'queryHisystemEventExits',
    `select 
      event_name 
      from stat s 
      where s.event_name = 'trace_hisys_event' 
      and s.stat_type ='received' and s.count > 0`
  );

export const queryEbpfSamplesCount = (startTime: number, endTime: number, ipids: number[]): Promise<Array<any>> =>
  query(
    'queryEbpfSamplesCount',
    `
    select
fsCount,
    vmCount from
(select count(1) as fsCount from file_system_sample s,trace_range t where s.end_ts between $startTime + t.start_ts and $endTime + t.start_ts ${
      ipids.length > 0 ? `and s.ipid in (${ipids.join(',')})` : ''
    })
,(select count(1) as vmCount from paged_memory_sample s,trace_range t where s.end_ts between $startTime + t.start_ts and $endTime + t.start_ts ${
      ipids.length > 0 ? `and s.ipid in (${ipids.join(',')})` : ''
    });
`,
    { $startTime: startTime, $endTime: endTime }
  );

export const querySysLockDetailsData = (rightNs: number, eventName: string): Promise<Array<SystemDetailsEnergy>> =>
  query(
    'querySysLockDetailsData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS appValue
    FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key
    WHERE
        D.data in ($eventName)
    AND
        D2.data in ('UID', 'TYPE', 'WORKID', 'NAME', 'INTERVAL', 'TAG', 'STATE', 'STACK', 'APPNAME', 'MESSAGE', 'PID', 'LOG_LEVEL')
    AND
        (S.ts - TR.start_ts) <= $rightNS
    GROUP BY
        S.serial, APP.app_key, D.data, D2.data;`,
    { $rightNS: rightNs, $eventName: eventName }
  );

export const queryStateInitValue = (eventName: string, keyName: string): Promise<Array<EnergyStateStruct>> =>
  query(
    'queryStateInitValue',
    `select
  0 as startNs,
  $eventName as type,
  '' as appKey,
  (case $keyName
  when 'brightness' then device_state.brightness
  when 'wifi' then device_state.wifi
  when 'bt_state' then device_state.bt_state
  when 'location' then device_state.location
  else 0 end) as value
  from device_state;`,
    { $eventName: eventName, $keyName: keyName }
  );

export const querySysLocationDetailsData = (rightNs: number, eventName: string): Promise<Array<SystemDetailsEnergy>> =>
  query(
    'querySysLocationDetailsData',
    `SELECT
        ( S.ts - TR.start_ts ) AS ts,
        D.data AS eventName,
        D2.data AS appKey,
        group_concat( ( CASE WHEN S.type == 1 THEN S.string_value ELSE S.int_value END ), ',' ) AS appValue 
        FROM
        trace_range AS TR,
        hisys_event_measure AS S
        LEFT JOIN data_dict AS D ON D.id = S.name_id
        LEFT JOIN app_name AS APP ON APP.id = S.key_id
        LEFT JOIN data_dict AS D2 ON D2.id = APP.app_key 
        WHERE
        D.data in ($eventName) 
        and 
        D2.data in ('UID', 'TYPE', 'WORKID', 'NAME', 'INTERVAL', 'TAG', 'STATE', 'STACK', 'APPNAME', 'MESSAGE', 'PID', 'LOG_LEVEL')
        and (S.ts - TR.start_ts) <= $rightNS
        GROUP BY
        S.serial,
        APP.app_key,
        D.data,
        D2.data;`,
    { $rightNS: rightNs, $eventName: eventName }
  );
export const queryNativeMemoryRealTime = (): Promise<Array<any>> =>
  query(
    'queryNativeMemoryRealTime',
    `select cs.ts,cs.clock_name from datasource_clockid dc left join clock_snapshot cs on dc.clock_id = cs.clock_id where data_source_name = 'memory-plugin' or data_source_name = 'nativehook'
`,
    {}
  );

export const queryBootTime = (): Promise<Array<any>> =>
  query(
    'queryBootTime',
    `select CS.ts -TR.start_ts as ts ,clock_name from clock_snapshot as CS ,trace_range as TR
      where clock_name = 'boottime'`,
    {}
  );

export const queryConfigSysEventAppName = (): Promise<
  Array<{
    process_name: string;
  }>
> =>
  query(
    'queryConfigSysEventAppName',
    `
    SELECT value from trace_config where trace_source = 'hisys_event' and key = 'process_name'`
  );

export const queryClockData = (): Promise<
  Array<{
    name: string;
    num: number;
    srcname: string;
  }>
> =>
  query(
    'queryClockData',
    `
    select name || ' Frequency' name, COUNT(*) num, name srcname
from (select id, name
      from clock_event_filter
      where type = 'clock_set_rate')
group by name
union
select name || ' State' name, COUNT(*) num, name srcname
from (select id, name
      from clock_event_filter
      where type != 'clock_set_rate')
group by name;
`
  );

export const queryClockFrequency = (clockName: string): Promise<Array<ClockStruct>> =>
  query(
    'queryClockFrequency',
    `with freq as (  select measure.filter_id, measure.ts, measure.type, measure.value from clock_event_filter
left join measure
where clock_event_filter.name = $clockName and clock_event_filter.type = 'clock_set_rate' and clock_event_filter.id = measure.filter_id
order by measure.ts)
select freq.filter_id as filterId,freq.ts - r.start_ts as startNS,freq.type,freq.value from freq,trace_range r order by startNS`,
    { $clockName: clockName }
  );

export const queryClockState = (clockName: string): Promise<Array<ClockStruct>> =>
  query(
    'queryClockState',
    `with state as (
select filter_id, ts, endts, endts-ts as dur, type, value from
(select measure.filter_id, measure.ts, lead(ts, 1, null) over( order by measure.ts) endts, measure.type, measure.value from clock_event_filter,trace_range
left join measure
where clock_event_filter.name = $clockName and clock_event_filter.type != 'clock_set_rate' and clock_event_filter.id = measure.filter_id
order by measure.ts))
select s.filter_id as filterId,s.ts-r.start_ts as startNS,s.type,s.value,s.dur from state s,trace_range r`,
    { $clockName: clockName }
  );

export const queryScreenState = (): Promise<Array<ClockStruct>> =>
  query(
    'queryScreenState',
    `select m.type, m.ts-r.start_ts as startNS, value, filter_id  as filterId from measure m,trace_range r where filter_id in (select id from process_measure_filter where name = 'ScreenState')  order by startNS;
`
  );

export const queryIrqList = (): Promise<Array<{ name: string; cpu: number }>> =>
  query('queryIrqList', `select cat as name,callid as cpu from irq where cat!= 'ipi' group by cat,callid`);

export const queryIrqData = (callid: number, cat: string): Promise<Array<IrqStruct>> => {
  let sqlSoftIrq = `
    select i.ts - t.start_ts as startNS,i.dur,i.name,i.depth,argsetid as argSetId,i.id from irq i,
trace_range t where i.callid = ${callid} and i.cat = 'softirq'
    `;
  let sqlIrq = `
    select i.ts - t.start_ts as startNS,i.dur,
        case when i.cat = 'ipi' then 'IPI' || i.name else i.name end as name,
        i.depth,
        argsetid as argSetId,
        i.id 
        from irq i,trace_range t 
        where i.callid = ${callid} and ((i.cat = 'irq' and i.flag ='1') or i.cat = 'ipi') 
    `;
  return query('queryIrqData', cat === 'irq' ? sqlIrq : sqlSoftIrq, {});
};

export const queryAllJankProcess = (): Promise<
  Array<{
    pid: number;
  }>
> =>
  query(
    'queryAllJankProcess',
    `
        SELECT DISTINCT p.pid
        FROM frame_slice AS a
        LEFT JOIN process AS p ON a.ipid = p.ipid
        `
  );

export const queryAllExpectedData = (): Promise<Array<any>> =>
  query(
    'queryAllExpectedData',
    `
        SELECT
            a.id,
            (a.ts - TR.start_ts) AS ts,
            a.vsync as name,
            a.type,
            a.dur,
            p.pid,
            p.name as cmdline
        FROM frame_slice AS a, trace_range AS TR
             LEFT JOIN process AS p ON a.ipid = p.ipid
        WHERE a.type = 1
          and (a.flag <> 2 or a.flag is null)
        ORDER BY a.ipid,ts;`
  );

export const queryAllActualData = (): Promise<Array<any>> =>
  query(
    'queryAllActualData',
    `
        SELECT 
               a.id,
               (a.ts - TR.start_ts) AS ts,
               a.vsync AS name,
               a.type,
               a.dur,
               a.src AS src_slice,
               a.flag AS jank_tag,
               a.dst AS dst_slice,
               p.pid,
               p.name AS cmdline,
               (case when p.name like '%render_service' then 'render_service' else 'app' end) as frame_type
        FROM frame_slice AS a, trace_range AS TR
                 LEFT JOIN process AS p ON a.ipid = p.ipid
        WHERE a.type = 0
          AND a.flag <> 2
        ORDER BY a.ipid, ts;`
  );

export const queryActualFrameDate = (): Promise<Array<any>> =>
  query(
    'queryActualFrameDate',
    `SELECT
         sf.id,
         'frameTime' as frame_type,
         fs.ipid,
         fs.vsync as name,
         fs.dur as app_dur,
         (sf.ts + sf.dur - fs.ts) as dur,
         (fs.ts - TR.start_ts) AS ts,
         fs.type,
         (case when (sf.flag == 1 or fs.flag == 1 ) then 1  when (sf.flag == 3 or fs.flag == 3 ) then 3 else 0 end) as jank_tag,
         pro.pid,
         pro.name as cmdline,
         (sf.ts - TR.start_ts) AS rs_ts,
         sf.vsync AS rs_vsync,
         sf.dur AS rs_dur,
         sf.ipid AS rs_ipid,
         proc.pid AS rs_pid,
         proc.name AS rs_name
     FROM frame_slice AS fs
              LEFT JOIN process AS pro ON pro.id = fs.ipid
              LEFT JOIN frame_slice AS sf ON fs.dst = sf.id
              LEFT JOIN process AS proc ON proc.id = sf.ipid
              LEFT JOIN trace_range TR
     WHERE fs.dst IS NOT NULL
       AND fs.type = 0
       AND fs.flag <> 2
     UNION
     SELECT
         -1 as id,
         'frameTime' as frame_type,
         fs.ipid,
         fs.vsync  as name,
         fs.dur as app_dur,
         fs.dur,
         (fs.ts - TR.start_ts) AS ts,
         fs.type,
         fs.flag as jank_tag,
         pro.pid,
         pro.name as cmdline,
         NULL AS rs_ts,
         NULL AS rs_vsync,
         NULL AS rs_dur,
         NULL AS rs_ipid,
         NULL AS rs_pid,
         NULL AS rs_name
     FROM frame_slice AS fs
              LEFT JOIN process AS pro ON pro.id = fs.ipid
              LEFT JOIN trace_range TR
     WHERE fs.dst IS NULL
       AND pro.name NOT LIKE '%render_service%'
       AND fs.type = 0
       AND fs.flag <> 2
     ORDER BY ts;`
  );

export const queryExpectedFrameDate = (): Promise<Array<any>> =>
  query(
    'queryExpectedFrameDate',
    `
    SELECT
        sf.id,
        'frameTime' as frame_type,
        fs.ipid,
        fs.vsync as name,
        fs.dur as app_dur,
        (sf.ts + sf.dur - fs.ts) as dur,
        (fs.ts - TR.start_ts) AS ts,
        fs.type,
        fs.flag,
        pro.pid,
        pro.name as cmdline,
        (sf.ts - TR.start_ts) AS rs_ts,
        sf.vsync AS rs_vsync,
        sf.dur AS rs_dur,
        sf.ipid AS rs_ipid,
        proc.pid AS rs_pid,
        proc.name AS rs_name
    FROM frame_slice AS fs
    LEFT JOIN process AS pro ON pro.id = fs.ipid
    LEFT JOIN frame_slice AS sf ON fs.dst = sf.id
    LEFT JOIN process AS proc ON proc.id = sf.ipid
    LEFT JOIN trace_range TR
    WHERE fs.dst IS NOT NULL
        AND fs.type = 1
    UNION
    SELECT
        -1 as id,
        'frameTime' as frame_type,
        fs.ipid,
        fs.vsync  as name,
        fs.dur as app_dur,
        fs.dur,
        (fs.ts - TR.start_ts) AS ts,
        fs.type,
        fs.flag,
        pro.pid,
        pro.name as cmdline,
        NULL AS rs_ts,
        NULL AS rs_vsync,
        NULL AS rs_dur,
        NULL AS rs_ipid,
        NULL AS rs_pid,
        NULL AS rs_name
    FROM frame_slice AS fs
    LEFT JOIN process AS pro ON pro.id = fs.ipid
    LEFT JOIN trace_range TR
    WHERE fs.dst IS NULL
    AND pro.name NOT LIKE '%render_service%'
    AND fs.type = 1
    ORDER BY ts;`
  );

export const queryFlowsData = (src_slice: Array<string>): Promise<Array<any>> =>
  query(
    'queryFlowsData',
    `
    SELECT fs.vsync AS name,
        p.pid,
        p.name  AS cmdline,
        fs.type
    FROM frame_slice AS fs
    LEFT JOIN process AS p ON fs.ipid = p.ipid
    WHERE fs.type = 0
        AND fs.id IN (${src_slice.join(',')});`
  );

export const queryPrecedingData = (dst_slice: string): Promise<Array<any>> =>
  query(
    'queryFlowsData',
    `
        SELECT a.vsync AS name,
               p.pid,
               p.name  AS cmdline,
               a.type
        FROM frame_slice AS a
                 LEFT JOIN process AS p ON a.ipid = p.ipid
        WHERE a.type = 0
          AND a.id = $dst_slice;`,
    { $dst_slice: dst_slice }
  );

export const queryFrameTimeData = (): Promise<Array<any>> =>
  query(
    'queryFrameTimeData',
    `
        SELECT DISTINCT p.pid
        FROM frame_slice AS a
            LEFT JOIN process AS p
            ON a.ipid = p.ipid;`
  );

export const queryGpuDur = (id: number): Promise<any> =>
  query(
    'queryGpuDur',
    `
        SELECT dur AS gpu_dur
        FROM gpu_slice
        WHERE frame_row = $id;`,
    { $id: id }
  );

export const queryHeapFile = (): Promise<Array<FileInfo>> =>
  query(
    'queryHeapFile',
    `SELECT f.id,
        f.file_name AS name,
        f.start_time - t.start_ts AS startTs,
        f.end_time - t.start_ts AS endTs,
        f.self_size AS size,
        c.pid
      FROM
        js_heap_files f,
        trace_range t,
        js_config c
      WHERE
        ( t.end_ts >= f.end_time AND f.file_name != 'Timeline' )
        OR f.file_name = 'Timeline'`
  );

export const queryHeapInfo = (fileId: number): Promise<Array<any>> =>
  query(
    'queryHeapInfo',
    `SELECT file_id as fileId, key, type, int_value as intValue, str_value as strValue
      FROM js_heap_info WHERE file_id = ${fileId}`
  );

export const queryHeapNode = (fileId: number): Promise<Array<HeapNode>> =>
  query(
    'queryHeapNode',
    `SELECT node_index as nodeIndex,type,name as nameIdx,id,self_size as selfSize,edge_count as edgeCount,trace_node_id as traceNodeId,detachedness 
      FROM js_heap_nodes WHERE file_id = ${fileId}`
  );

export const queryHeapEdge = (fileId: number): Promise<Array<HeapEdge>> =>
  query(
    'queryHeapEdge',
    `SELECT edge_index as edgeIndex,type,name_or_index as nameOrIndex,to_node as nodeId,from_node_id as fromNodeId,to_node_id as toNodeId
      FROM js_heap_edges WHERE file_id = ${fileId}`
  );

export const queryHeapFunction = (fileId: number): Promise<Array<HeapTraceFunctionInfo>> =>
  query(
    'queryHeapFunction',
    `SELECT function_index as index ,function_id as id ,name,script_name as scriptName,script_id as scriptId,line,column
      FROM js_heap_trace_function_info WHERE file_id = ${fileId}`
  );

export const queryHeapTraceNode = (fileId: number): Promise<Array<any>> =>
  query(
    'queryHeapTraceNode',
    `SELECT F.name,
        F.script_name as scriptName,
        F.script_id as scriptId,
        F.column,
        F.line,
        N.id,
        N.function_info_index as functionInfoIndex,
        N.parent_id as parentId,
        N.count,
        N.size,
        IFNULL( S.live_count, 0 ) AS liveCount,
        IFNULL( S.live_size, 0 ) AS liveSize
    FROM
        js_heap_trace_node N
        LEFT JOIN (
            SELECT
                trace_node_id as traceNodeId,
                SUM( self_size ) AS liveSize,
                count( * ) AS liveCount
            FROM
                js_heap_nodes
            WHERE
                file_id = ${fileId}
                AND trace_node_id != 0
            GROUP BY
                trace_node_id
        ) S ON N.id = S.trace_node_id
    LEFT JOIN js_heap_trace_function_info F ON (F.file_id = N.file_id
                AND F.function_index = N.function_info_index)
    WHERE
        N.file_id = ${fileId}
    ORDER BY
        N.id`
  );

export const queryHeapSample = (fileId: number): Promise<Array<HeapSample>> =>
  query(
    'queryHeapSample',
    `SELECT timestamp_us as timestamp , last_assigned_id as lastAssignedId, 0 as size
      FROM js_heap_sample WHERE file_id = ${fileId}`
  );

export const queryHeapLocation = (fileId: number): Promise<Array<HeapLocation>> =>
  query(
    'queryHeapLocation',
    `SELECT object_index as objectIndex,script_id as scriptId ,column
      FROM js_heap_location WHERE file_id = ${fileId}`
  );

export const queryHeapString = (fileId: number): Promise<Array<any>> =>
  query(
    'queryHeapString',
    `SELECT string
      FROM js_heap_string WHERE file_id = ${fileId}`
  );
export const queryTraceRange = (): Promise<Array<any>> =>
  query('queryTraceRange', `SELECT t.start_ts as startTs, t.end_ts as endTs FROM trace_range t`);

export const queryHiPerfProcessCount = (
  leftNs: number,
  rightNs: number,
  cpus: Array<number>,
  threads: Array<number>,
  processes: Array<number>
): Promise<Array<any>> => {
  let str = '';
  if (processes.length > 0) {
    str = ` and C.process_id in (${processes.join(',')})`;
  }
  if (threads.length > 0) {
    str = ` and A.thread_id in (${threads.join(',')}) `;
  }
  if (processes.length > 0 && threads.length > 0) {
    str = ` and (C.process_id in (${processes.join(',')}) or A.thread_id in (${threads.join(',')}))`;
  }
  if (cpus.length > 0) {
    str = ` and A.cpu_id in (${cpus.join(',')})`;
  }
  if (cpus.length > 0 && processes.length > 0) {
    str = ` and (C.process_id in (${processes.join(',')}) or A.cpu_id in (${cpus.join(',')}))`;
  }
  return query(
    'queryHiPerfProcessCount',
    `
    select     C.process_id as pid,
               (A.timestamp_trace - R.start_ts) as time,
               C.thread_name as threadName,
               A.thread_id as tid,
               A.id,
               A.callchain_id
        from perf_sample A,trace_range R
             left join perf_thread C on A.thread_id = C.thread_id  and  A.thread_id != 0
        where time >= $leftNs and time <= $rightNs and A.callchain_id > 0
        ${str} 
    `,
    { $leftNs: leftNs, $rightNs: rightNs }
  );
};

export const queryConcurrencyTask = (itid: number, selectStartTime: number, selectEndTime: number) =>
  query<TaskTabStruct>(
    'queryConcurrencyTask',
    `SELECT thread.tid,
            thread.ipid,
            callstack.name                AS funName,
            callstack.ts                  AS startTs,
            (case when callstack.dur = -1 then (SELECT end_ts FROM trace_range) else callstack.dur end) as dur,
            callstack.id,
            task_pool.priority,
            task_pool.allocation_task_row AS allocationTaskRow,
            task_pool.execute_task_row    AS executeTaskRow,
            task_pool.return_task_row     AS returnTaskRow,
            task_pool.execute_id          AS executeId
     FROM thread
            LEFT JOIN callstack ON thread.id = callstack.callid
            LEFT JOIN task_pool ON callstack.id = task_pool.execute_task_row
     WHERE ipid = (SELECT thread.ipid
                   FROM thread
                   WHERE thread.itid = $itid)
       AND thread.name = 'TaskWorkThread'
       AND callstack.name LIKE 'H:Task Perform:%'
       AND -- 左包含
           (($selectStartTime <= callstack.ts AND $selectEndTime > callstack.ts)
        OR -- 右包含
       ($selectStartTime < callstack.ts + callstack.dur AND $selectEndTime >= callstack.ts + callstack.dur)
        OR -- 包含
       ($selectStartTime >= callstack.ts AND $selectEndTime <= callstack.ts +
        (case when callstack.dur = -1 then (SELECT end_ts FROM trace_range) else callstack.dur end))
        OR -- 被包含
       ($selectStartTime <= callstack.ts AND $selectEndTime >= callstack.ts + callstack.dur))
     ORDER BY callstack.ts;`,
    { $itid: itid, $selectStartTime: selectStartTime, $selectEndTime: selectEndTime }
  );

export const queryBySelectExecute = (
  executeId: string,
  itid: number
): Promise<
  Array<{
    tid: number;
    allocation_task_row: number;
    execute_task_row: number;
    return_task_row: number;
    priority: number;
  }>
> => {
  let sqlStr = `SELECT thread.tid,
                       task_pool.allocation_task_row,
                       task_pool.execute_task_row,
                       task_pool.return_task_row,
                       task_pool.priority
                FROM task_pool
                       LEFT JOIN callstack ON callstack.id = task_pool.allocation_task_row
                       LEFT JOIN thread ON thread.id = callstack.callid
                WHERE task_pool.execute_id = $executeId AND task_pool.execute_itid = $itid;
    `;
  return query('queryBySelectExecute', sqlStr, { $executeId: executeId, $itid: itid });
};

export const queryBySelectAllocationOrReturn = (
  executeId: string,
  itid: number
): Promise<
  Array<{
    tid: number;
    allocation_task_row: number;
    execute_task_row: number;
    return_task_row: number;
    priority: number;
  }>
> => {
  let sqlStr = `SELECT thread.tid,
                       task_pool.allocation_task_row,
                       task_pool.execute_task_row,
                       task_pool.return_task_row,
                       task_pool.priority
                FROM task_pool
                       LEFT JOIN callstack ON callstack.id = task_pool.execute_task_row
                       LEFT JOIN thread ON thread.id = callstack.callid
                WHERE task_pool.execute_task_row IS NOT NULL AND task_pool.execute_id = $executeId
                AND task_pool.allocation_itid = $itid;
    `;
  return query('queryBySelectAllocationOrReturn', sqlStr, { $executeId: executeId, $itid: itid });
};

export const queryTaskListByExecuteTaskIds = (
  executeTaskIds: Array<number>,
  ipid: number
): Promise<Array<TaskTabStruct>> => {
  let sqlStr = `
    SELECT thread.ipid,
           task_pool.allocation_task_row AS allocationTaskRow,
           task_pool.execute_task_row    AS executeTaskRow,
           task_pool.return_task_row     AS returnTaskRow,
           task_pool.execute_id          AS executeId,
           task_pool.priority
    FROM task_pool
           LEFT JOIN callstack ON callstack.id = task_pool.allocation_task_row
           LEFT JOIN thread ON thread.id = callstack.callid
    WHERE task_pool.execute_id IN (${executeTaskIds.join(',')})
      AND thread.ipid = $ipid
      AND task_pool.execute_task_row IS NOT NULL;
    `;
  return query('queryTaskListByExecuteTaskIds', sqlStr, { $executeTaskIds: executeTaskIds, $ipid: ipid });
};

export const queryTaskPoolCallStack = (): Promise<Array<{ id: number; ts: number; dur: number; name: string }>> => {
  let sqlStr = `select * from callstack where name like 'H:Task%';`;
  return query('queryTaskPoolCallStack', sqlStr, {});
};

export const queryTaskPoolTotalNum = (itid: number) =>
  query<number>(
    'queryTaskPoolTotalNum',
    `SELECT thread.tid
         FROM thread
                LEFT JOIN callstack ON thread.id = callstack.callid
         WHERE ipid = (SELECT thread.ipid
                       FROM thread
                       WHERE thread.itid = $itid)
           AND thread.name = 'TaskWorkThread'
         GROUP BY thread.tid;`,
    { $itid: itid }
  );

export const queryFrameAnimationData = (): Promise<Array<FrameAnimationStruct>> =>
  query(
    'queryFrameAnimationData',
    `SELECT
           a.id AS animationId,
           (CASE
               WHEN a.input_time not null THEN (a.input_time - R.start_ts)
               ELSE (a.start_point- R.start_ts)
               END) AS ts,
           (a.start_point - R.start_ts) AS dynamicStartTs,
           (a.end_point - R.start_ts) AS dynamicEndTs
        FROM 
            animation AS a,
            trace_range AS R
        ORDER BY 
            ts;`
  );

export const queryFrameDynamicData = (): Promise<Array<FrameDynamicStruct>> =>
  query(
    'queryFrameDynamicData',
    `SELECT
           d.id,
           d.x,
           d.y,
           d.width,
           d.height,
           d.alpha,
           d.name AS appName,
           (d.end_time - R.start_ts) AS ts
        FROM 
            dynamic_frame AS d,
            trace_range AS R
        ORDER BY 
            d.end_time;`
  );

export const queryFrameApp = (): Promise<
  Array<{
    name: string;
  }>
> =>
  query(
    'queryFrameApp',
    `SELECT 
            DISTINCT d.name
         FROM 
             dynamic_frame AS d, 
             trace_range AS R
         WHERE 
            d.end_time >= R.start_ts
            AND
            d.end_time <= R.end_ts;`
  );

export const queryAnimationFrameFps = (
  startTime: number,
  endTime: number
): Promise<
  Array<{
    fps: number;
  }>
> =>
  query(
    'queryAnimationFrameFps',
    `SELECT
            count(*) as fps
        FROM
            dynamic_frame AS d,
            trace_range AS R
        WHERE 
            d.end_time >= (${startTime} + R.start_ts)
        AND
            d.end_time <= (${endTime} + R.start_ts)`
  );

export const queryFrameSpacing = (): Promise<Array<FrameSpacingStruct>> =>
  query(
    'queryFrameSpacing',
    `SELECT
         d.id,
         d.width AS currentFrameWidth,
         d.height AS currentFrameHeight,
         d.name AS nameId,
         (d.end_time - R.start_ts) AS currentTs,
         d.x,
         d.y
     FROM
         dynamic_frame AS d,
         trace_range AS R
     ORDER BY
         d.end_time;`
  );

export const queryPhysicalData = (): Promise<Array<DeviceStruct>> =>
  query(
    'queryPhysicalData',
    `SELECT physical_width AS physicalWidth,
            physical_height AS physicalHeight,
            physical_frame_rate AS physicalFrameRate
     FROM device_info;`
  );

export const queryJsCpuProfilerConfig = (): Promise<Array<any>> =>
  query('queryJsCpuProfilerConfig', `SELECT pid, type, enable_cpu_Profiler as enableCpuProfiler FROM js_config`);
export const queryJsCpuProfilerData = (): Promise<Array<any>> =>
  query('queryJsCpuProfilerData', `SELECT 1 WHERE EXISTS(select 1 from js_cpu_profiler_node)`);

export const queryJsMemoryData = (): Promise<Array<any>> =>
  query('queryJsMemoryData', `SELECT 1 WHERE EXISTS(SELECT 1 FROM js_heap_nodes)`);

export const queryAllTaskPoolPid = (): Promise<Array<{ pid: number }>> =>
  query(
    'queryAllTaskPoolPid',
    `SELECT DISTINCT pid from task_pool LEFT JOIN callstack ON callstack.id = task_pool.execute_task_row
    LEFT JOIN thread ON thread.id = callstack.callid LEFT JOIN process ON
        process.id = thread.ipid WHERE task_pool.execute_task_row IS NOT NULL`
  );
export const queryVmTrackerShmData = (iPid: number): Promise<Array<any>> =>
  query(
    'queryVmTrackerShmData',
    `SELECT (A.ts - B.start_ts) as startNs,
      sum(A.size) as value 
    FROM
      memory_ashmem A,trace_range B 
    where
      A.ipid = ${iPid}
      AND A.ts < B.end_ts
    and
      flag = 0
    GROUP by A.ts`,
    {}
  );

export const queryVmTrackerShmSizeData = (
  leftNs: number,
  rightNs: number,
  iPid: number,
  dur: number
): Promise<Array<any>> =>
  query(
    'queryVmTrackerShmSizeData',
    `SELECT ( A.ts - B.start_ts ) AS startNS,
        A.flag,
        avg( A.size ) AS avg,
        max( A.size ) AS max,
        min( A.size ) AS min,
        sum( A.size ) AS sum 
      FROM
        memory_ashmem A,
        trace_range B 
      WHERE 
        startNS <= ${rightNs}  and (startNS+ ${dur}) >=${leftNs}
        AND ipid = ${iPid}`,
    {}
  );

export const queryVmTrackerShmSelectionData = (startNs: number, ipid: number): Promise<Array<any>> =>
  query(
    'queryVmTrackerShmSelectionData',
    `SELECT (A.ts - B.start_ts) as startNS,A.ipid,
             A.fd,A.size,A.adj,A.ashmem_name_id as name,
             A.ashmem_id as id,A.time,A.purged,A.ref_count as count,
             A.flag
             FROM memory_ashmem A,trace_range B 
             where startNS = ${startNs} and ipid = ${ipid};`,
    {}
  );
export const getTabSmapsRecordData = (rightNs: number): Promise<Array<Smaps>> =>
  query<Smaps>(
    'getTabSmapsRecordData',
    `
      SELECT
     (A.timestamp - t.start_ts) AS startNs,
     start_addr as startAddr,
     end_addr as endAddr,
     A.type,
     resident_size * 1024 AS rss,
     protection_id as pid,
     pss * 1024 as pss,virtaul_size * 1024 AS size,reside,A.path_id AS path,
     shared_clean * 1024 as sharedClean,shared_dirty * 1024 as sharedDirty,private_clean * 1024 as privateClean,
     private_dirty * 1024 as privateDirty,swap * 1024 as swap,swap_pss * 1024 as swapPss
     FROM smaps A,
     trace_range AS t
     WHERE (startNs) = $rightNs`,
    { $rightNs: rightNs },
    'exec'
  );

export const getTabSmapsStatisticMaxSize = (rightNs: number): Promise<Array<any>> =>
  query<Smaps>(
    'getTabSmapsStatisticMaxRss',
    `
SELECT (A.timestamp - B.start_ts) as startNS, sum(virtaul_size) * 1024 as max_value FROM smaps A,trace_range B where startNS = $rightNs`,
    { $rightNs: rightNs }
  );
export const queryMemoryConfig = (): Promise<Array<MemoryConfig>> =>
  query(
    'queryMemoryConfiig',
    `SELECT ipid as iPid, process.pid AS pid,
      process.name AS processName,
      (SELECT value FROM trace_config WHERE trace_source = 'memory_config' AND key = 'sample_interval') AS interval
    FROM
      trace_config
      LEFT JOIN process ON value = ipid
    WHERE
      trace_source = 'memory_config'
      AND key = 'ipid'
      ;`
  );

//   Ability Monitor Dma泳道图
export const queryDmaAbilityData = (): Promise<Array<SnapshotStruct>> =>
  query(
    'queryDmaAbilityData',
    `SELECT 
      (A.ts - B.start_ts) as startNs,
      sum(A.size) as value,
      E.data as expTaskComm,
      A.flag as flag
    FROM memory_dma A,trace_range B
    left join data_dict as E on E.id=A.exp_task_comm_id
    WHERE
      A.flag = 0
      AND A.ts < B.end_ts
    GROUP by A.ts;`
  );

//   Ability Monitor SkiaGpuMemory泳道图
export const queryGpuMemoryAbilityData = (): Promise<Array<SnapshotStruct>> =>
  query(
    'queryGpuMemoryAbilityData',
    `SELECT 
    (A.ts - B.start_ts) as startNs,
    sum(A.used_gpu_size) as value
    FROM memory_process_gpu A,trace_range B
    WHERE A.ts < B.end_ts
    GROUP by A.ts;`
  );

//   VM Tracker Dma泳道图
export const queryDmaSampsData = (process: number): Promise<Array<SnapshotStruct>> =>
  query(
    'queryDmaSampsData',
    `SELECT 
      (A.ts - B.start_ts) as startNs,
      sum(A.size) as value,
      A.flag as flag,
      A.ipid as ipid,
      E.data as expTaskComm
      FROM memory_dma A,trace_range B 
      left join data_dict as E on E.id=A.exp_task_comm_id
    WHERE
      A.flag = 0
      AND  $pid = A.ipid
      AND A.ts < B.end_ts
    GROUP by A.ts;`,
    { $pid: process }
  );

//  VM Tracker Gpu Memory泳道图
export const queryGpuMemoryData = (processId: number): Promise<Array<SnapshotStruct>> =>
  query(
    'queryGpuMemorySampsData',
    `SELECT
    (A.ts - B.start_ts) as startNs,
    sum(A.used_gpu_size) as value,
    A.ipid as ipid
    FROM memory_process_gpu A,trace_range B
    WHERE
    $pid = A.ipid
    AND A.ts < B.end_ts
    GROUP by A.ts;`,
    { $pid: processId }
  );

// Ability Monitor Purgeable泳道图
export const queryPurgeableSysData = (isPin?: boolean): Promise<Array<any>> => {
  const pinCondition = isPin ? ' AND a.ref_count > 0' : '';
  const names = isPin ? " ('sys.mem.pined.purg')" : "('sys.mem.active.purg','sys.mem.inactive.purg')";
  return query(
    'queryPurgeableSysData',
    `SELECT
      startNs,
      sum( value ) AS value 
  FROM
      (
      SELECT
          m.ts - tr.start_ts AS startNs,
          sum( m.value ) AS value 
      FROM
          sys_mem_measure m,
          trace_range tr
          LEFT JOIN sys_event_filter f ON f.id = m.filter_id 
      WHERE
          m.ts < tr.end_ts 
          AND f.name IN ${names}
      GROUP BY
          m.ts UNION ALL
      SELECT
          a.ts - tr.start_ts AS startNs,
          sum( a.size ) AS value 
      FROM
          memory_ashmem a,
          trace_range tr 
      WHERE
          a.ts < tr.end_ts 
          AND a.flag = 0 
          ${pinCondition}
          GROUP BY
              a.ts 
          ) 
      GROUP BY startNs`
  );
};

// VM Tracker Purgeable泳道图
export const queryPurgeableProcessData = (ipid: number, isPin?: boolean): Promise<Array<any>> => {
  const pinSql = isPin ? ' AND a.ref_count > 0' : '';
  const names = isPin ? " ('mem.purg_pin')" : "('mem.purg_sum')";
  return query(
    'queryPurgeableProcessData',
    `SELECT startNs, sum( value ) AS value 
    FROM
        (SELECT
            m.ts - tr.start_ts AS startNs,
            sum(m.value) AS value
        FROM
            process_measure m,
            trace_range tr
            LEFT JOIN process_measure_filter f ON f.id = m.filter_id
        WHERE
            m.ts < tr.end_ts
            AND f.name = ${names}
            AND f.ipid = ${ipid}
        GROUP BY m.ts
        UNION ALL
        SELECT
            a.ts - tr.start_ts AS startNs,
            sum( a.pss ) AS value 
        FROM
            memory_ashmem a,
            trace_range tr 
        WHERE
            a.ts < tr.end_ts
            AND a.flag = 0
            AND a.ipid = ${ipid}
            ${pinSql}
            GROUP BY a.ts) 
        GROUP BY startNs`
  );
};

//Ability Monitor Purgeable 框选 tab页
export const querySysPurgeableTab = (
  leftNs: number,
  rightNs: number,
  dur: number,
  isPin?: boolean
): Promise<Array<any>> => {
  let pinsql = isPin ? ' AND ref_count > 0' : '';
  const names = isPin ? " ('sys.mem.pined.purg')" : "('sys.mem.active.purg','sys.mem.inactive.purg')";
  return query(
    'querySysPurgeableTab',
    `SELECT name, MAX( size ) AS maxSize,MIN( size ) AS minSize,AVG( size ) AS avgSize
    FROM
        (SELECT
          'ShmPurg' AS name,
          ts - tr.start_ts AS startTs,
          SUM( size ) AS size
        FROM
          memory_ashmem,
          trace_range tr
        WHERE flag = 0
        ${pinsql}
        GROUP BY ts UNION
        SELECT
        CASE
          WHEN
            f.name = 'sys.mem.active.purg' THEN
              'ActivePurg'
              WHEN f.name = 'sys.mem.inactive.purg' THEN
              'InActivePurg' ELSE 'PinedPurg'
            END AS name,
            m.ts - tr.start_ts AS startTs,
            m.value AS size
          FROM
            sys_mem_measure m,
            trace_range tr
            LEFT JOIN sys_event_filter f ON f.id = m.filter_id
          WHERE
            f.name IN ${names}
          ),
          trace_range tr
        WHERE ${leftNs} <= startTs + ${dur} AND ${rightNs} >= startTs
        GROUP BY name`
  );
};

//Ability Monitor Purgeable 点选 tab页
export const querySysPurgeableSelectionTab = (startNs: number, isPin?: boolean): Promise<Array<any>> => {
  const pinSql = isPin ? ' AND ref_count > 0' : '';
  const names = isPin ? " ('sys.mem.pined.purg')" : "('sys.mem.active.purg','sys.mem.inactive.purg')";
  return query(
    'querySysPurgeableSelectionTab',
    `SELECT
    ( CASE WHEN f.name = 'sys.mem.active.purg' THEN 'ActivePurg' WHEN f.name = 'sys.mem.inactive.purg' THEN 'InActivePurg' ELSE 'PinedPurg' END ) AS name,
    m.value AS value
    FROM
    sys_mem_measure m,
    trace_range tr
    LEFT JOIN sys_event_filter f ON f.id = m.filter_id
    WHERE
    f.name IN ${names}
    AND m.ts - tr.start_ts = ${startNs} 
    UNION
    SELECT
    'ShmPurg' AS name,
    SUM( size ) AS value
    FROM
    memory_ashmem,
    trace_range tr
    WHERE
    memory_ashmem.ts - tr.start_ts = ${startNs}
    AND flag=0
    ${pinSql}
    GROUP BY ts`
  );
};

///////////////////////////////////////////////
//VM  Purgeable 框选 tab页
export const queryProcessPurgeableTab = (
  leftNs: number,
  rightNs: number,
  dur: number,
  ipid: number,
  isPin?: boolean
): Promise<Array<any>> => {
  const pinSql = isPin ? ' AND ref_count > 0' : '';
  let filterSql = isPin ? "'mem.purg_pin'" : "'mem.purg_sum'";
  return query(
    'queryProcessPurgeableTab',
    `SELECT name, MAX(size) AS maxSize, MIN(size) AS minSize, AVG(size) AS avgSize
    FROM
      (SELECT
        'ShmPurg' AS name, ts - tr.start_ts AS startTs, SUM( pss ) AS size
      FROM
        memory_ashmem,
        trace_range tr
      WHERE
        ipid = ${ipid}
        AND flag = 0
        ${pinSql}
      GROUP BY ts
      UNION
      SELECT
      CASE
          WHEN f.name = 'mem.purg_pin' THEN
          'PinedPurg' ELSE 'TotalPurg'
        END AS name,
        m.ts - tr.start_ts AS startTs,
        sum( m.value ) AS size
      FROM
        process_measure m,
        trace_range tr
        LEFT JOIN process_measure_filter f ON f.id = m.filter_id 
      WHERE f.name = ${filterSql}
        AND f.ipid = ${ipid}
      GROUP BY m.ts
    ) combined_data, trace_range tr
    WHERE ${leftNs} <= startTs + ${dur} AND ${rightNs} >= startTs
    GROUP BY name`
  );
};

//VM  Purgeable 点选 tab页
export const queryProcessPurgeableSelectionTab = (
  startNs: number,
  ipid: number,
  isPin?: boolean
): Promise<Array<any>> => {
  const condition = isPin ? "'mem.purg_pin'" : "'mem.purg_sum'";
  const pinSql = isPin ? ' AND ref_count > 0' : '';
  return query(
    'queryProcessPurgeableSelectionTab',
    `SELECT
        ( CASE WHEN f.name = 'mem.purg_pin' THEN 'PinedPurg' ELSE 'TotalPurg' END ) AS name,
        SUM( m.value )  AS value 
    FROM
        process_measure m,
        trace_range tr
        left join process_measure_filter f on f.id = m.filter_id 
    WHERE
        f.name = ${condition} 
        AND m.ts - tr.start_ts = ${startNs}
    AND f.ipid = ${ipid}
    GROUP BY m.ts
    UNION
    SELECT
        'ShmPurg' AS name,
        SUM( pss ) AS size
    FROM
        memory_ashmem,
        trace_range tr
    WHERE
        ipid = ${ipid}
        AND ts - tr.start_ts = ${startNs}
        AND flag = 0
        ${pinSql}
    GROUP BY ts`
  );
};

export const getTabSmapsStatisticData = (rightNs: number): Promise<Array<Smaps>> =>
  query<Smaps>(
    'getTabSmapsStatisticData',
    `SELECT
     (A.timestamp - t.start_ts) AS startNs,
     start_addr as startAddr,
     end_addr as endAddr,
     A.type,
     sum(resident_size) * 1024 AS rss,
     protection_id as pid,
     count(A.path_id) as count,
     sum(pss) * 1024 as pss ,sum(virtaul_size) * 1024 AS size,sum(reside) as reside,A.path_id AS path,
     sum(shared_clean) * 1024 as sharedClean,sum(shared_dirty) * 1024 as sharedDirty,sum(private_clean) * 1024 as privateClean,sum(private_dirty) * 1024 as privateDirty,
     sum(swap) * 1024 as swap,sum(swap_pss) * 1024 as swapPss
     FROM smaps A,
     trace_range AS t
     WHERE (startNs) =$rightNs
     group by type,path`,
    { $rightNs: rightNs },
    'exec'
  );

export const getTabSmapsStatisticSelectData = (leftNs: number, rightNs: number, dur: number): Promise<Array<Smaps>> =>
  query<Smaps>(
    'getTabSmapsStatisticData',
    `SELECT
     (A.timestamp - t.start_ts) AS startNs,
     start_addr as startAddr,
     end_addr as endAddr,
     A.type,
     sum(resident_size) * 1024 AS rss,
     protection_id as pid,
     count(A.path_id) as count,
     sum(pss) * 1024 as pss ,sum(virtaul_size) * 1024 AS size,sum(reside) as reside,A.path_id AS path,
     sum(shared_clean) * 1024 as sharedClean,sum(shared_dirty) * 1024 as sharedDirty,sum(private_clean) * 1024 as privateClean,sum(private_dirty) * 1024 as privateDirty,
     sum(swap) * 1024 as swap,sum(swap_pss) * 1024 as swapPss
     FROM smaps A,
     trace_range AS t
     WHERE (startNs) <=$rightNs and (startNs+$dur)>=$leftNs
     group by type,path`,
    { $rightNs: rightNs, $leftNs: leftNs, $dur: dur },
    'exec'
  );

//Ability Monitor Dma 框选
export const getTabDmaAbilityData = (leftNs: number, rightNs: number, dur: number): Promise<Array<Dma>> =>
  query<Dma>(
    'getTabDmaAbilityData',
    `SELECT (S.ts-TR.start_ts) as startNs,
        MAX(S.size) as maxSize,
        MIN(S.size) as minSize,
        Avg(S.size) as avgSize,
        E.pid as processId,
        E.name as processName
    from trace_range as TR,memory_dma as S
    left join process as E on E.ipid=S.ipid
    WHERE
      $leftNS <= startNs + ${dur} and $rightNS >= startNs
      and flag = 0
    GROUP by E.pid
              `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

//Ability Monitor SkiaGpuMemory 框选
export const getTabGpuMemoryAbilityData = (leftNs: number, rightNs: number, dur: number): Promise<Array<GpuMemory>> =>
  query<GpuMemory>(
    'getTabGpuMemoryAbilityData',
    `SELECT (S.ts-TR.start_ts) as startNs,
    gpu_name_id as gpuNameId,
    MAX(S.used_gpu_size) as maxSize,
    MIN(S.used_gpu_size) as minSize,
    Avg(S.used_gpu_size) as avgSize,
    E.pid as processId,
    E.name as processName
    from trace_range as TR,memory_process_gpu as S
    left join process as E on E.ipid=S.ipid
    WHERE
    $leftNS <= startNs + ${dur}
    and
    $rightNS >= startNs
    GROUP by 
    E.pid ,S.gpu_name_id
            `,
    { $leftNS: leftNs, $rightNS: rightNs }
  );

//VM Tracker Dma 框选
export const getTabDmaVmTrackerData = (
  leftNs: number,
  rightNs: number,
  processId: number,
  dur: number
): Promise<Array<Dma>> =>
  query<Dma>(
    'getTabDmaVmTrackerData',
    `SELECT (S.ts-TR.start_ts) as startNs,
      MAX(S.size) as maxSize,
      MIN(S.size) as minSize,
      Avg(S.size) as avgSize
    from trace_range as TR,memory_dma as S
    left join data_dict as C on C.id=S.exp_task_comm_id
    where
      $leftNS <= startNs + ${dur} and $rightNS >= startNs
      and flag = 0
    and
        $pid = S.ipid
              `,
    { $leftNS: leftNs, $rightNS: rightNs, $pid: processId }
  );
//VM Tracker SkiaGpuMemory 框选
export const getTabGpuMemoryData = (
  leftNs: number,
  rightNs: number,
  processId: number,
  dur: number
): Promise<Array<GpuMemory>> =>
  query<GpuMemory>(
    'getTabGpuMemoryData',
    `SELECT  
      (S.ts-TR.start_ts) as startNs,
      gpu_name_id as gpuNameId,
      T.tid as threadId,
      T.name as threadName,
      MAX(S.used_gpu_size) as maxSize,
      MIN(S.used_gpu_size) as minSize,
      Avg(S.used_gpu_size) as avgSize
      from trace_range as TR,memory_process_gpu as S
      left join thread as T on T.itid=S.itid
      where
       $leftNS <= startNs + ${dur}
      and
      $rightNS >= startNs
      and
        $pid = S.ipid
      group by gpu_name_id,threadId
              `,
    { $leftNS: leftNs, $rightNS: rightNs, $pid: processId }
  );

//Ability Monitor Dma 点选
export const getTabDmaAbilityClickData = (startNs: number): Promise<Array<Dma>> =>
  query<Dma>(
    'getTabDmaAbilityClickData',
    `SELECT
  (S.ts-TR.start_ts) as startNs,
    S.fd as fd,
    S.size as size,
    S.ino as ino,
    S.exp_pid as expPid,
    buf_name_id as bufName,
    exp_name_id as expName,
    exp_task_comm_id as expTaskComm,
    E.pid as processId,
    E.name as processName,
    S.flag as flag
    from trace_range as TR,memory_dma as S
    left join process as E on E.ipid=S.ipid
    WHERE
    startNs = ${startNs}
              `,
    { $startNs: startNs }
  );

//VM Tracker Dma 点选
export const getTabDmaVMTrackerClickData = (startNs: number, processId: number): Promise<Array<Dma>> =>
  query<Dma>(
    'getTabDmaVMTrackerClickData',
    `SELECT
    (S.ts-TR.start_ts) as startNs,
    S.fd as fd,
    S.size as size,
    S.ino as ino,
    S.exp_pid as expPid,
    buf_name_id as bufName,
    exp_name_id as expName,
    exp_task_comm_id as expTaskComm,
    S.flag as flag
    from trace_range as TR,memory_dma as S
    WHERE
    startNs = ${startNs}
    AND
    $pid = S.ipid
              `,
    { $startNs: startNs, $pid: processId }
  );

//Ability Monitor SkiaGpuMemory 点选
export const getTabGpuMemoryAbilityClickData = (startNs: number): Promise<Array<GpuMemory>> =>
  query<GpuMemory>(
    'getTabGpuMemoryAbilityClickData',
    `SELECT
    (S.ts-TR.start_ts) as startNs,
    S.used_gpu_size as size,
    E.pid as processId,
    E.name as processName,
    A.data as gpuName
    from trace_range as TR,memory_process_gpu as S
    left join process as E on E.ipid=S.ipid
    left join data_dict as A on A.id=S.gpu_name_id
    WHERE
    startNs = ${startNs}
              `,
    { $startNs: startNs }
  );

//VM Tracker SkiaGpuMemory 点选
export const getTabGpuMemoryVMTrackerClickData = (startNs: number, processId: number): Promise<Array<GpuMemory>> =>
  query<GpuMemory>(
    'getTabGpuMemoryVMTrackerClickData',
    `SELECT
    (S.ts-TR.start_ts) as startNs,
    S.used_gpu_size as size,
    T.tid as threadId,
    T.name as threadName,
    A.data as gpuName
    from trace_range as TR,memory_process_gpu as S
    left join thread as T on T.itid=S.itid
    left join data_dict as A on A.id=S.gpu_name_id
    WHERE
    startNs = ${startNs}
    AND
    $pid = S.ipid
              `,
    { $startNs: startNs, $pid: processId }
  );

//Ability Monitor Dma 点选比较
export const getTabDmaAbilityComparisonData = (startNs: number): Promise<Array<DmaComparison>> =>
  query<DmaComparison>(
    'getTabDmaAbilityComparisonData',
    `SELECT
      (S.ts-TR.start_ts) as startNs,
      sum(S.size) as value,
      E.pid as processId,
      E.name as processName
      from trace_range as TR,memory_dma as S
      left join process as E on E.ipid=S.ipid
      WHERE
      startNs = ${startNs}
      GROUP by
      E.pid
                `,
    { $startNs: startNs }
  );

//Ability Monitor Gpu Memory 点选比较
export const getTabGpuMemoryComparisonData = (startNs: number): Promise<Array<GpuMemoryComparison>> =>
  query<GpuMemoryComparison>(
    'getTabGpuMemoryComparisonData',
    `SELECT
      (S.ts-TR.start_ts) as startNs,
      sum(S.used_gpu_size) as value,
      E.pid as processId,
      S.gpu_name_id as gpuNameId,
      E.name as processName
      from trace_range as TR,memory_process_gpu as S
      left join process as E on E.ipid=S.ipid
      WHERE
      startNs = ${startNs}
      GROUP by
      E.pid, S.gpu_name_id
                `,
    { $startNs: startNs }
  );

//VM Tracker Dma 点选比较
export const getTabDmaVmTrackerComparisonData = (startNs: number, processId: number): Promise<Array<DmaComparison>> =>
  query<DmaComparison>(
    'getTabDmaVmTrackerComparisonData',
    `SELECT
    (S.ts-TR.start_ts) as startNs,
    sum(S.size) as value
    from trace_range as TR,memory_dma as S
    WHERE
    startNs = ${startNs}
    AND
    $pid = S.ipid
                `,
    { $startNs: startNs, $pid: processId }
  );

//VM Tracker Gpu Memory 点选比较
export const getTabGpuMemoryVmTrackerComparisonData = (
  startNs: number,
  processId: number
): Promise<Array<GpuMemoryComparison>> =>
  query<GpuMemoryComparison>(
    'getTabGpuMemoryVmTrackerComparisonData',
    `SELECT
    (S.ts-TR.start_ts) as startNs,
    sum(S.used_gpu_size) as value,
    T.tid as threadId,
    T.name as threadName,
    S.gpu_name_id as gpuNameId
    from trace_range as TR,memory_process_gpu as S
    left join thread as T on T.itid=S.itid
    WHERE
    startNs = ${startNs}
    AND
    $pid = S.ipid
                `,
    { $startNs: startNs, $pid: processId }
  );
