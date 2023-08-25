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
import { NativeMemory, NativeHookCallInfo } from '../../../../bean/NativeHook.js';
import '../TabPaneFilter.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { TabPaneNMSampleList } from './TabPaneNMSampleList.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';
import {
  formatRealDateMs,
  getByteWithUnit,
  getTimeString
} from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { SpNativeMemoryChart } from '../../../chart/SpNativeMemoryChart.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-native-memory')
export class TabPaneNMemory extends BaseElement {
  private defaultNativeTypes = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
  private memoryTbl: LitTable | null | undefined;
  private filter: TabPaneFilter | null | undefined;
  private tblData: LitTable | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private nmMemoryLoadingList: number[] = [];
  private loadingPage: any;
  private memorySource: Array<NativeMemory> = [];
  private native_type: Array<string> = [...this.defaultNativeTypes];
  private statsticsSelection: Array<any> = [];
  private filterAllocationType: string = '0';
  private filterNativeType: string = '0';
  private filterResponseType: number = -1;
  private filterResponseSelect: string = '0';
  private currentSelection: SelectionParam | undefined;
  private rowSelectData: any = undefined;
  private sortColumn: string = '';
  private sortType: number = 0;
  private responseTypes: any[] = [];
  private eventTypes: string[] = [];

  set data(memoryParam: SelectionParam | any) {
    if (memoryParam == this.currentSelection) {
      return;
    }
    this.currentSelection = memoryParam;
    this.queryData(memoryParam);
  }

  queryData(memoryParam: SelectionParam | any) {
    this.eventTypes = [];
    if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[0]) != -1) {
      this.eventTypes.push("'AllocEvent'");
      this.eventTypes.push("'MmapEvent'");
    } else {
      if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[1]) != -1) {
        this.eventTypes.push("'AllocEvent'");
      }
      if (memoryParam.nativeMemory.indexOf(this.defaultNativeTypes[2]) != -1) {
        this.eventTypes.push("'MmapEvent'");
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
    this.resetFilter();
    this.getDataByNativeMemoryWorker(memoryParam, true);
  }

  getDataByNativeMemoryWorker(val: SelectionParam | any, refresh = false) {
    let args = new Map<string, any>();
    args.set('filterAllocType', this.filterAllocationType);
    args.set('filterEventType', this.filterNativeType);
    args.set('filterResponseType', this.filterResponseType);
    args.set('leftNs', val.leftNs);
    args.set('rightNs', val.rightNs);
    args.set('types', this.eventTypes);
    args.set('refresh', refresh);
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
    args.set('sortColumn', this.sortColumn);
    args.set('sortType', this.sortType);
    this.memorySource = [];
    if (this.memoryTbl!.recycleDs.length > 1_0000) {
      this.memoryTbl!.recycleDataSource = [];
    }
    this.startNmMemoryWorker('native-memory-queryNativeHookEvent', args, (results: any[]) => {
      this.tblData!.recycleDataSource = [];
      if (refresh) {
        this.setNmMemoryLoading(true);
        this.initFilterTypes(() => this.setNmMemoryLoading(false));
      }
      if (results.length > 0) {
        this.memorySource = results;
        this.memoryTbl!.recycleDataSource = this.memorySource;
      } else {
        this.memorySource = [];
        this.memoryTbl!.recycleDataSource = [];
      }
    });
  }

  startNmMemoryWorker(type: string, args: any, handler: Function) {
    this.setNmMemoryLoading(true);
    procedurePool.submitWithName('logic1', type, args, undefined, (res: any) => {
      if (Array.isArray(res) || (res.tag === 'end' && res.index === 0)) {
        handler(res.data ? res.data : res);
        this.setNmMemoryLoading(false);
      } else {
        this.memorySource.push(res.data);
        if (res.tag == 'end') {
          handler(this.memorySource);
          this.setNmMemoryLoading(false);
        }
      }
    });
  }

  setNmMemoryLoading(loading: boolean) {
    if (loading) {
      this.nmMemoryLoadingList.push(1);
      this.progressEL!.loading = true;
      this.loadingPage.style.visibility = 'visible';
    } else {
      this.nmMemoryLoadingList.splice(0, 1);
      if (this.nmMemoryLoadingList.length == 0) {
        this.progressEL!.loading = false;
        this.loadingPage.style.visibility = 'hidden';
      }
    }
  }

  fromStastics(val: SelectionParam | any) {
    let nmFilterEl = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    if (this.currentSelection != val) {
      this.initFilterTypes(() => {
        this.currentSelection = val;
        this.filterSetSelectList(nmFilterEl!, typeIndexOf);
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
      this.filterSetSelectList(nmFilterEl!, typeIndexOf);
      this.filterNativeType = typeIndexOf + '';
      //直接将当前数据过滤即可
      this.getDataByNativeMemoryWorker(val);
    }
  }

  private filterSetSelectList(nmFilterEl: TabPaneFilter, typeIndexOf: number) {
    nmFilterEl!.setSelectList(
      null,
      this.native_type,
      'Allocation Lifespan',
      'Allocation Type',
      this.responseTypes.map((item: any) => {
        return item.value;
      })
    );
    nmFilterEl!.secondSelect = typeIndexOf + '';
    nmFilterEl!.thirdSelect = this.filterResponseSelect;
  }

  initFilterTypes(initCallback?: () => void) {
    this.native_type = [...this.defaultNativeTypes];
    this.statsticsSelection = [];
    procedurePool.submitWithName('logic1', 'native-memory-get-responseType', {}, undefined, (res: any) => {
      this.filter!.setSelectList(
        null,
        this.native_type,
        'Allocation Lifespan',
        'Allocation Type',
        res.map((item: any) => {
          return item.value;
        })
      );
      this.filter!.setFilterModuleSelect('#first-select', 'width', '150px');
      this.filter!.setFilterModuleSelect('#second-select', 'width', '150px');
      this.filter!.setFilterModuleSelect('#third-select', 'width', '150px');
      this.responseTypes = res;
      this.resetFilter();
      this.rowSelectData = undefined;
      if (initCallback) {
        initCallback();
      }
    });
  }

  resetFilter() {
    this.filter!.firstSelect = '0';
    this.filter!.secondSelect = '0';
    this.filter!.thirdSelect = '0';
    this.filterResponseSelect = '0';
    this.filterAllocationType = '0';
    this.filterNativeType = '0';
    this.filterResponseType = -1;
  }

  initElements(): void {
    this.loadingPage = this.shadowRoot?.querySelector('.loading');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.memoryTbl = this.shadowRoot?.querySelector<LitTable>('#tb-native-memory');
    this.tblData = this.shadowRoot?.querySelector<LitTable>('#tb-native-data');
    this.filter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
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
    this.memoryTbl!.addEventListener('column-click', (evt: any) => {
      this.sortColumn = evt.detail.key;
      this.sortType = evt.detail.sort;
      this.getDataByNativeMemoryWorker(this.currentSelection);
    });
    this.memoryTbl!.itemTextHandleMap.set('startTs', (startTs) => {
      return SpNativeMemoryChart.REAL_TIME_DIF === 0 ? getTimeString(startTs) : formatRealDateMs(startTs + SpNativeMemoryChart.REAL_TIME_DIF);
    });
    this.memoryTbl!.itemTextHandleMap.set('endTs', (endTs) => {
      return (endTs > this.currentSelection!.leftNs &&
        endTs <= this.currentSelection!.rightNs &&
        endTs !== 0 &&
        endTs !== null
      ) ? 'Freed' : 'Existing';
    });
    this.memoryTbl!.itemTextHandleMap.set('heapSize', (heapSize) => {
      return getByteWithUnit(heapSize);
    })
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
                  let args = new Map<string, any>();
                  args.set('startTs', this.rowSelectData.startTs);
                  args.set('actionType', 'native-memory-state-change');
                  this.startNmMemoryWorker('native-memory-action', args, (results: any[]) => {});
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
    this.filter!.firstSelect = '1';
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

  setRightTableData(nativeMemoryHook: NativeMemory) {
    let args = new Map<string, any>();
    args.set('eventId', nativeMemoryHook.eventId);
    args.set('actionType', 'memory-stack');
    this.startNmMemoryWorker('native-memory-action', args, (results: any[]) => {
      let thread = new NativeHookCallInfo();
      thread.threadId = nativeMemoryHook.threadId;
      thread.threadName = Utils.THREAD_MAP.get(thread.threadId) || 'Thread';
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
        .nm-memory-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
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
                            <lit-table-column class="nm-memory-column" width="1fr" title="Timestamp" data-index="startTs" key="startTs"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="State" data-index="endTs" key="endTs"  align="flex-start">
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="1fr" title="Size" data-index="heapSize" key="heapSize"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="20%" title="Responsible Library" data-index="library" key="library"  align="flex-start">
                            </lit-table-column>
                            <lit-table-column class="nm-memory-column" width="20%" title="Responsible Caller" data-index="symbol" key="symbol"  align="flex-start">
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track></lit-slicer-track>
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
