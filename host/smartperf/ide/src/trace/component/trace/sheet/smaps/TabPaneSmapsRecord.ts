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
import { SmapsType } from '../../../../bean/SmapsStruct.js';
import { querySmapsRecordTabData } from '../../../../database/SqlLite.js';
import { getByteWithUnit } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { SnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerSnapshot.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { resizeObserver } from '../SheetUtils.js';
@element('tabpane-smaps-record')
export class TabPaneSmapsRecord extends BaseElement {
  private smapsRecordTable: LitTable | undefined | null;
  private smapsRecordDataSource: Array<any> = [];
  private _GLESHostCache: Array<SnapshotStruct> = [];
  private pixelmapId = -1;
  private typeId = SmapsType.TYPE_NATIVE_HEAP;

  set GLESHostCache(value: Array<SnapshotStruct>) {
    this._GLESHostCache = value;
  }
  set data(smapsValue: SelectionParam | any) {
    this.smapsRecordDataSource = [];
    if (smapsValue) {
      if (this.pixelmapId == -1) {
        for (let [key, value] of SpSystemTrace.DATA_DICT) {
          if (value === 'pixelmap') {
            this.pixelmapId = key;
            break;
          }
        }
      }
      this.setSmapsRecordTableData(smapsValue.leftNs);
    }
  }

  private async setSmapsRecordTableData(startNs: number): Promise<void> {
    await querySmapsRecordTabData(startNs, MemoryConfig.getInstance().iPid,this.pixelmapId, this.typeId).then((results) => {
      if (results.length > 0) {
        let totalSize = 0;
        let RSGSize = 0;
        let virtaulSize = 0;
        let currentData = this._GLESHostCache.filter((item: SnapshotStruct) => item.startNs === startNs) || [];
        if (currentData.length === 1) {
          // GLESHostCache === currentData[0].aSize，改值Gpu Resource泳道图中已经获取过，所以在这个sql里只是设置为0，占位置，不用多查一遍
          RSGSize = currentData[0].aSize || 0;
        }
        for (let res of results) {
          if (res.name === 'VirtaulSize') {
            virtaulSize = res.size;
          } else {
            // RSGSize = RenderServiceCpu + SkiaCpu + GLESHostCache
            RSGSize += res.size;
          }
          switch (res.name) {
            case 'RenderServiceCpu':
              this.smapsRecordDataSource.push({ name: 'RenderServiceCpu', size: getByteWithUnit(res.size) });
              break;
            case 'SkiaCpu':
              this.smapsRecordDataSource.push({ name: 'SkiaCpu', size: getByteWithUnit(res.size) });
              break;
            case 'GLESHostCache':
              let size = currentData.length > 0 ? currentData[0].aSize : 0;
              this.smapsRecordDataSource.push({ name: 'GLESHostCache', size: getByteWithUnit(size) });
              break;
            default:
              break;
          }
        }
        //   ProcessCacheSize = virtaul_size - RenderServiceCpu - SkiaCpu - GLESHostCache
        const ProcessCacheSize = virtaulSize - RSGSize;
        // totalSize = RenderServiceCpu + SkiaCpu + GLESHostCache + ProcessCacheSize
        totalSize = RSGSize + ProcessCacheSize;
        this.smapsRecordDataSource.push({ name: 'ProcessCache', size: getByteWithUnit(ProcessCacheSize) });
        this.smapsRecordDataSource.unshift(
          { name: 'TimeStamp', size: ns2s(startNs) },
          { name: 'TimeStamp(Absolute)', size: (startNs + (window as any).recordStartNS) / 1000000000 },
          { name: 'Total', size: getByteWithUnit(totalSize) }
        );
      }
      this.smapsRecordTable!.recycleDataSource = this.smapsRecordDataSource;
    });
  }

  public initElements(): void {
    this.smapsRecordTable = this.shadowRoot?.querySelector<LitTable>('#smaps-record-tbl');
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.smapsRecordTable!);
  }
  public initHtml(): string {
    return `<style>
        :host{
            display: flex;
            padding: 10px 10px;
            flex-direction: column;
        }
        </style>
        <lit-table id="smaps-record-tbl" style="height: auto" no-head>
            <lit-table-column title="Name" data-index="name" align="flex-start" width="27%">
                <template><div>{{name}}</div></template>
            </lit-table-column>
            <lit-table-column title="size" data-index="size" align="flex-start" width="1fr">
                <template><div style="display: flex;">{{size}}</div></template>
            </lit-table-column>
        </lit-table>
        `;
  }
}
