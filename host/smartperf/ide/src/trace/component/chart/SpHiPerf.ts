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
import { TraceRow } from '../trace/base/TraceRow.js';
import {
  queryHiPerfCpuData,
  queryHiPerfCpuMergeData,
  queryHiPerfCpuMergeData2,
  queryHiPerfProcessData,
  queryHiPerfThreadData,
  queryPerfEventType,
  queryPerfCmdline,
  queryPerfThread,
} from '../../database/SqlLite.js';
import { Utils } from '../trace/base/Utils.js';
import { PerfThread } from '../../bean/PerfProfile.js';
import { HiperfCpuRender, HiPerfCpuStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfCPU.js';
import { HiPerfCallChartRender, HiPerfCallChartStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfCallChart.js';
import { HiperfThreadRender, HiPerfThreadStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfThread.js';
import { HiperfProcessRender, HiPerfProcessStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfProcess.js';
import { info } from '../../../log/Log.js';
import { HiPerfEventStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfEvent.js';
import { perfDataQuery } from './PerfDataQuery.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { type HiPerfReportStruct } from '../../database/ui-worker/ProcedureWorkerHiPerfReport.js';
import { SpChartManager } from './SpChartManager.js';
import { procedurePool } from '../../database/Procedure.js';
import { HiPerfChartFrame } from '../../bean/PerfStruct.js';

export interface ResultData {
  existA: boolean | null | undefined;
  existF: boolean | null | undefined;
  fValue: number;
}

export class SpHiPerf {
  static selectCpuStruct: HiPerfCpuStruct | undefined;
  static selectProcessStruct: HiPerfProcessStruct | undefined;
  static selectThreadStruct: HiPerfThreadStruct | undefined;
  static stringResult: ResultData | undefined;

  private cpuData: Array<any> | undefined;
  public maxCpuId: number = 0;
  private rowFolder!: TraceRow<any>;
  private perfThreads: Array<PerfThread> | undefined;
  private trace: SpSystemTrace;
  private group: any;
  private rowList: TraceRow<any>[] | undefined;
  private eventTypeList: Array<{ id: number; report: string }> = [];
  private callChartType: number = 0;
  private callChartId: number = 0;
  private eventTypeId: number = -2;
  private stackChartMaxDepth: number = 1;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    await this.initCmdLine();
    this.eventTypeList = await queryPerfEventType();
    this.rowList = [];
    this.perfThreads = await queryPerfThread();
    info('PerfThread Data size is: ', this.perfThreads!.length);
    this.group = Utils.groupBy(this.perfThreads || [], 'pid');
    this.cpuData = await queryHiPerfCpuMergeData2();
    this.callChartType = 0;
    this.callChartId = 0;
    this.eventTypeId = -2;
    this.maxCpuId = this.cpuData.length > 0 ? this.cpuData[0].cpu_id : -Infinity;
    if (this.cpuData.length > 0) {
      await this.initFolder();
      await this.initCallChart();
      await this.initCpuMerge();
      await this.initCpu();
      await this.initProcess();
    }
    info('HiPerf Data initialized');
  }

  getStringResult(s: string = '') {
    let list = s.split(' ').filter((e) => e);
    let sA = list.findIndex((item) => item == '-a');
    let sF = list.findIndex((item) => item == '-f');
    SpHiPerf.stringResult = {
      existA: sA !== -1,
      existF: sF !== -1,
      fValue: Number((1000 / (sF !== -1 ? parseInt(list[sF + 1]) : 1000)).toFixed(2)),
    };
  }

  async initCmdLine() {
    let perfCmdLines = await queryPerfCmdline();
    if (perfCmdLines.length > 0) {
      this.getStringResult(perfCmdLines[0].report_value);
    } else {
      SpHiPerf.stringResult = {
        existA: true,
        existF: false,
        fValue: 1,
      };
    }
  }

  async initFolder() {
    let row = TraceRow.skeleton();
    row.setAttribute('disabled-check', '');
    row.rowId = `HiPerf`;
    row.index = 0;
    row.rowType = TraceRow.ROW_TYPE_HIPERF;
    row.rowParentId = '';
    row.folder = true;
    row.drawType = -2;
    row.rowSetting = 'enable';
    row.rowSettingPopoverDirection = 'bottomLeft';
    row.rowSettingList = [
      {
        key: '-2',
        title: 'Cpu Usage',
        checked: true,
      },
      ...this.eventTypeList.map(et => {
        return {
          key: `${et.id}`,
          title: et.report
        };
      }),
    ];
    row.onRowSettingChangeHandler = (value) => {
      let drawType = parseInt(value[0]);
      if (this.eventTypeId !== drawType) {
        this.eventTypeId = drawType;
        row.drawType = drawType;
        row.childrenList.forEach((child) => {
          if (child.drawType !== drawType) {
            child.drawType = drawType;
            if (child.rowType === TraceRow.ROW_TYPE_PERF_CALLCHART) {
              this.resetChartData(child);
            } else {
              child.drawType = drawType;
              child.dataList2 = [];
              child.childrenList.forEach((sz) => {
                sz.drawType = drawType
                sz.dataList2 = [];
              });
            }
          }
        });
        this.trace.refreshCanvas(false);
      }
    };
    row.style.height = '40px';
    if (SpHiPerf.stringResult?.existA === true) {
      row.name = `HiPerf (All)`;
    } else {
      let names = Reflect.ownKeys(this.group)
        .map((pid: any) => {
          let array = this.group[pid] as Array<PerfThread>;
          let process = array.filter((th) => th.pid === th.tid)[0];
          return process.processName;
        })
        .join(',');
      row.name = `HiPerf (${names})`;
    }
    row.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
    row.onThreadHandler = (useCache) => {
      row.canvasSave(this.trace.canvasPanelCtx!);
      if (row.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, row.frame.width, row.frame.height);
      } else {
        (renders['empty'] as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: ``,
          },
          row
        );
      }
      row.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.rowFolder = row;
    this.trace.rowsEL?.appendChild(row);
  }

  async initCallChart() {
    let perfCallCutRow = TraceRow.skeleton<HiPerfCallChartStruct>();
    perfCallCutRow.rowId = `HiPerf-callchart`;
    perfCallCutRow.index = 0;
    perfCallCutRow.rowType = TraceRow.ROW_TYPE_PERF_CALLCHART;
    perfCallCutRow.enableCollapseChart();
    perfCallCutRow.rowParentId = 'HiPerf';
    perfCallCutRow.rowHidden = !this.rowFolder.expansion;
    perfCallCutRow.folder = false;
    perfCallCutRow.drawType = -2;
    perfCallCutRow.name = 'CallChart [cpu0]';
    perfCallCutRow.setAttribute('children', '');
    perfCallCutRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    perfCallCutRow.selectChangeHandler = this.trace.selectChangeHandler;
    this.rowFolder.addChildTraceRow(perfCallCutRow);
    perfCallCutRow.focusHandler = (): void => {
      let hoverStruct = HiPerfCallChartStruct.hoverPerfCallCutStruct;
      if (hoverStruct) {
        let selfDur = hoverStruct?.totalTime || 0;
        hoverStruct?.children?.forEach(child => {
          selfDur -= child.totalTime;
        });
        let callName = HiPerfCallChartStruct.hoverPerfCallCutStruct?.name || '';
        callName = callName.replace(/</g, '&lt;').replace(/>/g, '&gt;');
        this.trace?.displayTip(
          perfCallCutRow!,
          HiPerfCallChartStruct.hoverPerfCallCutStruct,
          `<span style="font-weight: bold;color:'#000'">Name: </span>
        <span>${callName}</span><br>
        <span style='font-weight: bold;'>Lib: </span>
        <span>${perfDataQuery.getLibName(hoverStruct!.fileId,hoverStruct!.symbolId)}</span><br>
        <span style='font-weight: bold;'>Self Time: </span>
        <span>${Utils.getProbablyTime(selfDur || 0)}</span><br>
        <span style='font-weight: bold;'>Duration: </span>
        <span>${Utils.getProbablyTime(hoverStruct.totalTime)}</span><br>
        <span style='font-weight: bold;'>Event Count: </span>
        <span>${HiPerfCallChartStruct.hoverPerfCallCutStruct?.eventCount || ''}</span><br>`
        );
      }
    };
    // @ts-ignore
    perfCallCutRow.supplier = () => this.getHiPerfChartData(this.callChartType, this.callChartId, this.eventTypeId, perfCallCutRow);
    perfCallCutRow.findHoverStruct = () => {
      HiPerfCallChartStruct.hoverPerfCallCutStruct = perfCallCutRow.getHoverStruct();
    };
    await this.setCallTotalRow(perfCallCutRow, this.cpuData, this.perfThreads);
  }

  async setCallTotalRow(row: TraceRow<any>, cpuData: any = Array, threadData: any = Array) {
    let pt: Map<string, any> = threadData.reduce((map: Map<string, any>, current: any) => {
      const key = `${current.processName || 'Process'}(${current.pid})`;
      const thread = {
        key: `${current.tid}-t`,
        title: `${current.threadName || 'Thread'}(${current.tid})`
      }
      if (map.has(key)) {
        if (map.get(key).children) {
          map.get(key).children.push(thread);
        } else {
          map.get(key).children = [thread];
        }
      } else {
        map.set(key, {
          key: `${current.pid}-p`,
          title: key,
          children: [thread],
          disable: true
        });
      }
      return map;
    }, new Map<string, any>());
    row.addTemplateTypes('hiperf-callchart');
    row.rowSetting = 'enable';
    row.rowSettingList = [
      ...cpuData.reverse().map((it: any): {
          key: string;
          title: string;
          checked?: boolean
        } => {
          return {
            key: `${it.cpu_id}-c`,
            checked: it.cpu_id === 0,
            title: `cpu${it.cpu_id}`,
          };
        }
      ),
      ...Array.from(pt.values())
    ];
    row.onRowSettingChangeHandler = (setting: any, nodes): void => {
      if (setting && setting.length > 0) {
        //type 0:cpu,1:process,2:thread
        let key: string = setting[0];
        let type = this.callChartType;
        if (key.includes('p')) {
          type = 1;
        } else if (key.includes('t')) {
          type = 2;
        } else {
          type = 0;
        }
        let id = Number(key.split('-')[0]);
        if (this.callChartType === type && this.callChartId === id) {
          return;
        }
        this.callChartType = type;
        this.callChartId = id;
        row.name = `CallChart [${nodes[0].title}]`;
        this.resetChartData(row);
        this.trace.refreshCanvas(false);
      }
    };
    row.onThreadHandler = (useCache: any) => {
      let context: CanvasRenderingContext2D;
      if (row.currentContext) {
        context = row.currentContext;
      } else {
        context = row.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      row.canvasSave(context);
      (renders['HiPerf-callchart'] as HiPerfCallChartRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `HiPerf-callchart`,
        },
        row
      );
      row.canvasRestore(context);
    };
  }

  async initCpuMerge() {
    let cpuMergeRow = TraceRow.skeleton<HiPerfCpuStruct>();
    cpuMergeRow.rowId = `HiPerf-cpu-merge`;
    cpuMergeRow.index = 0;
    cpuMergeRow.rowType = TraceRow.ROW_TYPE_HIPERF_CPU;
    cpuMergeRow.rowParentId = 'HiPerf';
    cpuMergeRow.rowHidden = !this.rowFolder.expansion;
    cpuMergeRow.folder = false;
    cpuMergeRow.drawType = -2;
    cpuMergeRow.name = `HiPerf`;
    cpuMergeRow.style.height = '40px';
    cpuMergeRow.setAttribute('children', '');
    cpuMergeRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    cpuMergeRow.selectChangeHandler = this.trace.selectChangeHandler;
    cpuMergeRow.supplier = () => queryHiPerfCpuMergeData();
    cpuMergeRow.focusHandler = () => this.hoverTip(cpuMergeRow, HiPerfCpuStruct.hoverStruct);
    cpuMergeRow.findHoverStruct = () => {
      HiPerfCpuStruct.hoverStruct = cpuMergeRow.getHoverStruct(false, (TraceRow.range?.scale || 50) <= 30_000_000);
    };
    cpuMergeRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (cpuMergeRow.currentContext) {
        context = cpuMergeRow.currentContext;
      } else {
        context = cpuMergeRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      cpuMergeRow.canvasSave(context);
      (renders['HiPerf-Cpu'] as HiperfCpuRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          scale: TraceRow.range?.scale || 50,
          type: `HiPerf-Cpu-Merge`,
          maxCpu: this.maxCpuId + 1,
          intervalPerf: SpHiPerf.stringResult?.fValue || 1,
          range: TraceRow.range,
        },
        cpuMergeRow
      );
      cpuMergeRow.canvasRestore(context);
    };
    this.rowFolder.addChildTraceRow(cpuMergeRow);
    this.rowList?.push(cpuMergeRow);
  }

  async initCpu() {
    for (let i = 0; i <= this.maxCpuId; i++) {
      let perfCpuRow = TraceRow.skeleton<HiPerfCpuStruct>();
      perfCpuRow.rowId = `HiPerf-cpu-${i}`;
      perfCpuRow.index = i;
      perfCpuRow.rowType = TraceRow.ROW_TYPE_HIPERF_CPU;
      perfCpuRow.rowParentId = 'HiPerf';
      perfCpuRow.rowHidden = !this.rowFolder.expansion;
      perfCpuRow.folder = false;
      perfCpuRow.drawType = -2;
      perfCpuRow.name = `Cpu ${i}`;
      perfCpuRow.setAttribute('children', '');
      perfCpuRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      perfCpuRow.selectChangeHandler = this.trace.selectChangeHandler;
      perfCpuRow.style.height = '40px';
      perfCpuRow.supplier = () => queryHiPerfCpuData(i);
      perfCpuRow.focusHandler = () => this.hoverTip(perfCpuRow, HiPerfCpuStruct.hoverStruct);
      perfCpuRow.findHoverStruct = () => {
        HiPerfCpuStruct.hoverStruct = perfCpuRow.getHoverStruct(false, (TraceRow.range?.scale || 50) <= 30_000_000);
      };
      perfCpuRow.onThreadHandler = (useCache) => {
        let context: CanvasRenderingContext2D;
        if (perfCpuRow.currentContext) {
          context = perfCpuRow.currentContext;
        } else {
          context = perfCpuRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        }
        perfCpuRow.canvasSave(context);
        (renders['HiPerf-Cpu'] as HiperfCpuRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            scale: TraceRow.range?.scale || 50,
            type: `HiPerf-Cpu-${i}`,
            maxCpu: this.maxCpuId + 1,
            intervalPerf: SpHiPerf.stringResult?.fValue || 1,
            range: TraceRow.range,
          },
          perfCpuRow
        );
        perfCpuRow.canvasRestore(context);
      };
      this.rowFolder.addChildTraceRow(perfCpuRow);
      this.rowList?.push(perfCpuRow);
    }
  }

  async initProcess() {
    Reflect.ownKeys(this.group).forEach((key, index) => {
      let array = this.group[key] as Array<PerfThread>;
      let process = array.filter((th) => th.pid === th.tid)[0];
      let row = TraceRow.skeleton<HiPerfProcessStruct>();
      row.rowId = `${process.pid}-Perf-Process`;
      row.index = index;
      row.rowType = TraceRow.ROW_TYPE_HIPERF_PROCESS;
      row.rowParentId = 'HiPerf';
      row.rowHidden = !this.rowFolder.expansion;
      row.folder = true;
      row.drawType = -2;
      if (SpChartManager.APP_STARTUP_PID_ARR.find((pid) => pid === process.pid) !== undefined) {
        row.addTemplateTypes('AppStartup');
      }
      row.addTemplateTypes('HiPerf');
      row.name = `${process.processName || 'Process'} [${process.pid}]`;
      row.folderPaddingLeft = 6;
      row.style.height = '40px';
      row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      row.selectChangeHandler = this.trace.selectChangeHandler;
      row.supplier = () => queryHiPerfProcessData(process.pid);
      row.focusHandler = () => this.hoverTip(row, HiPerfProcessStruct.hoverStruct);
      row.findHoverStruct = () => {
        HiPerfProcessStruct.hoverStruct = row.getHoverStruct(false, (TraceRow.range?.scale || 50) <= 30_000_000);
      };
      row.onThreadHandler = (useCache) => {
        let context: CanvasRenderingContext2D;
        if (row.currentContext) {
          context = row.currentContext;
        } else {
          context = row.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        }
        row.canvasSave(context);
        if (row.expansion) {
          this.trace.canvasPanelCtx?.clearRect(0, 0, row.frame.width, row.frame.height);
        } else {
          (renders['HiPerf-Process'] as HiperfProcessRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              scale: TraceRow.range?.scale || 50,
              type: `HiPerf-Process-${row.index}`,
              intervalPerf: SpHiPerf.stringResult?.fValue || 1,
              range: TraceRow.range,
            },
            row
          );
        }
        row.canvasRestore(context);
      };
      this.rowFolder.addChildTraceRow(row);
      this.rowList?.push(row);
      array.forEach((thObj, thIdx) => {
        let thread = TraceRow.skeleton<HiPerfThreadStruct>();
        thread.rowId = `${thObj.tid}-Perf-Thread`;
        thread.index = thIdx;
        thread.rowType = TraceRow.ROW_TYPE_HIPERF_THREAD;
        thread.rowParentId = row.rowId;
        thread.rowHidden = !row.expansion;
        thread.folder = false;
        thread.drawType = -2;
        thread.name = `${thObj.threadName || 'Thread'} [${thObj.tid}]`;
        thread.setAttribute('children', '');
        thread.folderPaddingLeft = 0;
        thread.style.height = '40px';
        thread.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        thread.selectChangeHandler = this.trace.selectChangeHandler;
        thread.supplier = () => queryHiPerfThreadData(thObj.tid);
        thread.focusHandler = () => this.hoverTip(thread, HiPerfThreadStruct.hoverStruct);
        thread.findHoverStruct = () => {
          HiPerfThreadStruct.hoverStruct = thread.getHoverStruct(false, (TraceRow.range?.scale || 50) <= 30_000_000);
        };
        thread.onThreadHandler = (useCache) => {
          let context: CanvasRenderingContext2D;
          if (thread.currentContext) {
            context = thread.currentContext;
          } else {
            context = thread.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
          }
          thread.canvasSave(context);
          (renders['HiPerf-Thread'] as HiperfThreadRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              scale: TraceRow.range?.scale || 50,
              type: `HiPerf-Thread-${row.index}-${thread.index}`,
              intervalPerf: SpHiPerf.stringResult?.fValue || 1,
              range: TraceRow.range,
            },
            thread
          );
          thread.canvasRestore(context);
        };
        row.addChildTraceRow(thread);
        this.rowList?.push(thread);
      });
    });
  }

  async getHiPerfChartData(type: number, id: number, eventTypeId: number, row: TraceRow<any>) {
    let source: Array<HiPerfChartFrame> = [];
    this.stackChartMaxDepth = 1;
    await new Promise((resolve) => {
      procedurePool.submitWithName('logic0', 'perf-callstack-chart', [type, id, eventTypeId, (window as any).totalNS], undefined, (res: any) => {
        this.getAllCombineData(res, source);
        let maxHeight = this.stackChartMaxDepth * 20;
        row.funcMaxHeight = maxHeight;
        if (row.funcExpand) {
          row!.style.height = `${maxHeight}px`;
          if (row.collect) {
            window.publish(window.SmartEvent.UI.RowHeightChange, {
              expand: true,
              value: row.funcMaxHeight - 20,
            });
          }
        }
        resolve(source);
      });
    })
    return source;
  }

  getAllCombineData(
    combineData: Array<HiPerfChartFrame>,
    allCombineData: Array<HiPerfChartFrame>
  ): void {
    for (let data of combineData) {
      if (data.name != 'name') {
        allCombineData.push(data);
      }
      if (data.depth + 1 > this.stackChartMaxDepth) {
        this.stackChartMaxDepth = data.depth + 1;
      }
      if (data.children && data.children.length > 0) {
        this.getAllCombineData(data.children, allCombineData);
      }
    }
  }

  resetChartData(row: TraceRow<any>) {
    row.dataList = [];
    row.dataList2 = [];
    row.dataListCache = [];
    row.isComplete = false;
  }

  resetAllChartData() : void {
    this.rowList?.forEach(row => this.resetChartData(row));
  }

  hoverTip(
    row: TraceRow<any>,
    struct:
      | HiPerfThreadStruct
      | HiPerfProcessStruct
      | HiPerfEventStruct
      | HiPerfReportStruct
      | HiPerfCpuStruct
      | undefined
  ) {
    let tip = '';
    let groupBy10MS = (TraceRow.range?.scale || 50) > 30_000_000;
    if (struct) {
      if (groupBy10MS) {
        if (row.drawType === -2) {
          let num = 0;
          if (struct instanceof HiPerfEventStruct) {
            num = Math.trunc(((struct.sum || 0) / (struct.max || 0)) * 100);
          } else {
            num = Math.trunc(((struct.height || 0) / 40) * 100);
          }
          if (num > 0) {
            tip = `<span>${num * (this.maxCpuId + 1)}% (10.00ms)</span>`;
          }
        } else {
          tip = `<span>${struct.eventCount} (10.00ms)</span>`;
        }
      } else {
        let perfCall = perfDataQuery.callChainMap.get(struct.callchain_id || 0);
        tip = `<span>${perfCall ? perfCall.name : ''} (${perfCall ? perfCall.depth : '0'} other frames)</span>`;
      }
    }
    this.trace?.displayTip(row, struct, tip);
  }
}
