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

@element('tabpane-virtual-memory-statistics-analysis')
export class TabPaneVirtualMemoryStatisticsAnalysis extends BaseElement {
  private vmStatisticsAnalysisPie: LitChartPie | null | undefined;
  private vmStatisticsAnalysisCurrentSelection: SelectionParam | null | undefined;
  private vmStatisticsAnalysisProcessData: any;
  private vmStatisticsAnalysisPidData!: any[];
  private vmStatisticsAnalysisThreadData!: any[];
  private vmStatisticsAnalysisSoData!: any[];
  private vmStatisticsAnalysisFunctionData!: any[];
  private vmStatisticsAnalysisTypeData!: any[];
  private vmStatisticsAnalysisTableProcess: LitTable | null | undefined;
  private vmStatisticsAnalysisTableType: LitTable | null | undefined;
  private vmStatisticsAnalysisTableThread: LitTable | null | undefined;
  private vmStatisticsAnalysisTableSo: LitTable | null | undefined;
  private vmStatisticsAnalysisTableFunction: LitTable | null | undefined;
  private sumDur: number = 0;
  private vmStatisticsAnalysisRange: HTMLLabelElement | null | undefined;
  private back: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private vmStatisticsAnalysisProgressEL: LitProgressBar | null | undefined;
  private processName: string = '';
  private typeName: string = '';
  private threadName: string = '';
  private vmStatisticsAnalysisSortColumn: string = '';
  private vmStatisticsAnalysisSortType: number = 0;
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private processStatisticsData!: {};
  private typeStatisticsData!: {};
  private threadStatisticsData!: {};
  private libStatisticsData!: {};
  private functionStatisticsData!: {};
  set data(vmStatisticsAnalysisSelection: SelectionParam) {
    if (vmStatisticsAnalysisSelection === this.vmStatisticsAnalysisCurrentSelection) {
      this.vmStatisticsAnalysisPidData.unshift(this.processStatisticsData);
      this.vmStatisticsAnalysisTableProcess!.recycleDataSource = this.vmStatisticsAnalysisPidData;
      // @ts-ignore
      this.vmStatisticsAnalysisPidData.shift(this.processStatisticsData);
      return;
    }
    this.clearData();
    this.vmStatisticsAnalysisCurrentSelection = vmStatisticsAnalysisSelection;
    this.vmStatisticsAnalysisTableProcess!.style.display = 'grid';
    this.vmStatisticsAnalysisTableType!.style.display = 'none';
    this.vmStatisticsAnalysisTableThread!.style.display = 'none';
    this.vmStatisticsAnalysisTableSo!.style.display = 'none';
    this.vmStatisticsAnalysisTableFunction!.style.display = 'none';
    this.back!.style.visibility = 'hidden';
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = '';
    this.vmStatisticsAnalysisRange!.textContent =
      'Selected range: ' +
      parseFloat(
        ((vmStatisticsAnalysisSelection.rightNs - vmStatisticsAnalysisSelection.leftNs) / 1000000.0).toFixed(5)
      ) +
      '  ms';
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    this.getVmDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [{ queryFuncName: 'virtualMemory', ...vmStatisticsAnalysisSelection }],
        },
      ],
      (results: any[]) => {
        this.getVirtualMemoryProcess(results);
      }
    );
  }
  initElements(): void {
    this.vmStatisticsAnalysisRange = this.shadowRoot?.querySelector('#time-range');
    this.vmStatisticsAnalysisPie = this.shadowRoot!.querySelector<LitChartPie>('#vm-chart-pie');
    this.vmStatisticsAnalysisTableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.vmStatisticsAnalysisTableType = this.shadowRoot!.querySelector<LitTable>('#tb-type-usage');
    this.vmStatisticsAnalysisTableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.vmStatisticsAnalysisTableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.vmStatisticsAnalysisTableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.back = this.shadowRoot!.querySelector<HTMLDivElement>('.vm-go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.vm-subheading');
    this.vmStatisticsAnalysisProgressEL = this.shadowRoot?.querySelector('.vm-progress') as LitProgressBar;
    this.goBack();
  }
  clearData(): void {
    this.vmStatisticsAnalysisPie!.dataSource = [];
    this.vmStatisticsAnalysisTableProcess!.recycleDataSource = [];
    this.vmStatisticsAnalysisTableType!.recycleDataSource = [];
    this.vmStatisticsAnalysisTableThread!.recycleDataSource = [];
    this.vmStatisticsAnalysisTableSo!.recycleDataSource = [];
    this.vmStatisticsAnalysisTableFunction!.recycleDataSource = [];
  }
  goBack(): void {
    this.back!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By type AllDuration') {
        this.vmStatisticsAnalysisTableProcess!.style.display = 'grid';
        this.vmStatisticsAnalysisTableType!.style.display = 'none';
        this.back!.style.visibility = 'hidden';
        this.vmStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
        this.vmStatisticsAnalysisTableProcess?.removeAttribute('hideDownload');
        this.currentLevel = 0;
        this.processPieChart();
      } else if (this.tabName!.textContent === 'Statistic By Thread AllDuration') {
        this.vmStatisticsAnalysisTableType!.style.display = 'grid';
        this.vmStatisticsAnalysisTableThread!.style.display = 'none';
        this.vmStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
        this.vmStatisticsAnalysisTableType?.removeAttribute('hideDownload');
        this.currentLevel = 1;
        this.typePieChart();
      } else if (this.tabName!.textContent === 'Statistic By Library AllDuration') {
        this.vmStatisticsAnalysisTableThread!.style.display = 'grid';
        this.vmStatisticsAnalysisTableSo!.style.display = 'none';
        this.vmStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
        this.vmStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
        this.currentLevel = 2;
        this.threadPieChart();
      } else if (this.tabName!.textContent === 'Statistic By Function AllDuration') {
        this.vmStatisticsAnalysisTableSo!.style.display = 'grid';
        this.vmStatisticsAnalysisTableFunction!.style.display = 'none';
        this.vmStatisticsAnalysisTableFunction!.setAttribute('hideDownload', '');
        this.vmStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
        this.currentLevel = 3;
        this.libraryPieChart();
      }
    });
  }
  processPieChart(): void {
    // @ts-ignore
    this.sumDur = this.processStatisticsData.allDuration;
    this.vmStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getVmPieChartData(this.vmStatisticsAnalysisPidData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getVmTip(),
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.vmProcessLevelClickEvent(it);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.vmStatisticsAnalysisTableProcess!.setCurrentHover(data);
        } else {
          this.vmStatisticsAnalysisTableProcess!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.vmStatisticsAnalysisTableProcess!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let vmProcessData = evt.detail;
      if (vmProcessData.data) {
        let vmsData = vmProcessData.data;
        vmsData.isHover = true;
        if (vmProcessData.callBack) {
          vmProcessData.callBack(true);
        }
      }
      this.vmStatisticsAnalysisPie?.showHover();
      this.vmStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process AllDuration';
    this.vmStatisticsAnalysisPidData.unshift(this.processStatisticsData);
    this.vmStatisticsAnalysisTableProcess!.recycleDataSource = this.vmStatisticsAnalysisPidData;
    // @ts-ignore
    this.vmStatisticsAnalysisPidData.shift(this.processStatisticsData);
    this.currentLevelData = this.vmStatisticsAnalysisPidData;
    this.vmStatisticsAnalysisTableProcess?.reMeauseHeight();
    this.vmStatisticsAnalysisTableProcess!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.vmStatisticsAnalysisTableProcess!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.vmProcessLevelClickEvent(data);
      }
    });
  }

  private getVmTip() {
    return (obj: { obj: { tableName: any; durFormat: any; percent: any; }; }): string => {
      return `<div>
                                <div>ProcessName:${obj.obj.tableName}</div>
                                <div>Duration:${obj.obj.durFormat}</div>
                                <div>Percent:${obj.obj.percent}%</div> 
                            </div>
                                `;
    };
  }

  vmProcessLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.vmStatisticsAnalysisTableProcess!.style.display = 'none';
    this.vmStatisticsAnalysisTableType!.style.display = 'grid';
    this.vmStatisticsAnalysisTableProcess!.setAttribute('hideDownload', '');
    this.vmStatisticsAnalysisTableType?.removeAttribute('hideDownload');
    this.getVirtualMemoryType(it);
    this.processName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName;
    this.vmStatisticsAnalysisPie?.hideTip();
  }
  typePieChart(): void {
    this.vmStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.vmStatisticsAnalysisTypeData,
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getVmTip(),
      angleClick: (it): void => {
        this.vmTypeLevelClickEvent(it);
      },
      hoverHandler: (data): void => {
        if (data) {
          this.vmStatisticsAnalysisTableType!.setCurrentHover(data);
        } else {
          this.vmStatisticsAnalysisTableType!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.vmStatisticsAnalysisTableType!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let vmTypeData = evt.detail;
      if (vmTypeData.data) {
        let satData = vmTypeData.data;
        satData.isHover = true;
        if (vmTypeData.callBack) {
          vmTypeData.callBack(true);
        }
      }
      this.vmStatisticsAnalysisPie?.showHover();
      this.vmStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName;
    this.tabName!.textContent = 'Statistic By type AllDuration';
    this.vmStatisticsAnalysisTypeData.unshift(this.typeStatisticsData);
    this.vmStatisticsAnalysisTableType!.recycleDataSource = this.vmStatisticsAnalysisTypeData;
    // @ts-ignore
    this.vmStatisticsAnalysisTypeData.shift(this.typeStatisticsData);
    this.currentLevelData = this.vmStatisticsAnalysisTypeData;
    this.vmStatisticsAnalysisTableType?.reMeauseHeight();
    this.vmStatisticsAnalysisTableType!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.vmStatisticsAnalysisTableType!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.vmTypeLevelClickEvent(data);
      }
    });
  }
  vmTypeLevelClickEvent(it: any): void {
    this.clearData();
    this.vmStatisticsAnalysisTableType!.style.display = 'none';
    this.vmStatisticsAnalysisTableThread!.style.display = 'grid';
    this.vmStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
    this.vmStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
    this.getVirtualMemoryThread(it);
    this.typeName = it.tableName;
    this.vmStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName + ' / ' + this.typeName;
  }
  threadPieChart(): void {
    // @ts-ignore
    this.sumDur = this.threadStatisticsData.allDuration;
    this.vmStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getVmPieChartData(this.vmStatisticsAnalysisThreadData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getVmTip(),
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.vmThreadLevelClickEvent(it);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.vmStatisticsAnalysisTableThread!.setCurrentHover(data);
        } else {
          this.vmStatisticsAnalysisTableThread!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.vmStatisticsAnalysisTableThread!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let vmThreadData = evt.detail;
      if (vmThreadData.data) {
        let threadData = vmThreadData.data;
        threadData.isHover = true;
        if (vmThreadData.callBack) {
          vmThreadData.callBack(true);
        }
      }
      this.vmStatisticsAnalysisPie?.showHover();
      this.vmStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.processName + ' / ' + this.typeName;
    this.tabName!.textContent = 'Statistic By Thread AllDuration';
    this.vmStatisticsAnalysisThreadData.unshift(this.threadStatisticsData);
    this.vmStatisticsAnalysisTableThread!.recycleDataSource = this.vmStatisticsAnalysisThreadData;
    // @ts-ignore
    this.vmStatisticsAnalysisThreadData.shift(this.threadStatisticsData);
    this.currentLevelData = this.vmStatisticsAnalysisThreadData;
    this.vmStatisticsAnalysisTableThread?.reMeauseHeight();
    this.vmStatisticsAnalysisTableThread!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.vmStatisticsAnalysisTableThread!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.vmThreadLevelClickEvent(data);
      }
    });
  }

  vmThreadLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.vmStatisticsAnalysisTableThread!.style.display = 'none';
    this.vmStatisticsAnalysisTableSo!.style.display = 'grid';
    this.vmStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
    this.vmStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
    this.getVirtualMemorySo(it);
    this.threadName = it.tableName;
    this.vmStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName;
  }
  libraryPieChart(): void {
    // @ts-ignore
    this.sumDur = this.libStatisticsData.allDuration;
    this.vmStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getVmPieChartData(this.vmStatisticsAnalysisSoData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (vmLibraryObj): string => {
        return `<div>
                                <div>Library:${ vmLibraryObj.obj.tableName }</div>
                                <div>Duration:${ vmLibraryObj.obj.durFormat }</div>
                                <div>percent:${ vmLibraryObj.obj.percent }%</div> 
                            </div>
                                `;
      },
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.vmSoLevelClickEvent(it);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.vmStatisticsAnalysisTableSo!.setCurrentHover(data);
        } else {
          this.vmStatisticsAnalysisTableSo!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.vmStatisticsAnalysisTableSo!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let vmSoData = evt.detail;
      if (vmSoData.data) {
        let soData = vmSoData.data;
        soData.isHover = true;
        if (vmSoData.callBack) {
          vmSoData.callBack(true);
        }
      }
      this.vmStatisticsAnalysisPie?.showHover();
      this.vmStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName;
    this.tabName!.textContent = 'Statistic By Library AllDuration';
    this.vmStatisticsAnalysisSoData.unshift(this.libStatisticsData);
    this.vmStatisticsAnalysisTableSo!.recycleDataSource = this.vmStatisticsAnalysisSoData;
    // @ts-ignore
    this.vmStatisticsAnalysisSoData.shift(this.libStatisticsData);
    this.currentLevelData = this.vmStatisticsAnalysisSoData;
    this.vmStatisticsAnalysisTableSo?.reMeauseHeight();
    this.vmStatisticsAnalysisTableSo!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.vmStatisticsAnalysisTableSo!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.vmSoLevelClickEvent(data);
      }
    });
  }
  vmSoLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.vmStatisticsAnalysisTableSo!.style.display = 'none';
    this.vmStatisticsAnalysisTableFunction!.style.display = 'grid';
    this.vmStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
    this.vmStatisticsAnalysisTableFunction?.removeAttribute('hideDownload');
    this.getVirtualMemoryFunction(it);
    this.vmStatisticsAnalysisPie?.hideTip();
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.processName + ' / ' + this.typeName + ' / ' + this.threadName + ' / ' + it.tableName;
  }
  sortByColumn(column: string, vmsSort: number): void {
    this.vmStatisticsAnalysisSortColumn = column;
    this.vmStatisticsAnalysisSortType = vmsSort;
    let vmsCurrentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        vmsCurrentTable = this.vmStatisticsAnalysisTableProcess;
        break;
      case 1:
        vmsCurrentTable = this.vmStatisticsAnalysisTableType;
        break;
      case 2:
        vmsCurrentTable = this.vmStatisticsAnalysisTableThread;
        break;
      case 3:
        vmsCurrentTable = this.vmStatisticsAnalysisTableSo;
        break;
      case 4:
        vmsCurrentTable = this.vmStatisticsAnalysisTableFunction;
        break;
    }
    if (!vmsCurrentTable) {
      return;
    }
    if (vmsSort === 0) {
      let vmsArr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          vmsArr.unshift(this.processStatisticsData);
          break;
        case 1:
          vmsArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          vmsArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          vmsArr.unshift(this.libStatisticsData);
          break;
        case 4:
          vmsArr.unshift(this.functionStatisticsData);
          break;
      }
      vmsCurrentTable!.recycleDataSource = vmsArr;
    } else {
      let vmsArray = [...this.currentLevelData];
      if (column === 'tableName') {
        vmsCurrentTable!.recycleDataSource = vmsArray.sort((firstVMElement, secondVMElement) => {
          if (vmsSort === 1) {
            if (firstVMElement.tableName > secondVMElement.tableName) {
              return 1;
            } else if (firstVMElement.tableName === secondVMElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (secondVMElement.tableName > firstVMElement.tableName) {
              return 1;
            } else if (firstVMElement.tableName === secondVMElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (column === 'durFormat' || column === 'percent') {
        vmsCurrentTable!.recycleDataSource = vmsArray.sort((a, b) => {
          return vmsSort === 1 ? a.duration - b.duration : b.duration - a.duration;
        });
      }
      switch (this.currentLevel) {
        case 0:
          vmsArray.unshift(this.processStatisticsData);
          break;
        case 1:
          vmsArray.unshift(this.typeStatisticsData);
          break;
        case 2:
          vmsArray.unshift(this.threadStatisticsData);
          break;
        case 3:
          vmsArray.unshift(this.libStatisticsData);
          break;
        case 4:
          vmsArray.unshift(this.functionStatisticsData);
          break;
      }
      vmsCurrentTable!.recycleDataSource = vmsArray;
    }
  }
  getVirtualMemoryProcess(result: Array<any>): void {
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    this.vmStatisticsAnalysisProcessData = JSON.parse(JSON.stringify(result));
    if (!this.vmStatisticsAnalysisProcessData || this.vmStatisticsAnalysisProcessData.length === 0) {
      this.vmStatisticsAnalysisPidData = [];
      this.processStatisticsData = [];
      this.processPieChart();
      return;
    }
    let allDur = 0;
    let vmMap = new Map<string, Array<number | string>>();
    for (let itemData of result) {
      allDur += itemData.dur;
      if (vmMap.has(itemData.pid)) {
        vmMap.get(itemData.pid)?.push(itemData);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(itemData);
        vmMap.set(itemData.pid, itemArray);
      }
    }
    this.vmStatisticsAnalysisPidData = [];
    vmMap.forEach((value: Array<any>, key: string) => {
      let vmPidDataDur = 0;
      let pName = '';
      for (let item of value) {
        pName = item.processName =
          item.processName === null || item.processName === undefined
            ? `Process(${ item.pid })`
            : `${ item.processName }(${ item.pid })`;
        vmPidDataDur += item.dur;
      }
      this.vmStatisticsAnalysisPidData.push({
        tableName: pName,
        pid: key,
        percent: ((vmPidDataDur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(vmPidDataDur),
        duration: vmPidDataDur,
      });
    });
    this.vmStatisticsAnalysisPidData.sort((a, b) => b.duration - a.duration);
    this.processStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 0;
    this.vmStatisticsAnalysisProgressEL!.loading = false;
    this.processPieChart();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        this.vmStatisticsAnalysisTableProcess!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.vmStatisticsAnalysisTableProcess?.reMeauseHeight();
        this.vmStatisticsAnalysisTableType!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.vmStatisticsAnalysisTableType?.reMeauseHeight();
        this.vmStatisticsAnalysisTableThread!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.vmStatisticsAnalysisTableThread?.reMeauseHeight();
        this.vmStatisticsAnalysisTableSo!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.vmStatisticsAnalysisTableSo?.reMeauseHeight();
        this.vmStatisticsAnalysisTableFunction!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.vmStatisticsAnalysisTableFunction?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }
  getVirtualMemoryType(item: any): void {
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    let vmTypeMap = new Map<number, Array<number | string>>();
    let pid = item.pid;
    let allDur = 0;
    if (!this.vmStatisticsAnalysisProcessData || this.vmStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let vmsItem of this.vmStatisticsAnalysisProcessData) {
      if (vmsItem.pid !== pid) {
        continue;
      }
      allDur += vmsItem.dur;
      if (vmTypeMap.has(vmsItem.type)) {
        vmTypeMap.get(vmsItem.type)?.push(vmsItem);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(vmsItem);
        vmTypeMap.set(vmsItem.type, itemArray);
      }
    }
    this.vmStatisticsAnalysisTypeData = [];
    vmTypeMap.forEach((value: Array<any>, key: number) => {
      let dur = 0;
      for (let vmItem of value) {
        dur += vmItem.dur;
      }
      const vmTypeData = {
        tableName: this.typeIdToString(key),
        pid: item.pid,
        type: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.vmStatisticsAnalysisTypeData.push(vmTypeData);
    });
    this.vmStatisticsAnalysisTypeData.sort((a, b) => b.duration - a.duration);
    this.typeStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 1;
    this.typePieChart();
    this.vmStatisticsAnalysisProgressEL!.loading = false;
  }
  getVirtualMemoryThread(item: any): void {
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    let threadMap = new Map<string, Array<number | string>>();
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    if (!this.vmStatisticsAnalysisProcessData || this.vmStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let vmapItem of this.vmStatisticsAnalysisProcessData) {
      if (vmapItem.pid !== pid || vmapItem.type !== type) {
        continue;
      }
      allDur += vmapItem.dur;
      if (threadMap.has(vmapItem.tid)) {
        threadMap.get(vmapItem.tid)?.push(vmapItem);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(vmapItem);
        threadMap.set(vmapItem.tid, itemArray);
      }
    }
    this.vmStatisticsAnalysisThreadData = [];
    threadMap.forEach((value: Array<any>, key: string) => {
      let vmThreadDur = 0;
      let tName = '';
      for (let item of value) {
        vmThreadDur += item.dur;
        tName = item.threadName =
          item.threadName === null || item.threadName === undefined ? `Thread(${ item.tid })` : `${ item.threadName }`;
      }
      const threadData = {
        tableName: tName,
        pid: item.pid,
        type: item.type,
        tid: key,
        percent: ((vmThreadDur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(vmThreadDur),
        duration: vmThreadDur,
      };
      this.vmStatisticsAnalysisThreadData.push(threadData);
    });
    this.vmStatisticsAnalysisThreadData.sort((a, b) => b.duration - a.duration);
    this.threadStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 2;
    this.vmStatisticsAnalysisProgressEL!.loading = false;
    this.threadPieChart();
  }
  getVirtualMemorySo(item: any): void {
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    let libMap = new Map<number, Array<number | string>>();
    if (!this.vmStatisticsAnalysisProcessData || this.vmStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let vmItemData of this.vmStatisticsAnalysisProcessData) {
      if (vmItemData.pid !== pid || vmItemData.tid !== tid || vmItemData.type !== type) {
        continue;
      }
      allDur += vmItemData.dur;
      if (libMap.has(vmItemData.libId)) {
        libMap.get(vmItemData.libId)?.push(vmItemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(vmItemData);
        libMap.set(vmItemData.libId, dataArray);
      }
    }
    this.vmStatisticsAnalysisSoData = [];
    libMap.forEach((value: any[], key: number) => {
      let dur = 0;
      let vmLibName = '';
      for (let item of value) {
        dur += item.dur;
        if (key === null) {
          item.libName = 'unknown';
        }
        vmLibName = item.libName;
      }
      let libPath = vmLibName?.split('/');
      if (libPath) {
        vmLibName = libPath[libPath.length - 1];
      }
      const soData = {
        tableName: vmLibName,
        pid: item.pid,
        type: item.type,
        tid: item.tid,
        libId: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.vmStatisticsAnalysisSoData.push(soData);
    });
    this.vmStatisticsAnalysisSoData.sort((a, b) => b.duration - a.duration);
    this.libStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 3;
    this.vmStatisticsAnalysisProgressEL!.loading = false;
    this.libraryPieChart();
  }

  getVirtualMemoryFunction(item: any): void {
    this.vmStatisticsAnalysisProgressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.vm-subheading')!.textContent = 'Statistic By Function AllDuration';
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let libId = item.libId;
    let allDur = 0;
    let symbolMap = new Map<number, Array<any>>();
    if (!this.vmStatisticsAnalysisProcessData || this.vmStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let vmProcessData of this.vmStatisticsAnalysisProcessData) {
      if (
        vmProcessData.pid !== pid ||
        vmProcessData.tid !== tid ||
        vmProcessData.type !== type ||
        vmProcessData.libId !== libId
      ) {
        continue;
      }
      allDur += vmProcessData.dur;
      if (symbolMap.has(vmProcessData.symbolId)) {
        symbolMap.get(vmProcessData.symbolId)?.push(vmProcessData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(vmProcessData);
        symbolMap.set(vmProcessData.symbolId, dataArray);
      }
    }
    this.vmStatisticsAnalysisFunctionData = [];
    symbolMap.forEach((symbolItems, key) => {
      let dur = 0;
      let symbolName = '';
      for (let symbolItem of symbolItems) {
        symbolName = symbolItem.symbolName;
        dur += symbolItem.dur;
      }
      let symbolPath = symbolName?.split('/');
      if (symbolPath) {
        symbolName = symbolPath[symbolPath.length - 1];
      }
      const symbolData = {
        pid: item.pid,
        tid: item.tid,
        percent: ((dur / allDur) * 100).toFixed(2),
        tableName: symbolName,
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.vmStatisticsAnalysisFunctionData.push(symbolData);
    });
    this.vmStatisticsAnalysisFunctionData.sort((a, b) => b.duration - a.duration);
    this.functionStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 4;
    // @ts-ignore
    this.sumDur = this.libStatisticsData.allDuration;
    this.vmStatisticsAnalysisProgressEL!.loading = false;
    this.vmStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getVmPieChartData(this.vmStatisticsAnalysisFunctionData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (vmObj): string => {
        return `<div>
                                    <div>Function:${ vmObj.obj.tableName }</div>
                                    <div>Duration:${ vmObj.obj.durFormat }</div>
                                    <div>percent:${ vmObj.obj.percent }</div>
                                        </div>
                                                `;
      },
      hoverHandler: (vmPieData): void => {
        if (vmPieData) {
          this.vmStatisticsAnalysisTableFunction!.setCurrentHover(vmPieData);
        } else {
          this.vmStatisticsAnalysisTableFunction!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.vmStatisticsAnalysisTableFunction!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let vmFunctionData = evt.detail;
      if (vmFunctionData.data) {
        let data = vmFunctionData.data;
        data.isHover = true;
        if (vmFunctionData.callBack) {
          vmFunctionData.callBack(true);
        }
      }
      this.vmStatisticsAnalysisPie?.showHover();
      this.vmStatisticsAnalysisPie?.hideTip();
    });
    this.vmStatisticsAnalysisFunctionData.unshift(this.functionStatisticsData);
    this.vmStatisticsAnalysisTableFunction!.recycleDataSource = this.vmStatisticsAnalysisFunctionData;
    this.vmStatisticsAnalysisTableFunction?.reMeauseHeight();
    // @ts-ignore
    this.vmStatisticsAnalysisFunctionData.shift(this.functionStatisticsData);
    this.currentLevelData = this.vmStatisticsAnalysisFunctionData;
    this.vmStatisticsAnalysisTableFunction!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }
  typeIdToString(type: number): string {
    let vmReleaseType: string;
    if (type === 1) {
      vmReleaseType = 'File Backed In';
    } else if (type === 7) {
      vmReleaseType = 'Copy On Writer';
    }
    // @ts-ignore
    return vmReleaseType;
  }
  totalDurationData(duration: number): { durFormat: string; percent: string; tableName: string; duration: number; } {
    return {
      durFormat: Utils.getProbablyTime(duration),
      percent: ((duration / duration) * 100).toFixed(2),
      tableName: '',
      duration: 0,
    };
  }
  getVmPieChartData(vmRes: any[]): unknown[] {
    if (vmRes.length > 20) {
      let vmPieChartArr: string[] = [];
      let other: any = {
        tableName: 'other',
        duration: 0,
        percent: 0,
        durFormat: 0,
      };
      for (let i = 0 ; i < vmRes.length ; i++) {
        if (i < 19) {
          vmPieChartArr.push(vmRes[i]);
        } else {
          other.duration += vmRes[i].duration;
          other.durFormat = Utils.getProbablyTime(other.duration);
          other.percent = ((other.duration / this.sumDur) * 100).toFixed(2);
        }
      }
      vmPieChartArr.push(other);
      return vmPieChartArr;
    }
    return vmRes;
  }

  getVmDataByWorker(args: any[], handler: Function): void {
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-action',
      { args, callType: 'virtualMemory', isAnalysis: true },
      undefined,
      (results: any) => {
        handler(results);
        this.vmStatisticsAnalysisProgressEL!.loading = false;
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
        #vm-chart-pie{
            height: 300px;
        }
        .vm-table-box{
            width: 60%;
            border-left: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
        }
        .vm-go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
            margin-left: 20px;
            visibility: hidden;
        }
        .vm-back-box{
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
        .vm-subheading{
            font-weight: bold;
            text-align: center;
        }
        .vm-progress{
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        } 
        </style>
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label> 
        <div style="display: flex;flex-direction: row;"class="d-box">
            <lit-progress-bar class="vm-progress"></lit-progress-bar>
                     <div id="left_table" style="width: 40%;height:auto;">
                         <div style="display: flex;margin-bottom: 10px">
                           <div class="vm-go-back">
                              <div class="vm-back-box">
                                  <lit-icon name="arrowleft"></lit-icon>
                              </div>
                           </div>
                         <div class="title"></div>
                        </div>
                         <div class="vm-subheading"></div>                     
                         <lit-chart-pie  id="vm-chart-pie"></lit-chart-pie>     
                     </div>
                     <div class="vm-table-box" style="height:auto;overflow: auto">
                    <lit-table id="tb-process-usage" style="max-height:565px;min-height: 350px">
                        <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-type-usage" class="vm-analysis" style="max-height:565px;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Type" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-thread-usage" class="vm-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                     <lit-table id="tb-so-usage" class="vm-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-function-usage" class="vm-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    </div>

        </div>
`;
  }
}
