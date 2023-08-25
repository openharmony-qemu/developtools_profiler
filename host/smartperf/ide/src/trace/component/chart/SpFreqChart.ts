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
  getCpuLimitFreq,
  getCpuLimitFreqId,
  getCpuLimitFreqMax,
  queryCpuFreq,
  queryCpuFreqData,
  queryCpuMaxFreq,
  queryCpuState,
  queryCpuStateFilter,
} from '../../database/SqlLite.js';
import { info } from '../../../log/Log.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { procedurePool } from '../../database/Procedure.js';
import { ColorUtils } from '../trace/base/ColorUtils.js';
import { CpuFreqLimitRender, CpuFreqLimitsStruct } from '../../database/ui-worker/ProcedureWorkerCpuFreqLimits.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { CpuFreqStruct, FreqRender } from '../../database/ui-worker/ProcedureWorkerFreq.js';
import { CpuStateRender, CpuStateStruct } from '../../database/ui-worker/ProcedureWorkerCpuState.js';
import { Utils } from '../trace/base/Utils.js';

export class SpFreqChart {
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let cpuFreqStartTime = new Date().getTime();
    let freqList = await queryCpuFreq();
    let cpuStateFilterIds = await queryCpuStateFilter();
    let cpuFreqLimits = await getCpuLimitFreqId();
    let cpuFreqLimitsMax = await getCpuLimitFreqMax(
      cpuFreqLimits
        .map((limit) => {
          return limit.maxFilterId;
        })
        .join(',')
    );
    info('Cpu Freq data size is: ', freqList!.length);
    let freqMaxList = await queryCpuMaxFreq();
    CpuFreqStruct.maxFreq = freqMaxList[0].maxFreq;
    let maxFreqObj = Utils.getFrequencyWithUnit(freqMaxList[0].maxFreq);
    CpuFreqStruct.maxFreq = maxFreqObj.maxFreq;
    CpuFreqStruct.maxFreqName = maxFreqObj.maxFreqName;
    for (let i = 0; i < freqList.length; i++) {
      const it = freqList[i];
      let traceRow = TraceRow.skeleton<CpuFreqStruct>();
      traceRow.rowId = `${it.filterId}`;
      traceRow.rowType = TraceRow.ROW_TYPE_CPU_FREQ;
      traceRow.rowParentId = '';
      traceRow.style.height = '40px';
      traceRow.name = `Cpu ${it.cpu} Frequency`;
      traceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      traceRow.selectChangeHandler = this.trace.selectChangeHandler;
      traceRow.supplier = () => queryCpuFreqData(it.cpu);
      traceRow.focusHandler = (ev) => {
        this.trace?.displayTip(
          traceRow,
          CpuFreqStruct.hoverCpuFreqStruct,
          `<span>${ColorUtils.formatNumberComma(CpuFreqStruct.hoverCpuFreqStruct?.value!)} kHz</span>`
        );
      };
      traceRow.onThreadHandler = (useCache) => {
        let context = traceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        traceRow.canvasSave(context);
        (renders['freq'] as FreqRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `freq${it.cpu}`,
          },
          traceRow
        );
        traceRow.canvasRestore(context);
      };
      this.trace.rowsEL?.appendChild(traceRow);
    }
    let heights = [4, 12, 21, 30];
    for (let it of cpuStateFilterIds) {
      let cpuStateRow = TraceRow.skeleton<CpuStateStruct>();
      cpuStateRow.rowId = `${it.filterId}`;
      cpuStateRow.rowType = TraceRow.ROW_TYPE_CPU_STATE;
      cpuStateRow.rowParentId = '';
      cpuStateRow.style.height = '40px';
      cpuStateRow.name = `Cpu ${it.cpu} State`;
      cpuStateRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      cpuStateRow.selectChangeHandler = this.trace.selectChangeHandler;
      cpuStateRow.isHover = true;
      cpuStateRow.supplier = () =>
        queryCpuState(it.filterId).then((res) => {
          res.forEach((r) => {
            r.height = heights[it.value];
            r.cpu = it.cpu;
          });
          return res;
        });
      cpuStateRow.focusHandler = (ev) => {
        this.trace.displayTip(
          cpuStateRow,
          CpuStateStruct.hoverStateStruct,
          `<span>State: ${CpuStateStruct.hoverStateStruct?.value}</span>`
        );
      };
      cpuStateRow.onThreadHandler = (useCache: boolean) => {
        let context = cpuStateRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        cpuStateRow.canvasSave(context);
        (renders['cpu-state'] as CpuStateRender).renderMainThread(
          {
            cpuStateContext: context,
            useCache: useCache,
            type: `cpu-state-${it.cpu}`,
            cpu: it.cpu,
          },
          cpuStateRow
        );
        cpuStateRow.canvasRestore(context);
      };
      this.trace.rowsEL?.appendChild(cpuStateRow);
    }
    let durTime = new Date().getTime() - cpuFreqStartTime;
    info('The time to load the CpuFreq data is: ', durTime);
    for (let limit of cpuFreqLimits) {
      let findMax = Utils.getFrequencyWithUnit(
        cpuFreqLimitsMax.find((maxLimit) => {
          return maxLimit.filterId == limit.maxFilterId;
        })?.maxValue || 0
      );
      let cpuFreqLimitRow = TraceRow.skeleton<CpuFreqLimitsStruct>();
      cpuFreqLimitRow.rowId = `${limit.cpu}`;
      cpuFreqLimitRow.rowType = TraceRow.ROW_TYPE_CPU_FREQ_LIMIT;
      cpuFreqLimitRow.rowParentId = '';
      cpuFreqLimitRow.style.height = '40px';
      cpuFreqLimitRow.name = `Cpu ${limit.cpu} Freq Limit`;
      cpuFreqLimitRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      cpuFreqLimitRow.selectChangeHandler = this.trace.selectChangeHandler;
      cpuFreqLimitRow.isHover = true;
      cpuFreqLimitRow.supplier = () => getCpuLimitFreq(limit.maxFilterId, limit.minFilterId, limit.cpu);
      cpuFreqLimitRow.focusHandler = (ev) => {
        this.trace.displayTip(
          cpuFreqLimitRow,
          CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct,
          `<span>Max Freq: ${ColorUtils.formatNumberComma(
            CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct?.max || 0
          )} kHz</span><span>Min Freq: ${ColorUtils.formatNumberComma(
            CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct?.min || 0
          )} kHz</span>`
        );
      };
      cpuFreqLimitRow.onThreadHandler = (useCache: boolean) => {
        let context = cpuFreqLimitRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        cpuFreqLimitRow.canvasSave(context);
        (renders['cpu-limit-freq'] as CpuFreqLimitRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `cpu-limit-freq-${limit.cpu}`,
            cpu: limit.cpu,
            maxFreq: findMax?.maxFreq || 0,
            maxFreqName: findMax?.maxFreqName || '',
          },
          cpuFreqLimitRow
        );
        cpuFreqLimitRow.canvasRestore(context);
      };
      this.trace.rowsEL?.appendChild(cpuFreqLimitRow);
    }
  }
}

export class CpuFreqRowLimit {
  cpu: number = 0;
  maxFilterId: number = 0;
  minFilterId: number = 0;
}
