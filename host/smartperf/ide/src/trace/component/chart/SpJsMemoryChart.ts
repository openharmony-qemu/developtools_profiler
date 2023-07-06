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
import { queryHeapFile, queryTraceRange } from '../../database/SqlLite.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { info } from '../../../log/Log.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { HeapTimelineRender, HeapTimelineStruct } from '../../database/ui-worker/ProcedureWorkerHeapTimeline.js';
import { HeapDataInterface, ParseListener } from '../../../js-heap/HeapDataInterface.js';
import { LoadDatabase } from '../../../js-heap/LoadDatabase.js';
import { FileInfo } from '../../../js-heap/model/UiStruct.js';
import { HeapSnapshotRender, HeapSnapshotStruct } from '../../database/ui-worker/ProcedureWorkerHeapSnapshot.js';

export class SpJsMemoryChart implements ParseListener {
  private trace: SpSystemTrace;
  private loadJsDatabase: LoadDatabase;
  static file: any;
  constructor(trace: SpSystemTrace) {
    this.trace = trace;
    this.loadJsDatabase = LoadDatabase.getInstance();
  }
  async parseDone(fileModule: Array<FileInfo>): Promise<void> {
    if (fileModule.length > 0) {
      let jsHeapRow = TraceRow.skeleton();
      let process = '';
      let heapFile = HeapDataInterface.getInstance().getFileStructs();
      process = String(heapFile[0].pid);
      jsHeapRow.rowId = `js-memory`;
      jsHeapRow.index = 0;
      jsHeapRow.rowType = TraceRow.ROW_TYPE_JS_MEMORY;
      jsHeapRow.drawType = 0;
      jsHeapRow.style.height = '40px';
      jsHeapRow.rowParentId = '';
      jsHeapRow.folder = true;
      jsHeapRow.name = `Js Memory ` + process;
      jsHeapRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      jsHeapRow.selectChangeHandler = this.trace.selectChangeHandler;
      jsHeapRow.onDrawTypeChangeHandler = (type) => {};
      jsHeapRow.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
      jsHeapRow.onThreadHandler = (useCache) => {
        jsHeapRow.canvasSave(this.trace.canvasPanelCtx!);
        if (jsHeapRow.expansion) {
          this.trace.canvasPanelCtx?.clearRect(0, 0, jsHeapRow.frame.width, jsHeapRow.frame.height);
        } else {
          (renders['empty'] as EmptyRender).renderMainThread(
            {
              context: this.trace.canvasPanelCtx,
              useCache: useCache,
              type: ``,
            },
            jsHeapRow
          );
        }
        jsHeapRow.canvasRestore(this.trace.canvasPanelCtx!);
      };
      this.trace.rowsEL?.appendChild(jsHeapRow);
      let file = heapFile[0];
      SpJsMemoryChart.file = file;
      if (file.name.includes('Timeline')) {
        let samples = HeapDataInterface.getInstance().getSamples(file.id);
        let heapTimelineRow = TraceRow.skeleton<HeapTimelineStruct>();
        heapTimelineRow.index = 0;
        heapTimelineRow.rowParentId = `js-memory`;
        heapTimelineRow.rowHidden = !jsHeapRow.expansion;
        heapTimelineRow.style.height = '40px';
        heapTimelineRow.name = `Heaptimeline`;
        heapTimelineRow.rowId = `heaptimeline` + file.id;
        heapTimelineRow.drawType = 0;
        heapTimelineRow.isHover = true;
        heapTimelineRow.folder = false;
        heapTimelineRow.rowType = TraceRow.ROW_TYPE_HEAP_TIMELINE;
        heapTimelineRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        heapTimelineRow.selectChangeHandler = this.trace.selectChangeHandler;
        heapTimelineRow.setAttribute('children', '');
        heapTimelineRow.focusHandler = () => {};
        heapTimelineRow.supplier = () => new Promise<any>((resolve) => resolve(samples));
        heapTimelineRow.onThreadHandler = (useCache) => {
          let context = heapTimelineRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
          heapTimelineRow.canvasSave(context);
          (renders['heapTimeline'] as HeapTimelineRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              type: `heapTimeline`,
              samples: samples,
            },
            heapTimelineRow
          );
          heapTimelineRow.canvasRestore(context);
        };
        jsHeapRow.addChildTraceRow(heapTimelineRow);
      } else {
        let traceRange = await queryTraceRange();
        let heapSnapshotRow = TraceRow.skeleton<HeapSnapshotStruct>();
        heapSnapshotRow.rowParentId = `js-memory`;
        heapSnapshotRow.rowHidden = !jsHeapRow.expansion;
        heapSnapshotRow.style.height = '40px';
        heapSnapshotRow.name = `Heapsnapshot`;
        heapSnapshotRow.rowId = `heapsnapshot`;
        heapSnapshotRow.isHover = true;
        heapSnapshotRow.folder = false;
        heapSnapshotRow.rowType = TraceRow.ROW_TYPE_HEAP_SNAPSHOT;
        heapSnapshotRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        heapSnapshotRow.selectChangeHandler = this.trace.selectChangeHandler;
        heapSnapshotRow.setAttribute('children', '');
        heapSnapshotRow.focusHandler = () => {};
        heapSnapshotRow.supplier = () => new Promise<Array<any>>((resolve) => resolve(heapFile));
        heapSnapshotRow.onThreadHandler = (useCache) => {
          let context = heapSnapshotRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
          heapSnapshotRow.canvasSave(context);
          (renders['heapSnapshot'] as HeapSnapshotRender).renderMainThread(
            {
              context: context,
              useCache: useCache,
              type: `heapSnapshot`,
              traceRange: traceRange,
            },
            heapSnapshotRow
          );
          heapSnapshotRow.canvasRestore(context);
        };
        jsHeapRow.addChildTraceRow(heapSnapshotRow);
      }
    }
  }

  process(info: string, process: number): void {}

  initChart = async () => {
    let time = new Date().getTime();
    await this.loadJsDatabase.loadDatabase(this);
    let durTime = new Date().getTime() - time;
    info('The time to load the js Memory data is: ', durTime);
  };
}
