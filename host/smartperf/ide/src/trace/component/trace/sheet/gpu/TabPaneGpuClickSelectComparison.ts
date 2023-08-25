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
import { LitSelect } from '../../../../../base-ui/select/LitSelect.js';
import { LitSelectOption } from '../../../../../base-ui/select/LitSelectOption.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { queryGpuDataByTs } from '../../../../database/SqlLite.js';
import { SnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerSnapshot.js';
import { VmTrackerChart } from '../../../chart/SpVmTrackerChart.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';
import { compare, CompareStruct, resizeObserverFromMemory } from '../SheetUtils.js';
import '../TabPaneJsMemoryFilter.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';
import { TabPaneGpuClickSelect } from './TabPaneGpuClickSelect.js';
interface GpuTreeItem {
  name: string;
  id: number;
  size: number;
  sizeStr: string;
  children?: GpuTreeItem[] | undefined;
}
@element('tabpane-gpu-click-select-comparison')
export class TabPaneGpuClickSelectComparison extends TabPaneGpuClickSelect {
  private comparisonSelect: TabPaneJsMemoryFilter | null | undefined;
  private selectEl: LitSelect | null | undefined;
  private gpuComparisonTbl: LitTable | null | undefined;
  private baseGpuData: Array<GpuDumpComparison> = [];
  private gpuComparisonSource: Array<GpuTreeItem> = [];
  private comparisonSortData: Array<GpuTreeItem> = [];

  initElements(): void {
    this.gpuComparisonTbl = this.shadowRoot?.querySelector<LitTable>('#tb-gpu');
    this.comparisonSelect = this.shadowRoot?.querySelector('#filter') as TabPaneJsMemoryFilter;
    this.selectEl = this.comparisonSelect?.shadowRoot?.querySelector<LitSelect>('lit-select');
    this.gpuComparisonTbl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortGpuByColumn(e.detail.key, e.detail.sort);
    });
  }
  connectedCallback(): void {
    super.connectedCallback();
    resizeObserverFromMemory(this.parentElement!, this.gpuComparisonTbl!, this.comparisonSelect!);
  }
  async queryDataByDB(type: string, startTs: number): Promise<any> {
    let gpuData: Array<any> = [];
    let window = type === 'total' ? 0 : VmTrackerChart.gpuWindow;
    let module = type === 'total' ? VmTrackerChart.gpuTotalModule : VmTrackerChart.gpuWindowModule;
    await queryGpuDataByTs(startTs, window || 0, module).then((result) => {
      gpuData = result;
    });
    return gpuData;
  }
  async getGpuClickDataByDB(type: string, startTs: number, dataList: Array<SnapshotStruct>) {
    const dataArray = [];
    let label = this.gpuComparisonTbl!.shadowRoot!.querySelector('.thead')?.firstChild?.firstChild?.firstChild;
    if (label) {
      (label as HTMLLabelElement).innerHTML = type === 'total' ? 'Module / Category' : 'Window / Module / Category';
    }
    //@ts-ignore
    this.gpuComparisonTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    this.gpuComparisonTbl!.loading = true;
    this.baseGpuData = [];
    let baseGpuData = await this.queryDataByDB(type, startTs);
    for (const item of baseGpuData) {
      this.baseGpuData.push(
        new GpuDumpComparison(item.windowNameId, item.windowId, item.moduleId, item.categoryId, item.size)
      );
    }
    for (let item of dataList) {
      if (item.startNs !== startTs) {
        dataArray.push(item);
      }
    }
    this.selectStamps(dataArray, type);
    this.getComparisonData(dataArray[0].startNs, type);
  }
  selectStamps(gpuSelectComList: Array<SnapshotStruct>, type: string): void {
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (gpuSelectComList[0].name) {
      option.setAttribute('value', gpuSelectComList[0].name);
    }
    option.setAttribute('value', gpuSelectComList[0].name);
    this.selectEl!.defaultValue = gpuSelectComList[0].name || '';
    this.selectEl!.placeholder = gpuSelectComList[0].name || '';
    this.selectEl!.dataSource = gpuSelectComList;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((option) => {
      option.addEventListener('onSelected', async (e) => {
        for (let f of gpuSelectComList) {
          if (input.value === f.name) {
            this.getComparisonData(f.startNs, type);
          }
        }
        e.stopPropagation();
      });
    });
  }
  async getComparisonData(targetStartNs: number, type: string) {
    let comparisonData: Array<GpuDumpComparison> = [];
    let targetGpuData: Array<GpuDumpComparison> = [];
    let data = await this.queryDataByDB(type, targetStartNs);
    for (const item of data) {
      targetGpuData.push(
        new GpuDumpComparison(item.windowNameId, item.windowId, item.moduleId, item.categoryId, item.size)
      );
    }
    comparisonData = compare(this.baseGpuData!, targetGpuData);
    for (const item of comparisonData) {
      item.size = item.value;
    }
    let items = this.createTreeData(comparisonData);
    this.gpuComparisonSource = (type === 'total' ? items[0].children : items) || [];
    this.comparisonSortData = this.gpuComparisonSource;
    this.gpuComparisonTbl!.recycleDataSource = this.gpuComparisonSource;
    this.gpuComparisonTbl!.loading = false;
  }

  sortGpuByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.gpuComparisonTbl!.recycleDataSource = this.comparisonSortData;
        break;
      default:
        let array = [...this.comparisonSortData];
        switch (column) {
          case 'name':
            this.gpuComparisonTbl!.recycleDataSource = array.sort((gpuComparisonLeftData, gpuComparisonRightData) => {
              return sort === 1
                ? `${gpuComparisonLeftData.name}`.localeCompare(`${gpuComparisonRightData.name}`)
                : `${gpuComparisonRightData.name}`.localeCompare(`${gpuComparisonLeftData.name}`);
            });
            break;
          case 'sizeDelta':
            this.gpuComparisonTbl!.recycleDataSource = array.sort((gpuComparisonLeftData, gpuComparisonRightData) => {
              return sort === 1
                ? gpuComparisonLeftData.size - gpuComparisonRightData.size
                : gpuComparisonRightData.size - gpuComparisonLeftData.size;
            });
            break;
        }
        break;
    }
  }

  initHtml(): string {
    return `
    <style>
    :host{
        display: flex;
        flex-direction: column;
        padding: 10px 10px;
    }
    </style>
    <lit-table id="tb-gpu" style="height: auto" tree>
            <lit-table-column width="50%" title="Window / Module / Category" data-index="name" key="name" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="SizeDelta" data-index="sizeStr" key="sizeDelta"  align="flex-start" order >
            </lit-table-column>
    </lit-table>
    <tab-pane-js-memory-filter id="filter" first hideFilter ></tab-pane-js-memory-filter>
        `;
  }
}
export class GpuDumpComparison extends CompareStruct {
  windowNameId: number = -1;
  windowId: number = -1;
  moduleId: number = -1;
  categoryId: number = -1;
  size: number = -1;
  constructor(windowNameId: number, windowId: number, moduleId: number, categoryId: number, value: number) {
    super(`${windowNameId}` + '' + `${windowId}` + '' + `${moduleId}` + '' + `${categoryId}`, value);
    this.windowNameId = windowNameId;
    this.moduleId = moduleId;
    this.windowId = windowId;
    this.categoryId = categoryId;
  }
  clone(isBase?: boolean): GpuDumpComparison {
    const value = isBase ? this.value : -this.value;
    return new GpuDumpComparison(this.windowNameId, this.windowId, this.moduleId, this.categoryId, value);
  }
}
