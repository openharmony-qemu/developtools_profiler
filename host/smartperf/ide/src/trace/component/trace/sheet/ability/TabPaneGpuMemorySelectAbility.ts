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
import { GpuMemory } from '../../../../bean/AbilityMonitor.js';
import { getTabGpuMemoryAbilityClickData } from '../../../../database/SqlLite.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-gpu-memory-selection-ability')
export class TabPaneGpuMemorySelectAbility extends BaseElement {
  private gpuMemoryClickTable: LitTable | null | undefined;
  private gpuMemoryClickSource: Array<GpuMemory> = [];
  private tableThead: HTMLDivElement | undefined | null;

  set data(data: number) {
    // @ts-ignore
    this.queryDataByDB(data);
    this.init();
  }

  initElements(): void {
    this.gpuMemoryClickTable = this.shadowRoot?.querySelector<LitTable>('#gpuMemoryClickTable');
    this.tableThead = this.gpuMemoryClickTable?.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.gpuMemoryClickTable!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortGpuMemoryByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight !== 0) {
        let gpuMemoryTbl = this.gpuMemoryClickTable?.shadowRoot?.querySelector('.table');
        // @ts-ignore
        gpuMemoryTbl.style.height = this.parentElement.clientHeight - 18 + 'px';
        this.parentElement!.style.overflow = 'hidden';
        this.gpuMemoryClickTable?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const gpuMemorySelectTblNode = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      gpuMemorySelectTblNode.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  queryGpuMemoryClickDataByDB(startNs: number): void {
    this.init();
    getTabGpuMemoryAbilityClickData(startNs).then((data) => {
      if (data.length !== null && data.length > 0) {
        data.forEach((item) => {
          if (item.processName !== null) {
            item.process = `${item.processName}(${item.processId})`;
          } else {
            item.process = `Process(${item.processId})`;
          }
          item.sizes = Utils.getBinaryByteWithUnit(item.size);
          item.timeStamp = ns2s(item.startNs);
        });
        this.gpuMemoryClickTable!.recycleDataSource = data.sort(function (
          gpuMemoryLeftData: GpuMemory,
          gpuMemoryRightData: GpuMemory
        ) {
          return gpuMemoryRightData.size - gpuMemoryLeftData.size;
        });
        this.gpuMemoryClickSource = data;
      } else {
        this.gpuMemoryClickTable!.recycleDataSource = [];
        this.gpuMemoryClickSource = [];
      }
    });
  }

  initHtml(): string {
    return `
<style>
.gpuMemoryClickTable{
    height: auto;
}
:host{
    display: flex;
    flex-direction: column;
    padding: 10px 10px;
}
</style>
<lit-table id="gpuMemoryClickTable" class="gpuMemoryClickTable">
    <lit-table-column order title="TimeStamp" data-index="timeStamp" key="startNs" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="GpuName" data-index="gpuName" key="gpuName" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Process(pid)" data-index="process" key="process" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Size" data-index="sizes" key="size" align="flex-start" width="1fr" >
    </lit-table-column>
</lit-table>
        `;
  }

  sortGpuMemoryByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.gpuMemoryClickTable!.recycleDataSource = this.gpuMemoryClickSource;
        break;
      default:
        let array = [...this.gpuMemoryClickSource];
        switch (column) {
          case 'process':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? `${gpuMemoryLeftData.process}`.localeCompare(`${gpuMemoryRightData.process}`)
                : `${gpuMemoryRightData.process}`.localeCompare(`${gpuMemoryLeftData.process}`);
            });
            break;
          case 'startNs':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? gpuMemoryLeftData.startNs - gpuMemoryRightData.startNs
                : gpuMemoryRightData.startNs - gpuMemoryLeftData.startNs;
            });
            break;
          case 'gpuName':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? `${gpuMemoryLeftData.gpuName}`.localeCompare(`${gpuMemoryRightData.gpuName}`)
                : `${gpuMemoryRightData.gpuName}`.localeCompare(`${gpuMemoryLeftData.gpuName}`);
            });
            break;
          case 'size':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort((gpuMemoryLeftData, gpuMemoryRightData) => {
              return sort === 1
                ? gpuMemoryLeftData.size - gpuMemoryRightData.size
                : gpuMemoryRightData.size - gpuMemoryLeftData.size;
            });
            break;
        }
        break;
    }
  }
}
