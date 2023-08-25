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
import { HeapDataInterface } from '../../../../../js-heap/HeapDataInterface.js';
import { ConstructorComparison, ConstructorItem, ConstructorType } from '../../../../../js-heap/model/UiStruct.js';
import { LitTableColumn } from '../../../../../base-ui/table/lit-table-column.js';
import '../../../../../base-ui/table/lit-table-column.js';
import { TabPaneJsMemoryFilter } from '../TabPaneJsMemoryFilter.js';
import '../TabPaneJsMemoryFilter.js';
import { HeapSnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerHeapSnapshot.js';
import { LitSelectOption } from '../../../../../base-ui/select/LitSelectOption.js';
import { LitSelect } from '../../../../../base-ui/select/LitSelect.js';

@element('tabpane-comparison')
export class TabPaneComparison extends BaseElement {
  private comparisonTableEl: LitTable | undefined | null;
  private retainerTableEl: LitTable | undefined | null;
  private comparisonsData: Array<ConstructorComparison> = [];
  private retainsData: Array<ConstructorItem> = [];
  private baseFileId: number | undefined | null;
  private targetFileId: number | undefined | null;
  private filterEl: TabPaneJsMemoryFilter | undefined | null;
  private selectEl: LitSelect | undefined | null;
  private search: HTMLInputElement | undefined | null;
  private comparisonData: Array<ConstructorItem> = [];
  private comparisonFilter: Array<ConstructorComparison> = [];
  private leftArray: Array<ConstructorComparison> = [];
  private rightArray: Array<ConstructorItem> = [];
  private rightTheadTable: HTMLDivElement | undefined | null;
  private leftTheadTable: HTMLDivElement | undefined | null;
  private comparisonTable: HTMLDivElement | undefined | null;
  private fileSize: number = 0;

  initElements(): void {
    this.comparisonTableEl = this.shadowRoot!.querySelector<LitTable>('#tb-comparison') as LitTable;
    this.retainerTableEl = this.shadowRoot!.querySelector<LitTable>('#tb-retainer') as LitTable;
    this.filterEl = this.shadowRoot!.querySelector<TabPaneJsMemoryFilter>('#filter');
    this.selectEl = this.filterEl?.shadowRoot?.querySelector<LitSelect>('lit-select');
    this.search = this.filterEl?.shadowRoot?.querySelector('#js-memory-filter-input') as HTMLInputElement;
    this.rightTheadTable = this.retainerTableEl!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.leftTheadTable = this.comparisonTableEl!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.comparisonTable = this.comparisonTableEl.shadowRoot?.querySelector('.table') as HTMLDivElement;
    this.comparisonTableEl!.addEventListener('icon-click', (e) => {
      // @ts-ignore
      let clickRow = e.detail.data;
      if (clickRow.status) {
        clickRow.targetFileId = this.targetFileId;
        let next = HeapDataInterface.getInstance().getNextForComparison(clickRow);
        clickRow.children = next;
        if (clickRow.children.length > 0) {
          for (let item of clickRow.children) {
            let nodeName = item.nodeName + ` @${item.id}`;
            item.nodeId = ` @${item.id}`;
            if (item.isString()) {
              item.objectName = '"' + item.nodeName + '"' + ` @${item.id}`;
            } else {
              item.objectName = nodeName;
            }
            item.deltaCount = '-';
            item.deltaSize = '-';
            if (item.edgeName != '') {
              item.objectName = item.edgeName + '\xa0' + '::' + '\xa0' + nodeName;
            } else {
              if (item.fileId == this.baseFileId) {
                item.addedCount = '•';
                item.addedSize = item.shallowSize;
                item.removedCount = '-';
                item.removedSize = '-';
              } else if (item.fileId) {
                item.removedCount = '•';
                item.removedSize = item.shallowSize;
                item.addedCount = '-';
                item.addedSize = '-';
              }
            }
            if (item.type == ConstructorType.FiledType) {
              item.removedCount = '-';
              item.removedSize = '-';
              item.addedCount = '-';
              item.addedSize = '-';
            }
          }
        } else {
          this.comparisonTableEl!.snapshotDataSource = [];
        }
      } else {
        clickRow.status = true;
      }
      if (this.search!.value != '') {
        if (this.leftTheadTable!.hasAttribute('sort')) {
          this.comparisonTableEl!.snapshotDataSource = this.leftArray;
        } else {
          this.comparisonTableEl!.snapshotDataSource = this.comparisonFilter;
        }
      } else {
        if (this.leftTheadTable!.hasAttribute('sort')) {
          this.comparisonTableEl!.snapshotDataSource = this.leftArray;
        } else {
          this.comparisonTableEl!.snapshotDataSource = this.comparisonsData;
        }
      }
      new ResizeObserver(() => {
        this.comparisonTableEl!.style.height = '100%';
        this.comparisonTableEl!.reMeauseHeight();
      }).observe(this.parentElement!);
    });
    this.retainerTableEl!.addEventListener('icon-click', (e) => {
      // @ts-ignore
      let retainerNext = e.detail.data as ConstructorItem;
      if (retainerNext) {
        if (this.retainsData.length > 0) {
          if (retainerNext.status) {
            retainerNext.getChildren();
            let i = 0;
            let that = this;
            let retainsTable = () => {
              const getList = (comList: Array<ConstructorItem>) => {
                comList.forEach((row) => {
                  let shallow = Math.round((row.shallowSize / this.fileSize) * 100) + '%';
                  let retained = Math.round((row.retainedSize / this.fileSize) * 100) + '%';
                  row.shallowPercent = shallow;
                  row.retainedPercent = retained;
                  let nodeId = row.nodeName + ` @${row.id}`;
                  row.objectName = row.edgeName + '\xa0' + 'in' + '\xa0' + nodeId;
                  if (row.distance >= 100000000 || row.distance == -5) {
                    // @ts-ignore
                    row.distance = '-';
                  }
                  i++;
                  // @ts-ignore
                  if (i < that.retainsData[0].distance - 1 && comList[0].distance != '-') {
                    comList[0].getChildren();
                    comList[0].expanded = false;
                    if (row.hasNext) {
                      getList(row.children);
                    }
                  } else {
                    return;
                  }
                });
              };
              getList(retainerNext.children);
            };
            retainsTable();
          } else {
            retainerNext.status = true;
          }
          if (this.rightTheadTable!.hasAttribute('sort')) {
            this.retainerTableEl!.snapshotDataSource = this.rightArray;
          } else {
            this.retainerTableEl!.snapshotDataSource = this.retainsData;
          }
        } else {
          this.retainerTableEl!.snapshotDataSource = [];
        }
        new ResizeObserver(() => {
          this.retainerTableEl!.style.height = 'calc(100% - 21px)';
          this.retainerTableEl!.reMeauseHeight();
        }).observe(this.parentElement!);
      }
    });
    this.comparisonTableEl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortComprisonByColumn(e.detail.key, e.detail.sort);
      this.comparisonTableEl!.reMeauseHeight();
    });
    this.retainerTableEl!.addEventListener('column-click', (e) => {
      // @ts-ignore
      this.sortRetainerByColumn(e.detail.key, e.detail.sort);
      this.retainerTableEl!.reMeauseHeight();
    });
    this.comparisonTableEl!.addEventListener('row-click', (e) => {
      this.rightTheadTable!.removeAttribute('sort');
      // @ts-ignore
      let item = e.detail.data as ConstructorItem;
      (item as any).isSelected = true;
      this.retainsData = HeapDataInterface.getInstance().getRetains(item);
      if (this.retainsData && this.retainsData.length > 0) {
        this.retainsData.forEach((comparisonRetainEl) => {
          let shallow = Math.round((comparisonRetainEl.shallowSize / this.fileSize) * 100) + '%';
          let retained = Math.round((comparisonRetainEl.retainedSize / this.fileSize) * 100) + '%';
          comparisonRetainEl.shallowPercent = shallow;
          comparisonRetainEl.retainedPercent = retained;
          if (comparisonRetainEl.distance >= 100000000 || comparisonRetainEl.distance === -5) {
            // @ts-ignore
            comparisonRetainEl.distance = '-';
          }
          let nodeId = comparisonRetainEl.nodeName + ` @${comparisonRetainEl.id}`;
          comparisonRetainEl.objectName = comparisonRetainEl.edgeName + '\xa0' + 'in' + '\xa0' + nodeId;
        });
        let i = 0;
        let that = this;
        if (this.retainsData[0].distance > 1) {
          this.retainsData[0].getChildren();
          this.retainsData[0].expanded = false;
        }
        let retainsTable = () => {
          const getList = (list: Array<ConstructorItem>) => {
            list.forEach((structRow) => {
              let shallow = Math.round((structRow.shallowSize / this.fileSize) * 100) + '%';
              let retained = Math.round((structRow.retainedSize / this.fileSize) * 100) + '%';
              structRow.shallowPercent = shallow;
              structRow.retainedPercent = retained;
              let nodeId = structRow.nodeName + ` @${structRow.id}`;
              structRow.objectName = structRow.edgeName + '\xa0' + 'in' + '\xa0' + nodeId;
              if (structRow.distance >= 100000000 || structRow.distance === -5) {
                // @ts-ignore
                structRow.distance = '-';
              }
              i++;
              // @ts-ignore
              if (i < that.retainsData[0].distance - 1 && list[0].distance != '-') {
                list[0].getChildren();
                list[0].expanded = false;
                if (structRow.hasNext) {
                  getList(structRow.children);
                }
              } else {
                return;
              }
            });
          };
          getList(that.retainsData[0].children);
        };
        retainsTable();
        this.retainerTableEl!.snapshotDataSource = this.retainsData;
      } else {
        this.retainerTableEl!.snapshotDataSource = [];
      }
      new ResizeObserver(() => {
        this.retainerTableEl!.style.height = 'calc(100% - 21px)';
        this.retainerTableEl!.reMeauseHeight();
      }).observe(this.parentElement!);
      // @ts-ignore
      if ((e.detail as any).callBack) {
        // @ts-ignore
        (e.detail as any).callBack(true);
      }
    });
    this.retainerTableEl!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data as ConstructorItem;
      (data as any).isSelected = true;
      if ((evt.detail as any).callBack) {
        (evt.detail as any).callBack(true);
      }
    });
    this.classFilter();
  }

  initComparison(data: HeapSnapshotStruct, dataList: Array<HeapSnapshotStruct>) {
    this.clear();
    this.retainerTableEl!.snapshotDataSource = [];
    let fileArr: HeapSnapshotStruct[] = [];
    let that = this;
    for (let file of dataList) {
      if (file.id !== data.id) {
        fileArr.push(file);
      }
    }
    fileArr = fileArr.sort();
    this.fileSize = data.size;
    this.initSelect(data.id, fileArr);
    this.baseFileId = data.id;
    this.targetFileId = fileArr[0].id;
    that.updateComparisonData(data.id, fileArr[0].id);
    new ResizeObserver(() => {
      this.comparisonTableEl!.style.height = '100%';
      this.comparisonTableEl!.reMeauseHeight();
    }).observe(this.parentElement!);
  }

  updateComparisonData(baseFileId: number, targetFileId: number) {
    this.comparisonsData = HeapDataInterface.getInstance().getClassListForComparison(baseFileId, targetFileId);
    this.comparisonsData.forEach((dataList) => {
      dataList.objectName = dataList.nodeName;
    });
    if (this.comparisonsData.length > 0) {
      this.comparisonData = this.comparisonsData;
      this.comparisonTableEl!.snapshotDataSource = this.comparisonsData;
    } else {
      this.comparisonTableEl!.snapshotDataSource = [];
    }
    this.comparisonTableEl!.reMeauseHeight();
  }

  initSelect(fileId: number, comFileArr: Array<HeapSnapshotStruct>) {
    let that = this;
    let input = this.selectEl!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectEl!.innerHTML = '';
    let option = new LitSelectOption();
    option.innerHTML = 'File Name';
    option.setAttribute('disabled', 'disabled');
    this.selectEl?.appendChild(option);
    if (comFileArr[0].name) {
      option.setAttribute('value', comFileArr[0].name);
    }
    this.selectEl!.defaultValue = comFileArr[0].name || '';
    this.selectEl!.placeholder = comFileArr[0].name || '';
    this.selectEl!.dataSource = comFileArr;
    this.selectEl!.querySelectorAll('lit-select-option').forEach((a) => {
      a.addEventListener('onSelected', (e) => {
        this.comparisonTable!.scrollTop = 0;
        this.retainerTableEl!.snapshotDataSource = [];
        for (let f of comFileArr) {
          if (input.value == f.name) {
            that.updateComparisonData(fileId, f.id);
          }
        }
        e.stopPropagation();
      });
    });
  }

  sortComprisonByColumn(column: string, sort: number) {
    switch (sort) {
      case 0:
        if (this.search!.value === '') {
          this.comparisonTableEl!.snapshotDataSource = this.comparisonsData;
        } else {
          this.comparisonTableEl!.snapshotDataSource = this.comparisonFilter;
        }
        break;
      default:
        if (this.search!.value === '') {
          this.leftArray = [...this.comparisonsData];
        } else {
          this.leftArray = [...this.comparisonFilter];
        }
        switch (column) {
          case 'addedCount':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.addedCount - b.addedCount : b.addedCount - a.addedCount;
            });
            break;
          case 'removedCount':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.removedCount - b.removedCount : b.removedCount - a.removedCount;
            });
            break;
          case 'deltaCount':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.deltaCount - b.deltaCount : b.deltaCount - a.deltaCount;
            });
            break;
          case 'objectName':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1
                ? (a.objectName + '').localeCompare(b.objectName + '')
                : (b.objectName + '').localeCompare(a.objectName + '');
            });
            break;
          case 'addedSize':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.addedSize - b.addedSize : b.addedSize - a.addedSize;
            });
            break;
          case 'removedSize':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.removedSize - b.removedSize : b.removedSize - a.removedSize;
            });
            break;
          case 'deltaSize':
            this.comparisonTableEl!.snapshotDataSource = this.leftArray.sort((a, b) => {
              return sort === 1 ? a.deltaSize - b.deltaSize : b.deltaSize - a.deltaSize;
            });
            break;
        }
        break;
    }
  }

  sortRetainerByColumn(column: string, sort: number) {
    switch (sort) {
      case 0:
        this.retainerTableEl!.snapshotDataSource = this.retainsData;
        break;
      default:
        this.rightArray = [...this.retainsData];
        switch (column) {
          case 'distance':
            this.retainerTableEl!.snapshotDataSource = this.rightArray.sort((a, b) => {
              return sort === 1 ? a.distance - b.distance : b.distance - a.distance;
            });
            this.rightArray.forEach((list) => {
              let retainsTable = function () {
                const getList = function (currentList: Array<ConstructorItem>) {
                  currentList.sort((a, b) => {
                    return sort === 1 ? a.distance - b.distance : b.distance - a.distance;
                  });
                  currentList.forEach(function (currentRow) {
                    if (currentRow.children.length > 0) {
                      getList(currentRow.children);
                    }
                  });
                };
                getList(list.children);
              };
              retainsTable();
            });
            this.retainerTableEl!.snapshotDataSource = this.rightArray;
            break;
          case 'shallowSize':
            this.retainerTableEl!.snapshotDataSource = this.rightArray.sort((rightArrA, rightArrB) => {
              return sort === 1
                ? rightArrA.shallowSize - rightArrB.shallowSize
                : rightArrB.shallowSize - rightArrA.shallowSize;
            });
            this.rightArray.forEach((list) => {
              let retainsTable = function () {
                const getList = function (listArr: Array<ConstructorItem>) {
                  listArr.sort((listArrA, listArrB) => {
                    return sort === 1
                      ? listArrA.shallowSize - listArrB.shallowSize
                      : listArrB.shallowSize - listArrA.shallowSize;
                  });
                  listArr.forEach(function (rowEl) {
                    if (rowEl.children.length > 0) {
                      getList(rowEl.children);
                    }
                  });
                };
                getList(list.children);
              };
              retainsTable();
            });
            this.retainerTableEl!.snapshotDataSource = this.rightArray;
            break;
          case 'retainedSize':
            this.retainerTableEl!.snapshotDataSource = this.rightArray.sort((rightArrA, rightArrB) => {
              return sort === 1
                ? rightArrA.retainedSize - rightArrB.retainedSize
                : rightArrB.retainedSize - rightArrA.retainedSize;
            });
            this.rightArray.forEach((list) => {
              let retainsTable = function () {
                const getList = function (listArr: Array<ConstructorItem>) {
                  listArr.sort((listArrA, listArrB) => {
                    return sort === 1
                      ? listArrA.retainedSize - listArrB.retainedSize
                      : listArrB.retainedSize - listArrA.retainedSize;
                  });
                  listArr.forEach(function (row) {
                    if (row.children.length > 0) {
                      getList(row.children);
                    }
                  });
                };
                getList(list.children);
              };
              retainsTable();
            });
            this.retainerTableEl!.snapshotDataSource = this.rightArray;
            break;
          case 'objectName':
            this.retainerTableEl!.snapshotDataSource = this.rightArray.sort((rightArrA, rightArrB) => {
              return sort === 1
                ? (rightArrA.objectName + '').localeCompare(rightArrB.objectName + '')
                : (rightArrB.objectName + '').localeCompare(rightArrA.objectName + '');
            });
            this.rightArray.forEach((list) => {
              let retainsTable = function () {
                const getList = function (listArr: Array<ConstructorItem>) {
                  listArr.sort((listArrA, listArrB) => {
                    return sort === 1
                      ? (listArrA.objectName + '').localeCompare(listArrB.objectName + '')
                      : (listArrB.objectName + '').localeCompare(listArrA.objectName + '');
                  });
                  listArr.forEach(function (currentRow) {
                    if (currentRow.children.length > 0) {
                      getList(currentRow.children);
                    }
                  });
                };
                getList(list.children);
              };
              retainsTable();
            });
            this.retainerTableEl!.snapshotDataSource = this.rightArray;
            break;
        }
        break;
    }
  }

  classFilter() {
    this.search!.addEventListener('keyup', () => {
      this.comparisonFilter = [];
      this.comparisonData.forEach((a: any, key: number) => {
        if (a.objectName.toLowerCase().includes(this.search!.value.toLowerCase())) {
          this.comparisonFilter.push(a);
        } else {
        }
      });
      this.comparisonTableEl!.snapshotDataSource = this.comparisonFilter;
      let summaryTable = this.comparisonTableEl!.shadowRoot?.querySelector('.table') as HTMLDivElement;
      summaryTable.scrollTop = 0;
      this.comparisonTableEl!.reMeauseHeight();
    });
  }

  clear() {
    this.search!.value = '';
    this.rightTheadTable!.removeAttribute('sort');
    this.leftTheadTable!.removeAttribute('sort');
    this.comparisonTable!.scrollTop = 0;
  }

  connectedCallback() {
    super.connectedCallback();
    let filterHeight = 0;
    new ResizeObserver((entries) => {
      let comparisonPanelFilter = this.shadowRoot!.querySelector('#filter') as HTMLElement;
      if (comparisonPanelFilter.clientHeight > 0) filterHeight = comparisonPanelFilter.clientHeight;
      if (this.parentElement!.clientHeight > filterHeight) {
        comparisonPanelFilter.style.display = 'flex';
      } else {
        comparisonPanelFilter.style.display = 'none';
      }
    }).observe(this.parentElement!);
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
        selector{
            display: none;
        }
        tab-pane-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
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
                    <div style="width: 65%">
                        <lit-table id="tb-comparison" style="height: auto" tree>
                            <lit-table-column width="30%" title="#Constructor" data-index="" key="objectName"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="#New" data-index="addedCount" key="addedCount"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="#Deleted" data-index="removedCount" key="removedCount" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="#Delta" data-index="deltaCount" key="deltaCount" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Alloc.Size" data-index="addedSize" key="addedSize" align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Freed Size" data-index="removedSize" key="removedSize" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Size Delta" data-index="deltaSize" key="deltaSize" align="flex-start"  order>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track ></lit-slicer-track>
                    <div style="flex: 1;display: flex; flex-direction: row;">
                        <div style="flex: 1;display: flex; flex-direction: column;">
                            <span slot="head" >Retainers</span>
                            <lit-table id="tb-retainer" style="height: calc(100% - 21px);" tree>
                                <lit-table-column width="30%" title="Object" data-index="" key="objectName"  align="flex-start" order>
                                </lit-table-column>
                                <lit-table-column width="1fr" title="distance" data-index="distance" key="distance"  align="flex-start" order>
                                </lit-table-column>
                                <lit-table-column width="1fr" title="ShallowSize" data-index="shallowSize" key="shallowSize" align="flex-start"  order>
                                </lit-table-column>
                                <lit-table-column width="1fr" title="" data-index="shallowPercent" key="shallowPercent" align="flex-start">
                                </lit-table-column>
                                <lit-table-column width="1fr" title="RetainedSize" data-index="retainedSize" key="retainedSize" align="flex-start" order>
                                </lit-table-column>
                                <lit-table-column width="1fr" title="" data-index="retainedPercent" key="retainedPercent" align="flex-start">
                                </lit-table-column>
                            </div>
                        </div>
                    </lit-table>
                </lit-slicer>
            </selector>
            <lit-progress-bar class="progress"></lit-progress-bar>
            <tab-pane-js-memory-filter id="filter" input inputLeftText first ></tab-pane-js-memory-filter>
            <div class="loading"></div>
        </div>
        `;
  }
}
