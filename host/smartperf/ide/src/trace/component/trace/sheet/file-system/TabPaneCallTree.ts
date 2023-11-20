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
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { ChartMode } from '../../../../bean/FrameChartStruct.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { MerageBean } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { showButtonMenu } from '../SheetUtils.js';
import { CallTreeLevelStruct } from '../../../../bean/EbpfStruct.js';
import '../../../../../base-ui/headline/lit-headline.js';
import { LitHeadLine } from '../../../../../base-ui/headline/lit-headline.js';

const InvertOptionIndex: number = 0;
const hideEventOptionIndex: number = 2;
const hideThreadOptionIndex: number = 3;

@element('tabpane-calltree')
export class TabPaneCallTree extends BaseElement {
  public queryFuncName: string = '';
  public procedureAction: string = '';
  private callTreeTbl: LitTable | null | undefined;
  private callTreeTbr: LitTable | null | undefined;
  private callTreeProgressEL: LitProgressBar | null | undefined;
  private callTreeRightSource: Array<MerageBean> = [];
  private callTreeFilter: any;
  private callTreeDataSource: any[] = [];
  private callTreeSortKey: string = 'weight';
  private callTreeSortType: number = 0;
  private callTreeSelectedData: any = undefined;
  private frameChart: FrameChart | null | undefined;
  private isChartShow: boolean = false;
  private systmeRuleName: string = '/system/';
  private callTreeNumRuleName: string = '/max/min/';
  private needShowMenu: boolean = true;
  private searchValue: string = '';
  private loadingList: number[] = [];
  private loadingPage: any;
  private currentSelection: SelectionParam | undefined;
  private flameChartMode: ChartMode = ChartMode.Duration;
  private currentCallTreeDataSource: Array<MerageBean> = [];
  private currentRowClickData: any;
  private initWidth: number = 0;
  private _pieTitle: string = '';
  private _cWidth: number = 0;
  private _currentCallTreeLevel: number = 0;
  private _rowClickData: any = undefined;
  private callTreeLevel: CallTreeLevelStruct | undefined | null;
  private headLine: LitHeadLine | null | undefined;

  set pieTitle(value: string) {
    this._pieTitle = value;
    if (this._pieTitle.length > 0) {
      this.headLine!.isShow = true;
      this.headLine!.titleTxt = this._pieTitle;
      this.headLine!.closeCallback = () => {
        this.restore();
      };
    }
  }

  set cWidth(value: number) {
    this._cWidth = value;
  }

  set currentCallTreeLevel(value: number) {
    this._currentCallTreeLevel = value;
  }

  set rowClickData(value: any) {
    this._rowClickData = value;
  }

  set data(callTreeSelection: SelectionParam | any) {
    if (callTreeSelection !== this.currentSelection && this._rowClickData === this.currentRowClickData) {
      this._rowClickData = undefined;
    }
    if (callTreeSelection === this.currentSelection && !this.currentSelection?.isRowClick) {
      return;
    }
    this.searchValue = '';
    this.initModeAndAction();
    this.currentSelection = callTreeSelection;
    this.currentRowClickData = this._rowClickData;
    this.callTreeTbl!.style.visibility = 'visible';
    if (this.parentElement!.clientHeight > this.callTreeFilter!.clientHeight) {
      this.callTreeFilter!.style.display = 'flex';
    } else {
      this.callTreeFilter!.style.display = 'none';
    }
    procedurePool.submitWithName('logic0', 'fileSystem-reset', [], undefined, () => {});
    this.callTreeFilter!.initializeFilterTree(true, true, true);
    this.callTreeFilter!.filterValue = '';
    this.callTreeProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    this.getDataByWorkAndUpDateCanvas(callTreeSelection);
  }

  getDataByWorkAndUpDateCanvas(callTreeSelection: SelectionParam): void {
    if (this.clientWidth === 0) {
      this.initWidth = this._cWidth;
    } else {
      this.initWidth = this.clientWidth;
    }
    if (this._rowClickData && this.currentRowClickData !== undefined && this.currentSelection?.isRowClick) {
      this.getCallTreeDataByPieLevel();
    } else {
      this.headLine!.isShow = false;
      this.getCallTreeData(callTreeSelection, this.initWidth);
    }
  }

  private getCallTreeData(callTreeSelection: SelectionParam | any, initWidth: number): void {
    this.getDataByWorker(
      [
        {
          funcName: 'setSearchValue',
          funcArgs: [''],
        },
        {
          funcName: 'getCurrentDataFromDb',
          funcArgs: [{ queryFuncName: this.queryFuncName, ...callTreeSelection }],
        },
      ],
      (results: any[]) => {
        this.setLTableData(results);
        this.callTreeTbr!.recycleDataSource = [];
        this.frameChart!.mode = this.flameChartMode;
        this.frameChart?.updateCanvas(true, initWidth);
        this.frameChart!.data = this.callTreeDataSource;
        this.currentCallTreeDataSource = this.callTreeDataSource;
        this.switchFlameChart();
        this.callTreeFilter.icon = 'block';
      }
    );
  }

  /**
   * 根据Analysis Tab饼图跳转过来的层级绘制对应的CallTree Tab火焰图和表格
   */
  private getCallTreeDataByPieLevel(): void {
    this.callTreeLevel = new CallTreeLevelStruct();
    this.callTreeLevel = {
      processId: this._rowClickData.pid,
      threadId: this._rowClickData.tid,
      typeId: this._rowClickData.type,
      libId: this._rowClickData.libId,
      symbolId: this._rowClickData.symbolId,
    };
    let args = [];
    args.push({
      funcName: 'getCurrentDataFromDb',
      funcArgs: [this.currentSelection, this.callTreeLevel],
    });

    if (this._rowClickData && this._rowClickData.libId !== undefined && this._currentCallTreeLevel === 3) {
      this.callTreeLevel.libName = this._rowClickData.tableName;
      args.push({
        funcName: 'showLibLevelData',
        funcArgs: [this.callTreeLevel.libId, this.callTreeLevel.libName],
      });
    } else if (this._rowClickData && this._rowClickData.symbolId !== undefined && this._currentCallTreeLevel === 4) {
      this.callTreeLevel.symbolName = this._rowClickData.tableName;
      args.push({
        funcName: 'showFunLevelData',
        funcArgs: [this.callTreeLevel.symbolId, this.callTreeLevel.symbolName],
      });
    }

    this.getDataByWorker(args, (results: any[]) => {
      this.callTreeProgressEL!.loading = false;
      this.loadingPage.style.visibility = 'hidden';
      this.setLTableData(results);
      this.callTreeTbr!.recycleDataSource = [];
      this.frameChart!.mode = this.flameChartMode;
      this.frameChart?.updateCanvas(true, this.initWidth);
      this.frameChart!.data = this.callTreeDataSource;
      this.currentCallTreeDataSource = this.callTreeDataSource;
      this.switchFlameChart();
      this.callTreeFilter.icon = 'block';
    });
  }

  private restore(): void {
    this.searchValue = '';
    this.callTreeFilter!.filterValue = '';
    this.headLine!.isShow = false;
    this._rowClickData = undefined;
    this.getCallTreeData(this.currentSelection, this.initWidth);
  }

  initModeAndAction(): void {
    if (this.procedureAction === '' && this.hasAttribute('action')) {
      this.procedureAction = this.getAttribute('action') || '';
    }
    if (this.hasAttribute('flame-mode')) {
      let callTreeFlameMode = this.getAttribute('flame-mode');
      switch (callTreeFlameMode) {
        case 'Byte':
          this.flameChartMode = ChartMode.Byte;
          break;
        case 'Count':
          this.flameChartMode = ChartMode.Count;
          break;
        case 'Duration':
          this.flameChartMode = ChartMode.Duration;
          break;
      }
    }
    if (this.hasAttribute('query')) {
      this.queryFuncName = this.getAttribute('query') || '';
    }
  }

  getParentTree(callTreeSrc: Array<MerageBean>, target: MerageBean, parents: Array<MerageBean>): boolean {
    for (let callTreeBean of callTreeSrc) {
      if (callTreeBean.id === target.id) {
        parents.push(callTreeBean);
        return true;
      } else {
        if (this.getParentTree(callTreeBean.children as Array<MerageBean>, target, parents)) {
          parents.push(callTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  getChildTree(callTreeSrc: Array<MerageBean>, id: string, children: Array<MerageBean>): boolean {
    for (let callTreeBean of callTreeSrc) {
      if (callTreeBean.id === id && callTreeBean.children.length === 0) {
        children.push(callTreeBean);
        return true;
      } else {
        if (this.getChildTree(callTreeBean.children as Array<MerageBean>, id, children)) {
          children.push(callTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  setRightTableData(bean: MerageBean): void {
    let parents: Array<MerageBean> = [];
    let children: Array<MerageBean> = [];
    this.getParentTree(this.callTreeDataSource, bean, parents);
    let maxId: string = bean.id;
    let maxDur: number = 0;

    function findMaxStack(bean: MerageBean): void {
      if (bean.children.length === 0) {
        if (bean.dur > maxDur) {
          maxDur = bean.dur;
          maxId = bean.id;
        }
      } else {
        bean.children.map((callChild: any) => {
          findMaxStack(<MerageBean>callChild);
        });
      }
    }

    findMaxStack(bean);
    this.getChildTree(bean.children as Array<MerageBean>, maxId, children);
    let callTreeArr = parents.reverse().concat(children.reverse());
    for (let data of callTreeArr) {
      data.type =
        data.libName.endsWith('.so.1') || data.libName.endsWith('.dll') || data.libName.endsWith('.so') ? 0 : 1;
    }
    let len = callTreeArr.length;
    this.callTreeRightSource = callTreeArr;
    this.callTreeTbr!.dataSource = len === 0 ? [] : callTreeArr;
  }

  connectedCallback(): void {
    this.parentElement!.onscroll = (): void => {
      this.frameChart!.tabPaneScrollTop = this.parentElement!.scrollTop;
    };
    this.frameChart!.addChartClickListener((needShowMenu: boolean) => {
      this.parentElement!.scrollTo(0, 0);
      showButtonMenu(this.callTreeFilter, needShowMenu);
      this.needShowMenu = needShowMenu;
    });
    let filterHeight = 0;
    new ResizeObserver((entries: ResizeObserverEntry[]): void => {
      let callTreeTabFilter = this.shadowRoot!.querySelector('#filter') as HTMLElement;
      if (callTreeTabFilter.clientHeight > 0) filterHeight = callTreeTabFilter.clientHeight;
      if (this.parentElement!.clientHeight > filterHeight) {
        callTreeTabFilter.style.display = 'flex';
      } else {
        callTreeTabFilter.style.display = 'none';
      }
      if (this.callTreeTbl!.style.visibility === 'hidden') {
        callTreeTabFilter.style.display = 'none';
      }
      if (this.parentElement?.clientHeight !== 0) {
        if (this.isChartShow) {
          this.frameChart?.updateCanvas(false, entries[0].contentRect.width);
          this.frameChart?.calculateChartData();
        }
        // @ts-ignore
        this.callTreeTbl?.shadowRoot.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 10 - 35 + 'px';
        this.callTreeTbl?.reMeauseHeight();
        // @ts-ignore
        this.callTreeTbr?.shadowRoot.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 45 - 21 + 'px';
        this.callTreeTbr?.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  initElements(): void {
    this.headLine = this.shadowRoot?.querySelector<LitHeadLine>('.titleBox');
    this.callTreeTbl = this.shadowRoot?.querySelector<LitTable>('#tb-calltree');
    this.callTreeProgressEL = this.shadowRoot?.querySelector('.call-tree-progress') as LitProgressBar;
    this.frameChart = this.shadowRoot?.querySelector<FrameChart>('#framechart');
    this.loadingPage = this.shadowRoot?.querySelector('.call-tree-loading');
    this.callTreeTbl!.rememberScrollTop = true;
    this.callTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#filter');
    this.callTreeFilter!.disabledTransfer(true);
    this.addEventListener('contextmenu', (event) => {
      event.preventDefault(); // 阻止默认的上下文菜单弹框
    });
    this.callTreeTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data as MerageBean;
      document.dispatchEvent(
        new CustomEvent('number_calibration', {
          detail: { time: data.tsArray, durations: data.durArray },
        })
      );
      this.setRightTableData(data);
      data.isSelected = true;
      this.callTreeSelectedData = data;
      this.callTreeTbr?.clearAllSelection(data);
      this.callTreeTbr?.setCurrentSelection(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    this.callTreeTbr = this.shadowRoot?.querySelector<LitTable>('#tb-list');
    this.callTreeTbr!.addEventListener('row-click', (evt: any): void => {
      // @ts-ignore
      let data = evt.detail.data as MerageBean;
      this.callTreeTbl?.clearAllSelection(data);
      (data as any).isSelected = true;
      this.callTreeTbl!.scrollToData(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    let filterFunc = (data: any): void => {
      let callTreeFuncArgs: any[] = [];
      if (data.type === 'check') {
        if (data.item.checked) {
          callTreeFuncArgs.push({
            funcName: 'splitTree',
            funcArgs: [data.item.name, data.item.select === '0', data.item.type === 'symbol'],
          });
        } else {
          callTreeFuncArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[data.item.name]],
          });
          callTreeFuncArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          callTreeFuncArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [data.item.name],
          });
        }
      } else if (data.type === 'select') {
        callTreeFuncArgs.push({
          funcName: 'resotreAllNode',
          funcArgs: [[data.item.name]],
        });
        callTreeFuncArgs.push({
          funcName: 'clearSplitMapData',
          funcArgs: [data.item.name],
        });
        callTreeFuncArgs.push({
          funcName: 'splitTree',
          funcArgs: [data.item.name, data.item.select === '0', data.item.type === 'symbol'],
        });
      } else if (data.type === 'button') {
        if (data.item === 'symbol') {
          if (this.callTreeSelectedData && !this.callTreeSelectedData.canCharge) {
            return;
          }
          if (this.callTreeSelectedData !== undefined) {
            this.callTreeFilter!.addDataMining({ name: this.callTreeSelectedData.symbolName }, data.item);
            callTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.callTreeSelectedData.symbolName, false, true],
            });
          } else {
            return;
          }
        } else if (data.item === 'library') {
          if (this.callTreeSelectedData && !this.callTreeSelectedData.canCharge) {
            return;
          }
          if (this.callTreeSelectedData !== undefined && this.callTreeSelectedData.libName !== '') {
            this.callTreeFilter!.addDataMining({ name: this.callTreeSelectedData.libName }, data.item);
            callTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.callTreeSelectedData.libName, false, false],
            });
          } else {
            return;
          }
        } else if (data.item === 'restore') {
          if (data.remove !== undefined && data.remove.length > 0) {
            let list = data.remove.map((item: any) => {
              return item.name;
            });
            callTreeFuncArgs.push({
              funcName: 'resotreAllNode',
              funcArgs: [list],
            });
            callTreeFuncArgs.push({
              funcName: 'resetAllNode',
              funcArgs: [],
            });
            list.forEach((symbolName: string) => {
              callTreeFuncArgs.push({
                funcName: 'clearSplitMapData',
                funcArgs: [symbolName],
              });
            });
          }
        }
      }
      this.getDataByWorker(callTreeFuncArgs, (result: any[]) => {
        this.setLTableData(result);
        this.frameChart!.data = this.callTreeDataSource;
        if (this.isChartShow) this.frameChart?.calculateChartData();
        this.callTreeTbl!.move1px();
        if (this.callTreeSelectedData) {
          this.callTreeSelectedData.isSelected = false;
          this.callTreeTbl?.clearAllSelection(this.callTreeSelectedData);
          this.callTreeTbr!.recycleDataSource = [];
          this.callTreeSelectedData = undefined;
        }
      });
    };
    this.callTreeFilter!.getDataLibrary(filterFunc);
    this.callTreeFilter!.getDataMining(filterFunc);
    this.callTreeFilter!.getCallTreeData((data: any) => {
      if ([InvertOptionIndex, hideThreadOptionIndex, hideEventOptionIndex].includes(data.value)) {
        this.refreshAllNode({
          ...this.callTreeFilter!.getFilterTreeData(),
          callTree: data.checks,
        });
      } else {
        let callTreeArgs: any[] = [];
        if (data.checks[1]) {
          callTreeArgs.push({
            funcName: 'hideSystemLibrary',
            funcArgs: [true],
          });
          callTreeArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
        } else {
          callTreeArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[this.systmeRuleName]],
          });
          callTreeArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          callTreeArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [this.systmeRuleName],
          });
        }
        this.getDataByWorker(callTreeArgs, (result: any[]) => {
          this.setLTableData(result);
          this.frameChart!.data = this.callTreeDataSource;
          if (this.isChartShow) this.frameChart?.calculateChartData();
        });
      }
    });
    this.callTreeFilter!.getCallTreeConstraintsData((data: any) => {
      let callTreeConstraintsArgs: any[] = [
        {
          funcName: 'resotreAllNode',
          funcArgs: [[this.callTreeNumRuleName]],
        },
        {
          funcName: 'clearSplitMapData',
          funcArgs: [this.callTreeNumRuleName],
        },
      ];
      if (data.checked) {
        callTreeConstraintsArgs.push({
          funcName: 'hideNumMaxAndMin',
          funcArgs: [parseInt(data.min), data.max],
        });
      }
      callTreeConstraintsArgs.push({
        funcName: 'resetAllNode',
        funcArgs: [],
      });
      this.getDataByWorker(callTreeConstraintsArgs, (result: any[]) => {
        this.setLTableData(result);
        this.frameChart!.data = this.callTreeDataSource;
        if (this.isChartShow) this.frameChart?.calculateChartData();
      });
    });
    this.callTreeFilter!.getFilterData((callTreeFilterData: FilterData) => {
      if (this.searchValue !== this.callTreeFilter!.filterValue) {
        this.searchValue = this.callTreeFilter!.filterValue;
        let callTreeArgs = [
          {
            funcName: 'setSearchValue',
            funcArgs: [this.searchValue],
          },
          {
            funcName: 'resetAllNode',
            funcArgs: [],
          },
        ];
        this.getDataByWorker(callTreeArgs, (result: any[]): void => {
          this.callTreeTbl!.isSearch = true;
          this.callTreeTbl!.setStatus(result, true);
          this.setLTableData(result);
          this.frameChart!.data = this.callTreeDataSource;
          this.switchFlameChart(callTreeFilterData);
        });
      } else {
        this.switchFlameChart(callTreeFilterData);
      }
    });
    this.callTreeTbl!.addEventListener('column-click', (evt: Event): void => {
      // @ts-ignore
      this.callTreeSortKey = evt.detail.key;
      // @ts-ignore
      this.callTreeSortType = evt.detail.sort;
      // @ts-ignore
      this.setLTableData(this.callTreeDataSource);
      this.frameChart!.data = this.callTreeDataSource;
    });
  }

  switchFlameChart(data?: any): void {
    let callTreePageTab = this.shadowRoot?.querySelector('#show_table');
    let callTreePageChart = this.shadowRoot?.querySelector('#show_chart');
    if (!data || data.icon === 'block') {
      callTreePageChart?.setAttribute('class', 'show');
      callTreePageTab?.setAttribute('class', '');
      this.isChartShow = true;
      this.callTreeFilter!.disabledMining = true;
      showButtonMenu(this.callTreeFilter, this.needShowMenu);
      this.frameChart?.calculateChartData();
    } else if (data.icon === 'tree') {
      callTreePageChart?.setAttribute('class', '');
      callTreePageTab?.setAttribute('class', 'show');
      showButtonMenu(this.callTreeFilter, true);
      this.isChartShow = false;
      this.callTreeFilter!.disabledMining = false;
      this.frameChart!.clearCanvas();
      this.callTreeTbl!.reMeauseHeight();
    }
  }

  refreshAllNode(filterData: any): void {
    let callTreeArgs: any[] = [];
    let isTopDown: boolean = !filterData.callTree[0];
    let isHideSystemLibrary = filterData.callTree[1];
    let isHideEvent: boolean = filterData.callTree[2];
    let isHideThread: boolean = filterData.callTree[3];
    let list = filterData.dataMining.concat(filterData.dataLibrary);
    callTreeArgs.push({
      funcName: 'hideThread',
      funcArgs: [isHideThread],
    });
    callTreeArgs.push({
      funcName: 'hideEvent',
      funcArgs: [isHideEvent],
    });
    callTreeArgs.push({
      funcName: 'getCallChainsBySampleIds',
      funcArgs: [isTopDown, this.queryFuncName],
    });
    this.callTreeTbr!.recycleDataSource = [];
    if (isHideSystemLibrary) {
      callTreeArgs.push({
        funcName: 'hideSystemLibrary',
        funcArgs: [true],
      });
    }
    if (filterData.callTreeConstraints.checked) {
      callTreeArgs.push({
        funcName: 'hideNumMaxAndMin',
        funcArgs: [parseInt(filterData.callTreeConstraints.inputs[0]), filterData.callTreeConstraints.inputs[1]],
      });
    }
    callTreeArgs.push({
      funcName: 'splitAllProcess',
      funcArgs: [list],
    });
    callTreeArgs.push({
      funcName: 'resetAllNode',
      funcArgs: [],
    });
    if (this._rowClickData && this._rowClickData.libId !== undefined && this._currentCallTreeLevel === 3) {
      callTreeArgs.push({
        funcName: 'showLibLevelData',
        funcArgs: [this.callTreeLevel!.libId, this.callTreeLevel!.libName],
      });
    } else if (this._rowClickData && this._rowClickData.symbolId !== undefined && this._currentCallTreeLevel === 4) {
      callTreeArgs.push({
        funcName: 'showFunLevelData',
        funcArgs: [this.callTreeLevel!.symbolId, this.callTreeLevel!.symbolName],
      });
    }
    this.getDataByWorker(callTreeArgs, (result: any[]): void => {
      this.setLTableData(result);
      this.frameChart!.data = this.callTreeDataSource;
      if (this.isChartShow) this.frameChart?.calculateChartData();
    });
  }

  setLTableData(resultData: any[]): void {
    this.callTreeDataSource = this.sortCallFnTree(resultData);
    this.callTreeTbl!.recycleDataSource = this.callTreeDataSource;
  }

  sortCallFnTree(arr: Array<any>): Array<any> {
    let sortArr = arr.sort((compareFnA: any, compareFnB: any): number => {
      if (this.callTreeSortKey === 'self') {
        if (this.callTreeSortType === 0) {
          return compareFnB.dur - compareFnA.dur;
        } else if (this.callTreeSortType === 1) {
          return compareFnA.selfDur - compareFnB.selfDur;
        } else {
          return compareFnB.selfDur - compareFnA.selfDur;
        }
      } else {
        if (this.callTreeSortType === 0) {
          return compareFnB.dur - compareFnA.dur;
        } else if (this.callTreeSortType === 1) {
          return compareFnA.dur - compareFnB.dur;
        } else {
          return compareFnB.dur - compareFnA.dur;
        }
      }
    });
    sortArr.map((call: any): void => {
      call.children = this.sortCallFnTree(call.children);
    });
    return sortArr;
  }

  getDataByWorker(args: any[], handler: Function): void {
    this.loadingList.push(1);
    this.loadingPage.style.visibility = 'visible';
    this.callTreeProgressEL!.loading = true;
    procedurePool.submitWithName(
      'logic0',
      this.procedureAction,
      { args, callType: this.queryFuncName },
      undefined,
      (callTreeResults: any): void => {
        handler(callTreeResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length === 0) {
          this.callTreeProgressEL!.loading = false;
          this.loadingPage.style.visibility = 'hidden';
        }
      }
    );
  }

  initHtml(): string {
    return `
        <style>
        .call-tree-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        .call-tree-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        selector{
            display: none;
        }
        .call-tree-loading{
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
    <div class="call-tree-content" style="display: flex;flex-direction: column">
    <lit-headline class="titleBox"></lit-headline>
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <lit-table id="tb-calltree" style="height: auto" tree>
                <lit-table-column class="call-tree-column" width="70%" title="Call Stack" data-index="symbolName" key="symbolName"  align="flex-start"retract></lit-table-column>
                <lit-table-column class="call-tree-column" width="1fr" title="Local" data-index="self" key="self"  align="flex-start"  order></lit-table-column>
                <lit-table-column class="call-tree-column" width="1fr" title="Weight" data-index="weight" key="weight"  align="flex-start"  order></lit-table-column>
                <lit-table-column class="call-tree-column" width="1fr" title="%" data-index="weightPercent" key="weightPercent"  align="flex-start"  order></lit-table-column>
            </lit-table>
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <lit-table id="tb-list" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
            <span slot="head">Heaviest Stack Trace</span>
            <lit-table-column class="call-tree-column" width="30px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column class="call-tree-column" width="60px" title="" data-index="count" key="count"  align="flex-start"></lit-table-column>
            <lit-table-column class="call-tree-column" width="1fr" title="" data-index="symbolName" key="symbolName"  align="flex-start"></lit-table-column>
        </lit-table>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="filter" class="call-tree-filter" input inputLeftText icon tree fileSystem></tab-pane-filter>
     <lit-progress-bar class="progress call-tree-progress"></lit-progress-bar>
    <selector id='show_chart' class="call-tree-selector" >
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>
    <div class="loading call-tree-loading"></div>
    </div>`;
  }
}
