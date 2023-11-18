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
import { querySysPurgeableTab, queryProcessPurgeableTab } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-purg-total')
export class TabPanePurgTotal extends BaseElement {
  private purgeableTotalTable: LitTable | null | undefined;
  private purgeableTotalSource: Array<PurgeableTabStruct> = [];
  private tabTitle: HTMLDivElement | undefined | null;
  private purgTotalTimeRange: HTMLLabelElement | undefined | null;
  private sortKey = 'avgSize';
  private sortType = 2;
  set data(selection: SelectionParam) {
    //@ts-ignore
    this.purgeableTotalTable?.shadowRoot?.querySelector('.table')?.style?.height = `${
      this.parentElement!.clientHeight - 45
    }px`;
    this.init();
    this.purgTotalTimeRange!.textContent =
      'Selected range: ' + ((selection.rightNs - selection.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.purgeableTotalTable!.loading = true;
    this.purgeableTotalTable!.recycleDataSource = [];
    // 框选了 Purgeable Total
    if (selection.purgeableTotalAbility.length > 0) {
      this.purgeableTotalSource = [];
      querySysPurgeableTab(
        selection.leftNs,
        selection.rightNs,
        (MemoryConfig.getInstance().interval * 1000000) / 5
      ).then((purgeTotalResults) => {
        this.purgeableTotalTable!.loading = false;
        if (purgeTotalResults.length > 0) {
          for (let i = 0; i < purgeTotalResults.length; i++) {
            this.purgeableTotalSource.push(
              this.toTabStruct(purgeTotalResults[i].name, purgeTotalResults[i].maxSize, purgeTotalResults[i].minSize, purgeTotalResults[i].avgSize)
            );
          }
          this.sortByColumn({ key: this.sortKey, sort: this.sortType });
          let total = this.totalData(this.purgeableTotalSource);
          this.purgeableTotalSource.unshift(total);
          this.purgeableTotalTable!.recycleDataSource = this.purgeableTotalSource;
          this.purgeableTotalSource.shift();
        } else {
          this.purgeableTotalSource = [];
          this.purgeableTotalTable!.recycleDataSource = [];
        }
      });
    } else if (selection.purgeableTotalVM.length > 0) {
      this.purgeableTotalSource = [];
      queryProcessPurgeableTab(
        selection.leftNs,
        selection.rightNs,
        (MemoryConfig.getInstance().interval * 1000000) / 5,
        MemoryConfig.getInstance().iPid
      ).then((results) => {
        this.purgeableTotalTable!.loading = false;
        if (results.length > 0) {
          for (let i = 0; i < results.length; i++) {
            this.purgeableTotalSource.push(
              this.toTabStruct(results[i].name, results[i].maxSize, results[i].minSize, results[i].avgSize)
            );
          }
          this.sortByColumn({ key: this.sortKey, sort: this.sortType });
          let total = this.totalData(this.purgeableTotalSource);
          this.purgeableTotalSource.unshift(total);
          this.purgeableTotalTable!.recycleDataSource = this.purgeableTotalSource;
          this.purgeableTotalSource.shift();
        } else {
          this.purgeableTotalSource = [];
          this.purgeableTotalTable!.recycleDataSource = [];
        }
      });
    }
  }

  private init() {
    const thTable = this.tabTitle!.querySelector('.th');
    const purgeTotalTblNode = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      purgeTotalTblNode.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
    this.sortKey = 'avgSize';
    this.sortType = 2;
  }

  private toTabStruct(type: string, maxSize: number, minSize: number, avgSize: number): PurgeableTabStruct {
    const tabStruct = new PurgeableTabStruct(
      type,
      maxSize,
      minSize,
      avgSize,
      Utils.getBinaryByteWithUnit(avgSize),
      Utils.getBinaryByteWithUnit(maxSize),
      Utils.getBinaryByteWithUnit(minSize)
    );
    return tabStruct;
  }

  private totalData(source: Array<PurgeableTabStruct>): PurgeableTabStruct {
    // 计算总的time作为表格的第一行显示
    let totalAvg = 0;
    let totalMax = 0;
    let totalMin = 0;
    for (let item of source) {
      totalAvg += item.avgSize;
      totalMax += item.maxSize;
      totalMin += item.minSize;
    }
    let totalData = this.toTabStruct('Total', totalAvg, totalMax, totalMin);
    return totalData;
  }

  private sortByColumn(detail: any): void {
    // @ts-ignore
    function compare(key, sort, type) {
      return function (purgeTotalLeftData: any, purgeTotalRightData: any) {
        // 不管哪一列的排序方式是0（默认排序），都按照avgSize列从大到小排序
        if (sort === 0) {
          sort = 2;
          key = 'avgSize';
          type = 'number';
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(purgeTotalRightData[key]) - parseFloat(purgeTotalLeftData[key]) : parseFloat(purgeTotalLeftData[key]) - parseFloat(purgeTotalRightData[key]);
        } else {
          if (sort === 2) {
            return purgeTotalRightData[key].toString().localeCompare(purgeTotalLeftData[key].toString());
          } else {
            return purgeTotalLeftData[key].toString().localeCompare(purgeTotalRightData[key].toString());
          }
        }
      };
    }
    if (detail.key === 'type') {
      this.purgeableTotalSource.sort(compare(detail.key, detail.sort, 'string'));
    } else {
      this.purgeableTotalSource.sort(compare(detail.key, detail.sort, 'number'));
    }
    let total = this.totalData(this.purgeableTotalSource);
    this.purgeableTotalSource.unshift(total);
    this.purgeableTotalTable!.recycleDataSource = this.purgeableTotalSource;
    this.purgeableTotalSource.shift();
  }

  initElements(): void {
    this.purgeableTotalTable = this.shadowRoot?.querySelector<LitTable>('#tb-purgeable-total');
    this.tabTitle = this.purgeableTotalTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.purgTotalTimeRange = this.shadowRoot?.querySelector('#purg-total-time-range') as HTMLLabelElement;
    this.purgeableTotalTable!.addEventListener('column-click', (evt: any) => {
      this.sortKey = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.purgeableTotalTable!);
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        .purg-total-label{
            display: flex;
            height: 20px;
            align-items: center;
            flex-direction: row;
            margin-bottom: 5px;
        }
        #purg-total-time-range{
            width: auto;
            text-align: end;
            font-size: 10pt;
        }
        </style>
        <div class="purg-total-label">
            <div style="flex: 1"></div>
            <label id="purg-total-time-range">Selected range:0.0 ms</label>
        </div>
        <lit-table id="tb-purgeable-total" style="height: auto">
            <lit-table-column width="1fr" title="Type" data-index="type" key="type"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="AvgSize" data-index="avgSizeStr" key="avgSize"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="MaxSize" data-index="maxSizeStr" key="maxSize"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="MinSize" data-index="minSizeStr" key="minSize"  align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }
}

export class PurgeableTabStruct {
  type: string;
  avgSize: number;
  maxSize: number;
  minSize: number;
  avgSizeStr: string;
  maxSizeStr: string;
  minSizeStr: string;

  constructor(
    type: string,
    avgSize: number,
    maxSize: number,
    minSize: number,
    avgSizeStr: string,
    maxSizeStr: string,
    minSizeStr: string
  ) {
    this.type = type;
    this.avgSize = avgSize;
    this.maxSize = maxSize;
    this.minSize = minSize;
    this.avgSizeStr = avgSizeStr;
    this.maxSizeStr = maxSizeStr;
    this.minSizeStr = minSizeStr;
  }
}
