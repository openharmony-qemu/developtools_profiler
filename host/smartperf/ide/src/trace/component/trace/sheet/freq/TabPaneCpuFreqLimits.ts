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
import { Utils } from '../../base/Utils.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { CpuFreqLimitsStruct } from '../../../../database/ui-worker/ProcedureWorkerCpuFreqLimits.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-cpu-freq-limits')
export class TabPaneCpuFreqLimits extends BaseElement {
  private cpuFreqLimitsTbl: LitTable | null | undefined;
  private selectionParam: SelectionParam | null | undefined;
  private cpuFreqLimitSource: CpuFreqLimit[] = [];
  private cpuFreqLimitSortKey: string = 'cpu';
  private cpuFreqLimitSortType: number = 0;

  set data(cpuFreqLimitSelection: SelectionParam | any) {
    if (cpuFreqLimitSelection == this.selectionParam) {
      return;
    }
    this.selectionParam = cpuFreqLimitSelection;
    // @ts-ignore
    this.cpuFreqLimitsTbl!.shadowRoot!.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 25 + 'px';
    let list: any[] = [];
    cpuFreqLimitSelection.cpuFreqLimitDatas.forEach((limitRowDatas: any) => {
      for (let i = 0, len = limitRowDatas.length; i < len; i++) {
        let it = limitRowDatas[i];
        if (it.startNs > cpuFreqLimitSelection.rightNs) {
          break;
        }
        if (i === limitRowDatas.length - 1) {
          it.dur = (cpuFreqLimitSelection.rightNs || 0) - (it.startNs || 0);
        } else {
          it.dur = (limitRowDatas[i + 1].startNs || 0) - (it.startNs || 0);
        }
        list.push(it);
      }
    });
    this.formatData(list, cpuFreqLimitSelection.leftNs, cpuFreqLimitSelection.rightNs);
  }

  initElements(): void {
    this.cpuFreqLimitsTbl = this.shadowRoot!.querySelector<LitTable>('#tb-cpu-freq-limit');
    this.cpuFreqLimitsTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.cpuFreqLimitSortKey = evt.detail.key;
      // @ts-ignore
      this.cpuFreqLimitSortType = evt.detail.sort;
      // @ts-ignore
      this.sortCpuFreqLimitTable(evt.detail.key, evt.detail.sort);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.cpuFreqLimitsTbl!, 25);
  }

  formatData(list: CpuFreqLimitsStruct[], start: number, end: number) {
    let limitsMap = new Map<string, CpuFreqLimit>();
    let groupMapData = (time: number, id: string, item: CpuFreqLimitsStruct) => {
      if (limitsMap.has(id)) {
        limitsMap.get(id)!.time += time;
      } else {
        let isMax = id.endsWith('max');
        let limit = new CpuFreqLimit();
        limit.cpu = `Cpu ${item.cpu}`;
        limit.time = time;
        limit.type = isMax ? 'Max Freqency' : 'Min Frequency';
        limit.value = isMax ? item.max! : item.min!;
        limitsMap.set(id, limit);
      }
    };
    list.forEach((item) => {
      if (item.startNs! > end) {
        return;
      }
      let max = Math.max(item.startNs || 0, start);
      let min = Math.min((item.startNs || 0) + item.dur, end);
      if (max < min) {
        let maxId = `${item.cpu}-${item.max}-max`;
        let minId = `${item.cpu}-${item.min}-min`;
        groupMapData(min - max, maxId, item);
        groupMapData(min - max, minId, item);
      }
    });
    this.cpuFreqLimitSource = Array.from(limitsMap.values()).map((item) => {
      item.timeStr = Utils.getProbablyTime(item.time);
      item.valueStr = `${ColorUtils.formatNumberComma(item.value!)} kHz`;
      return item;
    });
    this.sortCpuFreqLimitTable(this.cpuFreqLimitSortKey, this.cpuFreqLimitSortType);
  }

  sortCpuFreqLimitTable(key: string, type: number) {
    if (type == 0) {
      this.cpuFreqLimitsTbl!.recycleDataSource = this.cpuFreqLimitSource;
    } else {
      let cpuFreqLimitsArr = Array.from(this.cpuFreqLimitSource);
      cpuFreqLimitsArr.sort((cpuFreqLimitA, cpuFreqLimitB): number => {
        if (key == 'timeStr') {
          if (type == 1) {
            return cpuFreqLimitA.time - cpuFreqLimitB.time;
          } else {
            return cpuFreqLimitB.time - cpuFreqLimitA.time;
          }
        } else if (key == 'valueStr') {
          if (type == 1) {
            return cpuFreqLimitA.value - cpuFreqLimitB.value;
          } else {
            return cpuFreqLimitB.value - cpuFreqLimitA.value;
          }
        } else if (key == 'cpu') {
          if (cpuFreqLimitA.cpu > cpuFreqLimitB.cpu) {
            return type === 2 ? -1 : 1;
          } else if (cpuFreqLimitA.cpu == cpuFreqLimitB.cpu) {
            return 0;
          } else {
            return type === 2 ? 1 : -1;
          }
        } else if (key == 'type') {
          if (cpuFreqLimitA.type > cpuFreqLimitB.type) {
            return type === 2 ? 1 : -1;
          } else if (cpuFreqLimitA.type == cpuFreqLimitB.type) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else {
          return 0;
        }
      });
      this.cpuFreqLimitsTbl!.recycleDataSource = cpuFreqLimitsArr;
    }
  }

  initHtml(): string {
    return `
        <style>
        .cpu-freq-limit-tbl {
            height: auto;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <lit-table id="tb-cpu-freq-limit" class="cpu-freq-limit-tbl">
            <lit-table-column class="cpu-freq-limit-column" width="20%" title="Cpu" data-index="cpu" key="cpu" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-freq-limit-column" width="1fr" title="Type" data-index="type" key="type" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-freq-limit-column" width="1fr" title="Time" data-index="timeStr" key="timeStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-freq-limit-column" width="1fr" title="Value" data-index="valueStr" key="valueStr" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }
}

class CpuFreqLimit {
  cpu: string = '';
  type: string = '';
  time: number = 0;
  value: number = 0;
  timeStr: string = '';
  valueStr: string = '';
}
