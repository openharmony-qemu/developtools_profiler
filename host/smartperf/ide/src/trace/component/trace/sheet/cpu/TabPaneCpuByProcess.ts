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
import { getTabCpuByProcess } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-cpu-process')
export class TabPaneCpuByProcess extends BaseElement {
  private cpuByProcessTbl: LitTable | null | undefined;
  private cpuByProcessRange: HTMLLabelElement | null | undefined;
  private cpuByProcessSource: Array<SelectionData> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(cpuByProcessValue: SelectionParam | any) {
    if (this.currentSelectionParam === cpuByProcessValue) {
      return;
    }
    this.currentSelectionParam = cpuByProcessValue;
    this.cpuByProcessRange!.textContent =
      'Selected range: ' +
      parseFloat(((cpuByProcessValue.rightNs - cpuByProcessValue.leftNs) / 1000000.0).toFixed(5)) +
      ' ms';
    // @ts-ignore
    this.cpuByProcessTbl!.shadowRoot!.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    this.cpuByProcessTbl!.recycleDataSource = [];
    this.cpuByProcessTbl!.loading = true;
    getTabCpuByProcess(cpuByProcessValue.cpus, cpuByProcessValue.leftNs, cpuByProcessValue.rightNs).then((result) => {
      this.cpuByProcessTbl!.loading = false;
      if (result != null && result.length > 0) {
        log('getTabCpuByProcess size :' + result.length);
        let sumWall = 0.0;
        let sumOcc = 0;
        for (let e of result) {
          let process = Utils.PROCESS_MAP.get(e.pid);
          e.process = process == null || process.length == 0 ? '[NULL]' : process;
          sumWall += e.wallDuration;
          sumOcc += e.occurrences;
          e.wallDuration = parseFloat((e.wallDuration / 1000000.0).toFixed(5));
          e.avgDuration = parseFloat((parseFloat(e.avgDuration) / 1000000.0).toFixed(5)).toString();
        }
        let count = new SelectionData();
        count.process = ' ';
        count.wallDuration = parseFloat((sumWall / 1000000.0).toFixed(5));
        count.occurrences = sumOcc;
        result.splice(0, 0, count);
        this.cpuByProcessSource = result;
        this.cpuByProcessTbl!.recycleDataSource = result;
      } else {
        this.cpuByProcessSource = [];
        this.cpuByProcessTbl!.recycleDataSource = this.cpuByProcessSource;
      }
    });
  }

  initElements(): void {
    this.cpuByProcessTbl = this.shadowRoot?.querySelector<LitTable>('#tb-cpu-process');
    this.cpuByProcessRange = this.shadowRoot?.querySelector('#cpu-process-time-range');
    this.cpuByProcessTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.cpuByProcessTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .cpu-process-label{
            font-size: 10pt;
            margin-bottom: 5px;
            text-align: end;
        }
        :host{
            border: 0;
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="cpu-process-time-range" class="cpu-process-label" style="width: 100%;height: 20px;">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-process" style="height: auto">
            <lit-table-column class="cpu-process-column" order width="30%" title="Process" data-index="process" key="process" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="PID" data-index="pid" key="pid" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Wall duration(ms)" data-index="wallDuration" key="wallDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Avg Wall duration(ms)" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Occurrences" data-index="occurrences" key="occurrences" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(cpuByProcessDetail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (cpuByProcessLeftData: SelectionData, cpuByProcessRightData: SelectionData) {
        if (cpuByProcessLeftData.process == ' ' || cpuByProcessRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(cpuByProcessRightData[property]) - parseFloat(cpuByProcessLeftData[property]) : parseFloat(cpuByProcessLeftData[property]) - parseFloat(cpuByProcessRightData[property]);
        } else {
          // @ts-ignore
          if (cpuByProcessRightData[property] > cpuByProcessLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (cpuByProcessRightData[property] == cpuByProcessLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (
      cpuByProcessDetail.key === 'pid' ||
      cpuByProcessDetail.key === 'wallDuration' ||
      cpuByProcessDetail.key === 'avgDuration' ||
      cpuByProcessDetail.key === 'occurrences'
    ) {
      this.cpuByProcessSource.sort(compare(cpuByProcessDetail.key, cpuByProcessDetail.sort, 'number'));
    } else {
      this.cpuByProcessSource.sort(compare(cpuByProcessDetail.key, cpuByProcessDetail.sort, 'string'));
    }
    this.cpuByProcessTbl!.recycleDataSource = this.cpuByProcessSource;
  }
}
