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
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { FrameChart } from '../../../chart/FrameChart.js';
import '../../../chart/FrameChart.js';
import { DisassemblingWindow } from '../../../DisassemblingWindow.js';
import '../../../DisassemblingWindow.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { ChartMode } from '../../../../bean/FrameChartStruct.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import '../TabPaneFilter.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { FileMerageBean } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { showButtonMenu } from '../SheetUtils.js';

@element('tabpane-filesystem-calltree')
export class TabpaneFilesystemCalltree extends BaseElement {
  private fsCallTreeTbl: LitTable | null | undefined;
  private fsCallTreeTbr: LitTable | null | undefined;
  private fsCallTreeProgressEL: LitProgressBar | null | undefined;
  private fsCallTreeRightSource: Array<FileMerageBean> = [];
  private fsCallTreeFilter: any;
  private fsCallTreeDataSource: any[] = [];
  private fsCallTreeSortKey = 'weight';
  private fsCallTreeSortType = 0;
  private fsCallTreeCurrentSelectedData: any = undefined;
  private frameChart: FrameChart | null | undefined;
  private isChartShow: boolean = false;
  private systmeRuleName = '/system/';
  private fsCallTreeNumRuleName = '/max/min/';
  private modal: DisassemblingWindow | null | undefined;
  private needShowMenu = true;
  private searchValue: string = '';
  private loadingList: number[] = [];
  private loadingPage: any;
  private currentSelection: SelectionParam | undefined;

  set data(fsCallTreeSelection: SelectionParam | any) {
    if (fsCallTreeSelection == this.currentSelection) {
      return;
    }
    this.searchValue = '';
    this.currentSelection = fsCallTreeSelection;
    this.modal!.style.display = 'none';
    this.fsCallTreeTbl!.style.visibility = 'visible';
    if (this.parentElement!.clientHeight > this.fsCallTreeFilter!.clientHeight) {
      this.fsCallTreeFilter!.style.display = 'flex';
    } else {
      this.fsCallTreeFilter!.style.display = 'none';
    }
    this.fsCallTreeFilter!.initializeFilterTree(true, true, true);
    this.fsCallTreeFilter!.filterValue = '';
    this.fsCallTreeProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    this.getDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [{ queryFuncName: 'fileSystem', ...fsCallTreeSelection }],
        },
      ],
      (fsCallTreeResults: any[]) => {
        this.setLTableData(fsCallTreeResults);
        this.fsCallTreeTbr!.recycleDataSource = [];
        this.frameChart!.mode = ChartMode.Duration;
        this.frameChart!.data = this.fsCallTreeDataSource;
        this.frameChart?.updateCanvas(true, this.clientWidth);
        this.frameChart?.calculateChartData();
        this.switchFlameChart();
      }
    );
  }

  getParentTree(
    fsCallTreeSrc: Array<FileMerageBean>,
    fsCallTreeTarget: FileMerageBean,
    parents: Array<FileMerageBean>
  ): boolean {
    for (let fsCallTreeBean of fsCallTreeSrc) {
      if (fsCallTreeBean.id == fsCallTreeTarget.id) {
        parents.push(fsCallTreeBean);
        return true;
      } else {
        if (this.getParentTree(fsCallTreeBean.children as Array<FileMerageBean>, fsCallTreeTarget, parents)) {
          parents.push(fsCallTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  getChildTree(fsCallTreeSrc: Array<FileMerageBean>, id: string, children: Array<FileMerageBean>): boolean {
    for (let fsCallTreeBean of fsCallTreeSrc) {
      if (fsCallTreeBean.id == id && fsCallTreeBean.children.length == 0) {
        children.push(fsCallTreeBean);
        return true;
      } else {
        if (this.getChildTree(fsCallTreeBean.children as Array<FileMerageBean>, id, children)) {
          children.push(fsCallTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  setRightTableData(merageBean: FileMerageBean) {
    let parents: Array<FileMerageBean> = [];
    let children: Array<FileMerageBean> = [];
    this.getParentTree(this.fsCallTreeDataSource, merageBean, parents);
    let maxId = merageBean.id;
    let maxDur = 0;

    function findMaxStack(fsMerageBean: FileMerageBean) {
      if (fsMerageBean.children.length == 0) {
        if (fsMerageBean.dur > maxDur) {
          maxDur = fsMerageBean.dur;
          maxId = fsMerageBean.id;
        }
      } else {
        fsMerageBean.children.map((callChild: any) => {
          findMaxStack(<FileMerageBean>callChild);
        });
      }
    }

    findMaxStack(merageBean);
    this.getChildTree(merageBean.children as Array<FileMerageBean>, maxId, children);
    let fsMerageParentsList = parents.reverse().concat(children.reverse());
    for (let data of fsMerageParentsList) {
      data.type =
        data.libName.endsWith('.so.1') || data.libName.endsWith('.dll') || data.libName.endsWith('.so') ? 0 : 1;
    }
    let len = fsMerageParentsList.length;
    this.fsCallTreeRightSource = fsMerageParentsList;
    this.fsCallTreeTbr!.dataSource = len == 0 ? [] : fsMerageParentsList;
  }

  initElements(): void {
    this.fsCallTreeTbl = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-calltree');
    this.fsCallTreeProgressEL = this.shadowRoot?.querySelector('.fs-call-tree-progress') as LitProgressBar;
    this.frameChart = this.shadowRoot?.querySelector<FrameChart>('#framechart');
    this.modal = this.shadowRoot?.querySelector<DisassemblingWindow>('tab-native-data-modal');
    this.loadingPage = this.shadowRoot?.querySelector('.fs-call-tree-loading');
    this.frameChart!.addChartClickListener((needShowMenu: boolean) => {
      this.parentElement!.scrollTo(0, 0);
      showButtonMenu(this.fsCallTreeFilter, needShowMenu);
      this.needShowMenu = needShowMenu;
    });
    this.fsCallTreeTbl!.rememberScrollTop = true;
    this.fsCallTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    this.fsCallTreeTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data as FileMerageBean;
      this.setRightTableData(data);
      data.isSelected = true;
      this.fsCallTreeCurrentSelectedData = data;
      this.fsCallTreeTbr?.clearAllSelection(data);
      this.fsCallTreeTbr?.setCurrentSelection(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    this.fsCallTreeTbr = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-list');
    this.fsCallTreeTbr!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data as FileMerageBean;
      this.fsCallTreeTbl?.clearAllSelection(data);
      (data as any).isSelected = true;
      this.fsCallTreeTbl!.scrollToData(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    this.modal!.setCloseListener(() => {
      this.modal!.style.display = 'none';
      this.fsCallTreeTbl!.style.visibility = 'visible';
      this.shadowRoot!.querySelector<TabPaneFilter>('#filter')!.style.display = 'flex';
    });
    let filterFunc = (data: any) => {
      let fsCallTreeFuncArgs: any[] = [];
      if (data.type == 'check') {
        if (data.item.checked) {
          fsCallTreeFuncArgs.push({
            funcName: 'splitTree',
            funcArgs: [data.item.name, data.item.select == '0', data.item.type == 'symbol'],
          });
        } else {
          fsCallTreeFuncArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[data.item.name]],
          });
          fsCallTreeFuncArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          fsCallTreeFuncArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [data.item.name],
          });
        }
      } else if (data.type == 'select') {
        fsCallTreeFuncArgs.push({
          funcName: 'resotreAllNode',
          funcArgs: [[data.item.name]],
        });
        fsCallTreeFuncArgs.push({
          funcName: 'clearSplitMapData',
          funcArgs: [data.item.name],
        });
        fsCallTreeFuncArgs.push({
          funcName: 'splitTree',
          funcArgs: [data.item.name, data.item.select == '0', data.item.type == 'symbol'],
        });
      } else if (data.type == 'button') {
        if (data.item == 'symbol') {
          if (this.fsCallTreeCurrentSelectedData && !this.fsCallTreeCurrentSelectedData.canCharge) {
            return;
          }
          if (this.fsCallTreeCurrentSelectedData != undefined) {
            this.fsCallTreeFilter!.addDataMining({ name: this.fsCallTreeCurrentSelectedData.symbolName }, data.item);
            fsCallTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.fsCallTreeCurrentSelectedData.symbolName, false, true],
            });
          } else {
            return;
          }
        } else if (data.item == 'library') {
          if (this.fsCallTreeCurrentSelectedData && !this.fsCallTreeCurrentSelectedData.canCharge) {
            return;
          }
          if (this.fsCallTreeCurrentSelectedData != undefined && this.fsCallTreeCurrentSelectedData.libName != '') {
            this.fsCallTreeFilter!.addDataMining({ name: this.fsCallTreeCurrentSelectedData.libName }, data.item);
            fsCallTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.fsCallTreeCurrentSelectedData.libName, false, false],
            });
          } else {
            return;
          }
        } else if (data.item == 'restore') {
          if (data.remove != undefined && data.remove.length > 0) {
            let list = data.remove.map((item: any) => {
              return item.name;
            });
            fsCallTreeFuncArgs.push({
              funcName: 'resotreAllNode',
              funcArgs: [list],
            });
            fsCallTreeFuncArgs.push({
              funcName: 'resetAllNode',
              funcArgs: [],
            });
            list.forEach((symbolName: string) => {
              fsCallTreeFuncArgs.push({
                funcName: 'clearSplitMapData',
                funcArgs: [symbolName],
              });
            });
          }
        }
      }
      this.getDataByWorker(fsCallTreeFuncArgs, (result: any[]) => {
        this.setLTableData(result);
        this.frameChart!.data = this.fsCallTreeDataSource;
        if (this.isChartShow) this.frameChart?.calculateChartData();
        this.fsCallTreeTbl!.move1px();
        if (this.fsCallTreeCurrentSelectedData) {
          this.fsCallTreeCurrentSelectedData.isSelected = false;
          this.fsCallTreeTbl?.clearAllSelection(this.fsCallTreeCurrentSelectedData);
          this.fsCallTreeTbr!.recycleDataSource = [];
          this.fsCallTreeCurrentSelectedData = undefined;
        }
      });
    };
    this.fsCallTreeFilter!.getDataLibrary(filterFunc);
    this.fsCallTreeFilter!.getDataMining(filterFunc);
    this.fsCallTreeFilter!.getCallTreeData((data: any) => {
      if (data.value == 0) {
        this.refreshAllNode({
          ...this.fsCallTreeFilter!.getFilterTreeData(),
          callTree: data.checks,
        });
      } else {
        let fileSysCallTreeArgs: any[] = [];
        if (data.checks[1]) {
          fileSysCallTreeArgs.push({
            funcName: 'hideSystemLibrary',
            funcArgs: [],
          });
          fileSysCallTreeArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
        } else {
          fileSysCallTreeArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[this.systmeRuleName]],
          });
          fileSysCallTreeArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          fileSysCallTreeArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [this.systmeRuleName],
          });
        }
        this.getDataByWorker(fileSysCallTreeArgs, (result: any[]) => {
          this.setLTableData(result);
          this.frameChart!.data = this.fsCallTreeDataSource;
          if (this.isChartShow) this.frameChart?.calculateChartData();
        });
      }
    });
    this.fsCallTreeFilter!.getCallTreeConstraintsData((data: any) => {
      let fsCallTreeConstraintsArgs: any[] = [
        {
          funcName: 'resotreAllNode',
          funcArgs: [[this.fsCallTreeNumRuleName]],
        },
        {
          funcName: 'clearSplitMapData',
          funcArgs: [this.fsCallTreeNumRuleName],
        },
      ];
      if (data.checked) {
        fsCallTreeConstraintsArgs.push({
          funcName: 'hideNumMaxAndMin',
          funcArgs: [parseInt(data.min), data.max],
        });
      }
      fsCallTreeConstraintsArgs.push({
        funcName: 'resetAllNode',
        funcArgs: [],
      });
      this.getDataByWorker(fsCallTreeConstraintsArgs, (result: any[]) => {
        this.setLTableData(result);
        this.frameChart!.data = this.fsCallTreeDataSource;
        if (this.isChartShow) this.frameChart?.calculateChartData();
      });
    });
    this.fsCallTreeFilter!.getFilterData((data: FilterData) => {
      if (this.searchValue != this.fsCallTreeFilter!.filterValue) {
        this.searchValue = this.fsCallTreeFilter!.filterValue;
        let fileArgs = [
          {
            funcName: 'setSearchValue',
            funcArgs: [this.searchValue],
          },
          {
            funcName: 'resetAllNode',
            funcArgs: [],
          },
        ];
        this.getDataByWorker(fileArgs, (result: any[]) => {
          this.setLTableData(result);
          this.frameChart!.data = this.fsCallTreeDataSource;
          this.switchFlameChart(data);
        });
      } else {
        this.switchFlameChart(data);
      }
    });
    this.fsCallTreeTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.fsCallTreeSortKey = evt.detail.key;
      // @ts-ignore
      this.fsCallTreeSortType = evt.detail.sort;
      // @ts-ignore
      this.setLTableData(this.fsCallTreeDataSource);
      this.frameChart!.data = this.fsCallTreeDataSource;
    });
  }

  connectedCallback() {
    super.connectedCallback();
    let filterHeight = 0;
    new ResizeObserver((entries) => {
      let fsCallTreeTabFilter = this.shadowRoot!.querySelector('#filter') as HTMLElement;
      if (fsCallTreeTabFilter.clientHeight > 0) filterHeight = fsCallTreeTabFilter.clientHeight;
      if (this.parentElement!.clientHeight > filterHeight) {
        fsCallTreeTabFilter.style.display = 'flex';
      } else {
        fsCallTreeTabFilter.style.display = 'none';
      }
      this.modal!.style.height = this.fsCallTreeTbl!.clientHeight - 2 + 'px'; //2 is borderWidth
      if (this.fsCallTreeTbl!.style.visibility == 'hidden') {
        fsCallTreeTabFilter.style.display = 'none';
      }
      if (this.parentElement?.clientHeight != 0) {
        if (this.isChartShow) {
          this.frameChart?.updateCanvas(false, entries[0].contentRect.width);
          this.frameChart?.calculateChartData();
        }
        // @ts-ignore
        this.fsCallTreeTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 10 - 35 + 'px';
        this.fsCallTreeTbl?.reMeauseHeight();
        // @ts-ignore
        this.fsCallTreeTbr?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 - 21 + 'px';
        this.fsCallTreeTbr?.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
    this.parentElement!.onscroll = () => {
      this.frameChart!.tabPaneScrollTop = this.parentElement!.scrollTop;
    };
  }

  switchFlameChart(data?: any) {
    let fsCallTreePageTab = this.shadowRoot?.querySelector('#show_table');
    let fsCallTreePageChart = this.shadowRoot?.querySelector('#show_chart');
    if (!data || data.icon == 'block') {
      fsCallTreePageChart?.setAttribute('class', 'show');
      fsCallTreePageTab?.setAttribute('class', '');
      this.isChartShow = true;
      this.fsCallTreeFilter!.disabledMining = true;
      showButtonMenu(this.fsCallTreeFilter, this.needShowMenu);
      this.frameChart!.data = this.fsCallTreeDataSource;
      this.frameChart?.calculateChartData();
    } else if (data.icon == 'tree') {
      fsCallTreePageChart?.setAttribute('class', '');
      fsCallTreePageTab?.setAttribute('class', 'show');
      showButtonMenu(this.fsCallTreeFilter, true);
      this.isChartShow = false;
      this.fsCallTreeFilter!.disabledMining = false;
      this.frameChart!.clearCanvas();
      this.fsCallTreeTbl!.reMeauseHeight();
    }
  }

  refreshAllNode(filterData: any) {
    let fileSysCallTreeArgs: any[] = [];
    let isTopDown: boolean = !filterData.callTree[0];
    let isHideSystemLibrary = filterData.callTree[1];
    let list = filterData.dataMining.concat(filterData.dataLibrary);
    fileSysCallTreeArgs.push({
      funcName: 'getCallChainsBySampleIds',
      funcArgs: [isTopDown, 'fileSystem'],
    });
    this.fsCallTreeTbr!.recycleDataSource = [];
    if (isHideSystemLibrary) {
      fileSysCallTreeArgs.push({
        funcName: 'hideSystemLibrary',
        funcArgs: [],
      });
    }
    if (filterData.callTreeConstraints.checked) {
      fileSysCallTreeArgs.push({
        funcName: 'hideNumMaxAndMin',
        funcArgs: [parseInt(filterData.callTreeConstraints.inputs[0]), filterData.callTreeConstraints.inputs[1]],
      });
    }
    fileSysCallTreeArgs.push({
      funcName: 'splitAllProcess',
      funcArgs: [list],
    });
    fileSysCallTreeArgs.push({
      funcName: 'resetAllNode',
      funcArgs: [],
    });
    this.getDataByWorker(fileSysCallTreeArgs, (result: any[]) => {
      this.setLTableData(result);
      this.frameChart!.data = this.fsCallTreeDataSource;
      if (this.isChartShow) this.frameChart?.calculateChartData();
    });
  }

  setLTableData(resultData: any[]) {
    this.fsCallTreeDataSource = this.sortTree(resultData);
    this.fsCallTreeTbl!.recycleDataSource = this.fsCallTreeDataSource;
  }

  sortTree(arr: Array<any>): Array<any> {
    let fsCallTreeSortArr = arr.sort((fsCallTreeA, fsCallTreeB) => {
      if (this.fsCallTreeSortKey == 'self') {
        if (this.fsCallTreeSortType == 0) {
          return fsCallTreeB.dur - fsCallTreeA.dur;
        } else if (this.fsCallTreeSortType == 1) {
          return fsCallTreeA.selfDur - fsCallTreeB.selfDur;
        } else {
          return fsCallTreeB.selfDur - fsCallTreeA.selfDur;
        }
      } else {
        if (this.fsCallTreeSortType == 0) {
          return fsCallTreeB.dur - fsCallTreeA.dur;
        } else if (this.fsCallTreeSortType == 1) {
          return fsCallTreeA.dur - fsCallTreeB.dur;
        } else {
          return fsCallTreeB.dur - fsCallTreeA.dur;
        }
      }
    });
    fsCallTreeSortArr.map((call) => {
      call.children = this.sortTree(call.children);
    });
    return fsCallTreeSortArr;
  }

  getDataByWorker(args: any[], handler: Function) {
    this.loadingList.push(1);
    this.fsCallTreeProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    procedurePool.submitWithName(
      'logic0',
      'fileSystem-action',
      { args, callType: 'fileSystem' },
      undefined,
      (fsCallTreeResults: any) => {
        handler(fsCallTreeResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length == 0) {
          this.fsCallTreeProgressEL!.loading = false;
          this.loadingPage.style.visibility = 'hidden';
        }
      }
    );
  }

  initHtml(): string {
    return `
        <style>
        .fs-call-tree-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .fs-call-tree-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        selector{
            display: none;
        }
        .fs-call-tree-loading{
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
    <div class="fs-call-tree-content" style="display: flex;flex-direction: row">
    
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <tab-native-data-modal id="modal"></tab-native-data-modal>
            <lit-table id="tb-filesystem-calltree" style="height: auto" tree>
                <lit-table-column class="fs-call-tree-column" width="70%" title="Call Stack" data-index="symbolName" key="symbolName"  align="flex-start" ></lit-table-column>
                <lit-table-column class="fs-call-tree-column" width="1fr" title="Local" data-index="self" key="self"  align="flex-start"  order></lit-table-column>
                <lit-table-column class="fs-call-tree-column" width="1fr" title="Weight" data-index="weight" key="weight"  align="flex-start"  order></lit-table-column>
                <lit-table-column class="fs-call-tree-column" width="1fr" title="%" data-index="weightPercent" key="weightPercent"  align="flex-start"  order></lit-table-column>
            </lit-table>
            
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <lit-table id="tb-filesystem-list" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
            <span slot="head">Heaviest Stack Trace</span>
            <lit-table-column class="fs-call-tree-column" width="30px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column class="fs-call-tree-column" width="60px" title="" data-index="count" key="count"  align="flex-start"></lit-table-column>
            <lit-table-column class="fs-call-tree-column" width="1fr" title="" data-index="symbolName" key="symbolName"  align="flex-start"></lit-table-column>
        </lit-table>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="filter" class="fs-call-tree-filter" input inputLeftText icon tree></tab-pane-filter>
     <lit-progress-bar class="progress fs-call-tree-progress"></lit-progress-bar>
    <selector id='show_chart' class="fs-call-tree-selector" >
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>  
    <div class="loading fs-call-tree-loading"></div>
    </div>`;
  }
}
