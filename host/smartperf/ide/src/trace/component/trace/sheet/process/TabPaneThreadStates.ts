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
import { getTabThreadStates } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { StackBar } from '../../../StackBar.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-thread-states')
export class TabPaneThreadStates extends BaseElement {
  private threadStatesTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private stackBar: StackBar | null | undefined;
  private threadStatesTblSource: Array<SelectionData> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(threadStatesParam: SelectionParam | any) {
    if (this.currentSelectionParam === threadStatesParam) {
      return;
    }
    this.currentSelectionParam = threadStatesParam;
    //@ts-ignore
    this.threadStatesTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    // // @ts-ignore
    this.range!.textContent =
      'Selected range: ' + ((threadStatesParam.rightNs - threadStatesParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.threadStatesTbl!.loading = true;
    getTabThreadStates(threadStatesParam.threadIds, threadStatesParam.leftNs, threadStatesParam.rightNs).then(
      (result) => {
        if (result != null && result.length > 0) {
          log('getTabThreadStates result size : ' + result.length);
          let sumWall = 0.0;
          let sumOcc = 0;
          let targetList = [];
          for (let e of result) {
            if (threadStatesParam.processIds.includes(e.pid)) {
              let process = Utils.PROCESS_MAP.get(e.pid);
              let thread = Utils.THREAD_MAP.get(e.tid);
              e.process = process == null || process.length == 0 ? '[NULL]' : process;
              e.thread = thread == null || thread.length == 0 ? '[NULL]' : thread;
              sumWall += e.wallDuration;
              sumOcc += e.occurrences;
              e.stateJX = e.state;
              e.state = Utils.getEndState(e.stateJX);
              e.wallDuration = parseFloat((e.wallDuration / 1000000.0).toFixed(5));
              e.avgDuration = parseFloat((e.avgDuration / 1000000.0).toFixed(5));
              targetList.push(e);
            }
          }
          if (targetList.length > 0) {
            let count: any = {};
            count.process = ' ';
            count.state = ' ';
            count.wallDuration = parseFloat((sumWall / 1000000.0).toFixed(5));
            count.occurrences = sumOcc;
            targetList.splice(0, 0, count);
          }
          this.threadStatesTblSource = targetList;
          this.threadStatesTbl!.recycleDataSource = targetList;
          this.stackBar!.data = targetList;
        } else {
          this.threadStatesTblSource = [];
          this.stackBar!.data = [];
          this.threadStatesTbl!.recycleDataSource = [];
        }
        this.threadStatesTbl!.loading = false;
      }
    );
  }

  initElements(): void {
    this.threadStatesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-thread-states');
    this.range = this.shadowRoot?.querySelector('#thread-states-time-range');
    this.stackBar = this.shadowRoot?.querySelector('#thread-states-stack-bar');
    this.threadStatesTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.threadStatesTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .tread-states-table{
            display: flex;
            height: 20px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="tread-states-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px;justify-content: space-between">
            <stack-bar id="thread-states-stack-bar" style="width: calc(100vw - 520px)"></stack-bar>
            <label id="thread-states-time-range"  style="width: 250px;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <lit-table id="tb-thread-states" style="height: auto;overflow-x: auto;width: 100%">
            <lit-table-column class="tread-states-column" width="240px" title="Process" data-index="process" key="process"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="120px" title="PID" data-index="pid" key="pid"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="240px" title="Thread" data-index="thread" key="thread"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="120px" title="TID" data-index="tid" key="tid"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="240px" title="State" data-index="state" key="state"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="120px" title="Wall duration(ms)" data-index="wallDuration" key="wallDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="120px" title="Avg Wall duration(ms)" data-index="avgDuration" key="avgDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="tread-states-column" width="120px" title="Occurrences" data-index="occurrences" key="occurrences"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(treadStatesDetail: any) {
    function compare(property: any, treadStatesSort: any, type: any) {
      return function (threadStatesLeftData: SelectionData | any, threadStatesRightData: SelectionData | any) {
        if (threadStatesLeftData.process == ' ' || threadStatesRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          return treadStatesSort === 2
            ? parseFloat(threadStatesRightData[property]) - parseFloat(threadStatesLeftData[property])
            : parseFloat(threadStatesLeftData[property]) - parseFloat(threadStatesRightData[property]);
        } else {
          if (threadStatesRightData[property] > threadStatesLeftData[property]) {
            return treadStatesSort === 2 ? 1 : -1;
          } else if (threadStatesRightData[property] == threadStatesLeftData[property]) {
            return 0;
          } else {
            return treadStatesSort === 2 ? -1 : 1;
          }
        }
      };
    }

    if (treadStatesDetail.key === 'name' || treadStatesDetail.key === 'thread' || treadStatesDetail.key === 'state') {
      this.threadStatesTblSource.sort(compare(treadStatesDetail.key, treadStatesDetail.sort, 'string'));
    } else {
      this.threadStatesTblSource.sort(compare(treadStatesDetail.key, treadStatesDetail.sort, 'number'));
    }
    this.threadStatesTbl!.recycleDataSource = this.threadStatesTblSource;
  }
}
