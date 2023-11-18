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
import { getTabLiveProcessData } from '../../../../database/SqlLite.js';
import { LiveProcess } from '../../../../bean/AbilityMonitor.js';
import { Utils } from '../../base/Utils.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-live-processes')
export class TabPaneLiveProcesses extends BaseElement {
  private liveProcessTbl: LitTable | null | undefined;
  private liveProcessSource: Array<LiveProcess> = [];
  private queryLiveResult: Array<LiveProcess> = [];
  private search: HTMLInputElement | undefined | null;

  set data(liveProcessValue: SelectionParam | any) {
    // @ts-ignore
    this.liveProcessTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 + 'px';
    this.queryDataByDB(liveProcessValue);
  }

  initElements(): void {
    this.liveProcessTbl = this.shadowRoot?.querySelector<LitTable>('#tb-live-processes');
    this.liveProcessTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.liveProcessTbl!);
  }

  filterData() {
    if (this.queryLiveResult.length > 0) {
      let filterLive = this.queryLiveResult.filter((item) => {
        let array = this.toLiveProcessArray(item);
        let isInclude = array.filter((value) => value.indexOf(this.search!.value) > -1);
        return isInclude.length > 0;
      });
      if (filterLive.length > 0) {
        this.liveProcessSource = filterLive;
        this.liveProcessTbl!.recycleDataSource = this.liveProcessSource;
      } else {
        this.liveProcessSource = [];
        this.liveProcessTbl!.recycleDataSource = [];
      }
    }
  }

  toLiveProcessArray(liveProcess: LiveProcess): any[] {
    let array: Array<string> = [];
    array.push(liveProcess.processId.toString());
    array.push(liveProcess.processName);
    array.push(liveProcess.responsibleProcess);
    array.push(liveProcess.userName);
    array.push(liveProcess.threads.toString());
    array.push(liveProcess.cpu);
    array.push(liveProcess.memory);
    array.push(liveProcess.diskReads.toString());
    array.push(liveProcess.diskWrite.toString());
    return array;
  }

  queryDataByDB(val: SelectionParam | any) {
    getTabLiveProcessData(val.leftNs, val.rightNs).then((item) => {
      if (item.length != null && item.length > 0) {
        log('getTabLiveProcessData result size : ' + item.length);
        for (const liveProcess of item) {
          liveProcess.processName = liveProcess.processName + '(' + liveProcess.processId + ')';
          liveProcess.memoryNumber = Number(liveProcess.memory);
          liveProcess.memory = Utils.getBinaryByteWithUnit(liveProcess.memoryNumber);
          if (Number(liveProcess.cpu) > 0) {
            liveProcess.cpu = Number(Number(liveProcess.cpu).toFixed(3)) + '%';
          } else {
            liveProcess.cpu = '0%';
          }
          liveProcess.cpuTimeNumber = Number(liveProcess.cpuTime);
          liveProcess.cpuTime = this.timeFormat(Number(liveProcess.cpuTime));
        }
        this.liveProcessSource = item;
        this.queryLiveResult = item;
        this.liveProcessTbl!.recycleDataSource = this.liveProcessSource;
      } else {
        this.liveProcessSource = [];
        this.queryLiveResult = [];
        this.liveProcessTbl!.recycleDataSource = [];
      }
    });
  }

  timeFormat(ms: number): string {
    let currentMsTime = ms;
    let hours = 3600000;
    let minute1 = 60000;
    let second1 = 1000;
    let res = '';
    if (currentMsTime >= hours) {
      res += Math.floor(currentMsTime / hours) + ' h ';
      currentMsTime = currentMsTime - Math.floor(currentMsTime / hours) * hours;
    }
    if (currentMsTime >= minute1) {
      res += Math.floor(currentMsTime / minute1) + ' min ';
      currentMsTime = currentMsTime - Math.floor(currentMsTime / minute1) * minute1;
    }
    if (currentMsTime >= second1) {
      res += Math.floor(currentMsTime / second1) + ' s ';
      currentMsTime = currentMsTime - Math.floor(currentMsTime / second1) * second1;
    }
    if (currentMsTime > 0) {
      res += currentMsTime + ' ms ';
    } else {
      res += '0 ms ';
    }
    return res;
  }

  initHtml(): string {
    return `
<style>
.live-processes-table{
    height: auto;
}
:host{
    display: flex;
    flex-direction: column;
    padding: 10px 10px;
}
</style>
<lit-table id="tb-live-processes" class="live-processes-table">
    <lit-table-column order title="Process ID" data-index="processId" key="processId" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="Process Name" data-index="processName" key="processName" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="Responsible Process" data-index="responsibleProcess" key="responsibleProcess" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="User ID" data-index="userName" key="userName" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="%CPU" data-index="cpu" key="cpu" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="CPU Time" data-index="cpuTime" key="cpuTime" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="#Threads" data-index="threads" key="threads" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="Memory" data-index="memory" key="memory" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="Disk Writes(B)" data-index="diskWrite" key="diskWrite" align="flex-start" width="1fr" ></lit-table-column>
    <lit-table-column order title="Disk Reads(B)" data-index="diskReads" key="diskReads" align="flex-start" width="1fr" ></lit-table-column>
</lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (liveProcessLeftData: LiveProcess, liveProcessRightData: LiveProcess) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(liveProcessRightData[property]) - parseFloat(liveProcessLeftData[property]) : parseFloat(liveProcessLeftData[property]) - parseFloat(liveProcessRightData[property]);
        } else if (type === 'cpuTime') {
          return sort === 2
            ? liveProcessRightData.cpuTimeNumber - liveProcessLeftData.cpuTimeNumber
            : liveProcessLeftData.cpuTimeNumber - liveProcessRightData.cpuTimeNumber;
        } else if (type === 'memory') {
          return sort === 2
            ? liveProcessRightData.memoryNumber - liveProcessLeftData.memoryNumber
            : liveProcessLeftData.memoryNumber - liveProcessRightData.memoryNumber;
        } else {
          // @ts-ignore
          if (liveProcessRightData[property] > liveProcessLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (liveProcessRightData[property] == liveProcessLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (detail.key == 'startTime' || detail.key == 'processName') {
      this.liveProcessSource.sort(compare(detail.key, detail.sort, 'string'));
    } else if (detail.key == 'cpuTime') {
      this.liveProcessSource.sort(compare(detail.key, detail.sort, 'cpuTime'));
    } else if (detail.key == 'memory') {
      this.liveProcessSource.sort(compare(detail.key, detail.sort, 'memory'));
    } else {
      this.liveProcessSource.sort(compare(detail.key, detail.sort, 'number'));
    }
    this.liveProcessTbl!.recycleDataSource = this.liveProcessSource;
  }
}
