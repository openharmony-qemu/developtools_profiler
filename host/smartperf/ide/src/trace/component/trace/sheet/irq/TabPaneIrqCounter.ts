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
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-irq-counter')
export class TabPaneIrqCounter extends BaseElement {
  private irqCounterTbl: LitTable | null | undefined;
  private irqRange: HTMLLabelElement | null | undefined;
  private irqCounterSource: Array<SelectionData> = [];

  set data(irqParam: SelectionParam | any) {
    //@ts-ignore
    this.irqCounterTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    this.irqRange!.textContent =
      'Selected range: ' + parseFloat(((irqParam.rightNs - irqParam.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    let dataSource: Array<SelectionData> = [];
    let collect = irqParam.irqMapData;
    let sumCount = 0;
    for (let key of collect.keys()) {
      let counters = collect.get(key);
      let selectCounterData = this.createSelectCounterData(key, counters);
      sumCount += Number.parseInt(selectCounterData.count || '0');
      selectCounterData.avgDuration = Utils.getProbablyTime(
        selectCounterData.wallDuration / parseInt(selectCounterData.count)
      );
      dataSource.push(selectCounterData);
    }
    this.irqCounterSource = dataSource;
    this.irqCounterTbl!.recycleDataSource = dataSource;
  }

  initElements(): void {
    this.irqCounterTbl = this.shadowRoot?.querySelector<LitTable>('#tb-irq-counter');
    this.irqRange = this.shadowRoot?.querySelector('#time-range');
    this.irqCounterTbl!.addEventListener('column-click', (event) => {
      // @ts-ignore
      this.sortByColumn(event.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.irqCounterTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .irq-counter-label{
            font-size: 10pt;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="time-range" class="irq-counter-label" style="width: 100%;height: 20px;text-align: end;margin-bottom: 5px;">Selected range:0.0 ms</label>
        <lit-table id="tb-irq-counter" style="height: auto">
            <lit-table-column width="30%" title="Name" data-index="name" key="name"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="Duration" data-index="wallDurationFormat" key="wallDurationFormat"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Average Duration" data-index="avgDuration" key="avgDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Occurrences" data-index="count" key="count"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  createSelectCounterData(name: string, list: Array<any>): SelectionData {
    let selectData = new SelectionData();
    if (list.length > 0) {
      selectData.name = name;
      selectData.count = list.length + '';
      for (let index = 0; index < list.length; index++) {
        selectData.wallDuration += list[index].dur;
      }
      selectData.wallDurationFormat = Utils.getProbablyTime(selectData.wallDuration);
    }
    return selectData;
  }

  sortByColumn(detail: any) {
    let type = detail.sort;
    let key = detail.key;
    if (type == 0) {
      this.irqCounterTbl!.recycleDataSource = this.irqCounterSource;
    } else {
      let arr = Array.from(this.irqCounterSource);
      arr.sort((irqCounterLeftData, irqCounterRightData): number => {
        if (key == 'wallDurationFormat') {
          if (type == 1) {
            return irqCounterLeftData.wallDuration - irqCounterRightData.wallDuration;
          } else {
            return irqCounterRightData.wallDuration - irqCounterLeftData.wallDuration;
          }
        } else if (key == 'count') {
          if (type == 1) {
            return parseInt(irqCounterLeftData.count) >= parseInt(irqCounterRightData.count) ? 1 : -1;
          } else {
            return parseInt(irqCounterRightData.count) >= parseInt(irqCounterLeftData.count) ? 1 : -1;
          }
        } else if (key == 'avgDuration') {
          if (type == 1) {
            return (
              irqCounterLeftData.wallDuration / parseInt(irqCounterLeftData.count) -
              irqCounterRightData.wallDuration / parseInt(irqCounterRightData.count)
            );
          } else {
            return (
              irqCounterRightData.wallDuration / parseInt(irqCounterRightData.count) -
              irqCounterLeftData.wallDuration / parseInt(irqCounterLeftData.count)
            );
          }
        } else if (key == 'name') {
          if (irqCounterLeftData.name > irqCounterRightData.name) {
            return type === 2 ? 1 : -1;
          } else if (irqCounterLeftData.name == irqCounterRightData.name) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else {
          return 0;
        }
      });
      this.irqCounterTbl!.recycleDataSource = arr;
    }
  }
}
