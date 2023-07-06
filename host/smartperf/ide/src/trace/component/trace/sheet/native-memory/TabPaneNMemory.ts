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
import '../../../../../base-ui/slicer/lit-slicer.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { query, queryNativeHookEventTid } from '../../../../database/SqlLite.js';
import { NativeHookStatistics, NativeMemory, NativeHookCallInfo } from '../../../../bean/NativeHook.js';
import '../TabPaneFilter.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { TabPaneNMSampleList } from './TabPaneNMSampleList.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';

@element('tabpane-native-memory')
export class TabPaneNMemory extends BaseElement {
  private defaultNativeTypes = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
  private memoryTbl: LitTable | null | undefined;
  private tblData: LitTable | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private loadingList: number[] = [];
  private loadingPage: any;
  private memorySource: Array<NativeMemory> = [];
  private native_type: Array<string> = [...this.defaultNativeTypes];
  private statsticsSelection: Array<any> = [];
  private queryResult: Array<NativeHookStatistics> = [];
  private filterAllocationType: string = '0';
  private filterNativeType: string = '0';
  private filterResponseType: number = -1;
  private filterResponseSelect: string = '0';
  private currentSelection: SelectionParam | undefined;
  private rowSelectData: any = undefined;
  private sortColumn: string = '';
  private sortType: number = 0;
  private leftNs: number = 0;
  private rightNs: number = 0;
  private responseTypes: any[] = [];

  set data(memoryParam: SelectionParam | any) {
    if (memoryParam == this.currentSelection) {
      return;
    }
    this.currentSelection = memoryParam;
    this.initFilterTypes();
    this.queryData(memoryParam);
  }

  queryData(memoryParam: SelectionParam | any) {
    let types: Array<string> = [];
    if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[0]) != -1) {
      types.push("'AllocEvent'");
      types.push("'MmapEvent'");
    } else {
      if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[1]) != -1) {
        types.push("'AllocEvent'");
      }
      if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[2]) != -1) {
        types.push("'MmapEvent'");
      }
    }
    TabPaneNMSampleList.serSelection(memoryParam);
    // @ts-ignore
    this.memoryTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.tblData?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.tblData?.recycleDataSource = [];
    // @ts-ignore
    this.memoryTbl?.recycleDataSource = [];
    this.leftNs = memoryParam.leftNs;
    this.rightNs = memoryParam.rightNs;
    this.progressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    queryNativeHookEventTid(memoryParam.leftNs, memoryParam.rightNs, types).then((result) => {
      this.queryResult = result;
      this.getDataByNativeMemoryWorker(memoryParam);
    });
  }

  getDataByNativeMemoryWorker(val: SelectionParam | any) {
    let args = new Map<string, any>();
    args.set('data', this.queryResult);
    args.set('filterAllocType', this.filterAllocationType);
    args.set('filterEventType', this.filterNativeType);
    args.set('filterResponseType', this.filterResponseType);
    args.set('leftNs', val.leftNs);
    args.set('rightNs', val.rightNs);
    let selections: Array<any> = [];
    if (this.statsticsSelection.length > 0) {
      this.statsticsSelection.map((memory) => {
        selections.push({
          memoryTap: memory.memoryTap,
          max: memory.max,
        });
      });
    }
    args.set('statisticsSelection', selections);
    args.set('actionType', 'native-memory');
    this.startWorker(args, (results: any[]) => {
      this.tblData!.recycleDataSource = [];
      this.progressEL!.loading = false;
      if (results.length > 0) {
        this.memorySource = results;
        this.sortByColumn(this.sortColumn, this.sortType);
      } else {
        this.memorySource = [];
        this.memoryTbl!.recycleDataSource = [];
      }
    });
  }

  startWorker(args: Map<string, any>, handler: Function) {
    this.loadingList.push(1);
    this.progressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    procedurePool.submitWithName('logic1', 'native-memory-action', args, undefined, (res: any) => {
      handler(res);
      this.loadingList.splice(0, 1);
      if (this.loadingList.length == 0) {
        this.progressEL!.loading = false;
        this.loadingPage.style.visibility = 'hidden';
      }
    });
  }

  fromStastics(val: SelectionParam | any) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    if (this.currentSelection != val) {
      this.initFilterTypes(() => {
        this.currentSelection = val;
        filter!.setSelectList(
          null,
          this.native_type,
          'Allocation Lifespan',
          'Allocation Type',
          this.responseTypes.map((item: any) => {
            return item.value;
          })
        );
        filter!.secondSelect = typeIndexOf + '';
        filter!.thirdSelect = this.filterResponseSelect;
        this.filterNativeType = typeIndexOf + '';
        this.queryData(val);
      });
    }
    let typeIndexOf = this.native_type.indexOf(val.statisticsSelectData.memoryTap);
    if (this.statsticsSelection.indexOf(val.statisticsSelectData) == -1 && typeIndexOf == -1) {
      this.statsticsSelection.push(val.statisticsSelectData);
      this.native_type.push(val.statisticsSelectData.memoryTap);
      typeIndexOf = this.native_type.length - 1;
    } else {
      let index = this.statsticsSelection.findIndex((mt) => mt.memoryTap == val.statisticsSelectData.memoryTap);
      if (index != -1) {
        this.statsticsSelection[index] = val.statisticsSelectData;
      }
    }
    if (this.currentSelection == val) {
      this.tblData!.recycleDataSource = [];
      this.rowSelectData = undefined;
      filter!.setSelectList(
        null,
        this.native_type,
        'Allocation Lifespan',
        'Allocation Type',
        this.responseTypes.map((item: any) => {
          return item.value;
        })
      );
      filter!.secondSelect = typeIndexOf + '';
      filter!.thirdSelect = this.filterResponseSelect;
      this.filterNativeType = typeIndexOf + '';
      //直接将当前数据过滤即可
      this.getDataByNativeMemoryWorker(val);
    }
  }

  initFilterTypes(initCallback?: () => void) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    this.queryResult = [];
    this.native_type = [...this.defaultNativeTypes];
    this.statsticsSelection = [];
    procedurePool.submitWithName('logic1', 'native-memory-get-responseType', {}, undefined, (res: any) => {
      filter!.setSelectList(
        null,
        this.native_type,
        'Allocation Lifespan',
        'Allocation Type',
        res.map((item: any) => {
          return item.value;
        })
      );
      filter!.setFilterModuleSelect('#first-select', 'width', '150px');
      filter!.setFilterModuleSelect('#second-select', 'width', '150px');
      filter!.setFilterModuleSelect('#third-select', 'width', '150px');
      this.responseTypes = res;
      filter!.firstSelect = '0';
      filter!.secondSelect = '0';
      filter!.thirdSelect = '0';
      this.filterResponseSelect = '0';
      this.filterAllocationType = '0';
      this.filterNativeType = '0';
      this.filterResponseType = -1;
      this.rowSelectData = undefined;
      if (initCallback) {
        initCallback();
      }
    });
  }

  initElements(): void {
    this.loadingPage = this.shadowRoot?.querySelector('.loading');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.memoryTbl = this.shadowRoot?.querySelector<LitTable>('#tb-native-memory');
    this.tblData = this.shadowRoot?.querySelector<LitTable>('#tb-native-data');
    this.memoryTbl!.addEventListener('row-click', (e) => {
      // @ts-ignore
      let data = e.detail.data as NativeMemory;
      this.rowSelectData = data;
      this.setRightTableData(data);
      document.dispatchEvent(
        new CustomEvent('triangle-flag', {
          detail: { time: data.startTs, type: 'triangle' },
        })
      );
    });
    this.memoryTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');

    this.shadowRoot?.querySelector<TabPaneFilter>('#filter')!.getFilterData((data: FilterData) => {
      if (data.mark) {
        document.dispatchEvent(
          new CustomEvent('triangle-flag', {
            detail: {
              time: '',
              type: 'square',
              timeCallback: (t: any) => {
                let minTs = 0;
                let minItem: any = undefined;
                let filterTemp = this.memorySource.filter((tempItem) => {
                  if (minTs == 0 || (tempItem.startTs - t != 0 && Math.abs(tempItem.startTs - t) < minTs)) {
                    minTs = Math.abs(tempItem.startTs - t);
                    minItem = tempItem;
                  }
                  return tempItem.startTs == t;
                });
                if (filterTemp.length > 0) {
                  filterTemp[0].isSelected = true;
                } else {
                  if (minItem) {
                    filterTemp.push(minItem);
                    minItem.isSelected = true;
                  }
                }
                if (filterTemp.length > 0) {
                  this.rowSelectData = filterTemp[0];
                  let currentSelection = this.queryResult.filter((item) => {
                    return item.startTs == this.rowSelectData.startTs;
                  });
                  if (currentSelection.length > 0) {
                    currentSelection[0].isSelected = true;
                  }
                  TabPaneNMSampleList.addSampleData(this.rowSelectData);
                  this.memoryTbl!.scrollToData(this.rowSelectData);
                }
              },
            },
          })
        );
      } else {
        this.filterAllocationType = data.firstSelect || '0';
        this.filterNativeType = data.secondSelect || '0';
        this.filterResponseSelect = data.thirdSelect || '0';
        let thirdIndex = parseInt(data.thirdSelect || '0');
        if (this.responseTypes.length > thirdIndex) {
          this.filterResponseType =
            this.responseTypes[thirdIndex].key == undefined ? -1 : this.responseTypes[thirdIndex].key;
        }
        this.getDataByNativeMemoryWorker(this.currentSelection);
      }
    });
    filter!.firstSelect = '1';
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.memoryTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.memoryTbl?.reMeauseHeight();
        // @ts-ignore
        this.tblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.tblData?.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  sortByColumn(nmMemoryColumn: string, nmMemorySort: number) {
    this.sortColumn = nmMemoryColumn;
    this.sortType = nmMemorySort;
    if (nmMemorySort == 0) {
      this.memoryTbl!.recycleDataSource = this.memorySource;
    } else {
      let arr = [...this.memorySource];
      if (nmMemoryColumn == 'index') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          return nmMemorySort == 1
            ? memoryLeftData.index - memoryRightData.index
            : memoryRightData.index - memoryLeftData.index;
        });
      } else if (nmMemoryColumn == 'addr') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          if (nmMemorySort == 1) {
            if (memoryLeftData.addr > memoryRightData.addr) {
              return 1;
            } else if (memoryLeftData.addr == memoryRightData.addr) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (memoryRightData.addr > memoryLeftData.addr) {
              return 1;
            } else if (memoryLeftData.addr == memoryRightData.addr) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (nmMemoryColumn == 'timestamp') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          return nmMemorySort == 1
            ? memoryLeftData.startTs - memoryRightData.startTs
            : memoryRightData.startTs - memoryLeftData.startTs;
        });
      } else if (nmMemoryColumn == 'heapSizeUnit') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          return nmMemorySort == 1
            ? memoryLeftData.heapSize - memoryRightData.heapSize
            : memoryRightData.heapSize - memoryLeftData.heapSize;
        });
      } else if (nmMemoryColumn == 'library') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          if (nmMemorySort == 1) {
            if (memoryLeftData.library > memoryRightData.library) {
              return 1;
            } else if (memoryLeftData.library == memoryRightData.library) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (memoryRightData.library > memoryLeftData.library) {
              return 1;
            } else if (memoryLeftData.library == memoryRightData.library) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (nmMemoryColumn == 'symbol') {
        this.memoryTbl!.recycleDataSource = arr.sort((memoryLeftData, memoryRightData) => {
          if (nmMemorySort == 1) {
            if (memoryLeftData.symbol > memoryRightData.symbol) {
              return 1;
            } else if (memoryLeftData.symbol == memoryRightData.symbol) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (memoryRightData.symbol > memoryLeftData.symbol) {
              return 1;
            } else if (memoryLeftData.symbol == memoryRightData.symbol) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      }
    }
  }

  setRightTableData(nativeMemoryHook: NativeMemory) {
    let args = new Map<string, any>();
    args.set('eventId', nativeMemoryHook.eventId);
    args.set('actionType', 'memory-stack');
    this.startWorker(args, (results: any[]) => {
      let thread = new NativeHookCallInfo();
      thread.threadId = nativeMemoryHook.threadId;
      thread.threadName = nativeMemoryHook.threadName;
      thread.title = `${nativeMemoryHook.threadName ?? ''}【${nativeMemoryHook.threadId}】`;
      thread.type = -1;
      let currentSource = [];
      currentSource.push(thread);
      currentSource.push(...results);
      this.progressEL!.loading = false;
      this.tblData!.dataSource = currentSource;
    });
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .nm-memory-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .nm-memory-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .nm-memory-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="nm-memory-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="tb-native-memory" style="height: auto">
                            <lit-table-column class="nm-memory-column" width="60px" title="#" data-index="index" key="index"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="Address" data-index="addr" key="addr"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="Memory Type" data-index="eventType" key="eventType"  align="flex-start">
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="Timestamp" data-index="timestamp" key="timestamp"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="State" data-index="state" key="state"  align="flex-start">
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="Size" data-index="heapSizeUnit" key="heapSizeUnit"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="20%" title="Responsible Library" data-index="library" key="library"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="20%" title="Responsible Caller" data-index="symbol" key="symbol"  align="flex-start" order>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track ></lit-slicer-track>
                    <lit-table id="tb-native-data" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="nm-memory-column" width="80px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="nm-memory-column" width="1fr" title="" data-index="title" key="title"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress nm-memory-progress"></lit-progress-bar>
            <tab-pane-filter id="filter" class="nm-memory-filter" mark first second></tab-pane-filter>
            <div class="loading nm-memory-loading"></div>
        </div>
        `;
  }
}
