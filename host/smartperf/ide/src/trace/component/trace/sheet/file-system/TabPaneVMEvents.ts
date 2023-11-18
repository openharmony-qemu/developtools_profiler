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
  FileSysEvent,
  VirtualMemoryEvent,
  VM_TYPE_MAP,
} from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { getTabVirtualMemoryType } from '../../../../database/SqlLite.js';

@element('tabpane-virtualmemory-event')
export class TabPaneVirtualMemoryEvents extends BaseElement {
  // @ts-ignore
  private defaultNativeTypes = ['All', ...Object.values(VM_TYPE_MAP)];
  private native_type: Array<string> = [...this.defaultNativeTypes];
  private vmEventTbl: LitTable | null | undefined;
  private vmEventTblData: LitTable | null | undefined;
  private vmEventProgressEL: LitProgressBar | null | undefined;
  private loadingList: number[] = [];
  private loadingPage: any;
  private vmEventSource: Array<VirtualMemoryEvent> = [];
  private queryVmEventDataSource: Array<VirtualMemoryEvent> = [];
  private vmEventSortKey: string = 'startTs';
  private vmEventSortType: number = 0;
  private currentSelection: SelectionParam | undefined | null;
  private statsticsSelection: Array<any> = [];

  set data(vmEventSelection: SelectionParam | null | undefined) {
    if (vmEventSelection == this.currentSelection) {
      return;
    }
    this.currentSelection = vmEventSelection;
    this.initFilterTypes(vmEventSelection!).then(() => {
      this.queryData(vmEventSelection!);
    });
    // @ts-ignore
    this.vmEventTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    // @ts-ignore
    this.vmEventTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 31 + 'px';
    this.vmEventTbl!.recycleDataSource = [];
    this.vmEventTblData!.recycleDataSource = [];
  }

  connectedCallback() {
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.vmEventTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.vmEventTbl?.reMeauseHeight();
        // @ts-ignore
        this.vmEventTblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 33 + 'px';
        this.vmEventTblData?.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  initElements(): void {
    this.loadingPage = this.shadowRoot?.querySelector('.loading');
    this.vmEventProgressEL = this.shadowRoot?.querySelector('.vm-event-progress') as LitProgressBar;
    this.vmEventTbl = this.shadowRoot?.querySelector<LitTable>('#vm-event-tbl');
    this.vmEventTblData = this.shadowRoot?.querySelector<LitTable>('#vm-event-tbr');
    this.vmEventTbl!.addEventListener('row-click', (vmEventRowClick) => {
      // @ts-ignore
      let data = vmEventRowClick.detail.data;
      (data as any).isSelected = true;
      // @ts-ignore
      if ((vmEventRowClick.detail as any).callBack) {
        // @ts-ignore
        (vmEventRowClick.detail as any).callBack(true);
      }
      procedurePool.submitWithName(
        'logic0',
        'fileSystem-queryStack',
        { callchainId: data.callchainId },
        undefined,
        (res: any) => {
          this.vmEventTblData!.recycleDataSource = res;
        }
      );
    });
    this.vmEventTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.vmEventSortKey = evt.detail.key;
      // @ts-ignore
      this.vmEventSortType = evt.detail.sort;
      // @ts-ignore
      this.sortVmEventTable(evt.detail.key, evt.detail.sort);
    });
    this.shadowRoot?.querySelector<TabPaneFilter>('#vm-event-filter')!.getFilterData((data: FilterData) => {
      let index = parseInt(data.firstSelect || '0');
      if (index > this.defaultNativeTypes.length - 1) {
        this.filterTypeData(this.statsticsSelection[index - this.defaultNativeTypes.length]);
      } else {
        this.filterTypeData(undefined);
      }
      this.vmEventTbl!.recycleDataSource = this.vmEventSource;
    });
  }

  async initFilterTypes(vmEventParam: SelectionParam) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#vm-event-filter');
    let typeKeys = await getTabVirtualMemoryType(vmEventParam.leftNs, vmEventParam.rightNs);
    this.defaultNativeTypes = ['All'];
    this.statsticsSelection = [];
    typeKeys.forEach((item) => {
      // @ts-ignore
      this.defaultNativeTypes.push(VM_TYPE_MAP[item.type + '']);
    });
    this.native_type = [...this.defaultNativeTypes];
    filter!.setSelectList([...this.defaultNativeTypes], null, 'Operation Type');
    filter!.firstSelect = '0';
  }

  async fromStastics(vmEventParam: SelectionParam | any) {
    if (vmEventParam.fileSystemVMData == undefined) {
      return;
    }
    this.vmEventTblData!.recycleDataSource = [];
    this.vmEventTblData?.clearAllSelection(undefined);
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#vm-event-filter');
    if (this.currentSelection != vmEventParam) {
      await this.initFilterTypes(vmEventParam);
    }
    let typeIndexOf = this.native_type.indexOf(vmEventParam.fileSystemVMData.path.value);
    if (typeIndexOf == -1) {
      this.statsticsSelection.push(vmEventParam.fileSystemVMData.path);
      this.native_type.push(vmEventParam.fileSystemVMData.path.value);
      typeIndexOf = this.native_type.length - 1;
    }
    if (this.currentSelection != vmEventParam) {
      this.currentSelection = vmEventParam;
      filter!.setSelectList(this.native_type, null, 'Operation Type');
      filter!.firstSelect = typeIndexOf + '';
      this.queryData(vmEventParam);
    } else {
      if (typeIndexOf == parseInt(filter!.firstSelect)) {
        return;
      }
      filter!.setSelectList(this.native_type, null, 'Operation Type');
      filter!.firstSelect = typeIndexOf + '';
      this.filterTypeData(vmEventParam?.fileSystemVMData?.path || undefined);
      vmEventParam.fileSystemVMData = undefined;
      this.vmEventTbl!.recycleDataSource = this.vmEventSource;
    }
  }

  queryData(vmEventParam: SelectionParam) {
    this.loadingList.push(1);
    this.vmEventProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    this.vmEventSource = [];
    this.queryVmEventDataSource = [];
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-queryVMEvents',
      {
        leftNs: vmEventParam.leftNs,
        rightNs: vmEventParam.rightNs,
        typeArr: vmEventParam.fileSystemType,
      },
      undefined,
      (res: any) => {
        this.vmEventSource = this.vmEventSource.concat(res.data);
        this.queryVmEventDataSource = this.queryVmEventDataSource.concat(res.data);
        this.filterTypeData(vmEventParam?.fileSystemVMData?.path || undefined);
        vmEventParam.fileSystemVMData = undefined;
        res.data = null;
        if (!res.isSending) {
          this.vmEventTbl!.recycleDataSource = this.vmEventSource;
          this.loadingList.splice(0, 1);
          if (this.loadingList.length == 0) {
            this.vmEventProgressEL!.loading = false;
            this.loadingPage.style.visibility = 'hidden';
          }
        }
      }
    );
  }

  filterTypeData(pathData: any) {
    let filter = this.shadowRoot?.querySelector<TabPaneFilter>('#vm-event-filter');
    let firstSelect = filter!.firstSelect;
    let type = -1;
    let tid = -1;
    let pid = -1;
    if (parseInt(firstSelect) <= this.defaultNativeTypes.length - 1) {
      // @ts-ignore
      let typeEntry = Object.entries(VM_TYPE_MAP).find((entry) => {
        return entry[1] == this.defaultNativeTypes[parseInt(firstSelect)];
      });
      type = typeEntry ? parseInt(typeEntry[0]) : 0;
    } else if (pathData != undefined) {
      type = parseInt(pathData.type || 0);
      tid = pathData.tid || -1;
      pid = pathData.pid || -1;
    } else if (pathData == undefined) {
      return;
    }
    let isTidFilter = false;
    let isPidFilter = false;
    let isTypeFilter = false;
    this.vmEventSource = this.queryVmEventDataSource.filter((item) => {
      if (tid == -1) {
        isTidFilter = true;
      } else {
        isTidFilter = item.tid == tid;
      }
      if (pid == -1) {
        isPidFilter = true;
      } else {
        isPidFilter = item.pid == pid;
      }
      isTypeFilter = type == 0 || item.type == type;
      return isTidFilter && isPidFilter && isTypeFilter;
    });
  }

  sortVmEventTable(key: string, type: number) {
    if (type == 0) {
      this.vmEventTbl!.recycleDataSource = this.vmEventSource;
    } else {
      let arr = Array.from(this.vmEventSource);
      arr.sort((vmEventA, vmEventB): number => {
        if (key == 'startTsStr') {
          if (type == 1) {
            return vmEventA.startTs - vmEventB.startTs;
          } else {
            return vmEventB.startTs - vmEventA.startTs;
          }
        } else if (key == 'durStr') {
          if (type == 1) {
            return vmEventA.dur - vmEventB.dur;
          } else {
            return vmEventB.dur - vmEventA.dur;
          }
        } else if (key == 'thread') {
          if (vmEventA.thread > vmEventB.thread) {
            return type === 2 ? 1 : -1;
          } else if (vmEventA.thread == vmEventB.thread) {
            return 0;
          } else {
            return type === 2 ? -1 : 1;
          }
        } else if (key == 'sizeStr') {
          if (type == 1) {
            return vmEventA.size - vmEventB.size;
          } else {
            return vmEventB.size - vmEventA.size;
          }
        } else {
          return 0;
        }
      });
      this.vmEventTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
    <style>
        .vm-event-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        .vm-event-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            z-index: 99;
            left: 0;
            right: 0;
        }
        #vm-event-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        </style>
        <div class="vm-event-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row;">
                <lit-slicer style="width:100%">
                    <div style="width: 65%">
                        <lit-table id="vm-event-tbl" style="height: auto">
                            <lit-table-column class="vm-event-column" width="1fr" title="Start Time" data-index="startTsStr" key="startTsStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="vm-event-column" width="1fr" title="Duration" data-index="durStr" key="durStr" align="flex-start" order></lit-table-column>
                            <lit-table-column class="vm-event-column" width="1fr" title="Thread" data-index="thread" key="thread" align="flex-start" order></lit-table-column>
                            <lit-table-column class="vm-event-column" width="1fr" title="Operation" data-index="operation" key="operation" align="flex-start" ></lit-table-column>
                            <lit-table-column class="vm-event-column" width="1fr" title="Adress" data-index="address" key="address" align="flex-start" ></lit-table-column>
                            <lit-table-column class="vm-event-column" width="1fr" title="Size" data-index="sizeStr" key="sizeStr" align="flex-start" order></lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track ></lit-slicer-track>
                    <lit-table id="vm-event-tbr" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
                        <lit-table-column class="vm-event-column" width="60px" title="" data-index="type" key="type"  align="flex-start" >
                            <template>
                                <div v-if=" type == -1 ">Thread:</div>
                                <img src="img/library.png" size="20" v-if=" type == 1 ">
                                <img src="img/function.png" size="20" v-if=" type == 0 ">
                            </template>
                        </lit-table-column>
                        <lit-table-column class="vm-event-column" width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress vm-event-progress"></lit-progress-bar>
            <tab-pane-filter id="vm-event-filter" first></tab-pane-filter>
            <div class="loading vm-event-loading"></div>
        </div>
`;
  }
}
