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
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { FileSysEvent } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { procedurePool } from '../../../../database/Procedure.js';

@element('tabpane-filesystem-desc-history')
export class TabPaneFileSystemDescHistory extends BaseElement {
  private fsDescHistoryTbl: LitTable | null | undefined;
  private fsDescHistoryTblData: LitTable | null | undefined;
  private fsDescHistoryFilter: TabPaneFilter | null | undefined;
  private fsDescHistoryProgressEL: LitProgressBar | null | undefined;
  private fsDescHistoryLoadingList: number[] = [];
  private fsDescHistoryLoadingPage: any;
  private fsDescHistorySource: Array<FileSysEvent> = [];
  private fsDescHistoryFilterSource: Array<FileSysEvent> = [];
  private fsDescHistorySortKey: string = 'startTs';
  private fsDescHistorySortType: number = 0;
  private filterEventType: string = '0';
  private filterProcess: string = '0';
  private filterPath: string = '0';
  private currentSelection: SelectionParam | undefined | null;
  private eventList: string[] | null | undefined;
  private processList: string[] | null | undefined;
  private pathList: string[] | null | undefined;

  set data(fsDescHistorySelection: SelectionParam | null | undefined) {
    if (fsDescHistorySelection == this.currentSelection) {
      return;
    }
    this.currentSelection = fsDescHistorySelection;
    // @ts-ignore
    this.fsDescHistoryTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.fsDescHistoryTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    this.fsDescHistoryTbl!.recycleDataSource = [];
    this.fsDescHistoryTblData!.recycleDataSource = [];
    if (fsDescHistorySelection) {
      this.fsDescHistoryLoadingList.push(1);
      this.fsDescHistoryProgressEL!.loading = true;
      this.fsDescHistoryLoadingPage.style.visibility = 'visible';
      this.fsDescHistorySource = [];
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryFileSysEvents',
        {
          leftNs: fsDescHistorySelection.leftNs,
          rightNs: fsDescHistorySelection.rightNs,
          typeArr: [0, 1],
          tab: 'history',
        },
        undefined,
        (res: any) => {
          this.fsDescHistorySource = this.fsDescHistorySource.concat(res.data);
          res.data = null;
          if (!res.isSending) {
            this.fsDescHistoryTbl!.recycleDataSource = this.fsDescHistorySource;
            this.fsDescHistoryFilterSource = this.fsDescHistorySource;
            this.setProcessFilter();
            this.fsDescHistoryLoadingList.splice(0, 1);
            if (this.fsDescHistoryLoadingList.length == 0) {
              this.fsDescHistoryProgressEL!.loading = false;
              this.fsDescHistoryLoadingPage.style.visibility = 'hidden';
            }
          }
        }
      );
    }
  }

  setProcessFilter() {
    this.processList = ['All Process'];
    this.pathList = ['All Path'];
    this.fsDescHistorySource.map((historyItem) => {
      if (this.processList!.findIndex((processItem) => processItem === historyItem.process) == -1) {
        this.processList!.push(historyItem.process);
      }
      if (this.pathList!.findIndex((pathItem) => pathItem === historyItem.path) == -1) {
        this.pathList!.push(historyItem.path);
      }
    });
    this.fsDescHistoryFilter!.setSelectList(this.eventList, this.processList, '', '', this.pathList, '');
    this.fsDescHistoryFilter!.firstSelect = '0';
    this.fsDescHistoryFilter!.secondSelect = '0';
    this.fsDescHistoryFilter!.thirdSelect = '0';
    this.filterProcess = '0';
    this.filterPath = '0';
    this.filterEventType = '0';
  }

  filterData() {
    let pfv = parseInt(this.filterProcess);
    let pathIndex = parseInt(this.filterPath);
    this.fsDescHistoryFilterSource = this.fsDescHistorySource.filter((fsHistoryEvent) => {
      let pathFilter = true;
      let eventFilter = true;
      let processFilter = true;
      if (this.filterPath != '0') {
        pathFilter = fsHistoryEvent.path == this.pathList![pathIndex];
      }
      if (this.filterEventType == '1') {
        eventFilter = fsHistoryEvent.type == 0;
      } else if (this.filterEventType == '2') {
        eventFilter = fsHistoryEvent.type == 1;
      }
      if (this.filterProcess != '0') {
        processFilter = fsHistoryEvent.process == this.processList![pfv];
      }
      return pathFilter && eventFilter && processFilter;
    });
    this.fsDescHistoryTblData!.recycleDataSource = [];
    this.sortFsDescHistoryTable(this.fsDescHistorySortKey, this.fsDescHistorySortType);
  }

  initElements(): void {
    this.fsDescHistoryLoadingPage = this.shadowRoot?.querySelector('.filesystem-desc-history-loading');
    this.fsDescHistoryProgressEL = this.shadowRoot?.querySelector(
      '.filesystem-desc-history-progress'
    ) as LitProgressBar;
    this.fsDescHistoryTbl = this.shadowRoot?.querySelector<LitTable>('#tbl-file-system-desc-history');
    this.fsDescHistoryTblData = this.shadowRoot?.querySelector<LitTable>('#tbr-file-system-desc-history');
    this.fsDescHistoryTbl!.addEventListener('row-click', (e) => {
      // @ts-ignore
      let data = e.detail.data as FileSysEvent;
      (data as any).isSelected = true;
      // @ts-ignore
      if ((e.detail as any).callBack) {
        // @ts-ignore
        (e.detail as any).callBack(true);
      }
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryStack',
        { callchainId: data.callchainId },
        undefined,
        (res: any) => {
          this.fsDescHistoryTblData!.recycleDataSource = res;
        }
      );
    });
    this.fsDescHistoryTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.fsDescHistorySortKey = evt.detail.key;
      // @ts-ignore
      this.fsDescHistorySortType = evt.detail.sort;
      // @ts-ignore
      this.sortFsDescHistoryTable(evt.detail.key, evt.detail.sort);
    });
    this.fsDescHistoryFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#filesystem-desc-history-filter');
    this.eventList = ['All FD Event', 'All Open Event', 'All Close Event'];
    this.processList = ['All Process'];
    this.pathList = ['All Path'];
    this.fsDescHistoryFilter!.setSelectList(this.eventList, this.processList, '', '', this.pathList, '');
    this.fsDescHistoryFilter!.firstSelect = '0';
    this.fsDescHistoryFilter!.getFilterData((data: FilterData) => {
      this.filterEventType = data.firstSelect || '0';
      this.filterProcess = data.secondSelect || '0';
      this.filterPath = data.thirdSelect || '0';
      this.filterData();
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.fsDescHistoryTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.fsDescHistoryTbl?.reMeauseHeight();
        // @ts-ignore
        this.fsDescHistoryTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.fsDescHistoryTblData?.reMeauseHeight();
        this.fsDescHistoryLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  sortFsDescHistoryTable(key: string, type: number) {
    if (type == 0) {
      this.fsDescHistoryTbl!.recycleDataSource = this.fsDescHistoryFilterSource;
    } else {
      let arr = Array.from(this.fsDescHistoryFilterSource);
      arr.sort((fsHistoryA, fsHistoryB): number => {
        if (key == 'startTsStr') {
          if (type == 1) {
            return fsHistoryA.startTs - fsHistoryB.startTs;
          } else {
            return fsHistoryB.startTs - fsHistoryA.startTs;
          }
        } else if (key == 'durStr') {
          if (type == 1) {
            return fsHistoryA.dur - fsHistoryB.dur;
          } else {
            return fsHistoryB.dur - fsHistoryA.dur;
          }
        } else if (key == 'process') {
          if (fsHistoryA.process > fsHistoryB.process) {
            return type === 2 ? 1 : -1;
          } else if (fsHistoryA.process == fsHistoryB.process) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'typeStr') {
          if (fsHistoryA.typeStr > fsHistoryB.typeStr) {
            return type === 2 ? 1 : -1;
          } else if (fsHistoryA.typeStr == fsHistoryB.typeStr) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'fd') {
          if (type == 1) {
            return fsHistoryA.fd - fsHistoryB.fd;
          } else {
            return fsHistoryB.fd - fsHistoryA.fd;
          }
        } else {
          return 0;
        }
      });
      this.fsDescHistoryTbl!.recycleDataSource = arr;
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
        .filesystem-desc-history-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .filesystem-desc-history-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            z-index: 99;
            left: 0;
            right: 0;
        }
        tab-pane-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="fs-history-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="tbl-file-system-desc-history" style="height: auto">
                            <lit-table-column class="fs-history-column" width="200px" title="Start" data-index="startTsStr" key="startTsStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-history-column"width="160px" title="Duration" data-index="durStr" key="durStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-history-column"width="200px" title="Process" data-index="process" key="process" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-history-column"width="120px" title="Type" data-index="typeStr" key="typeStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-history-column"width="160px" title="File Descriptor" data-index="fd" key="fd" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-history-column"width="300px" title="Path" data-index="path" key="path" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-history-column"width="600px" title="Backtrace" data-index="backtrace" key="backtrace" align="flex-start" >
                                <template>
                                    <div>
                                        <span class="fs-desc-backtrace-data-span">{{backtrace[0]}}</span>
                                        <span v-if="backtrace.length > 1">⬅</span>
                                        <span v-if="backtrace.length > 1" style="color: #565656"> {{backtrace[1]}}</span>
                                    </div>
                                </template>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track class="fs-desc-history-tracker"></lit-slicer-track>
                    <lit-table id="tbr-file-system-desc-history" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="fs-history-column"width="60px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="fs-history-column"width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="filesystem-desc-history-progress"></lit-progress-bar>
            <tab-pane-filter id="filesystem-desc-history-filter" first second></tab-pane-filter>
            <div class="filesystem-desc-history-loading"></div>
        </div>
`;
  }
}
