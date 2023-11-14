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
import { queryClockData, queryClockFrequency, queryClockState, queryScreenState } from '../../database/SqlLite.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { info } from '../../../log/Log.js';
import { ClockRender, ClockStruct } from '../../database/ui-worker/ProcedureWorkerClock.js';
import { ColorUtils } from '../trace/base/ColorUtils.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { Utils } from '../trace/base/Utils.js';

export class SpClockChart {
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let folder = await this.initFolder();
    await this.initData(folder);
  }

  async initData(folder: TraceRow<any>) {
    let clockStartTime = new Date().getTime();
    let clockList = await queryClockData();
    if (clockList.length == 0) {
      return;
    }
    info('clockList data size is: ', clockList!.length);
    this.trace.rowsEL?.appendChild(folder);
    ClockStruct.maxValue = clockList.map((item) => item.num).reduce((a, b) => Math.max(a, b));
    for (let i = 0; i < clockList.length; i++) {
      const it = clockList[i];
      let maxValue = 0;
      let traceRow = TraceRow.skeleton<ClockStruct>();
      let isState = it.name.endsWith(' State');
      let isScreenState = it.name.endsWith('ScreenState');
      traceRow.rowId = it.name;
      traceRow.rowType = TraceRow.ROW_TYPE_CLOCK;
      traceRow.rowParentId = folder.rowId;
      traceRow.style.height = '40px';
      traceRow.name = it.name;
      traceRow.rowHidden = !folder.expansion;
      traceRow.setAttribute('children', '');
      traceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      traceRow.selectChangeHandler = this.trace.selectChangeHandler;
      traceRow.supplier = () => {
        let promiseData = null;
        if (it.name.endsWith(' Frequency')) {
          promiseData = queryClockFrequency(it.srcname);
        } else if (isState) {
          promiseData = queryClockState(it.srcname);
        } else if (isScreenState) {
          promiseData = queryScreenState();
        }
        if (promiseData == null) {
          return new Promise<Array<any>>((resolve) => resolve([]));
        } else {
          return promiseData.then((resultClock) => {
            for (let j = 0; j < resultClock.length; j++) {
              if (!isState) {
                if (j == resultClock.length - 1) {
                  resultClock[j].dur = (TraceRow.range?.totalNS || 0) - (resultClock[j].startNS || 0);
                } else {
                  resultClock[j].dur = (resultClock[j + 1].startNS || 0) - (resultClock[j].startNS || 0);
                }
              }
              if ((resultClock[j].value || 0) > maxValue) {
                maxValue = resultClock[j].value || 0;
              }
              if (j > 0) {
                resultClock[j].delta = (resultClock[j].value || 0) - (resultClock[j - 1].value || 0);
              } else {
                resultClock[j].delta = 0;
              }
            }
            return resultClock;
          });
        }
      };
      traceRow.focusHandler = (ev) => {
        this.trace?.displayTip(
          traceRow,
          ClockStruct.hoverClockStruct,
          `<span>${ColorUtils.formatNumberComma(ClockStruct.hoverClockStruct?.value!)}</span>`
        );
      };
      traceRow.findHoverStruct = () => {
        ClockStruct.hoverClockStruct = traceRow.getHoverStruct();
      };
      traceRow.onThreadHandler = (useCache) => {
        let context: CanvasRenderingContext2D;
        if (traceRow.currentContext) {
          context = traceRow.currentContext;
        } else {
          context = traceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        }
        traceRow.canvasSave(context);
        (renders['clock'] as ClockRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: it.name,
            maxValue: maxValue === 0 ? 1 : maxValue,
            index: i,
            maxName:
              isState || isScreenState ? maxValue.toString() : Utils.getFrequencyWithUnit(maxValue / 1000).maxFreqName,
          },
          traceRow
        );
        traceRow.canvasRestore(context);
      };
      folder.addChildTraceRow(traceRow);
    }
    let durTime = new Date().getTime() - clockStartTime;
    info('The time to load the ClockData is: ', durTime);
  }

  async initFolder(): Promise<TraceRow<any>> {
    let clockFolder = TraceRow.skeleton();
    clockFolder.rowId = `Clocks`;
    clockFolder.index = 0;
    clockFolder.rowType = TraceRow.ROW_TYPE_CLOCK_GROUP;
    clockFolder.rowParentId = '';
    clockFolder.style.height = '40px';
    clockFolder.folder = true;
    clockFolder.name = `Clocks`; /* & I/O Latency */
    clockFolder.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    clockFolder.selectChangeHandler = this.trace.selectChangeHandler;
    clockFolder.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
    clockFolder.onThreadHandler = (useCache) => {
      clockFolder.canvasSave(this.trace.canvasPanelCtx!);
      if (clockFolder.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, clockFolder.frame.width, clockFolder.frame.height);
      } else {
        (renders['empty'] as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: ``,
          },
          clockFolder
        );
      }
      clockFolder.canvasRestore(this.trace.canvasPanelCtx!);
    };
    return clockFolder;
  }
}
