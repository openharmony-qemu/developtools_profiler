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
import { queryVmTrackerShmSelectionData } from '../../../../database/SqlLite.js';
import { SnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerSnapshot.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { Utils } from '../../base/Utils.js';
import { LitSelectOption } from '../../../../../base-ui/select/LitSelectOption.js';
import { LitSelect } from '../../../../../base-ui/select/LitSelect.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';
import { resizeObserverFromMemory } from '../SheetUtils.js';

@element('tabpane-vmtracker-shm-comparison')
export class TabPaneVmTrackerShmComparison extends BaseElement {
  private comparisonTableEl: LitTable | undefined | null;
  private baseFileTs: number | undefined | null;
  private targetFileTs: number | undefined | null;
  private comparisonData!: any[];
  private baseFileData: Array<any> = [];
  private targetFileData: Array<any> = [];
  private memoryConfig: MemoryConfig = MemoryConfig.getInstance();
  private selectEl: LitSelect | undefined | null;
  private filterEl: TabPaneJsMemoryFilter | undefined | null;
  private comparisonSource: Array<ShmObj> = [];

  initElements(): void {
    this.comparisonTableEl = this.shadowRoot!.querySelector<LitTable>('#tb-comparison') as LitTable;
    this.filterEl = this.shadowRoot!.querySelector<TabPaneJsMemoryFilter>('#filter');
    this.selectEl = this.filterEl?.shadowRoot?.querySelector<LitSelect>('lit-select');
    this.comparisonTableEl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortShmByColumn(e.detail.key, e.detail.sort);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserverFromMemory(this.parentElement!, this.comparisonTableEl!, this.filterEl!);
  }

  setShmData(data: SnapshotStruct, dataList: Array<SnapshotStruct>): void {
    let fileArr: SnapshotStruct[] = [];
    let that = this;
    for (let file of dataList) {
      if (file.startNs !== data.startNs) {
        fileArr.push(file);
      }
    }
    fileArr = fileArr.sort();
    this.baseFileTs = data.startNs;
    this.initSelect(data.startNs, fileArr);
    this.targetFileTs = fileArr[0].startNs;
    that.updateComparisonData(data.startNs, fileArr[0].startNs);
  }

  async updateComparisonData(baseFileTs: number, targetFileTs: number) {
    await queryVmTrackerShmSelectionData(baseFileTs, this.memoryConfig.iPid).then((result) => {
      this.baseFileData = result;
    });
    await queryVmTrackerShmSelectionData(targetFileTs, this.memoryConfig.iPid).then((result) => {
      this.targetFileData = result;
    });
    let sizeData = this.calSizeObj(this.baseFileData, this.targetFileData);
    this.comparisonData = [];
    this.comparisonData.push(sizeData);
    this.comparisonSource = this.comparisonData;
    this.comparisonTableEl!.snapshotDataSource = this.comparisonData;
  }

  calSizeObj(baseFileData: Array<any>, targetFileData: Array<any>): ShmObj {
    let sizeObj = new ShmObj();
    let baseSumSize = 0;
    let targetSumSize = 0;
    for (let file of baseFileData) {
      baseSumSize += file.size;
    }
    for (let file of targetFileData) {
      targetSumSize += file.size;
    }
    sizeObj.sizeDelta = baseSumSize - targetSumSize;
    sizeObj.sizeDeltaStr = Utils.getBinaryByteWithUnit(sizeObj.sizeDelta);
    return sizeObj;
  }

  initSelect(fileId: number, fileArr: Array<SnapshotStruct>) {
    let that = this;
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (fileArr[0].name) {
      option.setAttribute('value', fileArr[0].name);
    }
    this.selectEl!.defaultValue = fileArr[0].name || '';
    this.selectEl!.placeholder = fileArr[0].name || '';
    this.selectEl!.dataSource = fileArr;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((a) => {
      a.addEventListener('onSelected', (e) => {
        this.comparisonTableEl!.scrollTop = 0;
        for (let f of fileArr) {
          if (input.value == f.name) {
            that.updateComparisonData(fileId, f.startNs);
          }
        }
        e.stopPropagation();
      });
    });
  }

  sortShmByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.comparisonTableEl!.snapshotDataSource = this.comparisonSource;
        break;
      default:
        let array = [...this.comparisonSource];
        switch (column) {
          case 'sizeDelta':
            this.comparisonTableEl!.snapshotDataSource = array.sort((shmComparisonLeftData, shmComparisonRightData) => {
              return sort === 1
                ? shmComparisonLeftData.sizeDelta - shmComparisonRightData.sizeDelta
                : shmComparisonRightData.sizeDelta - shmComparisonLeftData.sizeDelta;
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
            padding: 10px 10px 0 10px;
            height: calc(100% - 10px - 31px);
        }
        tab-pane-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        selector{
            display: none;
        }
        .show{
            display: flex;
            flex: 1;
        }
        .progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <div style="display: flex;flex-direction: row;height: 100%;">
            <selector id='show_table' class="show">
                <lit-slicer style="width:100%">
                    <div style="width: 100%">
                        <lit-table id="tb-comparison" style="height: auto">
                            <lit-table-column width="1fr" title="SizeDelta" data-index="sizeDeltaStr" key="sizeDelta" align="flex-start"  order>
                            </lit-table-column>
                        </lit-table>
                    </div>
                </lit-slicer>
            </selector>
            <lit-progress-bar class="progress"></lit-progress-bar>
            <tab-pane-js-memory-filter id="filter" first hideFilter ></tab-pane-js-memory-filter>
            <div class="loading"></div>
        </div>
        `;
  }
}

class ShmObj {
  sizeDelta = 0;
  sizeDeltaStr: string = '';
}
