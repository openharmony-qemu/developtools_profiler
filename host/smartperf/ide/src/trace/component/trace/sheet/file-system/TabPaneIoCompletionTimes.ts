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
import '../../../../../base-ui/slicer/lit-slicer.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';
import {
  DISKIO_TYPE_MAP,
  FileSysEvent,
  IoCompletionTimes,
  VirtualMemoryEvent,
  VM_TYPE_MAP,
} from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { getTabIoCompletionTimesType, getTabVirtualMemoryType } from '../../../../database/SqlLite.js';

@element('tabpane-io-completiontimes')
export class TabPaneIoCompletionTimes extends BaseElement {
  // @ts-ignore
  private defaultNativeTypes = ['All', ...Object.values(VM_TYPE_MAP)];
  private native_type: Array<string> = [...this.defaultNativeTypes];
  private ioCompletionTimesTbl: LitTable | null | undefined;
  private ioCompletionTimesTblData: LitTable | null | undefined;
  private ioCompletionTimesProgressEL: LitProgressBar | null | undefined;
  private ioCompletionTimesLoadingList: number[] = [];
  private ioCompletionTimesLoadingPage: any;
  private ioCompletionTimesSortKey: string = 'startTs';
  private ioCompletionTimesSortType: number = 0;
  private currentSelection: SelectionParam | undefined | null;
  private ioCompletionTimesSource: Array<IoCompletionTimes> = [];
  private ioCompletionTimesQueryDataSource: Array<IoCompletionTimes> = [];
  private ioCompletionTimesSelection: Array<any> = [];

  set data(ioCompletionTimesSelection: SelectionParam | null | undefined) {
    if (ioCompletionTimesSelection == this.currentSelection) {
      return;
    }
    this.currentSelection = ioCompletionTimesSelection;
    this.initFilterTypes(ioCompletionTimesSelection!).then(() => {
      this.queryData(ioCompletionTimesSelection!);
    });
    // @ts-ignore
    this.ioCompletionTimesTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.ioCompletionTimesTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    this.ioCompletionTimesTbl!.recycleDataSource = [];
    this.ioCompletionTimesTblData!.recycleDataSource = [];
  }

  connectedCallback() {
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.ioCompletionTimesTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.ioCompletionTimesTbl?.reMeauseHeight();
        // @ts-ignore
        this.ioCompletionTimesTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.ioCompletionTimesTblData?.reMeauseHeight();
        this.ioCompletionTimesLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  initElements(): void {
    this.ioCompletionTimesLoadingPage = this.shadowRoot?.querySelector('.io-completiontimes-loading');
    this.ioCompletionTimesProgressEL = this.shadowRoot?.querySelector('.io-completion-progress') as LitProgressBar;
    this.ioCompletionTimesTbl = this.shadowRoot?.querySelector<LitTable>('#tbl-io-completion-times');
    this.ioCompletionTimesTblData = this.shadowRoot?.querySelector<LitTable>('#tbr-io-completion-times');
    this.ioCompletionTimesTbl!.addEventListener('row-click', (e) => {
      // @ts-ignore
      let ioCompletionTimeData = e.detail.data as FileSysEvent;
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryStack',
        { callchainId: ioCompletionTimeData.callchainId },
        undefined,
        (res: any) => {
          this.ioCompletionTimesTblData!.recycleDataSource = res;
        }
      );
    });
    this.ioCompletionTimesTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.ioCompletionTimesSortKey = evt.detail.key;
      // @ts-ignore
      this.ioCompletionTimesSortType = evt.detail.sort;
      // @ts-ignore
      this.sortioCompletionTimesTable(evt.detail.key, evt.detail.sort);
    });
    this.shadowRoot?.querySelector<TabPaneFilter>('#io-completion-filter')!.getFilterData((data: FilterData) => {
      let index = parseInt(data.firstSelect || '0');
      if (index > this.defaultNativeTypes.length - 1) {
        this.filterTypeData(this.ioCompletionTimesSelection[index - this.defaultNativeTypes.length]);
      } else {
        this.filterTypeData(undefined);
      }
      this.ioCompletionTimesTbl!.recycleDataSource = this.ioCompletionTimesSource;
    });
  }

  async initFilterTypes(ioCompletionTimeParam: SelectionParam) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#io-completion-filter');
    let typeKeys = await getTabIoCompletionTimesType(ioCompletionTimeParam.leftNs, ioCompletionTimeParam.rightNs);
    this.defaultNativeTypes = ['All'];
    this.ioCompletionTimesSelection = [];
    typeKeys.forEach((item) => {
      // @ts-ignore
      this.defaultNativeTypes.push(item.tier + '');
    });
    this.native_type = [...this.defaultNativeTypes];
    filter!.setSelectList([...this.defaultNativeTypes], null, 'Tier');
    filter!.firstSelect = '0';
  }

  async fromStastics(ioCompletionTimeParam: SelectionParam | any) {
    if (ioCompletionTimeParam.fileSystemIoData == undefined) {
      return;
    }
    this.ioCompletionTimesTblData!.recycleDataSource = [];
    this.ioCompletionTimesTblData?.clearAllSelection(undefined);
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#io-completion-filter');
    if (this.currentSelection != ioCompletionTimeParam) {
      await this.initFilterTypes(ioCompletionTimeParam);
    }
    let typeIndexOf = this.native_type.indexOf(ioCompletionTimeParam.fileSystemIoData.path.value);
    if (typeIndexOf == -1) {
      this.ioCompletionTimesSelection.push(ioCompletionTimeParam.fileSystemIoData.path);
      this.native_type.push(ioCompletionTimeParam.fileSystemIoData.path.value);
      typeIndexOf = this.native_type.length - 1;
    }
    if (this.currentSelection != ioCompletionTimeParam) {
      this.currentSelection = ioCompletionTimeParam;
      filter!.setSelectList(this.native_type, null, 'Tier');
      filter!.firstSelect = typeIndexOf + '';
      this.queryData(ioCompletionTimeParam);
    } else {
      if (typeIndexOf == parseInt(filter!.firstSelect)) {
        return;
      }
      filter!.setSelectList(this.native_type, null, 'Tier');
      filter!.firstSelect = typeIndexOf + '';
      this.filterTypeData(ioCompletionTimeParam?.fileSystemIoData?.path || undefined);
      ioCompletionTimeParam.fileSystemIoData = undefined;
      this.ioCompletionTimesTbl!.recycleDataSource = this.ioCompletionTimesSource;
    }
  }

  queryData(ioCompletionTimeParam: SelectionParam) {
    this.ioCompletionTimesLoadingList.push(1);
    this.ioCompletionTimesProgressEL!.loading = true;
    this.ioCompletionTimesLoadingPage.style.visibility = 'visible';
    this.ioCompletionTimesSource = [];
    this.ioCompletionTimesQueryDataSource = [];
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-queryIOEvents',
      {
        leftNs: ioCompletionTimeParam.leftNs,
        rightNs: ioCompletionTimeParam.rightNs,
        diskIOipids: ioCompletionTimeParam.diskIOipids,
      },
      undefined,
      (res: any) => {
        this.ioCompletionTimesSource = this.ioCompletionTimesSource.concat(res.data);
        this.ioCompletionTimesQueryDataSource = this.ioCompletionTimesQueryDataSource.concat(res.data);
        this.filterTypeData(ioCompletionTimeParam?.fileSystemIoData?.path || undefined);
        ioCompletionTimeParam.fileSystemIoData = undefined;
        res.data = null;
        if (!res.isSending) {
          this.ioCompletionTimesTbl!.recycleDataSource = this.ioCompletionTimesSource;
          this.ioCompletionTimesLoadingList.splice(0, 1);
          if (this.ioCompletionTimesLoadingList.length == 0) {
            this.ioCompletionTimesProgressEL!.loading = false;
            this.ioCompletionTimesLoadingPage.style.visibility = 'hidden';
          }
        }
      }
    );
  }

  filterTypeData(pathTypeData: any) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#io-completion-filter');
    let firstSelect = filter!.firstSelect;
    let tier = -1;
    let path = '';
    let pid = -1;
    if (parseInt(firstSelect) <= this.defaultNativeTypes.length - 1) {
      let index = parseInt(firstSelect);
      tier = index == 0 ? -1 : parseInt(this.defaultNativeTypes[index]);
    } else if (pathTypeData != undefined) {
      tier = parseInt(pathTypeData.tier);
      path = pathTypeData.path || '';
      pid = pathTypeData.pid || -1;
    } else if (pathTypeData == undefined) {
      return;
    }
    let isTierFilter = false;
    let isPidFilter = false;
    let isPathFilter = false;
    this.ioCompletionTimesSource = this.ioCompletionTimesQueryDataSource.filter((ioCompletionTimesQueryData) => {
      if (tier == -1) {
        isTierFilter = true;
      } else {
        isTierFilter = ioCompletionTimesQueryData.tier == tier;
      }
      if (pid == -1) {
        isPidFilter = true;
      } else {
        isPidFilter = ioCompletionTimesQueryData.pid == pid;
      }
      isPathFilter = path == '' || ioCompletionTimesQueryData.path == path;
      return isTierFilter && isPidFilter && isPathFilter;
    });
  }

  sortioCompletionTimesTable(ioCompletionTimesKey: string, type: number) {
    if (type == 0) {
      this.ioCompletionTimesTbl!.recycleDataSource = this.ioCompletionTimesSource;
    } else {
      let arr = Array.from(this.ioCompletionTimesSource);
      arr.sort((ioCompletionTimesA, ioCompletionTimesB): number => {
        if (ioCompletionTimesKey == 'startTsStr') {
          if (type == 1) {
            return ioCompletionTimesA.startTs - ioCompletionTimesB.startTs;
          } else {
            return ioCompletionTimesB.startTs - ioCompletionTimesA.startTs;
          }
        } else if (ioCompletionTimesKey == 'durStr') {
          if (type == 1) {
            return ioCompletionTimesA.dur - ioCompletionTimesB.dur;
          } else {
            return ioCompletionTimesB.dur - ioCompletionTimesA.dur;
          }
        } else if (ioCompletionTimesKey == 'process') {
          if (ioCompletionTimesA.process > ioCompletionTimesB.process) {
            return type === 2 ? 1 : -1;
          } else if (ioCompletionTimesA.process == ioCompletionTimesB.process) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (ioCompletionTimesKey == 'durPer4kStr') {
          if (type == 1) {
            return ioCompletionTimesA.durPer4k - ioCompletionTimesB.durPer4k;
          } else {
            return ioCompletionTimesB.durPer4k - ioCompletionTimesA.durPer4k;
          }
        } else if (ioCompletionTimesKey == 'thread') {
          if (ioCompletionTimesA.thread > ioCompletionTimesB.thread) {
            return type === 2 ? 1 : -1;
          } else if (ioCompletionTimesA.thread == ioCompletionTimesB.thread) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (ioCompletionTimesKey == 'operation') {
          if (ioCompletionTimesA.operation > ioCompletionTimesB.operation) {
            return type === 2 ? 1 : -1;
          } else if (ioCompletionTimesA.operation == ioCompletionTimesB.operation) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (ioCompletionTimesKey == 'sizeStr') {
          if (type == 1) {
            return ioCompletionTimesA.size - ioCompletionTimesB.size;
          } else {
            return ioCompletionTimesB.size - ioCompletionTimesA.size;
          }
        } else if (ioCompletionTimesKey == 'tier') {
          if (type == 1) {
            return ioCompletionTimesA.tier - ioCompletionTimesB.tier;
          } else {
            return ioCompletionTimesB.tier - ioCompletionTimesA.tier;
          }
        } else {
          return 0;
        }
      });
      this.ioCompletionTimesTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
    <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .io-completiontimes-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .io-completion-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            z-index: 99;
            left: 0;
            right: 0;
        }
        #io-completion-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="io-completion-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row;">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="tbl-io-completion-times" style="height: auto">
                            <lit-table-column class="io-completion-column" width="200px" title="Start" data-index="startTsStr" key="startTsStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="260px" title="Total Latency" data-index="durStr" key="durStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Process" data-index="process" key="process" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Latency per 4KB" data-index="durPer4kStr" key="durPer4kStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Thread" data-index="thread" key="thread" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Operation" data-index="operation" key="operation" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Bytes" data-index="sizeStr" key="sizeStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="280px" title="Path" data-index="path" key="path" align="flex-start" ></lit-table-column>
                            <lit-table-column class="io-completion-column" width="200px" title="Block number" data-index="blockNumber" key="blockNumber" align="flex-start" ></lit-table-column>
                            <lit-table-column class="io-completion-column" width="240px" title="Tier" data-index="tier" key="tier" align="flex-start" order></lit-table-column>
                            <lit-table-column class="io-completion-column" width="600px" title="Backtrace" data-index="backtrace" key="backtrace" align="flex-start" >
                                <template>
                                    <div>
                                        <span>{{backtrace[0]}}</span>
                                        <span v-if="backtrace.length > 1">⬅</span>
                                        <span v-if="backtrace.length > 1"style="color: #565656"> {{backtrace[1]}}</span>
                                    </div>
                                </template>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track class="io-completion-slicer-tracker"></lit-slicer-track>
                    <lit-table id="tbr-io-completion-times" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="io-completion-column" width="60px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="io-completion-column" width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress io-completion-progress"></lit-progress-bar>
            <tab-pane-filter id="io-completion-filter" first></tab-pane-filter>
            <div class="io-completiontimes-loading"></div>
        </div>
`;
  }
}
