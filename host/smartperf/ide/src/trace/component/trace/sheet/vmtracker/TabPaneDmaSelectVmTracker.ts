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
import { Dma } from '../../../../bean/AbilityMonitor.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { getTabDmaVMTrackerClickData } from '../../../../database/SqlLite.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';
import { ns2s } from '../../TimerShaftElement.js';

@element('tabpane-dma-selection-vmtracker')
export class TabPaneDmaSelectVmTracker extends BaseElement {
  private damClickTable: LitTable | null | undefined;
  private dmaClickSource: Array<Dma> = [];
  private tableThead: HTMLDivElement | undefined | null;

  initElements(): void {
    this.damClickTable = this.shadowRoot?.querySelector<LitTable>('#damClickTable');
    this.tableThead = this.damClickTable?.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.damClickTable!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortDmaByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.damClickTable?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 18 + 'px';
        this.parentElement!.style.overflow = 'hidden';
        this.damClickTable?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const dmaSelectVmTrackerTblNodes = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      dmaSelectVmTrackerTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  queryDmaVmTrackerClickDataByDB(startNs: number): void {
    this.init();
    getTabDmaVMTrackerClickData(startNs, MemoryConfig.getInstance().iPid).then((data) => {
      if (data.length !== null && data.length > 0) {
        data.forEach((item) => {
          item.bufName = SpSystemTrace.DATA_DICT.get(item.bufName as number) || '-';
          item.expName = SpSystemTrace.DATA_DICT.get(item.expName as number) || '-';
          item.expTaskComm = SpSystemTrace.DATA_DICT.get(item.expTaskComm as number) || '-';
          item.timeStamp = ns2s(item.startNs);
          item.sizes = Utils.getBinaryByteWithUnit(item.size);
          this.damClickTable!.getItemTextColor = (item: Dma): any => {
            if (item.flag === 1) {
              return '#d4b550';
            } else if (item.flag === 2) {
              return '#f86b6b';
            } else {
              return '#000000';
            }
          };
        });
        this.damClickTable!.recycleDataSource = data.sort(function (dmaVmLeftData: Dma, dmaVmRightData: Dma) {
          return dmaVmRightData.size - dmaVmLeftData.size;
        });
        this.dmaClickSource = data;
      } else {
        this.damClickTable!.recycleDataSource = [];
        this.dmaClickSource = [];
      }
    });
  }

  initHtml(): string {
    return `
<style>
.damClickTable{
    height: auto;
}
:host{
    display: flex;
    flex-direction: column;
    padding: 10px 10px;
}
</style>
<lit-table id="damClickTable" class="damClickTable">
    <lit-table-column order title="TimeStamp" data-index="timeStamp" key="startNs" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Fd" data-index="fd" key="fd" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Size" data-index="sizes" key="size" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Ino" data-index="ino" key="ino" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="ExpPid" data-index="expPid" key="expPid" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="ExpTaskComm" data-index="expTaskComm" key="expTaskComm" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="BufName" data-index="bufName" key="bufName" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="ExpName" data-index="expName" key="expName" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Flag" data-index="flag" key="flag" align="flex-start" width="1fr" >
    </lit-table-column>
</lit-table>
        `;
  }

  sortDmaByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.damClickTable!.recycleDataSource = this.dmaClickSource;
        break;
      default:
        let array = [...this.dmaClickSource];
        switch (column) {
          case 'startNs':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? dmaVmLeftData.startNs - dmaVmRightData.startNs
                : dmaVmRightData.startNs - dmaVmLeftData.startNs;
            });
            break;
          case 'expTaskComm':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? `${dmaVmLeftData.expTaskComm}`.localeCompare(`${dmaVmRightData.expTaskComm}`)
                : `${dmaVmRightData.expTaskComm}`.localeCompare(`${dmaVmLeftData.expTaskComm}`);
            });
            break;
          case 'fd':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1 ? dmaVmLeftData.fd - dmaVmRightData.fd : dmaVmRightData.fd - dmaVmLeftData.fd;
            });
            break;
          case 'size':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1 ? dmaVmLeftData.size - dmaVmRightData.size : dmaVmRightData.size - dmaVmLeftData.size;
            });
            break;
          case 'ino':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1 ? dmaVmLeftData.ino - dmaVmRightData.ino : dmaVmRightData.ino - dmaVmLeftData.ino;
            });
            break;
          case 'expPid':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? dmaVmLeftData.expPid - dmaVmRightData.expPid
                : dmaVmRightData.expPid - dmaVmLeftData.expPid;
            });
            break;
          case 'flag':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1 ? dmaVmLeftData.flag - dmaVmRightData.flag : dmaVmRightData.flag - dmaVmLeftData.flag;
            });
            break;
          case 'bufName':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? `${dmaVmLeftData.bufName}`.localeCompare(`${dmaVmRightData.bufName}`)
                : `${dmaVmRightData.bufName}`.localeCompare(`${dmaVmLeftData.bufName}`);
            });
            break;
          case 'expName':
            this.damClickTable!.recycleDataSource = array.sort((dmaVmLeftData, dmaVmRightData) => {
              return sort === 1
                ? `${dmaVmLeftData.expName}`.localeCompare(`${dmaVmRightData.expName}`)
                : `${dmaVmRightData.expName}`.localeCompare(`${dmaVmLeftData.expName}`);
            });
            break;
        }
        break;
    }
  }
}
