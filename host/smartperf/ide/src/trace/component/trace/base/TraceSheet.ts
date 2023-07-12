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

import { BaseElement, element } from '../../../../base-ui/BaseElement.js';
import { LitTabs } from '../../../../base-ui/tabs/lit-tabs.js';
import { LitTabpane } from '../../../../base-ui/tabs/lit-tabpane.js';
import { BoxJumpParam, SelectionParam } from '../../../bean/BoxSelection.js';
import { TabPaneCurrentSelection } from '../sheet/TabPaneCurrentSelection.js';
import { TabPaneFlag } from '../timer-shaft/TabPaneFlag.js';
import { Flag } from '../timer-shaft/Flag.js';
import { WakeupBean } from '../../../bean/WakeupBean.js';
import { LitIcon } from '../../../../base-ui/icon/LitIcon.js';
import { tabConfig } from './TraceSheetConfig.js';
import { TabPaneBoxChild } from '../sheet/cpu/TabPaneBoxChild.js';
import { CpuStruct } from '../../../database/ui-worker/ProcedureWorkerCPU.js';
import { CpuFreqStruct } from '../../../database/ui-worker/ProcedureWorkerFreq.js';
import { CpuFreqLimitsStruct } from '../../../database/ui-worker/ProcedureWorkerCpuFreqLimits.js';
import { ThreadStruct } from '../../../database/ui-worker/ProcedureWorkerThread.js';
import { FuncStruct } from '../../../database/ui-worker/ProcedureWorkerFunc.js';
import { ProcessMemStruct } from '../../../database/ui-worker/ProcedureWorkerMem.js';
import { CpuStateStruct } from '../../../database/ui-worker/ProcedureWorkerCpuState.js';
import { ClockStruct } from '../../../database/ui-worker/ProcedureWorkerClock.js';
import { IrqStruct } from '../../../database/ui-worker/ProcedureWorkerIrq.js';
import { JankStruct } from '../../../database/ui-worker/ProcedureWorkerJank.js';
import { HeapStruct } from '../../../database/ui-worker/ProcedureWorkerHeap.js';
import { LitTable } from '../../../../base-ui/table/lit-table.js';
import { threadPool } from '../../../database/SqlLite.js';
import { HeapSnapshotStruct } from '../../../database/ui-worker/ProcedureWorkerHeapSnapshot.js';
import { TabPaneComparison } from '../sheet/snapshot/TabPaneComparison.js';
import { TabPaneSummary } from '../sheet/snapshot/TabPaneSummary.js';
import { TabPaneNMStatisticAnalysis } from '../sheet/native-memory/TabPaneNMStatisticAnalysis.js';

@element('trace-sheet')
export class TraceSheet extends BaseElement {
  private litTabs: LitTabs | undefined | null;
  private importDiv: HTMLDivElement | undefined | null;
  private nav: HTMLDivElement | undefined | null;
  private selection: SelectionParam | undefined | null;
  private currentPaneID: string = 'current-selection';
  private fragment: DocumentFragment | undefined;

  static get observedAttributes() {
    return ['mode'];
  }

  buildTabs(litTabs: LitTabs | undefined | null) {
    this.fragment = document.createDocumentFragment();
    Reflect.ownKeys(tabConfig).forEach((key, index) => {
      let pane = new LitTabpane();
      pane.id = key.toString();
      pane.className = 'tabHeight';
      pane.tab = tabConfig[key].title;
      pane.hidden = true;
      pane.key = `${tabConfig[key].key || index}`;
      let cls = tabConfig[key].type;
      let node = new cls();
      pane.append(node);
      this.fragment?.appendChild(pane);
    });
    litTabs!.appendChild(this.fragment);
  }

  displayTab<T>(...names: string[]): T {
    this.setAttribute('mode', 'max');
    this.showUploadSoBt(null);
    this.shadowRoot
      ?.querySelectorAll<LitTabpane>('#tabs lit-tabpane')
      .forEach((it) => (it.hidden = !names.some((k) => k === it.id)));
    let litTabpane = this.shadowRoot?.querySelector<LitTabpane>(`#tabs lit-tabpane[id='${names[0]}']`);
    this.shadowRoot?.querySelector<LitTabs>('#tabs')?.activePane(litTabpane!.key);
    return litTabpane!.children.item(0) as unknown as T;
  }

  getComponentByID<T>(id: string): T {
    return this.getPaneByID(id)!.children.item(0) as unknown as T;
  }

  getPaneByID(id: string): LitTabpane {
    return this.shadowRoot!.querySelector(`#${id}`)!;
  }

  initElements(): void {
    this.litTabs = this.shadowRoot?.querySelector('#tabs');
    this.importDiv = this.shadowRoot?.querySelector('#import_div');
    this.buildTabs(this.litTabs);
    let minBtn = this.shadowRoot?.querySelector('#min-btn');
    minBtn?.addEventListener('click', () => {});
    this.litTabs!.onTabClick = (e: any) => this.loadTabPaneData(e.detail.key);
    this.litTabs!.addEventListener('close-handler', () => {
      Reflect.ownKeys(tabConfig)
        .reverse()
        .forEach((id) => {
          let element = tabConfig[id];
          let pane = this.shadowRoot!.querySelector<LitTabpane>(`#${id as string}`);
          if (element.require) {
            pane!.hidden = !element.require(this.selection);
          } else {
            pane!.hidden = true;
          }
        });
      this.litTabs?.activeByKey(`${this.getPaneByID(this.currentPaneID).key}`);
    });
    this.getComponentByID<any>('box-spt')!.addEventListener('row-click', this.rowClickHandler.bind(this));
    this.getComponentByID<any>('box-pts')!.addEventListener('row-click', this.rowClickHandler.bind(this));
    this.getComponentByID<any>('box-native-statstics')!.addEventListener('row-click', (e: any) => {
      this.selection!.statisticsSelectData = e.detail;
      let pane = this.getPaneByID('box-native-memory');
      this.litTabs?.activeByKey(pane.key);
      (pane.children.item(0) as any)!.fromStastics(this.selection);
    });
    this.getComponentByID<any>('box-virtual-memory-statistics')!.addEventListener('row-click', (e: any) => {
      this.selection!.fileSystemVMData = { path: e.detail.path };
      let pane = this.getPaneByID('box-vm-events');
      this.litTabs?.activeByKey(pane.key);
      if (e.detail.path) {
        (pane.children.item(0) as any)!.fromStastics(this.selection);
      }
    });
    this.getComponentByID<any>('box-io-tier-statistics')!.addEventListener('row-click', (e: any) => {
      this.selection!.fileSystemIoData = { path: e.detail.path };
      let pane = this.getPaneByID('box-io-events');
      this.litTabs?.activeByKey(pane.key);
      if (e.detail.path) {
        (pane.children.item(0) as any)!.fromStastics(this.selection);
      }
    });
    this.getComponentByID<any>('box-file-system-statistics')!.addEventListener('row-click', (e: any) => {
      this.selection!.fileSystemFsData = e.detail.data;
      let pane = this.getPaneByID('box-file-system-event');
      this.litTabs?.activeByKey(pane.key);
      if (e.detail.data) {
        (pane.children.item(0) as any)!.fromStastics(this.selection);
      }
    });
  }

  connectedCallback() {
    this.nav = this.shadowRoot?.querySelector('#tabs')?.shadowRoot?.querySelector('.tab-nav-container');
    let tabs: HTMLDivElement | undefined | null = this.shadowRoot?.querySelector('#tabs');
    let navRoot: HTMLDivElement | null | undefined = this.shadowRoot
      ?.querySelector('#tabs')
      ?.shadowRoot?.querySelector('.nav-root');
    let search: HTMLDivElement | undefined | null = document
      .querySelector('body > sp-application')
      ?.shadowRoot?.querySelector('div > div.search-container');
    let timerShaft: HTMLDivElement | undefined | null = this.parentElement?.querySelector('.timer-shaft');
    let spacer: HTMLDivElement | undefined | null = this.parentElement?.querySelector('.spacer');
    let rowsPaneEL: HTMLDivElement | undefined | null = this.parentElement?.querySelector('.rows-pane');

    let borderTop: number = 1;
    let initialHeight = { tabs: `calc(30vh + 39px)`, node: '30vh' };
    this.nav!.onmousedown = (event) => {
      (window as any).isSheetMove = true;
      let litTabpane: NodeListOf<HTMLDivElement> | undefined | null =
        this.shadowRoot?.querySelectorAll('#tabs > lit-tabpane');
      let preY = event.pageY;
      let preHeight = tabs!.offsetHeight;
      document.onmousemove = function (event) {
        let moveY: number = preHeight - (event.pageY - preY);
        litTabpane!.forEach((node: HTMLDivElement) => {
          if (spacer!.offsetHeight > rowsPaneEL!.offsetHeight) {
            tabs!.style.height = moveY + 'px';
            node!.style.height = moveY - navRoot!.offsetHeight + 'px';
            tabsPackUp!.name = 'down';
          } else if (
            navRoot!.offsetHeight <= moveY &&
            search!.offsetHeight + timerShaft!.offsetHeight + borderTop + spacer!.offsetHeight <=
              window.innerHeight - moveY
          ) {
            tabs!.style.height = moveY + 'px';
            node!.style.height = moveY - navRoot!.offsetHeight + 'px';
            tabsPackUp!.name = 'down';
          } else if (navRoot!.offsetHeight >= moveY) {
            tabs!.style.height = navRoot!.offsetHeight + 'px';
            node!.style.height = '0px';
            tabsPackUp!.name = 'up';
          } else if (
            search!.offsetHeight + timerShaft!.offsetHeight + borderTop + spacer!.offsetHeight >=
            window.innerHeight - moveY
          ) {
            tabs!.style.height =
              window.innerHeight -
              search!.offsetHeight -
              timerShaft!.offsetHeight -
              borderTop -
              spacer!.offsetHeight +
              'px';
            node!.style.height =
              window.innerHeight -
              search!.offsetHeight -
              timerShaft!.offsetHeight -
              navRoot!.offsetHeight -
              borderTop -
              spacer!.offsetHeight +
              'px';
            tabsPackUp!.name = 'down';
          }
        });
      };
      document.onmouseup = function () {
        setTimeout(() => {
          (window as any).isSheetMove = false;
        }, 100);
        litTabpane!.forEach((node: HTMLDivElement) => {
          if (node!.style.height !== '0px' && tabs!.style.height != '') {
            initialHeight.node = node!.style.height;
            initialHeight.tabs = tabs!.style.height;
          }
        });
        this.onmousemove = null;
        this.onmouseup = null;
      };
    };
    let tabsOpenUp: LitIcon | undefined | null = this.shadowRoot?.querySelector<LitIcon>('#max-btn');
    let tabsPackUp: LitIcon | undefined | null = this.shadowRoot?.querySelector<LitIcon>('#min-btn');
    let importFileBt: HTMLInputElement | undefined | null =
      this.shadowRoot?.querySelector<HTMLInputElement>('#import-file');
    let exportDataBt: LitIcon | undefined | null = this.shadowRoot?.querySelector<LitIcon>('#export-btn');
    tabsOpenUp!.onclick = () => {
      tabs!.style.height = window.innerHeight - search!.offsetHeight - timerShaft!.offsetHeight - borderTop + 'px';
      let litTabpane: NodeListOf<HTMLDivElement> | undefined | null =
        this.shadowRoot?.querySelectorAll('#tabs > lit-tabpane');
      litTabpane!.forEach((node: HTMLDivElement) => {
        node!.style.height =
          window.innerHeight -
          search!.offsetHeight -
          timerShaft!.offsetHeight -
          navRoot!.offsetHeight -
          borderTop +
          'px';
        initialHeight.node = node!.style.height;
      });
      initialHeight.tabs = tabs!.style.height;
      tabsPackUp!.name = 'down';
    };
    tabsPackUp!.onclick = () => {
      let litTabpane: NodeListOf<HTMLDivElement> | undefined | null =
        this.shadowRoot?.querySelectorAll('#tabs > lit-tabpane');
      if (tabsPackUp!.name == 'down') {
        tabs!.style.height = navRoot!.offsetHeight + 'px';
        litTabpane!.forEach((node: HTMLDivElement) => (node!.style.height = '0px'));
        tabsPackUp!.name = 'up';
        (window as any).isPackUpTable = true;
      } else {
        tabsPackUp!.name = 'down';
        tabs!.style.height = initialHeight.tabs;
        litTabpane!.forEach((node: HTMLDivElement) => (node!.style.height = initialHeight.node));
      }
    };
    importFileBt!.addEventListener('change', (event) => {
      let files = importFileBt?.files;
      if (files) {
        let fileList: Array<File> = [];
        for (let file of files) {
          if (file.name.endsWith('.so')) {
            fileList.push(file);
          }
        }
        if (fileList.length > 0) {
          window.publish(window.SmartEvent.UI.Loading, true);
          threadPool.submit(
            'upload-so',
            '',
            fileList,
            (res: any) => {
              window.publish(window.SmartEvent.UI.UploadSOFile, {});
            },
            'upload-so'
          );
        }
      }
      importFileBt!.files = null;
      importFileBt!.value = '';
    });
    exportDataBt!.onclick = () => {
      let currentTab = this.getTabpaneByKey(this.litTabs?.activekey!);
      if (currentTab) {
        let tables = Array.from(
          (currentTab.firstChild as BaseElement).shadowRoot?.querySelectorAll<LitTable>('lit-table') || []
        );
        for (let table of tables) {
          if (!table.hasAttribute('hideDownload')) {
            table.exportData();
          }
        }
      }
    };
  }

  getTabpaneByKey(key: string): LitTabpane | undefined {
    let tabs = Array.from(this.shadowRoot?.querySelectorAll<LitTabpane>('#tabs lit-tabpane').values() || []);
    return tabs.find((it) => it.key === key);
  }

  initHtml(): string {
    return `
            <style>
                :host([mode='hidden']){
                    display: none;
                }
                :host{
                    display: block;
                    background-color: rebeccapurple;
                }
                .tabHeight{
                    height: 30vh;
                    background-color: var(--dark-background,#FFFFFF);
                }
            </style>
            <div id="container" style="border-top: 1px solid var(--dark-border1,#D5D5D5);">
                <lit-tabs id="tabs" position="top-left" activekey="1" mode="card" >
                    <div slot="right" style="margin: 0 10px; color: var(--dark-icon,#606060);display: flex;align-items: center;">
                        <div title="SO导入" id="import_div" style="width: 20px;height: 20px;display: flex;flex-direction: row;margin-right: 10px">
                            <input id="import-file" style="display: none;pointer-events: none" type="file" webkitdirectory>
                            <label style="width: 20px;height: 20px;cursor: pointer;" for="import-file">
                                <lit-icon id="import-btn" name="copy-csv" style="pointer-events: none" size="20">
                                </lit-icon>
                            </label>
                        </div>
                        <lit-icon title="下载数据" id="export-btn" name="import-so" style="font-weight: bold;cursor: pointer;margin-right: 10px" size="20">
                        </lit-icon>
                        <lit-icon title="最大化" id="max-btn" name="vertical-align-top" style="font-weight: bold;cursor: pointer;margin-right: 10px" size="20">
                        </lit-icon>
                        <lit-icon title="最小化" id="min-btn" name="down" style="font-weight: bold;cursor: pointer;" size="20">
                        </lit-icon>
                    </div>
                </lit-tabs>
            </div>`;
  }

  displayThreadData = (
    data: ThreadStruct,
    scrollCallback: ((e: ThreadStruct) => void) | undefined,
    scrollWakeUp: (d: any) => void | undefined
  ) => this.displayTab<TabPaneCurrentSelection>('current-selection').setThreadData(data, scrollCallback, scrollWakeUp);
  displayMemData = (data: ProcessMemStruct) =>
    this.displayTab<TabPaneCurrentSelection>('current-selection').setMemData(data);
  displayClockData = (data: ClockStruct) =>
    this.displayTab<TabPaneCurrentSelection>('current-selection').setClockData(data);
  displayIrqData = (data: IrqStruct) => this.displayTab<TabPaneCurrentSelection>('current-selection').setIrqData(data);

  displayNativeHookData = (data: HeapStruct, rowType: string) => {
    let val = new SelectionParam();
    val.nativeMemoryStatistic.push(rowType);
    val.nativeMemory = [];
    val.leftNs = data.startTime!;
    val.rightNs = data.startTime! + data.dur! - 1;
    this.selection = val;
    this.displayTab<TabPaneNMStatisticAnalysis>('box-native-statistic-analysis', 'box-native-calltree').data = val;
    this.showUploadSoBt(val);
  };

  displayFuncData = (data: FuncStruct, scrollCallback: Function) =>
    this.displayTab<TabPaneCurrentSelection>('current-selection').setFunctionData(data, scrollCallback);
  displayCpuData = (
    data: CpuStruct,
    callback: ((data: WakeupBean | null) => void) | undefined = undefined,
    scrollCallback?: (data: CpuStruct) => void
  ) => this.displayTab<TabPaneCurrentSelection>('current-selection').setCpuData(data, callback, scrollCallback);
  displayJankData = (
    data: JankStruct,
    callback: ((data: Array<any>) => void) | undefined = undefined,
    scrollCallback: ((e: JankStruct) => void) | undefined
  ) => this.displayTab<TabPaneCurrentSelection>('current-selection').setJankData(data, callback, scrollCallback);
  displaySnapshotData = (
    data: HeapSnapshotStruct,
    dataList: Array<HeapSnapshotStruct>,
    scrollCallback?: (data: HeapSnapshotStruct, dataList: Array<HeapSnapshotStruct>) => void
  ) => {
    if (dataList.length > 1) {
      this.displayTab<TabPaneSummary>('box-heap-summary', 'box-heap-comparison').setSnapshotData(
        data,
        dataList,
        scrollCallback
      );
      let nav = this.shadowRoot!.querySelector('#tabs')!.shadowRoot!.querySelector(
        '#nav > #nav-comparison'
      ) as HTMLDivElement;
      let tabPaneComparison = this.shadowRoot!.querySelector(
        '#box-heap-comparison > tabpane-comparison'
      ) as TabPaneComparison;
      nav!.onclick = () => {
        tabPaneComparison.initComparison(data, dataList);
      };
    } else {
      this.displayTab<TabPaneSummary>('box-heap-summary').setSnapshotData(data, dataList, scrollCallback);
    }
  };
  displayFlagData = (flagObj: Flag) => this.displayTab<TabPaneFlag>('box-flag').setFlagObj(flagObj);
  displayFreqData = () =>
    (this.displayTab<TabPaneCurrentSelection>('box-freq').data = CpuFreqStruct.selectCpuFreqStruct);
  displayCpuStateData = () =>
    (this.displayTab<TabPaneCurrentSelection>('cpu-state-click').data = CpuStateStruct.selectStateStruct);
  displayFreqLimitData = () =>
    (this.displayTab<TabPaneCurrentSelection>('box-freq-limit').data = CpuFreqLimitsStruct.selectCpuFreqLimitsStruct);

  rangeSelect(selection: SelectionParam, restore = false): boolean {
    this.selection = selection;
    this.showUploadSoBt(selection);
    Reflect.ownKeys(tabConfig)
      .reverse()
      .forEach((id) => {
        let element = tabConfig[id];
        if (element.require) {
          if (element.require(selection)) {
            let pane = this.shadowRoot!.querySelector<LitTabpane>(`#${id as string}`);
            pane!.hidden = false;
          } else {
            this.shadowRoot!.querySelector<LitTabpane>(`#${id as string}`)!.hidden = true;
          }
        } else {
          this.shadowRoot!.querySelector<LitTabpane>(`#${id as string}`)!.hidden = true;
        }
      });
    if (restore) {
      if (this.litTabs?.activekey) {
        this.loadTabPaneData(this.litTabs?.activekey);
        this.setAttribute('mode', 'max');
        return true;
      } else {
        this.setAttribute('mode', 'hidden');
        return false;
      }
    } else {
      let firstPane = this.shadowRoot!.querySelector<LitTabpane>(`lit-tabpane[hidden='false']`);
      if (firstPane) {
        this.litTabs?.activeByKey(firstPane.key);
        this.loadTabPaneData(firstPane.key);
        this.setAttribute('mode', 'max');
        return true;
      } else {
        this.setAttribute('mode', 'hidden');
        return false;
      }
    }
  }

  updateRangeSelect(): boolean {
    if (
      this.selection &&
      (this.selection.nativeMemory.length > 0 ||
        this.selection.nativeMemoryStatistic.length > 0 ||
        this.selection.perfSampleIds.length > 0 ||
        this.selection.fileSystemType.length > 0 ||
        this.selection.fsCount > 0 ||
        this.selection.fileSysVirtualMemory ||
        this.selection.vmCount > 0 ||
        this.selection.diskIOLatency ||
        this.selection.diskIOipids.length > 0)
    ) {
      let param: SelectionParam = new SelectionParam();
      Object.assign(param, this.selection);
      this.rangeSelect(param, true);
      return true;
    } else {
      return false;
    }
  }

  showUploadSoBt(selection: SelectionParam | null | undefined) {
    if (
      selection &&
      (selection.nativeMemory.length > 0 ||
        selection.nativeMemoryStatistic.length > 0 ||
        selection.perfSampleIds.length > 0 ||
        selection.fileSystemType.length > 0 ||
        selection.fsCount > 0 ||
        selection.fileSysVirtualMemory ||
        selection.vmCount > 0 ||
        selection.diskIOLatency ||
        selection.diskIOipids.length > 0)
    ) {
      this.importDiv!.style.display = 'flex';
    } else {
      this.importDiv!.style.display = 'none';
    }
  }

  loadTabPaneData(key: string) {
    let component: any = this.shadowRoot
      ?.querySelector<LitTabpane>(`#tabs lit-tabpane[key='${key}']`)
      ?.children.item(0);
    if (component) {
      component.data = this.selection;
    }
  }

  rowClickHandler(e: any) {
    this.currentPaneID = e.target.parentElement.id;
    this.shadowRoot!.querySelectorAll<LitTabpane>(`lit-tabpane`).forEach((it) =>
      it.id != this.currentPaneID ? (it.hidden = true) : (it.hidden = false)
    );
    let pane = this.getPaneByID('box-cpu-child');
    pane.closeable = true;
    pane.hidden = false;
    this.litTabs!.activeByKey(pane.key);
    pane.tab = e.detail.title;
    let param = new BoxJumpParam();
    param.leftNs = this.selection!.leftNs;
    param.rightNs = this.selection!.rightNs;
    param.state = e.detail.state;
    param.threadId = e.detail.threadId;
    param.processId = e.detail.processId;
    (pane.children.item(0) as TabPaneBoxChild).data = param;
  }

  clearMemory() {
    let allTabs = Array.from(this.shadowRoot?.querySelectorAll<LitTabpane>('#tabs lit-tabpane').values() || []);
    allTabs.forEach( tab => {
      if (tab) {
        let tables = Array.from(
          (tab.firstChild as BaseElement).shadowRoot?.querySelectorAll<LitTable>('lit-table') || []
        );
        for (let table of tables) {
          table.recycleDataSource = [];
        }
      }
    });
  }
}
