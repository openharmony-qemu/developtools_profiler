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
import { Utils } from '../../base/Utils.js';
import { SpHiPerf } from '../../../chart/SpHiPerf.js';

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
  private processName: HTMLDivElement | null | undefined;
  private threadName: HTMLDivElement | null | undefined;
  private callChainMap!: Map<number, any>;
  private sortColumn: string = '';
  private sortType: number = 0;
  private allProcessCount!: {};
  private allThreadCount!: {};
  private allLibCount!: {};
  private allSymbolCount!: {};
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  set data(val: SelectionParam) {
    if (val === this.currentSelection) {
      this.pidData.unshift(this.allProcessCount);
      this.perfTableProcess!.recycleDataSource = this.pidData;
      // @ts-ignore
      this.pidData.shift(this.allProcessCount);
      return;
    }
    this.clearData();
    this.currentSelection = val;
    this.perfTableProcess!.style.display = 'grid';
    this.perfTableThread!.style.display = 'none';
    this.perfTableSo!.style.display = 'none';
    this.tableFunction!.style.display = 'none';
    this.back!.style.visibility = 'hidden';
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = '';
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
    this.getBack();
  }

  clearData(): void {
    this.perfAnalysisPie!.dataSource = [];
    this.perfTableProcess!.recycleDataSource = [];
    this.perfTableThread!.recycleDataSource = [];
    this.perfTableSo!.recycleDataSource = [];
    this.tableFunction!.recycleDataSource = [];
  }

  getBack(): void {
    this.back!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By Thread Count') {
        this.perfTableProcess!.style.display = 'grid';
        this.perfTableThread!.style.display = 'none';
        this.perfTableThread!.setAttribute('hideDownload', '');
        this.perfTableProcess?.removeAttribute('hideDownload');
        this.back!.style.visibility = 'hidden';
        this.currentLevel = 0;
        this.currentLevelData = this.pidData;
        this.processPieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Library Count') {
        this.perfTableThread!.style.display = 'grid';
        this.perfTableSo!.style.display = 'none';
        this.perfTableSo!.setAttribute('hideDownload', '');
        this.perfTableThread?.removeAttribute('hideDownload');
        this.currentLevel = 1;
        this.currentLevelData = this.threadData;
        this.threadPieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Function Count') {
        this.perfTableSo!.style.display = 'grid';
        this.tableFunction!.style.display = 'none';
        this.tableFunction!.setAttribute('hideDownload', '');
        this.perfTableSo?.removeAttribute('hideDownload');
        this.currentLevel = 2;
        this.currentLevelData = this.soData;
        this.libraryPieChart(this.currentSelection);
      }
    });
  }
  processPieChart(val: SelectionParam): void {
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
                                <div>Process:${ perfObj.obj.tableName }</div>
                                <div>Weight:${ perfObj.obj.countFormat }</div>
                                <div>Percent:${ perfObj.obj.percent }%</div> 
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
    this.perfTableProcess!.addEventListener('row-hover', (tblProcessRowHover) => {
      // @ts-ignore
      let tblProcessData = tblProcessRowHover.detail;
      if (tblProcessData.data) {
        let data = tblProcessData.data;
        data.isHover = true;
        if (tblProcessData.callBack) {
          tblProcessData.callBack(true);
        }
      }
      this.perfAnalysisPie?.showHover();
      this.perfAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process Count';
    this.pidData.unshift(this.allProcessCount);
    this.perfTableProcess!.recycleDataSource = this.pidData;
    this.perfTableProcess?.reMeauseHeight();
    // @ts-ignore
    this.pidData.shift(this.allProcessCount);
    this.currentLevelData = this.pidData;
    this.perfTableProcess!.addEventListener('row-click', (perfTableRowClick) => {
      // @ts-ignore
      let data = perfTableRowClick.detail.data;
      if (data.tableName !== '' && data.count !== 0) {
        this.perfProcessLevelClickEvent(data, val);
      }
    });
    this.perfTableProcess!.addEventListener('column-click', (perfTableColumnClick) => {
      // @ts-ignore
      this.sortByColumn(perfTableColumnClick.detail.key, perfTableColumnClick.detail.sort);
    });
  }
  perfProcessLevelClickEvent(it: any, val: SelectionParam): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.perfTableProcess!.style.display = 'none';
    this.perfTableThread!.style.display = 'grid';
    this.perfTableProcess!.setAttribute('hideDownload', '');
    this.perfTableThread?.removeAttribute('hideDownload');
    this.getHiperfThread(it, val);
    // @ts-ignore
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = it.tableName;
    // @ts-ignore
    this.processName = it.tableName;
    this.perfAnalysisPie?.hideTip();
  }
  threadPieChart(val: SelectionParam): void {
    if (val.perfThread.length > 0 && val.perfProcess.length === 0) {
      this.back!.style.visibility = 'hidden';
      this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
      this.perfTableThread!.style.display = 'grid';
    } else {
      // @ts-ignore
      this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName;
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
                                <div>Thread:${ obj.obj.tableName }</div>
                                <div>Weight:${ obj.obj.countFormat }</div>
                                <div>Percent:${ obj.obj.percent }%</div> 
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
    this.perfTableThread!.addEventListener('row-hover', (perfTblThreadRowHover) => {
      // @ts-ignore
      let perfTblThreadData = perfTblThreadRowHover.detail;
      if (perfTblThreadData.data) {
        let data = perfTblThreadData.data;
        data.isHover = true;
        if (perfTblThreadData.callBack) {
          perfTblThreadData.callBack(true);
        }
      }
      this.perfAnalysisPie?.showHover();
      this.perfAnalysisPie?.hideTip();
    });
    this.perfTableThread!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tabName!.textContent = 'Statistic By Thread Count';
    this.threadData.unshift(this.allThreadCount);
    this.perfTableThread!.recycleDataSource = this.threadData;
    this.perfTableThread?.reMeauseHeight();
    // @ts-ignore
    this.threadData.shift(this.allThreadCount);
    this.currentLevelData = this.threadData;
    this.perfTableThread!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.count !== 0) {
        this.perfThreadLevelClickEvent(data, val);
      }
    });
  }
  perfThreadLevelClickEvent(it: any, val: SelectionParam): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.perfTableThread!.style.display = 'none';
    this.perfTableSo!.style.display = 'grid';
    this.perfTableThread!.setAttribute('hideDownload', '');
    this.perfTableSo?.removeAttribute('hideDownload');
    this.getHiperfSo(it, val);
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      // @ts-ignore
      this.processName + ' / ' + it.tableName;
    // @ts-ignore
    this.threadName = it.tableName;
    this.perfAnalysisPie?.hideTip();
  }
  libraryPieChart(val: SelectionParam): void {
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
                                <div>Library:${ obj.obj.tableName }</div>
                                <div>Weight:${ obj.obj.countFormat }</div>
                                <div>Percent:${ obj.obj.percent }%</div> 
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
    this.perfTableSo!.addEventListener('row-hover', (tableSoRowHover) => {
      // @ts-ignore
      let perfTableSoData = tableSoRowHover.detail;
      if (perfTableSoData.data) {
        let data = perfTableSoData.data;
        data.isHover = true;
        if (perfTableSoData.callBack) {
          perfTableSoData.callBack(true);
        }
      }
      this.perfAnalysisPie?.showHover();
      this.perfAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName + ' / ' + this.threadName;
    this.tabName!.textContent = 'Statistic By Library Count';
    this.soData.unshift(this.allLibCount);
    this.perfTableSo!.recycleDataSource = this.soData;
    this.perfTableSo?.reMeauseHeight();
    // @ts-ignore
    this.soData.shift(this.allLibCount);
    this.currentLevelData = this.soData;
    this.perfTableSo!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.perfTableSo!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.count !== 0) {
        this.perfSoLevelClickEvent(data, val);
      }
    });
  }
  perfSoLevelClickEvent(it: any, val: SelectionParam): void {
    this.clearData();
    this.perfTableSo!.style.display = 'none';
    this.tableFunction!.style.display = 'grid';
    this.perfTableSo!.setAttribute('hideDownload', '');
    this.tableFunction?.removeAttribute('hideDownload');
    this.getHiperfFunction(it);
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      // @ts-ignore
      this.processName + ' / ' + this.threadName + ' / ' + it.tableName;
    this.perfAnalysisPie?.hideTip();
  }
  sortByColumn(column: string, sort: number): void {
    this.sortColumn = column;
    this.sortType = sort;
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
    if (sort === 0) {
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
      if (column === 'tableName') {
        currentTable!.recycleDataSource = array.sort((leftA, rightB) => {
          if (sort === 1) {
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
      } else if (column === 'countFormat' || column === 'percent') {
        currentTable!.recycleDataSource = array.sort((a, b) => {
          return sort === 1 ? a.count - b.count : b.count - a.count;
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
    let pidMap = new Map<number, Array<number | string>>();
    if (val.perfThread.length > 0 && val.perfProcess.length === 0) {
      this.perfTableProcess!.style.display = 'none';
      this.getHiperfThread(this.processData[0], val);
    } else {
      for (let itemData of this.processData) {
        allCount += itemData.count;
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
        for (let item of arr) {
          count += item.count;
        }
        const pName = arr[0].processName + '(' + pid + ')';
        const pidData = {
          tableName: pName,
          pid: pid,
          percent: ((count / allCount) * 100).toFixed(2),
          countFormat: Utils.timeMsFormat2p(count * (SpHiPerf.stringResult?.fValue || 1)),
          count: count,
        };
        this.pidData.push(pidData);
      });
      this.pidData.sort((a, b) => b.count - a.count);
      this.allProcessCount = this.totalCountData(allCount);
      this.currentLevel = 0;
      this.progressEL!.loading = false;
      this.processPieChart(val);
    }
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        this.perfTableProcess!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.perfTableProcess?.reMeauseHeight();
        this.perfTableThread!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.perfTableThread?.reMeauseHeight();
        this.tableFunction!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.tableFunction?.reMeauseHeight();
        this.perfTableSo!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.perfTableSo?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }
  getHiperfThread(item: any, val: SelectionParam): void {
    this.progressEL!.loading = true;
    let threadMap = new Map<number, Array<number | string>>();
    let pid = item.pid;
    let allCount = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (itemData.pid !== pid) {
        continue;
      }
      allCount += itemData.count;
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
      let tName = arr[0].threadName + '(' + tid + ')';
      for (let item of arr) {
        threadCount += item.count;
      }
      const threadData = {
        pid: item.pid,
        tid: tid,
        tableName: tName,
        countFormat: Utils.timeMsFormat2p(threadCount * (SpHiPerf.stringResult?.fValue || 1)),
        count: threadCount,
        percent: ((threadCount / allCount) * 100).toFixed(2),
      };
      this.threadData.push(threadData);
    });
    this.allThreadCount = this.totalCountData(allCount);
    this.currentLevel = 1;
    this.threadData.sort((a, b) => b.count - a.count);
    this.progressEL!.loading = false;
    this.threadPieChart(val);
  }
  getHiperfSo(item: any, val: SelectionParam): void {
    this.progressEL!.loading = true;
    let parentCount = item.count;
    let tid = item.tid;
    let pid = item.pid;
    let allCount = 0;
    let libMap = new Map<number, Array<number | string>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (itemData.pid !== pid || itemData.tid !== tid) {
        continue;
      }
      allCount += itemData.count;
      if (libMap.has(itemData.libId)) {
        libMap.get(itemData.libId)?.push(itemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(itemData);
        libMap.set(itemData.libId, dataArray);
      }
    }
    this.soData = [];
    libMap.forEach((arr: Array<any>, libId: number) => {
      let libCount = 0;
      let libName = arr[0].libName;
      for (let item of arr) {
        libCount += item.count;
      }
      const libData = {
        pid: item.pid,
        tid: item.tid,
        percent: ((libCount / parentCount) * 100).toFixed(2),
        countFormat: Utils.timeMsFormat2p(libCount * (SpHiPerf.stringResult?.fValue || 1)),
        count: libCount,
        tableName: libName,
        libId: libId,
      };
      this.soData.push(libData);
    });
    this.allLibCount = this.totalCountData(allCount);
    this.soData.sort((a, b) => b.count - a.count);
    this.currentLevel = 2;
    this.progressEL!.loading = false;
    this.libraryPieChart(val);
  }
  getHiperfFunction(item: any): void {
    this.progressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.perf-subheading')!.textContent = 'Statistic By Function Count';
    let parentCount = item.count;
    let tid = item.tid;
    let pid = item.pid;
    let libId = item.libId;
    let allCount = 0;
    let symbolMap = new Map<number, Array<any>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (itemData.pid !== pid || itemData.tid !== tid || itemData.libId !== libId) {
        continue;
      }
      allCount += itemData.count;
      if (symbolMap.has(itemData.symbolId)) {
        symbolMap.get(itemData.symbolId)?.push(itemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(itemData);
        symbolMap.set(itemData.symbolId, dataArray);
      }
    }
    this.functionData = [];
    symbolMap.forEach((arr, symbolId) => {
      let symbolCount = 0;
      for (let item of arr) {
        symbolCount += item.count;
      }
      let symbolName = arr[0].symbolName;
      const symbolData = {
        pid: item.pid,
        tid: item.tid,
        percent: ((symbolCount / parentCount) * 100).toFixed(2),
        countFormat: Utils.timeMsFormat2p(symbolCount * (SpHiPerf.stringResult?.fValue || 1)),
        count: symbolCount,
        tableName: symbolName,
      };
      this.functionData.push(symbolData);
    });
    this.functionData.sort((a, b) => b.count - a.count);
    this.allSymbolCount = this.totalCountData(allCount);
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
    this.tableFunction!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let functionData = evt.detail;
      if (functionData.data) {
        let data = functionData.data;
        data.isHover = true;
        if (functionData.callBack) {
          functionData.callBack(true);
        }
      }
      this.perfAnalysisPie?.showHover();
      this.perfAnalysisPie?.hideTip();
    });
    this.functionData.unshift(this.allSymbolCount);
    this.tableFunction!.recycleDataSource = this.functionData;
    this.tableFunction?.reMeauseHeight();
    // @ts-ignore
    this.functionData.shift(this.allSymbolCount);
    this.currentLevelData = this.functionData;
    this.tableFunction!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }

  private getTip() {
    return (obj: { obj: { tableName: any; countFormat: any; percent: any; }; }): string => {
      return `<div>
                    <div>Function:${obj.obj.tableName}</div>
                    <div>Weight:${obj.obj.countFormat}</div>
                    <div>Percent:${obj.obj.percent}%</div>
                </div>`;
    };
  }

  totalCountData(count: number): { countFormat: string; percent: string; count: number; allCount: number; pid: string; } {
    let allCount;
    allCount = {
      countFormat: Utils.timeMsFormat2p(count * (SpHiPerf.stringResult?.fValue || 1)),
      percent: ((count / count) * 100).toFixed(2),
      count: 0,
      allCount: count,
      pid: '',
    };
    return allCount;
  }

  getPerfPieChartData(res: any[]): unknown[] {
    if (res.length > 20) {
      let pieChartArr: string[] = [];
      let other: any = {
        tableName: 'other',
        count: 0,
        percent: 0,
        countFormat: 0,
      };
      for (let i = 0 ; i < res.length ; i++) {
        if (i < 19) {
          pieChartArr.push(res[i]);
        } else {
          other.count += res[i].count;
          other.countFormat = Utils.timeMsFormat2p(other.count * (SpHiPerf.stringResult?.fValue || 1));
          // @ts-ignore
          other.percent = ((other.count / this.sumCount) * 100).toFixed(2);
        }
      }
      pieChartArr.push(other);
      return pieChartArr;
    }
    return res;
  }
  getCallChainDataFromWorker(val: SelectionParam): void {
    this.getDataByWorker(val, (results: any) => {
      this.processData = results;
      this.getHiperfProcess(val);
    });
  }
  getDataByWorker(val: SelectionParam, handler: Function): void {
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
        </style>
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label> 
        <div style="display: flex;flex-direction: row;"class="d-box">
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
                        <lit-table-column width="1fr" title="Weight" data-index="countFormat" key="countFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-thread-usage" style="display: none;min-height: 380px"hideDownload>
                        <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Weight" data-index="countFormat" key="countFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-so-usage" style="display: none;min-height: 380px"hideDownload>
                        <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Weight" data-index="countFormat" key="countFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-function-usage" style="display: none;min-height: 380px"hideDownload>
                        <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Weight" data-index="countFormat" key="countFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    </div>

        </div>
`;
  }
}
