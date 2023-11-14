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

import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabPaneCounterSampleData } from '../../../../database/SqlLite.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { dataFilterHandler, drawLines } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { TraceRow } from '../../base/TraceRow.js';
import { CpuFreqStruct } from '../../../../database/ui-worker/ProcedureWorkerFreq.js';
import { CpuState } from '../../../../database/logic-worker/ProcedureLogicWorkerCpuState.js';
import { CpuStateStruct } from '../../../../database/ui-worker/ProcedureWorkerCpuState.js';

@element('tabpane-counter-sample')
export class TabPaneCounterSample extends BaseElement {
  private counterSampleTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private loadDataInCache: boolean = true;
  private selectionParam: SelectionParam | null | undefined;
  private sampleProgressEL: LitProgressBar | null | undefined;
  private counterLoadingPage: any;
  private counterLoadingList: number[] = [];
  private counterSampleSource: any[] = [];
  private counterSortKey: string = 'counter';
  private counterSortType: number = 0;
  private systemTrace: SpSystemTrace | undefined | null;
  private _rangeRow: Array<TraceRow<any>> | undefined | null;

  set data(counterSampleValue: SelectionParam | any) {
    if (counterSampleValue == this.selectionParam) {
      return;
    }
    this.sampleProgressEL!.loading = true;
    this.counterLoadingPage.style.visibility = 'visible';
    this.selectionParam = counterSampleValue;
    // @ts-ignore
    this.counterSampleTbl!.shadowRoot?.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 25 + 'px';
    this.queryDataByDB(counterSampleValue);
  }

  set rangeTraceRow(rangeRow: Array<TraceRow<any>> | undefined) {
    this._rangeRow = rangeRow;
  }

  initElements(): void {
    this.sampleProgressEL = this.shadowRoot!.querySelector<LitProgressBar>('.progressCounter');
    this.counterLoadingPage = this.shadowRoot!.querySelector('.loadingCounter');
    this.counterSampleTbl = this.shadowRoot!.querySelector<LitTable>('#tb-counter-sample');
    this.systemTrace = document
      .querySelector('body > sp-application')?.
      shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    this.counterSampleTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.counterSortKey = evt.detail.key;
      // @ts-ignore
      this.counterSortType = evt.detail.sort;
      // @ts-ignore
      this.sortTable(evt.detail.key, evt.detail.sort);
    });

    this.counterSampleTbl!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      let path = new Path2D();
      if (this._rangeRow && this._rangeRow!.length > 0) {
        let rangeTraceRow = this._rangeRow!.filter(function (item) {
          return item.name.includes('State');
        });
        let cpuStateFilter = [];
        for (let row of rangeTraceRow!) {
          let context = row.collect ? this.systemTrace!.canvasFavoritePanelCtx! : this.systemTrace!.canvasPanelCtx!;
          cpuStateFilter.push(...row.dataListCache);
          row.canvasSave(context);
          context.clearRect(row.frame.x, row.frame.y, row.frame.width, row.frame.height);
          drawLines(context!, TraceRow.range?.xs || [], row.frame.height, this.systemTrace!.timerShaftEL!.lineColor());
          if (row.name.includes('State') && parseInt(row.name.replace(/[^\d]/g, ' ')) === data.cpu) {
            CpuFreqStruct.hoverCpuFreqStruct = undefined;
            for (let i = 0; i < cpuStateFilter!.length; i++) {
              if (
                cpuStateFilter[i].value === data.value &&
                cpuStateFilter[i].cpu === data.cpu &&
                Math.max(TraceRow.rangeSelectObject?.startNS!, cpuStateFilter[i].startTs!) <
                  Math.min(TraceRow.rangeSelectObject?.endNS!, cpuStateFilter[i].startTs! + cpuStateFilter[i].dur!)
              ) {
                CpuStateStruct.hoverStateStruct = cpuStateFilter[i];
              }
              if (cpuStateFilter[i].cpu === data.cpu) {
                CpuStateStruct.draw(context, path, cpuStateFilter[i]);
              }
            }
          } else {
            for (let i = 0; i < cpuStateFilter!.length; i++) {
              if (
                row.name.includes('State') &&
                cpuStateFilter[i].cpu !== data.cpu &&
                cpuStateFilter[i].cpu === parseInt(row.name.replace(/[^\d]/g, ' '))
              ) {
                CpuStateStruct.draw(context, path, cpuStateFilter[i]);
              }
            }
          }
          row.canvasRestore(context);
        }
      }
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.counterSampleTbl!, 25, this.counterLoadingPage, 24);
  }

  queryDataByDB(counterSampleParam: SelectionParam | any) {
    this.counterLoadingList.push(1);
    this.sampleProgressEL!.loading = true;
    this.counterLoadingPage.style.visibility = 'visible';

    getTabPaneCounterSampleData(
      counterSampleParam.leftNs + counterSampleParam.recordStartNs,
      counterSampleParam.rightNs + counterSampleParam.recordStartNs,
      counterSampleParam.cpuStateFilterIds
    ).then((result) => {
      this.counterLoadingList.splice(0, 1);
      if (this.counterLoadingList.length == 0) {
        this.sampleProgressEL!.loading = false;
        this.counterLoadingPage.style.visibility = 'hidden';
      }
      let sampleMap = new Map<any, any>();
      counterSampleParam.cpuStateFilterIds.forEach((a: number) => {
        this.getInitTime(
          result.filter((f) => f.filterId == a),
          sampleMap,
          counterSampleParam
        );
      });
      let counterSampleList: Array<any> = [];
      sampleMap.forEach((a) => {
        a.timeStr = parseFloat((a.time / 1000000.0).toFixed(6));
        counterSampleList.push(a);
      });
      this.counterSampleSource = counterSampleList;
      this.sortTable(this.counterSortKey, this.counterSortType);
    });
  }

  getInitTime(initCounterResultList: Array<any>, sampleMap: Map<any, any>, val: SelectionParam) {
    let leftNs = val.leftNs + val.recordStartNs;
    let rightNs = val.rightNs + val.recordStartNs;
    if (initCounterResultList.length == 0) return;
    let idx = initCounterResultList.findIndex((a) => a.ts >= leftNs);
    if (idx !== 0) {
      initCounterResultList = initCounterResultList.slice(
        idx == -1 ? initCounterResultList.length - 1 : idx - 1,
        initCounterResultList.length
      );
    }
    if (initCounterResultList[0].ts < leftNs && idx !== 0) initCounterResultList[0].ts = leftNs;
    initCounterResultList.forEach((item, idx) => {
      if (idx + 1 == initCounterResultList.length) {
        item.time = rightNs - item.ts;
      } else {
        item.time = initCounterResultList[idx + 1].ts - item.ts;
      }
      if (sampleMap.has(item.filterId + '-' + item.value)) {
        let obj = sampleMap.get(item.filterId + '-' + item.value);
        obj.time += item.time;
      } else {
        sampleMap.set(item.filterId + '-' + item.value, {
          ...item,
          counter: 'Cpu ' + item.cpu,
        });
      }
    });
  }

  sortTable(key: string, type: number) {
    if (type == 0) {
      this.counterSampleTbl!.recycleDataSource = this.counterSampleSource;
    } else {
      let arr = Array.from(this.counterSampleSource);
      arr.sort((sortByColumnLeftData, sortByColumnRightData): number => {
        if (key == 'timeStr') {
          if (type == 1) {
            return sortByColumnLeftData.time - sortByColumnRightData.time;
          } else {
            return sortByColumnRightData.time - sortByColumnLeftData.time;
          }
        } else if (key == 'counter') {
          if (sortByColumnLeftData.counter > sortByColumnRightData.counter) {
            return type === 2 ? -1 : 1;
          } else if (sortByColumnLeftData.counter == sortByColumnRightData.counter) {
            return 0;
          } else {
            return type === 2 ? 1 : -1;
          }
        } else if (key == 'value') {
          if (type == 1) {
            return sortByColumnLeftData.value - sortByColumnRightData.value;
          } else {
            return sortByColumnRightData.value - sortByColumnLeftData.value;
          }
        } else {
          return 0;
        }
      });
      this.counterSampleTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
        <style>
        .progressCounter{
            height: 1px;
            left: 0;
            right: 0;
            bottom: 5px;
            position: absolute;
        }
        :host{
            display: flex;
            padding: 10px 10px;
            flex-direction: column;
        }
        .loadingCounter{
            left: 0;
            right: 0;
            width:100%;
            bottom: 0;
            position: absolute;
            background:transparent;
            z-index: 999999;
        }
        .counter-sample-table{
            height: auto;
        }
        </style>
        <lit-table id="tb-counter-sample" class="counter-sample-table">
            <lit-table-column class="counter-sample-column" width="20%" order data-index="counter" key="counter" align="flex-start" title="Cpu" >
            </lit-table-column>
            <lit-table-column class="counter-sample-column" width="1fr" order data-index="timeStr" key="timeStr" align="flex-start" title="Time(ms)" >
            </lit-table-column>
            <lit-table-column class="counter-sample-column" width="1fr" order data-index="value" key="value" align="flex-start" title="Value" >
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="progressCounter"></lit-progress-bar>
        <div class="loadingCounter"></div>
        `;
  }
}
