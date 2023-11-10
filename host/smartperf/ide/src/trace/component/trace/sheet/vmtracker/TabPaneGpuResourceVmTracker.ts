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
import { queryGpuResourceTabData } from '../../../../database/SqlLite.js';
import { getByteWithUnit } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { resizeObserver } from '../SheetUtils.js';
@element('tabpane-gpu-resource')
export class TabPaneGpuResourceVmTracker extends BaseElement {
  private gpuResourceTable: LitTable | undefined | null;
  private gpuResourceDataSource: Array<any> = [];

  set data(startNs: number) {
    this.parentElement!.style.overflow = 'unset';
    this.gpuResourceDataSource = [];
    this.setGpuResourceTableData(startNs);
  }

  private async setGpuResourceTableData(startNs: number): Promise<void> {
    await queryGpuResourceTabData(startNs).then((results) => {
      if (results.length > 0) {
        results.sort(function (a, b) {
          return b.totalSize - a.totalSize;
        });
        let totalSize = 0;
        for (let i = 0; i < results.length; i++) {
          this.gpuResourceDataSource.push({
            name: SpSystemTrace.DATA_DICT.get(results[i].channelId),
            size: getByteWithUnit(results[i].totalSize || 0),
          });
          totalSize += results[i].totalSize;
        }
        this.gpuResourceDataSource.unshift(
          { name: 'TimeStamp', size: ns2s(startNs) },
          { name: 'TimeStamp(Absolute)', size: (startNs + (window as any).recordStartNS) / 1000000000 },
          { name: 'Total', size: getByteWithUnit(totalSize) }
        );
      }
      this.gpuResourceTable!.recycleDataSource = this.gpuResourceDataSource;
    });
  }

  public initElements(): void {
    this.gpuResourceTable = this.shadowRoot?.querySelector<LitTable>('#gpu-resource-tbl');
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.gpuResourceTable!);
  }
  public initHtml(): string {
    return `<style>
        :host{
            display: flex;
            padding: 10px 10px;
            flex-direction: column;
        }
        </style>
        <lit-table id="gpu-resource-tbl" no-head>
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
