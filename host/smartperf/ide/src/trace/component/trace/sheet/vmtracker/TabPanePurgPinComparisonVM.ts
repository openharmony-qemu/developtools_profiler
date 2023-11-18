/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this data except in compliance with the License.
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
import { LitSelect } from '../../../../../base-ui/select/LitSelect.js';
import { LitSelectOption } from '../../../../../base-ui/select/LitSelectOption.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { queryProcessPurgeableSelectionTab } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { CompareStruct, compare, resizeObserverFromMemory } from '../SheetUtils.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';
@element('tabpane-purgeable-pin-comparison-vm')
export class TabPanePurgPinComparisonVM extends BaseElement {
  private purgeablePinTable: LitTable | null | undefined;
  private purgeablePinSource: Array<unknown> = [];
  private filterEl: TabPaneJsMemoryFilter | undefined | null;
  private selectEl: LitSelect | undefined | null;

  public initElements(): void {
    this.purgeablePinTable = this.shadowRoot?.querySelector<LitTable>('#tb-purgeable-pin');
    this.filterEl = this.shadowRoot!.querySelector<TabPaneJsMemoryFilter>('#filter');
    this.selectEl = this.filterEl?.shadowRoot?.querySelector<LitSelect>('lit-select');
  }
  public totalData(data: SelectionParam | any, dataList: any): void {
    //@ts-ignore
    this.purgeablePinTable?.shadowRoot?.querySelector('.table')?.style?.height = `${
      this.parentElement!.clientHeight - 45
    }px`;
    this.purgeablePinSource = [];
    let fileArr: any[] = [];
    for (let fileData of dataList) {
      if (fileData.startNs !== data.startNs) {
        fileArr.push(fileData);
      }
    }
    fileArr = fileArr.sort();
    this.initSelect(data.startNs, fileArr);
    this.updateComparisonData(data.startNs, fileArr[0].startNs);
  }
  private initSelect(fileStartNs: number, purgePinComVmList: Array<any>): void {
    let that = this;
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (purgePinComVmList[0].name) {
      option.setAttribute('value', purgePinComVmList[0].name);
    }
    this.selectEl!.defaultValue = purgePinComVmList[0].name;
    this.selectEl!.placeholder = purgePinComVmList[0].name;
    this.selectEl!.dataSource = purgePinComVmList;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((a) => {
      a.addEventListener('onSelected', (e: any) => {
        for (let f of purgePinComVmList) {
          if (input.value === f.name) {
            that.updateComparisonData(fileStartNs, f.startNs);
          }
        }
        e.stopPropagation();
      });
    });
  }
  private async updateComparisonData(baseTime: number, targetTime: number): Promise<void> {
    this.purgeablePinSource = [];
    let tableData = await this.queryPinVMData(baseTime, targetTime);
    this.purgeablePinSource.push(tableData);
    if (this.purgeablePinSource.length > 0) {
      this.purgeablePinTable!.recycleDataSource = this.purgeablePinSource;
    } else {
      this.purgeablePinTable!.recycleDataSource = [];
    }
  }
  private async queryPinVMData(baseTime: number, targetTime: number): Promise<any> {
    let delta = {
      purgPinDelta: '0Bytes',
      shmPurgPinDelta: '0Bytes',
    };
    const baseArr: CompareStruct[] = [];
    const targetArr: CompareStruct[] = [];
    // 点击的
    await queryProcessPurgeableSelectionTab(baseTime, MemoryConfig.getInstance().iPid).then(async (results) => {
      for (let i = 0; i < results.length; i++) {
        baseArr.push(new CompareStruct(results[i].name, results[i].value));
      }
      // 被比较的
      await queryProcessPurgeableSelectionTab(targetTime, MemoryConfig.getInstance().iPid).then((results) => {
        for (let i = 0; i < results.length; i++) {
          targetArr.push(new CompareStruct(results[i].name, results[i].value));
        }
        let compareData = compare(targetArr, baseArr);
        for (let data of compareData) {
          if (data.key === 'PinedPurg') {
            delta.purgPinDelta = Utils.getBinaryByteWithUnit(data.value);
          } else if (data.key === 'ShmPurg') {
            delta.shmPurgPinDelta = Utils.getBinaryByteWithUnit(data.value);
          }
        }
      });
    });
    return delta;
  }

  public connectedCallback(): void {
    super.connectedCallback();
    resizeObserverFromMemory(this.parentElement!, this.purgeablePinTable!, this.filterEl!);
  }
  public initHtml(): string {
    return `
    <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
    </style>
    <lit-table id="tb-purgeable-pin" style="height: auto">
        <lit-table-column width="1fr" title="PurgPinDelta" data-index="purgPinDelta" align="flex-start">
        </lit-table-column>
        <lit-table-column width="1fr" title="ShmPurgPinDelta" data-index="shmPurgPinDelta" align="flex-start">
        </lit-table-column>
    </lit-table>
    <lit-progress-bar class="progress"></lit-progress-bar>
    <tab-pane-js-memory-filter id="filter" first hideFilter></tab-pane-js-memory-filter>
    `;
  }
}
