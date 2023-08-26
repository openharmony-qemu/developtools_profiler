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
import '../TabPaneFilter.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { PerfCallChainMerageData } from '../../../../bean/PerfProfile.js';
import '../../../chart/FrameChart.js';
import { FrameChart } from '../../../chart/FrameChart.js';
import { ChartMode } from '../../../../bean/FrameChartStruct.js';
import '../../../DisassemblingWindow.js';
import { DisassemblingWindow } from '../../../DisassemblingWindow.js';
import { Cmd } from '../../../../../command/Cmd.js';
import { SpApplication } from '../../../../SpApplication.js';
import '../../../../../base-ui/slicer/lit-slicer.js';
import '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { showButtonMenu } from '../SheetUtils.js';
import { SpHiPerf } from '../../../chart/SpHiPerf.js';

@element('tabpane-perf-profile')
export class TabpanePerfProfile extends BaseElement {
  private perfProfilerTbl: LitTable | null | undefined;
  private perfProfilerList: LitTable | null | undefined;
  private perfProfileProgressEL: LitProgressBar | null | undefined;
  private perfProfilerRightSource: Array<PerfCallChainMerageData> = [];
  private perfProfilerFilter: any;
  private perfProfilerDataSource: any[] = [];
  private perfProfileSortKey = 'weight';
  private perfProfileSortType = 0;
  private perfSelectedData: any = undefined;
  private perfProfileFrameChart: FrameChart | null | undefined;
  private isChartShow: boolean = false;
  private systemRuleName = '/system/';
  private perfProfileNumRuleName = '/max/min/';
  private perfProfilerModal: DisassemblingWindow | null | undefined;
  private needShowMenu = true;
  private searchValue: string = '';
  private perfProfileLoadingList: number[] = [];
  private perfProfileLoadingPage: any;
  private currentSelection: SelectionParam | undefined;

  set data(perfProfilerSelection: SelectionParam | any) {
    if (perfProfilerSelection == this.currentSelection) {
      return;
    }
    this.searchValue = '';
    this.currentSelection = perfProfilerSelection;
    this.perfProfilerModal!.style.display = 'none';
    this.perfProfilerTbl!.style.visibility = 'visible';
    if (this.parentElement!.clientHeight > this.perfProfilerFilter!.clientHeight) {
      this.perfProfilerFilter!.style.display = 'flex';
    } else {
      this.perfProfilerFilter!.style.display = 'none';
    }
    this.perfProfilerFilter!.initializeFilterTree(true, true, true);
    this.perfProfilerFilter!.filterValue = '';
    this.perfProfileProgressEL!.loading = true;
    this.perfProfileLoadingPage.style.visibility = 'visible';
    this.getDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [perfProfilerSelection],
        },
      ],
      (results: any[]) => {
        this.setPerfProfilerLeftTableData(results);
        this.perfProfilerList!.recycleDataSource = [];
        this.perfProfileFrameChart!.mode = ChartMode.Count;
        this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
        this.perfProfileFrameChart?.updateCanvas(true, this.clientWidth);
        this.perfProfileFrameChart?.calculateChartData();
        this.switchFlameChart();
        this.perfProfilerFilter.icon = 'block';
      }
    );
  }

  getParentTree(
    perfCallSrc: Array<PerfCallChainMerageData>,
    target: PerfCallChainMerageData,
    parentsData: Array<PerfCallChainMerageData>
  ): boolean {
    for (let perfCall of perfCallSrc) {
      if (perfCall.id == target.id) {
        parentsData.push(perfCall);
        return true;
      } else {
        if (this.getParentTree(perfCall.children as Array<PerfCallChainMerageData>, target, parentsData)) {
          parentsData.push(perfCall);
          return true;
        }
      }
    }
    return false;
  }

  getChildTree(
    perfCallSrc: Array<PerfCallChainMerageData>,
    id: string,
    children: Array<PerfCallChainMerageData>
  ): boolean {
    for (let perfCall of perfCallSrc) {
      if (perfCall.id == id && perfCall.children.length == 0) {
        children.push(perfCall);
        return true;
      } else {
        if (this.getChildTree(perfCall.children as Array<PerfCallChainMerageData>, id, children)) {
          children.push(perfCall);
          return true;
        }
      }
    }
    return false;
  }

  setRightTableData(chainMerageData: PerfCallChainMerageData) {
    let parentsMerageData: Array<PerfCallChainMerageData> = [];
    let childrenMerageData: Array<PerfCallChainMerageData> = [];
    this.getParentTree(this.perfProfilerDataSource, chainMerageData, parentsMerageData);
    let maxId = chainMerageData.id;
    let maxDur = 0;

    function findMaxStack(call: PerfCallChainMerageData) {
      if (call.children.length == 0) {
        if (call.dur > maxDur) {
          maxDur = call.dur;
          maxId = call.id;
        }
      } else {
        call.children.map((callChild) => {
          findMaxStack(<PerfCallChainMerageData>callChild);
        });
      }
    }

    findMaxStack(chainMerageData);
    this.getChildTree(chainMerageData.children as Array<PerfCallChainMerageData>, maxId, childrenMerageData);
    let perfProfileParentsList = parentsMerageData.reverse().concat(childrenMerageData.reverse());
    for (let data of perfProfileParentsList) {
      data.type =
        data.libName.endsWith('.so.1') || data.libName.endsWith('.dll') || data.libName.endsWith('.so') ? 0 : 1;
    }
    let len = perfProfileParentsList.length;
    this.perfProfilerRightSource = perfProfileParentsList;
    let rightSource: Array<any> = [];
    if (len != 0) {
      rightSource = this.perfProfilerRightSource.filter((item) => {
        return item.canCharge;
      });
    }
    this.perfProfilerList!.dataSource = rightSource;
  }

  initElements(): void {
    this.perfProfilerTbl = this.shadowRoot?.querySelector<LitTable>('#tb-perf-profile');
    this.perfProfileProgressEL = this.shadowRoot?.querySelector('.perf-profile-progress') as LitProgressBar;
    this.perfProfileFrameChart = this.shadowRoot?.querySelector<FrameChart>('#framechart');
    this.perfProfilerModal = this.shadowRoot?.querySelector<DisassemblingWindow>('tab-native-data-modal');
    this.perfProfileLoadingPage = this.shadowRoot?.querySelector('.perf-profile-loading');
    this.perfProfileFrameChart!.addChartClickListener((needShowMenu: boolean) => {
      this.parentElement!.scrollTo(0, 0);
      showButtonMenu(this.perfProfilerFilter, needShowMenu);
      this.needShowMenu = needShowMenu;
    });
    this.perfProfilerTbl!.rememberScrollTop = true;
    this.perfProfilerFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    this.perfProfilerTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data as PerfCallChainMerageData;
      this.setRightTableData(data);
      data.isSelected = true;
      this.perfSelectedData = data;
      this.perfProfilerList?.clearAllSelection(data);
      this.perfProfilerList?.setCurrentSelection(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    this.perfProfilerList = this.shadowRoot?.querySelector<LitTable>('#tb-perf-list');
    let lastClikTime = 0;
    this.perfProfilerList!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data as PerfCallChainMerageData;
      this.perfProfilerTbl?.clearAllSelection(data);
      (data as any).isSelected = true;
      this.perfProfilerTbl!.scrollToData(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
      let spApplication = <SpApplication>document.getElementsByTagName('sp-application')[0];
      if (Date.now() - lastClikTime < 200 && spApplication.vs) {
        this.perfProfilerTbl!.style.visibility = 'hidden';
        this.perfProfilerFilter.style.display = 'none';
        new ResizeObserver((entries) => {
          this.perfProfilerModal!.style.width = this.perfProfilerTbl!.clientWidth + 'px';
          this.perfProfilerModal!.style.height = this.perfProfilerTbl!.clientHeight + 'px';
        }).observe(this.perfProfilerTbl!);
        this.perfProfilerModal!.showLoading();
        // @ts-ignore
        let data = evt.detail.data as PerfCallChainMerageData;
        let perfProfilerPath = data.path;
        let perfProfilerAddr = data.vaddrInFile;
        let perfProfilerAddrHex = perfProfilerAddr.toString(16);
        if (perfProfilerPath.trim() === '[kernel.kallsyms]') {
          this.perfProfilerModal?.showContent(
            `error : Symbol ${data.symbol} lib is [kernel.kallsyms] ,not support `,
            perfProfilerAddrHex
          );
        } else if (perfProfilerPath.trim() === '') {
          this.perfProfilerModal?.showContent(`error : Symbol ${data.symbol} lib is null `, perfProfilerAddrHex);
        } else if (perfProfilerAddr < 0) {
          this.perfProfilerModal?.showContent(
            `error : Symbol ${data.symbol} current addr is error ` + perfProfilerAddrHex,
            perfProfilerAddrHex
          );
        } else {
          const binDir = 'C:/binary_cache';
          let binPath = binDir + perfProfilerPath;
          let cmd = 'C:/binary_cache/llvm-objdump.exe -S ' + binPath;
          Cmd.execObjDump(cmd, perfProfilerAddrHex, (result: any) => {
            this.perfProfilerModal?.showContent(result, perfProfilerAddrHex);
          });
        }
      }
      lastClikTime = Date.now();
    });
    this.perfProfilerModal!.setCloseListener(() => {
      this.perfProfilerModal!.style.display = 'none';
      this.perfProfilerTbl!.style.visibility = 'visible';
      this.shadowRoot!.querySelector<TabPaneFilter>('#filter')!.style.display = 'flex';
    });
    this.perfProfilerList = this.shadowRoot?.querySelector<LitTable>('#tb-perf-list');
    let filterFunc = (data: any) => {
      let perfProfileFuncArgs: any[] = [];
      if (data.type == 'check') {
        if (data.item.checked) {
          perfProfileFuncArgs.push({
            funcName: 'splitTree',
            funcArgs: [data.item.name, data.item.select == '0', data.item.type == 'symbol'],
          });
        } else {
          perfProfileFuncArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[data.item.name]],
          });
          perfProfileFuncArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          perfProfileFuncArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [data.item.name],
          });
        }
      } else if (data.type == 'select') {
        perfProfileFuncArgs.push({
          funcName: 'resotreAllNode',
          funcArgs: [[data.item.name]],
        });
        perfProfileFuncArgs.push({
          funcName: 'clearSplitMapData',
          funcArgs: [data.item.name],
        });
        perfProfileFuncArgs.push({
          funcName: 'splitTree',
          funcArgs: [data.item.name, data.item.select == '0', data.item.type == 'symbol'],
        });
      } else if (data.type == 'button') {
        if (data.item == 'symbol') {
          if (this.perfSelectedData && !this.perfSelectedData.canCharge) {
            return;
          }
          if (this.perfSelectedData != undefined) {
            this.perfProfilerFilter!.addDataMining({ name: this.perfSelectedData.symbolName }, data.item);
            perfProfileFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.perfSelectedData.symbolName, false, true],
            });
          } else {
            return;
          }
        } else if (data.item == 'library') {
          if (this.perfSelectedData && !this.perfSelectedData.canCharge) {
            return;
          }
          if (this.perfSelectedData != undefined && this.perfSelectedData.libName != '') {
            this.perfProfilerFilter!.addDataMining({ name: this.perfSelectedData.libName }, data.item);
            perfProfileFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.perfSelectedData.libName, false, false],
            });
          } else {
            return;
          }
        } else if (data.item == 'restore') {
          if (data.remove != undefined && data.remove.length > 0) {
            let list = data.remove.map((item: any) => {
              return item.name;
            });
            perfProfileFuncArgs.push({
              funcName: 'resotreAllNode',
              funcArgs: [list],
            });
            perfProfileFuncArgs.push({
              funcName: 'resetAllNode',
              funcArgs: [],
            });
            list.forEach((symbolName: string) => {
              perfProfileFuncArgs.push({
                funcName: 'clearSplitMapData',
                funcArgs: [symbolName],
              });
            });
          }
        }
      }
      this.getDataByWorker(perfProfileFuncArgs, (result: any[]) => {
        this.setPerfProfilerLeftTableData(result);
        this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
        if (this.isChartShow) this.perfProfileFrameChart?.calculateChartData();
        this.perfProfilerTbl!.move1px();
        if (this.perfSelectedData) {
          this.perfSelectedData.isSelected = false;
          this.perfProfilerTbl?.clearAllSelection(this.perfSelectedData);
          this.perfProfilerList!.recycleDataSource = [];
          this.perfSelectedData = undefined;
        }
      });
    };
    this.perfProfilerFilter!.getDataLibrary(filterFunc);
    this.perfProfilerFilter!.getDataMining(filterFunc);
    this.perfProfilerFilter!.getCallTreeData((data: any) => {
      if (data.value == 0) {
        this.refreshAllNode({
          ...this.perfProfilerFilter!.getFilterTreeData(),
          callTree: data.checks,
        });
      } else {
        let perfProfileArgs: any[] = [];
        if (data.checks[1]) {
          perfProfileArgs.push({
            funcName: 'hideSystemLibrary',
            funcArgs: [],
          });
          perfProfileArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
        } else {
          perfProfileArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[this.systemRuleName]],
          });
          perfProfileArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          perfProfileArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [this.systemRuleName],
          });
        }
        this.getDataByWorker(perfProfileArgs, (result: any[]) => {
          this.setPerfProfilerLeftTableData(result);
          this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
          if (this.isChartShow) this.perfProfileFrameChart?.calculateChartData();
        });
      }
    });
    this.perfProfilerFilter!.getCallTreeConstraintsData((data: any) => {
      let perfProfilerConstraintsArgs: any[] = [
        {
          funcName: 'resotreAllNode',
          funcArgs: [[this.perfProfileNumRuleName]],
        },
        {
          funcName: 'clearSplitMapData',
          funcArgs: [this.perfProfileNumRuleName],
        },
      ];
      if (data.checked) {
        perfProfilerConstraintsArgs.push({
          funcName: 'hideNumMaxAndMin',
          funcArgs: [parseInt(data.min), data.max],
        });
      }
      perfProfilerConstraintsArgs.push({
        funcName: 'resetAllNode',
        funcArgs: [],
      });
      this.getDataByWorker(perfProfilerConstraintsArgs, (result: any[]) => {
        this.setPerfProfilerLeftTableData(result);
        this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
        if (this.isChartShow) this.perfProfileFrameChart?.calculateChartData();
      });
    });
    this.perfProfilerFilter!.getFilterData((data: FilterData) => {
      if (this.searchValue != this.perfProfilerFilter!.filterValue) {
        this.searchValue = this.perfProfilerFilter!.filterValue;
        let perfArgs = [
          {
            funcName: 'setSearchValue',
            funcArgs: [this.searchValue],
          },
          {
            funcName: 'resetAllNode',
            funcArgs: [],
          },
        ];
        this.getDataByWorker(perfArgs, (result: any[]) => {
          this.setPerfProfilerLeftTableData(result);
          this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
          this.switchFlameChart(data);
        });
      } else {
        this.switchFlameChart(data);
      }
    });
    this.perfProfilerTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.perfProfileSortKey = evt.detail.key;
      // @ts-ignore
      this.perfProfileSortType = evt.detail.sort;
      // @ts-ignore
      this.setPerfProfilerLeftTableData(this.perfProfilerDataSource);
      this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
    });
  }

  connectedCallback() {
    super.connectedCallback();
    this.parentElement!.onscroll = () => {
      this.perfProfileFrameChart!.tabPaneScrollTop = this.parentElement!.scrollTop;
    };
    let filterHeight = 0;
    new ResizeObserver((entries) => {
      let perfProfileTabFilter = this.shadowRoot!.querySelector('#filter') as HTMLElement;
      if (perfProfileTabFilter.clientHeight > 0) filterHeight = perfProfileTabFilter.clientHeight;
      if (this.parentElement!.clientHeight > filterHeight) {
        perfProfileTabFilter.style.display = 'flex';
      } else {
        perfProfileTabFilter.style.display = 'none';
      }
      this.perfProfilerModal!.style.height = this.perfProfilerTbl!.clientHeight - 2 + 'px'; //2 is borderWidth
      if (this.perfProfilerTbl!.style.visibility == 'hidden') {
        perfProfileTabFilter.style.display = 'none';
      }
      if (this.parentElement?.clientHeight != 0) {
        if (this.isChartShow) {
          this.perfProfileFrameChart?.updateCanvas(false, entries[0].contentRect.width);
          this.perfProfileFrameChart?.calculateChartData();
        }
        // @ts-ignore
        this.perfProfilerTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 35 + 'px';
        this.perfProfilerTbl?.reMeauseHeight();
        // @ts-ignore
        this.perfProfilerList?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 - 21 + 'px';
        this.perfProfilerList?.reMeauseHeight();
        this.perfProfileLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  switchFlameChart(data?: any) {
    let perfProfilerPageTab = this.shadowRoot?.querySelector('#show_table');
    let perfProfilerPageChart = this.shadowRoot?.querySelector('#show_chart');
    if (!data || data.icon == 'block') {
      perfProfilerPageChart?.setAttribute('class', 'show');
      perfProfilerPageTab?.setAttribute('class', '');
      this.isChartShow = true;
      this.perfProfilerFilter!.disabledMining = true;
      showButtonMenu(this.perfProfilerFilter, this.needShowMenu);
      this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
      this.perfProfileFrameChart?.calculateChartData();
    } else if (data.icon == 'tree') {
      perfProfilerPageChart?.setAttribute('class', '');
      perfProfilerPageTab?.setAttribute('class', 'show');
      showButtonMenu(this.perfProfilerFilter, true);
      this.isChartShow = false;
      this.perfProfilerFilter!.disabledMining = false;
      this.perfProfileFrameChart!.clearCanvas();
      this.perfProfilerTbl!.reMeauseHeight();
    }
  }

  refreshAllNode(filterData: any) {
    let perfProfileArgs: any[] = [];
    let isTopDown: boolean = !filterData.callTree[0];
    let isHideSystemLibrary = filterData.callTree[1];
    let list = filterData.dataMining.concat(filterData.dataLibrary);
    perfProfileArgs.push({
      funcName: 'getCallChainsBySampleIds',
      funcArgs: [isTopDown],
    });
    this.perfProfilerList!.recycleDataSource = [];
    if (isHideSystemLibrary) {
      perfProfileArgs.push({
        funcName: 'hideSystemLibrary',
        funcArgs: [],
      });
    }
    if (filterData.callTreeConstraints.checked) {
      perfProfileArgs.push({
        funcName: 'hideNumMaxAndMin',
        funcArgs: [parseInt(filterData.callTreeConstraints.inputs[0]), filterData.callTreeConstraints.inputs[1]],
      });
    }
    perfProfileArgs.push({
      funcName: 'splitAllProcess',
      funcArgs: [list],
    });
    perfProfileArgs.push({
      funcName: 'resetAllNode',
      funcArgs: [],
    });
    this.getDataByWorker(perfProfileArgs, (result: any[]) => {
      this.setPerfProfilerLeftTableData(result);
      this.perfProfileFrameChart!.data = this.perfProfilerDataSource;
      if (this.isChartShow) this.perfProfileFrameChart?.calculateChartData();
    });
  }

  setPerfProfilerLeftTableData(resultData: any[]) {
    this.perfProfilerDataSource = this.sortTree(resultData);
    this.perfProfilerTbl!.recycleDataSource = this.perfProfilerDataSource;
  }

  sortTree(arr: Array<any>): Array<any> {
    let perfProfileSortArr = arr.sort((perfProfileA, perfProfileB) => {
      if (this.perfProfileSortKey == 'self') {
        if (this.perfProfileSortType == 0) {
          return perfProfileB.dur - perfProfileA.dur;
        } else if (this.perfProfileSortType == 1) {
          return perfProfileA.selfDur - perfProfileB.selfDur;
        } else {
          return perfProfileB.selfDur - perfProfileA.selfDur;
        }
      } else {
        if (this.perfProfileSortType == 0) {
          return perfProfileB.dur - perfProfileA.dur;
        } else if (this.perfProfileSortType == 1) {
          return perfProfileA.dur - perfProfileB.dur;
        } else {
          return perfProfileB.dur - perfProfileA.dur;
        }
      }
    });
    perfProfileSortArr.map((call) => {
      call.children = this.sortTree(call.children);
    });
    return perfProfileSortArr;
  }

  getDataByWorker(args: any[], handler: Function) {
    this.perfProfileLoadingList.push(1);
    this.perfProfileProgressEL!.loading = true;
    this.perfProfileLoadingPage.style.visibility = 'visible';
    procedurePool.submitWithName('logic0', 'perf-action', args, undefined, (results: any) => {
      handler(results);
      this.perfProfileLoadingList.splice(0, 1);
      if (this.perfProfileLoadingList.length == 0) {
        this.perfProfileProgressEL!.loading = false;
        this.perfProfileLoadingPage.style.visibility = 'hidden';
      }
    });
  }

  initHtml(): string {
    return `
        <style>
        tab-pane-filter {
            position: fixed;
            bottom: 0;
            width: 100%;
            border: solid rgb(216,216,216) 1px;
            float: left;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .perf-profile-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        selector{
            display: none;
        }
        .perf-profile-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        .show{
            display: flex;
            flex: 1;
        }
    </style>
    <div class="perf-profile-content" style="display: flex;flex-direction: row">
    
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <tab-native-data-modal id="modal"></tab-native-data-modal>
            <lit-table id="tb-perf-profile" style="height: auto" tree>
                <lit-table-column width="70%" title="Call Stack" data-index="symbol" key="symbol"  align="flex-start" ></lit-table-column>
                <lit-table-column width="1fr" title="Local" data-index="self" key="self"  align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="Weight" data-index="weight" key="weight"  align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="%" data-index="weightPercent" key="weightPercent"  align="flex-start"  order></lit-table-column>
            </lit-table>
            
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <lit-table id="tb-perf-list" no-head hideDownload style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)">
            <span slot="head">Heaviest Stack Trace</span>
            <lit-table-column width="60px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column width="1fr" title="" data-index="symbolName" key="symbolName"  align="flex-start"></lit-table-column>
        </lit-table>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="filter" input inputLeftText icon tree></tab-pane-filter>
     <lit-progress-bar class="progress perf-profile-progress"></lit-progress-bar>
    <selector id='show_chart'>
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>  
    <div class="loading perf-profile-loading"></div>
    </div>`;
  }
}
