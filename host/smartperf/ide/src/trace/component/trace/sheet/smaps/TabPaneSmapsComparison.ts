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
import { element } from '../../../../../base-ui/BaseElement.js';
import { LitSelect } from '../../../../../base-ui/select/LitSelect.js';
import { LitSelectOption } from '../../../../../base-ui/select/LitSelectOption.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabSmapsStatisticData } from '../../../../database/SqlLite.js';
import { resizeObserverFromMemory } from '../SheetUtils.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';
import { TabPaneSmapsStatistics } from './TabPaneSmapsStatistics.js';
import { SmapsType } from '../../../../bean/SmapsStruct.js';

@element('tabpane-smaps-comparison')
export class TabPaneSmapsComparison extends TabPaneSmapsStatistics {
  private smapsCompariosnTable: LitTable | null | undefined;
  private filterEl: TabPaneJsMemoryFilter | undefined | null;
  private selectEl: LitSelect | undefined | null;

  public initElements(): void {
    this.smapsCompariosnTable = this.shadowRoot?.querySelector<LitTable>('#tb-smaps-comparison');
    this.filterEl = this.shadowRoot!.querySelector<TabPaneJsMemoryFilter>('#filter');
    this.selectEl = this.filterEl?.shadowRoot?.querySelector<LitSelect>('lit-select');
    this.tabTitle = this.smapsCompariosnTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.smapsCompariosnTable!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort, this.smapsCompariosnTable);
    });
  }
  public setData(data: SelectionParam | any, dataList: any): void {
    //@ts-ignore
    this.smapsCompariosnTable?.shadowRoot?.querySelector('.table')?.style?.height = `${
      this.parentElement!.clientHeight - 45
    }px`;
    this.smapsCompariosnTable!.loading = true;
    this.init(this.tabTitle!);
    let fileArr: any[] = [];
    for (let file of dataList) {
      if (file.startNs !== data.leftNs) {
        fileArr.push(file);
      }
    }
    fileArr = fileArr.sort();
    this.initSelect(data.leftNs, fileArr);
    this.querySmapsData(data.leftNs, fileArr[0].startNs);
  }
  private initSelect(fileStartNs: number, smapsComFileArr: Array<any>): void {
    let that = this;
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (smapsComFileArr[0].name) {
      option.setAttribute('value', smapsComFileArr[0].name);
    }
    this.selectEl!.defaultValue = smapsComFileArr[0].name;
    this.selectEl!.placeholder = smapsComFileArr[0].name;
    this.selectEl!.dataSource = smapsComFileArr;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((a) => {
      a.addEventListener('onSelected', (e: any) => {
        for (let f of smapsComFileArr) {
          if (input.value === f.name) {
            that.querySmapsData(fileStartNs, f.startNs);
          }
        }
        e.stopPropagation();
      });
    });
  }

  private async querySmapsData(baseTime: number, targetTime: number): Promise<void> {
    const baseArr: SmapsCompareStruct[] = [];
    const targetArr: SmapsCompareStruct[] = [];
    // 点击的
    await getTabSmapsStatisticData(baseTime).then(async (smapsComResults) => {
      this.smapsCompariosnTable!.loading = false;
      for (let i = 0; i < smapsComResults.length; i++) {
        baseArr.push(
          new SmapsCompareStruct(
            smapsComResults[i].type,
            smapsComResults[i].path,
            smapsComResults[i].size,
            smapsComResults[i].count,
            smapsComResults[i].rss,
            smapsComResults[i].pss,
            smapsComResults[i].sharedClean,
            smapsComResults[i].sharedDirty,
            smapsComResults[i].privateClean,
            smapsComResults[i].privateDirty,
            smapsComResults[i].swap,
            smapsComResults[i].swapPss
          )
        );
      }
      // 被比较的
      await getTabSmapsStatisticData(targetTime).then((results) => {
        for (let i = 0; i < results.length; i++) {
          targetArr.push(
            new SmapsCompareStruct(
              results[i].type,
              results[i].path,
              results[i].size,
              results[i].count,
              results[i].rss,
              results[i].pss,
              results[i].sharedClean,
              results[i].sharedDirty,
              results[i].privateClean,
              results[i].privateDirty,
              results[i].swap,
              results[i].swapPss
            )
          );
        }
        let compareData = this.compare(baseArr, targetArr);
        this.filteredData(compareData, this.smapsCompariosnTable!);
      });
    });
  }

  private compare(base: Array<SmapsCompareStruct>, target: Array<SmapsCompareStruct>): Array<SmapsCompareStruct> {
    const diffMap = new Map<string, SmapsCompareStruct>();

    for (const item of base) {
      diffMap.set(item.type + ' ' + item.path, item.clone(true));
    }

    for (const item of target) {
      if (diffMap.has(item.type + ' ' + item.path)) {
        const diffItem = diffMap.get(item.type + ' ' + item.path);
        diffItem!.size = diffItem!.size - item.size;
        diffItem!.count = diffItem!.count - item.count;
        diffItem!.rss = diffItem!.rss - item.rss;
        diffItem!.pss = diffItem!.pss - item.pss;
        diffItem!.sharedClean = diffItem!.sharedClean - item.sharedClean;
        diffItem!.sharedDirty = diffItem!.sharedDirty - item.sharedDirty;
        diffItem!.privateClean = diffItem!.privateClean - item.privateClean;
        diffItem!.privateDirty = diffItem!.privateDirty - item.privateDirty;
        diffItem!.swap = diffItem!.swap - item.swap;
        diffItem!.swapPss = diffItem!.swapPss - item.swapPss;
      } else {
        diffMap.set(item.type + ' ' + item.path, item.clone());
      }
    }
    return Array.from(diffMap.values());
  }

  public connectedCallback(): void {
    super.connectedCallback();
    resizeObserverFromMemory(this.parentElement!, this.smapsCompariosnTable!, this.filterEl!);
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
    <div style="overflow: auto;" class="d-box">
        <lit-table id="tb-smaps-comparison" class="smaps-comparison-table" style="height: auto;" tree>
            <lit-table-column width="250px" title="Type" data-index="typeName" key="typeName" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="Path" data-index="path" key="path" align="flex-start" >
            </lit-table-column>
            <lit-table-column  width="150px" title="SizeDelta" data-index="sizeStr" key="sizeStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column  width="150px" title="Count" data-index="count" key="count" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="RssDelta" data-index="rssStr" key="rssStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="PssDelta" data-index="pssStr" key="pssStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column  width="150px" title="SharedCleanDelta" data-index="sharedCleanStr" key="sharedCleanStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="SharedDirtyDelta" data-index="sharedDirtyStr" key="sharedDirtyStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="PrivateCleanDelta" data-index="privateCleanStr" key="privateCleanStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="PrivateDirtyDelta" data-index="privateDirtyStr" key="privateDirtyStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="SwapDelta" data-index="swapStr" key="swapStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="150px" title="SwapPssDelta" data-index="swapPssStr" key="swapPssStr" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <tab-pane-js-memory-filter id="filter" first hideFilter></tab-pane-js-memory-filter>
    </div>`;
  }
}

class SmapsCompareStruct {
  type: SmapsType;
  path: string;
  size: number;
  count: number;
  rss: number;
  pss: number;
  sharedClean: number;
  sharedDirty: number;
  privateClean: number;
  privateDirty: number;
  swap: number;
  swapPss: number;

  constructor(
    type: SmapsType,
    path: string,
    size: number,
    count: number,
    rss: number,
    pss: number,
    sharedClean: number,
    sharedDirty: number,
    privateClean: number,
    privateDirty: number,
    swap: number,
    swapPss: number
  ) {
    this.type = type;
    this.path = path;
    this.size = size;
    this.count = count;
    this.rss = rss;
    this.pss = pss;
    this.sharedClean = sharedClean;
    this.sharedDirty = sharedDirty;
    this.privateClean = privateClean;
    this.privateDirty = privateDirty;
    this.swap = swap;
    this.swapPss = swapPss;
  }

  clone(isBase?: boolean): SmapsCompareStruct {
    if (isBase) {
      return new SmapsCompareStruct(
        this.type,
        this.path,
        this.size,
        this.count,
        this.rss,
        this.pss,
        this.sharedClean,
        this.sharedDirty,
        this.privateClean,
        this.privateDirty,
        this.swap,
        this.swapPss
      );
    } else {
      return new SmapsCompareStruct(
        this.type,
        this.path,
        -this.size,
        -this.count,
        -this.rss,
        -this.pss,
        -this.sharedClean,
        -this.sharedDirty,
        -this.privateClean,
        -this.privateDirty,
        -this.swap,
        -this.swapPss
      );
    }
  }
}
