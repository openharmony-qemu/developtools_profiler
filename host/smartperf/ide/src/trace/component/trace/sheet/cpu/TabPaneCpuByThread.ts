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
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabCpuByThread } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-cpu-thread')
export class TabPaneCpuByThread extends BaseElement {
  private cpuByThreadTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private cpuByThreadSource: Array<SelectionData> = [];
  private currentSelectionParam: SelectionParam | undefined;
  private pubColumns = `
            <lit-table-column order width="250px" title="Process" data-index="process" key="process" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="120px" title="PID" data-index="pid" key="pid" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="250px" title="Thread" data-index="thread" key="thread" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="120px" title="TID" data-index="tid" key="tid" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="200px" title="Wall duration(ms)" data-index="wallDuration" key="wallDuration" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="200px" title="Avg Wall duration(ms)" data-index="avgDuration" key="avgDuration" align="flex-start" order >
            </lit-table-column>
            <lit-table-column order width="120px" title="Occurrences" data-index="occurrences" key="occurrences" align="flex-start" order >
            </lit-table-column>
    `;

  set data(cpuByThreadValue: SelectionParam | any) {
    if (this.currentSelectionParam === cpuByThreadValue) {
      return;
    }
    this.currentSelectionParam = cpuByThreadValue;
    this.cpuByThreadTbl!.innerHTML = this.getTableColumns(cpuByThreadValue.cpus);
    this.range!.textContent =
      'Selected range: ' +
      parseFloat(((cpuByThreadValue.rightNs - cpuByThreadValue.leftNs) / 1000000.0).toFixed(5)) +
      ' ms';
    this.cpuByThreadTbl!.loading = true;
    getTabCpuByThread(cpuByThreadValue.cpus, cpuByThreadValue.leftNs, cpuByThreadValue.rightNs).then((result) => {
      this.cpuByThreadTbl!.loading = false;
      if (result != null && result.length > 0) {
        log('getTabCpuByThread size :' + result.length);
        let sumWall = 0.0;
        let sumOcc = 0;
        let map: Map<string, any> = new Map<string, any>();
        for (let e of result) {
          sumWall += e.wallDuration;
          sumOcc += e.occurrences;
          if (map.has(`${e.tid}`)) {
            let thread = map.get(`${e.tid}`)!;
            thread.wallDuration += e.wallDuration;
            thread.occurrences += e.occurrences;
            thread[`cpu${e.cpu}`] = e.wallDuration || 0;
            thread[`cpu${e.cpu}TimeStr`] = getProbablyTime(e.wallDuration || 0);
            thread[`cpu${e.cpu}Ratio`] = (
              (100.0 * (e.wallDuration || 0)) /
              (cpuByThreadValue.rightNs - cpuByThreadValue.leftNs)
            ).toFixed(2);
          } else {
            let process = Utils.PROCESS_MAP.get(e.pid);
            let thread = Utils.THREAD_MAP.get(e.tid);
            let cpuByThreadObject: any = {
              tid: e.tid,
              pid: e.pid,
              thread: thread == null || thread.length == 0 ? '[NULL]' : thread,
              process: process == null || process.length == 0 ? '[NULL]' : process,
              wallDuration: e.wallDuration || 0,
              occurrences: e.occurrences || 0,
              avgDuration: 0,
            };
            for (let i of cpuByThreadValue.cpus) {
              cpuByThreadObject[`cpu${i}`] = 0;
              cpuByThreadObject[`cpu${i}TimeStr`] = '0';
              cpuByThreadObject[`cpu${i}Ratio`] = '0';
            }
            cpuByThreadObject[`cpu${e.cpu}`] = e.wallDuration || 0;
            cpuByThreadObject[`cpu${e.cpu}TimeStr`] = getProbablyTime(e.wallDuration || 0);
            cpuByThreadObject[`cpu${e.cpu}Ratio`] = (
              (100.0 * (e.wallDuration || 0)) /
              (cpuByThreadValue.rightNs - cpuByThreadValue.leftNs)
            ).toFixed(2);
            map.set(`${e.tid}`, cpuByThreadObject);
          }
        }
        let arr = Array.from(map.values()).sort((a, b) => b.wallDuration - a.wallDuration);
        for (let e of arr) {
          e.avgDuration = (e.wallDuration / (e.occurrences || 1.0) / 1000000.0).toFixed(5);
          e.wallDuration = parseFloat((e.wallDuration / 1000000.0).toFixed(5));
        }
        let count: any = {};
        count.process = ' ';
        count.wallDuration = parseFloat((sumWall / 1000000.0).toFixed(7));
        count.occurrences = sumOcc;
        arr.splice(0, 0, count);
        this.cpuByThreadSource = arr;
        this.cpuByThreadTbl!.recycleDataSource = arr;
      } else {
        this.cpuByThreadSource = [];
        this.cpuByThreadTbl!.recycleDataSource = this.cpuByThreadSource;
      }
    });
  }

  getTableColumns(cpus: Array<number>) {
    let cpuByThreadTblHtml = `${this.pubColumns}`;
    let cpuByThreadList = cpus.sort((cpuByThreadA, cpuByThreadB) => cpuByThreadA - cpuByThreadB);
    for (let index of cpuByThreadList) {
      cpuByThreadTblHtml = `${cpuByThreadTblHtml}
            <lit-table-column width="100px" title="cpu${index}" data-index="cpu${index}TimeStr" key="cpu${index}TimeStr"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="100px" title="%" data-index="cpu${index}Ratio" key="cpu${index}Ratio"  align="flex-start" order>
            </lit-table-column>
            `;
    }
    return cpuByThreadTblHtml;
  }

  initElements(): void {
    this.cpuByThreadTbl = this.shadowRoot?.querySelector<LitTable>('#tb-cpu-thread');
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.cpuByThreadTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.cpuByThreadTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data;
      data.isSelected = true;
      this.cpuByThreadTbl?.clearAllSelection(data);
      this.cpuByThreadTbl?.setCurrentSelection(data);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.cpuByThreadTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .cpu-by-thread-label{
            width: 100%;
            height: 20px;
        }
        :host{
            width: auto;
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="time-range" class="cpu-by-thread-label" style="text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-thread" style="height:calc( 30vh - 25px )" >
            
        </lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (cpuByThreadLeftData: SelectionData, cpuByThreadRightData: SelectionData) {
        if (cpuByThreadLeftData.process == ' ' || cpuByThreadRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(cpuByThreadRightData[property]) - parseFloat(cpuByThreadLeftData[property]) : parseFloat(cpuByThreadLeftData[property]) - parseFloat(cpuByThreadRightData[property]);
        } else {
          // @ts-ignore
          if (cpuByThreadRightData[property] > cpuByThreadLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (cpuByThreadRightData[property] == cpuByThreadLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }
    if ((detail.key as string).includes('cpu')) {
      if ((detail.key as string).includes('Ratio')) {
        this.cpuByThreadSource.sort(compare(detail.key, detail.sort, 'string'));
      } else {
        this.cpuByThreadSource.sort(compare((detail.key as string).replace('TimeStr', ''), detail.sort, 'number'));
      }
    } else {
      if (
        detail.key === 'pid' ||
        detail.key == 'tid' ||
        detail.key === 'wallDuration' ||
        detail.key === 'avgDuration' ||
        detail.key === 'occurrences'
      ) {
        this.cpuByThreadSource.sort(compare(detail.key, detail.sort, 'number'));
      } else {
        this.cpuByThreadSource.sort(compare(detail.key, detail.sort, 'string'));
      }
    }

    this.cpuByThreadTbl!.recycleDataSource = this.cpuByThreadSource;
  }
}
