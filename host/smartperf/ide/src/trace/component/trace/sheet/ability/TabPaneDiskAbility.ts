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
import { getTabDiskAbilityData } from '../../../../database/SqlLite.js';
import { SystemDiskIOSummary } from '../../../../bean/AbilityMonitor.js';
import { Utils } from '../../base/Utils.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-disk-ability')
export class TabPaneDiskAbility extends BaseElement {
  private diskAbilityTbl: LitTable | null | undefined;
  private diskAbilitySource: Array<SystemDiskIOSummary> = [];
  private queryDiskResult: Array<SystemDiskIOSummary> = [];
  private search: HTMLInputElement | undefined | null;

  set data(diskAbilityValue: SelectionParam | any) {
    // @ts-ignore
    this.diskAbilityTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 + 'px';
    this.queryDataByDB(diskAbilityValue);
  }

  initElements(): void {
    this.diskAbilityTbl = this.shadowRoot?.querySelector<LitTable>('#tb-disk-ability');
    this.diskAbilityTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.diskAbilityTbl!);
  }

  filterData() {
    if (this.queryDiskResult.length > 0) {
      let filterDisk = this.queryDiskResult.filter((item) => {
        let array = this.toDiskAbilityArray(item);
        let isInclude = array.filter((value) => value.indexOf(this.search!.value) > -1);
        return isInclude.length > 0;
      });
      if (filterDisk.length > 0) {
        this.diskAbilitySource = filterDisk;
        this.diskAbilityTbl!.recycleDataSource = this.diskAbilitySource;
      } else {
        this.diskAbilitySource = [];
        this.diskAbilityTbl!.recycleDataSource = [];
      }
    }
  }

  toDiskAbilityArray(systemDiskIOSummary: SystemDiskIOSummary): any[] {
    let array: Array<string> = [];
    array.push(systemDiskIOSummary.startTimeStr);
    array.push(systemDiskIOSummary.durationStr);
    array.push(systemDiskIOSummary.dataReadStr);
    array.push(systemDiskIOSummary.dataReadSecStr);
    array.push(systemDiskIOSummary.dataWriteStr);
    array.push(systemDiskIOSummary.readsInStr);
    array.push(systemDiskIOSummary.readsInSecStr);
    array.push(systemDiskIOSummary.writeOutStr);
    array.push(systemDiskIOSummary.writeOutSecStr);
    return array;
  }

  queryDataByDB(val: SelectionParam | any) {
    getTabDiskAbilityData(val.leftNs, val.rightNs).then((result) => {
      log('getTabDiskAbilityData result size : ' + result.length);
      if (result.length != null && result.length > 0) {
        for (const systemDiskIOSummary of result) {
          if (systemDiskIOSummary.startTime <= 0) {
            systemDiskIOSummary.startTimeStr = '0:000.000.000';
          } else {
            systemDiskIOSummary.startTimeStr = Utils.getTimeStampHMS(systemDiskIOSummary.startTime);
          }
          systemDiskIOSummary.durationStr = Utils.getDurString(systemDiskIOSummary.duration);
          systemDiskIOSummary.dataReadStr = systemDiskIOSummary.dataRead + 'KB';
          systemDiskIOSummary.dataReadSecStr = systemDiskIOSummary.dataReadSec + 'KB/S';
          systemDiskIOSummary.dataWriteStr = systemDiskIOSummary.dataWrite + 'KB';
          systemDiskIOSummary.dataWriteSecStr = systemDiskIOSummary.dataWriteSec + 'KB/S';
          systemDiskIOSummary.readsInStr = ColorUtils.formatNumberComma(systemDiskIOSummary.readsIn);
          systemDiskIOSummary.readsInSecStr = systemDiskIOSummary.readsInSec.toString();
          systemDiskIOSummary.writeOutStr = ColorUtils.formatNumberComma(systemDiskIOSummary.writeOut);
          systemDiskIOSummary.writeOutSecStr = systemDiskIOSummary.writeOutSec.toString();
        }
        this.diskAbilitySource = result;
        this.queryDiskResult = result;
        this.diskAbilityTbl!.recycleDataSource = result;
      } else {
        this.diskAbilitySource = [];
        this.queryDiskResult = [];
        this.diskAbilityTbl!.recycleDataSource = [];
      }
    });
  }

  initHtml(): string {
    return `
        <style>
        .disk-ability-table{
            height: auto;
        }
        :host{
            flex-direction: column;
            display: flex;
            padding: 10px 10px;
        }
        </style>
        <lit-table id="tb-disk-ability" class="disk-ability-table">
            <lit-table-column order width="1fr" title="StartTime" data-index="startTimeStr" key="startTimeStr" align="flex-start">
            </lit-table-column>
            <lit-table-column order width="1fr" title="Duration" data-index="durationStr" key="durationStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Data Read" data-index="dataReadStr" key="dataReadStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Data Read/sec" data-index="dataReadSecStr" key="dataReadSecStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Data Write" data-index="dataWriteStr" key="dataWriteStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Data Write/sec" data-index="dataWriteSecStr" key="dataWriteSecStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Reads In" data-index="readsIn" key="readsInStr" align="flex-startStr" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Reads In/sec" data-index="readsInSecStr" key="readsInSecStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Write Out" data-index="writeOutStr" key="writeOutStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="1fr" title="Write Out/sec" data-index="writeOutSecStr" key="writeOutSecStr" align="flex-start" >
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (diskAbilityLeftData: SystemDiskIOSummary, diskAbilityRightData: SystemDiskIOSummary) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(diskAbilityRightData[property]) - parseFloat(diskAbilityLeftData[property]) : parseFloat(diskAbilityLeftData[property]) - parseFloat(diskAbilityRightData[property]);
        } else if (type === 'durationStr') {
          return sort === 2
            ? diskAbilityRightData.duration - diskAbilityLeftData.duration
            : diskAbilityLeftData.duration - diskAbilityRightData.duration;
        } else if (type === 'dataReadStr') {
          return sort === 2
            ? diskAbilityRightData.dataRead - diskAbilityLeftData.dataRead
            : diskAbilityLeftData.dataRead - diskAbilityRightData.dataRead;
        } else if (type === 'dataReadSecStr') {
          return sort === 2
            ? diskAbilityRightData.dataReadSec - diskAbilityLeftData.dataReadSec
            : diskAbilityLeftData.dataReadSec - diskAbilityRightData.dataReadSec;
        } else if (type === 'dataWriteStr') {
          return sort === 2
            ? diskAbilityRightData.dataWrite - diskAbilityLeftData.dataWrite
            : diskAbilityLeftData.dataWrite - diskAbilityRightData.dataWrite;
        } else if (type === 'dataWriteSecStr') {
          return sort === 2
            ? diskAbilityRightData.dataWriteSec - diskAbilityLeftData.dataWriteSec
            : diskAbilityLeftData.dataWriteSec - diskAbilityRightData.dataWriteSec;
        } else if (type === 'readsInStr') {
          return sort === 2
            ? diskAbilityRightData.readsIn - diskAbilityLeftData.readsIn
            : diskAbilityLeftData.readsIn - diskAbilityRightData.readsIn;
        } else if (type === 'readsInSecStr') {
          return sort === 2
            ? diskAbilityRightData.readsInSec - diskAbilityLeftData.readsInSec
            : diskAbilityLeftData.readsInSec - diskAbilityRightData.readsInSec;
        } else if (type === 'writeOutStr') {
          return sort === 2
            ? diskAbilityRightData.writeOut - diskAbilityLeftData.writeOut
            : diskAbilityLeftData.writeOut - diskAbilityRightData.writeOut;
        } else if (type === 'writeOutSecStr') {
          return sort === 2
            ? diskAbilityRightData.writeOutSec - diskAbilityLeftData.writeOutSec
            : diskAbilityLeftData.writeOutSec - diskAbilityRightData.writeOutSec;
        } else {
          // @ts-ignore
          if (diskAbilityRightData[property] > diskAbilityLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (diskAbilityRightData[property] == diskAbilityLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (detail.key === 'startTime') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'string'));
    } else if (detail.key === 'durationStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'durationStr'));
    } else if (detail.key === 'dataReadStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'dataReadStr'));
    } else if (detail.key === 'dataReadSecStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'dataReadSecStr'));
    } else if (detail.key === 'dataWriteStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'dataWriteStr'));
    } else if (detail.key === 'dataWriteSecStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'dataWriteSecStr'));
    } else if (detail.key === 'readsInStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'readsInStr'));
    } else if (detail.key === 'readsInSecStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'readsInSecStr'));
    } else if (detail.key === 'writeOutStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'writeOutStr'));
    } else if (detail.key === 'writeOutSecStr') {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'writeOutSecStr'));
    } else {
      this.diskAbilitySource.sort(compare(detail.key, detail.sort, 'number'));
    }
    this.diskAbilityTbl!.recycleDataSource = this.diskAbilitySource;
  }
}
