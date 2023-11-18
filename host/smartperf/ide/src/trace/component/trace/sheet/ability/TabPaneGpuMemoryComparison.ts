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
import { GpuMemoryComparison } from '../../../../bean/AbilityMonitor.js';
import { getTabGpuMemoryComparisonData } from '../../../../database/SqlLite.js';
import { SnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerSnapshot.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';
import { compare, resizeObserverFromMemory } from '../SheetUtils.js';
import '../TabPaneJsMemoryFilter.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';

@element('tabpane-gpu-memory-comparison')
export class TabPaneGpuMemoryComparison extends BaseElement {
  private gpuMemoryClickTable: LitTable | null | undefined;
  private comparisonSelect: TabPaneJsMemoryFilter | null | undefined;
  private selectEl: LitSelect | null | undefined;
  private selfData: Array<GpuMemoryComparison> = [];
  private comparisonSource: Array<GpuMemoryComparison> = [];

  initElements(): void {
    this.gpuMemoryClickTable = this.shadowRoot?.querySelector<LitTable>('#gpuMemoryClickTable');
    this.comparisonSelect = this.shadowRoot?.querySelector('#filter') as TabPaneJsMemoryFilter;
    this.selectEl = this.comparisonSelect?.shadowRoot?.querySelector<LitSelect>('lit-select');
    this.gpuMemoryClickTable!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortGpuMemoryByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserverFromMemory(this.parentElement!, this.gpuMemoryClickTable!, this.comparisonSelect!);
  }

  async queryDataByDB(startNs: number): Promise<GpuMemoryComparison[]> {
    let timeStampData: Array<GpuMemoryComparison> = [];
    await getTabGpuMemoryComparisonData(startNs).then((data) => {
      data.forEach((item) => {
        if (item.processName !== null) {
          item.process = `${item.processName}(${item.processId})`;
        } else {
          item.process = `Process(${item.processId})`;
        }
        item.gpuName = SpSystemTrace.DATA_DICT.get(item.gpuNameId as number) || '-';
      });
      timeStampData = data;
    });
    return timeStampData;
  }

  async comparisonDataByDB(startNs: number, dataList: Array<SnapshotStruct>): Promise<void> {
    this.selfData = [];
    let selfData = await this.queryDataByDB(startNs);
    const dataArray = [];
    for (const item of selfData) {
      this.selfData.push(new GpuMemoryComparison(item.process, '', item.gpuName, item.value));
    }
    for (let item of dataList) {
      if (item.startNs !== startNs) {
        dataArray.push(item);
      }
    }
    this.selectStamps(dataArray);
    this.getComparisonData(dataArray[0].startNs);
  }

  selectStamps(gpuMemoryComList: Array<SnapshotStruct>): void {
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (gpuMemoryComList[0].name) {
      option.setAttribute('value', gpuMemoryComList[0].name);
    }
    option.setAttribute('value', gpuMemoryComList[0].name);
    this.selectEl!.defaultValue = gpuMemoryComList[0].name || '';
    this.selectEl!.placeholder = gpuMemoryComList[0].name || '';
    this.selectEl!.dataSource = gpuMemoryComList;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((option) => {
      option.addEventListener('onSelected', async (e) => {
        for (let f of gpuMemoryComList) {
          if (input.value === f.name) {
            this.getComparisonData(f.startNs);
          }
        }
        e.stopPropagation();
      });
    });
  }

  async getComparisonData(targetStartNs: number) {
    let comparisonData: GpuMemoryComparison[] = [];
    let comparison: GpuMemoryComparison[] = [];
    let data = await this.queryDataByDB(targetStartNs);
    for (const item of data) {
      comparison.push(new GpuMemoryComparison(item.process, '', item.gpuName, item.value));
    }
    comparisonData = compare(this.selfData!, comparison);
    for (const item of comparisonData) {
      item.sizes = Utils.getBinaryByteWithUnit(item.value);
    }
    this.comparisonSource = comparisonData;
    this.gpuMemoryClickTable!.recycleDataSource = comparisonData;
  }

  sortGpuMemoryByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.gpuMemoryClickTable!.recycleDataSource = this.comparisonSource;
        break;
      default:
        let array = [...this.comparisonSource];
        switch (column) {
          case 'process':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort(
              (gpuMComparisonLeftData, gpuMComparisonRightData) => {
                return sort === 1
                  ? `${gpuMComparisonLeftData.process}`.localeCompare(`${gpuMComparisonRightData.process}`)
                  : `${gpuMComparisonRightData.process}`.localeCompare(`${gpuMComparisonLeftData.process}`);
              }
            );
            break;
          case 'gpuName':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort(
              (gpuMComparisonLeftData, gpuMComparisonRightData) => {
                return sort === 1
                  ? `${gpuMComparisonLeftData.gpuName}`.localeCompare(`${gpuMComparisonRightData.gpuName}`)
                  : `${gpuMComparisonRightData.gpuName}`.localeCompare(`${gpuMComparisonLeftData.gpuName}`);
              }
            );
            break;
          case 'sizeDelta':
            this.gpuMemoryClickTable!.recycleDataSource = array.sort(
              (gpuMComparisonLeftData, gpuMComparisonRightData) => {
                return sort === 1
                  ? gpuMComparisonLeftData.value - gpuMComparisonRightData.value
                  : gpuMComparisonRightData.value - gpuMComparisonLeftData.value;
              }
            );
            break;
        }
        break;
    }
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
    <lit-table-column order title="Process" data-index="process" key="process" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="GpuName" data-index="gpuName" key="gpuName" align="flex-start" width="1fr" >
    </lit-table-column>
    <lit-table-column order title="SizeDelta" data-index="sizes" key="sizeDelta" align="flex-start" width="1fr" >
    </lit-table-column>
</lit-table>
<tab-pane-js-memory-filter id="filter" first hideFilter ></tab-pane-js-memory-filter>
        `;
  }
}
