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
import { LitChartPie } from '../../../../../base-ui/chart/pie/LitChartPie.js';
import '../../../../../base-ui/chart/pie/LitChartPie.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { TabPaneFilter } from '../TabPaneFilter.js';
import { LitCheckBox } from '../../../../../base-ui/checkbox/LitCheckBox.js';
import { initSort } from '../SheetUtils.js';
import { TabpanePerfProfile } from './TabPerfProfile.js';

@element('tabpane-perf-analysis')
export class TabPanePerfAnalysis extends BaseElement {
  private currentSelection: SelectionParam | any;
  private perfAnalysisPie: LitChartPie | null | undefined;
  private processData!: Array<any>;
  private pidData!: any[];
  private threadData!: any[];
  private soData!: any[];
  private functionData!: any[];
  private perfTableThread: LitTable | null | undefined;
  private perfTableProcess: LitTable | null | undefined;
  private perfTableSo: LitTable | null | undefined;
  private tableFunction: LitTable | null | undefined;
  private sumCount: number | undefined | null;
  private perfAnalysisRange: HTMLLabelElement | null | undefined;
  private back: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private processName: string = '';
  private threadName: string = '';
  private callChainMap!: Map<number, any>;
  private sortColumn: string = '';
  private sortType: number = 0;
  private allProcessCount!: {};
  private allThreadCount!: {};
  private allLibCount!: {};
  private allSymbolCount!: {};
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private titleEl: HTMLDivElement | undefined | null;
  private filterEl: TabPaneFilter | undefined | null;
  private hideProcessCheckBox: LitCheckBox | undefined | null;
  private hideThreadCheckBox: LitCheckBox | undefined | null;
  private checkBoxs: NodeListOf<LitCheckBox> | undefined | null;
  private tableArray: NodeListOf<LitTable> | undefined | null;

  set data(val: SelectionParam) {
    if (val === this.currentSelection) {
      this.pidData.unshift(this.allProcessCount);
      this.perfTableProcess!.recycleDataSource = this.pidData;
      // @ts-ignore
      this.pidData.shift(this.allProcessCount);
      return;
    }
    if (this.tableArray) {
      for (let table of this.tableArray) {
        initSort(table!, this.sortColumn, this.sortType);
      }
    }
    this.currentSelection = val;
    this.tabName!.textContent = '';
    this.hideProcessCheckBox!.checked = false;
    this.hideThreadCheckBox!.checked = false;
    this.reset(this.perfTableProcess!, false);
    this.titleEl!.textContent = '';
    this.perfAnalysisRange!.textContent =
      'Selected range: ' + parseFloat(((val.rightNs - val.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    if (!this.callChainMap) {
      this.getCallChainDataFromWorker(val);
    }
  }
  initElements(): void {
    this.perfAnalysisPie = this.shadowRoot!.querySelector<LitChartPie>('#perf-chart-pie');
    this.perfAnalysisRange = this.shadowRoot?.querySelector('#time-range');
    this.perfTableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.perfTableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.tableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.perfTableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.back = this.shadowRoot!.querySelector<HTMLDivElement>('.perf-go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.perf-subheading');
    this.progressEL = this.shadowRoot?.querySelector('.perf-progress') as LitProgressBar;
    this.titleEl = this.shadowRoot!.querySelector<HTMLDivElement>('.title');
    this.filterEl = this.shadowRoot?.querySelector('#filter');
    this.filterEl!.setOptionsList(['Hide Process', 'Hide Thread']);
    let popover = this.filterEl!.shadowRoot!.querySelector('#check-popover');
    this.hideProcessCheckBox = popover!!.querySelector<LitCheckBox>('div > #hideProcess');
    this.hideThreadCheckBox = popover!!.querySelector<LitCheckBox>('div > #hideThread');
    this.checkBoxs = popover!.querySelectorAll<LitCheckBox>('.check-wrap > lit-check-box');
    this.tableArray = this.shadowRoot!.querySelectorAll('lit-table') as NodeListOf<LitTable>;
    for (let perfTable of this.tableArray) {
      let querySelector = perfTable.shadowRoot?.querySelector<HTMLDivElement>('.table');
      if (querySelector) {
        querySelector.style.height = 'calc(100% - 31px)';
      }
      perfTable!.addEventListener('column-click', (evt) => {
        // @ts-ignore
        this.sortColumn = evt.detail.key;
        // @ts-ignore
        this.sortType = evt.detail.sort;
        this.sortByColumn();
      });
      perfTable!.addEventListener('contextmenu', function (event) {
        event.preventDefault(); // 阻止默认的上下文菜单弹框
      });
      perfTable!.addEventListener('row-hover', (evt) => {
        // @ts-ignore
        let detail = evt.detail;
        if (detail.data) {
          let data = detail.data;
          data.isHover = true;
          if (detail.callBack) {
            detail.callBack(true);
          }
        }
        this.perfAnalysisPie?.showHover();
        this.perfAnalysisPie?.hideTip();
      });
      perfTable!.addEventListener('row-click', (evt) => {
        // @ts-ignore
        let detail = evt.detail;
        let perfProfileTab = this.parentElement?.parentElement?.querySelector<TabpanePerfProfile>(
          '#box-perf-profile > tabpane-perf-profile'
        );
        if (detail.button === 2) {
          perfProfileTab!.cWidth = this.clientWidth;
          perfProfileTab!.currentLevel = this.currentLevel;
          if (this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
            detail.data.pid = undefined;
            detail.data.tid = undefined;
          }
          perfProfileTab!.rowClickData = detail.data;
          let title = '';
          if (this.titleEl?.textContent === '') {
            title = detail.data.tableName;
          } else {
            title = this.titleEl?.textContent + ' / ' + detail.data.tableName;
          }
          perfProfileTab!.pieTitle = title;
          //  是否是在表格上右键点击跳转到火焰图的
          this.currentSelection.isRowClick = true;
          perfProfileTab!.data = this.currentSelection;
        }
      });
    }
    for (let box of this.checkBoxs) {
      box!.addEventListener('change', (event) => {
        if (
          (this.hideProcessCheckBox!.checked && this.hideThreadCheckBox!.checked) ||
          (this.hideThreadCheckBox!.checked &&
            this.currentSelection.perfThread.length > 0 &&
            this.currentSelection.perfProcess.length === 0)
        ) {
          this.processName = '';
          this.hideThread();
          this.back!.style.visibility = 'hidden';
        } else if (this.hideProcessCheckBox!.checked && !this.hideThreadCheckBox!.checked) {
          this.hideProcess();
        } else {
          this.getHiperfProcess(this.currentSelection);
        }
      });
    }
    this.getBack();

    const addRowClickEventListener = (table: LitTable, clickEvent: Function) => {
      table.addEventListener('row-click', (evt) => {
        // @ts-ignore
        const detail = evt.detail;
        // @ts-ignore
        const data = detail.data;
        if (detail.button === 0 && data.tableName !== '' && data.count !== 0) {
          clickEvent(data, this.currentSelection);
        }
      });
    };

    addRowClickEventListener(this.perfTableProcess!, this.perfProcessLevelClickEvent.bind(this));
    addRowClickEventListener(this.perfTableThread!, this.perfThreadLevelClickEvent.bind(this));
    addRowClickEventListener(this.perfTableSo!, this.perfSoLevelClickEvent.bind(this));
  }

  private reset(showTable: LitTable, isShowBack: boolean): void {
    this.clearData();
    if (isShowBack) {
      this.back!.style.visibility = 'visible';
    } else {
      this.back!.style.visibility = 'hidden';
      this.titleEl!.textContent = '';
    }
    if (this.tableArray) {
      for (let table of this.tableArray) {
        if (table === showTable) {
          initSort(table!, this.sortColumn, this.sortType);
          table.style.display = 'grid';
          table.setAttribute('hideDownload', '');
        } else {
          table!.style.display = 'none';
          table!.removeAttribute('hideDownload');
        }
      }
    }
  }

  private clearData(): void {
    this.perfAnalysisPie!.dataSource = [];
    this.perfTableProcess!.recycleDataSource = [];
    this.perfTableThread!.recycleDataSource = [];
    this.perfTableSo!.recycleDataSource = [];
    this.tableFunction!.recycleDataSource = [];
  }

  private showAssignLevel(
    showTable: LitTable,
    hideTable: LitTable,
    currentLevel: number,
    currentLevelData: Array<any>
  ): void {
    showTable!.style.display = 'grid';
    hideTable!.style.display = 'none';
    hideTable.setAttribute('hideDownload', '');
    showTable?.removeAttribute('hideDownload');
    this.currentLevel = currentLevel;
    this.currentLevelData = currentLevelData;
  }

  private getBack(): void {
    this.back!.addEventListener('click', (e) => {
      if (this.tabName!.textContent === 'Statistic By Thread Count') {
        this.showAssignLevel(this.perfTableProcess!, this.perfTableThread!, 0, this.pidData);
        this.back!.style.visibility = 'hidden';
        this.processPieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Library Count') {
        if (this.hideThreadCheckBox?.checked) {
          this.showAssignLevel(this.perfTableProcess!, this.perfTableSo!, 0, this.pidData);
          this.back!.style.visibility = 'hidden';
          this.processPieChart(this.currentSelection);
        } else {
          this.showAssignLevel(this.perfTableThread!, this.perfTableSo!, 1, this.threadData);
          this.threadPieChart(this.currentSelection);
        }
      } else if (this.tabName!.textContent === 'Statistic By Function Count') {
        this.showAssignLevel(this.perfTableSo!, this.tableFunction!, 2, this.soData);
        this.libraryPieChart(this.currentSelection);
      }
      if (
        (this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) ||
        (this.hideProcessCheckBox?.checked && this.tabName!.textContent === 'Statistic By Thread Count') ||
        (this.hideThreadCheckBox?.checked &&
          this.tabName!.textContent === 'Statistic By Library Count' &&
          this.currentSelection.perfThread.length > 0 &&
          this.currentSelection.perfProcess.length === 0)
      ) {
        this.back!.style.visibility = 'hidden';
        this.titleEl!.textContent = '';
      }
    });
  }

  private hideProcess(): void {
    this.reset(this.perfTableThread!, false);
    this.processName = '';
    this.titleEl!.textContent = '';
    this.getHiperfThread(null, this.currentSelection);
  }

  private hideThread(it?: any): void {
    this.reset(this.perfTableSo!, true);
    this.threadName = '';
    if (it) {
      this.getHiperfSo(it, this.currentSelection);
    } else {
      this.getHiperfSo(null, this.currentSelection);
    }
  }

  private processPieChart(val: SelectionParam): void {
    // @ts-ignore
    this.sumCount = this.allProcessCount.allCount;
    this.perfAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPerfPieChartData(this.pidData),
      angleField: 'count',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (perfObj): string => {
        return `<div>
                                <div>Process:${perfObj.obj.tableName}</div>
                                <div>Sample Count:${perfObj.obj.count}</div>
                                <div>Percent:${perfObj.obj.percent}%</div> 
                                <div>Event Count:${perfObj.obj.eventCount}</div>
                                <div>Percent:${perfObj.obj.eventPercent}%</div> 
                            </div>
                               `;
      },
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.perfProcessLevelClickEvent(it, val);
        }
      },
      hoverHandler: (perfAnalyData): void => {
        if (perfAnalyData) {
          this.perfTableProcess!.setCurrentHover(perfAnalyData);
        } else {
          this.perfTableProcess!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.titleEl!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process Count';
    this.pidData.unshift(this.allProcessCount);
    this.perfTableProcess!.recycleDataSource = this.pidData;
    this.perfTableProcess?.reMeauseHeight();
    // @ts-ignore
    this.pidData.shift(this.allProcessCount);
    this.currentLevelData = this.pidData;
  }

  private perfProcessLevelClickEvent(it: any, val: SelectionParam): void {
    if (this.hideThreadCheckBox!.checked) {
      this.hideThread(it);
    } else {
      this.reset(this.perfTableThread!, true);
      this.getHiperfThread(it, val);
    }
    // @ts-ignore
    this.titleEl!.textContent = it.tableName;
    // @ts-ignore
    this.processName = it.tableName;
    this.perfAnalysisPie?.hideTip();
  }

  private threadPieChart(val: SelectionParam): void {
    if (val.perfThread.length > 0 && val.perfProcess.length === 0) {
      this.back!.style.visibility = 'hidden';
      this.titleEl!.textContent = '';
      this.perfTableThread!.style.display = 'grid';
    } else {
      // @ts-ignore
      this.titleEl!.textContent = this.processName;
    }
    // @ts-ignore
    this.sumCount = this.allThreadCount.allCount;
    this.perfAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPerfPieChartData(this.threadData),
      angleField: 'count',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj): string => {
        return `<div>
                                <div>Thread:${obj.obj.tableName}</div>
                                <div>Sample Count:${obj.obj.count}</div>
                                <div>Percent:${obj.obj.percent}%</div>
                                <div>Event Count:${obj.obj.eventCount}</div>
                                <div>Percent:${obj.obj.eventPercent}%</div>  
                            </div>
                                `;
      },
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.perfThreadLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.perfTableThread!.setCurrentHover(data);
        } else {
          this.perfTableThread!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.tabName!.textContent = 'Statistic By Thread Count';
    this.threadData.unshift(this.allThreadCount);
    this.perfTableThread!.recycleDataSource = this.threadData;
    this.perfTableThread?.reMeauseHeight();
    // @ts-ignore
    this.threadData.shift(this.allThreadCount);
    this.currentLevelData = this.threadData;
  }

  private perfThreadLevelClickEvent(it: any, val: SelectionParam): void {
    this.reset(this.perfTableSo!, true);
    this.getHiperfSo(it, val);
    let pName = this.processName;
    if (this.processName.length > 0 && it.tableName.length > 0) {
      pName = this.processName + ' / ';
    }
    this.titleEl!.textContent = pName + it.tableName;
    // @ts-ignore
    this.threadName = it.tableName;
    this.perfAnalysisPie?.hideTip();
  }

  private libraryPieChart(val: SelectionParam): void {
    // @ts-ignore
    this.sumCount = this.allLibCount.allCount;
    this.perfAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPerfPieChartData(this.soData),
      angleField: 'count',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj): string => {
        return `<div>
                                <div>Library:${obj.obj.tableName}</div>
                                <div>Sample Count:${obj.obj.count}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                                <div>Event Count:${obj.obj.eventCount}</div>
                                <div>Percent:${obj.obj.eventPercent}%</div>  
                            </div>
                                `;
      },
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.perfSoLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.perfTableSo!.setCurrentHover(data);
        } else {
          this.perfTableSo!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let pName = this.processName;
    if (this.processName.length > 0 && this.threadName.length > 0) {
      pName = this.processName + ' / ';
    }
    this.titleEl!.textContent = pName + this.threadName;
    this.tabName!.textContent = 'Statistic By Library Count';
    this.soData.unshift(this.allLibCount);
    this.perfTableSo!.recycleDataSource = this.soData;
    this.perfTableSo?.reMeauseHeight();
    // @ts-ignore
    this.soData.shift(this.allLibCount);
    this.currentLevelData = this.soData;
  }

  private perfSoLevelClickEvent(it: any, val: SelectionParam): void {
    this.reset(this.tableFunction!, true);
    this.getHiperfFunction(it);
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.threadName.length > 0) {
      title += this.threadName + ' / ';
    }
    if (it.tableName.length > 0) {
      title += it.tableName;
    }
    this.titleEl!.textContent = title;
    this.perfAnalysisPie?.hideTip();
  }

  private sortByColumn(): void {
    let currentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        currentTable = this.perfTableProcess;
        break;
      case 1:
        currentTable = this.perfTableThread;
        break;
      case 2:
        currentTable = this.perfTableSo;
        break;
      case 3:
        currentTable = this.tableFunction;
        break;
    }
    if (!currentTable) {
      return;
    }
    if (this.sortType === 0) {
      let arr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          arr.unshift(this.allProcessCount);
          break;
        case 1:
          arr.unshift(this.allThreadCount);
          break;
        case 2:
          arr.unshift(this.allLibCount);
          break;
        case 3:
          arr.unshift(this.allSymbolCount);
          break;
      }
      currentTable!.recycleDataSource = arr;
    } else {
      let array = [...this.currentLevelData];
      if (this.sortColumn === 'tableName') {
        currentTable!.recycleDataSource = array.sort((leftA, rightB) => {
          if (this.sortType === 1) {
            if (leftA.tableName > rightB.tableName) {
              return 1;
            } else if (leftA.tableName === rightB.tableName) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (rightB.tableName > leftA.tableName) {
              return 1;
            } else if (leftA.tableName === rightB.tableName) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (this.sortColumn === 'count' || this.sortColumn === 'percent') {
        currentTable!.recycleDataSource = array.sort((a, b) => {
          return this.sortType === 1 ? a.count - b.count : b.count - a.count;
        });
      } else if (this.sortColumn === 'eventCount' || this.sortColumn === 'eventPercent') {
        currentTable!.recycleDataSource = array.sort((a, b) => {
          return this.sortType === 1 ? a.eventCount - b.eventCount : b.eventCount - a.eventCount;
        });
      }
      switch (this.currentLevel) {
        case 0:
          array.unshift(this.allProcessCount);
          break;
        case 1:
          array.unshift(this.allThreadCount);
          break;
        case 2:
          array.unshift(this.allLibCount);
          break;
        case 3:
          array.unshift(this.allSymbolCount);
          break;
      }
      currentTable!.recycleDataSource = array;
    }
  }

  async getHiperfProcess(val: SelectionParam): Promise<void> {
    this.reset(this.perfTableProcess!, false);
    this.progressEL!.loading = true;
    if (!this.processData || this.processData.length === 0) {
      this.progressEL!.loading = false;
      if (val.perfThread.length > 0 && val.perfProcess.length === 0) {
        this.threadData = [];
        this.allThreadCount = [];
        this.perfTableProcess!.style.display = 'none';
        this.threadPieChart(val);
      } else {
        this.pidData = [];
        this.allProcessCount = [];
        this.processPieChart(val);
      }
      return;
    }
    let allCount = 0;
    let allEventCount = 0;
    let pidMap = new Map<number, Array<number | string>>();
    if (val.perfThread.length > 0 && val.perfProcess.length === 0) {
      this.perfTableProcess!.style.display = 'none';
      this.getHiperfThread(this.processData[0], val);
    } else {
      for (let itemData of this.processData) {
        allCount += itemData.count;
        allEventCount += itemData.eventCount;
        if (pidMap.has(itemData.pid)) {
          pidMap.get(itemData.pid)?.push(itemData);
        } else {
          let itemArray = new Array<number | string>();
          itemArray.push(itemData);
          pidMap.set(itemData.pid, itemArray);
        }
      }
      this.pidData = [];
      pidMap.forEach((arr: Array<any>, pid: number) => {
        let count = 0;
        let eventCount = 0;
        for (let item of arr) {
          count += item.count;
          eventCount += item.eventCount;
        }
        const pName = arr[0].processName + '(' + pid + ')';
        const pidData = {
          tableName: pName,
          pid: pid,
          percent: ((count / allCount) * 100).toFixed(2),
          count: count,
          eventCount: eventCount,
          eventPercent: ((eventCount / allEventCount) * 100).toFixed(2),
        };
        this.pidData.push(pidData);
      });
      this.pidData.sort((a, b) => b.count - a.count);
      this.allProcessCount = this.totalCountData(allCount, allEventCount);
      this.currentLevel = 0;
      this.progressEL!.loading = false;
      this.processPieChart(val);
    }
  }

  private getHiperfThread(item: any, val: SelectionParam): void {
    this.progressEL!.loading = true;
    let threadMap = new Map<number, Array<number | string>>();
    let allCount = 0;
    let allEventCount = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (item && itemData.pid !== item.pid && !this.hideProcessCheckBox?.checked) {
        continue;
      }
      allCount += itemData.count;
      allEventCount += itemData.eventCount;
      if (threadMap.has(itemData.tid)) {
        threadMap.get(itemData.tid)?.push(itemData);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(itemData);
        threadMap.set(itemData.tid, itemArray);
      }
    }
    this.threadData = [];
    threadMap.forEach((arr: Array<any>, tid: number) => {
      let threadCount = 0;
      let threadEventCount = 0;
      let tName = arr[0].threadName + '(' + tid + ')';
      for (let item of arr) {
        threadCount += item.count;
        threadEventCount += item.eventCount;
      }
      const threadData = {
        pid: item === null ? arr[0].pid : item.pid,
        tid: tid,
        tableName: tName,
        count: threadCount,
        percent: ((threadCount / allCount) * 100).toFixed(2),
        eventCount: threadEventCount,
        eventPercent: ((threadEventCount / allEventCount) * 100).toFixed(2),
      };
      this.threadData.push(threadData);
    });
    this.allThreadCount = this.totalCountData(allCount, allEventCount);
    this.currentLevel = 1;
    this.threadData.sort((a, b) => b.count - a.count);
    this.progressEL!.loading = false;
    this.threadPieChart(val);
  }

  private getHiperfSo(item: any, val: SelectionParam): void {
    this.progressEL!.loading = true;
    let parentEventCount = 0;
    let allCount = 0;
    let allEventCount = 0;
    let libMap = new Map<string, Array<number | string>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (!this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (item && (itemData.pid !== item.pid || itemData.tid !== item.tid)) {
          continue;
        }
      } else if (!this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (item && itemData.pid !== item.pid) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (item && itemData.tid !== item.tid) {
          continue;
        }
      }
      allCount += itemData.count;
      allEventCount += itemData.eventCount;
      if (libMap.has(itemData.libId + '-' + itemData.libName)) {
        libMap.get(itemData.libId + '-' + itemData.libName)?.push(itemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(itemData);
        libMap.set(itemData.libId + '-' + itemData.libName, dataArray);
      }
    }
    if (!item) {
      parentEventCount = allEventCount;
    }
    this.soData = [];
    libMap.forEach((arr: Array<any>, libInfo: string) => {
      let libCount = 0;
      let libEventCount = 0;
      let libName = arr[0].libName;
      let libId = arr[0].libId;
      for (let item of arr) {
        libCount += item.count;
        libEventCount += item.eventCount;
      }
      const libData = {
        pid: item === null ? arr[0].pid : item.pid,
        tid: item === null ? arr[0].tid : item.tid,
        percent: ((libCount / allCount) * 100).toFixed(2),
        count: libCount,
        eventPercent: ((libEventCount / parentEventCount) * 100).toFixed(2),
        eventCount: libEventCount,
        tableName: libName,
        libId: libId,
      };
      this.soData.push(libData);
    });
    this.allLibCount = this.totalCountData(allCount, allEventCount);
    this.soData.sort((a, b) => b.count - a.count);
    this.currentLevel = 2;
    this.progressEL!.loading = false;
    this.libraryPieChart(val);
  }

  private getHiperfFunction(item: any): void {
    this.progressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.perf-subheading')!.textContent = 'Statistic By Function Count';
    let parentCount = item.count;
    let parentEventCount = item.eventCount;
    let tid = item.tid;
    let pid = item.pid;
    let libId = item.libId;
    let allCount = 0;
    let allEventCount = 0;
    let symbolMap = new Map<string, Array<any>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (!this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (itemData.pid !== pid || itemData.tid !== tid || itemData.libId !== libId) {
          continue;
        }
      } else if (!this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (itemData.pid !== pid || itemData.libId !== libId) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (itemData.tid !== tid || itemData.libId !== libId) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (itemData.libId !== libId) {
          continue;
        }
      }
      allCount += itemData.count;
      allEventCount += itemData.eventCount;
      if (symbolMap.has(itemData.symbolId + '-' + itemData.symbolName)) {
        symbolMap.get(itemData.symbolId + '-' + itemData.symbolName)?.push(itemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(itemData);
        symbolMap.set(itemData.symbolId + '-' + itemData.symbolName, dataArray);
      }
    }
    this.functionData = [];
    symbolMap.forEach((arr, symbolInfo) => {
      let symbolCount = 0;
      let symbolEventCount = 0;
      for (let item of arr) {
        symbolCount += item.count;
        symbolEventCount += item.eventCount;
      }
      let symbolName = arr[0].symbolName;
      let symbolId = arr[0].symbolId;
      const symbolData = {
        pid: item.pid,
        tid: item.tid,
        libId: item.libId,
        percent: ((symbolCount / parentCount) * 100).toFixed(2),
        count: symbolCount,
        symbolId: symbolId,
        eventPercent: ((symbolEventCount / parentEventCount) * 100).toFixed(2),
        eventCount: symbolEventCount,
        tableName: symbolName,
      };
      this.functionData.push(symbolData);
    });
    this.functionData.sort((a, b) => b.count - a.count);
    this.allSymbolCount = this.totalCountData(allCount, allEventCount);
    this.currentLevel = 3;
    this.progressEL!.loading = false;
    // @ts-ignore
    this.sumCount = this.allSymbolCount.allCount;
    this.perfAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPerfPieChartData(this.functionData),
      angleField: 'count',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getTip(),
      hoverHandler: (data): void => {
        if (data) {
          this.tableFunction!.setCurrentHover(data);
        } else {
          this.tableFunction!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.functionData.unshift(this.allSymbolCount);
    this.tableFunction!.recycleDataSource = this.functionData;
    this.tableFunction?.reMeauseHeight();
    // @ts-ignore
    this.functionData.shift(this.allSymbolCount);
    this.currentLevelData = this.functionData;
  }

  private getTip() {
    return (obj: { obj: { tableName: any; count: any; percent: any; eventCount: any; eventPercent: any } }): string => {
      return `<div>
                    <div>Function:${obj.obj.tableName}</div>
                    <div>Sample Count:${obj.obj.count}</div>
                    <div>Percent:${obj.obj.percent}%</div>
                    <div>Event Count:${obj.obj.eventCount}</div>
                    <div>Percent:${obj.obj.eventPercent}%</div>
                </div>`;
    };
  }

  totalCountData(
    count: number,
    eventCount: number
  ): {
    percent: string;
    count: number;
    allCount: number;
    eventCount: number;
    eventPercent: string;
    allEventCount: number;
    pid: string;
  } {
    let allCount;
    allCount = {
      percent: ((count / count) * 100).toFixed(2),
      count: count,
      allCount: count,
      eventCount: eventCount,
      allEventCount: eventCount,
      eventPercent: '100.00',
      pid: '',
    };
    return allCount;
  }

  private getPerfPieChartData(res: any[]): unknown[] {
    if (res.length > 20) {
      let pieChartArr: string[] = [];
      let other: any = {
        tableName: 'other',
        count: 0,
        percent: 0,
      };
      for (let i = 0; i < res.length; i++) {
        if (i < 19) {
          pieChartArr.push(res[i]);
        } else {
          other.count += res[i].count;
          other.percent = ((other.count / this.sumCount!) * 100).toFixed(2);
        }
      }
      pieChartArr.push(other);
      return pieChartArr;
    }
    return res;
  }

  private getCallChainDataFromWorker(val: SelectionParam): void {
    this.getDataByWorker(val, (results: any) => {
      this.processData = results;
      this.getHiperfProcess(val);
    });
  }

  private getDataByWorker(val: SelectionParam, handler: Function): void {
    this.progressEL!.loading = true;
    const args = [
      {
        funcName: 'setCombineCallChain',
        funcArgs: [''],
      },
      {
        funcName: 'setSearchValue',
        funcArgs: [''],
      },
      {
        funcName: 'getCurrentDataFromDb',
        funcArgs: [val],
      },
    ];
    procedurePool.submitWithName('logic0', 'perf-action', args, undefined, (results: any) => {
      handler(results);
      this.progressEL!.loading = false;
    });
  }

  public connectedCallback(): void {
    new ResizeObserver((entries) => {
      this.perfTableProcess!.style.height = this.parentElement!.clientHeight - 50 + 'px';
      this.perfTableProcess?.reMeauseHeight();
      this.perfTableThread!.style.height = this.parentElement!.clientHeight - 50 + 'px';
      this.perfTableThread?.reMeauseHeight();
      this.tableFunction!.style.height = this.parentElement!.clientHeight - 50 + 'px';
      this.tableFunction?.reMeauseHeight();
      this.perfTableSo!.style.height = this.parentElement!.clientHeight - 50 + 'px';
      this.perfTableSo?.reMeauseHeight();
      if (this.parentElement!.clientHeight >= 0 && this.parentElement!.clientHeight <= 31) {
        this.filterEl!.style.display = 'none';
      } else {
        this.filterEl!.style.display = 'flex';
      }
    }).observe(this.parentElement!);
  }

  initHtml(): string {
    return `
        <style>
        :host {
            display: flex;
            flex-direction: column;
        }
        #perf-chart-pie{
            height: 300px;
        }
        .perf-table-box{
            width: 60%;
            border-left: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
        }
        .perf-go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
            margin-left: 20px;
            visibility: hidden;
        }
        .perf-back-box{
            background-color: var(--bark-expansion,#0C65D1);
            border-radius: 5px;
            color: #fff;
            display: flex;
            margin-right: 10px;
            width: 40px;
            height: 20px;
            justify-content: center;
            align-items: center;
        }
        .perf-subheading{
            font-weight: bold;
            text-align: center;
        }
        .perf-progress{
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        #filter{
            position: absolute;
            bottom: 0px;
        }
        </style>
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label> 
        <div style="display: flex; flex-direction: row;" class="d-box">
            <lit-progress-bar class="perf-progress"></lit-progress-bar>
            <div id="left_table" style="width: 40%;height:auto;">
                <div style="display: flex;margin-bottom: 10px">
                    <div class="perf-go-back">
                        <div class="perf-back-box">
                            <lit-icon name="arrowleft"></lit-icon>
                        </div>
                    </div>
                    <div class="title"></div>
                </div>
                <div class="perf-subheading"></div>
                <lit-chart-pie  id="perf-chart-pie"></lit-chart-pie>
            </div>
            <div class="perf-table-box" style="height:auto;overflow: auto">
                <lit-table id="tb-process-usage" style="display: none;min-height: 380px" >
                    <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Sample Count" data-index="count" key="count" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Event Count" data-index="eventCount" key="eventCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="eventPercent" key="eventPercent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-thread-usage" style="display: none;min-height: 380px"hideDownload>
                    <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Sample Count" data-index="count" key="count" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Event Count" data-index="eventCount" key="eventCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="eventPercent" key="eventPercent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-so-usage" style="display: none;min-height: 380px"hideDownload>
                    <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Sample Count" data-index="count" key="count" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Event Count" data-index="eventCount" key="eventCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="eventPercent" key="eventPercent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-function-usage" style="display: none;min-height: 380px"hideDownload>
                    <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Sample Count" data-index="count" key="count" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Event Count" data-index="eventCount" key="eventCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="eventPercent" key="eventPercent" align="flex-start"order></lit-table-column>
                </lit-table>
            </div>
        </div>
        <tab-pane-filter id="filter" options></tab-pane-filter>
`;
  }
}
