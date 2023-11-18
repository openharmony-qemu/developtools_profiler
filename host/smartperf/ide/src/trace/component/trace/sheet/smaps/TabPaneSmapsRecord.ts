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
import { getTabSmapsData, getTabSmapsRecordData } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { log } from '../../../../../log/Log.js';
import { Smaps, SmapsType, TYPE_STRING } from '../../../../bean/SmapsStruct.js';
import { resizeObserver } from '../SheetUtils.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
@element('tabpane-smaps-record')
export class TabPaneSmapsRecord extends BaseElement {
  private tblSmapsRecord: LitTable | null | undefined;
  private sourceSmapsRecord: Array<Smaps> = [];
  private querySmapsRecordResult: Array<Smaps> = [];
  private isClick = false;
  private tabTitle: HTMLDivElement | undefined | null;
  set data(valSmapsRecord: SelectionParam) {
    this.parentElement!.style.overflow = 'unset';
    this.isClick = valSmapsRecord.smapsType.length === 0;
    this.init();
    this.tblSmapsRecord!.loading = true;
    if (!this.isClick) {
      if (valSmapsRecord.smapsType.length > 0) {
        this.queryDataByDB(valSmapsRecord);
      }
    } else {
      this.setSmaps(valSmapsRecord);
    }
  }
  initElements(): void {
    this.tblSmapsRecord = this.shadowRoot?.querySelector<LitTable>('#tb-smaps-record');
    this.tabTitle = this.tblSmapsRecord!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.tblSmapsRecord!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }
  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.tblSmapsRecord?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight  - 15+ 'px';
        this.tblSmapsRecord?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }
  queryDataByDB(srVal: SelectionParam | any): void {
    getTabSmapsData(srVal.leftNs, srVal.rightNs, (MemoryConfig.getInstance().interval * 1000_000) / 5).then(
      (result) => {
        log('getTabSmapsData size :' + result.length);
        this.tblSmapsRecord!.loading = false;
        this.filteredData(result);
      }
    );
  }
  setSmaps(data: SelectionParam): void {
    getTabSmapsRecordData(data.rightNs).then((result) => {
      this.tblSmapsRecord!.loading = false;
      this.filteredData(result);
    });
  }
  private init(): void {
    const thTable = this.tabTitle!.querySelector('.th');
    const smapsRecordTblNodes = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      smapsRecordTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }
  filteredData(result: any): void {
    if (result.length !== null && result.length > 0) {
      for (const smaps of result) {
        smaps.typeName = TYPE_STRING[smaps.type];
        smaps.address = smaps.startAddr + ' - ' + smaps.endAddr;
        smaps.swapStr = Utils.getBinaryByteWithUnit(smaps.swap);
        smaps.rssStr = Utils.getBinaryByteWithUnit(smaps.rss);
        smaps.pssStr = Utils.getBinaryByteWithUnit(smaps.pss);
        smaps.sizeStr = Utils.getBinaryByteWithUnit(smaps.size);
        smaps.sharedCleanStr = Utils.getBinaryByteWithUnit(smaps.sharedClean);
        smaps.sharedDirtyStr = Utils.getBinaryByteWithUnit(smaps.sharedDirty);
        smaps.privateCleanStr = Utils.getBinaryByteWithUnit(smaps.privateClean);
        smaps.privateDirtyStr = Utils.getBinaryByteWithUnit(smaps.privateDirty);
        smaps.swapPssStr = Utils.getBinaryByteWithUnit(smaps.swapPss);
        smaps.time = Utils.getTimeString(smaps.startNs);
        smaps.path = SpSystemTrace.DATA_DICT.get(smaps.path)?.split('/');
        smaps.permission = SpSystemTrace.DATA_DICT.get(smaps.pid)?.split('/');
        let resideS = smaps.reside.toFixed(2);
        if (resideS === '0.00') {
          smaps.resideStr = '0 %';
        } else {
          smaps.resideStr = resideS + '%';
        }
      }
      this.sourceSmapsRecord = result;
      this.querySmapsRecordResult = result;
      this.tblSmapsRecord!.recycleDataSource = this.sourceSmapsRecord;
    } else {
      this.sourceSmapsRecord = [];
      this.querySmapsRecordResult = [];
      this.tblSmapsRecord!.recycleDataSource = [];
    }
  }
  initHtml(): string {
    return `
        <style>
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        .smaps-record-table{
            height: auto;
        }
        </style>
        <lit-table id="tb-smaps-record" class="smaps-record-table" style="overflow: auto">
            <lit-table-column order width="100px" title="TimeStamp" data-index="time" key="time" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Type" data-index="typeName" key="typeName" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Path" data-index="path" key="path" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="250px" title="Address Range" data-index="address" key="address" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Rss" data-index="rssStr" key="rssStr" align="flex-start" >
            </lit-table-column>
              <lit-table-column order width="150px" title="Pss" data-index="pssStr" key="pssStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column  width="150px" title="SharedClean" data-index="sharedCleanStr" key="sharedCleanStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="SharedDirty" data-index="sharedDirtyStr" key="sharedDirtyStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="PrivateClean" data-index="privateCleanStr" key="privateCleanStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="PrivateDirty" data-index="privateDirtyStr" key="privateDirtyStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="Swap" data-index="swapStr" key="swapStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="SwapPss" data-index="swapPssStr" key="swapPssStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column order width="150px" title="Reside" data-index="resideStr" key="resideStr" align="flex-start" >
            </lit-table-column>
             <lit-table-column order width="150px" title="Protection" data-index="permission" key="permission" align="flex-start" >
            </lit-table-column>
        </lit-table>
        `;
  }
  sortByColumn(detail: any): void {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (aSmapsRecord: Smaps, bSmapsRecord: Smaps) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(bSmapsRecord[property]) - parseFloat(aSmapsRecord[property]) : parseFloat(aSmapsRecord[property]) - parseFloat(bSmapsRecord[property]);
        } else {
          // @ts-ignore
          if (bSmapsRecord[property] > aSmapsRecord[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (bSmapsRecord[property] === aSmapsRecord[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }
    if (
      detail.key === 'rssStr' ||
      detail.key === 'sizeStr' ||
      detail.key === 'resideStr'
    ) {
      let key = detail.key.substring(0, detail.key.indexOf('Str'));
      this.sourceSmapsRecord.sort(compare(key, detail.sort, 'number'));
    } else {
      this.sourceSmapsRecord.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.tblSmapsRecord!.recycleDataSource = this.sourceSmapsRecord;
  }
}
