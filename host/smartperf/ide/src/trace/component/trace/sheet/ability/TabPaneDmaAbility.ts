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
import { Dma } from '../../../../bean/AbilityMonitor.js';
import { resizeObserver } from '../SheetUtils.js';
import { getTabDmaAbilityData } from '../../../../database/SqlLite.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-dma-ability')
export class TabPaneDmaAbility extends BaseElement {
  private dmaTbl: LitTable | null | undefined;
  private dmaSource: Array<Dma> = [];
  private tableThead: HTMLDivElement | undefined | null;
  private dmaTimeRange: HTMLLabelElement | null | undefined;
  private total: Dma = new Dma();

  set data(dmaAbilityValue: SelectionParam | any) {
    if (dmaAbilityValue.dmaAbilityData.length > 0) {
      this.init();
      this.dmaTimeRange!.textContent =
        'Selected range: ' + ((dmaAbilityValue.rightNs - dmaAbilityValue.leftNs) / 1000000.0).toFixed(5) + ' ms';
      this.dmaTbl!.loading = true;
      this.queryDataByDB(dmaAbilityValue);
    }
  }

  initElements(): void {
    this.dmaTbl = this.shadowRoot?.querySelector<LitTable>('#damTable');
    this.tableThead = this.dmaTbl?.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.dmaTimeRange = this.shadowRoot?.querySelector<HTMLLabelElement>('#dma-time-range');
    this.dmaTbl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortDmaByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.dmaTbl!);
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      list.forEach((item) => {
        item.querySelectorAll('svg').forEach((svgEl) => {
          svgEl.style.display = 'none';
        });
      });
    }
  }

  queryDataByDB(val: SelectionParam | any): void {
    getTabDmaAbilityData(val.leftNs, val.rightNs, (MemoryConfig.getInstance().interval * 1000000) / 5).then((data) => {
      this.dmaSource = data;
      this.dmaTbl!.loading = false;
      if (data.length !== null && data.length > 0) {
        this.total = new Dma();
        this.total.process = '*All*';
        data.forEach((dmaItem) => {
          if (dmaItem.processName !== null) {
            dmaItem.process = `${dmaItem.processName}(${dmaItem.processId})`;
          } else {
            dmaItem.process = `Process(${dmaItem.processId})`;
          }

          this.total.avgSize += dmaItem.avgSize;
          if (this.total.minSize < 0) {
            this.total.minSize = dmaItem.minSize;
          }
          if (this.total.maxSize < 0) {
            this.total.maxSize = dmaItem.maxSize;
          }
          this.total.minSize = Math.min(this.total.minSize, dmaItem.minSize);
          this.total.maxSize = Math.max(this.total.maxSize, dmaItem.maxSize);

          dmaItem.avgSizes = Utils.getBinaryByteWithUnit(Math.round(dmaItem.avgSize));
          dmaItem.minSizes = Utils.getBinaryByteWithUnit(dmaItem.minSize);
          dmaItem.maxSizes = Utils.getBinaryByteWithUnit(dmaItem.maxSize);
        });
        this.total.avgSizes = Utils.getBinaryByteWithUnit(Math.round(this.total.avgSize / data.length));
        this.total.minSizes = Utils.getBinaryByteWithUnit(this.total.minSize);
        this.total.maxSizes = Utils.getBinaryByteWithUnit(this.total.maxSize);
        this.dmaSource.sort(function (dmaAbilityLeftData: Dma, dmaAbilityRightData: Dma) {
          return dmaAbilityRightData.avgSize - dmaAbilityLeftData.avgSize;
        });
        this.dmaTbl!.recycleDataSource = [this.total, ...this.dmaSource];
      } else {
        this.dmaTbl!.recycleDataSource = [];
        this.dmaSource = [];
      }
    });
  }

  initHtml(): string {
    return `
        <style>
        .dma-label{
            flex-direction: row;
            margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="dma-label" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <div style="flex: 1"></div>
            <label id="dma-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
        <lit-table id="damTable" class="damTable">
            <lit-table-column order title="Process" data-index="process" key="process" align="flex-start" width="2fr" >
            </lit-table-column>
            <lit-table-column order title="AvgSize" data-index="avgSizes" key="avgSize" align="flex-start" width="1fr" >
            </lit-table-column>
            <lit-table-column order title="MaxSize" data-index="maxSizes" key="maxSize" align="flex-start" width="1fr" >
            </lit-table-column>
            <lit-table-column order title="MinSize" data-index="minSizes" key="minSize" align="flex-start" width="1fr" >
            </lit-table-column>
        </lit-table>
        </div>
        `;
  }

  sortDmaByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.dmaTbl!.recycleDataSource = [this.total, ...this.dmaSource];
        break;
      default:
        let array = [...this.dmaSource];
        switch (column) {
          case 'process':
            array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? `${dmaAbilityLeftData.process}`.localeCompare(`${dmaAbilityRightData.process}`)
                : `${dmaAbilityRightData.process}`.localeCompare(`${dmaAbilityLeftData.process}`);
            });
            break;
          case 'avgSize':
            array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.avgSize - dmaAbilityRightData.avgSize
                : dmaAbilityRightData.avgSize - dmaAbilityLeftData.avgSize;
            });
            break;
          case 'minSize':
            array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.minSize - dmaAbilityRightData.minSize
                : dmaAbilityRightData.minSize - dmaAbilityLeftData.minSize;
            });
            break;
          case 'maxSize':
            array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.maxSize - dmaAbilityRightData.maxSize
                : dmaAbilityRightData.maxSize - dmaAbilityLeftData.maxSize;
            });
            break;
        }
        this.dmaTbl!.recycleDataSource = [this.total, ...array];
        break;
    }
  }
}
