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
import { FileSysEvent } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import '../TabPaneFilter.js';

@element('tabpane-filesystem-event')
export class TabPaneFileSystemEvents extends BaseElement {
  private fsSysEventTbl: LitTable | null | undefined;
  private fsSysEventTblData: LitTable | null | undefined;
  private fsSysEventProgressEL: LitProgressBar | null | undefined;
  private fsSysEventFilter: TabPaneFilter | null | undefined;
  private fsSysEventLoadingList: number[] = [];
  private loadingPage: any;
  private fsSysEventSource: Array<FileSysEvent> = [];
  private fsSysEventFilterSource: Array<FileSysEvent> = [];
  private fsSysEventSortKey: string = 'startTs';
  private fsSysEventSortType: number = 0;
  private currentSelection: SelectionParam | undefined | null;
  private filterEventType: string = '0';
  private filterProcess: string = '0';
  private filterPath: string = '0';
  private eventList: string[] | null | undefined;
  private processList: string[] | null | undefined;
  private pathList: string[] | null | undefined;

  set data(fsSysEventSelection: SelectionParam | null | undefined) {
    if (fsSysEventSelection == this.currentSelection) {
      return;
    }
    this.currentSelection = fsSysEventSelection;
    // @ts-ignore
    this.fsSysEventTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.fsSysEventTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    this.filterEventType = '0';
    this.filterProcess = '0';
    this.queryData(fsSysEventSelection);
  }

  queryData(fsEventParam: SelectionParam | null | undefined) {
    this.fsSysEventTbl!.recycleDataSource = [];
    this.fsSysEventTblData!.recycleDataSource = [];
    if (fsEventParam) {
      this.fsSysEventLoadingList.push(1);
      this.fsSysEventProgressEL!.loading = true;
      this.loadingPage.style.visibility = 'visible';
      this.fsSysEventSource = [];
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryFileSysEvents',
        {
          leftNs: fsEventParam.leftNs,
          rightNs: fsEventParam.rightNs,
          typeArr: fsEventParam.fileSystemType,
          tab: 'events',
        },
        undefined,
        (res: any) => {
          this.fsSysEventSource = this.fsSysEventSource.concat(res.data);
          res.data = null;
          if (!res.isSending) {
            this.setProcessFilter();
            this.filterData();
            this.fsSysEventLoadingList.splice(0, 1);
            if (this.fsSysEventLoadingList.length == 0) {
              this.fsSysEventProgressEL!.loading = false;
              this.loadingPage.style.visibility = 'hidden';
            }
          }
        }
      );
    }
  }

  setProcessFilter() {
    this.processList = ['All Process'];
    this.pathList = ['All Path'];
    this.fsSysEventSource.map((it) => {
      if (this.processList!.findIndex((a) => a === it.process) == -1) {
        this.processList!.push(it.process);
      }
      if (this.pathList!.findIndex((a) => a === it.path) == -1) {
        this.pathList!.push(it.path);
      }
    });
    this.fsSysEventFilter!.setSelectList(this.eventList, this.processList, '', '', this.pathList, '');
    if (this.filterProcess == '-1') {
      this.filterProcess =
        this.processList.indexOf(
          `${this.currentSelection?.fileSystemFsData.name}[${this.currentSelection?.fileSystemFsData.pid}]`
        ) + '';
    }
    this.fsSysEventFilter!.firstSelect = this.filterEventType;
    this.fsSysEventFilter!.secondSelect = this.filterProcess;
    this.fsSysEventFilter!.thirdSelect = this.filterPath;
  }

  filterData() {
    let pfv = parseInt(this.filterProcess);
    let pathIndex = parseInt(this.filterPath);
    let eventType = parseInt(this.filterEventType) - 1;
    this.fsSysEventFilterSource = this.fsSysEventSource.filter((fsEvent) => {
      let pathFilter = true;
      let eventFilter = fsEvent.type == eventType || eventType == -1;
      let processFilter = true;
      if (this.filterPath != '0') {
        pathFilter = fsEvent.path == this.pathList![pathIndex];
      }
      if (this.filterProcess != '0') {
        processFilter = fsEvent.process == this.processList![pfv];
      }
      return pathFilter && eventFilter && processFilter;
    });
    this.fsSysEventTblData!.recycleDataSource = [];
    this.sortFsSysEventTable(this.fsSysEventSortKey, this.fsSysEventSortType);
  }

  initElements(): void {
    this.loadingPage = this.shadowRoot?.querySelector('.loading');
    this.fsSysEventProgressEL = this.shadowRoot?.querySelector('.fs-event-progress') as LitProgressBar;
    this.fsSysEventTbl = this.shadowRoot?.querySelector<LitTable>('#tbl-filesystem-event');
    this.fsSysEventTblData = this.shadowRoot?.querySelector<LitTable>('#tbr-filesystem-event');
    this.fsSysEventTbl!.addEventListener('row-click', (fsEventRowClick) => {
      // @ts-ignore
      let data = fsEventRowClick.detail.data as FileSysEvent;
      (data as any).isSelected = true;
      // @ts-ignore
      if ((fsEventRowClick.detail as any).callBack) {
        // @ts-ignore
        (fsEventRowClick.detail as any).callBack(true);
      }
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryStack',
        { callchainId: data.callchainId },
        undefined,
        (res: any) => {
          this.fsSysEventTblData!.recycleDataSource = res;
        }
      );
    });
    this.fsSysEventTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.fsSysEventSortKey = evt.detail.key;
      // @ts-ignore
      this.fsSysEventSortType = evt.detail.sort;
      // @ts-ignore
      this.sortFsSysEventTable(evt.detail.key, evt.detail.sort);
    });
    this.fsSysEventFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#fs-event-filter');
    this.eventList = ['All Event', 'All Open Event', 'All Close Event', 'All Read Event', 'All Write Event'];
    this.processList = ['All Process'];
    this.pathList = ['All Path'];
    this.fsSysEventFilter!.setSelectList(this.eventList, this.processList, '', '', this.pathList, '');
    this.fsSysEventFilter!.firstSelect = '0';
    this.fsSysEventFilter!.getFilterData((data: FilterData) => {
      this.filterEventType = data.firstSelect || '0';
      this.filterProcess = data.secondSelect || '0';
      this.filterPath = data.thirdSelect || '0';
      this.filterData();
    });
  }

  fromStastics(val: SelectionParam | any) {
    if (val.fileSystemFsData == undefined) {
      return;
    }
    if (val.fileSystemFsData.title == 'All') {
      this.filterEventType = '0';
      this.filterProcess = '0';
    } else if (val.fileSystemFsData.pid == undefined) {
      this.filterEventType = '' + (val.fileSystemFsData.type + 1);
      this.filterProcess = '0';
    } else {
      this.filterEventType = '' + (val.fileSystemFsData.type + 1);
      this.filterProcess = '-1';
    }
    this.filterPath = '0';
    if (this.currentSelection == val) {
      if (this.filterProcess == '-1') {
        this.filterProcess =
          this.processList?.indexOf(`${val.fileSystemFsData.name}[${val.fileSystemFsData.pid}]`) + '';
      }
      this.fsSysEventFilter!.firstSelect = this.filterEventType;
      this.fsSysEventFilter!.secondSelect = this.filterProcess;
      this.fsSysEventFilter!.thirdSelect = this.filterPath;
      this.filterData();
    } else {
      this.currentSelection = val;
      this.queryData(val);
    }
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.fsSysEventTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.fsSysEventTbl?.reMeauseHeight();
        // @ts-ignore
        this.fsSysEventTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.fsSysEventTblData?.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  sortFsSysEventTable(key: string, type: number) {
    if (type == 0) {
      this.fsSysEventTbl!.recycleDataSource = this.fsSysEventFilterSource;
    } else {
      let arr = Array.from(this.fsSysEventFilterSource);
      arr.sort((fsEventA, fsEventB): number => {
        if (key == 'startTsStr') {
          if (type == 1) {
            return fsEventA.startTs - fsEventB.startTs;
          } else {
            return fsEventB.startTs - fsEventA.startTs;
          }
        } else if (key == 'durStr') {
          if (type == 1) {
            return fsEventA.dur - fsEventB.dur;
          } else {
            return fsEventB.dur - fsEventA.dur;
          }
        } else if (key == 'process') {
          if (fsEventA.process > fsEventB.process) {
            return type === 2 ? 1 : -1;
          } else if (fsEventA.process == fsEventB.process) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'thread') {
          if (fsEventA.thread > fsEventB.thread) {
            return type === 2 ? 1 : -1;
          } else if (fsEventA.thread == fsEventB.thread) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'typeStr') {
          if (fsEventA.typeStr > fsEventB.typeStr) {
            return type === 2 ? 1 : -1;
          } else if (fsEventA.typeStr == fsEventB.typeStr) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'fd') {
          if (type == 1) {
            return (fsEventA.fd || 0) - (fsEventB.fd || 0);
          } else {
            return (fsEventB.fd || 0) - (fsEventA.fd || 0);
          }
        } else if (key == 'path') {
          if (fsEventA.path > fsEventB.path) {
            return type === 2 ? 1 : -1;
          } else if (fsEventA.path == fsEventB.path) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else {
          return 0;
        }
      });
      this.fsSysEventTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
    <style>
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        .fs-event-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .fs-event-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            z-index: 99;
            left: 0;
            right: 0;
        }
        #fs-event-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="fs-event-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row;">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="tbl-filesystem-event" style="height: auto">
                            <lit-table-column class="fs-event-column" width="200px" title="Start" data-index="startTsStr" key="startTsStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Duration" data-index="durStr" key="durStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="240px" title="Process" data-index="process" key="process" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="240px" title="Thread" data-index="thread" key="thread" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="120px" title="Type" data-index="typeStr" key="typeStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="120px" title="File Descriptor" data-index="fd" key="fd" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="File Path" data-index="path" key="path" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="First Argument" data-index="firstArg" key="firstArg" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Second Argument" data-index="secondArg" key="secondArg" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Third Argument" data-index="thirdArg" key="thirdArg" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Fourth Argument" data-index="fourthArg" key="fourthArg" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Return" data-index="returnValue" key="returnValue" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="160px" title="Error" data-index="error" key="error" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-event-column" width="600px" title="Backtrace" data-index="backtrace" key="backtrace" align="flex-start" >
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
                    <lit-slicer-track class="fs-evnet-slicer-tracker"></lit-slicer-track>
                    <lit-table id="tbr-filesystem-event" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="fs-event-column" width="60px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="fs-event-column" width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress fs-event-progress"></lit-progress-bar>
            <tab-pane-filter id="fs-event-filter" first second></tab-pane-filter>
            <div class="loading fs-event-loading"></div>
        </div>
`;
  }
}
