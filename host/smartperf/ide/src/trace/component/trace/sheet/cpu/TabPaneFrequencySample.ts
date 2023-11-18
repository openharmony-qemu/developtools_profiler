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
import { getTabPaneFrequencySampleData, getTabPaneCounterSampleData } from '../../../../database/SqlLite.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { resizeObserver } from '../SheetUtils.js';
import { CpuFreqStruct } from '../../../../database/ui-worker/ProcedureWorkerFreq.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { TraceRow } from '../../base/TraceRow.js';
import {
  drawLines,
} from '../../../../database/ui-worker/ProcedureWorkerCommon.js';

@element('tabpane-frequency-sample')
export class TabPaneFrequencySample extends BaseElement {
  private frequencySampleTbl: LitTable | null | undefined;
  private selectionParam: SelectionParam | null | undefined;
  private frequencyLoadingPage: any;
  private frequencySampleSource: any[] = [];
  private frequencySampleSortKey: string = 'counter';
  private frequencySampleSortType: number = 0;
  private systemTrace: SpSystemTrace | undefined | null;
  private _rangeRow: Array<TraceRow<any>> | undefined | null;
  private frequencySampleClickType: boolean = false;
  private busyTimeLoadingHide: boolean = false;
  private freqBusyDataList: Array<any> = [];
  private worker: Worker | undefined;
  private freqResult: Array<any> = [];

  set data(frequencySampleValue: SelectionParam | any) {
    if (frequencySampleValue == this.selectionParam) {
      return;
    }
    this.selectionParam = frequencySampleValue;
    // @ts-ignore
    this.frequencySampleTbl!.shadowRoot?.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 25 + 'px';
    this.queryDataByDB(frequencySampleValue);
  }

  set rangeTraceRow(rangeRow: Array<TraceRow<any>> | undefined) {
    this._rangeRow = rangeRow;
  }

  initElements(): void {
    this.frequencyLoadingPage = this.shadowRoot!.querySelector('.loadingFre');
    this.frequencySampleTbl = this.shadowRoot!.querySelector<LitTable>('#tb-states');
    this.systemTrace = document
      .querySelector('body > sp-application')
      ?.shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    this.frequencySampleTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.frequencySampleSortKey = evt.detail.key;
      // @ts-ignore
      this.frequencySampleSortType = evt.detail.sort;
      // @ts-ignore
      this.sortTable(evt.detail.key, evt.detail.sort);
    });
    this.frequencySampleTbl!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (this._rangeRow && this._rangeRow!.length > 0) {
        let rangeTraceRow = this._rangeRow!.filter(function (item) {
          return item.name.includes('Frequency');
        });
        let freqFilter = [];
        for (let row of rangeTraceRow!) {
          let context = row.collect ? this.systemTrace!.canvasFavoritePanelCtx! : this.systemTrace!.canvasPanelCtx!;
          freqFilter.push(...row.dataListCache);
          row.canvasSave(context);
          context.clearRect(row.frame.x, row.frame.y, row.frame.width, row.frame.height);
          drawLines(context!, TraceRow.range?.xs || [], row.frame.height, this.systemTrace!.timerShaftEL!.lineColor());
          if (row.name.includes('Frequency') && parseInt(row.name.replace(/[^\d]/g, ' ')) === data.cpu) {
            CpuFreqStruct.hoverCpuFreqStruct = undefined;
            for (let i = 0; i < freqFilter!.length; i++) {
              if (
                freqFilter[i].value === data.value &&
                freqFilter[i].cpu === data.cpu &&
                Math.max(TraceRow.rangeSelectObject?.startNS!, freqFilter[i].startNS!) <
                  Math.min(TraceRow.rangeSelectObject?.endNS!, freqFilter[i].startNS! + freqFilter[i].dur!)
              ) {
                CpuFreqStruct.hoverCpuFreqStruct = freqFilter[i];
              }
              if (freqFilter[i].cpu === data.cpu) {
                CpuFreqStruct.draw(context, freqFilter[i]);
              }
            }
          } else {
            for (let i = 0; i < freqFilter!.length; i++) {
              if (
                row.name.includes('Frequency') &&
                freqFilter[i].cpu !== data.cpu &&
                freqFilter[i].cpu === parseInt(row.name.replace(/[^\d]/g, ' '))
              ) {
                CpuFreqStruct.draw(context, freqFilter[i]);
              }
            }
          }
          let s = CpuFreqStruct.maxFreqName;
          let textMetrics = context.measureText(s);
          context.globalAlpha = 0.8;
          context.fillStyle = '#f0f0f0';
          context.fillRect(0, 5, textMetrics.width + 8, 18);
          context.globalAlpha = 1;
          context.fillStyle = '#333';
          context.textBaseline = 'middle';
          context.fillText(s, 4, 5 + 9);
          row.canvasRestore(context);
        }
      }
    });
    this.frequencySampleTbl!.addEventListener('button-click', (evt) => {
      //@ts-ignore
      this.frequencySampleClickKey = evt.detail.key;
      this.frequencySampleClickType = !this.frequencySampleClickType;
      //@ts-ignore
      this.handleClick(evt.detail.key, this.frequencySampleClickType);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.frequencySampleTbl!, 25, this.frequencyLoadingPage, 24);
  }

  async queryDataByDB(frqSampleParam: SelectionParam | any) {
    let sampleMap = new Map<any, any>();
    let frqSampleList = new Array();
    let stateFiliterIds: Array<any> = [];
    let cpuFiliterOrder: Array<any> = [];
    this.frequencySampleTbl!.loading = true;
    if (this.frequencySampleClickType) this.frequencySampleClickType = !this.frequencySampleClickType;
    if (this.busyTimeLoadingHide) this.busyTimeLoadingHide = !this.busyTimeLoadingHide;
    let result = await getTabPaneFrequencySampleData(
      frqSampleParam.leftNs + frqSampleParam.recordStartNs,
      frqSampleParam.rightNs + frqSampleParam.recordStartNs,
      frqSampleParam.cpuFreqFilterIds
    );
    this.freqResult = result;
    frqSampleParam.cpuFreqFilterIds.forEach((a: number) => {
      this.getInitTime(
        result.filter((f) => f.filterId == a),
        sampleMap,
        frqSampleParam
      );
    });
    sampleMap.forEach((a) => {
      a.timeStr = parseFloat((a.time / 1000000.0).toFixed(6));
      frqSampleList.push(a);
    });
    this.frequencySampleSource = frqSampleList;
    this.frequencySampleTbl!.loading = false;
    this.sortTable(this.frequencySampleSortKey, this.frequencySampleSortType);
    //找出框选的cpu fre所对应的cpu state
    this.freqBusyDataList = [];
    if (!frqSampleParam.cpuStateRowsId.length) {
      sampleMap.forEach((value: any) => {
        value.busyTime = 'NULL';
        this.freqBusyDataList.push(value);
      });
    } else {
      frqSampleParam.cpuFreqFilterNames.forEach((item: string) => {
        let cpuStateIds = frqSampleParam.cpuStateRowsId.filter(
          (it: any) => it.cpu == item.replace(/[^\d]/g, ' ').trim()
        );
        stateFiliterIds.push(cpuStateIds[0].filterId);
        cpuFiliterOrder.push(cpuStateIds[0].cpu);
      });
      let res = await getTabPaneCounterSampleData(
        frqSampleParam.leftNs + frqSampleParam.recordStartNs,
        frqSampleParam.rightNs + frqSampleParam.recordStartNs,
        stateFiliterIds
      );
      //开启一个线程计算busyTime
      this.worker = new Worker('trace/database/StateBusyTimeWorker.js');
      let msg = {
        frqSampleParam,
        result,
        sampleMap,
        res,
        cpuFiliterOrder,
      };
      this.worker!.postMessage(msg);
      this.worker!.onmessage = (event: MessageEvent) => {
        sampleMap = event.data;
        sampleMap.forEach((value) => {
          this.freqBusyDataList.push(value);
        });
        this.busyTimeLoadingHide = true;
        //当busyTimebutton的状态为true但busyTime的计算未完成时
        if (this.frequencySampleClickType) {
          this.handleClick(this.frequencySampleSortKey, this.frequencySampleClickType);
        }
        this.worker!.terminate();
      };
    }
  }

  getInitTime(initFreqResult: Array<any>, sampleMap: Map<any, any>, selectionParam: SelectionParam) {
    let leftStartNs = selectionParam.leftNs + selectionParam.recordStartNs;
    let rightEndNs = selectionParam.rightNs + selectionParam.recordStartNs;
    if (initFreqResult.length == 0) return;
    let includeData = initFreqResult.findIndex((a) => a.ts >= leftStartNs);
    if (includeData !== 0) {
      initFreqResult = initFreqResult.slice(
        includeData == -1 ? initFreqResult.length - 1 : includeData - 1,
        initFreqResult.length
      );
    }
    if (initFreqResult[0].ts < leftStartNs && includeData !== 0) initFreqResult[0].ts = leftStartNs;
    initFreqResult.forEach((item, idx) => {
      if (idx + 1 == initFreqResult.length) {
        item.time = rightEndNs - item.ts;
      } else {
        item.time = initFreqResult[idx + 1].ts - item.ts;
      }
      if (sampleMap.has(item.filterId + '-' + item.value)) {
        let obj = sampleMap.get(item.filterId + '-' + item.value);
        obj.time += item.time;
      } else {
        sampleMap.set(item.filterId + '-' + item.value, {
          ...item,
          counter: 'Cpu ' + item.cpu,
          valueStr: ColorUtils.formatNumberComma(item.value),
          busyTimeStr: '-',
          busyTime: 0,
        });
      }
    });
  }

  //点击按钮控制busyTime显示与否
  handleClick(key: string, type: boolean) {
    let res = new Array();
    if (this.freqResult.length == 0) {
      return;
    }
    //当busyTime的值计算完毕后进入if判断
    if (this.busyTimeLoadingHide) {
      this.busyTimeLoadingHide = false;
      this.frequencySampleSource = this.freqBusyDataList;
      this.sortTable(this.frequencySampleSortKey, this.frequencySampleSortType);
    }
    this.frequencySampleTbl!.loading = this.freqBusyDataList.length > 0 ? false : true;
    if (this.freqBusyDataList.length > 0) {
      this.frequencySampleTbl!.recycleDataSource.forEach((value) => {
        value.busyTimeStr = type ? value.busyTime : '-';
        res.push(value);
      });
      this.frequencySampleTbl!.recycleDataSource = res;
    }
  }

  sortTable(key: string, type: number) {
    if (type == 0) {
      this.frequencySampleTbl!.recycleDataSource = this.frequencySampleSource;
    } else {
      let arr = Array.from(this.frequencySampleSource);
      arr.sort((frequencySampleLeftData, frequencySampleRightData): number => {
        if (key == 'timeStr') {
          if (type == 1) {
            return frequencySampleLeftData.time - frequencySampleRightData.time;
          } else {
            return frequencySampleRightData.time - frequencySampleLeftData.time;
          }
        } else if (key == 'counter') {
          if (frequencySampleLeftData.counter > frequencySampleRightData.counter) {
            return type === 2 ? -1 : 1;
          } else if (frequencySampleLeftData.counter == frequencySampleRightData.counter) {
            return 0;
          } else {
            return type === 2 ? 1 : -1;
          }
        } else if (key == 'valueStr') {
          if (type == 1) {
            return frequencySampleLeftData.value - frequencySampleRightData.value;
          } else {
            return frequencySampleRightData.value - frequencySampleLeftData.value;
          }
        } else if (key == 'busyTimeStr') {
          if (type == 1) {
            return frequencySampleLeftData.busyTimeStr - frequencySampleRightData.busyTimeStr;
          } else {
            return frequencySampleRightData.busyTimeStr - frequencySampleLeftData.busyTimeStr;
          }
        } else {
          return 0;
        }
      });
      this.frequencySampleTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
        <style>
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <lit-table id="tb-states" style="height: auto" >
            <lit-table-column class="freq-sample-column" width="20%" title="Cpu" data-index="counter" key="counter" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Time(ms)" data-index="timeStr" key="timeStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Value(kHz)" data-index="valueStr" key="valueStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" data-index="busyTimeStr" key="busyTimeStr" align="flex-start" order button>
            </lit-table-column>
        </lit-table>
        `;
  }
}
