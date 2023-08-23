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
import { info } from '../../../log/Log.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { HeapTimelineRender, HeapTimelineStruct } from '../../database/ui-worker/ProcedureWorkerHeapTimeline.js';
import { HeapDataInterface, ParseListener } from '../../../js-heap/HeapDataInterface.js';
import { LoadDatabase } from '../../../js-heap/LoadDatabase.js';
import { FileInfo } from '../../../js-heap/model/UiStruct.js';
import { HeapSnapshotRender, HeapSnapshotStruct } from '../../database/ui-worker/ProcedureWorkerHeapSnapshot.js';
import { procedurePool } from '../../database/Procedure.js';
import { Utils } from '../trace/base/Utils.js';
import { JsCpuProfilerChartFrame } from '../../bean/JsStruct.js';
import { JsCpuProfilerRender, JsCpuProfilerStruct } from '../../database/ui-worker/ProcedureWorkerCpuProfiler.js';
import { ns2s } from '../../database/ui-worker/ProcedureWorkerCommon.js';
import { queryJsCpuProfilerConfig, queryJsCpuProfilerData, queryJsMemoryData } from '../../database/SqlLite.js';
const TYPE_SNAPSHOT = 0;
const TYPE_TIMELINE = 1;
export class SpArkTsChart implements ParseListener {
  private trace: SpSystemTrace;
  private folderRow: TraceRow<any> | undefined;
  private jsCpuProfilerRow: TraceRow<JsCpuProfilerStruct> | undefined;
  private heapTimelineRow: TraceRow<HeapTimelineStruct> | undefined;
  private heapSnapshotRow: TraceRow<HeapSnapshotStruct> | undefined;
  private loadJsDatabase: LoadDatabase;
  private allCombineDataMap = new Map<number, JsCpuProfilerChartFrame>();
  private process: string = '';
  constructor(trace: SpSystemTrace) {
    this.trace = trace;
    this.loadJsDatabase = LoadDatabase.getInstance();
  }

  public get chartFrameMap(): Map<number, JsCpuProfilerChartFrame> {
    return this.allCombineDataMap;
  }

  public async initFolder(): Promise<void> {
    let jsConfig = await queryJsCpuProfilerConfig();

    let jsCpu = await queryJsCpuProfilerData();
    let jsMemory = await queryJsMemoryData();
    if (jsMemory.length > 0 || jsCpu.length > 0) {
      this.folderRow = TraceRow.skeleton();
      this.process = jsConfig[0].pid;
      this.folderRow.rowId = this.process;
      this.folderRow.rowType = TraceRow.ROW_TYPE_ARK_TS;
      this.folderRow.style.height = '40px';
      this.folderRow.rowParentId = '';
      this.folderRow.folder = true;
      this.folderRow.name = `Ark Ts ` + this.process;
      this.folderRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      this.folderRow.selectChangeHandler = this.trace.selectChangeHandler;
      this.folderRow.supplier = (): Promise<Array<unknown>> => new Promise<Array<unknown>>((resolve) => resolve([]));
      this.folderRow.onThreadHandler = (useCache): void => {
        this.folderRow!.canvasSave(this.trace.canvasPanelCtx!);
        if (this.folderRow!.expansion) {
          this.trace.canvasPanelCtx?.clearRect(0, 0, this.folderRow!.frame.width, this.folderRow!.frame.height);
        } else {
          (renders['empty'] as EmptyRender).renderMainThread(
            {
              context: this.trace.canvasPanelCtx,
              useCache: useCache,
              type: ``,
            },
            this.folderRow!
          );
        }
        this.folderRow!.canvasRestore(this.trace.canvasPanelCtx!);
      };
      this.trace.rowsEL?.appendChild(this.folderRow);
      if (this.folderRow && jsConfig[0].type !== -1 && jsMemory.length > 0) {
        if (jsConfig[0].type === TYPE_SNAPSHOT) {
          // snapshot
          await this.initSnapshotChart();
        } else if (jsConfig[0].type === TYPE_TIMELINE) {
          // timeline
          await this.initTimelineChart();
        }
      }
      if (this.folderRow && jsConfig[0].enableCpuProfiler === 1 && jsCpu.length > 0) {
        await this.initJsCpuChart();
      }
      if ((this.heapSnapshotRow || this.heapTimelineRow) && jsMemory.length > 0) {
        await this.initJsMemoryChartData();
      }
      if (this.jsCpuProfilerRow && jsCpu.length > 0) {
        await this.initJsCpuProfilerChartData();
      }
    }
  }

  private async initTimelineChart(): Promise<void> {
    this.heapTimelineRow = TraceRow.skeleton<HeapTimelineStruct>();
    this.heapTimelineRow.rowParentId = this.process;
    this.heapTimelineRow.rowHidden = !this.folderRow!.expansion;
    this.heapTimelineRow.style.height = '40px';
    this.heapTimelineRow.name = `Heaptimeline`;
    this.heapTimelineRow.folder = false;
    this.heapTimelineRow.rowType = TraceRow.ROW_TYPE_HEAP_TIMELINE;
    this.heapTimelineRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    this.heapTimelineRow.selectChangeHandler = this.trace.selectChangeHandler;
    this.heapTimelineRow.setAttribute('children', '');
    this.heapTimelineRow!.focusHandler = (): void => {
      this.trace?.displayTip(
        this.heapTimelineRow!,
        HeapTimelineStruct.hoverHeapTimelineStruct,
        `<span>Size: ${Utils.getBinaryByteWithUnit(HeapTimelineStruct.hoverHeapTimelineStruct?.size || 0)}</span>`
      );
    };
    this.folderRow!.addChildTraceRow(this.heapTimelineRow!);
  }

  private async initSnapshotChart(): Promise<void> {
    this.heapSnapshotRow = TraceRow.skeleton<HeapSnapshotStruct>();
    this.heapSnapshotRow.rowParentId = this.process;
    this.heapSnapshotRow.rowHidden = !this.folderRow!.expansion;
    this.heapSnapshotRow.style.height = '40px';
    this.heapSnapshotRow.name = `Heapsnapshot`;
    this.heapSnapshotRow.rowId = `heapsnapshot`;
    this.heapSnapshotRow.folder = false;
    this.heapSnapshotRow.rowType = TraceRow.ROW_TYPE_HEAP_SNAPSHOT;
    this.heapSnapshotRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    this.heapSnapshotRow.selectChangeHandler = this.trace.selectChangeHandler;
    this.heapSnapshotRow.setAttribute('children', '');
    this.heapSnapshotRow!.focusHandler = (): void => {
      this.trace?.displayTip(
        this.heapSnapshotRow!,
        HeapSnapshotStruct.hoverSnapshotStruct,
        `<span>Name: ${HeapSnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
            <span>Size: ${Utils.getBinaryByteWithUnit(HeapSnapshotStruct.hoverSnapshotStruct?.size || 0)}</span>`
      );
    };
    this.folderRow!.addChildTraceRow(this.heapSnapshotRow);
  }

  private initJsMemoryChartData = async (): Promise<void> => {
    let time = new Date().getTime();
    await this.loadJsDatabase.loadDatabase(this);
    let durTime = new Date().getTime() - time;
    info('The time to load the js Memory data is: ', durTime);
  };

  public async parseDone(fileModule: Array<FileInfo>): Promise<void> {
    if (fileModule.length > 0) {
      let heapFile = HeapDataInterface.getInstance().getFileStructs();
      let file = heapFile[0];
      this.trace.snapshotFile = file;
      if (file.type === TYPE_TIMELINE) {
        let samples = HeapDataInterface.getInstance().getSamples(file.id);
        this.heapTimelineRow!.rowId = `heaptimeline` + file.id;
        this.heapTimelineRow!.supplier = (): Promise<any> => new Promise<any>((resolve) => resolve(samples));
        this.heapTimelineRow!.onThreadHandler = (useCache): void => {
          let context = this.heapTimelineRow!.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
          this.heapTimelineRow!.canvasSave(context);
          (renders['heap-timeline'] as HeapTimelineRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              type: `heap-timeline`,
              samples: samples,
            },
            this.heapTimelineRow!
          );
          this.heapTimelineRow!.canvasRestore(context);
        };
      } else if (file.type === TYPE_SNAPSHOT) {
        this.heapSnapshotRow!.supplier = (): Promise<Array<any>> =>
          new Promise<Array<any>>((resolve) => resolve(heapFile));
        this.heapSnapshotRow!.onThreadHandler = (useCache): void => {
          let context = this.heapSnapshotRow!.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
          this.heapSnapshotRow!.canvasSave(context);
          (renders['heap-snapshot'] as HeapSnapshotRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              type: `heap-snapshot`,
            },
            this.heapSnapshotRow!
          );
          this.heapSnapshotRow!.canvasRestore(context);
        };
      }
    }
  }

  private async initJsCpuProfilerChartData(): Promise<void> {
    procedurePool.submitWithName(
      'logic1',
      'jsCpuProfiler-init',
      SpSystemTrace.DATA_DICT,
      undefined,
      (res: Array<JsCpuProfilerChartFrame>) => {
        let allCombineData: Array<JsCpuProfilerChartFrame> = [];
        this.getAllCombineData(res, allCombineData);
        this.allCombineDataMap = new Map<number, JsCpuProfilerChartFrame>();
        for (let data of allCombineData) {
          this.allCombineDataMap.set(data.id, data);
        }
        let max = Math.max(...allCombineData.map((it) => it.depth || 0)) + 1;
        let maxHeight = max * 20;
        this.jsCpuProfilerRow!.style.height = `${maxHeight}px`;
        this.jsCpuProfilerRow!.supplier = (): Promise<Array<any>> =>
          new Promise<Array<any>>((resolve) => resolve(allCombineData));
      }
    );
  }

  private initJsCpuChart = async (): Promise<void> => {
    this.jsCpuProfilerRow = TraceRow.skeleton<JsCpuProfilerStruct>();
    this.jsCpuProfilerRow.rowParentId = this.process;
    this.jsCpuProfilerRow.rowHidden = !this.folderRow!.expansion;
    this.jsCpuProfilerRow.name = 'CpuProfiler';
    this.jsCpuProfilerRow.rowId = 'JsCpuProfiler';
    this.jsCpuProfilerRow.folder = false;
    this.jsCpuProfilerRow.rowType = TraceRow.ROW_TYPE_JS_CPU_PROFILER;
    this.jsCpuProfilerRow!.style.height = '40px';
    this.jsCpuProfilerRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    this.jsCpuProfilerRow.selectChangeHandler = this.trace.selectChangeHandler;
    this.jsCpuProfilerRow.setAttribute('children', '');
    this.jsCpuProfilerRow.focusHandler = (): void => {
      this.trace?.displayTip(
        this.jsCpuProfilerRow!,
        JsCpuProfilerStruct.hoverJsCpuProfilerStruct,
        `<span style='font-weight: bold;'>Name: </span>
        <span>${JsCpuProfilerStruct.hoverJsCpuProfilerStruct?.name || ''}</span><br>
        <span style='font-weight: bold;'>Self Time: </span>
        <span>${ns2s(JsCpuProfilerStruct.hoverJsCpuProfilerStruct?.selfTime || 0)}</span><br>
        <span style='font-weight: bold;'>Total Time: </span>
        <span>${ns2s(JsCpuProfilerStruct.hoverJsCpuProfilerStruct?.totalTime || 0)}</span><br>
        <span style='font-weight: bold;'>Url: </span>
        <span>${JsCpuProfilerStruct.hoverJsCpuProfilerStruct?.url || 0}</span>`
      );
    };
    this.jsCpuProfilerRow.onThreadHandler = (useCache): void => {
      let context = this.jsCpuProfilerRow!.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      this.jsCpuProfilerRow!.canvasSave(context);
      (renders['js-cpu-profiler'] as JsCpuProfilerRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `js-cpu-profiler`,
        },
        this.jsCpuProfilerRow!
      );
      this.jsCpuProfilerRow!.canvasRestore(context);
    };
    this.folderRow!.addChildTraceRow(this.jsCpuProfilerRow);
  };

  private getAllCombineData(
    combineData: Array<JsCpuProfilerChartFrame>,
    allCombineData: Array<JsCpuProfilerChartFrame>
  ): void {
    for (let data of combineData) {
      allCombineData.push(data);
      if (data.children.length > 0) {
        this.getAllCombineData(data.children, allCombineData);
      }
    }
  }
}
