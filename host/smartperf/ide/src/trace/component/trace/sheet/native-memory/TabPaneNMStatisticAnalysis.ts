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
import { Utils } from '../../base/Utils.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { procedurePool } from '../../../../database/Procedure.js';
const TYPE_ALLOC_STRING = 'AllocEvent';
const TYPE_MAP_STRING = 'MmapEvent';

const TYPE_ALLOC = 0;
const TYPE_MAP = 1;
const TYPE_FREE = 2;
const TYPE_UN_MAP = 3;
const PIE_CHART_LIMIT = 20;

class AnalysisObj {
  tName?: string;
  tid?: number;
  typeName?: string;
  typeId?: number;
  libName?: string;
  libId?: number;
  symbolName?: string;
  symbolId?: number;

  tableName = '';

  applySize: number;
  applySizeFormat: string;
  applyCount: number;
  releaseSize: number;
  releaseSizeFormat: string;
  releaseCount: number;
  existSize: number;
  existSizeFormat: string;
  existCount: number;

  applySizePercent?: string;
  applyCountPercent?: string;
  releaseSizePercent?: string;
  releaseCountPercent?: string;
  existSizePercent?: string;
  existCountPercent?: string;

  constructor(applySize: number, applyCount: number, releaseSize: number, releaseCount: number) {
    this.applySize = applySize;
    this.applyCount = applyCount;
    this.releaseSize = releaseSize;
    this.releaseCount = releaseCount;
    this.existSize = applySize - releaseSize;
    this.existCount = applyCount - releaseCount;
    this.applySizeFormat = Utils.getBinaryByteWithUnit(this.applySize);
    this.releaseSizeFormat = Utils.getBinaryByteWithUnit(this.releaseSize);
    this.existSizeFormat = Utils.getBinaryByteWithUnit(this.existSize);
  }
}

class SizeObj {
  applySize = 0;
  applyCount = 0;
  releaseSize = 0;
  releaseCount = 0;
}

@element('tabpane-nm-statistic-analysis')
export class TabPaneNMStatisticAnalysis extends BaseElement {
  private currentSelection: SelectionParam | any;
  private pie: LitChartPie | null | undefined;
  private processData!: Array<any>;
  private eventTypeData!: any[];
  private threadData!: any[];
  private soData!: any[];
  private functionData!: any[];
  private tableType: LitTable | null | undefined;
  private threadUsageTbl: LitTable | null | undefined;
  private soUsageTbl: LitTable | null | undefined;
  private functionUsageTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private back: HTMLDivElement | null | undefined;
  private tabName: HTMLDivElement | null | undefined;
  private progressEL: LitProgressBar | null | undefined;
  private type: string | null | undefined;
  private sortColumn: string = '';
  private sortType: number = 0;
  private isStatistic = false;
  private typeMap!: Map<number, Array<any>>;
  private currentLevel = -1;
  private currentLevelApplySize = 0;
  private currentLevelReleaseSize = 0;
  private currentLevelExistSize = 0;
  private currentLevelApplyCount = 0;
  private currentLevelReleaseCount = 0;
  private currentLevelExistCount = 0;
  private releaseLibMap!: Map<number, any>;
  private currentLevelData!: Array<any>;
  private typeStatisticsData!: {};
  private libStatisticsData!: {};
  private functionStatisticsData!: {};

  set data(statisticAnalysisParam: SelectionParam | any) {
    if (statisticAnalysisParam === this.currentSelection) {
      this.eventTypeData.unshift(this.typeStatisticsData);
      this.tableType!.recycleDataSource = this.eventTypeData;
      // @ts-ignore
      this.eventTypeData.shift(this.typeStatisticsData);
      return;
    }
    // @ts-ignore
    this.tableType?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 30 + 'px';
    // @ts-ignore
    this.soUsageTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 30 + 'px';
    // @ts-ignore
    this.functionUsageTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 30 + 'px';
    this.clearData();
    this.currentSelection = statisticAnalysisParam;
    this.tableType!.style.display = 'grid';
    this.threadUsageTbl!.style.display = 'none';
    this.soUsageTbl!.style.display = 'none';
    this.functionUsageTbl!.style.display = 'none';
    this.back!.style.visibility = 'hidden';
    this.range!.textContent =
      'Selected range: ' +
      parseFloat(((statisticAnalysisParam.rightNs - statisticAnalysisParam.leftNs) / 1000000.0).toFixed(5)) +
      ' ms';
    this.isStatistic = statisticAnalysisParam.nativeMemory.length === 0;

    this.getNMEventTypeSize(statisticAnalysisParam);
  }
  initElements(): void {
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.pie = this.shadowRoot!.querySelector<LitChartPie>('#chart-pie');
    this.tableType = this.shadowRoot!.querySelector<LitTable>('#tb-eventtype-usage');
    this.threadUsageTbl = this.shadowRoot!.querySelector<LitTable>('#tb-thread-usage');
    this.soUsageTbl = this.shadowRoot!.querySelector<LitTable>('#tb-so-usage');
    this.functionUsageTbl = this.shadowRoot!.querySelector<LitTable>('#tb-function-usage');
    this.back = this.shadowRoot!.querySelector<HTMLDivElement>('.go-back');
    this.tabName = this.shadowRoot!.querySelector<HTMLDivElement>('.subheading');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.getBack();
  }
  clearData() {
    this.pie!.dataSource = [];
    this.tableType!.recycleDataSource = [];
    this.threadUsageTbl!.recycleDataSource = [];
    this.soUsageTbl!.recycleDataSource = [];
    this.functionUsageTbl!.recycleDataSource = [];
  }
  getBack() {
    this.back!.addEventListener('click', () => {
      if (this.tabName!.textContent === 'Statistic By Library Existing') {
        this.tableType!.style.display = 'grid';
        this.soUsageTbl!.style.display = 'none';
        this.back!.style.visibility = 'hidden';
        this.soUsageTbl!.setAttribute('hideDownload', '');
        this.tableType?.removeAttribute('hideDownload');
        this.currentLevel = 0;
        this.currentLevelData = this.eventTypeData;
        this.typePieChart(this.currentSelection);
      } else if (this.tabName!.textContent === 'Statistic By Function Existing') {
        this.soUsageTbl!.style.display = 'grid';
        this.functionUsageTbl!.style.display = 'none';
        this.functionUsageTbl!.setAttribute('hideDownload', '');
        this.soUsageTbl?.removeAttribute('hideDownload');
        this.currentLevelData = this.soData;
        this.currentLevel = 1;
        this.libraryPieChart(this.currentSelection);
      }
    });
  }
  typePieChart(val: any) {
    this.pie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.eventTypeData),
      angleField: 'existSize',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (typeTipValue) => {
        return `<div>   
                        <div>Memory Type:${typeTipValue.obj.tableName}</div>
                        <div>Existing:${typeTipValue.obj.existSizeFormat} (${typeTipValue.obj.existSizePercent}%)</div>
                        <div># Existing:${typeTipValue.obj.existCount} (${typeTipValue.obj.existCountPercent}%)</div>
                        <div>Total Bytes:${typeTipValue.obj.applySizeFormat} (${typeTipValue.obj.applySizePercent}%)</div>
                        <div># Total:${typeTipValue.obj.applyCount} (${typeTipValue.obj.applyCountPercent}%)</div>
                        <div>Transient:${typeTipValue.obj.releaseSizeFormat} (${typeTipValue.obj.releaseSizePercent}%)</div>
                        <div># Transient:${typeTipValue.obj.releaseCount} (${typeTipValue.obj.releaseCountPercent}%)</div>
                        </div>`;
      },
      angleClick: (it: any) => {
        if (it.tableName != 'other') {
          this.nativeProcessLevelClickEvent(it, val);
        }
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
    this.tableType!.addEventListener('row-hover', (nmStatAnalysisTblRowHover: any) => {
      if (nmStatAnalysisTblRowHover.detail.data) {
        let data = nmStatAnalysisTblRowHover.detail.data;
        data.isHover = true;
        if ((nmStatAnalysisTblRowHover.detail as any).callBack) {
          (nmStatAnalysisTblRowHover.detail as any).callBack(true);
        }
      }
      this.pie?.showHover();
      this.pie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = '';
    this.tabName!.textContent = 'Statistic By Event Type Existing';
    this.eventTypeData.unshift(this.typeStatisticsData);
    this.tableType!.recycleDataSource = this.eventTypeData;
    this.tableType?.reMeauseHeight();
    // @ts-ignore
    this.eventTypeData.shift(this.typeStatisticsData);
    this.currentLevelData = this.eventTypeData;
  }
  nativeProcessLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.back!.style.visibility = 'visible';
    this.tableType!.style.display = 'none';
    this.soUsageTbl!.style.display = 'grid';
    this.tableType!.setAttribute('hideDownload', '');
    this.soUsageTbl?.removeAttribute('hideDownload');
    this.getLibSize(it, val);
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = it.typeName;
    this.type = it.typeName;
    this.pie?.hideTip();
  }
  threadPieChart(val: any) {
    this.pie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.threadData),
      angleField: 'existSize',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (threadTipValue) => {
        return `<div>
                        <div>Thread:${threadTipValue.obj.tableName}</div>
                        <div>Existing:${threadTipValue.obj.existSizeFormat} (${threadTipValue.obj.existSizePercent}%)</div>
                        <div># Existing:${threadTipValue.obj.existCount} (${threadTipValue.obj.existCountPercent}%)</div>
                        <div>Total Bytes:${threadTipValue.obj.applySizeFormat} (${threadTipValue.obj.applySizePercent}%)</div>
                        <div># Total:${threadTipValue.obj.applyCount} (${threadTipValue.obj.applyCountPercent}%)</div>
                        <div>Transient:${threadTipValue.obj.releaseSizeFormat} (${threadTipValue.obj.releaseSizePercent}%)</div>
                        <div># Transient:${threadTipValue.obj.releaseCount} (${threadTipValue.obj.releaseCountPercent}%)</div>
                    </div>`;
      },
      angleClick: (it: any) => {
        // @ts-ignore
        if (it.tid != 'other') {
          this.clearData();
          this.threadUsageTbl!.style.display = 'none';
          this.soUsageTbl!.style.display = 'grid';
          this.getLibSize(it, val);
          // @ts-ignore
          this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = it.type + ' / ' + 'Thread ' + it.tid;
          // @ts-ignore
          this.tid = it.tid;
          this.pie?.hideTip();
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.threadUsageTbl!.setCurrentHover(data);
        } else {
          this.threadUsageTbl!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.threadUsageTbl!.addEventListener('row-hover', (nmStatAnalysisThreadRowHover: any) => {
      if (nmStatAnalysisThreadRowHover.detail.data) {
        let data = nmStatAnalysisThreadRowHover.detail.data;
        data.isHover = true;
        if ((nmStatAnalysisThreadRowHover.detail as any).callBack) {
          (nmStatAnalysisThreadRowHover.detail as any).callBack(true);
        }
      }
      this.pie?.showHover();
      this.pie?.hideTip();
    });
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.type + '';
    this.tabName!.textContent = 'Statistic By Thread Existing';
    this.threadUsageTbl!.recycleDataSource = this.threadData;
    this.threadUsageTbl?.reMeauseHeight();
  }
  libraryPieChart(val: any) {
    this.pie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.soData),
      angleField: 'existSize',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (libraryTipValue) => {
        return `<div>
                        <div>Library:${libraryTipValue.obj.libName}</div>
                        <div>Existing:${libraryTipValue.obj.existSizeFormat} (${libraryTipValue.obj.existSizePercent}%)</div>
                        <div># Existing:${libraryTipValue.obj.existCount} (${libraryTipValue.obj.existCountPercent}%)</div>
                        <div>Total Bytes:${libraryTipValue.obj.applySizeFormat} (${libraryTipValue.obj.applySizePercent}%)</div>
                        <div># Total:${libraryTipValue.obj.applyCount} (${libraryTipValue.obj.applyCountPercent}%)</div>
                        <div>Transient:${libraryTipValue.obj.releaseSizeFormat} (${libraryTipValue.obj.releaseSizePercent}%)</div>
                        <div># Transient:${libraryTipValue.obj.releaseCount} (${libraryTipValue.obj.releaseCountPercent}%)</div>
                    </div>`;
      },
      angleClick: (it: any) => {
        // @ts-ignore
        if (it.tableName != 'other') {
          this.nativeSoLevelClickEvent(it, val);
        }
      },
      hoverHandler: (data) => {
        if (data) {
          this.soUsageTbl!.setCurrentHover(data);
        } else {
          this.soUsageTbl!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.type + '';
    this.soUsageTbl!.addEventListener('row-hover', (nmStatAnalysisUsageRowHover: any) => {
      if (nmStatAnalysisUsageRowHover.detail.data) {
        let data = nmStatAnalysisUsageRowHover.detail.data;
        data.isHover = true;
        if ((nmStatAnalysisUsageRowHover.detail as any).callBack) {
          (nmStatAnalysisUsageRowHover.detail as any).callBack(true);
        }
      }
      this.pie?.showHover();
      this.pie?.hideTip();
    });
    this.tabName!.textContent = 'Statistic By Library Existing';
    this.soData.unshift(this.libStatisticsData);
    this.soUsageTbl!.recycleDataSource = this.soData;
    // @ts-ignore
    this.soData.shift(this.libStatisticsData);
    this.currentLevelData = this.soData;
    this.soUsageTbl?.reMeauseHeight();
    this.soUsageTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.soUsageTbl!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.existSize !== 0) {
        this.nativeSoLevelClickEvent(data, val);
      }
    });
  }
  nativeSoLevelClickEvent(it: any, val: any) {
    this.clearData();
    this.soUsageTbl!.style.display = 'none';
    this.functionUsageTbl!.style.display = 'grid';
    this.soUsageTbl!.setAttribute('hideDownload', '');
    this.functionUsageTbl?.removeAttribute('hideDownload');
    this.getNMFunctionSize(it, val);
    this.shadowRoot!.querySelector<HTMLDivElement>('.title')!.textContent = this.type + ' / ' + it.libName;
    this.pie?.hideTip();
  }
  functionPieChart(val: any) {
    this.pie!.config = {
      appendPadding: 0,
      data: this.getPieChartData(this.functionData),
      angleField: 'existSize',
      colorField: 'tableName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (functionTipValue) => {
        return `<div>
                        <div>Function:${functionTipValue.obj.symbolName}</div>
                        <div>Existing:${functionTipValue.obj.existSizeFormat} (${functionTipValue.obj.existSizePercent}%)</div>
                        <div># Existing:${functionTipValue.obj.existCount} (${functionTipValue.obj.existCountPercent}%)</div>
                        <div>Total Bytes:${functionTipValue.obj.applySizeFormat} (${functionTipValue.obj.applySizePercent}%)</div>
                        <div># Total:${functionTipValue.obj.applyCount} (${functionTipValue.obj.applyCountPercent}%)</div>
                        <div>Transient:${functionTipValue.obj.releaseSizeFormat} (${functionTipValue.obj.releaseSizePercent}%)</div>
                        <div># Transient:${functionTipValue.obj.releaseCount} (${functionTipValue.obj.releaseCountPercent}%)</div>
                    </div>`;
      },
      hoverHandler: (data) => {
        if (data) {
          this.functionUsageTbl!.setCurrentHover(data);
        } else {
          this.functionUsageTbl!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.functionUsageTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.pie?.showHover();
      this.pie?.hideTip();
    });
    this.functionData.unshift(this.functionStatisticsData);
    this.functionUsageTbl!.recycleDataSource = this.functionData;
    // @ts-ignore
    this.functionData.shift(this.functionStatisticsData);
    this.currentLevelData = this.functionData;
    this.functionUsageTbl?.reMeauseHeight();
    this.functionUsageTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }

  sortByColumn(column: string, sort: number) {
    this.sortColumn = column;
    this.sortType = sort;
    let currentTable: LitTable | null | undefined;
    switch (this.currentLevel) {
      case 0:
        currentTable = this.tableType;
        break;
      case 1:
        currentTable = this.soUsageTbl;
        break;
      case 2:
        currentTable = this.functionUsageTbl;
        break;
    }
    if (!currentTable) {
      return;
    }
    if (sort === 0) {
      let arr = [...this.currentLevelData];
      switch (this.currentLevel) {
        case 0:
          arr.unshift(this.typeStatisticsData);
          break;
        case 1:
          arr.unshift(this.libStatisticsData);
          break;
        case 2:
          arr.unshift(this.functionStatisticsData);
          break;
      }
      currentTable!.recycleDataSource = arr;
    } else {
      let arr = [...this.currentLevelData];
      switch (column) {
        case 'tableName':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            if (sort === 1) {
              if (statisticAnalysisLeftData.tableName > statisticAnalysisRightData.tableName) {
                return 1;
              } else if (statisticAnalysisLeftData.tableName === statisticAnalysisRightData.tableName) {
                return 0;
              } else {
                return -1;
              }
            } else {
              if (statisticAnalysisRightData.tableName > statisticAnalysisLeftData.tableName) {
                return 1;
              } else if (statisticAnalysisLeftData.tableName === statisticAnalysisRightData.tableName) {
                return 0;
              } else {
                return -1;
              }
            }
          });
          break;
        case 'existSizeFormat':
        case 'existSizePercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.existSize - statisticAnalysisRightData.existSize
              : statisticAnalysisRightData.existSize - statisticAnalysisLeftData.existSize;
          });
          break;
        case 'existCount':
        case 'existCountPercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.existCount - statisticAnalysisRightData.existCount
              : statisticAnalysisRightData.existCount - statisticAnalysisLeftData.existCount;
          });
          break;
        case 'releaseSizeFormat':
        case 'releaseSizePercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.releaseSize - statisticAnalysisRightData.releaseSize
              : statisticAnalysisRightData.releaseSize - statisticAnalysisLeftData.releaseSize;
          });
          break;
        case 'releaseCount':
        case 'releaseCountPercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.releaseCount - statisticAnalysisRightData.releaseCount
              : statisticAnalysisRightData.releaseCount - statisticAnalysisLeftData.releaseCount;
          });
          break;
        case 'applySizeFormat':
        case 'applySizePercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.applySize - statisticAnalysisRightData.applySize
              : statisticAnalysisRightData.applySize - statisticAnalysisLeftData.applySize;
          });
          break;
        case 'applyCount':
        case 'applyCountPercent':
          currentTable!.recycleDataSource = arr.sort((statisticAnalysisLeftData, statisticAnalysisRightData) => {
            return sort === 1
              ? statisticAnalysisLeftData.applyCount - statisticAnalysisRightData.applyCount
              : statisticAnalysisRightData.applyCount - statisticAnalysisLeftData.applyCount;
          });
          break;
      }
      switch (this.currentLevel) {
        case 0:
          arr.unshift(this.typeStatisticsData);
          break;
        case 1:
          arr.unshift(this.libStatisticsData);
          break;
        case 2:
          arr.unshift(this.functionStatisticsData);
          break;
      }
      currentTable!.recycleDataSource = arr;
    }
  }

  getNMEventTypeSize(val: any) {
    this.progressEL!.loading = true;
    let typeFilter = [];
    if (this.isStatistic) {
      for (let type of val.nativeMemoryStatistic) {
        if (type === 'All Heap & Anonymous VM') {
          typeFilter = [0, 1];
          break;
        } else if (type === 'All Heap') {
          typeFilter.push(0);
        } else {
          typeFilter.push(1);
        }
      }
      this.getDataFromWorker(val, typeFilter);
    } else {
      for (let type of val.nativeMemory) {
        if (type === 'All Heap & Anonymous VM') {
          typeFilter = [];
          typeFilter.push(...["'AllocEvent'", "'FreeEvent'", "'MmapEvent'", "'MunmapEvent'"]);
          break;
        } else if (type === 'All Heap') {
          typeFilter.push(...["'AllocEvent'", "'FreeEvent'"]);
        } else {
          typeFilter.push(...["'MmapEvent'", "'MunmapEvent'"]);
        }
      }
      this.getDataFromWorker(val, typeFilter);
    }

    this.tableType!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
    this.tableType!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      if (data.tableName !== '' && data.existSize !== 0) {
        this.nativeProcessLevelClickEvent(data, val);
      }
    });
    new ResizeObserver(() => {
      // @ts-ignore
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.tableType?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 30 + 'px';
        this.tableType?.reMeauseHeight();
        // @ts-ignore
        this.soUsageTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 30 + 'px';
        this.soUsageTbl?.reMeauseHeight();
        // @ts-ignore
        this.functionUsageTbl?.shadowRoot?.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 30 + 'px';
        this.functionUsageTbl?.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  private calTypeSize(val: any, result: any) {
    this.processData = JSON.parse(JSON.stringify(result));
    this.resetCurrentLevelData();
    this.typeMap = this.typeSizeGroup(this.processData);
    this.currentLevelExistSize = this.currentLevelApplySize - this.currentLevelReleaseSize;
    this.currentLevelExistCount = this.currentLevelApplyCount - this.currentLevelReleaseCount;
    this.eventTypeData = [];
    if (this.typeMap.has(TYPE_ALLOC)) {
      let allocType = this.setTypeMap(this.typeMap, TYPE_ALLOC, TYPE_ALLOC_STRING);
      if (allocType) {
        this.calPercent(allocType);
        this.eventTypeData.push(allocType);
      }
    }
    if (this.typeMap.has(TYPE_MAP)) {
      let subTypeMap = new Map<string, Array<any>>();
      for (let item of this.typeMap.get(TYPE_MAP)!) {
        if (item.subType) {
          if (subTypeMap.has(item.subType)) {
            subTypeMap.get(item.subType)?.push(item);
          } else {
            let dataArray = new Array<any>();
            dataArray.push(item);
            subTypeMap.set(item.subType, dataArray);
          }
        } else {
          if (subTypeMap.has(TYPE_MAP_STRING)) {
            subTypeMap.get(TYPE_MAP_STRING)?.push(item);
          } else {
            let dataArray = new Array<any>();
            dataArray.push(item);
            subTypeMap.set(TYPE_MAP_STRING, dataArray);
          }
        }
      }
      subTypeMap.forEach((arr: Array<any>, subType: any) => {
        let mapType = this.setTypeMap(this.typeMap, TYPE_MAP, subType);
        if (mapType) {
          this.calPercent(mapType);
          this.eventTypeData.push(mapType);
        }
      });
    }
    this.eventTypeData.sort((a, b) => b.existSize - a.existSize);
    this.typeStatisticsData = this.totalData(this.typeStatisticsData);
    this.progressEL!.loading = false;
    this.currentLevel = 0;
    this.typePieChart(val);
  }

  getNMThreadSize(item: any, val: any) {
    this.progressEL!.loading = true;
    let threadMap = new Map<number, Array<any>>();
    let types = this.getTypes(item);
    this.resetCurrentLevelData(item);

    for (let itemData of this.processData) {
      // @ts-ignore
      if (!types.includes(itemData.type)) {
        continue;
      }
      if (threadMap.has(itemData.tid)) {
        threadMap.get(itemData.tid)?.push(itemData);
      } else {
        let itemArray = new Array<any>();
        itemArray.push(itemData);
        threadMap.set(itemData.tid, itemArray);
      }
    }
    this.threadData = [];
    threadMap.forEach((dbData: Array<any>, tid: number) => {
      const sizeObj = this.calSizeObj(dbData);
      let analysis = new AnalysisObj(sizeObj.applySize, sizeObj.applyCount, sizeObj.releaseSize, sizeObj.releaseCount);
      this.calPercent(analysis);
      analysis.typeId = item.typeId;
      analysis.typeName = item.typeName;
      analysis.tid = tid;
      analysis.tName = 'Thread ' + tid;
      analysis.tableName = analysis.tName;
      this.threadData.push(analysis);
    });
    this.threadData.sort((a, b) => b.existSize - a.existSize);
    this.currentLevelData = this.threadData;
    this.progressEL!.loading = false;
    this.threadPieChart(val);
    this.threadUsageTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }

  getLibSize(item: any, val: any) {
    this.progressEL!.loading = true;
    let typeId = item.typeId;
    let typeName = item.typeName;
    let tid = item.tid;
    let libMap = new Map<number, Array<any>>();
    this.resetCurrentLevelData(item);
    let types = this.getTypes(item);
    this.soData = [];
    if (!this.processData) return;
    for (let itemData of this.processData) {
      if (typeName === TYPE_ALLOC_STRING) {
        // @ts-ignore
        if (!types.includes(itemData.type)) {
          continue;
        }
      } else if (typeName === TYPE_MAP_STRING) {
        if (this.isStatistic) {
          if (itemData.subType) {
            // @ts-ignore
            if (!types.includes(itemData.subType) || !types.includes(itemData.type)) {
              continue;
            }
          } else {
            continue;
          }
        } else {
          if (!itemData.subType) {
            // @ts-ignore
            if (!types.includes(itemData.type)) {
              continue;
            }
          } else {
            continue;
          }
        }
      } else {
        if (itemData.subType) {
          // @ts-ignore
          if (!types.includes(itemData.subType) || !types.includes(itemData.type)) {
            continue;
          }
        } else {
          continue;
        }
      }
      let libId = itemData.libId;

      if (libMap.has(libId)) {
        libMap.get(libId)?.push(itemData);
      } else {
        let dataArray = new Array<any>();
        dataArray.push(itemData);
        libMap.set(libId, dataArray);
      }
    }
    libMap.forEach((libItems, libId) => {
      let libPath = SpSystemTrace.DATA_DICT.get(libId)?.split('/');
      let libName = '';
      if (libPath) {
        libName = libPath[libPath.length - 1];
      }
      const sizeObj = this.calSizeObj(libItems);
      let analysis = new AnalysisObj(sizeObj.applySize, sizeObj.applyCount, sizeObj.releaseSize, sizeObj.releaseCount);
      this.calPercent(analysis);
      analysis.typeId = typeId;
      analysis.typeName = typeName;
      analysis.tid = tid;
      analysis.tName = 'Thread ' + tid;
      analysis.libId = libId;
      analysis.libName = libName;
      analysis.tableName = analysis.libName;
      this.soData.push(analysis);
    });
    this.soData.sort((a, b) => b.existSize - a.existSize);
    this.libStatisticsData = this.totalData(this.libStatisticsData);
    this.currentLevel = 1;
    this.libraryPieChart(val);
    this.progressEL!.loading = false;
  }

  getNMFunctionSize(item: any, val: any) {
    this.progressEL!.loading = true;
    this.shadowRoot!.querySelector<HTMLDivElement>('.subheading')!.textContent = 'Statistic By Function Existing';
    let typeId = item.typeId;
    let typeName = item.typeName;
    let tid = item.tid;
    let libId = item.libId;
    let symbolMap = new Map<number, Array<any>>();
    this.resetCurrentLevelData(item);
    let types = this.getTypes(item);
    if (!this.processData) {
      return;
    }
    for (let data of this.processData) {
      if (typeName === TYPE_ALLOC_STRING) {
        // @ts-ignore
        if (!types.includes(data.type) || data.libId !== libId) {
          continue;
        }
      } else if (typeName === TYPE_MAP_STRING) {
        if (this.isStatistic) {
          if (data.subType) {
            // @ts-ignore
            if (!types.includes(data.subType) || !types.includes(data.type) || data.libId !== libId) {
              continue;
            }
          } else {
            continue;
          }
        } else {
          if (!data.subType) {
            // @ts-ignore
            if (!types.includes(data.type) || data.libId !== libId) {
              continue;
            }
          } else {
            continue;
          }
        }
      } else {
        if (data.subType) {
          // @ts-ignore
          if (!types.includes(data.subType) || !types.includes(data.type) || data.libId !== libId) {
            continue;
          }
        } else {
          continue;
        }
      }
      if (symbolMap.has(data.symbolId)) {
        symbolMap.get(data.symbolId)?.push(data);
      } else {
        let dataArray = new Array<any>();
        dataArray.push(data);
        symbolMap.set(data.symbolId, dataArray);
      }
    }

    this.functionData = [];
    symbolMap.forEach((symbolItems, symbolId) => {
      let symbolPath = SpSystemTrace.DATA_DICT.get(symbolId)?.split('/');
      let symbolName = symbolPath ? symbolPath[symbolPath.length - 1] : 'null';
      const sizeObj = this.calSizeObj(symbolItems);
      let analysis = new AnalysisObj(sizeObj.applySize, sizeObj.applyCount, sizeObj.releaseSize, sizeObj.releaseCount);
      this.calPercent(analysis);
      analysis.typeId = typeId;
      analysis.typeName = typeName;
      analysis.tid = tid;
      analysis.tName = 'Thread ' + tid;
      analysis.libId = libId;
      analysis.libName = item.libName;
      analysis.symbolId = symbolId;
      analysis.symbolName = symbolName;
      analysis.tableName = analysis.symbolName;
      this.functionData.push(analysis);
    });
    this.functionData.sort((a, b) => b.existSize - a.existSize);
    // @ts-ignore
    this.functionStatisticsData = this.totalData(this.functionStatisticsData);
    this.currentLevel = 2;
    this.progressEL!.loading = false;
    this.functionPieChart(val);
  }

  getPieChartData(res: any[]) {
    if (res.length > PIE_CHART_LIMIT) {
      let pieChartArr: any[] = [];
      let other: any = {
        tableName: 'other',
        symbolName: 'other',
        existSizePercent: 0,
        libName: 'other',
        existSize: 0,
        existSizeFormat: '',
        existCount: 0,
        countPercent: 'other',
        existCountPercent: 0,
      };
      for (let i = 0; i < res.length; i++) {
        if (i < PIE_CHART_LIMIT - 1) {
          pieChartArr.push(res[i]);
        } else {
          other.existCount += res[i].existCount;
          other.existSize += res[i].existSize;
          other.existSizeFormat = Utils.getBinaryByteWithUnit(other.existSize);
          other.existSizePercent = ((other.existSize / this.currentLevelExistSize) * 100).toFixed(2);
          other.existCountPercent = ((other.existCount / this.currentLevelExistCount) * 100).toFixed(2);
        }
      }
      pieChartArr.push(other);
      return pieChartArr;
    }
    return res;
  }

  setTypeMap(typeMap: Map<number, any>, tyeId: number, typeName: string): AnalysisObj | null {
    let applySize = 0;
    let releaseSize = 0;
    let applyCount = 0;
    let releaseCount = 0;
    let currentType = typeMap.get(tyeId);
    if (!currentType) {
      return null;
    }

    for (let applySample of typeMap.get(tyeId)!) {
      if (tyeId === TYPE_ALLOC) {
        applySize += applySample.size;
        applyCount += applySample.count;
        if (this.isStatistic) {
          releaseSize += applySample.releaseSize;
          releaseCount += applySample.releaseCount;
        } else {
          if (applySample.isRelease) {
            releaseSize += applySample.size;
            releaseCount += applySample.count;
          }
        }
      } else {
        if (applySample.subType) {
          if (applySample.subType === typeName) {
            applySize += applySample.size;
            applyCount += applySample.count;
            if (this.isStatistic) {
              releaseSize += applySample.releaseSize;
              releaseCount += applySample.releaseCount;
            } else {
              if (applySample.isRelease) {
                releaseSize += applySample.size;
                releaseCount += applySample.count;
              }
            }
          }
        } else {
          if (typeName === TYPE_MAP_STRING) {
            applySize += applySample.size;
            applyCount += applySample.count;
            if (this.isStatistic) {
              releaseSize += applySample.releaseSize;
              releaseCount += applySample.releaseCount;
            } else {
              if (applySample.isRelease) {
                releaseSize += applySample.size;
                releaseCount += applySample.count;
              }
            }
          }
        }
      }
    }
    let typeItem = new AnalysisObj(applySize, applyCount, releaseSize, releaseCount);
    typeItem.typeId = tyeId;
    typeItem.typeName = typeName;
    typeItem.tableName = typeName;
    return typeItem;
  }

  private calPercent(item: AnalysisObj) {
    item.applySizePercent = ((item.applySize / this.currentLevelApplySize) * 100).toFixed(2);
    item.applyCountPercent = ((item.applyCount / this.currentLevelApplyCount) * 100).toFixed(2);
    item.releaseSizePercent = ((item.releaseSize / this.currentLevelReleaseSize) * 100).toFixed(2);
    item.releaseCountPercent = ((item.releaseCount / this.currentLevelReleaseCount) * 100).toFixed(2);
    item.existSizePercent = ((item.existSize / this.currentLevelExistSize) * 100).toFixed(2);
    item.existCountPercent = ((item.existCount / this.currentLevelExistCount) * 100).toFixed(2);
  }

  private resetCurrentLevelData(parent?: any) {
    if (parent) {
      this.currentLevelApplySize = parent.applySize;
      this.currentLevelApplyCount = parent.applyCount;
      this.currentLevelExistSize = parent.existSize;
      this.currentLevelExistCount = parent.existCount;
      this.currentLevelReleaseSize = parent.releaseSize;
      this.currentLevelReleaseCount = parent.releaseCount;
    } else {
      this.currentLevelApplySize = 0;
      this.currentLevelApplyCount = 0;
      this.currentLevelExistSize = 0;
      this.currentLevelExistCount = 0;
      this.currentLevelReleaseSize = 0;
      this.currentLevelReleaseCount = 0;
    }
  }

  private typeSizeGroup(dbArray: Array<any>): Map<number, Array<any>> {
    let typeMap = new Map<number, Array<any>>();
    if (!dbArray || dbArray.length === 0) {
      return typeMap;
    }

    let that = this;
    function setSize(item: any) {
      that.currentLevelApplySize += item.size;
      that.currentLevelApplyCount += item.count;
      if (that.isStatistic) {
        that.currentLevelReleaseSize += item.releaseSize;
        that.currentLevelReleaseCount += item.releaseCount;
      } else {
        if (item.isRelease) {
          that.currentLevelReleaseSize += item.size;
          that.currentLevelReleaseCount += item.count;
        }
      }
    }

    for (let itemData of dbArray) {
      switch (itemData.type) {
        case TYPE_ALLOC:
          setSize(itemData);
          if (typeMap.has(TYPE_ALLOC)) {
            typeMap.get(TYPE_ALLOC)?.push(itemData);
          } else {
            let itemArray = new Array<any>();
            itemArray.push(itemData);
            typeMap.set(TYPE_ALLOC, itemArray);
          }
          break;
        case TYPE_MAP:
          setSize(itemData);
          if (typeMap.has(TYPE_MAP)) {
            typeMap.get(TYPE_MAP)?.push(itemData);
          } else {
            let itemArray = new Array<any>();
            itemArray.push(itemData);
            typeMap.set(TYPE_MAP, itemArray);
          }
          break;
      }
    }
    return typeMap;
  }

  totalData(total: any) {
    total = {
      existSizeFormat: Utils.getBinaryByteWithUnit(this.currentLevelExistSize),
      existSizePercent: ((this.currentLevelExistSize / this.currentLevelExistSize) * 100).toFixed(2),
      existCount: this.currentLevelExistCount,
      existCountPercent: ((this.currentLevelExistCount / this.currentLevelExistCount) * 100).toFixed(2),
      releaseSizeFormat: Utils.getBinaryByteWithUnit(this.currentLevelReleaseSize),
      releaseSizePercent: ((this.currentLevelReleaseSize / this.currentLevelReleaseSize) * 100).toFixed(2),
      releaseCount: this.currentLevelReleaseCount,
      releaseCountPercent: ((this.currentLevelReleaseCount / this.currentLevelReleaseCount) * 100).toFixed(2),
      applySizeFormat: Utils.getBinaryByteWithUnit(this.currentLevelApplySize),
      applySizePercent: ((this.currentLevelApplySize / this.currentLevelApplySize) * 100).toFixed(2),
      applyCount: this.currentLevelApplyCount,
      applyCountPercent: ((this.currentLevelApplyCount / this.currentLevelApplyCount) * 100).toFixed(2),
      existSize: 0,
      tableName: '',
      libName: '',
      symbolName: '',
    };
    return total;
  }

  calSizeObj(dbData: Array<any>) {
    let sizeObj = new SizeObj();
    for (let item of dbData) {
      if (this.isStatistic) {
        sizeObj.applyCount += item.count;
        sizeObj.applySize += item.size;
        sizeObj.releaseCount += item.releaseCount;
        sizeObj.releaseSize += item.releaseSize;
      } else {
        // @ts-ignore
        sizeObj.applyCount += item.count;
        sizeObj.applySize += item.size;
        if (item.isRelease) {
          sizeObj.releaseCount += item.count;
          sizeObj.releaseSize += item.size;
        }
      }
    }
    return sizeObj;
  }

  getTypes(parent: AnalysisObj) {
    let types = new Array<any>();
    types.push(parent.typeId!);
    types.push(parent.typeName!);
    if (!this.isStatistic) {
      let releaseType;
      if (parent.typeId === TYPE_ALLOC) {
        releaseType = TYPE_FREE;
      } else {
        releaseType = TYPE_UN_MAP;
      }
      types.push(releaseType);
    }
    return types;
  }

  getDataFromWorker(val: SelectionParam | any, typeFilter: Array<number | string>) {
    this.getDataByWorkerQuery(
      {
        leftNs: val.leftNs,
        rightNs: val.rightNs,
        types: typeFilter,
        isStatistic: this.isStatistic,
      },
      (results: any) => {
        this.calTypeSize(val, results);
      }
    );
  }

  getDataByWorkerQuery(args: any, handler: Function) {
    this.progressEL!.loading = true;
    procedurePool.submitWithName('logic1', 'native-memory-queryAnalysis', args, undefined, (results: any) => {
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
                     <div class="table-box" style="height:auto;">
                    <lit-table id="tb-eventtype-usage" style="max-height: 380px">
                        <lit-table-column width="250px" title="Memory Type" data-index="tableName" key="tableName" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="Existing" data-index="existSizeFormat" key="existSizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existSizePercent" key="existSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Existing" data-index="existCount" key="existCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existCountPercent" key="existCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Total Bytes" data-index="applySizeFormat" key="applySizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applySizePercent" key="applySizePercent" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="# Total" data-index="applyCount" key="applyCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applyCountPercent" key="applyCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Transient" data-index="releaseSizeFormat" key="releaseSizeFormat" align="flex-start"order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseSizePercent" key="releaseSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Transient" data-index="releaseCount" key="releaseCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseCountPercent" key="releaseCountPercent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-thread-usage" style="display: none;max-height: 380px"hideDownload>
                        <lit-table-column width="100px" title="Memory Type" data-index="tableName" key="tableName" align="flex-start" ></lit-table-column>
                        <lit-table-column width="100px" title="Existing" data-index="existSizeFormat" key="existSizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existSizePercent" key="existSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Existing" data-index="existCount" key="existCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existCountPercent" key="existCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Total Bytes" data-index="applySizeFormat" key="applySizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applySizePercent" key="applySizePercent" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="# Total" data-index="applyCount" key="applyCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applyCountPercent" key="applyCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Transient" data-index="releaseSizeFormat" key="releaseSizeFormat" align="flex-start"order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseSizePercent" key="releaseSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Transient" data-index="releaseCount" key="releaseCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseCountPercent" key="releaseCountPercent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-so-usage" style="display: none;max-height: 380px"hideDownload>
                        <lit-table-column width="250px" title="Library" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Existing" data-index="existSizeFormat" key="existSizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existSizePercent" key="existSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Existing" data-index="existCount" key="existCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existCountPercent" key="existCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Total Bytes" data-index="applySizeFormat" key="applySizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applySizePercent" key="applySizePercent" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="# Total" data-index="applyCount" key="applyCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applyCountPercent" key="applyCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Transient" data-index="releaseSizeFormat" key="releaseSizeFormat" align="flex-start"order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseSizePercent" key="releaseSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Transient" data-index="releaseCount" key="releaseCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseCountPercent" key="releaseCountPercent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    <lit-table id="tb-function-usage" style="display: none;max-height: 380px"hideDownload>
                        <lit-table-column width="250px" title="Function" data-index="tableName" key="tableName" align="flex-start"order></lit-table-column>
                         <lit-table-column width="100px" title="Existing" data-index="existSizeFormat" key="existSizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existSizePercent" key="existSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Existing" data-index="existCount" key="existCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="existCountPercent" key="existCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Total Bytes" data-index="applySizeFormat" key="applySizeFormat" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applySizePercent" key="applySizePercent" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="# Total" data-index="applyCount" key="applyCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="applyCountPercent" key="applyCountPercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="Transient" data-index="releaseSizeFormat" key="releaseSizeFormat" align="flex-start"order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseSizePercent" key="releaseSizePercent" align="flex-start"order></lit-table-column>
                        <lit-table-column width="100px" title="# Transient" data-index="releaseCount" key="releaseCount" align="flex-start" order></lit-table-column>
                        <lit-table-column width="50px" title="%" data-index="releaseCountPercent" key="releaseCountPercent" align="flex-start"order></lit-table-column>
                    </lit-table>
                    </div>
        </div>
`;
  }
}
