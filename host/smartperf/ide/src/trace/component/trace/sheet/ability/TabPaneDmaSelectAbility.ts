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
import { getTabDmaAbilityClickData } from '../../../../database/SqlLite.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-dma-selection-ability')
export class TabPaneDmaSelectAbility extends BaseElement {
  private damClickTable: LitTable | null | undefined;
  private dmaClickSource: Array<Dma> = [];
  private tableThead: HTMLDivElement | undefined | null;
  private table: HTMLDivElement | undefined | null;

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
      if (this.parentElement?.clientHeight !== 0) {
        // @ts-ignore
        this.damClickTable?.shadowRoot?.querySelector('.table').style.height = (this.parentElement?.clientHeight - 18) + 'px';
        this.parentElement!.style.overflow = 'hidden';
        this.damClickTable?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  private init(): void {
    const thTable = this.tableThead!.querySelector('.th');
    const dmaSelectTblNodes = thTable!.querySelectorAll('div');
    if (this.tableThead!.hasAttribute('sort')) {
      this.tableThead!.removeAttribute('sort');
      dmaSelectTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  queryDmaClickDataByDB(startNs: number): void {
    this.init();
    getTabDmaAbilityClickData(startNs).then((data) => {
      if (data.length !== null && data.length > 0) {
        data.forEach((item) => {
          item.bufName = SpSystemTrace.DATA_DICT.get(item.bufName as number) || '-';
          item.expName = SpSystemTrace.DATA_DICT.get(item.expName as number) || '-';
          item.expTaskComm = SpSystemTrace.DATA_DICT.get(item.expTaskComm as number) || '-';
          if (item.processName !== null) {
            item.process = `${item.processName}(${item.processId})`;
          } else {
            item.process = `Process(${item.processId})`;
          }
          item.sizes = Utils.getBinaryByteWithUnit(item.size);
          item.timeStamp = ns2s(item.startNs);
          this.damClickTable!.getItemTextColor = (dmaItem: Dma): any => {
            if (dmaItem.flag === 1) {
              return '#d4b550';
            } else if (dmaItem.flag === 2) {
              return '#f86b6b';
            } else {
              return '#000000';
            }
          };
        });
        this.damClickTable!.snapshotDataSource = data.sort(function (
          dmaAbilityLeftData: Dma,
          dmaAbilityRightData: Dma
        ) {
          return dmaAbilityRightData.size - dmaAbilityLeftData.size;
        });
        this.dmaClickSource = data;
      } else {
        this.damClickTable!.snapshotDataSource = [];
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
    <lit-table-column order title="Process(pid)" data-index="process" key="process" align="flex-start" width="2fr" >
    </lit-table-column>
    <lit-table-column order title="Fd" data-index="fd" key="fd" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Size" data-index="sizes" key="size" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="Ino" data-index="ino" key="ino" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="ExpPid" data-index="expPid" key="expPid" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="ExpTaskComm" data-index="expTaskComm" key="expTaskComm" align="flex-start" width="2fr" >
    </lit-table-column>
    <lit-table-column order title="BufName" data-index="bufName" key="bufName" align="flex-start" width="2fr" >
    </lit-table-column>
    <lit-table-column order title="ExpName" data-index="expName" key="expName" align="flex-start" width="2fr" >
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
          case 'process':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? `${dmaAbilityLeftData.process}`.localeCompare(`${dmaAbilityRightData.process}`)
                : `${dmaAbilityRightData.process}`.localeCompare(`${dmaAbilityLeftData.process}`);
            });
            break;
          case 'startNs':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.startNs - dmaAbilityRightData.startNs
                : dmaAbilityRightData.startNs - dmaAbilityLeftData.startNs;
            });
            break;
          case 'expTaskComm':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? `${dmaAbilityLeftData.expTaskComm}`.localeCompare(`${dmaAbilityRightData.expTaskComm}`)
                : `${dmaAbilityRightData.expTaskComm}`.localeCompare(`${dmaAbilityLeftData.expTaskComm}`);
            });
            break;
          case 'fd':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.fd - dmaAbilityRightData.fd
                : dmaAbilityRightData.fd - dmaAbilityLeftData.fd;
            });
            break;
          case 'size':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.size - dmaAbilityRightData.size
                : dmaAbilityRightData.size - dmaAbilityLeftData.size;
            });
            break;
          case 'ino':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.ino - dmaAbilityRightData.ino
                : dmaAbilityRightData.ino - dmaAbilityLeftData.ino;
            });
            break;
          case 'expPid':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.expPid - dmaAbilityRightData.expPid
                : dmaAbilityRightData.expPid - dmaAbilityLeftData.expPid;
            });
            break;
          case 'flag':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? dmaAbilityLeftData.flag - dmaAbilityRightData.flag
                : dmaAbilityRightData.flag - dmaAbilityLeftData.flag;
            });
            break;
          case 'bufName':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? `${dmaAbilityLeftData.bufName}`.localeCompare(`${dmaAbilityRightData.bufName}`)
                : `${dmaAbilityRightData.bufName}`.localeCompare(`${dmaAbilityLeftData.bufName}`);
            });
            break;
          case 'expName':
            this.damClickTable!.recycleDataSource = array.sort((dmaAbilityLeftData, dmaAbilityRightData) => {
              return sort === 1
                ? `${dmaAbilityLeftData.expName}`.localeCompare(`${dmaAbilityRightData.expName}`)
                : `${dmaAbilityRightData.expName}`.localeCompare(`${dmaAbilityLeftData.expName}`);
            });
            break;
        }
        break;
    }
  }
}
