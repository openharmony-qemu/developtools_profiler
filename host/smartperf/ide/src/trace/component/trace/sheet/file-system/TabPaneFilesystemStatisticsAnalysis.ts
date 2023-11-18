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
import { LitChartPie } from '../../../../../base-ui/chart/pie/LitChartPie.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import '../../../../../base-ui/chart/pie/LitChartPie.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar';
import { Utils } from '../../base/Utils.js';
import { procedurePool } from '../../../../database/Procedure.js';

@element('tabpane-file-statistics-analysis')
export class TabPaneFilesystemStatisticsAnalysis extends BaseElement {
  private fileStatisticsAnalysisPie: LitChartPie | null | undefined;
  private fileStatisticsAnalysisCurrentSelection: SelectionParam | null | undefined;
  private fileStatisticsAnalysisProcessData: any;
  private fileStatisticsAnalysisThreadData!: any[];
  private fileStatisticsAnalysisSoData!: any[];
  private fileStatisticsAnalysisPidData!: any[];
  private fileStatisticsAnalysisTypeData!: any[];
  private fileStatisticsAnalysisFunctionData!: any[];
  private fileStatisticsAnalysisTableProcess: LitTable | null | undefined;
  private fileStatisticsAnalysisTableType: LitTable | null | undefined;
  private fileStatisticsAnalysisTableThread: LitTable | null | undefined;
  private fileStatisticsAnalysisTableSo: LitTable | null | undefined;
  private fileStatisticsAnalysisTableFunction: LitTable | null | undefined;
  private sumDur: number = 0;
  private fileStatisticsAnalysisRange: HTMLLabelElement | null | undefined;
  private back: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private fileStatisticsAnalysisProgressEL: LitProgressBar | null | undefined;
  private fileStatisticsAnalysisProcessName: string = '';
  private fileStatisticsAnalysisThreadName: string = '';
  private fileStatisticsAnalysisSortColumn: string = '';
  private fileStatisticsAnalysisSortType: number = 0;
  private typeName: string = '';
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private processStatisticsData!: {};
  private typeStatisticsData!: {};
  private threadStatisticsData!: {};
  private libStatisticsData!: {};
  private functionStatisticsData!: {};
  set data(val: SelectionParam) {
    if (val === this.fileStatisticsAnalysisCurrentSelection) {
      this.fileStatisticsAnalysisPidData.unshift(this.processStatisticsData);
      this.fileStatisticsAnalysisTableProcess!.recycleDataSource = this.fileStatisticsAnalysisPidData;
      // @ts-ignore
      this.fileStatisticsAnalysisPidData.shift(this.processStatisticsData);
      return;
    }
    this.clearData();
    this.fileStatisticsAnalysisCurrentSelection = val;
    this.fileStatisticsAnalysisTableProcess!.style.display = 'grid';
    this.fileStatisticsAnalysisTableThread!.style.display = 'none';
    this.fileStatisticsAnalysisTableSo!.style.display = 'none';
    this.fileStatisticsAnalysisTableType!.style.display = 'none';
    this.fileStatisticsAnalysisTableFunction!.style.display = 'none';
    this.back!.style.visibility = 'hidden';
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = '';
    this.fileStatisticsAnalysisRange!.textContent =
      'Selected range: ' + parseFloat(((val.rightNs - val.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    this.getDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [{ queryFuncName: 'fileSystem', ...val }],
        },
      ],
      (results: any[]) => {
        this.getFilesystemProcess(results);
      }
    );
  }
  initElements(): void {
    this.fileStatisticsAnalysisRange = this.shadowRoot?.querySelector('#time-range');
    this.fileStatisticsAnalysisPie = this.shadowRoot!.querySelector<LitChartPie>('#fs-chart-pie');
    this.fileStatisticsAnalysisTableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.fileStatisticsAnalysisTableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.fileStatisticsAnalysisTableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.fileStatisticsAnalysisTableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.back = this.shadowRoot!.querySelector<HTMLDivElement>('.fs-go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.fs-subheading');
    this.fileStatisticsAnalysisTableType = this.shadowRoot!.querySelector<LitTable>('#tb-type-usage');
    this.fileStatisticsAnalysisProgressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.goBack();
  }
  clearData(): void {
    this.fileStatisticsAnalysisPie!.dataSource = [];
    this.fileStatisticsAnalysisTableProcess!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableThread!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableType!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableSo!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableFunction!.recycleDataSource = [];
  }
  goBack(): void {
    this.back!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By type AllDuration') {
        this.fileStatisticsAnalysisTableProcess!.style.display = 'grid';
        this.fileStatisticsAnalysisTableType!.style.display = 'none';
        this.back!.style.visibility = 'hidden';
        this.fileStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableProcess?.removeAttribute('hideDownload');
        this.currentLevel = 0;
        this.processPieChart();
      } else if (this.tabName!.textContent === 'Statistic By Thread AllDuration') {
        this.fileStatisticsAnalysisTableType!.style.display = 'grid';
        this.fileStatisticsAnalysisTableThread!.style.display = 'none';
        this.fileStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableType?.removeAttribute('hideDownload');
        this.currentLevel = 1;
        this.typePieChart();
      } else if (this.tabName!.textContent === 'Statistic By Library AllDuration') {
        this.fileStatisticsAnalysisTableThread!.style.display = 'grid';
        this.fileStatisticsAnalysisTableSo!.style.display = 'none';
        this.fileStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
        this.currentLevel = 2;
        this.threadPieChart();
      } else if (this.tabName!.textContent === 'Statistic By Function AllDuration') {
        this.fileStatisticsAnalysisTableSo!.style.display = 'grid';
        this.fileStatisticsAnalysisTableFunction!.style.display = 'none';
        this.fileStatisticsAnalysisTableFunction!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
        this.currentLevel = 3;
        this.libraryPieChart();
      }
    });
  }
  processPieChart(): void {
    // @ts-ignore
    this.sumDur = this.processStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getFsPieChartData(this.fileStatisticsAnalysisPidData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getFsTip(),
      angleClick: (fsPieClickItem): void => {
        // @ts-ignore
        if (fsPieClickItem.tableName != 'other') {
          this.fileProcessLevelClickEvent(fsPieClickItem);
        }
      },
      hoverHandler: (fsPieData): void => {
        if (fsPieData) {
          this.fileStatisticsAnalysisTableProcess!.setCurrentHover(fsPieData);
        } else {
          this.fileStatisticsAnalysisTableProcess!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.fileStatisticsAnalysisTableProcess!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let processData = evt.detail;
      if (processData.data) {
        let fspData = processData.data;
        fspData.isHover = true;
        if (processData.callBack) {
          processData.callBack(true);
        }
      }
      this.fileStatisticsAnalysisPie?.showHover();
      this.fileStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process AllDuration';
    this.fileStatisticsAnalysisPidData.unshift(this.processStatisticsData);
    this.fileStatisticsAnalysisTableProcess!.recycleDataSource = this.fileStatisticsAnalysisPidData;
    // @ts-ignore
    this.fileStatisticsAnalysisPidData.shift(this.processStatisticsData);
    this.currentLevelData = this.fileStatisticsAnalysisPidData;
    this.fileStatisticsAnalysisTableProcess?.reMeauseHeight();
    this.fileStatisticsAnalysisTableProcess!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.fileStatisticsAnalysisTableProcess!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileProcessLevelClickEvent(data);
      }
    });
  }
  fileProcessLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableProcess!.style.display = 'none';
    this.fileStatisticsAnalysisTableType!.style.display = 'grid';
    this.fileStatisticsAnalysisTableProcess!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableType?.removeAttribute('hideDownload');
    this.getFilesystemType(it);
    // @ts-ignore
    this.fileStatisticsAnalysisProcessName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.fileStatisticsAnalysisProcessName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  typePieChart(): void {
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.fileStatisticsAnalysisTypeData,
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getFileTypeTip(),
      angleClick: (it): void => {
        this.fileTypeLevelClickEvent(it);
      },
      hoverHandler: (data): void => {
        if (data) {
          this.fileStatisticsAnalysisTableType!.setCurrentHover(data);
        } else {
          this.fileStatisticsAnalysisTableType!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.fileStatisticsAnalysisTableType!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let typeData = evt.detail;
      if (typeData.data) {
        let fsaData = typeData.data;
        fsaData.isHover = true;
        if (typeData.callBack) {
          typeData.callBack(true);
        }
      }
      this.fileStatisticsAnalysisPie?.showHover();
      this.fileStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.fileStatisticsAnalysisProcessName;
    this.tabName!.textContent = 'Statistic By type AllDuration';
    this.fileStatisticsAnalysisTypeData.unshift(this.typeStatisticsData);
    this.fileStatisticsAnalysisTableType!.recycleDataSource = this.fileStatisticsAnalysisTypeData;
    // @ts-ignore
    this.fileStatisticsAnalysisTypeData.shift(this.typeStatisticsData);
    this.currentLevelData = this.fileStatisticsAnalysisTypeData;
    this.fileStatisticsAnalysisTableType?.reMeauseHeight();
    this.fileStatisticsAnalysisTableType!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.fileStatisticsAnalysisTableType!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileTypeLevelClickEvent(data);
      }
    });
  }
  private getFileTypeTip() {
    return (obj: { obj: { tableName: any; durFormat: any; percent: any; }; }): string => {
      return `<div>
                    <div>Type:${ obj.obj.tableName }</div>
                    <div>Duration:${ obj.obj.durFormat }</div>
                    <div>Percent:${ obj.obj.percent }%</div> 
                </div>
                `;
    };
  }
  fileTypeLevelClickEvent(it: any): void {
    this.clearData();
    this.fileStatisticsAnalysisTableType!.style.display = 'none';
    this.fileStatisticsAnalysisTableThread!.style.display = 'grid';
    this.fileStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
    this.getFilesystemThread(it);
    // @ts-ignore
    this.typeName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  threadPieChart(): void {
    // @ts-ignore
    this.sumDur = this.threadStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getFsPieChartData(this.fileStatisticsAnalysisThreadData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getFileTypeTip(),
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.fileThreadLevelClickEvent(it);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.fileStatisticsAnalysisTableThread!.setCurrentHover(data);
        } else {
          this.fileStatisticsAnalysisTableThread!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.fileStatisticsAnalysisTableThread!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let threadData = evt.detail;
      if (threadData.data) {
        let tableData = threadData.data;
        tableData.isHover = true;
        if (threadData.callBack) {
          threadData.callBack(true);
        }
      }
      this.fileStatisticsAnalysisPie?.showHover();
      this.fileStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName;
    this.tabName!.textContent = 'Statistic By Thread AllDuration';
    this.fileStatisticsAnalysisThreadData.unshift(this.threadStatisticsData);
    this.fileStatisticsAnalysisTableThread!.recycleDataSource = this.fileStatisticsAnalysisThreadData;
    // @ts-ignore
    this.fileStatisticsAnalysisThreadData.shift(this.threadStatisticsData);
    this.currentLevelData = this.fileStatisticsAnalysisThreadData;
    this.fileStatisticsAnalysisTableThread?.reMeauseHeight();
    this.fileStatisticsAnalysisTableThread!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.fileStatisticsAnalysisTableThread!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileThreadLevelClickEvent(data);
      }
    });
  }
  private getFsTip() {
    return (obj: { obj: { tableName: any; durFormat: any; percent: any; }; }): string => {
      return `<div>
                                <div>ThreadName:${ obj.obj.tableName }</div>
                                <div>Duration:${ obj.obj.durFormat }</div>
                                <div>Percent:${ obj.obj.percent }%</div> 
                            </div>
                                `;
    };
  }
  fileThreadLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableThread!.style.display = 'none';
    this.fileStatisticsAnalysisTableSo!.style.display = 'grid';
    this.fileStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
    this.getFilesystemSo(it);
    // @ts-ignore
    this.fileStatisticsAnalysisThreadName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName + ' / ' + this.fileStatisticsAnalysisThreadName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  libraryPieChart(): void {
    // @ts-ignore
    this.sumDur = this.libStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getFsPieChartData(this.fileStatisticsAnalysisSoData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (fileSysObj): string => {
        return `<div>
                                <div>Library:${ fileSysObj.obj.tableName }</div>
                                <div>Duration:${ fileSysObj.obj.durFormat }</div>
                                <div>Percent:${ fileSysObj.obj.percent }%</div> 
                            </div>
                                `;
      },
      angleClick: (fileSysBean): void => {
        // @ts-ignore
        if (fileSysBean.tableName != 'other') {
          this.fileSoLevelClickEvent(fileSysBean);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.fileStatisticsAnalysisTableSo!.setCurrentHover(data);
        } else {
          this.fileStatisticsAnalysisTableSo!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.fileStatisticsAnalysisTableSo!.addEventListener('row-hover', (evt) => {
      // @ts-ignore
      let soData = evt.detail;
      if (soData.data) {
        let fsSoData = soData.data;
        fsSoData.isHover = true;
        if (soData.callBack) {
          soData.callBack(true);
        }
      }
      this.fileStatisticsAnalysisPie?.showHover();
      this.fileStatisticsAnalysisPie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName + ' / ' + this.fileStatisticsAnalysisThreadName;
    this.tabName!.textContent = 'Statistic By Library AllDuration';
    this.fileStatisticsAnalysisSoData.unshift(this.libStatisticsData);
    this.fileStatisticsAnalysisTableSo!.recycleDataSource = this.fileStatisticsAnalysisSoData;
    // @ts-ignore
    this.fileStatisticsAnalysisSoData.shift(this.libStatisticsData);
    this.currentLevelData = this.fileStatisticsAnalysisSoData;
    this.fileStatisticsAnalysisTableSo?.reMeauseHeight();
    this.fileStatisticsAnalysisTableSo!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.fileStatisticsAnalysisTableSo!.addEventListener('row-click', (evt) => {
      // @ts-ignore
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileSoLevelClickEvent(data);
      }
    });
  }
  fileSoLevelClickEvent(it: any): void {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableSo!.style.display = 'none';
    this.fileStatisticsAnalysisTableFunction!.style.display = 'grid';
    this.fileStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableFunction?.removeAttribute('hideDownload');
    this.getFilesystemFunction(it);
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      // @ts-ignore
      this.fileStatisticsAnalysisProcessName +
      ' / ' +
      this.typeName +
      ' / ' +
      this.fileStatisticsAnalysisThreadName +
      ' / ' +
      it.tableName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  sortByColumn(column: string, fsaSort: number): void {
    this.fileStatisticsAnalysisSortColumn = column;
    this.fileStatisticsAnalysisSortType = fsaSort;
    let fsaCurrentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        fsaCurrentTable = this.fileStatisticsAnalysisTableProcess;
        break;
      case 1:
        fsaCurrentTable = this.fileStatisticsAnalysisTableType;
        break;
      case 2:
        fsaCurrentTable = this.fileStatisticsAnalysisTableThread;
        break;
      case 3:
        fsaCurrentTable = this.fileStatisticsAnalysisTableSo;
        break;
      case 4:
        fsaCurrentTable = this.fileStatisticsAnalysisTableFunction;
        break;
    }
    if (!fsaCurrentTable) {
      return;
    }
    if (fsaSort === 0) {
      let fsaArr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          fsaArr.unshift(this.processStatisticsData);
          break;
        case 1:
          fsaArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          fsaArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          fsaArr.unshift(this.libStatisticsData);
          break;
        case 4:
          fsaArr.unshift(this.functionStatisticsData);
          break;
      }
      fsaCurrentTable!.recycleDataSource = fsaArr;
    } else {
      let fsaArray = [...this.currentLevelData];
      if (column === 'tableName') {
        fsaCurrentTable!.recycleDataSource = fsaArray.sort((firstElement, secondElement) => {
          if (fsaSort === 1) {
            if (firstElement.tableName > secondElement.tableName) {
              return 1;
            } else if (firstElement.tableName === secondElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (secondElement.tableName > firstElement.tableName) {
              return 1;
            } else if (firstElement.tableName === secondElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (column === 'durFormat' || column === 'percent') {
        fsaCurrentTable!.recycleDataSource = fsaArray.sort((a, b) => {
          return fsaSort === 1 ? a.duration - b.duration : b.duration - a.duration;
        });
      }
      switch (this.currentLevel) {
        case 0:
          fsaArray.unshift(this.processStatisticsData);
          break;
        case 1:
          fsaArray.unshift(this.typeStatisticsData);
          break;
        case 2:
          fsaArray.unshift(this.threadStatisticsData);
          break;
        case 3:
          fsaArray.unshift(this.libStatisticsData);
          break;
        case 4:
          fsaArray.unshift(this.functionStatisticsData);
          break;
      }
      fsaCurrentTable!.recycleDataSource = fsaArray;
    }
  }
  getFilesystemProcess(result: Array<any>): void {
    this.fileStatisticsAnalysisProcessData = JSON.parse(JSON.stringify(result));
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length === 0) {
      this.fileStatisticsAnalysisPidData = [];
      this.processStatisticsData = [];
      this.processPieChart();
      return;
    }
    let allDur = 0;
    let pidMap = new Map<string, Array<number | string>>();
    for (let itemData of result) {
      allDur += itemData.dur;
      if (pidMap.has(itemData.pid)) {
        pidMap.get(itemData.pid)?.push(itemData);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(itemData);
        pidMap.set(itemData.pid, itemArray);
      }
    }
    this.fileStatisticsAnalysisPidData = [];
    pidMap.forEach((value: Array<any>, key: string) => {
      let analysisPidDataDur = 0;
      let pName = '';
      for (let fileSysStatPidItem of value) {
        pName = fileSysStatPidItem.processName =
          fileSysStatPidItem.processName === null || fileSysStatPidItem.processName === undefined ?
            `Process(${ fileSysStatPidItem.pid })` : `${ fileSysStatPidItem.processName }(${ fileSysStatPidItem.pid })`;
        analysisPidDataDur += fileSysStatPidItem.dur;
      }
      this.fileStatisticsAnalysisPidData.push({
        tableName: pName,
        pid: key,
        percent: ((analysisPidDataDur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(analysisPidDataDur),
        duration: analysisPidDataDur,
      });
    });
    this.fileStatisticsAnalysisPidData.sort((a, b) => b.duration - a.duration);
    this.processStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 0;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    this.processPieChart();
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        this.fileStatisticsAnalysisTableProcess!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.fileStatisticsAnalysisTableProcess?.reMeauseHeight();
        this.fileStatisticsAnalysisTableThread!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.fileStatisticsAnalysisTableThread?.reMeauseHeight();
        this.fileStatisticsAnalysisTableSo!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.fileStatisticsAnalysisTableSo?.reMeauseHeight();
        this.fileStatisticsAnalysisTableFunction!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.fileStatisticsAnalysisTableFunction?.reMeauseHeight();
        this.fileStatisticsAnalysisTableType!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.fileStatisticsAnalysisTableType?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  getFilesystemType(fileSysStatTypeItem: any): void {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let typeMap = new Map<number, Array<number | string>>();
    let pid = fileSysStatTypeItem.pid;
    let allDur = 0;
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length == 0) {
      return;
    }
    for (let fsItem of this.fileStatisticsAnalysisProcessData) {
      if (fsItem.pid !== pid) {
        continue;
      }
      allDur += fsItem.dur;
      if (typeMap.has(fsItem.type)) {
        typeMap.get(fsItem.type)?.push(fsItem);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(fsItem);
        typeMap.set(fsItem.type, itemArray);
      }
    }
    this.fileStatisticsAnalysisTypeData = [];
    typeMap.forEach((value: Array<any>, key: number) => {
      let dur = 0;
      for (let item of value) {
        dur += item.dur;
      }
      const typeData = {
        tableName: this.typeIdToString(key),
        pid: fileSysStatTypeItem.pid,
        type: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.fileStatisticsAnalysisTypeData.push(typeData);
    });
    this.fileStatisticsAnalysisTypeData.sort((a, b) => b.duration - a.duration);
    this.typeStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 1;
    this.typePieChart();
    this.fileStatisticsAnalysisProgressEL!.loading = false;
  }

  getFilesystemThread(fileSysStatThreadItem: any): void {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let threadMap = new Map<string, Array<number | string>>();
    let pid = fileSysStatThreadItem.pid;
    let type = fileSysStatThreadItem.type;
    let allDur = 0;
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let fspItem of this.fileStatisticsAnalysisProcessData) {
      if (fspItem.pid !== pid || fspItem.type !== type) {
        continue;
      }
      allDur += fspItem.dur;
      if (threadMap.has(fspItem.tid)) {
        threadMap.get(fspItem.tid)?.push(fspItem);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(fspItem);
        threadMap.set(fspItem.tid, itemArray);
      }
    }
    this.fileStatisticsAnalysisThreadData = [];
    threadMap.forEach((value: Array<any>, key: string) => {
      let dur = 0;
      let tName = '';
      for (let fileSysStatThreadItem of value) {
        dur += fileSysStatThreadItem.dur;
        tName = fileSysStatThreadItem.threadName =
          fileSysStatThreadItem.threadName === null || fileSysStatThreadItem.threadName === undefined ? `Thread(${ fileSysStatThreadItem.tid })` : `${ fileSysStatThreadItem.threadName }`;
      }
      const threadData = {
        tableName: tName,
        pid: fileSysStatThreadItem.pid,
        type: fileSysStatThreadItem.type,
        tid: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.fileStatisticsAnalysisThreadData.push(threadData);
    });
    this.fileStatisticsAnalysisThreadData.sort((a, b) => b.duration - a.duration);
    this.threadStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 2;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    this.threadPieChart();
  }

  getFilesystemSo(item: any): void {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    let libMap = new Map<number, Array<number | string>>();
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let itemData of this.fileStatisticsAnalysisProcessData) {
      if (itemData.pid !== pid || itemData.tid !== tid || itemData.type !== type) {
        continue;
      }
      allDur += itemData.dur;
      if (libMap.has(itemData.libId)) {
        libMap.get(itemData.libId)?.push(itemData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(itemData);
        libMap.set(itemData.libId, dataArray);
      }
    }
    this.fileStatisticsAnalysisSoData = [];
    libMap.forEach((value: any[], key: number) => {
      let dur = 0;
      let soName = '';
      for (let item of value) {
        dur += item.dur;
        if (key === null) {
          item.libName = 'unknown';
        }
        soName = item.libName;
      }
      let libPath = soName?.split('/');
      if (libPath) {
        soName = libPath[libPath.length - 1];
      }
      const soData = {
        tableName: soName,
        pid: item.pid,
        type: item.type,
        tid: item.tid,
        libId: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.fileStatisticsAnalysisSoData.push(soData);
    });
    this.fileStatisticsAnalysisSoData.sort((a, b) => b.duration - a.duration);
    this.libStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 3;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    this.libraryPieChart();
  }

  getFilesystemFunction(item: any): void {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.fs-subheading')!.textContent = 'Statistic By Function AllDuration';
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let libId = item.libId;
    let allDur = 0;
    let symbolMap = new Map<number, Array<any>>();
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length === 0) {
      return;
    }
    for (let fsProcessData of this.fileStatisticsAnalysisProcessData) {
      if (
        fsProcessData.pid !== pid ||
        fsProcessData.tid !== tid ||
        fsProcessData.type !== type ||
        fsProcessData.libId !== libId
      ) {
        continue;
      }
      allDur += fsProcessData.dur;
      if (symbolMap.has(fsProcessData.symbolId)) {
        symbolMap.get(fsProcessData.symbolId)?.push(fsProcessData);
      } else {
        let dataArray = new Array<number | string>();
        dataArray.push(fsProcessData);
        symbolMap.set(fsProcessData.symbolId, dataArray);
      }
    }
    this.fileStatisticsAnalysisFunctionData = [];
    symbolMap.forEach((symbolItems, key) => {
      let dur = 0;
      let fsSymbolName = '';
      for (let symbolItem of symbolItems) {
        fsSymbolName = symbolItem.symbolName;
        dur += symbolItem.dur;
      }
      let symbolPath = fsSymbolName?.split('/');
      if (symbolPath) {
        fsSymbolName = symbolPath[symbolPath.length - 1];
      }
      const symbolData = {
        pid: item.pid,
        tid: item.tid,
        percent: ((dur / allDur) * 100).toFixed(2),
        tableName: fsSymbolName,
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.fileStatisticsAnalysisFunctionData.push(symbolData);
    });
    this.fileStatisticsAnalysisFunctionData.sort((a, b) => b.duration - a.duration);
    this.functionStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 4;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    // @ts-ignore
    this.sumDur = this.functionStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getFsPieChartData(this.fileStatisticsAnalysisFunctionData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (fsaObj): string => {
        return `<div>
                                    <div>Function:${ fsaObj.obj.tableName }</div>
                                    <div>Duration:${ fsaObj.obj.durFormat }</div>
                                    <div>percent:${ fsaObj.obj.percent }</div>
                                        </div>
                                                `;
      },
      hoverHandler: (data): void => {
        if (data) {
          this.fileStatisticsAnalysisTableFunction!.setCurrentHover(data);
        } else {
          this.fileStatisticsAnalysisTableFunction!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.fileStatisticsAnalysisTableFunction!.addEventListener('row-hover', (fsStatRowClickEvent) => {
      // @ts-ignore
      let fsFunctionData = fsStatRowClickEvent.detail;
      if (fsFunctionData.data) {
        let data = fsFunctionData.data;
        data.isHover = true;
        if (fsFunctionData.callBack) {
          fsFunctionData.callBack(true);
        }
      }
      this.fileStatisticsAnalysisPie?.showHover();
      this.fileStatisticsAnalysisPie?.hideTip();
    });
    this.fileStatisticsAnalysisFunctionData.unshift(this.functionStatisticsData);
    this.fileStatisticsAnalysisTableFunction!.recycleDataSource = this.fileStatisticsAnalysisFunctionData;
    this.fileStatisticsAnalysisTableFunction?.reMeauseHeight();
    // @ts-ignore
    this.fileStatisticsAnalysisFunctionData.shift(this.functionStatisticsData);
    this.currentLevelData = this.fileStatisticsAnalysisFunctionData;
    this.fileStatisticsAnalysisTableFunction!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }
  typeIdToString(transformType: number): string {
    let fsReleaseType: string;
    if (transformType === 0) {
      fsReleaseType = 'OPEN';
    } else if (transformType === 2) {
      fsReleaseType = 'READ';
    } else if (transformType === 3) {
      fsReleaseType = 'WRITE';
    } else if (transformType === 1) {
      fsReleaseType = 'CLOSE';
    }
    // @ts-ignore
    return fsReleaseType;
  }
  totalDurationData(durationTS: number): { durFormat: string; percent: string; tableName: string; duration: number; } {
    return {
      durFormat: Utils.getProbablyTime(durationTS),
      percent: ((durationTS / durationTS) * 100).toFixed(2),
      tableName: '',
      duration: 0,
    };
  }
  getFsPieChartData(fsPieChartData: any[]): unknown[] {
    if (fsPieChartData.length > 20) {
      let fsPieChartArr: string[] = [];
      let other: any = {
        tableName: 'other',
        duration: 0,
        percent: 0,
        durFormat: 0,
      };
      for (let pieDataIndex = 0 ; pieDataIndex < fsPieChartData.length ; pieDataIndex++) {
        if (pieDataIndex < 19) {
          fsPieChartArr.push(fsPieChartData[pieDataIndex]);
        } else {
          other.duration += fsPieChartData[pieDataIndex].duration;
          other.durFormat = Utils.getProbablyTime(other.duration);
          other.percent = ((other.duration / this.sumDur) * 100).toFixed(2);
        }
      }
      fsPieChartArr.push(other);
      return fsPieChartArr;
    }
    return fsPieChartData;
  }

  getDataByWorker(args: any[], handler: Function): void {
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-action',
      { args, callType: 'fileSystem', isAnalysis: true },
      undefined,
      (results: any) => {
        handler(results);
        this.fileStatisticsAnalysisProgressEL!.loading = false;
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
        #fs-chart-pie{
             height: 300px;
        }
        .fs-table-box{
            width: 60%;
            border-left: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
        }
        .fs-go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
            margin-left: 20px;
            visibility: hidden;
        }
        .fs-back-box{
            width: 40px;
            height: 20px;
            background-color: var(--bark-expansion,#0C65D1);
            border-radius: 5px;
            color: #fff;
            display: flex;
            margin-right: 10px;
            justify-content: center;
            align-items: center;
        }
        .fs-subheading{
            font-weight: bold;
            text-align: center;
        }
        .fs-stat-analysis-progress{
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        } 
        </style>
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label> 
        <div style="display: flex;flex-direction: row;" class="d-box">
            <lit-progress-bar class="progress fs-stat-analysis-progress"></lit-progress-bar>
                     <div id="left_table" style="width: 40%;height:auto;">
                         <div style="display: flex;margin-bottom: 10px">
                           <div class="fs-go-back">
                              <div class="fs-back-box">
                                  <lit-icon class="file-analysis" name="arrowleft"></lit-icon>
                              </div>
                           </div>
                         <div class="title"></div>
                        </div>
                         <div class="fs-subheading"></div>                     
                         <lit-chart-pie  id="fs-chart-pie"></lit-chart-pie>     
                     </div>
                     <div class="fs-table-box" style="height:auto;overflow: auto">
                    <lit-table id="tb-process-usage"class="file-analysis" style="max-height:565px;min-height: 350px">
                        <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-type-usage" class="file-analysis" style="max-height:565px;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Type" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-thread-usage" class="file-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                     <lit-table id="tb-so-usage" class="file-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-function-usage" class="file-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                        <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    </div>

        </div>
`;
  }
}
