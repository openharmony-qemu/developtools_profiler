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
import { getTabGpuMemoryData } from '../../../../database/SqlLite.js';
import { GpuMemory } from '../../../../bean/AbilityMonitor.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { Utils } from '../../base/Utils.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';

@element('tabpane-gpu-memory-vmtracker')
export class TabPaneGpuMemoryVmTracker extends BaseElement {
  private gpuMemoryTableTbl: LitTable | null | undefined;
  private gpuMemorySource: Array<GpuMemory> = [];
  private tableThead: HTMLDivElement | undefined | null;
  private gpuMemoryTimeRange: HTMLDivElement | undefined | null;
  private total: GpuMemory = new GpuMemory();

  set data(gpuMemoryValue: SelectionParam | any) {
    if (gpuMemoryValue.gpuMemoryTrackerData.length > 0) {
      this.gpuMemoryTimeRange!.textContent =
        'Selected range: ' + ((gpuMemoryValue.rightNs - gpuMemoryValue.leftNs) / 1000000.0).toFixed(5) + ' ms';
      this.gpuMemoryTableTbl!.loading = true;
      this.queryDataByDB(gpuMemoryValue);
      this.init();
    }
  }

  initElements(): void {
    this.gpuMemoryTableTbl = this.shadowRoot?.querySelector<LitTable>('#damTable');
    this.tableThead = this.gpuMemoryTableTbl?.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.gpuMemoryTimeRange = this.shadowRoot?.querySelector('#gpu-memory-time-range') as HTMLDivElement;
    this.gpuMemoryTableTbl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortGpuMemoryByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.gpuMemoryTableTbl!);
  }

  queryDataByDB(val: SelectionParam | any): void {
    getTabGpuMemoryData(
      val.leftNs,
      val.rightNs,
      MemoryConfig.getInstance().iPid,
      MemoryConfig.getInstance().snapshotDur
    ).then((data) => {
      this.gpuMemoryTableTbl!.loading = false;
      if (data.length !== null && data.length > 0) {
        this.total = new GpuMemory();
        this.total.thread = '*All*';
        this.total.gpuName = '*All*';
        data.forEach((item) => {
          if (item.threadName !== null) {
            item.thread = `${item.threadName}(${item.threadId})`;
          } else {
            item.thread = `Thread(${item.threadId})`;
          }
          this.total.avgSize += item.avgSize;
          if (this.total.minSize < 0) {
            this.total.minSize = item.minSize;
          }
          if (this.total.maxSize < 0) {
            this.total.maxSize = item.maxSize;
          }
          this.total.minSize = Math.min(this.total.minSize, item.minSize);
          this.total.maxSize = Math.max(this.total.maxSize, item.maxSize);

          item.gpuName = SpSystemTrace.DATA_DICT.get(item.gpuNameId) || '';
          item.avgSizes = Utils.getBinaryByteWithUnit(Math.round(item.avgSize));
          item.minSizes = Utils.getBinaryByteWithUnit(item.minSize);
          item.maxSizes = Utils.getBinaryByteWithUnit(item.maxSize);
        });
        this.total.avgSizes = Utils.getBinaryByteWithUnit(Math.round(this.total.avgSize / data.length));
        this.total.minSizes = Utils.getBinaryByteWithUnit(this.total.minSize);
        this.total.maxSizes = Utils.getBinaryByteWithUnit(this.total.maxSize);
        this.gpuMemorySource = data;
        this.gpuMemorySource.sort(function (gpuMemoryLeftData: GpuMemory, gpuMemoryRightData: GpuMemory) {
          return gpuMemoryRightData.avgSize - gpuMemoryLeftData.avgSize;
        });
        this.gpuMemoryTableTbl!.recycleDataSource = [...this.gpuMemorySource];
      } else {
        this.gpuMemoryTableTbl!.recycleDataSource = [];
        this.gpuMemorySource = [];
      }
    });
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      list.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  initHtml(): string {
    return `
    <style>
    .gpu-memory-table{
        flex-direction: row;
        margin-bottom: 5px;
    }
    :host{
        display: flex;
        flex-direction: column;
        padding: 10px 10px;
    }
    </style>
    <div class="gpu-memory-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
        <div style="flex: 1"></div>
        <label id="gpu-memory-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
    </div>
    <div style="overflow: auto">
    <lit-table id="damTable" class="damTable">
        <lit-table-column order title="GpuName" data-index="gpuName" key="gpuName" align="flex-start" width="1fr" >
        </lit-table-column>
        <lit-table-column order title="Thread" data-index="thread" key="thread" align="flex-start" width="1fr" >
        </lit-table-column>
        <lit-table-column order title="AvgSize" data-index="avgSizes" key="avgSize" align="flex-start" width="1fr" >
        </lit-table-column>5
        <lit-table-column order title="MaxSize" data-index="maxSizes" key="maxSize" align="flex-start" width="1fr" >
        </lit-table-column>
        <lit-table-column order title="MinSize" data-index="minSizes" key="minSize" align="flex-start" width="1fr" >
        </lit-table-column>
    </lit-table>
    </div>
            `;
  }

  sortGpuMemoryByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.gpuMemoryTableTbl!.recycleDataSource = [...this.gpuMemorySource];
        break;
      default:
        let array = [...this.gpuMemorySource];
        switch (column) {
          case 'gpuName':
            array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? `${gpuMemoryLeftData.gpuName}`.localeCompare(`${gpuMemoryRightData.gpuName}`)
                : `${gpuMemoryRightData.gpuName}`.localeCompare(`${gpuMemoryLeftData.gpuName}`);
            });
            break;
          case 'avgSize':
            array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? gpuMemoryLeftData.avgSize - gpuMemoryRightData.avgSize
                : gpuMemoryRightData.avgSize - gpuMemoryLeftData.avgSize;
            });
            break;
          case 'minSize':
            array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? gpuMemoryLeftData.minSize - gpuMemoryRightData.minSize
                : gpuMemoryRightData.minSize - gpuMemoryLeftData.minSize;
            });
            break;
          case 'maxSize':
            array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? gpuMemoryLeftData.maxSize - gpuMemoryRightData.maxSize
                : gpuMemoryRightData.maxSize - gpuMemoryLeftData.maxSize;
            });
            break;
          case 'thread':
            array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? `${gpuMemoryLeftData.thread}`.localeCompare(`${gpuMemoryRightData.thread}`)
                : `${gpuMemoryRightData.thread}`.localeCompare(`${gpuMemoryLeftData.thread}`);
            });
            break;
        }
        this.gpuMemoryTableTbl!.recycleDataSource = [...array];
        break;
    }
  }
}
