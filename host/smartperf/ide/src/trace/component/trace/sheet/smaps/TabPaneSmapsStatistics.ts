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
import {
  getTabSmapsMaxSize,
  getTabSmapsStatisticData,
  getTabSmapsStatisticMaxSize,
  getTabSmapsStatisticSelectData,
} from '../../../../database/SqlLite.js';
import { Smaps, SmapsTreeObj, SmapsType, TYPE_STRING } from '../../../../bean/SmapsStruct.js';
import { Utils } from '../../base/Utils.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
@element('tabpane-smaps-statistics')
export class TabPaneSmapsStatistics extends BaseElement {
  private tblSmapsStatistics: LitTable | null | undefined;
  private isClick = false;
  private currentSelection: SelectionParam | null | undefined;
  private sumSize: number = 0;
  private sortArray: Array<SmapsTreeObj> = [];
  private totalTree: Array<SmapsTreeObj> = [];
  public tabTitle: HTMLDivElement | undefined | null;
  private allTree: SmapsTreeObj | undefined | null;

  public initElements(): void {
    this.tblSmapsStatistics = this.shadowRoot?.querySelector<LitTable>('lit-table');
    this.tabTitle = this.tblSmapsStatistics!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.tblSmapsStatistics!.addEventListener('column-click', (evt) => {
      //   @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort, this.tblSmapsStatistics);
    });
  }

  set data(valSmapsStatistics: SelectionParam) {
    if (!this.tblSmapsStatistics) {
      return;
    }
    this.parentElement!.style.overflow = 'unset';
    this.currentSelection = valSmapsStatistics;
    this.isClick = valSmapsStatistics.smapsType.length === 0;
    this.tblSmapsStatistics!.loading = true;
    this.init(this.tabTitle!);
    if (!this.isClick) {
      if (valSmapsStatistics.smapsType.length > 0) {
        this.queryDataByDB(valSmapsStatistics);
      }
    } else {
      this.setSmaps(valSmapsStatistics);
    }
  }

  connectedCallback(): void {
    super.connectedCallback();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.tblSmapsStatistics?.shadowRoot?.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 15 + 'px';
        this.tblSmapsStatistics?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }
  async queryDataByDB(smapsVal: SelectionParam) {
    getTabSmapsMaxSize(smapsVal.leftNs, smapsVal.rightNs, (MemoryConfig.getInstance().interval * 1000_000) / 5).then(
      (maxRes) => {
        this.sumSize = maxRes[0].max_value;
      }
    );
    await getTabSmapsStatisticSelectData(
      smapsVal.leftNs,
      smapsVal.rightNs,
      (MemoryConfig.getInstance().interval * 1000_000) / 5
    ).then((result) => {
      this.tblSmapsStatistics!.loading = false;
      this.filteredData(result, this.tblSmapsStatistics!, this.sumSize);
    });
  }

  private calculatePercentage(divisor: number, dividend: number) {
    if (dividend === 0) {
      return 0;
    } else {
      return (divisor / dividend) * 100;
    }
  }

  public init(tabTitle: HTMLDivElement): void {
    const thTable = tabTitle!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (tabTitle!.hasAttribute('sort')) {
      tabTitle!.removeAttribute('sort');
      list.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  private handleSmapsTreeObj(smapsTreeObj: SmapsTreeObj, sumSize?: number): void {
    smapsTreeObj.sizeStr = Utils.getBinaryByteWithUnit(smapsTreeObj.size);
    smapsTreeObj.rssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.rss);
    smapsTreeObj.pssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.pss);
    smapsTreeObj.sizePro = this.calculatePercentage(smapsTreeObj.size, sumSize!);
    smapsTreeObj.sizeProStr = smapsTreeObj.sizePro.toFixed(2) + '%';
    smapsTreeObj.sharedCleanStr = Utils.getBinaryByteWithUnit(smapsTreeObj.sharedClean);
    smapsTreeObj.sharedDirtyStr = Utils.getBinaryByteWithUnit(smapsTreeObj.sharedDirty);
    smapsTreeObj.privateCleanStr = Utils.getBinaryByteWithUnit(smapsTreeObj.privateClean);
    smapsTreeObj.privateDirtyStr = Utils.getBinaryByteWithUnit(smapsTreeObj.privateDirty);
    smapsTreeObj.swapStr = Utils.getBinaryByteWithUnit(smapsTreeObj.swap);
    smapsTreeObj.swapPssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.swapPss);
  }

  private handleAllDataTree(
    smaps: Smaps,
    id: number,
    parentId: string,
    smapsStatAllDataTree: SmapsTreeObj,
    sumSize?: number
  ): void {
    let type = smaps.typeName;
    let objTree = new SmapsTreeObj(id + '', parentId, type);
    objTree.path = SpSystemTrace.DATA_DICT.get(Number(smaps.path))?.split('/');
    if (sumSize) {
      objTree.sizePro = this.calculatePercentage(smaps.size, sumSize);
      objTree.sizeProStr = objTree.sizePro.toFixed(2) + '%';
    }
    objTree.size = smaps.size;
    objTree.sizeStr = Utils.getBinaryByteWithUnit(smaps.size);
    objTree.rss = smaps.rss;
    objTree.rssStr = Utils.getBinaryByteWithUnit(smaps.rss);
    objTree.pss = smaps.pss;
    objTree.pssStr = Utils.getBinaryByteWithUnit(smaps.pss);
    if (smapsStatAllDataTree.children.length >= 1 && smapsStatAllDataTree.path !== '< multiple >') {
      smapsStatAllDataTree.path = '< multiple >';
    }

    smapsStatAllDataTree.size += smaps.size;
    smapsStatAllDataTree.count += smaps.count;
    smapsStatAllDataTree.rss += smaps.rss;
    smapsStatAllDataTree.pss += smaps.pss;
    smapsStatAllDataTree.sharedClean += smaps.sharedClean;
    smapsStatAllDataTree.sharedDirty += smaps.sharedDirty;
    smapsStatAllDataTree.privateClean += smaps.privateClean;
    smapsStatAllDataTree.privateDirty += smaps.privateDirty;
    smapsStatAllDataTree.swap += smaps.swap;
    smapsStatAllDataTree.swapPss += smaps.swapPss;
  }

  private handleTree(smaps: Smaps, id: number, parentId: string, smapsStatDataTree: SmapsTreeObj, sumSize?: number): void {
    let type = TYPE_STRING[smaps.type];
    let treeObj = new SmapsTreeObj(id + '', parentId, type);
    treeObj.path = SpSystemTrace.DATA_DICT.get(Number(smaps.path))?.split('/');
    treeObj.size = smaps.size;
    treeObj.sizeStr = Utils.getBinaryByteWithUnit(smaps.size);
    treeObj.count = smaps.count;
    treeObj.rss = smaps.rss;
    treeObj.rssStr = Utils.getBinaryByteWithUnit(smaps.rss);
    treeObj.pss = smaps.pss;
    treeObj.pssStr = Utils.getBinaryByteWithUnit(smaps.pss);
    treeObj.sharedClean = smaps.sharedClean;
    treeObj.sharedCleanStr = Utils.getBinaryByteWithUnit(smaps.sharedClean);
    treeObj.sharedDirty = smaps.sharedDirty;
    treeObj.sharedDirtyStr = Utils.getBinaryByteWithUnit(smaps.sharedDirty);
    treeObj.privateClean = smaps.privateClean;
    treeObj.privateCleanStr = Utils.getBinaryByteWithUnit(smaps.privateClean);
    treeObj.privateDirty = smaps.privateDirty;
    treeObj.privateDirtyStr = Utils.getBinaryByteWithUnit(smaps.privateDirty);
    treeObj.swap = smaps.swap;
    treeObj.swapStr = Utils.getBinaryByteWithUnit(smaps.swap);
    treeObj.swapPss = smaps.swapPss;
    treeObj.swapPssStr = Utils.getBinaryByteWithUnit(smaps.swapPss);

    if (sumSize) {
      treeObj.sizePro = this.calculatePercentage(smaps.size, sumSize || 0);
      treeObj.sizeProStr = treeObj.sizePro.toFixed(2) + '%';
    }

    if (smapsStatDataTree.children.length >= 1 && smapsStatDataTree.path !== '< multiple >') {
      smapsStatDataTree.path = '< multiple >';
    }

    smapsStatDataTree.size += smaps.size;
    smapsStatDataTree.count += smaps.count;
    smapsStatDataTree.rss += smaps.rss;
    smapsStatDataTree.pss += smaps.pss;
    smapsStatDataTree.sharedClean += smaps.sharedClean;
    smapsStatDataTree.sharedDirty += smaps.sharedDirty;
    smapsStatDataTree.privateClean += smaps.privateClean;
    smapsStatDataTree.privateDirty += smaps.privateDirty;
    smapsStatDataTree.swap += smaps.swap;
    smapsStatDataTree.swapPss += smaps.swapPss;
    smapsStatDataTree.children.push(treeObj);
  }

  async setSmaps(data: SelectionParam) {
    getTabSmapsStatisticMaxSize(data.leftNs).then((maxRes) => {
      this.sumSize = maxRes[0].max_value;
    });
    await getTabSmapsStatisticData(data.leftNs).then((result) => {
      this.tblSmapsStatistics!.loading = false;
      this.filteredData(result, this.tblSmapsStatistics!, this.sumSize);
    });
  }

  public filteredData(result: Array<any>, table: LitTable, sumSize?: number): void {
    this.allTree = new SmapsTreeObj('All', '', '*All*');
    let codeSysTree: SmapsTreeObj = new SmapsTreeObj('CODE_SYS', '', 'CODE_SYS');
    let codeAppTree: SmapsTreeObj = new SmapsTreeObj('CODE_APP', '', 'CODE_APP');
    let dataSysTree: SmapsTreeObj = new SmapsTreeObj('DATA_SYS', '', 'DATA_SYS');
    let dataAppTree: SmapsTreeObj = new SmapsTreeObj('DATA_APP', '', 'DATA_APP');
    let unKownTree: SmapsTreeObj = new SmapsTreeObj('UNKNOWN_ANON', '', 'UNKNOWN_ANON');
    let stackTree: SmapsTreeObj = new SmapsTreeObj('STACK', '', 'STACK');
    let jsTree: SmapsTreeObj = new SmapsTreeObj('JS_HEAP', '', 'JS_HEAP');
    let javaVmTree: SmapsTreeObj = new SmapsTreeObj('JAVA_VM', '', 'JAVA_VM');
    let nativeTree: SmapsTreeObj = new SmapsTreeObj('NATIVE_HEAP', '', 'NATIVE_HEAP');
    let ashMemTree: SmapsTreeObj = new SmapsTreeObj('ASHMEM', '', 'ASHMEM');
    let otherSysTree: SmapsTreeObj = new SmapsTreeObj('OTHER_SYS', '', 'OTHER_SYS');
    let otherAppTree: SmapsTreeObj = new SmapsTreeObj('OTHER_APP', '', 'OTHER_APP');
    if (result.length !== null && result.length > 0) {
      for (let id = 0; id < result.length; id++) {
        let smaps = result[id];
        smaps.typeName = TYPE_STRING[smaps.type];
        switch (smaps.type) {
          case SmapsType.TYPE_CODE_SYS:
            this.handleTree(smaps, id, smaps.typeName, codeSysTree, sumSize);
            break;
          case SmapsType.TYPE_CODE_APP:
            this.handleTree(smaps, id, smaps.typeName, codeAppTree, sumSize);
            break;
          case SmapsType.TYPE_DATA_SYS:
            this.handleTree(smaps, id, smaps.typeName, dataSysTree, sumSize);
            break;
          case SmapsType.TYPE_DATA_APP:
            this.handleTree(smaps, id, smaps.typeName, dataAppTree, sumSize);
            break;
          case SmapsType.TYPE_UNKNOWN_ANON:
            this.handleTree(smaps, id, smaps.typeName, unKownTree, sumSize);
            break;
          case SmapsType.TYPE_STACK:
            this.handleTree(smaps, id, smaps.typeName, stackTree, sumSize);
            break;
          case SmapsType.TYPE_JS_HEAP:
            this.handleTree(smaps, id, smaps.typeName, jsTree, sumSize);
            break;
          case SmapsType.TYPE_JAVA_VM:
            this.handleTree(smaps, id, smaps.typeName, javaVmTree, sumSize);
            break;
          case SmapsType.TYPE_NATIVE_HEAP:
            this.handleTree(smaps, id, smaps.typeName, nativeTree, sumSize);
            break;
          case SmapsType.TYPE_ASHMEM:
            this.handleTree(smaps, id, smaps.typeName, ashMemTree, sumSize);
            break;
          case SmapsType.TYPE_OTHER_SYS:
            this.handleTree(smaps, id, smaps.typeName, otherSysTree, sumSize);
            break;
          case SmapsType.TYPE_OTHER_APP:
            this.handleTree(smaps, id, smaps.typeName, otherAppTree, sumSize);
            break;
        }

        this.handleAllDataTree(smaps, id, 'All', this.allTree, sumSize!);
        if (id === result.length - 1) {
          this.handleSmapsTreeObj(codeSysTree, sumSize);
          this.handleSmapsTreeObj(codeAppTree, sumSize);
          this.handleSmapsTreeObj(dataSysTree, sumSize);
          this.handleSmapsTreeObj(dataAppTree, sumSize);
          this.handleSmapsTreeObj(unKownTree, sumSize);
          this.handleSmapsTreeObj(stackTree, sumSize);
          this.handleSmapsTreeObj(jsTree, sumSize);
          this.handleSmapsTreeObj(javaVmTree, sumSize);
          this.handleSmapsTreeObj(nativeTree, sumSize);
          this.handleSmapsTreeObj(ashMemTree, sumSize);
          this.handleSmapsTreeObj(otherSysTree, sumSize);
          this.handleSmapsTreeObj(otherAppTree, sumSize);
          this.handleSmapsTreeObj(this.allTree, sumSize!);
        }
      }
      let treeList = [
        this.allTree,
        codeSysTree,
        codeAppTree,
        dataSysTree,
        dataAppTree,
        unKownTree,
        stackTree,
        jsTree,
        javaVmTree,
        nativeTree,
        ashMemTree,
        otherSysTree,
        otherAppTree,
      ];
      this.totalTree = [];
      for (let i = 0; i < treeList.length; i++) {
        let tree = treeList[i];
        if (tree.children.length !== 0) {
          this.totalTree.push(tree);
        }
      }

      // @ts-ignore
      this.totalTree.sort((previous, next) => next.size - previous.size);
      this.totalTree.unshift(this.allTree);
      table!.recycleDataSource = this.totalTree;
      this.totalTree.shift();
      table?.reMeauseHeight();
    } else {
      table!.recycleDataSource = [];
      table?.reMeauseHeight();
    }
  }

  public sortByColumn(column: string, sort: number, table: LitTable) {
    this.sortArray = [...this.totalTree];
    switch (sort) {
      case 0:
        this.sortArray.sort((previous, next) => {
          return next.size - previous.size;
        });
        this.sortArray.unshift(this.allTree!);
        table!.recycleDataSource = this.totalTree;
        this.sortArray.shift();
        break;
      default:
        switch (column) {
          case 'sizeStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.size - next.size : next.size - previous.size;
            });
            break;
          case 'sizeProStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.size - next.size : next.size - previous.size;
            });
            break;
          case 'count':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.count - next.count : next.count - previous.count;
            });
            break;
          case 'rssStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.rss - next.rss : next.rss - previous.rss;
            });
            break;
          case 'typeName':
            this.sortArray.sort((previous, next) => {
              return sort === 1
                ? previous.typeName.toString().localeCompare(next.typeName.toString())
                : next.typeName.toString().localeCompare(previous.typeName.toString());
            });
            break;
          case 'pssStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.pss - next.pss : next.pss - previous.pss;
            });
            break;
          case 'sharedCleanStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.sharedClean - next.sharedClean : next.sharedClean - previous.sharedClean;
            });
            break;
          case 'sharedDirtyStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.sharedDirty - next.sharedDirty : next.sharedDirty - previous.sharedDirty;
            });
            break;
          case 'privateCleanStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.privateClean - next.privateClean : next.privateClean - previous.privateClean;
            });
            break;
          case 'privateDirtyStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.privateDirty - next.privateDirty : next.privateDirty - previous.privateDirty;
            });
            break;
          case 'swapStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.swap - next.swap : next.swap - previous.swap;
            });
            break;
          case 'swapPssStr':
            this.sortArray.sort((previous, next) => {
              return sort === 1 ? previous.swapPss - next.swapPss : next.swapPss - previous.swapPss;
            });
            break;
        }
        break;
    }
    this.sortArray.unshift(this.allTree!);
    table!.recycleDataSource = this.sortArray;
    this.sortArray.shift();
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
            <lit-table id="tb-smaps-statistics" class="smaps-statistics-table" style="height: auto;" tree>
                <lit-table-column width="250px" title="Type" data-index="typeName" key="typeName" align="flex-start" order>
                </lit-table-column>
                <lit-table-column width="150px" title="Path" data-index="path" key="path" align="flex-start">
                </lit-table-column>
                <lit-table-column  width="150px" title="Size" data-index="sizeStr" key="sizeStr" align="flex-start" order>
                </lit-table-column>
                <lit-table-column  width="150px" title="% of Size" data-index="sizeProStr" key="sizeProStr" align="flex-start" order>
                </lit-table-column>
                <lit-table-column  width="150px" title="Count" data-index="count" key="count" align="flex-start" order>
                </lit-table-column>
                <lit-table-column width="150px" title="Rss" data-index="rssStr" key="rssStr" align="flex-start" order>
                </lit-table-column>
                <lit-table-column width="150px" title="Pss" data-index="pssStr" key="pssStr" align="flex-start" order>
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
            </lit-table>
        </div>
        `;
  }
}
