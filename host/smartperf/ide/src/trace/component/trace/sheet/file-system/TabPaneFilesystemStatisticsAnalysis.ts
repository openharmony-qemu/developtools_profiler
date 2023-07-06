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

@element('tabpane-file-statistics-analysis')
export class TabPaneFilesystemStatisticsAnalysis extends BaseElement {
  private fileStatisticsAnalysisPie: LitChartPie | null | undefined;
  private fileStatisticsAnalysisCurrentSelection: SelectionParam | any;
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
  private sumDur: any;
  private fileStatisticsAnalysisRange: HTMLLabelElement | null | undefined;
  private back: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private fileStatisticsAnalysisProgressEL: LitProgressBar | null | undefined;
  private fileStatisticsAnalysisProcessName: string = '';
  private fileStatisticsAnalysisThreadName: string = '';
  private fileStatisticsAnalysisSortColumn: string = '';
  private fileStatisticsAnalysisSortType: number = 0;
  private typeName: any;
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private processStatisticsData!: any;
  private typeStatisticsData!: any;
  private threadStatisticsData!: any;
  private libStatisticsData!: any;
  private functionStatisticsData!: any;
  set data(val: SelectionParam | any) {
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
        this.getFilesystemProcess(val, results);
      }
    );
  }
  initElements(): void {
    this.fileStatisticsAnalysisRange = this.shadowRoot?.querySelector('#time-range');
    this.fileStatisticsAnalysisPie = this.shadowRoot!.querySelector<LitChartPie>('#chart-pie');
    this.fileStatisticsAnalysisTableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.fileStatisticsAnalysisTableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.fileStatisticsAnalysisTableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.fileStatisticsAnalysisTableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.back = this.shadowRoot!.querySelector<HTMLDivElement>('.go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.subheading');
    this.fileStatisticsAnalysisTableType = this.shadowRoot!.querySelector<LitTable>('#tb-type-usage');
    this.fileStatisticsAnalysisProgressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.goBack();
  }
  clearData() {
    this.fileStatisticsAnalysisPie!.dataSource = [];
    this.fileStatisticsAnalysisTableProcess!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableThread!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableType!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableSo!.recycleDataSource = [];
    this.fileStatisticsAnalysisTableFunction!.recycleDataSource = [];
  }
  goBack() {
    this.back!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By type AllDuration') {
        this.fileStatisticsAnalysisTableProcess!.style.display = 'grid';
        this.fileStatisticsAnalysisTableType!.style.display = 'none';
        this.back!.style.visibility = 'hidden';
        this.fileStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableProcess?.removeAttribute('hideDownload');
        this.currentLevel = 0;
        this.processPieChart(this.fileStatisticsAnalysisCurrentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Thread AllDuration') {
        this.fileStatisticsAnalysisTableType!.style.display = 'grid';
        this.fileStatisticsAnalysisTableThread!.style.display = 'none';
        this.fileStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableType?.removeAttribute('hideDownload');
        this.currentLevel = 1;
        this.typePieChart(this.fileStatisticsAnalysisCurrentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Library AllDuration') {
        this.fileStatisticsAnalysisTableThread!.style.display = 'grid';
        this.fileStatisticsAnalysisTableSo!.style.display = 'none';
        this.fileStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
        this.currentLevel = 2;
        this.threadPieChart(this.fileStatisticsAnalysisCurrentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Function AllDuration') {
        this.fileStatisticsAnalysisTableSo!.style.display = 'grid';
        this.fileStatisticsAnalysisTableFunction!.style.display = 'none';
        this.fileStatisticsAnalysisTableFunction!.setAttribute('hideDownload', '');
        this.fileStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
        this.currentLevel = 3;
        this.libraryPieChart(this.fileStatisticsAnalysisCurrentSelection);
      }
    });
  }
  processPieChart(val: any) {
    this.sumDur = this.processStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.fileStatisticsAnalysisPidData),
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
          this.fileProcessLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.fileStatisticsAnalysisTableProcess!.setCurrentHover(data);
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
    this.fileStatisticsAnalysisTableProcess!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let fspData = evt.detail.data;
        fspData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
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
    this.fileStatisticsAnalysisTableProcess!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileProcessLevelClickEvent(data, val);
      }
    });
  }
  fileProcessLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableProcess!.style.display = 'none';
    this.fileStatisticsAnalysisTableType!.style.display = 'grid';
    this.fileStatisticsAnalysisTableProcess!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableType?.removeAttribute('hideDownload');
    this.getFilesystemType(it, val);
    // @ts-ignore
    this.fileStatisticsAnalysisProcessName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.fileStatisticsAnalysisProcessName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  typePieChart(val: any) {
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.fileStatisticsAnalysisTypeData,
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
        this.fileTypeLevelClickEvent(it, val);
      },
      hoverHandler: (data) => {
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
    this.fileStatisticsAnalysisTableType!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let fsaData = evt.detail.data;
        fsaData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
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
    this.fileStatisticsAnalysisTableType!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileTypeLevelClickEvent(data, val);
      }
    });
  }
  fileTypeLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.fileStatisticsAnalysisTableType!.style.display = 'none';
    this.fileStatisticsAnalysisTableThread!.style.display = 'grid';
    this.fileStatisticsAnalysisTableType!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableThread?.removeAttribute('hideDownload');
    this.getFilesystemThread(it, val);
    // @ts-ignore
    this.typeName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  threadPieChart(val: any) {
    this.sumDur = this.threadStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.fileStatisticsAnalysisThreadData),
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
          this.fileThreadLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
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
    this.fileStatisticsAnalysisTableThread!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let tableData = evt.detail.data;
        tableData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
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
    this.fileStatisticsAnalysisTableThread!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileThreadLevelClickEvent(data, val);
      }
    });
  }
  fileThreadLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableThread!.style.display = 'none';
    this.fileStatisticsAnalysisTableSo!.style.display = 'grid';
    this.fileStatisticsAnalysisTableThread!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableSo?.removeAttribute('hideDownload');
    this.getFilesystemSo(it, val);
    // @ts-ignore
    this.fileStatisticsAnalysisThreadName = it.tableName;
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent =
      this.fileStatisticsAnalysisProcessName + ' / ' + this.typeName + ' / ' + this.fileStatisticsAnalysisThreadName;
    this.fileStatisticsAnalysisPie?.hideTip();
  }
  libraryPieChart(val: any) {
    this.sumDur = this.libStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.fileStatisticsAnalysisSoData),
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
          this.fileSoLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
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
    this.fileStatisticsAnalysisTableSo!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let fsSoData = evt.detail.data;
        fsSoData.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
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
    this.fileStatisticsAnalysisTableSo!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.duration !== 0) {
        this.fileSoLevelClickEvent(data, val);
      }
    });
  }
  fileSoLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.fileStatisticsAnalysisTableSo!.style.display = 'none';
    this.fileStatisticsAnalysisTableFunction!.style.display = 'grid';
    this.fileStatisticsAnalysisTableSo!.setAttribute('hideDownload', '');
    this.fileStatisticsAnalysisTableFunction?.removeAttribute('hideDownload');
    this.getFilesystemFunction(it, val);
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
  sortByColumn(column: string, fsaSort: number) {
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
      let fsaArr = [...this.currentLevelData];
      if (column === 'tableName') {
        fsaCurrentTable!.recycleDataSource = fsaArr.sort((a, b) => {
          if (fsaSort === 1) {
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
        fsaCurrentTable!.recycleDataSource = fsaArr.sort((a, b) => {
          return fsaSort === 1 ? a.duration - b.duration : b.duration - a.duration;
        });
      }
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
    }
  }
  getFilesystemProcess(val: any, result: Array<any>) {
    this.fileStatisticsAnalysisProcessData = JSON.parse(JSON.stringify(result));
    if (!this.fileStatisticsAnalysisProcessData || this.fileStatisticsAnalysisProcessData.length === 0) {
      this.fileStatisticsAnalysisPidData = [];
      this.processStatisticsData = [];
      this.processPieChart(val);
      return;
    }
    let allDur = 0;
    let pidMap = new Map<string, Array<any>>();
    for (let itemData of result) {
      allDur += itemData.dur;
      if (pidMap.has(itemData.pid)) {
        pidMap.get(itemData.pid)?.push(itemData);
      } else {
        let itemArray = new Array<any>();
        itemArray.push(itemData);
        pidMap.set(itemData.pid, itemArray);
      }
    }
    this.fileStatisticsAnalysisPidData = [];
    pidMap.forEach((value: Array<any>, key: string) => {
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
      this.fileStatisticsAnalysisPidData.push(pidData);
    });
    this.fileStatisticsAnalysisPidData.sort((a, b) => b.duration - a.duration);
    this.processStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 0;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    this.processPieChart(val);
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight != 0) {
        this.fileStatisticsAnalysisTableProcess!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.fileStatisticsAnalysisTableThread!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.fileStatisticsAnalysisTableSo!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.fileStatisticsAnalysisTableFunction!.style.height = this.parentElement!.clientHeight - 30 + 'px';
        this.fileStatisticsAnalysisTableType!.style.height = this.parentElement!.clientHeight - 40 + 'px';
      }
    }).observe(this.parentElement!);
  }

  getFilesystemType(item: any, val: any) {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let typeMap = new Map<string, Array<any>>();
    let pid = item.pid;
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
        let itemArray = new Array<any>();
        itemArray.push(fsItem);
        typeMap.set(fsItem.type, itemArray);
      }
    }
    this.fileStatisticsAnalysisTypeData = [];
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
      this.fileStatisticsAnalysisTypeData.push(typeData);
    });
    this.fileStatisticsAnalysisTypeData.sort((a, b) => b.duration - a.duration);
    this.typeStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 1;
    this.typePieChart(val);
    this.fileStatisticsAnalysisProgressEL!.loading = false;
  }

  getFilesystemThread(item: any, val: any) {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let threadMap = new Map<string, Array<any>>();
    let pid = item.pid;
    let type = item.type;
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
        let itemArray = new Array<any>();
        itemArray.push(fspItem);
        threadMap.set(fspItem.tid, itemArray);
      }
    }
    this.fileStatisticsAnalysisThreadData = [];
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
      this.fileStatisticsAnalysisThreadData.push(threadData);
    });
    this.fileStatisticsAnalysisThreadData.sort((a, b) => b.duration - a.duration);
    this.threadStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 2;
    this.fileStatisticsAnalysisProgressEL!.loading = false;
    this.threadPieChart(val);
  }

  getFilesystemSo(item: any, val: any) {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    let tid = item.tid;
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    let libMap = new Map<number, Array<any>>();
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
        let dataArray = new Array<any>();
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
          item.libName = 'unkown';
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
    this.libraryPieChart(val);
  }

  getFilesystemFunction(item: any, val: any) {
    this.fileStatisticsAnalysisProgressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.subheading')!.textContent = 'Statistic By Function AllDuration';
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
        let dataArray = new Array<any>();
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
    this.sumDur = this.functionStatisticsData.allDuration;
    this.fileStatisticsAnalysisPie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.fileStatisticsAnalysisFunctionData),
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
    this.fileStatisticsAnalysisTableFunction!.addEventListener('row-hover', (fsStatRowClickEvent: any) => {
      if (fsStatRowClickEvent.detail.data) {
        let data = fsStatRowClickEvent.detail.data;
        data.isHover = true;
        if ((fsStatRowClickEvent.detail as any).callBack) {
          (fsStatRowClickEvent.detail as any).callBack(true);
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
  typeIdToString(transformType: any) {
    let releaseType: any;
    if (transformType === 0) {
      releaseType = 'OPEN';
    } else if (transformType === 2) {
      releaseType = 'READ';
    } else if (transformType === 3) {
      releaseType = 'WRITE';
    } else if (transformType === 1) {
      releaseType = 'CLOSE';
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
    };
    return allDuration;
  }
  getPieChartData(pieChartData: any[]) {
    if (pieChartData.length > 20) {
      let pieChartArr: any[] = [];
      let other: any = {
        tableName: 'other',
        duration: 0,
        percent: 0,
        durFormat: 0,
      };
      for (let pieDataIndex = 0; pieDataIndex < pieChartData.length; pieDataIndex++) {
        if (pieDataIndex < 19) {
          pieChartArr.push(pieChartData[pieDataIndex]);
        } else {
          other.duration += pieChartData[pieDataIndex].duration;
          other.durFormat = Utils.getProbablyTime(other.duration);
          other.percent = ((other.duration / this.sumDur) * 100).toFixed(2);
        }
      }
      pieChartArr.push(other);
      return pieChartArr;
    }
    return pieChartData;
  }

  getDataByWorker(args: any[], handler: Function) {
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
                        <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start" order></lit-table-column>
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
