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
  getTabSmapsData,
  getTabSmapsMaxSize,
  getTabSmapsRecordData,
  getTabSmapsStatisticData,
  getTabSmapsStatisticMaxSize,
  getTabSmapsStatisticSelectData,
} from '../../../../database/SqlLite.js';
import { Smaps, SmapsTreeObj, SmapsType, TYPE_STRING } from '../../../../bean/SmapsStruct.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
@element('tabpane-smaps-statistics')
export class TabPaneSmapsStatistics extends BaseElement {
  private tblSmapsStatistics: LitTable | null | undefined;
  private isClick = false;
  private currentSelection: SelectionParam | null | undefined;
  private sumSize: number = 0;
  private sortArray: Array<any> = [];
  private totalTree: Array<any> = [];
  private tabTitle: HTMLDivElement | undefined | null;
  set data(valSmapsStatistics: SelectionParam) {
    this.currentSelection = valSmapsStatistics;
    this.isClick = valSmapsStatistics.smapsType.length === 0;
    this.init();
    this.tblSmapsStatistics!.loading = true;
    if (!this.isClick) {
      if (valSmapsStatistics.smapsType.length > 0) {
        this.queryDataByDB(valSmapsStatistics);
      }
    } else {
      this.setSmaps(valSmapsStatistics);
    }
  }
  initElements(): void {
    this.tblSmapsStatistics = this.shadowRoot?.querySelector<LitTable>('#tb-smaps-statistics');
    this.tabTitle = this.tblSmapsStatistics!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.tblSmapsStatistics!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }
  connectedCallback(): void {
    super.connectedCallback();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.tblSmapsStatistics?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight  - 15+ 'px';
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
      this.filteredData(result, this.sumSize);
    });
  }
  private calculatePercentage(divisor: number, dividend: number) {
    if (dividend === 0) {
      return 0;
    } else {
      return (divisor / dividend) * 100;
    }
  }
  private init(): void {
    const thTable = this.tabTitle!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      list.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }
  private handleSmapsTreeObj(smapsTreeObj: SmapsTreeObj, sumRss: number): void {
    smapsTreeObj.regStr = smapsTreeObj.reg + '';
    smapsTreeObj.rssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.rss);
    smapsTreeObj.dirtyStr = Utils.getBinaryByteWithUnit(smapsTreeObj.dirty);
    smapsTreeObj.swapperStr = Utils.getBinaryByteWithUnit(smapsTreeObj.swapper);
    smapsTreeObj.sizeStr = Utils.getBinaryByteWithUnit(smapsTreeObj.size);
    smapsTreeObj.respro = this.calculatePercentage(smapsTreeObj.rss, smapsTreeObj.size);
    smapsTreeObj.pssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.pss);
    smapsTreeObj.resproStr = smapsTreeObj.respro.toFixed(2) + '%';
    smapsTreeObj.sizePro = this.calculatePercentage(smapsTreeObj.size, sumRss);
    smapsTreeObj.sizeProStr = smapsTreeObj.sizePro.toFixed(2) + '%';
    smapsTreeObj.sharedCleanStr = Utils.getBinaryByteWithUnit(smapsTreeObj.sharedClean);
    smapsTreeObj.sharedDirtyStr = Utils.getBinaryByteWithUnit(smapsTreeObj.sharedDirty);
    smapsTreeObj.privateCleanStr = Utils.getBinaryByteWithUnit(smapsTreeObj.privateClean);
    smapsTreeObj.privateDirtyStr = Utils.getBinaryByteWithUnit(smapsTreeObj.privateDirty);
    smapsTreeObj.swapStr = Utils.getBinaryByteWithUnit(smapsTreeObj.swap);
    smapsTreeObj.swapPssStr = Utils.getBinaryByteWithUnit(smapsTreeObj.swapPss);
  }
  private handleAllDataTree(smaps: Smaps, id: number, parentId: string, dataTree: SmapsTreeObj, sumRss: number): void {
    let type = smaps.typeName;
    let objTree = new SmapsTreeObj(id + '', parentId, type);
    objTree.path = SpSystemTrace.DATA_DICT.get(Number(smaps.path))?.split('/');
    objTree.rss = smaps.rss;
    objTree.sizePro = this.calculatePercentage(smaps.size, sumRss);
    objTree.sizeProStr = objTree.sizePro.toFixed(2) + '%';
    objTree.rssStr = Utils.getBinaryByteWithUnit(smaps.rss);
    objTree.dirty = smaps.dirty;
    objTree.dirtyStr = Utils.getBinaryByteWithUnit(smaps.dirty);
    objTree.swapper = smaps.swapper;
    objTree.swapperStr = Utils.getBinaryByteWithUnit(smaps.swapper);
    objTree.size = smaps.size;
    objTree.sizeStr = Utils.getBinaryByteWithUnit(smaps.size);
    objTree.pss = smaps.pss;
    objTree.pssStr = Utils.getBinaryByteWithUnit(smaps.pss);
    objTree.respro = smaps.reside;
    objTree.resproStr = smaps.reside.toFixed(2) + '%';
    dataTree.reg += 1;
    if (dataTree.children.length >= 1 && dataTree.path !== '< multiple >') {
      dataTree.path = '< multiple >';
    }
    dataTree.rss += smaps.rss;
    dataTree.dirty += smaps.dirty;
    dataTree.swapper += smaps.swapper;
    dataTree.size += smaps.size;
    dataTree.respro += smaps.reside;
    dataTree.pss += smaps.pss;
    dataTree.count += smaps.count;
    dataTree.sharedClean += smaps.shared_clean;
    dataTree.sharedDirty += smaps.shared_dirty;
    dataTree.privateClean += smaps.private_clean;
    dataTree.privateDirty += smaps.private_dirty;
    dataTree.swap += smaps.swap;
    dataTree.swapPss += smaps.swap_pss;
  }
  private handleTree(smaps: Smaps, id: number, parentId: string, dataTree: SmapsTreeObj, sumRss: number): void {
    let type = TYPE_STRING[smaps.type];
    let treeObj = new SmapsTreeObj(id + '', parentId, type);
    treeObj.path = SpSystemTrace.DATA_DICT.get(Number(smaps.path))?.split('/');
    treeObj.rss = smaps.rss;
    treeObj.pss = smaps.pss;
    treeObj.sizePro = this.calculatePercentage(smaps.size, sumRss);
    treeObj.sizeProStr = treeObj.sizePro.toFixed(2) + '%';
    treeObj.rssStr = Utils.getBinaryByteWithUnit(smaps.rss);
    treeObj.dirty = smaps.dirty;
    treeObj.count = smaps.count;
    treeObj.dirtyStr = Utils.getBinaryByteWithUnit(smaps.dirty);
    treeObj.swapper = smaps.swapper;
    treeObj.swapperStr = Utils.getBinaryByteWithUnit(smaps.swapper);
    treeObj.size = smaps.size;
    treeObj.sizeStr = Utils.getBinaryByteWithUnit(smaps.size);
    treeObj.pss = smaps.pss;
    treeObj.pssStr = Utils.getBinaryByteWithUnit(smaps.pss);
    treeObj.respro = smaps.reside;
    treeObj.resproStr = smaps.reside.toFixed(2) + '%';
    treeObj.sharedClean = smaps.shared_clean;
    treeObj.sharedCleanStr = Utils.getBinaryByteWithUnit(smaps.shared_clean);
    treeObj.sharedDirty = smaps.shared_dirty;
    treeObj.sharedDirtyStr = Utils.getBinaryByteWithUnit(smaps.shared_dirty);
    treeObj.privateClean = smaps.private_clean;
    treeObj.privateCleanStr = Utils.getBinaryByteWithUnit(smaps.private_clean);
    treeObj.privateDirty = smaps.private_dirty;
    treeObj.privateDirtyStr = Utils.getBinaryByteWithUnit(smaps.private_dirty);
    treeObj.swap = smaps.swap;
    treeObj.swapStr = Utils.getBinaryByteWithUnit(smaps.swap);
    treeObj.swapPss = smaps.swap_pss;
    treeObj.swapPssStr = Utils.getBinaryByteWithUnit(smaps.swap_pss);
    dataTree.reg += 1;
    if (dataTree.children.length >= 1 && dataTree.path !== '< multiple >') {
      dataTree.path = '< multiple >';
    }
    dataTree.rss += smaps.rss;
    dataTree.dirty += smaps.dirty;
    dataTree.swapper += smaps.swapper;
    dataTree.size += smaps.size;
    dataTree.pss += smaps.pss;
    dataTree.count += smaps.count;
    dataTree.sharedClean += smaps.shared_clean;
    dataTree.sharedDirty += smaps.shared_dirty;
    dataTree.privateClean += smaps.private_clean;
    dataTree.privateDirty += smaps.private_dirty;
    dataTree.swap += smaps.swap;
    dataTree.swapPss += smaps.swap_pss;
    dataTree.swap += smaps.swap;
    dataTree.swapPss += smaps.swap_pss;
    dataTree.children.push(treeObj);
  }
  async setSmaps(data: SelectionParam) {
    getTabSmapsStatisticMaxSize(data.rightNs).then((maxRes) => {
      this.sumSize = maxRes[0].max_value;
    });
    await getTabSmapsStatisticData(data.rightNs).then((result) => {
      this.tblSmapsStatistics!.loading = false;
      this.filteredData(result, this.sumSize);
    });
  }
  filteredData(result: any, sumRss: number): void {
    let allTree: SmapsTreeObj = new SmapsTreeObj('All', '', '*All*');
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
            this.handleTree(smaps, id, smaps.typeName, codeSysTree, sumRss);
            break;
          case SmapsType.TYPE_CODE_APP:
            this.handleTree(smaps, id, smaps.typeName, codeAppTree, sumRss);
            break;
          case SmapsType.TYPE_DATA_SYS:
            this.handleTree(smaps, id, smaps.typeName, dataSysTree, sumRss);
            break;
          case SmapsType.TYPE_DATA_APP:
            this.handleTree(smaps, id, smaps.typeName, dataAppTree, sumRss);
            break;
          case SmapsType.TYPE_UNKNOWN_ANON:
            this.handleTree(smaps, id, smaps.typeName, unKownTree, sumRss);
            break;
          case SmapsType.TYPE_STACK:
            this.handleTree(smaps, id, smaps.typeName, stackTree, sumRss);
            break;
          case SmapsType.TYPE_JS_HEAP:
            this.handleTree(smaps, id, smaps.typeName, jsTree, sumRss);
            break;
          case SmapsType.TYPE_JAVA_VM:
            this.handleTree(smaps, id, smaps.typeName, javaVmTree, sumRss);
            break;
          case SmapsType.TYPE_NATIVE_HEAP:
            this.handleTree(smaps, id, smaps.typeName, nativeTree, sumRss);
            break;
          case SmapsType.TYPE_ASHMEM:
            this.handleTree(smaps, id, smaps.typeName, ashMemTree, sumRss);
            break;
          case SmapsType.TYPE_OTHER_SYS:
            this.handleTree(smaps, id, smaps.typeName, otherSysTree, sumRss);
            break;
          case SmapsType.TYPE_OTHER_APP:
            this.handleTree(smaps, id, smaps.typeName, otherAppTree, sumRss);
            break;
        }
        this.handleAllDataTree(smaps, id, 'All', allTree, sumRss);
        if (id === result.length - 1) {
          this.handleSmapsTreeObj(codeSysTree, sumRss);
          this.handleSmapsTreeObj(codeAppTree, sumRss);
          this.handleSmapsTreeObj(dataSysTree, sumRss);
          this.handleSmapsTreeObj(dataAppTree, sumRss);
          this.handleSmapsTreeObj(unKownTree, sumRss);
          this.handleSmapsTreeObj(stackTree, sumRss);
          this.handleSmapsTreeObj(jsTree, sumRss);
          this.handleSmapsTreeObj(javaVmTree, sumRss);
          this.handleSmapsTreeObj(nativeTree, sumRss);
          this.handleSmapsTreeObj(ashMemTree, sumRss);
          this.handleSmapsTreeObj(otherSysTree, sumRss);
          this.handleSmapsTreeObj(otherAppTree, sumRss);
          this.handleSmapsTreeObj(allTree, sumRss);
        }
      }
      let treeList = [
        allTree,
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
      this.totalTree.push(allTree);
      // @ts-ignore
      this.totalTree.sort((a, b) => b.size - a.size);
      this.tblSmapsStatistics!.recycleDataSource = this.totalTree;
      this.tblSmapsStatistics?.reMeauseHeight();
    } else {
      this.tblSmapsStatistics!.recycleDataSource = [];
      this.tblSmapsStatistics?.reMeauseHeight();
    }
  }
  sortByColumn(column: string, sort: number) {
    switch (sort) {
      case 0:
        this.tblSmapsStatistics!.snapshotDataSource = this.totalTree;
        break;
      default:
        this.sortArray = [...this.totalTree];
        switch (column) {
          case 'sizeStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.size - b.size : b.size - a.size;
            });
            break;
          case 'count':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.count - b.count : b.count - a.count;
            });
            break;
          case 'rssStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.rss - b.rss : b.rss - a.rss;
            });
            break;
          case 'typeName':
            this.tblSmapsStatistics!.recycleDataSource = this.sortArray.sort((a, b) => {
              if (sort === 1) {
                if (a.typeName > b.typeName) {
                  return 1;
                } else if (a.typeName === b.typeName) {
                  return 0;
                } else {
                  return -1;
                }
              } else {
                if (b.typeName > a.typeName) {
                  return 1;
                } else if (a.typeName === b.typeName) {
                  return 0;
                } else {
                  return -1;
                }
              }
            });
            break;
          case 'pssStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.pss - b.pss : b.pss - a.pss;
            });
            break;
          case 'sharedCleanStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.sharedClean - b.sharedClean : b.sharedClean - a.sharedClean;
            });
            break;
          case 'sharedDirtyStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.sharedDirty - b.sharedDirty : b.sharedDirty - a.sharedDirty;
            });
            break;
          case 'privateCleanStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.privateClean - b.privateClean : b.privateClean - a.privateClean;
            });
            break;
          case 'privateDirtyStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.privateDirty - b.privateDirty : b.privateDirty - a.privateDirty;
            });
            break;
          case 'swapStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.swap - b.swap : b.swap - a.swap;
            });
            break;
          case 'swapPssStr':
            this.tblSmapsStatistics!.snapshotDataSource = this.sortArray.sort((a, b) => {
              return sort === 1 ? a.swapPss - b.swapPss : b.swapPss - a.swapPss;
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
        <div style="overflow: auto;" class="d-box">
            <lit-table id="tb-smaps-statistics" class="smaps-statistics-table" style="height: auto;" tree>
                <lit-table-column width="250px" title="Type" data-index="typeName" key="typeName" align="flex-start" order>
                </lit-table-column>
                <lit-table-column width="150px" title="Path" data-index="path" key="path" align="flex-start" >
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
