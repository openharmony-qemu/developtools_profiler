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
import { getTabProcessHistoryData } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { ProcessHistory } from '../../../../bean/AbilityMonitor.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-history-processes')
export class TabPaneHistoryProcesses extends BaseElement {
  private historyProcessTbl: LitTable | null | undefined;
  private historyProcessSource: Array<ProcessHistory> = [];
  private queryHistoryResult: Array<ProcessHistory> = [];
  private search: HTMLInputElement | undefined | null;

  set data(historyProcessValue: SelectionParam | any) {
    // @ts-ignore
    this.historyProcessTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 + 'px';
    this.queryDataByDB(historyProcessValue);
  }

  initElements(): void {
    this.historyProcessTbl = this.shadowRoot?.querySelector<LitTable>('#tb-history-processes');
    this.historyProcessTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.historyProcessTbl!);
  }

  filterData() {
    if (this.queryHistoryResult.length > 0) {
      let filterHistory = this.queryHistoryResult.filter((item) => {
        let array = this.toProcessHistoryArray(item);
        let isInclude = array.filter((value) => value.indexOf(this.search!.value) > -1);
        return isInclude.length > 0;
      });
      if (filterHistory.length > 0) {
        this.historyProcessSource = filterHistory;
        this.historyProcessTbl!.recycleDataSource = this.historyProcessSource;
      } else {
        this.historyProcessSource = [];
        this.historyProcessTbl!.recycleDataSource = [];
      }
    }
  }

  toProcessHistoryArray(process: ProcessHistory): any[] {
    let array: Array<string> = [];
    array.push(process.processId.toString());
    array.push(process.processName);
    array.push(process.alive);
    array.push(process.firstSeen);
    array.push(process.lastSeen);
    array.push(process.responsibleProcess);
    array.push(process.userName);
    array.push(process.cpuTime);
    return array;
  }

  queryDataByDB(val: SelectionParam | any) {
    getTabProcessHistoryData(val.leftNs, val.rightNs, val.processId, val.threadId).then((item) => {
      if (item.length != null && item.length > 0) {
        log('getTabProcessHistoryData result size : ' + item.length);
        for (const processHistory of item) {
          processHistory.alive = processHistory.alive == '0' ? 'No' : 'Yes';
          if (Number(processHistory.firstSeen) <= 0) {
            processHistory.firstSeen = '0:000.000.000';
            processHistory.firstSeenNumber = 0;
          } else {
            processHistory.firstSeenNumber = Number(processHistory.firstSeen);
            processHistory.firstSeen = Utils.getTimeStampHMS(processHistory.firstSeenNumber);
          }
          processHistory.lastSeenNumber = Number(processHistory.lastSeen);
          processHistory.lastSeen = Utils.getTimeStampHMS(Number(processHistory.lastSeenNumber));
          processHistory.processName = processHistory.processName + '(' + processHistory.processId + ')';
          processHistory.cpuTimeNumber = Number(processHistory.cpuTime);
          processHistory.cpuTime = this.timeFormat(processHistory.cpuTimeNumber);
        }
        this.historyProcessSource = item;
        this.queryHistoryResult = item;
        this.historyProcessTbl!.recycleDataSource = this.historyProcessSource;
      } else {
        this.historyProcessSource = [];
        this.queryHistoryResult = [];
        this.historyProcessTbl!.recycleDataSource = [];
      }
    });
  }

  timeFormat(ms: number): string {
    let currentTimeMs = ms;
    let hours = 3600000;
    let minute1 = 60000;
    let second1 = 1000;
    let res = '';
    if (currentTimeMs >= hours) {
      res += Math.floor(currentTimeMs / hours) + ' h ';
      currentTimeMs = currentTimeMs - Math.floor(currentTimeMs / hours) * hours;
    }
    if (currentTimeMs >= minute1) {
      res += Math.floor(currentTimeMs / minute1) + ' min ';
      currentTimeMs = currentTimeMs - Math.floor(currentTimeMs / minute1) * minute1;
    }
    if (currentTimeMs >= second1) {
      res += Math.floor(currentTimeMs / second1) + ' s ';
      currentTimeMs = currentTimeMs - Math.floor(currentTimeMs / second1) * second1;
    }
    if (currentTimeMs > 0) {
      res += currentTimeMs + ' ms ';
    } else {
      res += '0 ms ';
    }
    return res;
  }

  initHtml(): string {
    return `
<style>
.history-process-table{
    height: auto;
}
:host{
    flex-direction: column;
    display: flex;
    padding: 10px 10px;
}
</style>
<lit-table id="tb-history-processes" class="history-process-table">
    <lit-table-column order width="1fr" align="flex-start" title="Process ID" data-index="processId" key="processId"></lit-table-column>
    <lit-table-column order width="1fr" data-index="alive" key="alive" align="flex-start" title="Alive"></lit-table-column>
    <lit-table-column order width="1fr" title="First Seen" key="firstSeen" data-index="firstSeen" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" data-index="lastSeen" key="lastSeen" align="flex-start" title="Last Seen"></lit-table-column>
    <lit-table-column width="1fr" order title="Process Name" data-index="processName" key="processName" align="flex-start" ></lit-table-column>
    <lit-table-column order title="Responsible Process" width="1fr" data-index="responsibleProcess" key="responsibleProcess" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" data-index="userName" title="User ID" key="userName" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" data-index="cpuTime" key="cpuTime" align="flex-start" title="CPU Time"></lit-table-column>
</lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (historyProcessLeftData: ProcessHistory, historyProcessRightData: ProcessHistory) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(historyProcessRightData[property]) - parseFloat(historyProcessLeftData[property]) : parseFloat(historyProcessLeftData[property]) - parseFloat(historyProcessRightData[property]);
        } else if (type === 'cpuTime') {
          return sort === 2
            ? historyProcessRightData.cpuTimeNumber - historyProcessLeftData.cpuTimeNumber
            : historyProcessLeftData.cpuTimeNumber - historyProcessRightData.cpuTimeNumber;
        } else if (type === 'lastSeen') {
          return sort === 2
            ? historyProcessRightData.lastSeenNumber - historyProcessLeftData.lastSeenNumber
            : historyProcessLeftData.lastSeenNumber - historyProcessRightData.lastSeenNumber;
        } else if (type === 'firstSeen') {
          return sort === 2
            ? historyProcessRightData.firstSeenNumber - historyProcessLeftData.firstSeenNumber
            : historyProcessLeftData.firstSeenNumber - historyProcessRightData.firstSeenNumber;
        } else if (type === 'alive') {
          let aaaa = 0;
          let bbbb = 0;
          // @ts-ignore
          if (historyProcessRightData[property] == 'Yes') {
            bbbb = 1;
          }
          // @ts-ignore
          if (historyProcessLeftData[property] == 'Yes') {
            aaaa = 1;
          }
          if (aaaa - bbbb == 0) {
            return 0;
          }
          return aaaa - bbbb ? -1 : 1;
        } else {
          // @ts-ignore
          if (historyProcessRightData[property] > historyProcessLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (historyProcessRightData[property] == historyProcessLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (detail.key === 'startTime' || detail.key === 'processName') {
      this.historyProcessSource.sort(compare(detail.key, detail.sort, 'string'));
    } else if (detail.key == 'cpuTime') {
      this.historyProcessSource.sort(compare(detail.key, detail.sort, 'cpuTime'));
    } else if (detail.key === 'alive') {
      this.historyProcessSource.sort(compare(detail.key, detail.sort, 'alive'));
    } else {
      this.historyProcessSource.sort(compare(detail.key, detail.sort, 'number'));
    }
    this.historyProcessTbl!.recycleDataSource = this.historyProcessSource;
  }
}
