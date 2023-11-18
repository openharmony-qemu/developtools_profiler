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
import { FileMerageBean } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { queryNativeHookSubType, queryNativeHookStatisticSubType } from '../../../../database/SqlLite.js';
import { ParseExpression } from '../SheetUtils.js';
import { FilterByAnalysis, NativeMemoryExpression } from '../../../../bean/NativeHook.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import '../../../../../base-ui/headline/lit-headline.js';
import { LitHeadLine } from '../../../../../base-ui/headline/lit-headline.js';

const InvertOpyionIndex: number = 0;
const HideSystemSoOptionIndex: number = 1;
const HideThreadOptionIndex: number = 3;

@element('tabpane-nm-calltree')
export class TabpaneNMCalltree extends BaseElement {
  private nmCallTreeTbl: LitTable | null | undefined;
  private filesystemTbr: LitTable | null | undefined;
  private nmCallTreeProgressEL: LitProgressBar | null | undefined;
  private nmCallTreeFilter: TabPaneFilter | null | undefined;
  private nmCallTreeSource: any[] = [];
  private nativeType: Array<string> = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
  private sortKey: string = 'heapSizeStr';
  private sortType: number = 0;
  private currentSelectedData: any = undefined;
  private nmCallTreeFrameChart: FrameChart | null | undefined;
  private isChartShow: boolean = false;
  private systmeRuleName: string = '/system/';
  private numRuleName: string = '/max/min/';
  private needShowMenu: boolean = true;
  private searchValue: string = '';
  private loadingList: number[] = [];
  private nmCallTreeLoadingPage: any;
  private currentSelection: SelectionParam | undefined;
  private filterAllocationType: string = '0';
  private filterNativeType: string = '0';
  private filterResponseType: number = -1;
  private filterResponseSelect: string = '0';
  private responseTypes: any[] = [];
  private subTypeArr: number[] = [];
  private lastIsExpression = false;
  private currentNMCallTreeFilter: TabPaneFilter | undefined | null;
  private expressionStruct: NativeMemoryExpression | null = null;
  private isHideThread: boolean = false;
  private currentSelectIPid = 1;
  private headLine: LitHeadLine | null | undefined;
  private _filterData: FilterByAnalysis | undefined;
  private _analysisTabWidth: number = 0;
  private _initFromAnalysis = false;

  set analysisTabWidth(width: number) {
    this._analysisTabWidth = width;
  }

  set titleTxt(value: string) {
    this.headLine!.titleTxt = value;
  }

  set filterData(data: FilterByAnalysis) {
    // click from analysis
    this._filterData = data;
  }

  set titleBoxShow(value: Boolean) {
    this.headLine!.isShow = value;
  }

  set initFromAnalysis(flag: boolean) {
    this._initFromAnalysis = flag;
  }

  set data(nmCallTreeParam: SelectionParam) {
    if (nmCallTreeParam === this.currentSelection) {
      return;
    }
    this.nmCallTreeSource = [];
    this.currentSelection = nmCallTreeParam;
    this.currentSelectIPid = nmCallTreeParam.nativeMemoryCurrentIPid;
    this.init(nmCallTreeParam);
  }

  private async init(nmCallTreeParam: SelectionParam): Promise<void> {
    this.initUI();
    await this.initFilterTypes();
    let types: Array<string | number> = [];
    this.initTypes(nmCallTreeParam, types);
    const initWidth = this._analysisTabWidth > 0 ? this._analysisTabWidth : this.clientWidth;
    this.getDataByWorkerQuery(
      {
        leftNs: nmCallTreeParam.leftNs,
        rightNs: nmCallTreeParam.rightNs,
        types,
      },
      (results: any[]): void => {
        this.setLTableData(results);
        this.filesystemTbr!.recycleDataSource = [];

        this.nmCallTreeFrameChart?.updateCanvas(true, initWidth);
        if (this._initFromAnalysis) {
          this.filterByAnalysis();
        } else {
          this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
          this.switchFlameChart();
          this.nmCallTreeFilter!.icon = 'block';
        }
      }
    );
  }

  initUI(): void {
    this.headLine!.clear();
    this.isHideThread = false;
    this.searchValue = '';
    this.nmCallTreeTbl!.style.visibility = 'visible';
    if (this.parentElement!.clientHeight > this.nmCallTreeFilter!.clientHeight) {
      this.nmCallTreeFilter!.style.display = 'flex';
    } else {
      this.nmCallTreeFilter!.style.display = 'none';
    }
    procedurePool.submitWithName('logic1', 'native-memory-reset', [], undefined, () => {});
    this.nmCallTreeFilter!.disabledTransfer(true);
    this.nmCallTreeFilter!.initializeFilterTree(true, true, this.currentSelection!.nativeMemory.length > 0);
    this.nmCallTreeFilter!.filterValue = '';

    this.nmCallTreeProgressEL!.loading = true;
    this.nmCallTreeLoadingPage.style.visibility = 'visible';
  }

  initTypes(nmCallTreeParam: SelectionParam, types: Array<string | number>): void {
    if (nmCallTreeParam.nativeMemory.length > 0) {
      this.nmCallTreeFilter!.isStatisticsMemory = false;
      if (nmCallTreeParam.nativeMemory.indexOf(this.nativeType[0]) !== -1) {
        types.push("'AllocEvent'");
        types.push("'MmapEvent'");
      } else {
        if (nmCallTreeParam.nativeMemory.indexOf(this.nativeType[1]) !== -1) {
          types.push("'AllocEvent'");
        }
        if (nmCallTreeParam.nativeMemory.indexOf(this.nativeType[2]) !== -1) {
          types.push("'MmapEvent'");
        }
      }
    } else {
      this.nmCallTreeFilter!.isStatisticsMemory = true;
      if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.nativeType[0]) !== -1) {
        types.push(0);
        types.push(1);
      } else {
        if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.nativeType[1]) !== -1) {
          types.push(0);
        }
        if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.nativeType[2]) !== -1) {
          types.push(1);
        }
      }
    }
  }

  setFilterType(selections: Array<any>, data: FilterByAnalysis): void {
    if (data.type === 'AllocEvent') {
      data.type = '1';
    }
    const that = this;
    if (this.subTypeArr.length > 0) {
      this.subTypeArr.map((memory): void => {
        selections.push({
          memoryTap: memory,
        });
        if (that.currentSelection?.nativeMemory && that.currentSelection.nativeMemory.length > 0) {
          const typeName = SpSystemTrace.DATA_DICT.get(memory);
          if ((data.type === 'MmapEvent' && memory == -1) || data.type === typeName) {
            data.type = `${selections.length + 2}`;
          }
        } else {
          if (
            (data.type === 'MmapEvent' && memory === 1) ||
            (data.type === 'FILE_PAGE_MSG' && memory === 2) ||
            (data.type === 'MEMORY_USING_MSG' && memory === 3)
          ) {
            data.type = `${selections.length + 2}`;
          }
        }
      });
    }
  }

  filterByAnalysis(): void {
    let filterContent: any[] = [];
    let param = new Map<string, any>();
    let selections: Array<any> = [];
    this.setFilterType(selections, this._filterData!);
    param.set('filterByTitleArr', this._filterData);
    param.set('filterAllocType', '0');
    param.set('statisticsSelection', selections);
    param.set('leftNs', this.currentSelection?.leftNs);
    param.set('rightNs', this.currentSelection?.rightNs);
    filterContent.push(
      {
        funcName: 'groupCallchainSample',
        funcArgs: [param],
      },
      {
        funcName: 'getCallChainsBySampleIds',
        funcArgs: [true],
      }
    );
    this.getDataByWorker(filterContent, (result: any[]) => {
      this.setLTableData(result);
      this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
      this.switchFlameChart();
      this.nmCallTreeFilter!.icon = 'block';
      this.initFromAnalysis = false;
    });
    this.banTypeAndLidSelect();
  }
  banTypeAndLidSelect(): void {
    this.currentNMCallTreeFilter!.firstSelect = '0';
    let secondSelect = this.shadowRoot
      ?.querySelector('#nm-call-tree-filter')!
      .shadowRoot!.querySelector('#second-select');
    let thirdSelect = this.shadowRoot
      ?.querySelector('#nm-call-tree-filter')!
      .shadowRoot!.querySelector('#third-select');
    thirdSelect?.setAttribute('disabled', '');
    secondSelect?.setAttribute('disabled', '');
  }

  getParentTree(
    nmCallTreeSrc: Array<FileMerageBean>,
    nmCallTreeTarget: FileMerageBean,
    parents: Array<FileMerageBean>
  ): boolean {
    for (let nmCallTreeBean of nmCallTreeSrc) {
      if (nmCallTreeBean.id === nmCallTreeTarget.id) {
        parents.push(nmCallTreeBean);
        return true;
      } else {
        if (this.getParentTree(nmCallTreeBean.children as Array<FileMerageBean>, nmCallTreeTarget, parents)) {
          parents.push(nmCallTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  getChildTree(nmCallTreeSrc: Array<FileMerageBean>, id: string, children: Array<FileMerageBean>): boolean {
    for (let nmCallTreeBean of nmCallTreeSrc) {
      if (nmCallTreeBean.id === id && nmCallTreeBean.children.length === 0) {
        children.push(nmCallTreeBean);
        return true;
      } else {
        if (this.getChildTree(nmCallTreeBean.children as Array<FileMerageBean>, id, children)) {
          children.push(nmCallTreeBean);
          return true;
        }
      }
    }
    return false;
  }

  setRightTableData(fileMerageBean: FileMerageBean): void {
    let parents: Array<FileMerageBean> = [];
    let children: Array<FileMerageBean> = [];
    this.getParentTree(this.nmCallTreeSource, fileMerageBean, parents);
    let maxId = fileMerageBean.id;
    let maxDur = 0;

    function findMaxStack(merageBean: any): void {
      if (merageBean.children.length === 0) {
        if (merageBean.heapSize > maxDur) {
          maxDur = merageBean.heapSize;
          maxId = merageBean.id;
        }
      } else {
        merageBean.children.map((callChild: any): void => {
          findMaxStack(<FileMerageBean>callChild);
        });
      }
    }

    findMaxStack(fileMerageBean);
    this.getChildTree(fileMerageBean.children as Array<FileMerageBean>, maxId, children);
    let resultValue = parents.reverse().concat(children.reverse());
    for (let data of resultValue) {
      data.type =
        data.libName.endsWith('.so.1') || data.libName.endsWith('.dll') || data.libName.endsWith('.so') ? 0 : 1;
    }
    let resultLength = resultValue.length;
    this.filesystemTbr!.dataSource = resultLength == 0 ? [] : resultValue;
  }
  //底部的筛选菜单
  showBottomMenu(isShow: boolean): void {
    if (isShow) {
      this.nmCallTreeFilter?.showThird(true);
      this.nmCallTreeFilter?.setAttribute('first', '');
      this.nmCallTreeFilter?.setAttribute('second', '');
      this.nmCallTreeFilter?.setAttribute('tree', '');
      this.nmCallTreeFilter?.setAttribute('input', '');
      this.nmCallTreeFilter?.setAttribute('inputLeftText', '');
    } else {
      this.nmCallTreeFilter?.showThird(false);
      this.nmCallTreeFilter?.removeAttribute('first');
      this.nmCallTreeFilter?.removeAttribute('second');
      this.nmCallTreeFilter?.removeAttribute('tree');
      this.nmCallTreeFilter?.removeAttribute('input');
      this.nmCallTreeFilter?.removeAttribute('inputLeftText');
    }
  }

  async initFilterTypes(): Promise<void> {
    this.currentNMCallTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#nm-call-tree-filter');
    let secondFilterList = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];

    let that = this;
    function addSubType(subTypeList: any) {
      if (!subTypeList) {
        return;
      }
      that.subTypeArr = [];
      for (let data of subTypeList) {
        secondFilterList.push(data.subType);
        that.subTypeArr.push(data.subTypeId);
      }
    }

    if (this.currentSelection!.nativeMemory!.length > 0) {
      let subTypeList = await queryNativeHookSubType(
        this.currentSelection!.leftNs,
        this.currentSelection!.rightNs,
        this.currentSelectIPid
      );
      addSubType(subTypeList);
    } else {
      let subTypeList = await queryNativeHookStatisticSubType(
        this.currentSelection!.leftNs,
        this.currentSelection!.rightNs,
        this.currentSelectIPid
      );
      addSubType(subTypeList);
    }
    procedurePool.submitWithName('logic1', 'native-memory-get-responseType', {}, undefined, (res: any) => {
      this.responseTypes = res;
      let nullIndex = this.responseTypes.findIndex((item) => {
        return item.key == 0;
      });
      if (nullIndex != -1) {
        this.responseTypes.splice(nullIndex, 1);
      }
      this.currentNMCallTreeFilter!.setSelectList(
        null,
        secondFilterList,
        'Allocation Lifespan',
        'Allocation Type',
        this.responseTypes.map((item: any) => {
          return item.value;
        })
      );
      this.currentNMCallTreeFilter!.setFilterModuleSelect('#first-select', 'width', '150px');
      this.currentNMCallTreeFilter!.setFilterModuleSelect('#second-select', 'width', '150px');
      this.currentNMCallTreeFilter!.setFilterModuleSelect('#third-select', 'width', '150px');
      this.currentNMCallTreeFilter!.firstSelect = '0';
      this.currentNMCallTreeFilter!.secondSelect = '0';
      this.currentNMCallTreeFilter!.thirdSelect = '0';
      this.filterAllocationType = '0';
      this.filterNativeType = '0';
      this.filterResponseSelect = '0';
      this.filterResponseType = -1;
    });
  }

  initElements(): void {
    this.headLine = this.shadowRoot?.querySelector<LitHeadLine>('.titleBox');
    this.nmCallTreeTbl = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-calltree');
    this.nmCallTreeProgressEL = this.shadowRoot?.querySelector('.nm-call-tree-progress') as LitProgressBar;
    this.nmCallTreeFrameChart = this.shadowRoot?.querySelector<FrameChart>('#framechart');
    this.nmCallTreeFrameChart!.mode = ChartMode.Byte;
    this.nmCallTreeLoadingPage = this.shadowRoot?.querySelector('.nm-call-tree-loading');
    this.nmCallTreeFrameChart!.addChartClickListener((needShowMenu: boolean): void => {
      this.parentElement!.scrollTo(0, 0);
      this.showBottomMenu(needShowMenu);
      this.needShowMenu = needShowMenu;
    });
    this.nmCallTreeTbl!.rememberScrollTop = true;
    this.nmCallTreeTbl!.exportTextHandleMap.set('heapSizeStr', (value) => {
      return `${value['size']}`;
    });
    this.nmCallTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#nm-call-tree-filter');
    this.nmCallTreeTbl!.addEventListener('row-click', (event: any): void => {
      // @ts-ignore
      let data = event.detail.data as FileMerageBean;
      this.setRightTableData(data);
      data.isSelected = true;
      this.currentSelectedData = data;
      this.filesystemTbr?.clearAllSelection(data);
      this.filesystemTbr?.setCurrentSelection(data);
      // @ts-ignore
      if ((event.detail as any).callBack) {
        // @ts-ignore
        (event.detail as any).callBack(true);
      }
      document.dispatchEvent(
        new CustomEvent('number_calibration', {
          detail: { time: event.detail.tsArray, counts: event.detail.countArray },
        })
      );
    });
    this.filesystemTbr = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-list');
    this.filesystemTbr!.addEventListener('row-click', (evt: any): void => {
      // @ts-ignore
      let data = evt.detail.data as FileMerageBean;
      this.nmCallTreeTbl?.clearAllSelection(data);
      (data as any).isSelected = true;
      this.nmCallTreeTbl!.scrollToData(data);
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });
    let filterFunc = (nmCallTreeFuncData: any): void => {
      let nmCallTreeFuncArgs: any[] = [];
      if (nmCallTreeFuncData.type === 'check') {
        if (nmCallTreeFuncData.item.checked) {
          nmCallTreeFuncArgs.push({
            funcName: 'splitTree',
            funcArgs: [
              nmCallTreeFuncData.item.name,
              nmCallTreeFuncData.item.select === '0',
              nmCallTreeFuncData.item.type === 'symbol',
            ],
          });
        } else {
          nmCallTreeFuncArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[nmCallTreeFuncData.item.name]],
          });
          nmCallTreeFuncArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          nmCallTreeFuncArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [nmCallTreeFuncData.item.name],
          });
        }
      } else if (nmCallTreeFuncData.type == 'select') {
        nmCallTreeFuncArgs.push({
          funcName: 'resotreAllNode',
          funcArgs: [[nmCallTreeFuncData.item.name]],
        });
        nmCallTreeFuncArgs.push({
          funcName: 'clearSplitMapData',
          funcArgs: [nmCallTreeFuncData.item.name],
        });
        nmCallTreeFuncArgs.push({
          funcName: 'splitTree',
          funcArgs: [
            nmCallTreeFuncData.item.name,
            nmCallTreeFuncData.item.select === '0',
            nmCallTreeFuncData.item.type == 'symbol',
          ],
        });
      } else if (nmCallTreeFuncData.type === 'button') {
        if (nmCallTreeFuncData.item == 'symbol') {
          if (this.currentSelectedData && !this.currentSelectedData.canCharge) {
            return;
          }
          if (this.currentSelectedData !== undefined) {
            this.nmCallTreeFilter!.addDataMining(
              { name: this.currentSelectedData.symbolName },
              nmCallTreeFuncData.item
            );
            nmCallTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.currentSelectedData.symbolName, false, true],
            });
          } else {
            return;
          }
        } else if (nmCallTreeFuncData.item === 'library') {
          if (this.currentSelectedData && !this.currentSelectedData.canCharge) {
            return;
          }
          if (this.currentSelectedData != undefined && this.currentSelectedData.libName !== '') {
            this.nmCallTreeFilter!.addDataMining({ name: this.currentSelectedData.libName }, nmCallTreeFuncData.item);
            nmCallTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.currentSelectedData.libName, false, false],
            });
          } else {
            return;
          }
        } else if (nmCallTreeFuncData.item === 'restore') {
          if (nmCallTreeFuncData.remove != undefined && nmCallTreeFuncData.remove.length > 0) {
            let list = nmCallTreeFuncData.remove.map((item: any): any => {
              return item.name;
            });
            nmCallTreeFuncArgs.push({
              funcName: 'resotreAllNode',
              funcArgs: [list],
            });
            nmCallTreeFuncArgs.push({
              funcName: 'resetAllNode',
              funcArgs: [],
            });
            list.forEach((symbolName: string): void => {
              nmCallTreeFuncArgs.push({
                funcName: 'clearSplitMapData',
                funcArgs: [symbolName],
              });
            });
          }
        }
      }
      this.getDataByWorker(nmCallTreeFuncArgs, (result: any[]): void => {
        this.setLTableData(result);
        this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
        if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
        this.nmCallTreeTbl!.move1px();
        if (this.currentSelectedData) {
          this.currentSelectedData.isSelected = false;
          this.nmCallTreeTbl?.clearAllSelection(this.currentSelectedData);
          this.filesystemTbr!.recycleDataSource = [];
          this.currentSelectedData = undefined;
        }
      });
    };
    this.nmCallTreeFilter!.getDataLibrary(filterFunc);
    this.nmCallTreeFilter!.getDataMining(filterFunc);
    this.nmCallTreeFilter!.getCallTreeData((callTreeData: any): void => {
      if ([InvertOpyionIndex, HideSystemSoOptionIndex, HideThreadOptionIndex].includes(callTreeData.value)) {
        this.refreshAllNode({
          ...this.nmCallTreeFilter!.getFilterTreeData(),
          callTree: callTreeData.checks,
        });
      } else {
        let resultArgs: any[] = [];
        if (callTreeData.checks[1]) {
          resultArgs.push({
            funcName: 'hideSystemLibrary',
            funcArgs: [],
          });
          resultArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
        } else {
          resultArgs.push({
            funcName: 'resotreAllNode',
            funcArgs: [[this.systmeRuleName]],
          });
          resultArgs.push({
            funcName: 'resetAllNode',
            funcArgs: [],
          });
          resultArgs.push({
            funcName: 'clearSplitMapData',
            funcArgs: [this.systmeRuleName],
          });
        }
        this.getDataByWorker(resultArgs, (result: any[]): void => {
          this.setLTableData(result);
          this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
          if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
        });
      }
    });
    this.nmCallTreeFilter!.getCallTreeConstraintsData((nmCallTreeConstraintsData: any): void => {
      let nmCallTreeConstraintsArgs: any[] = [
        {
          funcName: 'resotreAllNode',
          funcArgs: [[this.numRuleName]],
        },
        {
          funcName: 'clearSplitMapData',
          funcArgs: [this.numRuleName],
        },
      ];
      if (nmCallTreeConstraintsData.checked) {
        nmCallTreeConstraintsArgs.push({
          funcName: 'hideNumMaxAndMin',
          funcArgs: [parseInt(nmCallTreeConstraintsData.min), nmCallTreeConstraintsData.max],
        });
      }
      nmCallTreeConstraintsArgs.push({
        funcName: 'resetAllNode',
        funcArgs: [],
      });
      this.getDataByWorker(nmCallTreeConstraintsArgs, (result: any[]): void => {
        this.setLTableData(result);
        this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
        if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
      });
    });
    this.nmCallTreeFilter!.getFilterData((nmCallTreeData: FilterData): void => {
      if (
        (this.isChartShow && nmCallTreeData.icon === 'tree') ||
        (!this.isChartShow && nmCallTreeData.icon === 'block')
      ) {
        this.switchFlameChart(nmCallTreeData);
      } else {
        if (
          this.filterAllocationType !== nmCallTreeData.firstSelect ||
          this.filterNativeType !== nmCallTreeData.secondSelect ||
          this.filterResponseSelect !== nmCallTreeData.thirdSelect
        ) {
          this.filterAllocationType = nmCallTreeData.firstSelect || '0';
          this.filterNativeType = nmCallTreeData.secondSelect || '0';
          this.filterResponseSelect = nmCallTreeData.thirdSelect || '0';
          let thirdIndex = parseInt(nmCallTreeData.thirdSelect || '0');
          if (this.responseTypes.length > thirdIndex) {
            this.filterResponseType = this.responseTypes[thirdIndex].key || -1;
          }
          this.searchValue = this.nmCallTreeFilter!.filterValue;
          this.expressionStruct = new ParseExpression(this.searchValue).parse();
          this.refreshAllNode(this.nmCallTreeFilter!.getFilterTreeData());
        } else if (this.searchValue !== this.nmCallTreeFilter!.filterValue) {
          this.searchValue = this.nmCallTreeFilter!.filterValue;
          this.expressionStruct = new ParseExpression(this.searchValue).parse();
          let nmArgs = [];
          if (this.expressionStruct) {
            this.refreshAllNode(this.nmCallTreeFilter!.getFilterTreeData());
            this.lastIsExpression = true;
            return;
          } else {
            if (this.lastIsExpression) {
              this.refreshAllNode(this.nmCallTreeFilter!.getFilterTreeData());
              this.lastIsExpression = false;
              return;
            }
            nmArgs.push({
              funcName: 'setSearchValue',
              funcArgs: [this.searchValue],
            });
            nmArgs.push({
              funcName: 'resetAllNode',
              funcArgs: [],
            });
            this.lastIsExpression = false;
          }
          this.getDataByWorker(nmArgs, (result: any[]): void => {
            this.nmCallTreeTbl!.isSearch = true;
            this.nmCallTreeTbl!.setStatus(result, true);
            this.setLTableData(result);
            this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
            this.switchFlameChart(nmCallTreeData);
          });
        } else {
          this.switchFlameChart(nmCallTreeData);
        }
      }
    });
    this.nmCallTreeTbl!.addEventListener('column-click', (evt): void => {
      // @ts-ignore
      this.sortKey = evt.detail.key;
      // @ts-ignore
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.setLTableData(this.nmCallTreeSource, true);
      this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
    });
    //点击之后删除掉筛选条件  将所有重置  将目前的title隐藏 高度恢复
    this.headLine!.closeCallback = () => {
      this.headLine!.clear();
      this.searchValue = '';
      this.currentNMCallTreeFilter!.filterValue = '';
      this._filterData = undefined;
      this.currentNMCallTreeFilter!.firstSelect = '0';
      this.currentNMCallTreeFilter!.secondSelect = '0';
      this.currentNMCallTreeFilter!.thirdSelect = '0';
      this.filterAllocationType = '0';
      this.refreshAllNode(this.nmCallTreeFilter!.getFilterTreeData(), true);
      this.initFilterTypes();
      this.nmCallTreeFrameChart?.resizeChange();
    };
  }

  connectedCallback(): void {
    super.connectedCallback();
    let filterHeight = 0;
    new ResizeObserver((entries: ResizeObserverEntry[]): void => {
      let nmCallTreeTabFilter = this.shadowRoot!.querySelector('#nm-call-tree-filter') as HTMLElement;
      if (nmCallTreeTabFilter.clientHeight > 0) {
        filterHeight = nmCallTreeTabFilter.clientHeight;
      }
      if (this.parentElement!.clientHeight > filterHeight) {
        nmCallTreeTabFilter.style.display = 'flex';
      } else {
        nmCallTreeTabFilter.style.display = 'none';
      }
      if (this.nmCallTreeTbl!.style.visibility === 'hidden') {
        nmCallTreeTabFilter.style.display = 'none';
      }
      if (this.parentElement?.clientHeight !== 0) {
        if (this.isChartShow) {
          this.nmCallTreeFrameChart?.updateCanvas(false, entries[0].contentRect.width);
          this.nmCallTreeFrameChart?.calculateChartData();
        }
        // @ts-ignore
        this.nmCallTreeTbl?.shadowRoot.querySelector('.table').style.height = `${
          this.parentElement!.clientHeight - 10 - 35
        }px`;
        this.nmCallTreeTbl?.reMeauseHeight();
        // @ts-ignore
        this.filesystemTbr?.shadowRoot.querySelector('.table').style.height = `${
          this.parentElement!.clientHeight - 45 - 21
        }px`;
        this.filesystemTbr?.reMeauseHeight();
        this.nmCallTreeLoadingPage.style.height = `${this.parentElement!.clientHeight - 24}px`;
      }
    }).observe(this.parentElement!);
    this.parentElement!.onscroll = (): void => {
      this.nmCallTreeFrameChart!.tabPaneScrollTop = this.parentElement!.scrollTop;
    };
  }

  switchFlameChart(flameChartData?: any): void {
    let nmCallTreePageTab = this.shadowRoot?.querySelector('#show_table');
    let nmCallTreePageChart = this.shadowRoot?.querySelector('#show_chart');
    if (!flameChartData || flameChartData.icon === 'block') {
      nmCallTreePageChart?.setAttribute('class', 'show');
      nmCallTreePageTab?.setAttribute('class', '');
      this.isChartShow = true;
      this.nmCallTreeFilter!.disabledMining = true;
      this.showBottomMenu(this.needShowMenu);
      this.nmCallTreeFrameChart?.calculateChartData();
    } else if (flameChartData.icon === 'tree') {
      nmCallTreePageChart?.setAttribute('class', '');
      nmCallTreePageTab?.setAttribute('class', 'show');
      this.showBottomMenu(true);
      this.isChartShow = false;
      this.nmCallTreeFilter!.disabledMining = false;
      this.nmCallTreeFrameChart!.clearCanvas();
      this.nmCallTreeTbl!.reMeauseHeight();
    }
  }

  refreshAllNode(filterData: any, isAnalysisReset?: boolean): void {
    let nmCallTreeArgs: any[] = [];
    let isTopDown: boolean = !filterData.callTree[0];
    let isHideSystemLibrary = filterData.callTree[1];
    this.isHideThread = filterData.callTree[3];
    let list = filterData.dataMining.concat(filterData.dataLibrary);
    let groupArgs = new Map<string, any>();
    groupArgs.set('filterAllocType', this.filterAllocationType);
    groupArgs.set('filterEventType', this.filterNativeType);
    if (this.expressionStruct) {
      groupArgs.set('filterExpression', this.expressionStruct);
      groupArgs.set('filterResponseType', -1);
      this.currentNMCallTreeFilter!.thirdSelect = '0';
    } else {
      groupArgs.set('filterResponseType', this.filterResponseType);
    }
    groupArgs.set('leftNs', this.currentSelection?.leftNs || 0);
    groupArgs.set('rightNs', this.currentSelection?.rightNs || 0);
    let selections: Array<any> = [];
    if (this.subTypeArr.length > 0) {
      this.subTypeArr.map((memory): void => {
        selections.push({
          memoryTap: memory,
        });
      });
    }
    groupArgs.set('statisticsSelection', selections);
    if (this._filterData) {
      groupArgs.set('filterByTitleArr', this._filterData);
    }
    groupArgs.set(
      'nativeHookType',
      this.currentSelection!.nativeMemory.length > 0 ? 'native-hook' : 'native-hook-statistic'
    );
    if ((this.lastIsExpression && !this.expressionStruct) || isAnalysisReset) {
      nmCallTreeArgs.push({
        funcName: 'setSearchValue',
        funcArgs: [this.searchValue],
      });
    }
    nmCallTreeArgs.push({
      funcName: 'hideThread',
      funcArgs: [this.isHideThread],
    });
    nmCallTreeArgs.push(
      {
        funcName: 'groupCallchainSample',
        funcArgs: [groupArgs],
      },
      {
        funcName: 'getCallChainsBySampleIds',
        funcArgs: [isTopDown],
      }
    );
    this.filesystemTbr!.recycleDataSource = [];
    if (isHideSystemLibrary) {
      nmCallTreeArgs.push({
        funcName: 'hideSystemLibrary',
        funcArgs: [],
      });
    }
    if (filterData.callTreeConstraints.checked) {
      nmCallTreeArgs.push({
        funcName: 'hideNumMaxAndMin',
        funcArgs: [parseInt(filterData.callTreeConstraints.inputs[0]), filterData.callTreeConstraints.inputs[1]],
      });
    }
    nmCallTreeArgs.push({
      funcName: 'splitAllProcess',
      funcArgs: [list],
    });
    nmCallTreeArgs.push({
      funcName: 'resetAllNode',
      funcArgs: [],
    });
    this.getDataByWorker(nmCallTreeArgs, (result: any[]) => {
      this.setLTableData(result);
      this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
      if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
    });
  }

  setLTableData(resultData: any[], sort?: boolean): void {
    if (sort) {
      this.nmCallTreeSource = this.sortTree(resultData);
    } else {
      if (resultData && resultData[0]) {
        this.nmCallTreeSource =
          this.currentSelection!.nativeMemory.length > 0 && !this.isHideThread
            ? this.sortTree(resultData)
            : this.sortTree(resultData[0].children || []);
      } else {
        this.nmCallTreeSource = [];
      }
    }
    this.nmCallTreeTbl!.recycleDataSource = this.nmCallTreeSource;
  }

  sortTree(arr: Array<any>): Array<any> {
    let nmCallTreeSortArr = arr.sort((callTreeLeftData, callTreeRightData): number => {
      if (this.sortKey === 'heapSizeStr' || this.sortKey === 'heapPercent') {
        if (this.sortType === 0) {
          return callTreeRightData.size - callTreeLeftData.size;
        } else if (this.sortType === 1) {
          return callTreeLeftData.size - callTreeRightData.size;
        } else {
          return callTreeRightData.size - callTreeLeftData.size;
        }
      } else {
        if (this.sortType === 0) {
          return callTreeRightData.count - callTreeLeftData.count;
        } else if (this.sortType === 1) {
          return callTreeLeftData.count - callTreeRightData.count;
        } else {
          return callTreeRightData.count - callTreeLeftData.count;
        }
      }
    });
    nmCallTreeSortArr.map((call): void => {
      call.children = this.sortTree(call.children);
    });
    return nmCallTreeSortArr;
  }

  getDataByWorker(args: any[], handler: Function): void {
    this.loadingList.push(1);
    this.nmCallTreeProgressEL!.loading = true;
    this.nmCallTreeLoadingPage.style.visibility = 'visible';
    procedurePool.submitWithName(
      'logic1',
      'native-memory-calltree-action',
      args,
      undefined,
      (callTreeActionResults: any): void => {
        handler(callTreeActionResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length === 0) {
          this.nmCallTreeProgressEL!.loading = false;
          this.nmCallTreeLoadingPage.style.visibility = 'hidden';
        }
      }
    );
  }

  getDataByWorkerQuery(args: any, handler: Function): void {
    this.loadingList.push(1);
    this.nmCallTreeProgressEL!.loading = true;
    this.nmCallTreeLoadingPage.style.visibility = 'visible';
    procedurePool.submitWithName(
      'logic1',
      this.currentSelection!.nativeMemory!.length > 0
        ? 'native-memory-queryCallchainsSamples'
        : 'native-memory-queryStatisticCallchainsSamples',
      args,
      undefined,
      (callChainsResults: any): void => {
        handler(callChainsResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length === 0) {
          this.nmCallTreeProgressEL!.loading = false;
          this.nmCallTreeLoadingPage.style.visibility = 'hidden';
        }
      }
    );
  }

  initHtml(): string {
    return `
        <style>
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        .show{
            display: flex;
            flex: 1;
        }
        #nm-call-tree-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        selector{
            display: none;
        }
        .nm-call-tree-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .nm-call-tree-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
    </style>
    <lit-headline class="titleBox"></lit-headline>
    <div class="nm-call-tree-content" style="display: flex;flex-direction: row">
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <tab-native-data-modal id="modal"></tab-native-data-modal>
            <lit-table id="tb-filesystem-calltree" style="height: auto" tree>
                <lit-table-column class="nm-call-tree-column" width="60%" title="Symbol Name" data-index="symbolName" key="symbolName"  align="flex-start"retract>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="Size" data-index="heapSizeStr" key="heapSizeStr"  align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="%" data-index="heapPercent" key="heapPercent" align="flex-start"  order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="Count" data-index="countValue" key="countValue" align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="%" data-index="countPercent" key="countPercent" align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="  " data-index="type" key="type"  align="flex-start" >
                    <template>
                        <img src="img/library.png" size="20" v-if=" type == 1 ">
                        <img src="img/function.png" size="20" v-if=" type == 0 ">
                        <div v-if=" type == - 1 "></div>
                    </template>
                </lit-table-column>
            </lit-table>
            
        </div>
        <lit-slicer-track class="nm-call-tree-slicer-track" ></lit-slicer-track>
        <lit-table id="tb-filesystem-list" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
            <span slot="head">Heaviest Stack Trace</span>
            <lit-table-column class="nm-call-tree-column" width="30px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column class="nm-call-tree-column" width="1fr" title="" data-index="symbolName" key="symbolName"  align="flex-start"></lit-table-column>
        </lit-table>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="nm-call-tree-filter" first second icon nativeMemory></tab-pane-filter>
     <lit-progress-bar class="progress nm-call-tree-progress"></lit-progress-bar>
    <selector class="nm-call-tree-selector" id='show_chart'>
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>  
    <div class="loading nm-call-tree-loading"></div>
    </div>`;
  }
}
