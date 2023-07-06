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
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar';
import { Utils } from '../../base/Utils.js';
import { procedurePool } from '../../../../database/Procedure.js';

@element('tabpane-tb-vm-statistics')
export class TabPaneIOTierStatisticsAnalysis extends BaseElement {
  private iOTierStatisticsAnalysisPie: LitChartPie | null | undefined;
  private currentSelection: SelectionParam | any;
  private processData: any;
  private pidData!: any[];
  private threadData!: any[];
  private soData!: any[];
  private functionData!: any[];
  private typeData!: any[];
  private tableProcess: LitTable | null | undefined;
  private tableThread: LitTable | null | undefined;
  private tableType: LitTable | null | undefined;
  private tableSo: LitTable | null | undefined;
  private tableFunction: LitTable | null | undefined;
  private sumDur: any;
  private range: HTMLLabelElement | null | undefined;
  private iOTierStatisticsAnalysisBack: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private processName: string = '';
  private threadName: string = '';
  private sortColumn: string = '';
  private sortType: number = 0;
  private typeName: any;
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private processStatisticsData!: any;
  private typeStatisticsData!: any;
  private threadStatisticsData!: any;
  private libStatisticsData!: any;
  private functionStatisticsData!: any;
  set data(ioTierStatisticsAnalysisSelection: SelectionParam | any) {
    if (ioTierStatisticsAnalysisSelection === this.currentSelection) {
      this.pidData.unshift(this.processStatisticsData);
      this.tableProcess!.recycleDataSource = this.pidData;
      // @ts-ignore
      this.pidData.shift(this.processStatisticsData);
      return;
    }
    this.clearData();
    this.currentSelection = ioTierStatisticsAnalysisSelection;
    this.tableProcess!.style.display = 'grid';
    this.tableType!.style.display = 'none';
    this.tableThread!.style.display = 'none';
    this.tableSo!.style.display = 'none';
    this.tableFunction!.style.display = 'none';
    this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
    this.range!.textContent =
      'Selected range: ' +
      parseFloat(
        ((ioTierStatisticsAnalysisSelection.rightNs - ioTierStatisticsAnalysisSelection.leftNs) / 1000000.0).toFixed(5)
      ) +
      ' ms';
    this.progressEL!.loading = true;
    this.getDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [{ queryFuncName: 'io', ...ioTierStatisticsAnalysisSelection }],
        },
      ],
      (results: any[]) => {
        this.getIOTierProcess(ioTierStatisticsAnalysisSelection, results);
      }
    );
  }
  initElements(): void {
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.iOTierStatisticsAnalysisPie = this.shadowRoot!.querySelector<LitChartPie>('#chart-pie');
    this.tableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.tableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.tableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.tableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.tableType = this.shadowRoot!.querySelector<LitTable>('#tb-type-usage');
    this.iOTierStatisticsAnalysisBack = this.shadowRoot!.querySelector<HTMLDivElement>('.go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.subheading');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.goBack();
  }
  clearData() {
    this.iOTierStatisticsAnalysisPie!.dataSource = [];
    this.tableProcess!.recycleDataSource = [];
    this.tableType!.recycleDataSource = [];
    this.tableThread!.recycleDataSource = [];
    this.tableSo!.recycleDataSource = [];
    this.tableFunction!.recycleDataSource = [];
  }
  goBack() {
    this.iOTierStatisticsAnalysisBack!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By type AllDuration') {
        this.tableProcess!.style.display = 'grid';
        this.tableType!.style.display = 'none';
        this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
        this.tableType!.setAttribute('hideDownload', '');
        this.tableProcess?.removeAttribute('hideDownload');
        this.currentLevel = 0;
        this.processPieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Thread AllDuration') {
        this.tableType!.style.display = 'grid';
        this.tableThread!.style.display = 'none';
        this.tableThread!.setAttribute('hideDownload', '');
        this.tableType?.removeAttribute('hideDownload');
        this.currentLevel = 1;
        this.typePieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Library AllDuration') {
        this.tableThread!.style.display = 'grid';
        this.tableSo!.style.display = 'none';
        this.tableSo!.setAttribute('hideDownload', '');
        this.tableThread?.removeAttribute('hideDownload');
        this.currentLevel = 2;
        this.threadPieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Function AllDuration') {
        this.tableSo!.style.display = 'grid';
        this.tableFunction!.style.display = 'none';
        this.tableFunction!.setAttribute('hideDownload', '');
        this.tableSo?.removeAttribute('hideDownload');
        this.currentLevel = 3;
        this.libraryPieChart(this.currentSelection);
      }
    });
  }
  processPieChart(val: any) {
    this.sumDur = this.processStatisticsData.allDuration;
    this.iOTierStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.pidData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                <div>ProcessName:${obj.obj.tableName}</div>
                                <div>Duration:${obj.obj.durFormat}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                            </div>
                                `;
      },
      angleClick: (it) => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.ioTierProcessLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.tableProcess!.setCurrentHover(data);
        } else {
          this.tableProcess!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.tableProcess!.addEventListener('row-hover', (event: any) => {
      if (event.detail.data) {
        let processData = event.detail.data;
        processData.isHover = true;
        if ((event.detail as any).callBack) {
          (event.detail as any).callBack(true);
        }
      }
      this.iOTierStatisticsAnalysisPie?.showHover();
      this.iOTierStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process AllDuration';
    this.pidData.unshift(this.processStatisticsData);
    this.tableProcess!.recycleDataSource = this.pidData;
    // @ts-ignore
    this.pidData.shift(this.processStatisticsData);
    this.currentLevelData = this.pidData;
    this.tableProcess?.reMeauseHeight();
    this.tableProcess!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tableProcess!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.ioTierProcessLevelClickEvent(data, val);
      }
    });
  }
  ioTierProcessLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.iOTierStatisticsAnalysisBack!.style.visibility = 'visible';
    this.tableProcess!.style.display = 'none';
    this.tableType!.style.display = 'grid';
    this.tableProcess!.setAttribute('hideDownload', '');
    this.tableType?.removeAttribute('hideDownload');
    this.getIOTierType(it, val);
    this.processName = it.tableName;
    this.iOTierStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName;
  }
  typePieChart(val: any) {
    this.iOTierStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.typeData,
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                <div>Type:${obj.obj.tableName}</div>
                                <div>Duration:${obj.obj.durFormat}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                            </div>
                                `;
      },
      angleClick: (it) => {
        this.ioTierTypeLevelClickEvent(it, val);
      },
      hoverHandler: (data) => {
        if (data) {
          this.tableType!.setCurrentHover(data);
        } else {
          this.tableType!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.tableType!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let tableData = evt.detail.data;
        tableData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.iOTierStatisticsAnalysisPie?.showHover();
      this.iOTierStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName;
    this.tabName!.textContent = 'Statistic By type AllDuration';
    this.typeData.unshift(this.typeStatisticsData);
    this.tableType!.recycleDataSource = this.typeData;
    // @ts-ignore
    this.typeData.shift(this.typeStatisticsData);
    this.currentLevelData = this.typeData;
    this.tableType?.reMeauseHeight();
    this.tableType!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tableType!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.ioTierTypeLevelClickEvent(data, val);
      }
    });
  }
  ioTierTypeLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.tableType!.style.display = 'none';
    this.tableThread!.style.display = 'grid';
    this.tableType!.setAttribute('hideDownload', '');
    this.tableThread?.removeAttribute('hideDownload');
    this.getIOTierThread(it, val);
    this.typeName = it.tableName;
    this.iOTierStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName + ' / ' + this.typeName;
  }
  threadPieChart(val: any) {
    this.sumDur = this.threadStatisticsData.allDuration;
    this.iOTierStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.threadData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                <div>ThreadName:${obj.obj.tableName}</div>
                                <div>Duration:${obj.obj.durFormat}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                            </div>
                                `;
      },
      angleClick: (it) => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.ioTierThreadLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.tableThread!.setCurrentHover(data);
        } else {
          this.tableThread!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.tableThread!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let threadData = evt.detail.data;
        threadData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.iOTierStatisticsAnalysisPie?.showHover();
      this.iOTierStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName + ' / ' + this.typeName;
    this.tabName!.textContent = 'Statistic By Thread AllDuration';
    this.threadData.unshift(this.threadStatisticsData);
    this.tableThread!.recycleDataSource = this.threadData;
    // @ts-ignore
    this.threadData.shift(this.threadStatisticsData);
    this.currentLevelData = this.threadData;
    this.tableThread?.reMeauseHeight();
    this.tableThread!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tableThread!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.ioTierThreadLevelClickEvent(data, val);
      }
    });
  }
  ioTierThreadLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.iOTierStatisticsAnalysisBack!.style.visibility = 'visible';
    this.tableThread!.style.display = 'none';
    this.tableSo!.style.display = 'grid';
    this.tableThread!.setAttribute('hideDownload', '');
    this.tableSo?.removeAttribute('hideDownload');
    this.getIOTierSo(it, val);
    this.threadName = it.tableName;
    this.iOTierStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName;
  }
  libraryPieChart(val: any) {
    this.sumDur = this.libStatisticsData.allDuration;
    this.iOTierStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.soData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                <div>Library:${obj.obj.tableName}</div>
                                <div>Duration:${obj.obj.durFormat}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                            </div>
                                `;
      },
      angleClick: (it) => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.ioTierSoLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.tableSo!.setCurrentHover(data);
        } else {
          this.tableSo!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.tableSo!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let soData = evt.detail.data;
        soData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.iOTierStatisticsAnalysisPie?.showHover();
      this.iOTierStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName;
    this.tabName!.textContent = 'Statistic By Library AllDuration';
    this.soData.unshift(this.libStatisticsData);
    this.tableSo!.recycleDataSource = this.soData;
    // @ts-ignore
    this.soData.shift(this.libStatisticsData);
    this.currentLevelData = this.soData;
    this.tableSo?.reMeauseHeight();
    this.tableSo!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tableSo!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.ioTierSoLevelClickEvent(data, val);
      }
    });
  }
  ioTierSoLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.iOTierStatisticsAnalysisBack!.style.visibility = 'visible';
    this.tableSo!.style.display = 'none';
    this.tableFunction!.style.display = 'grid';
    this.tableSo!.setAttribute('hideDownload', '');
    this.tableFunction?.removeAttribute('hideDownload');
    this.getIOTierFunction(it, val);
    this.iOTierStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName + ' / ' + it.tableName;
  }
  sortByColumn(column: string, ioSort: number) {
    this.sortColumn = column;
    this.sortType = ioSort;
    let currentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        currentTable = this.tableProcess;
        break;
      case 1:
        currentTable = this.tableType;
        break;
      case 2:
        currentTable = this.tableThread;
        break;
      case 3:
        currentTable = this.tableSo;
        break;
      case 4:
        currentTable = this.tableFunction;
        break;
    }
    if (!currentTable) {
      return;
    }
    if (ioSort === 0) {
      let ioArr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          ioArr.unshift(this.processStatisticsData);
          break;
        case 1:
          ioArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          ioArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          ioArr.unshift(this.libStatisticsData);
          break;
        case 4:
          ioArr.unshift(this.functionStatisticsData);
          break;
      }
      currentTable!.recycleDataSource = ioArr;
    } else {
      let ioArr = [...this.currentLevelData];
      if (column === 'tableName') {
        currentTable!.recycleDataSource = ioArr.sort((a, b) => {
          if (ioSort === 1) {
            if (a.tableName > b.tableName) {
              return 1;
            } else if (a.tableName === b.tableName) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (b.tableName > a.tableName) {
              return 1;
            } else if (a.tableName === b.tableName) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (column === 'durFormat' || column === 'percent') {
        currentTable!.recycleDataSource = ioArr.sort((a, b) => {
          return ioSort === 1 ? a.duration - b.duration : b.duration - a.duration;
        });
      }
      switch (this.currentLevel) {
        case 0:
          ioArr.unshift(this.processStatisticsData);
          break;
        case 1:
          ioArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          ioArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          ioArr.unshift(this.libStatisticsData);
          break;
        case 4:
          ioArr.unshift(this.functionStatisticsData);
          break;
      }
      currentTable!.recycleDataSource = ioArr;
    }
  }
  getIOTierProcess(val: any, result: Array<any>) {
    this.processData = JSON.parse(JSON.stringify(result));
    if (!this.processData || this.processData.length === 0) {
      this.pidData = [];
      this.processStatisticsData = [];
      this.processPieChart(val);
      return;
    }
    let allDur = 0;
    let ioMap = new Map<string, Array<any>>();
    for (let itemData of result) {
      allDur += itemData.dur;
      if (ioMap.has(itemData.pid)) {
        ioMap.get(itemData.pid)?.push(itemData);
      } else {
        let itemArray = new Array<any>();
        itemArray.push(itemData);
        ioMap.set(itemData.pid, itemArray);
      }
    }
    this.pidData = [];
    ioMap.forEach((value: Array<any>, key: string) => {
      let dur = 0;
      let pName = '';
      for (let item of value) {
        pName = item.processName =
          item.processName === null || item.processName === undefined
            ? `Process(${item.pid})`
            : `${item.processName}(${item.pid})`;
        dur += item.dur;
      }
      const pidData = {
        tableName: pName,
        pid: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.pidData.push(pidData);
    });
    this.pidData.sort((a, b) => b.duration - a.duration);
    this.processStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 0;
    this.progressEL!.loading = false;
    this.processPieChart(val);
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        this.tableProcess!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.tableThread!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.tableSo!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.tableFunction!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.tableType!.style.height = this.parentElement!.clientHeight - 40 + 'px';
      }
    }).observe(this.parentElement!);
  }
  getIOTierType(item: any, val: any) {
    this.progressEL!.loading = true;
    let typeMap = new Map<string, Array<any>>();
    let pid = item.pid;
    let allDur = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let processItem of this.processData) {
      if (processItem.pid !== pid) {
        continue;
      }
      allDur += processItem.dur;
      if (typeMap.has(processItem.type)) {
        typeMap.get(processItem.type)?.push(processItem);
      } else {
        let itemArray = new Array<any>();
        itemArray.push(processItem);
        typeMap.set(processItem.type, itemArray);
      }
    }
    this.typeData = [];
    typeMap.forEach((value: Array<any>, key: string) => {
      let dur = 0;
      for (let item of value) {
        dur += item.dur;
      }
      const typeData = {
        tableName: this.typeIdToString(key),
        pid: item.pid,
        type: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.typeData.push(typeData);
    });
    this.typeData.sort((a, b) => b.duration - a.duration);
    this.typeStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 1;
    this.typePieChart(val);
    this.progressEL!.loading = false;
  }
  getIOTierThread(item: any, val: any) {
    this.progressEL!.loading = true;
    let threadMap = new Map<string, Array<any>>();
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (itemData.pid !== pid || itemData.type !== type) {
        continue;
      }
      allDur += itemData.dur;
      if (threadMap.has(itemData.tid)) {
        threadMap.get(itemData.tid)?.push(itemData);
      } else {
        let itemArray = new Array<any>();
        itemArray.push(itemData);
        threadMap.set(itemData.tid, itemArray);
      }
    }
    this.threadData = [];
    threadMap.forEach((value: Array<any>, key: string) => {
      let dur = 0;
      let tName = '';
      for (let item of value) {
        dur += item.dur;
        tName = item.threadName =
          item.threadName === null || item.threadName === undefined ? `Thread(${item.tid})` : `${item.threadName}`;
      }
      const threadData = {
        tableName: tName,
        pid: item.pid,
        type: item.type,
        tid: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.threadData.push(threadData);
    });
    this.threadData.sort((a, b) => b.duration - a.duration);
    this.threadStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 2;
    this.progressEL!.loading = false;
    this.threadPieChart(val);
  }
  getIOTierSo(item: any, val: any) {
    this.progressEL!.loading = true;
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    let libMap = new Map<number, Array<any>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let processItemData of this.processData) {
      if (processItemData.pid !== pid || processItemData.tid !== tid || processItemData.type !== type) {
        continue;
      }
      allDur += processItemData.dur;
      if (libMap.has(processItemData.libId)) {
        libMap.get(processItemData.libId)?.push(processItemData);
      } else {
        let dataArray = new Array<any>();
        dataArray.push(processItemData);
        libMap.set(processItemData.libId, dataArray);
      }
    }
    this.soData = [];
    libMap.forEach((value: any[], key: number) => {
      let dur = 0;
      let libName = '';
      for (let item of value) {
        dur += item.dur;
        if (key === null) {
          item.libName = 'unkown';
        }
        libName = item.libName;
      }
      let libPath = libName?.split('/');
      if (libPath) {
        libName = libPath[libPath.length - 1];
      }
      const soData = {
        tableName: libName,
        pid: item.pid,
        type: item.type,
        tid: item.tid,
        libId: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.soData.push(soData);
    });
    this.soData.sort((a, b) => b.duration - a.duration);
    this.libStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 3;
    this.progressEL!.loading = false;
    this.libraryPieChart(val);
  }
  getIOTierFunction(item: any, val: any) {
    this.progressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.subheading')!.textContent = 'Statistic By Function AllDuration';
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let libId = item.libId;
    let allDur = 0;
    let symbolMap = new Map<number, Array<any>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let processData of this.processData) {
      if (
        processData.pid !== pid ||
        processData.tid !== tid ||
        processData.type !== type ||
        processData.libId !== libId
      ) {
        continue;
      }
      allDur += processData.dur;
      if (symbolMap.has(processData.symbolId)) {
        symbolMap.get(processData.symbolId)?.push(processData);
      } else {
        let dataArray = new Array<any>();
        dataArray.push(processData);
        symbolMap.set(processData.symbolId, dataArray);
      }
    }
    this.functionData = [];
    symbolMap.forEach((symbolItems, key) => {
      let dur = 0;
      let funSymbolName = '';
      for (let symbolItem of symbolItems) {
        funSymbolName = symbolItem.symbolName;
        dur += symbolItem.dur;
      }
      let symbolPath = funSymbolName?.split('/');
      if (symbolPath) {
        funSymbolName = symbolPath[symbolPath.length - 1];
      }
      const symbolData = {
        pid: item.pid,
        tid: item.tid,
        percent: ((dur / allDur) * 100).toFixed(2),
        tableName: funSymbolName,
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.functionData.push(symbolData);
    });
    this.functionData.sort((a, b) => b.duration - a.duration);
    this.functionStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 4;
    this.progressEL!.loading = false;
    this.sumDur = this.functionStatisticsData.allDuration;
    this.iOTierStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.functionData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                    <div>Function:${obj.obj.tableName}</div>
                                    <div>Duration:${obj.obj.durFormat}</div>
                                    <div>percent:${obj.obj.percent}</div>
                                        </div>
                                                `;
      },
      hoverHandler: (data) => {
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
    this.tableFunction!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let funData = evt.detail.data;
        funData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.iOTierStatisticsAnalysisPie?.showHover();
      this.iOTierStatisticsAnalysisPie?.hideTip();
    });
    this.functionData.unshift(this.functionStatisticsData);
    this.tableFunction!.recycleDataSource = this.functionData;
    this.tableFunction?.reMeauseHeight();
    // @ts-ignore
    this.functionData.shift(this.functionStatisticsData);
    this.currentLevelData = this.functionData;
    this.tableFunction!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }
  typeIdToString(type: any) {
    let releaseType: any;
    if (type === 1) {
      releaseType = 'DATA_READ';
    } else if (type === 2) {
      releaseType = 'DATA_WRITE';
    } else if (type === 3) {
      releaseType = 'METADATA_READ';
    } else if (type === 4) {
      releaseType = 'METADATA_WRITE';
    }
    return releaseType;
  }
  totalDurationData(duration: any) {
    let allDuration;
    allDuration = {
      durFormat: Utils.getProbablyTime(duration),
      percent: ((duration / duration) * 100).toFixed(2),
      tableName: '',
      duration: 0,
      allDuration: duration,
    };
    return allDuration;
  }
  getPieChartData(res: any[]) {
    if (res.length > 20) {
      let pieChartArr: any[] = [];
      let other: any = {
        tableName: 'other',
        duration: 0,
        percent: 0,
        durFormat: 0,
      };
      for (let i = 0; i < res.length; i++) {
        if (i < 19) {
          pieChartArr.push(res[i]);
        } else {
          other.duration += res[i].duration;
          other.durFormat = Utils.getProbablyTime(other.duration);
          other.percent = ((other.duration / this.sumDur) * 100).toFixed(2);
        }
      }
      pieChartArr.push(other);
      return pieChartArr;
    }
    return res;
  }

  getDataByWorker(args: any[], handler: Function) {
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-action',
      { args, callType: 'io', isAnalysis: true },
      undefined,
      (results: any) => {
        handler(results);
        this.progressEL!.loading = false;
      }
    );
  }

  initHtml(): string {
    return `
        <style>
        :host {
            display: flex;
            flex-direction: column;
        }
        #chart-pie{
            height: 300px;
        }
        .table-box{
            width: 60%;
            border-left: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
        }
        .go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
            margin-left: 20px;
            visibility: hidden;
        }
        .back-box{
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
        .subheading{
            font-weight: bold;
            text-align: center;
        }
        .progress{
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        } 
        </style>
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label> 
        <div style="display: flex;flex-direction: row;"class="d-box">
            <lit-progress-bar class="progress"></lit-progress-bar>
                     <div id="left_table" style="width: 40%;height:auto;">
                         <div style="display: flex;margin-bottom: 10px">
                           <div class="go-back">
                              <div class="back-box">
                                  <lit-icon name="arrowleft"></lit-icon>
                              </div>
                           </div>
                         <div class="title"></div>
                        </div>
                         <div class="subheading"></div>                       
                         <lit-chart-pie  id="chart-pie"></lit-chart-pie>     
                     </div>
                     <div class="table-box" style="height:auto;overflow: auto">
                    <lit-table id="tb-process-usage" style="max-height:565px;min-height: 350px">
                        <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-type-usage" style="max-height:565px;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Type" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-thread-usage" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                     <lit-table id="tb-so-usage" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-function-usage" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    </div>

        </div>
`;
  }
}
