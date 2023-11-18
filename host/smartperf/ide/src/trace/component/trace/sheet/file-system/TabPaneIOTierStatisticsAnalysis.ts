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
import '../../../../../base-ui/chart/pie/LitChartPie.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar';
import { Utils } from '../../base/Utils.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { LitCheckBox } from '../../../../../base-ui/checkbox/LitCheckBox.js';
import { TabPaneFilter } from '../TabPaneFilter.js';
import { initSort } from '../SheetUtils.js';
import { TabPaneIOCallTree } from './TabPaneIOCallTree.js';

@element('tabpane-tb-vm-statistics')
export class TabPaneIOTierStatisticsAnalysis extends BaseElement {
  private ioPieChart: LitChartPie | null | undefined;
  private currentSelection: SelectionParam | null | undefined;
  private processData: any;
  private pidData!: any[];
  private threadData!: any[];
  private soData!: any[];
  private functionData!: any[];
  private typeData!: any[];
  private ioTierTableProcess: LitTable | null | undefined;
  private ioTierTableThread: LitTable | null | undefined;
  private tableType: LitTable | null | undefined;
  private ioTierTableSo: LitTable | null | undefined;
  private tableFunction: LitTable | null | undefined;
  private sumDur: number = 0;
  private range: HTMLLabelElement | null | undefined;
  private iOTierStatisticsAnalysisBack: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private processName: string = '';
  private threadName: string = '';
  private ioSortColumn: string = '';
  private ioSortType: number = 0;
  private typeName: string = '';
  private currentLevel = -1;
  private currentLevelData!: Array<any>;
  private processStatisticsData!: {};
  private typeStatisticsData!: {};
  private threadStatisticsData!: {};
  private libStatisticsData!: {};
  private functionStatisticsData!: {};
  private titleEl: HTMLDivElement | undefined | null;
  private filterEl: TabPaneFilter | undefined | null;
  private hideProcessCheckBox: LitCheckBox | undefined | null;
  private hideThreadCheckBox: LitCheckBox | undefined | null;
  private checkBoxs: NodeListOf<LitCheckBox> | undefined | null;
  private ioTableArray: NodeListOf<LitTable> | undefined | null;

  set data(ioTierStatisticsAnalysisSelection: SelectionParam) {
    if (ioTierStatisticsAnalysisSelection === this.currentSelection) {
      this.pidData.unshift(this.processStatisticsData);
      this.ioTierTableProcess!.recycleDataSource = this.pidData;
      // @ts-ignore
      this.pidData.shift(this.processStatisticsData);
      return;
    }
    if (this.ioTableArray && this.ioTableArray.length > 0) {
      for (let ioTable of this.ioTableArray) {
        initSort(ioTable!, this.ioSortColumn, this.ioSortType);
      }
    }
    this.reset(this.ioTierTableProcess!, false);
    this.hideProcessCheckBox!.checked = false;
    this.hideThreadCheckBox!.checked = false;
    this.currentSelection = ioTierStatisticsAnalysisSelection;
    this.titleEl!.textContent = '';
    this.tabName!.textContent = '';
    this.range!.textContent =
      'Selected range: ' +
      parseFloat(
        ((ioTierStatisticsAnalysisSelection.rightNs - ioTierStatisticsAnalysisSelection.leftNs) / 1000000.0).toFixed(5)
      ) +
      '  ms';
    this.progressEL!.loading = true;
    this.getIoTierDataByWorker(
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
        this.processData = JSON.parse(JSON.stringify(results));
        this.getIOTierProcess(this.processData);
      }
    );
  }
  initElements(): void {
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.ioPieChart = this.shadowRoot!.querySelector<LitChartPie>('#io-tier-chart-pie');
    this.ioTierTableProcess = this.shadowRoot!.querySelector<LitTable>('#tb-process-usage');
    this.ioTierTableThread = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.ioTierTableSo = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.tableFunction = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.tableType = this.shadowRoot!.querySelector<LitTable>('#tb-type-usage');
    this.iOTierStatisticsAnalysisBack = this.shadowRoot!.querySelector<HTMLDivElement>('.io-tier-go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.io-tier-subheading');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.goBack();
    this.titleEl = this.shadowRoot!.querySelector<HTMLDivElement>('.title');
    this.filterEl = this.shadowRoot?.querySelector('#filter');
    this.filterEl!.setOptionsList(['Hide Process', 'Hide Thread']);
    let popover = this.filterEl!.shadowRoot!.querySelector('#check-popover');
    this.hideProcessCheckBox = popover!!.querySelector<LitCheckBox>('div > #hideProcess');
    this.hideThreadCheckBox = popover!!.querySelector<LitCheckBox>('div > #hideThread');
    this.checkBoxs = popover!.querySelectorAll<LitCheckBox>('.check-wrap > lit-check-box');
    this.ioTableArray = this.shadowRoot!.querySelectorAll('lit-table') as NodeListOf<LitTable>;
    for (let ioTable of this.ioTableArray) {
      ioTable.shadowRoot!.querySelector<HTMLDivElement>('.table')!.style.height = 'calc(100% - 31px)';
      ioTable!.addEventListener('column-click', (evt) => {
        // @ts-ignore
        this.ioSortColumn = evt.detail.key;
        // @ts-ignore
        this.ioSortType = evt.detail.sort;
        this.sortByColumn();
      });
      ioTable!.addEventListener('contextmenu', function (event) {
        event.preventDefault(); // 阻止默认的上下文菜单弹框
      });
      ioTable!.addEventListener('row-hover', (evt) => {
        // @ts-ignore
        let detail = evt.detail;
        if (detail.data) {
          let tableData = detail.data;
          tableData.isHover = true;
          if (detail.callBack) {
            detail.callBack(true);
          }
        }
        this.ioPieChart?.showHover();
        this.ioPieChart?.hideTip();
      });
      ioTable!.addEventListener('row-click', (evt) => {
        // @ts-ignore
        let detail = evt.detail;
        if (detail.button === 2) {
          let ioTab = this.parentElement?.parentElement?.querySelector<TabPaneIOCallTree>(
            '#box-io-calltree > tabpane-io-calltree'
          );
          if (detail.button === 2) {
            ioTab!.cWidth = this.clientWidth;
            ioTab!.currentCallTreeLevel = this.currentLevel;
            if (this.hideProcessCheckBox?.checked) {
              detail.data.pid = undefined;
            }
            if (this.hideThreadCheckBox?.checked) {
              detail.data.tid = undefined;
            }
            ioTab!.rowClickData = detail.data;
            let title = '';
            if (this.titleEl?.textContent === '') {
              title = detail.data.tableName;
            } else {
              title = this.titleEl?.textContent + ' / ' + detail.data.tableName;
            }
            ioTab!.pieTitle = title;
            //  是否是在表格上右键点击跳转到火焰图的
            this.currentSelection!.isRowClick = true;
            ioTab!.data = this.currentSelection;
          }
        }
      });
    }
    for (let box of this.checkBoxs) {
      box!.addEventListener('change', (event) => {
        if (this.hideProcessCheckBox!.checked && this.hideThreadCheckBox!.checked) {
          this.hideThread();
          this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
        } else if (this.hideProcessCheckBox!.checked && !this.hideThreadCheckBox!.checked) {
          this.hideProcess();
        } else {
          this.reset(this.ioTierTableProcess!, false);
          this.getIOTierProcess(this.processData);
        }
      });
    }

    const addRowClickEventListener = (ioTable: LitTable, clickEvent: Function) => {
      ioTable.addEventListener('row-click', (evt) => {
        // @ts-ignore
        const detail = evt.detail;
        if (detail.button === 0 && detail.data.tableName !== '' && detail.data.duration !== 0) {
          clickEvent(detail.data, this.currentSelection);
        }
      });
    };

    addRowClickEventListener(this.ioTierTableProcess!, this.ioTierProcessLevelClickEvent.bind(this));
    addRowClickEventListener(this.tableType!, this.ioTierTypeLevelClickEvent.bind(this));
    addRowClickEventListener(this.ioTierTableThread!, this.ioTierThreadLevelClickEvent.bind(this));
    addRowClickEventListener(this.ioTierTableSo!, this.ioTierSoLevelClickEvent.bind(this));
  }

  private reset(showTable: LitTable, isShowBack: boolean): void {
    this.clearData();
    if (isShowBack) {
      this.iOTierStatisticsAnalysisBack!.style.visibility = 'visible';
    } else {
      this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
      this.titleEl!.textContent = '';
    }
    if (this.ioTableArray) {
      for (let table of this.ioTableArray) {
        if (table === showTable) {
          initSort(table!, this.ioSortColumn, this.ioSortType);
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
    this.ioPieChart!.dataSource = [];
    this.ioTierTableProcess!.recycleDataSource = [];
    this.tableType!.recycleDataSource = [];
    this.ioTierTableThread!.recycleDataSource = [];
    this.ioTierTableSo!.recycleDataSource = [];
    this.tableFunction!.recycleDataSource = [];
  }

  private showAssignLevel(showIoTable: LitTable, hideIoTable: LitTable, currentLevel: number): void {
    showIoTable!.style.display = 'grid';
    hideIoTable!.style.display = 'none';
    hideIoTable.setAttribute('hideDownload', '');
    showIoTable?.removeAttribute('hideDownload');
    this.currentLevel = currentLevel;
  }

  private goBack(): void {
    this.iOTierStatisticsAnalysisBack!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By type AllDuration') {
        this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
        this.showAssignLevel(this.ioTierTableProcess!, this.tableType!, 0);
        this.processPieChart();
      } else if (this.tabName!.textContent === 'Statistic By Thread AllDuration') {
        if (this.hideProcessCheckBox?.checked) {
          this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
        } else {
          this.iOTierStatisticsAnalysisBack!.style.visibility = 'visible';
        }
        this.showAssignLevel(this.tableType!, this.ioTierTableThread!, 1);
        this.typePieChart();
      } else if (this.tabName!.textContent === 'Statistic By Library AllDuration') {
        if (this.hideThreadCheckBox?.checked) {
          if (this.hideProcessCheckBox?.checked) {
            this.iOTierStatisticsAnalysisBack!.style.visibility = 'hidden';
          }
          this.showAssignLevel(this.tableType!, this.ioTierTableSo!, 1);
          this.typePieChart();
        } else {
          this.showAssignLevel(this.ioTierTableThread!, this.ioTierTableSo!, 2);
          this.threadPieChart();
        }
      } else if (this.tabName!.textContent === 'Statistic By Function AllDuration') {
        this.showAssignLevel(this.ioTierTableSo!, this.tableFunction!, 3);
        this.libraryPieChart();
      }
    });
  }

  private hideProcess(): void {
    this.reset(this.tableType!, false);
    this.processName = '';
    this.getIOTierType(null);
  }

  private hideThread(it?: any): void {
    this.reset(this.tableType!, true);
    this.processName = '';
    this.threadName = '';
    if (it) {
      this.getIOTierType(it);
    } else {
      this.getIOTierType(null);
    }
  }

  private processPieChart(): void {
    // @ts-ignore
    this.sumDur = this.processStatisticsData.allDuration;
    this.ioPieChart!.config = {
      appendPadding: 0,
      data: this.getIOTierPieChartData(this.pidData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: this.getTip(),
      angleClick: (ioTierPieItem): void => {
        // @ts-ignore
        if (ioTierPieItem.tableName !== 'other') {
          this.ioTierProcessLevelClickEvent(ioTierPieItem);
        }
      },
      hoverHandler: (ioTierPieData): void => {
        if (ioTierPieData) {
          this.ioTierTableProcess!.setCurrentHover(ioTierPieData);
        } else {
          this.ioTierTableProcess!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.titleEl!.textContent = '';
    this.tabName!.textContent = 'Statistic By Process AllDuration';
    this.pidData.unshift(this.processStatisticsData);
    this.ioTierTableProcess!.recycleDataSource = this.pidData;
    // @ts-ignore
    this.pidData.shift(this.processStatisticsData);
    this.currentLevelData = this.pidData;
    this.ioTierTableProcess?.reMeauseHeight();
  }

  private ioTierProcessLevelClickEvent(it: any): void {
    this.reset(this.tableType!, true);
    this.getIOTierType(it);
    this.processName = it.tableName;
    this.ioPieChart?.hideTip();
    this.titleEl!.textContent = this.processName;
  }

  private typePieChart(): void {
    this.ioPieChart!.config = {
      appendPadding: 0,
      data: this.typeData,
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj): string => {
        return this.getIoTierTip(obj);
      },
      angleClick: (it): void => {
        this.ioTierTypeLevelClickEvent(it);
      },
      hoverHandler: (ioTierData): void => {
        if (ioTierData) {
          this.tableType!.setCurrentHover(ioTierData);
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
    this.titleEl!.textContent = this.processName;
    this.tabName!.textContent = 'Statistic By type AllDuration';
    this.typeData.unshift(this.typeStatisticsData);
    this.tableType!.recycleDataSource = this.typeData;
    // @ts-ignore
    this.typeData.shift(this.typeStatisticsData);
    this.currentLevelData = this.typeData;
    this.tableType?.reMeauseHeight();
  }

  private ioTierTypeLevelClickEvent(it: any): void {
    if (this.hideThreadCheckBox!.checked) {
      this.reset(this.ioTierTableSo!, true);
      this.getIOTierSo(it);
    } else {
      this.reset(this.ioTierTableThread!, true);
      this.getIOTierThread(it);
    }
    this.typeName = it.tableName;
    this.ioPieChart?.hideTip();
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.typeName.length > 0) {
      title += this.typeName;
    }
    this.titleEl!.textContent = title;
  }

  private threadPieChart(): void {
    // @ts-ignore
    this.sumDur = this.threadStatisticsData.allDuration;
    this.ioPieChart!.config = {
      appendPadding: 0,
      data: this.getIOTierPieChartData(this.threadData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj): string => {
        return this.getIoTierTip(obj);
      },
      angleClick: (it): void => {
        // @ts-ignore
        if (it.tableName !== 'other') {
          this.ioTierThreadLevelClickEvent(it);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.ioTierTableThread!.setCurrentHover(data);
        } else {
          this.ioTierTableThread!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.typeName.length > 0) {
      title += this.typeName;
    }
    this.titleEl!.textContent = title;
    this.tabName!.textContent = 'Statistic By Thread AllDuration';
    this.threadData.unshift(this.threadStatisticsData);
    this.ioTierTableThread!.recycleDataSource = this.threadData;
    // @ts-ignore
    this.threadData.shift(this.threadStatisticsData);
    this.currentLevelData = this.threadData;
    this.ioTierTableThread?.reMeauseHeight();
  }

  private getIoTierTip(obj: { obj: { tableName: any; durFormat: any; percent: any } }): string {
    return `<div>
                <div>ThreadName:${obj.obj.tableName}</div>
                <div>Duration:${obj.obj.durFormat}</div>
                <div>Percent:${obj.obj.percent}%</div> 
            </div>
        `;
  }

  private ioTierThreadLevelClickEvent(it: any): void {
    this.reset(this.ioTierTableSo!, true);
    this.getIOTierSo(it);
    this.threadName = it.tableName;
    this.ioPieChart?.hideTip();
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.typeName.length > 0) {
      title += this.typeName + ' / ';
    }
    if (this.threadName.length > 0) {
      title += this.threadName;
    }
    this.titleEl!.textContent = title;
  }

  private libraryPieChart(): void {
    // @ts-ignore
    this.sumDur = this.libStatisticsData.allDuration;
    this.ioPieChart!.config = {
      appendPadding: 0,
      data: this.getIOTierPieChartData(this.soData),
      angleField: 'duration',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (ioTierObj): string => {
        return `<div>
                    <div>Library:${ioTierObj.obj.tableName}</div>
                    <div>Duration:${ioTierObj.obj.durFormat}</div>
                    <div>Percent:${ioTierObj.obj.percent}%</div> 
                </div>
            `;
      },
      angleClick: (ioTierBean): void => {
        // @ts-ignore
        if (ioTierBean.tableName !== 'other') {
          this.ioTierSoLevelClickEvent(ioTierBean);
        }
      },
      hoverHandler: (data): void => {
        if (data) {
          this.ioTierTableSo!.setCurrentHover(data);
        } else {
          this.ioTierTableSo!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.typeName.length > 0) {
      if (this.hideThreadCheckBox?.checked) {
        title += this.typeName;
      } else {
        title += this.typeName + ' / ';
      }
    }
    if (this.threadName.length > 0) {
      title += this.threadName;
    }
    this.titleEl!.textContent = title;
    this.tabName!.textContent = 'Statistic By Library AllDuration';
    this.soData.unshift(this.libStatisticsData);
    this.ioTierTableSo!.recycleDataSource = this.soData;
    // @ts-ignore
    this.soData.shift(this.libStatisticsData);
    this.currentLevelData = this.soData;
    this.ioTierTableSo?.reMeauseHeight();
  }

  private ioTierSoLevelClickEvent(it: any): void {
    this.reset(this.tableFunction!, true);
    this.getIOTierFunction(it);
    this.ioPieChart?.hideTip();
    let title = '';
    if (this.processName.length > 0) {
      title += this.processName + ' / ';
    }
    if (this.typeName.length > 0) {
      title += this.typeName + ' / ';
    }
    if (this.threadName.length > 0 && !this.hideThreadCheckBox!.checked) {
      title += this.threadName + ' / ';
    }
    if (it.tableName.length > 0) {
      title += it.tableName;
    }
    this.titleEl!.textContent = title;
  }

  private sortByColumn(): void {
    let ioTierCurrentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        ioTierCurrentTable = this.ioTierTableProcess;
        break;
      case 1:
        ioTierCurrentTable = this.tableType;
        break;
      case 2:
        ioTierCurrentTable = this.ioTierTableThread;
        break;
      case 3:
        ioTierCurrentTable = this.ioTierTableSo;
        break;
      case 4:
        ioTierCurrentTable = this.tableFunction;
        break;
    }
    if (!ioTierCurrentTable) {
      return;
    }
    if (this.ioSortType === 0) {
      let sortZeroIoArr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          sortZeroIoArr.unshift(this.processStatisticsData);
          break;
        case 1:
          sortZeroIoArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          sortZeroIoArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          sortZeroIoArr.unshift(this.libStatisticsData);
          break;
        case 4:
          sortZeroIoArr.unshift(this.functionStatisticsData);
          break;
      }
      ioTierCurrentTable!.recycleDataSource = sortZeroIoArr;
    } else {
      let sortIoArr = [...this.currentLevelData];
      if (this.ioSortColumn === 'tableName') {
        ioTierCurrentTable!.recycleDataSource = sortIoArr.sort((firstIOElement, secondIOElement) => {
          if (this.ioSortType === 1) {
            if (firstIOElement.tableName > secondIOElement.tableName) {
              return 1;
            } else if (firstIOElement.tableName === secondIOElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (secondIOElement.tableName > firstIOElement.tableName) {
              return 1;
            } else if (firstIOElement.tableName === secondIOElement.tableName) {
              return 0;
            } else {
              return -1;
            }
          }
        });
      } else if (this.ioSortColumn === 'durFormat' || this.ioSortColumn === 'percent') {
        ioTierCurrentTable!.recycleDataSource = sortIoArr.sort((a, b) => {
          return this.ioSortType === 1 ? a.duration - b.duration : b.duration - a.duration;
        });
      }
      switch (this.currentLevel) {
        case 0:
          sortIoArr.unshift(this.processStatisticsData);
          break;
        case 1:
          sortIoArr.unshift(this.typeStatisticsData);
          break;
        case 2:
          sortIoArr.unshift(this.threadStatisticsData);
          break;
        case 3:
          sortIoArr.unshift(this.libStatisticsData);
          break;
        case 4:
          sortIoArr.unshift(this.functionStatisticsData);
          break;
      }
      ioTierCurrentTable!.recycleDataSource = sortIoArr;
    }
  }

  private getIOTierProcess(result: Array<any>): void {
    if (!this.processData || this.processData.length === 0) {
      this.pidData = [];
      this.processStatisticsData = [];
      this.processPieChart();
      return;
    }
    let allDur = 0;
    let ioMap = new Map<string, Array<number | string>>();
    for (let itemData of result) {
      allDur += itemData.dur;
      if (ioMap.has(itemData.pid)) {
        ioMap.get(itemData.pid)?.push(itemData);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(itemData);
        ioMap.set(itemData.pid, itemArray);
      }
    }
    this.pidData = [];
    ioMap.forEach((value: Array<any>, key: string) => {
      let ioPidDataDur = 0;
      let pName = '';
      for (let item of value) {
        if (item.processName && item.processName.length > 0) {
          if (!item.processName.endsWith(`(${item.pid})`)) {
            item.processName = `${item.processName}(${item.pid})`;
          }
        } else {
          item.processName = `Process(${item.pid})`;
        }
        pName = item.processName;
        ioPidDataDur += item.dur;
      }
      this.pidData.push({
        tableName: pName,
        pid: key,
        percent: ((ioPidDataDur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(ioPidDataDur),
        duration: ioPidDataDur,
      });
    });
    this.pidData.sort((a, b) => b.duration - a.duration);
    this.processStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 0;
    this.progressEL!.loading = false;
    this.processPieChart();
  }

  private getIOTierType(item: any): void {
    this.progressEL!.loading = true;
    let ioTypeMap = new Map<number, Array<number | string>>();
    let allDur = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let processItem of this.processData) {
      if (item && processItem.pid !== item.pid && !this.hideProcessCheckBox?.checked) {
        continue;
      }
      allDur += processItem.dur;
      if (ioTypeMap.has(processItem.type)) {
        ioTypeMap.get(processItem.type)?.push(processItem);
      } else {
        let itemArray = new Array<number | string>();
        itemArray.push(processItem);
        ioTypeMap.set(processItem.type, itemArray);
      }
    }
    this.typeData = [];
    ioTypeMap.forEach((value: Array<any>, key: number) => {
      let dur = 0;
      for (let ioItem of value) {
        dur += ioItem.dur;
      }
      const ioTypeData = {
        tableName: this.typeIdToString(key),
        pid: item === null ? value[0].pid : item.pid,
        tid: item === null ? value[0].tid : item.tid,
        type: key,
        percent: ((dur / allDur) * 100).toFixed(2),
        durFormat: Utils.getProbablyTime(dur),
        duration: dur,
      };
      this.typeData.push(ioTypeData);
    });
    this.typeData.sort((a, b) => b.duration - a.duration);
    this.typeStatisticsData = this.totalDurationData(allDur);
    this.currentLevel = 1;
    this.typePieChart();
    this.progressEL!.loading = false;
  }

  private getIOTierThread(item: any): void {
    this.progressEL!.loading = true;
    let threadMap = new Map<string, Array<number | string>>();
    let pid = item.pid;
    let type = item.type;
    let allDur = 0;
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let itemData of this.processData) {
      if (
        (!this.hideProcessCheckBox?.checked && itemData.pid !== pid) ||
        itemData.type !== type ||
        (itemData.type !== type && this.hideProcessCheckBox?.checked)
      ) {
        continue;
      }
      allDur += itemData.dur;
      if (threadMap.has(itemData.tid)) {
        threadMap.get(itemData.tid)?.push(itemData);
      } else {
        let itemArray = new Array<number | string>();
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
    this.threadPieChart();
  }

  private getIOTierSo(item: any): void {
    this.progressEL!.loading = true;
    let allDur = 0;
    let libMap = new Map<number, Array<number | string>>();
    if (!this.processData || this.processData.length === 0) {
      return;
    }
    for (let processItemData of this.processData) {
      if (!this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (
          item &&
          (processItemData.pid !== item.pid || processItemData.tid !== item.tid || processItemData.type !== item.type)
        ) {
          continue;
        }
      } else if (!this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (item && (processItemData.pid !== item.pid || processItemData.type !== item.type)) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if ((item && processItemData.tid !== item.tid) || processItemData.type !== item.type) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (item && processItemData.type !== item.type) {
          continue;
        }
      }
      allDur += processItemData.dur;
      if (libMap.has(processItemData.libId)) {
        libMap.get(processItemData.libId)?.push(processItemData);
      } else {
        let dataArray = new Array<number | string>();
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
          item.libName = 'unknown';
        }
        libName = item.libName;
      }
      let libPath = libName?.split('/');
      if (libPath) {
        libName = libPath[libPath.length - 1];
      }
      const soData = {
        tableName: libName,
        pid: item === null ? value[0].pid : item.pid,
        type: item === null ? value[0].type : item.type,
        tid: item === null ? value[0].tid : item.tid,
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
    this.libraryPieChart();
  }

  private getIOTierFunction(item: any): void {
    this.progressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.io-tier-subheading')!.textContent =
      'Statistic By Function AllDuration';
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
      if (!this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (
          processData.pid !== pid ||
          processData.tid !== tid ||
          processData.type !== type ||
          processData.libId !== libId
        ) {
          continue;
        }
      } else if (!this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (processData.pid !== pid || processData.type !== type || processData.libId !== libId) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && !this.hideThreadCheckBox?.checked) {
        if (processData.tid !== tid || processData.type !== type || processData.libId !== libId) {
          continue;
        }
      } else if (this.hideProcessCheckBox?.checked && this.hideThreadCheckBox?.checked) {
        if (processData.type !== type || processData.libId !== libId) {
          continue;
        }
      }
      allDur += processData.dur;
      if (symbolMap.has(processData.symbolId)) {
        symbolMap.get(processData.symbolId)?.push(processData);
      } else {
        let dataArray = new Array<number | string>();
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
        type: item.type,
        libId: item.libId,
        symbolId: key,
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
    // @ts-ignore
    this.sumDur = this.functionStatisticsData.allDuration;
    this.ioPieChart!.config = {
      appendPadding: 0,
      data: this.getIOTierPieChartData(this.functionData),
      angleField: 'duration',
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
    this.functionData.unshift(this.functionStatisticsData);
    this.tableFunction!.recycleDataSource = this.functionData;
    this.tableFunction?.reMeauseHeight();
    // @ts-ignore
    this.functionData.shift(this.functionStatisticsData);
    this.currentLevelData = this.functionData;
  }

  private getTip() {
    return (obj: { obj: { tableName: any; durFormat: any; percent: any } }): string => {
      return `<div>
                    <div>Function:${obj.obj.tableName}</div>
                    <div>Duration:${obj.obj.durFormat}</div>
                    <div>percent:${obj.obj.percent}</div>
                </div>
            `;
    };
  }

  private typeIdToString(type: number): string {
    let ioTierReleaseType: string;
    if (type === 1) {
      ioTierReleaseType = 'DATA_READ';
    } else if (type === 2) {
      ioTierReleaseType = 'DATA_WRITE';
    } else if (type === 3) {
      ioTierReleaseType = 'METADATA_READ';
    } else if (type === 4) {
      ioTierReleaseType = 'METADATA_WRITE';
    }
    // @ts-ignore
    return ioTierReleaseType;
  }

  private totalDurationData(duration: number): {
    durFormat: string;
    percent: string;
    tableName: string;
    duration: number;
    allDuration: number;
  } {
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

  private getIOTierPieChartData(res: any[]): unknown[] {
    if (res.length > 20) {
      let IOTierPieChartArr: string[] = [];
      let other: any = {
        tableName: 'other',
        duration: 0,
        percent: 0,
        durFormat: 0,
      };
      for (let i = 0; i < res.length; i++) {
        if (i < 19) {
          IOTierPieChartArr.push(res[i]);
        } else {
          other.duration += res[i].duration;
          other.durFormat = Utils.getProbablyTime(other.duration);
          other.percent = ((other.duration / this.sumDur) * 100).toFixed(2);
        }
      }
      IOTierPieChartArr.push(other);
      return IOTierPieChartArr;
    }
    return res;
  }

  private getIoTierDataByWorker(args: any[], handler: Function): void {
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

  public connectedCallback(): void {
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight !== 0) {
        this.ioTierTableProcess!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.ioTierTableProcess?.reMeauseHeight();
        this.ioTierTableThread!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.ioTierTableThread?.reMeauseHeight();
        this.ioTierTableSo!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.ioTierTableSo?.reMeauseHeight();
        this.tableFunction!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.tableFunction?.reMeauseHeight();
        this.tableType!.style.height = this.parentElement!.clientHeight - 50 + 'px';
        this.tableType?.reMeauseHeight();
        if (this.parentElement!.clientHeight >= 0 && this.parentElement!.clientHeight <= 31) {
          this.filterEl!.style.display = 'none';
        } else {
          this.filterEl!.style.display = 'flex';
        }
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
        #io-tier-chart-pie{
            height: 300px;
            margin-bottom: 31px;
        }
        .io-tier-table-box{
            width: 60%;
            border-left: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
            margin-bottom: 31px;
        }
        .io-tier-go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
            margin-left: 20px;
            visibility: hidden;
        }
        .io-tier-back-box{
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
        .io-tier-subheading{
            font-weight: bold;
            text-align: center;
        }
        .progress{
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
        <div style="display: flex;flex-direction: row;"class="d-box">
            <lit-progress-bar class="progress"></lit-progress-bar>
            <div id="left_table" style="width: 40%;height:auto;">
                <div style="display: flex;margin-bottom: 10px">
                    <div class="io-tier-go-back">
                        <div class="io-tier-back-box">
                            <lit-icon name="arrowleft"></lit-icon>
                        </div>
                    </div>
                    <div class="title"></div>
                </div>
                <div class="io-tier-subheading"></div>                       
                <lit-chart-pie  id="io-tier-chart-pie"></lit-chart-pie>     
            </div>
            <div class="io-tier-table-box" style="height:auto;overflow: auto">
                <lit-table id="tb-process-usage" style="max-height:565px;min-height: 350px">
                    <lit-table-column width="1fr" title="ProcessName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-type-usage" class="io-analysis" style="max-height:565px;min-height: 350px"hideDownload>
                    <lit-table-column width="1fr" title="Type" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-thread-usage" class="io-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                    <lit-table-column width="1fr" title="ThreadName" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                </lit-table>
                    <lit-table id="tb-so-usage" class="io-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                    <lit-table-column width="1fr" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                </lit-table>
                <lit-table id="tb-function-usage" class="io-analysis" style="max-height:565px;display: none;min-height: 350px"hideDownload>
                    <lit-table-column width="1fr" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                    <lit-table-column width="1fr" title="Duration" data-index="durFormat" key="durFormat" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="%" data-index="percent" key="percent" align="flex-start"order></lit-table-column>
                </lit-table>
            </div>
        </div>
        <tab-pane-filter id="filter" options></tab-pane-filter>
`;
  }
}
