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
import { getTabNetworkAbilityData } from '../../../../database/SqlLite.js';
import { SystemNetworkSummary } from '../../../../bean/AbilityMonitor.js';
import { Utils } from '../../base/Utils.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-network-ability')
export class TabPaneNetworkAbility extends BaseElement {
  private networkAbilityTbl: LitTable | null | undefined;
  private networkAbilitySource: Array<SystemNetworkSummary> = [];
  private float: HTMLDivElement | null | undefined;
  private queryResult: Array<SystemNetworkSummary> = [];
  private search: HTMLInputElement | undefined | null;

  set data(networkAbilityValue: SelectionParam | any) {
    // @ts-ignore
    this.networkAbilityTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 45 + 'px';
    this.queryDataByDB(networkAbilityValue);
  }

  initElements(): void {
    this.networkAbilityTbl = this.shadowRoot?.querySelector<LitTable>('#tb-network-ability');
    this.networkAbilityTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.networkAbilityTbl!);
  }

  filterData() {
    if (this.queryResult.length > 0) {
      let filterNetwork = this.queryResult.filter((item) => {
        let array = this.toNetWorkAbilityArray(item);
        let isInclude = array.filter((value) => value.indexOf(this.search!.value) > -1);
        return isInclude.length > 0;
      });
      if (filterNetwork.length > 0) {
        this.networkAbilitySource = filterNetwork;
        this.networkAbilityTbl!.recycleDataSource = this.networkAbilitySource;
      } else {
        this.networkAbilitySource = [];
        this.networkAbilityTbl!.recycleDataSource = [];
      }
    }
  }

  toNetWorkAbilityArray(systemNetworkSummary: SystemNetworkSummary): any[] {
    let array: Array<string> = [];
    array.push(systemNetworkSummary.startTimeStr);
    array.push(systemNetworkSummary.durationStr);
    array.push(systemNetworkSummary.dataReceivedStr);
    array.push(systemNetworkSummary.dataReceivedSecStr);
    array.push(systemNetworkSummary.dataSendSecStr);
    array.push(systemNetworkSummary.dataSendStr);
    array.push(systemNetworkSummary.packetsIn.toString());
    array.push(systemNetworkSummary.packetsOut.toString());
    array.push(systemNetworkSummary.packetsOutSec.toString());
    return array;
  }

  queryDataByDB(val: SelectionParam | any) {
    getTabNetworkAbilityData(val.leftNs, val.rightNs).then((item) => {
      log('getTabNetworkAbilityData result size : ' + item.length);
      if (item.length != null && item.length > 0) {
        for (const systemNetworkSummary of item) {
          if (systemNetworkSummary.startTime == 0) {
            systemNetworkSummary.startTimeStr = '0:000.000.000';
          } else {
            systemNetworkSummary.startTimeStr = Utils.getTimeStampHMS(systemNetworkSummary.startTime);
          }
          systemNetworkSummary.durationStr = Utils.getDurString(systemNetworkSummary.duration);
          systemNetworkSummary.dataReceivedStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataReceived);
          systemNetworkSummary.dataReceivedSecStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataReceivedSec);
          systemNetworkSummary.dataSendStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataSend);
          systemNetworkSummary.dataSendSecStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataSendSec);
          systemNetworkSummary.packetsInStr = ColorUtils.formatNumberComma(systemNetworkSummary.packetsIn);
          systemNetworkSummary.packetsInSecStr = systemNetworkSummary.packetsInSec.toFixed(2);
          systemNetworkSummary.packetsOutStr = ColorUtils.formatNumberComma(systemNetworkSummary.packetsOut);
          systemNetworkSummary.packetsOutSecStr = systemNetworkSummary.packetsOutSec.toFixed(2);
        }
        this.networkAbilitySource = item;
        this.queryResult = item;
        this.networkAbilityTbl!.recycleDataSource = this.networkAbilitySource;
      } else {
        this.networkAbilitySource = [];
        this.queryResult = [];
        this.networkAbilityTbl!.recycleDataSource = [];
      }
    });
  }

  initHtml(): string {
    return `
<style>
.network-ability-table{
    height: auto;
}
:host{
    padding: 10px 10px;
    display: flex;
    flex-direction: column;
}
</style>
<lit-table id="tb-network-ability" class="network-ability-table">
    <lit-table-column order width="1fr" title="StartTime" data-index="startTimeStr" key="startTimeStr" align="flex-start"></lit-table-column>
    <lit-table-column order width="1fr" title="Duration" data-index="durationStr" key="durationStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Received" data-index="dataReceivedStr" key="dataReceivedStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Received/sec" data-index="dataReceivedSecStr" key="dataReceivedSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Send" data-index="dataSendStr" key="dataSendStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Send/sec" data-index="dataSendSecStr" key="dataSendSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets In" data-index="packetsInStr" key="packetsInStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets In/sec" data-index="packetsInSecStr" key="packetsInSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets Out" data-index="packetsOutStr" key="packetsOutStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets Out/sec" data-index="packetsOutSecStr" key="packetsOutSecStr" align="flex-start" ></lit-table-column>
</lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (networkAbilityLeftData: SystemNetworkSummary, networkAbilityRightData: SystemNetworkSummary) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(networkAbilityRightData[property]) - parseFloat(networkAbilityLeftData[property]) : parseFloat(networkAbilityLeftData[property]) - parseFloat(networkAbilityRightData[property]);
        } else if (type === 'durationStr') {
          return sort === 2
            ? networkAbilityRightData.duration - networkAbilityLeftData.duration
            : networkAbilityLeftData.duration - networkAbilityRightData.duration;
        } else if (type === 'dataReceivedStr') {
          return sort === 2
            ? networkAbilityRightData.dataReceived - networkAbilityLeftData.dataReceived
            : networkAbilityLeftData.dataReceived - networkAbilityRightData.dataReceived;
        } else if (type === 'dataReceivedSecStr') {
          return sort === 2
            ? networkAbilityRightData.dataReceivedSec - networkAbilityLeftData.dataReceivedSec
            : networkAbilityLeftData.dataReceivedSec - networkAbilityRightData.dataReceivedSec;
        } else if (type === 'dataSendStr') {
          return sort === 2
            ? networkAbilityRightData.dataSend - networkAbilityLeftData.dataSend
            : networkAbilityLeftData.dataSend - networkAbilityRightData.dataSend;
        } else if (type === 'dataSendSecStr') {
          return sort === 2
            ? networkAbilityRightData.dataSendSec - networkAbilityLeftData.dataSendSec
            : networkAbilityLeftData.dataSendSec - networkAbilityRightData.dataSendSec;
        } else if (type === 'packetsInStr') {
          return sort === 2
            ? networkAbilityRightData.packetsIn - networkAbilityLeftData.packetsIn
            : networkAbilityLeftData.packetsIn - networkAbilityRightData.packetsIn;
        } else if (type === 'packetsInSecStr') {
          return sort === 2
            ? networkAbilityRightData.packetsInSec - networkAbilityLeftData.packetsInSec
            : networkAbilityLeftData.packetsInSec - networkAbilityRightData.packetsInSec;
        } else if (type === 'packetsOutStr') {
          return sort === 2
            ? networkAbilityRightData.packetsOut - networkAbilityLeftData.packetsOut
            : networkAbilityLeftData.packetsOut - networkAbilityRightData.packetsOut;
        } else if (type === 'packetsOutSecStr') {
          return sort === 2
            ? networkAbilityRightData.packetsOutSec - networkAbilityLeftData.packetsOutSec
            : networkAbilityLeftData.packetsOutSec - networkAbilityRightData.packetsOutSec;
        } else {
          // @ts-ignore
          if (networkAbilityRightData[property] > networkAbilityLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (networkAbilityRightData[property] == networkAbilityLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (detail.key === 'startTime') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'string'));
    } else if (detail.key === 'durationStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'durationStr'));
    } else if (detail.key === 'dataReceivedStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'dataReceivedStr'));
    } else if (detail.key === 'dataReceivedSecStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'dataReceivedSecStr'));
    } else if (detail.key === 'dataSendStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'dataSendStr'));
    } else if (detail.key === 'dataSendSecStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'dataSendSecStr'));
    } else if (detail.key === 'packetsInStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'packetsInStr'));
    } else if (detail.key === 'packetsInSecStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'packetsInSecStr'));
    } else if (detail.key === 'packetsOutStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'packetsOutStr'));
    } else if (detail.key === 'packetsOutSecStr') {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'packetsOutSecStr'));
    } else {
      this.networkAbilitySource.sort(compare(detail.key, detail.sort, 'number'));
    }
    this.networkAbilityTbl!.recycleDataSource = this.networkAbilitySource;
  }
}
