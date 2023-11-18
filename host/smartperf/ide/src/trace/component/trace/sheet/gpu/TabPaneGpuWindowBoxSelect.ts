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
import { queryGpuDataByRange } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { resizeObserver } from '../SheetUtils.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { Utils } from '../../base/Utils.js';

interface Gpu {
  startTs: number;
  startTsStr?: string;
  windowId: number;
  moduleId: number;
  categoryId: number;
  gpuName?: string;
  avgSize: number;
  avgSizeStr?: string;
  maxSize: number;
  maxSizeStr?: string;
  minSize: number;
  minSizeStr?: string;
}

@element('tabpane-gpu-window-box-select')
export class TabPaneGpuWindowBoxSelect extends BaseElement {
  private gpuBoxTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private gpuBoxSource: Array<Gpu> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(gpuBoxParam: SelectionParam | any) {
    if (this.currentSelectionParam === gpuBoxParam) {
      return;
    }
    this.currentSelectionParam = gpuBoxParam;
    //@ts-ignore
    this.gpuBoxTbl?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.range!.textContent =
      'Selected range: ' + ((gpuBoxParam.rightNs - gpuBoxParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.gpuBoxTbl!.loading = true;
    queryGpuDataByRange(gpuBoxParam.leftNs, gpuBoxParam.rightNs, MemoryConfig.getInstance().snapshotDur).then((result) => {
      this.gpuBoxTbl!.loading = false;
      if (result != null && result.length > 0) {
        log('getTabStartups result size : ' + result.length);
        let target = result.filter((it) => it.windowId !== 0);
        target.forEach((it: Gpu) => {
          let windowName = SpSystemTrace.DATA_DICT.get(it.windowId) || 'NULL';
          let moduleName = SpSystemTrace.DATA_DICT.get(it.moduleId) || 'NULL';
          let categoryName = SpSystemTrace.DATA_DICT.get(it.categoryId) || 'NULL';
          it.gpuName = `${windowName} / ${moduleName} / ${categoryName}`;
          it.startTsStr = getProbablyTime(it.startTs);
          it.avgSizeStr = Utils.getBinaryByteWithUnit(it.avgSize);
          it.minSizeStr = Utils.getBinaryByteWithUnit(it.minSize);
          it.maxSizeStr = Utils.getBinaryByteWithUnit(it.maxSize);
        });
        this.gpuBoxSource = target;
        this.gpuBoxTbl!.recycleDataSource = this.gpuBoxSource;
      } else {
        this.gpuBoxSource = [];
        this.gpuBoxTbl!.recycleDataSource = [];
      }
    });
  }

  initElements(): void {
    this.gpuBoxTbl = this.shadowRoot?.querySelector<LitTable>('#tb-gpu-box');
    this.range = this.shadowRoot?.querySelector('#gpu-box-time-range');
    this.gpuBoxTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.gpuBoxTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .gpuBox-table{
          flex-direction: row;
          margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="gpu-box-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <div style="flex: 1"></div>
            <label id="gpu-box-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
            <lit-table id="tb-gpu-box" style="height: auto" tree>
                </lit-table-column>
                <lit-table-column width="500px" title="Window / Module / Category" data-index="gpuName" key="gpuName"  align="flex-start">
                </lit-table-column>
                <lit-table-column width="160px" title="AvgSize" data-index="avgSizeStr" key="avgSizeStr"  align="flex-start" order >
                </lit-table-column>
                <lit-table-column width="160px" title="MaxSize" data-index="maxSizeStr" key="maxSizeStr"  align="flex-start" order >
                </lit-table-column>
                <lit-table-column width="160px" title="MinSize" data-index="minSizeStr" key="minSizeStr"  align="flex-start" order >
                </lit-table-column>
            </lit-table>
        </div>
        `;
  }

  sortByColumn(detail: { key: string; sort: number }): void {
    this.gpuBoxSource.sort((gpuA, gpuB) => {
      if (detail.sort === 0) {
        return gpuA.startTs - gpuB.startTs;
      } else {
        let key = detail.key.replace('Str', '');
        let valueA = (gpuA as any)[key];
        let valueB = (gpuB as any)[key];
        return detail.sort === 1 ? valueA - valueB : valueB - valueA;
      }
    });
    this.gpuBoxTbl!.recycleDataSource = this.gpuBoxSource;
  }
}
