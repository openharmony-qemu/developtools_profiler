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
import { Counter, SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabCounters, getTabVirtualCounters } from '../../../../database/SqlLite.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-counter')
export class TabPaneCounter extends BaseElement {
  private counterTbl: LitTable | null | undefined;
  private counterRange: HTMLLabelElement | null | undefined;
  private counterSource: Array<SelectionData> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(counterParam: SelectionParam | any) {
    if (this.currentSelectionParam === counterParam) {
      return;
    }
    this.currentSelectionParam = counterParam;
    //@ts-ignore
    this.counterTbl?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.counterRange!.textContent =
      'Selected range: ' + parseFloat(((counterParam.rightNs - counterParam.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    this.counterTbl!.loading = true;
    getTabCounters(counterParam.processTrackIds, counterParam.virtualTrackIds, counterParam.rightNs).then((result) => {
      this.counterTbl!.loading = false;
      if (result != null && result.length > 0) {
        let dataSource: Array<SelectionData> = [];
        let collect = this.groupByTrackIdToMap(result);
        let sumCount = 0;
        for (let key of collect.keys()) {
          let counters = collect.get(key);
          let list: Array<Counter> = [];
          let index = counters!.findIndex((item) => item.startTime >= counterParam.leftNs);
          if (index != -1) {
            list = counters!.splice(index > 0 ? index - 1 : index);
          } else {
            list.push(counters![counters!.length - 1]);
          }
          let sd = this.createSelectCounterData(list, counterParam.leftNs, counterParam.rightNs);
          sumCount += Number.parseInt(sd.count);
          dataSource.push(sd);
        }
        let sumData = new SelectionData();
        sumData.count = sumCount.toString();
        sumData.process = ' ';
        dataSource.splice(0, 0, sumData);
        this.counterSource = dataSource;
        this.counterTbl!.recycleDataSource = dataSource;
      } else {
        this.counterSource = [];
        this.counterTbl!.recycleDataSource = this.counterSource;
      }
    });
  }

  initElements(): void {
    this.counterTbl = this.shadowRoot?.querySelector<LitTable>('#tb-counter');
    this.counterRange = this.shadowRoot?.querySelector('#time-range');
    this.counterTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.counterTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .counter-label{
            width: 100%;
        }
        :host{
            flex-direction: column;
            display: flex;
            padding: 10px 10px;
        }
        </style>
        <label id="time-range" class="counter-label" style="height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-counter" style="height: auto">
            <lit-table-column width="25%" title="Name" data-index="name" key="name"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="Delta value" data-index="delta" key="delta"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Rate /s" data-index="rate" key="rate"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Weighted avg value" data-index="avgWeight" key="avgWeight"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Count" data-index="count" key="count"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="First value" data-index="first" key="first"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Last value" data-index="last" key="last"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Min value" data-index="min" key="min"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Max value" data-index="max" key="max"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  groupByTrackIdToMap(arr: Array<Counter>): Map<number, Array<Counter>> {
    let map = new Map<number, Array<Counter>>();
    for (let counter of arr) {
      counter.name = counter.name.replace('sys.virtual.mem.', '');
      if (map.has(counter.trackId)) {
        map.get(counter.trackId)!.push(counter);
      } else {
        let list: Array<Counter> = [];
        list.push(counter);
        map.set(counter.trackId, list);
      }
    }
    return map;
  }

  createSelectCounterData(list: Array<Counter>, leftNs: number, rightNs: number): SelectionData {
    let counterData = new SelectionData();
    if (list.length > 0) {
      let range = rightNs - leftNs;
      let first = list[0];
      counterData.trackId = first.trackId;
      counterData.name = first.name;
      counterData.first = first.value + '';
      counterData.count = list.length + '';
      counterData.last = list[list.length - 1].value + '';
      counterData.delta = parseInt(counterData.last) - parseInt(counterData.first) + '';
      counterData.rate = (parseInt(counterData.delta) / ((range * 1.0) / 1000000000)).toFixed(4);
      counterData.min = first.value + '';
      counterData.max = '0';
      let weightAvg = 0.0;
      for (let i = 0; i < list.length; i++) {
        let counter = list[i];
        if (counter.value < parseInt(counterData.min)) {
          counterData.min = counter.value.toString();
        }
        if (counter.value > parseInt(counterData.max)) {
          counterData.max = counter.value.toString();
        }
        let start = i == 0 ? leftNs : counter.startTime;
        let end = i == list.length - 1 ? rightNs : list[i + 1].startTime;
        weightAvg += counter.value * (((end - start) * 1.0) / range);
      }
      counterData.avgWeight = weightAvg.toFixed(2);
    }
    return counterData;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (counterLeftData: SelectionData, counterRightData: SelectionData) {
        if (counterLeftData.process == ' ' || counterRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(counterRightData[property]) - parseFloat(counterLeftData[property]) : parseFloat(counterLeftData[property]) - parseFloat(counterRightData[property]);
        } else {
          // @ts-ignore
          if (counterRightData[property] > counterLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (counterRightData[property] == counterLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (detail.key === 'name') {
      this.counterSource.sort(compare(detail.key, detail.sort, 'string'));
    } else {
      this.counterSource.sort(compare(detail.key, detail.sort, 'number'));
    }
    this.counterTbl!.recycleDataSource = this.counterSource;
  }
}
