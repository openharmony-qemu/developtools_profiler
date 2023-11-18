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
import { resizeObserver } from '../SheetUtils.js';
import { getTabDmaVmTrackerData } from '../../../../database/SqlLite.js';
import { Dma } from '../../../../bean/AbilityMonitor.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-dma-vmtracker')
export class TabPaneDmaVmTracker extends BaseElement {
  private dmaTbl: LitTable | null | undefined;
  private dmaSource: Array<Dma> = [];
  private tableThead: HTMLDivElement | undefined | null;
  private dmaTimeRange: HTMLDivElement | undefined | null;

  set data(dmaValue: SelectionParam | any) {
    if (dmaValue.dmaVmTrackerData.length > 0) {
      this.dmaTimeRange!.textContent =
        'Selected range: ' + ((dmaValue.rightNs - dmaValue.leftNs) / 1000000.0).toFixed(5) + ' ms';
      this.dmaTbl!.loading = true;
      this.queryDataByDB(dmaValue);
      this.init();
    }
  }

  initElements(): void {
    this.dmaTbl = this.shadowRoot?.querySelector<LitTable>('#damTable');
    this.tableThead = this.dmaTbl?.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.dmaTimeRange = this.shadowRoot?.querySelector('#dma-time-range') as HTMLDivElement;
    this.dmaTbl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortDmaByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.dmaTbl!, 18);
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const dmaVmTrackerTblNodes = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      dmaVmTrackerTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  queryDataByDB(val: SelectionParam | any): void {
    getTabDmaVmTrackerData(
      val.leftNs,
      val.rightNs,
      MemoryConfig.getInstance().iPid,
      (MemoryConfig.getInstance().interval * 1000000) / 5
    ).then((data) => {
      this.dmaTbl!.loading = false;
      if (data.length !== null && data.length > 0) {
        data.forEach((item) => {
          item.avgSizes = Utils.getBinaryByteWithUnit(Math.round(item.avgSize));
          item.minSizes = Utils.getBinaryByteWithUnit(item.minSize);
          item.maxSizes = Utils.getBinaryByteWithUnit(item.maxSize);
        });
        this.dmaSource = data;
        this.dmaTbl!.recycleDataSource = this.dmaSource.sort(function (dmaVmLeftData: Dma, dmaVmRightData: Dma) {
          return dmaVmRightData.avgSize - dmaVmLeftData.avgSize;
        });
      } else {
        this.dmaTbl!.recycleDataSource = [];
        this.dmaSource = [];
      }
    });
  }

  initHtml(): string {
    return `
    <style>
        .dma-table{
        flex-direction: row;
        margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
    </style>
    <div class="dma-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
        <div style="flex: 1"></div>
        <label id="dma-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
    </div>
    <div style="overflow: auto">
    <lit-table id="damTable" class="damTable">
        <lit-table-column order title="AvgSize" data-index="avgSizes" key="avgSize" align="flex-start" width="1fr" >
        </lit-table-column>
        <lit-table-column order title="MinSize" data-index="minSizes" key="minSize" align="flex-start" width="1fr" >
        </lit-table-column>
        <lit-table-column order title="MaxSize" data-index="maxSizes" key="maxSize" align="flex-start" width="1fr" >
        </lit-table-column>
    </lit-table>
    </div>
            `;
  }

  sortDmaByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.dmaTbl!.recycleDataSource = this.dmaSource;
        break;
      default:
        let array = [...this.dmaSource];
        switch (column) {
          case 'avgSize':
            this.dmaTbl!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? dmaVmLeftData.avgSize - dmaVmRightData.avgSize
                : dmaVmRightData.avgSize - dmaVmLeftData.avgSize;
            });
            break;
          case 'minSize':
            this.dmaTbl!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? dmaVmLeftData.minSize - dmaVmRightData.minSize
                : dmaVmRightData.minSize - dmaVmLeftData.minSize;
            });
            break;
          case 'maxSize':
            this.dmaTbl!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? dmaVmLeftData.maxSize - dmaVmRightData.maxSize
                : dmaVmRightData.maxSize - dmaVmLeftData.maxSize;
            });
            break;
        }
        break;
    }
  }
}
