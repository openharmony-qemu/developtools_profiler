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
  queryBootTime,
  queryHeapGroupByEvent,
  queryNativeHookProcess,
  queryNativeHookStatisticsCount,
  queryNativeMemoryRealTime,
} from '../../database/SqlLite.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { info } from '../../../log/Log.js';
import { procedurePool } from '../../database/Procedure.js';
import { NativeEventHeap } from '../../bean/NativeHook.js';
import { HeapRender, HeapStruct } from '../../database/ui-worker/ProcedureWorkerHeap.js';
import { Utils } from '../trace/base/Utils.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';

export class SpNativeMemoryChart {
  static EVENT_HEAP: Array<NativeEventHeap> = [];
  static REAL_TIME_DIF: number = 0;
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  initChart = async () => {
    let time = new Date().getTime();
    let nativeMemoryType = 'native_hook';
    let nmsCount = await queryNativeHookStatisticsCount();
    if (nmsCount && nmsCount[0] && (nmsCount[0] as any).num > 0) {
      nativeMemoryType = 'native_hook_statistic';
    }
    let nativeProcess = await queryNativeHookProcess(nativeMemoryType);
    info('NativeHook Process data size is: ', nativeProcess!.length);
    if (nativeProcess.length == 0) {
      return;
    }
    await this.initNativeMemory();
    SpNativeMemoryChart.EVENT_HEAP = await queryHeapGroupByEvent(nativeMemoryType);
    let nativeRow = TraceRow.skeleton();
    let process = '';
    if (nativeProcess.length > 0) {
      process = ` ${nativeProcess[0].pid}`;
    }
    nativeRow.rowId = `native-memory`;
    nativeRow.index = 0;
    nativeRow.rowType = TraceRow.ROW_TYPE_NATIVE_MEMORY;
    nativeRow.drawType = 0;
    nativeRow.style.height = '40px';
    nativeRow.rowParentId = '';
    nativeRow.folder = true;
    nativeRow.name = `Native Memory` + process;
    nativeRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    nativeRow.selectChangeHandler = this.trace.selectChangeHandler;
    nativeRow.rowSetting = 'enable';
    nativeRow.rowSettingPopoverDirection = 'bottomLeft';
    nativeRow.rowSettingList = [
      {
        key: '0',
        title: 'Current Bytes',
        checked: true,
      },
      {
        key: '1',
        title: 'Native Memory Density'
      }
    ]
    nativeRow.onRowSettingChangeHandler = (value) => {
      nativeRow.childrenList.forEach((row) => (row.drawType = parseInt(value[0])));
      this.trace.favoriteRowsEL?.querySelectorAll<TraceRow<any>>(`trace-row[row-type='heap']`).forEach((it) => {
        it.drawType = parseInt(value[0]);
      });
      this.trace.refreshCanvas(false);
    }
    nativeRow.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
    nativeRow.onThreadHandler = (useCache) => {
      nativeRow.canvasSave(this.trace.canvasPanelCtx!);
      if (nativeRow.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, nativeRow.frame.width, nativeRow.frame.height);
      } else {
        (renders['empty'] as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: ``,
          },
          nativeRow
        );
      }
      nativeRow.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.trace.rowsEL?.appendChild(nativeRow);
    /**
     * 添加heap信息
     */
    let native_memory = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
    for (let i = 0; i < native_memory.length; i++) {
      let nm = native_memory[i];
      let allHeapRow = TraceRow.skeleton<HeapStruct>();
      allHeapRow.index = i;
      allHeapRow.rowParentId = `native-memory`;
      allHeapRow.rowHidden = !nativeRow.expansion;
      allHeapRow.style.height = '40px';
      allHeapRow.name = nm;
      allHeapRow.rowId = nm;
      allHeapRow.drawType = 0;
      allHeapRow.isHover = true;
      allHeapRow.folder = false;
      allHeapRow.rowType = TraceRow.ROW_TYPE_HEAP;
      allHeapRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      allHeapRow.selectChangeHandler = this.trace.selectChangeHandler;
      allHeapRow.setAttribute('heap-type', nativeMemoryType);
      allHeapRow.setAttribute('children', '');
      allHeapRow.focusHandler = () => {
        let tip = '';
        if (HeapStruct.hoverHeapStruct) {
          if (allHeapRow.drawType === 1) {
            tip = `<span>${HeapStruct.hoverHeapStruct.density}</span>`;
          } else {
            tip = `<span>${Utils.getByteWithUnit(HeapStruct.hoverHeapStruct.heapsize!)}</span>`;
          }
        }
        this.trace?.displayTip(allHeapRow, HeapStruct.hoverHeapStruct, tip);
      };
      allHeapRow.supplier = () => {
        return nativeMemoryType === 'native_hook'
          ? this.getNativeMemoryDataByChartType(i, allHeapRow.drawType)
          : this.getNativeMemoryStatisticByChartType(i - 1);
      };
      allHeapRow.onThreadHandler = (useCache) => {
        let context = allHeapRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        allHeapRow.canvasSave(context);
        (renders['heap'] as HeapRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `heap`,
          },
          allHeapRow
        );
        allHeapRow.canvasRestore(context);
      };
      nativeRow.addChildTraceRow(allHeapRow);
    }
    let durTime = new Date().getTime() - time;
    info('The time to load the Native Memory data is: ', durTime);
  };

  getNativeMemoryStatisticByChartType = async (chartType: number): Promise<Array<HeapStruct>> => {
    let nmStatisticArray: Array<HeapStruct> = [];
    await new Promise<Array<HeapStruct>>((resolve, reject) => {
      procedurePool.submitWithName(
        'logic1',
        'native-memory-queryNativeHookStatistic',
        { type: chartType, totalNS: TraceRow.range?.totalNS! },
        undefined,
        (res: any) => {
          nmStatisticArray = nmStatisticArray.concat(res.data);
          res.data = null;
          if (res.tag == 'end') {
            resolve(nmStatisticArray);
          }
        }
      );
    });
    return nmStatisticArray;
  };

  getNativeMemoryDataByChartType = async (nativeMemoryType: number, chartType: number): Promise<Array<HeapStruct>> => {
    let args = new Map<string, any>();
    args.set('nativeMemoryType', nativeMemoryType);
    args.set('chartType', chartType);
    args.set('totalNS', TraceRow.range?.totalNS!);
    args.set('actionType', 'memory-chart');
    let nmArray: Array<HeapStruct> = [];
    await new Promise<Array<HeapStruct>>((resolve, reject) => {
      procedurePool.submitWithName('logic1', 'native-memory-chart-action', args, undefined, (res: any) => {
        nmArray = nmArray.concat(res.data);
        res.data = null;
        if (res.tag == 'end') {
          resolve(nmArray);
        }
      });
    });
    return nmArray;
  };

  initNativeMemory = async () => {
    let time = new Date().getTime();
    let isRealtime = false;
    let realTimeDif = 0;
    SpNativeMemoryChart.REAL_TIME_DIF = 0;
    let queryTime = await queryNativeMemoryRealTime();
    let bootTime = await queryBootTime();
    if (queryTime.length > 0) {
      isRealtime = queryTime[0].clock_name == 'realtime';
    }
    if (bootTime.length > 0 && isRealtime) {
      realTimeDif = queryTime[0].ts - bootTime[0].ts;
      SpNativeMemoryChart.REAL_TIME_DIF = realTimeDif;
    }
    await new Promise<any>((resolve, reject) => {
      procedurePool.submitWithName(
        'logic1',
        'native-memory-init',
        { isRealtime, realTimeDif, dataDict: SpSystemTrace.DATA_DICT },
        undefined,
        (res: any) => {
          resolve(res);
        }
      );
    });
    let durTime = new Date().getTime() - time;
    info('The time to init the native memory data is: ', durTime);
  };
}
