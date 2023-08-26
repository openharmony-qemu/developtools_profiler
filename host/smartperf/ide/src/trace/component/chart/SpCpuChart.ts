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

import { SpSystemTrace } from '../SpSystemTrace.js';
import {
  queryCpuCount,
  queryCpuData,
  queryCpuDataCount,
  queryCpuMax,
  queryCpuSchedSlice,
} from '../../database/SqlLite.js';
import { info } from '../../../log/Log.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { procedurePool } from '../../database/Procedure.js';
import { CpuRender, CpuStruct } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { Utils } from '../trace/base/Utils.js';

export class SpCpuChart {
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let CpuStartTime = new Date().getTime();
    let array = await queryCpuMax();
    let cpuCountResult = await queryCpuCount();
    if (cpuCountResult && cpuCountResult.length > 0 && cpuCountResult[0]) {
      (window as any).cpuCount = cpuCountResult[0].cpuCount;
    } else {
      (window as any).cpuCount = 0;
    }
    let dataCount: { count: number; cpu: number }[] = (await queryCpuDataCount()) as { count: number; cpu: number }[];
    let cpuSchedSlice = await queryCpuSchedSlice();
    this.initSchedSliceData(cpuSchedSlice);
    info('Cpu trace row data size is: ', array.length);
    if (array && array.length > 0 && array[0]) {
      let cpuMax = array[0].cpu;
      CpuStruct.cpuCount = cpuMax + 1;
      for (let i1 = 0; i1 < CpuStruct.cpuCount; i1++) {
        if (dataCount.find((it) => it.cpu === i1 && it.count > 0)) {
          const cpuId = i1;
          let traceRow = TraceRow.skeleton<CpuStruct>();
          traceRow.rowId = `${cpuId}`;
          traceRow.rowType = TraceRow.ROW_TYPE_CPU;
          traceRow.rowParentId = '';
          traceRow.style.height = '40px';
          traceRow.name = `Cpu ${cpuId}`;
          traceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
          traceRow.selectChangeHandler = this.trace.selectChangeHandler;
          traceRow.supplier = () =>
            queryCpuData(cpuId, TraceRow.range?.startNS || 0, TraceRow.range?.endNS || 0).then((res) => {
              res.forEach((it, i, arr) => {
                let p = Utils.PROCESS_MAP.get(it.processId!);
                let t = Utils.THREAD_MAP.get(it.tid!);
                let slice = Utils.SCHED_SLICE_MAP.get(`${it.id}-${it.startTime}`);
                if (slice) {
                  it.end_state = slice.endState;
                  it.priority = slice.priority;
                }
                it.processName = p;
                it.processCmdLine = p;
                it.name = t;
                it.type = 'thread';
                if (i !== arr.length - 1) {
                  if (it.startTime! + it.dur! > arr[i + 1]!.startTime! || it.dur == -1) {
                    it.dur = arr[i + 1]!.startTime! - it.startTime!;
                    it.nofinish = true;
                  }
                } else {
                  if (it.dur == -1) {
                    it.dur = TraceRow.range!.endNS - it.startTime!;
                    it.nofinish = true;
                  }
                }
              });
              return res;
            });
          traceRow.focusHandler = () => {
            this.trace?.displayTip(
              traceRow,
              CpuStruct.hoverCpuStruct,
              `<span>P：${CpuStruct.hoverCpuStruct?.processName || 'Process'} [${
                CpuStruct.hoverCpuStruct?.processId
              }]</span><span>T：${CpuStruct.hoverCpuStruct?.name} [${CpuStruct.hoverCpuStruct?.tid}] [Prio:${
                CpuStruct.hoverCpuStruct?.priority || 0
              }]</span>`
            );
          };
          traceRow.onThreadHandler = (useCache: boolean, buf: ArrayBuffer | undefined | null) => {
            let context = traceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
            traceRow.canvasSave(context);
            (renders['cpu-data'] as CpuRender).renderMainThread(
              {
                cpuRenderContext: context,
                useCache: useCache,
                type: `cpu-data-${i1}`,
                translateY: traceRow.translateY,
              },
              traceRow
            );
            traceRow.canvasRestore(context);
          };
          this.trace.rowsEL?.appendChild(traceRow);
        }
      }
    }
    let CpuDurTime = new Date().getTime() - CpuStartTime;
    info('The time to load the Cpu data is: ', CpuDurTime);
  }

  initProcessThreadStateData = async (progress: Function) => {
    let time = new Date().getTime();
    progress('StateProcessThread', 93);
    procedurePool.submitWithName('logic1', 'spt-init', {}, undefined, (res: any) => {});
    let durTime = new Date().getTime() - time;
    info('The time to load the first ProcessThreadState data is: ', durTime);
  };

  initCpuIdle0Data = async (progress: Function) => {
    let time = new Date().getTime();
    progress('CPU Idle', 94);
    procedurePool.submitWithName(
      'logic1',
      'scheduling-getCpuIdle0',
      {
        endTs: (window as any).recordEndNS,
        total: (window as any).totalNS,
      },
      undefined,
      (res: any) => {}
    );
    let durTime = new Date().getTime() - time;
    info('The time to load the first CPU Idle0 data is: ', durTime);
  };

  initSchedSliceData(arr: any[]) {
    Utils.SCHED_SLICE_MAP.clear();
    arr.forEach((value) => {
      Utils.SCHED_SLICE_MAP.set(`${value.itid}-${value.ts}`, { endState: value.endState, priority: value.priority });
    });
  }

  initSchedulingPTData = async (progress: Function) => {
    let time = new Date().getTime();
    progress('CPU Idle', 94);
    procedurePool.submitWithName('logic1', 'scheduling-getProcessAndThread', {}, undefined, (res: any) => {});
    let durTime = new Date().getTime() - time;
    info('The time to load the first CPU Idle0 data is: ', durTime);
  };

  initSchedulingFreqData = async (progress: Function) => {
    let time = new Date().getTime();
    progress('CPU Scheduling Freq', 94);
    procedurePool.submitWithName('logic1', 'scheduling-initFreqData', {}, undefined, (res: any) => {});
    let durTime = new Date().getTime() - time;
    info('The time to load the first CPU Idle0 data is: ', durTime);
  };
}
