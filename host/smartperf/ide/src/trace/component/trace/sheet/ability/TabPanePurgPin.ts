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
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { queryProcessPurgeableTab, querySysPurgeableTab } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';
import { PurgeableTabStruct } from './TabPanePurgTotal.js';

@element('tabpane-purg-pin')
export class TabPanePurgPin extends BaseElement {
  private purgeablePinTable: LitTable | null | undefined;
  private purgeablePinSource: Array<PurgeableTabStruct> = [];
  private tabTitle: HTMLDivElement | undefined | null;
  private purgPinTimeRange: HTMLLabelElement | undefined | null;
  private sortKey = 'avgSize';
  private sortType = 2;

  set data(selection: SelectionParam) {
    //@ts-ignore
    this.purgeablePinTable?.shadowRoot?.querySelector('.table')?.style?.height = `${
      this.parentElement!.clientHeight - 45
    }px`;
    this.init();
    this.purgPinTimeRange!.textContent =
      'Selected range: ' + ((selection.rightNs - selection.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.purgeablePinTable!.loading = true;
    this.purgeablePinTable!.recycleDataSource = [];
    if (selection.purgeablePinAbility.length > 0) {
      this.purgeablePinSource = [];
      querySysPurgeableTab(
        selection.leftNs,
        selection.rightNs,
        (MemoryConfig.getInstance().interval * 1000_000) / 5,
        true
      ).then((purgePinResults) => {
        this.purgeablePinTable!.loading = false;
        if (purgePinResults.length > 0) {
          for (let i = 0; i < purgePinResults.length; i++) {
            this.purgeablePinSource.push(
              this.toTabStruct(purgePinResults[i].name, purgePinResults[i].maxSize, purgePinResults[i].minSize, purgePinResults[i].avgSize)
            );
          }
          this.sortByColumn({ key: this.sortKey, sort: this.sortType });
          let total = this.totalData(this.purgeablePinSource);
          this.purgeablePinSource.unshift(total);
          this.purgeablePinTable!.recycleDataSource = this.purgeablePinSource;
          this.purgeablePinSource.shift();
        } else {
          this.purgeablePinSource = [];
          this.purgeablePinTable!.recycleDataSource = [];
        }
      });
    } else if (selection.purgeablePinVM.length > 0) {
      this.purgeablePinSource = [];
      queryProcessPurgeableTab(
        selection.leftNs,
        selection.rightNs,
        (MemoryConfig.getInstance().interval * 1000_000) / 5,
        MemoryConfig.getInstance().iPid,
        true
      ).then((results) => {
        this.purgeablePinTable!.loading = false;
        if (results.length > 0) {
          for (let i = 0; i < results.length; i++) {
            this.purgeablePinSource.push(
              this.toTabStruct(results[i].name, results[i].maxSize, results[i].minSize, results[i].avgSize)
            );
          }
          this.sortByColumn({ key: this.sortKey, sort: this.sortType });
          let total = this.totalData(this.purgeablePinSource);
          this.purgeablePinSource.unshift(total);
          this.purgeablePinTable!.recycleDataSource = this.purgeablePinSource;
          this.purgeablePinSource.shift();
        } else {
          this.purgeablePinSource = [];
          this.purgeablePinTable!.recycleDataSource = [];
        }
      });
    }
  }

  private init() {
    const thTable = this.tabTitle!.querySelector('.th');
    const purgePinTblNodes = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      purgePinTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
    this.sortKey = 'avgSize';
    this.sortType = 2;
  }

  private toTabStruct(type: string, maxPurgePinSize: number, minPurgePinSize: number, avgPurgePinSize: number
  ): PurgeableTabStruct {
    const tabStruct = new PurgeableTabStruct(
      type,
      maxPurgePinSize,
      minPurgePinSize,
      avgPurgePinSize,
      Utils.getBinaryByteWithUnit(avgPurgePinSize),
      Utils.getBinaryByteWithUnit(maxPurgePinSize),
      Utils.getBinaryByteWithUnit(minPurgePinSize)
    );
    return tabStruct;
  }

  private totalData(source: Array<PurgeableTabStruct>): PurgeableTabStruct {
    // 计算总的time作为表格的第一行显示
    let pinAvg = 0;
    let pinMax = 0;
    let pinMin = 0;
    for (let item of source) {
      pinAvg += item.avgSize;
      pinMax += item.maxSize;
      pinMin += item.minSize;
    }
    let pinData = this.toTabStruct('Total', pinAvg, pinMax, pinMin);
    return pinData;
  }

  private sortByColumn(detail: any): void {
    // @ts-ignore
    function compare(key, sort, type) {
      return function (purgePinLeftData: any, purgePinRightData: any) {
        // 不管哪一列的排序方式是0（默认排序），都按照avgSize列从大到小排序
        if (sort === 0) {
          sort = 2;
          key = 'avgSize';
          type = 'number';
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(purgePinRightData[key]) - parseFloat(purgePinLeftData[key]) : parseFloat(purgePinLeftData[key]) - parseFloat(purgePinRightData[key]);
        } else {
          if (sort === 2) {
            return purgePinRightData[key].toString().localeCompare(purgePinLeftData[key].toString());
          } else {
            return purgePinLeftData[key].toString().localeCompare(purgePinRightData[key].toString());
          }
        }
      };
    }
    if (detail.key === 'type') {
      this.purgeablePinSource.sort(compare(detail.key, detail.sort, 'string'));
    } else {
      this.purgeablePinSource.sort(compare(detail.key, detail.sort, 'number'));
    }
    let pin = this.totalData(this.purgeablePinSource);
    this.purgeablePinSource.unshift(pin);
    this.purgeablePinTable!.recycleDataSource = this.purgeablePinSource;
    this.purgeablePinSource.shift();
  }

  initElements(): void {
    this.purgeablePinTable = this.shadowRoot?.querySelector<LitTable>('#tb-purgeable-pin');
    this.tabTitle = this.purgeablePinTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.purgPinTimeRange = this.shadowRoot?.querySelector<HTMLLabelElement>('#purg-pin-time-range');
    this.purgeablePinTable!.addEventListener('column-click', (evt: any) => {
      this.sortKey = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.purgeablePinTable!);
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        .purg-pin-label{
            display: flex;
            height: 20px;
            align-items: center;
            flex-direction: row;
            margin-bottom: 5px;
        }
        #purg-pin-time-range{
            width: auto;
            text-align: end;
            font-size: 10pt;
        }
        </style>
        <div class="purg-pin-label">
            <div style="flex: 1"></div>
            <label id="purg-pin-time-range">Selected range:0.0 ms</label>
        </div>
        <lit-table id="tb-purgeable-pin" style="height: auto">
            <lit-table-column width="1fr" title="Type" data-index="type" key="type" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="AvgSize" data-index="avgSizeStr" key="avgSize" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="MaxSize" data-index="maxSizeStr" key="maxSize" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="MinSize" data-index="minSizeStr" key="minSize" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }
}
