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
import { FileSysEvent } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { procedurePool } from '../../../../database/Procedure.js';

@element('tabpane-filesystem-desc-time-slice')
export class TabPaneFileSystemDescTimeSlice extends BaseElement {
  private fsDescTimeSliceTbl: LitTable | null | undefined;
  private fsDescTimeSliceTblData: LitTable | null | undefined;
  private fsDescTimeSliceProgressEL: LitProgressBar | null | undefined;
  private fsDescTimeSliceLoadingList: number[] = [];
  private fsDescTimeSliceLoadingPage: any;
  private fsDescTimeSliceSource: Array<FileSysEvent> = [];
  private fsDescTimeSliceSortKey: string = 'startTs';
  private fsDescTimeSliceSortType: number = 0;
  private currentSelection: SelectionParam | undefined | null;

  set data(fsDescTimeSliceSelection: SelectionParam | null | undefined) {
    if (fsDescTimeSliceSelection == this.currentSelection) {
      return;
    }
    this.currentSelection = fsDescTimeSliceSelection;
    // @ts-ignore
    this.fsDescTimeSliceTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.fsDescTimeSliceTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    this.fsDescTimeSliceTbl!.recycleDataSource = [];
    this.fsDescTimeSliceTblData!.recycleDataSource = [];
    if (fsDescTimeSliceSelection) {
      this.fsDescTimeSliceLoadingList.push(1);
      this.fsDescTimeSliceProgressEL!.loading = true;
      this.fsDescTimeSliceLoadingPage.style.visibility = 'visible';
      let startNs = (window as any).recordStartNS ?? 0;
      this.fsDescTimeSliceSource = [];
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryFileSysEvents',
        {
          leftNs: startNs + fsDescTimeSliceSelection.leftNs,
          rightNs: startNs + fsDescTimeSliceSelection.rightNs,
          typeArr: [0],
          tab: 'time-slice',
        },
        undefined,
        (res: any) => {
          this.fsDescTimeSliceSource = this.fsDescTimeSliceSource.concat(res.data);
          res.data = null;
          if (!res.isSending) {
            this.fsDescTimeSliceTbl!.recycleDataSource = this.fsDescTimeSliceSource;
            this.fsDescTimeSliceLoadingList.splice(0, 1);
            if (this.fsDescTimeSliceLoadingList.length == 0) {
              this.fsDescTimeSliceProgressEL!.loading = false;
              this.fsDescTimeSliceLoadingPage.style.visibility = 'hidden';
            }
          }
        }
      );
    }
  }

  initElements(): void {
    this.fsDescTimeSliceLoadingPage = this.shadowRoot?.querySelector('.loading');
    this.fsDescTimeSliceProgressEL = this.shadowRoot?.querySelector('.fs-slice-progress') as LitProgressBar;
    this.fsDescTimeSliceTbl = this.shadowRoot?.querySelector<LitTable>('#tbl-filesystem-desc-time-slice');
    this.fsDescTimeSliceTblData = this.shadowRoot?.querySelector<LitTable>('#tbr-filesystem-desc-time-slice');
    this.fsDescTimeSliceTbl!.addEventListener('row-click', (fsTimeSliceRowClickEvent) => {
      // @ts-ignore
      let data = fsTimeSliceRowClickEvent.detail.data as FileSysEvent;
      (data as any).isSelected = true;
      // @ts-ignore
      if ((fsTimeSliceRowClickEvent.detail as any).callBack) {
        // @ts-ignore
        (fsTimeSliceRowClickEvent.detail as any).callBack(true);
      }
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryStack',
        { callchainId: data.callchainId },
        undefined,
        (res: any) => {
          this.fsDescTimeSliceTblData!.recycleDataSource = res;
        }
      );
    });
    this.fsDescTimeSliceTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.fsDescTimeSliceSortKey = evt.detail.key;
      // @ts-ignore
      this.fsDescTimeSliceSortType = evt.detail.sort;
      // @ts-ignore
      this.sortFsDescTimeSliceTable(evt.detail.key, evt.detail.sort);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.fsDescTimeSliceTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.fsDescTimeSliceTbl?.reMeauseHeight();
        // @ts-ignore
        this.fsDescTimeSliceTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 31 + 'px';
        this.fsDescTimeSliceTblData?.reMeauseHeight();
        this.fsDescTimeSliceLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  sortFsDescTimeSliceTable(key: string, type: number) {
    if (type == 0) {
      this.fsDescTimeSliceTbl!.recycleDataSource = this.fsDescTimeSliceSource;
    } else {
      let arr = Array.from(this.fsDescTimeSliceSource);
      arr.sort((fsTimeSliceA, fsTimeSliceB): number => {
        if (key == 'startTsStr') {
          if (type == 1) {
            return fsTimeSliceA.startTs - fsTimeSliceB.startTs;
          } else {
            return fsTimeSliceB.startTs - fsTimeSliceA.startTs;
          }
        } else if (key == 'durStr') {
          if (type == 1) {
            return fsTimeSliceA.dur - fsTimeSliceB.dur;
          } else {
            return fsTimeSliceB.dur - fsTimeSliceA.dur;
          }
        } else if (key == 'process') {
          if (fsTimeSliceA.process > fsTimeSliceB.process) {
            return type === 2 ? 1 : -1;
          } else if (fsTimeSliceA.process == fsTimeSliceB.process) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'fd') {
          if (type == 1) {
            return fsTimeSliceA.fd - fsTimeSliceB.fd;
          } else {
            return fsTimeSliceB.fd - fsTimeSliceA.fd;
          }
        } else {
          return 0;
        }
      });
      this.fsDescTimeSliceTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
    <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 1px 10px;
        }
        .fs-slice-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .fs-slice-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            z-index: 99;
            left: 0;
            right: 0;
        }
        #fs-slice-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="fs-slice-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="tbl-filesystem-desc-time-slice" style="height: auto">
                            <lit-table-column class="fs-slice-column" width="200px" title="Open Time" data-index="startTsStr" key="startTsStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-slice-column" width="200px" title="Open Duration" data-index="durStr" key="surStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-slice-column" width="200px" title="Process" data-index="process" key="process" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-slice-column" width="160px" title="File Descriptor" data-index="fd" key="fd" align="flex-start" order></lit-table-column>
                            <lit-table-column class="fs-slice-column" width="300px" title="Path" data-index="path" key="path" align="flex-start" ></lit-table-column>
                            <lit-table-column class="fs-slice-column" width="600px" title="Backtrace" data-index="backtrace" key="backtrace" align="flex-start" >
                                <template>
                                    <div>
                                        <span class="fs-desc-time-backtrace-data-span">{{backtrace[0]}}</span>
                                        <span v-if="backtrace.length > 1">⬅</span>
                                        <span v-if="backtrace.length > 1"style="color: #565656"> {{backtrace[1]}}</span>
                                    </div>
                                </template>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track class="fs-desc-time-slice"></lit-slicer-track>
                    <lit-table id="tbr-filesystem-desc-time-slice" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="fs-slice-column" width="60px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="fs-slice-column" width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress fs-slice-progress"></lit-progress-bar>
            <tab-pane-filter id="fs-slice-filter"></tab-pane-filter>
            <div class="loading fs-slice-loading"></div>
        </div>
`;
  }
}
