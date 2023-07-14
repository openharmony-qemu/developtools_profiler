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
import '../../../StackBar.js';
import { getTabThreadStatesCpu } from '../../../../database/SqlLite.js';
import { StackBar } from '../../../StackBar.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { Utils } from '../../base/Utils.js';
import { CpuStruct } from '../../../../database/ui-worker/ProcedureWorkerCPU.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-thread-usage')
export class TabPaneThreadUsage extends BaseElement {
  private threadUsageTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private stackBar: StackBar | null | undefined;
  private threadUsageSource: Array<SelectionData> = [];
  private cpuCount = 0;
  private currentSelectionParam: SelectionParam | undefined;
  private pubColumns = `
            <lit-table-column width="200px" title="Process" data-index="process" key="process"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="100px" title="PID" data-index="pid" key="pid"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="200px" title="Thread" data-index="thread" key="thread"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="100px" title="TID" data-index="tid" key="tid"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="160px" title="Wall duration" data-index="wallDurationTimeStr" key="wallDurationTimeStr"  align="flex-start" order >
            </lit-table-column>
    `;

  set data(threadUsageParam: SelectionParam | any) {
    if (this.currentSelectionParam === threadUsageParam) {
      return;
    }
    this.currentSelectionParam = threadUsageParam;
    if (this.cpuCount !== CpuStruct.cpuCount) {
      this.cpuCount = CpuStruct.cpuCount;
      this.threadUsageTbl!.innerHTML = this.getTableColumns();
    }
    //@ts-ignore
    this.threadUsageTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    // // @ts-ignore
    this.range!.textContent =
      'Selected range: ' + ((threadUsageParam.rightNs - threadUsageParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.threadUsageTbl!.loading = true;
    getTabThreadStatesCpu(threadUsageParam.threadIds, threadUsageParam.leftNs, threadUsageParam.rightNs).then(
      (result) => {
        this.threadUsageTbl!.loading = false;
        if (result != null && result.length > 0) {
          log('getTabThreadStates result size : ' + result.length);
          let filterArr = result.filter((it) => threadUsageParam.processIds.includes(it.pid));
          let map: Map<number, any> = new Map<number, any>();
          for (let resultEl of filterArr) {
            if (threadUsageParam.processIds.includes(resultEl.pid)) {
              if (map.has(resultEl.tid)) {
                map.get(resultEl.tid)[`cpu${resultEl.cpu}`] = resultEl.wallDuration || 0;
                map.get(resultEl.tid)[`cpu${resultEl.cpu}TimeStr`] = getProbablyTime(resultEl.wallDuration || 0);
                map.get(resultEl.tid)[`cpu${resultEl.cpu}Ratio`] = (
                  (100.0 * (resultEl.wallDuration || 0)) /
                  (threadUsageParam.rightNs - threadUsageParam.leftNs)
                ).toFixed(2);
                map.get(resultEl.tid)[`wallDuration`] =
                  map.get(resultEl.tid)[`wallDuration`] + (resultEl.wallDuration || 0);
                map.get(resultEl.tid)[`wallDurationTimeStr`] = getProbablyTime(map.get(resultEl.tid)[`wallDuration`]);
              } else {
                let process = Utils.PROCESS_MAP.get(resultEl.pid);
                let thread = Utils.THREAD_MAP.get(resultEl.tid);
                let threadStatesStruct: any = {
                  tid: resultEl.tid,
                  pid: resultEl.pid,
                  thread: thread || 'null',
                  process: process || 'null',
                  wallDuration: resultEl.wallDuration || 0,
                  wallDurationTimeStr: getProbablyTime(resultEl.wallDuration || 0),
                };
                for (let i = 0; i < this.cpuCount; i++) {
                  threadStatesStruct[`cpu${i}`] = 0;
                  threadStatesStruct[`cpu${i}TimeStr`] = '0';
                  threadStatesStruct[`cpu${i}Ratio`] = '0';
                }
                threadStatesStruct[`cpu${resultEl.cpu}`] = resultEl.wallDuration || 0;
                threadStatesStruct[`cpu${resultEl.cpu}TimeStr`] = getProbablyTime(resultEl.wallDuration || 0);
                threadStatesStruct[`cpu${resultEl.cpu}Ratio`] = (
                  (100.0 * (resultEl.wallDuration || 0)) /
                  (threadUsageParam.rightNs - threadUsageParam.leftNs)
                ).toFixed(2);
                map.set(resultEl.tid, threadStatesStruct);
              }
            }
          }
          this.threadUsageSource = Array.from(map.values());
          this.threadUsageTbl!.recycleDataSource = this.threadUsageSource;
        } else {
          this.threadUsageSource = [];
          this.threadUsageTbl!.recycleDataSource = [];
        }
      }
    );
  }

  getTableColumns() {
    let threadUsageHtml = `${this.pubColumns}`;
    let cpuCount = CpuStruct.cpuCount;
    for (let index = 0; index < cpuCount; index++) {
      threadUsageHtml = `${threadUsageHtml}
            <lit-table-column width="100px" title="cpu${index}" data-index="cpu${index}TimeStr" key="cpu${index}TimeStr"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="100px" title="%" data-index="cpu${index}Ratio" key="cpu${index}Ratio"  align="flex-start" order>
            </lit-table-column>
            `;
    }
    return threadUsageHtml;
  }

  initElements(): void {
    this.threadUsageTbl = this.shadowRoot?.querySelector<LitTable>('#tb-thread-states');
    this.range = this.shadowRoot?.querySelector('#thread-usage-time-range');
    this.stackBar = this.shadowRoot?.querySelector('#thread-usage-stack-bar');
    this.threadUsageTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.threadUsageTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .thread-usage-table{
          flex-direction: row;
          margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="thread-usage-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <stack-bar id="thread-usage-stack-bar" style="flex: 1"></stack-bar>
            <label id="thread-usage-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
            <lit-table id="tb-thread-states" style="height: auto"></lit-table>
        </div>
        `;
  }

  sortByColumn(treadUsageDetail: any) {
    function compare(property: any, treadUsageSort: any, type: any) {
      return function (threadUsageLeftData: SelectionData | any, threadUsageRightData: SelectionData | any) {
        if (threadUsageLeftData.process == ' ' || threadUsageRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          return treadUsageSort === 2
            ? parseFloat(threadUsageRightData[property]) - parseFloat(threadUsageLeftData[property])
            : parseFloat(threadUsageLeftData[property]) - parseFloat(threadUsageRightData[property]);
        } else {
          if (threadUsageRightData[property] > threadUsageLeftData[property]) {
            return treadUsageSort === 2 ? 1 : -1;
          } else if (threadUsageRightData[property] == threadUsageLeftData[property]) {
            return 0;
          } else {
            return treadUsageSort === 2 ? -1 : 1;
          }
        }
      };
    }

    if (
      treadUsageDetail.key === 'process' ||
      treadUsageDetail.key === 'thread' ||
      (treadUsageDetail.key as string).includes('Ratio')
    ) {
      this.threadUsageSource.sort(compare(treadUsageDetail.key, treadUsageDetail.sort, 'string'));
    } else {
      this.threadUsageSource.sort(
        compare((treadUsageDetail.key as string).replace('TimeStr', ''), treadUsageDetail.sort, 'number')
      );
    }
    this.threadUsageTbl!.recycleDataSource = this.threadUsageSource;
  }
}
