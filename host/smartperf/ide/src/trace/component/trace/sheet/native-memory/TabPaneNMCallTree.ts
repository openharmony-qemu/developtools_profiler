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
import { DisassemblingWindow } from '../../../DisassemblingWindow.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { ChartMode } from '../../../../bean/FrameChartStruct.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { FileMerageBean } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';
import { queryNativeHookSubType, queryNativeHookStatisticSubType } from '../../../../database/SqlLite.js';

@element('tabpane-nm-calltree')
export class TabpaneNMCalltree extends BaseElement {
  private nmCallTreeTbl: LitTable | null | undefined;
  private filesystemTbr: LitTable | null | undefined;
  private nmCallTreeProgressEL: LitProgressBar | null | undefined;
  private rightSource: Array<FileMerageBean> = [];
  private nmCallTreeFilter: any;
  private nmCallTreeSource: any[] = [];
  private native_type: Array<string> = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
  private sortKey = 'heapSizeStr';
  private sortType = 0;
  private currentSelectedData: any = undefined;
  private nmCallTreeFrameChart: FrameChart | null | undefined;
  private isChartShow: boolean = false;
  private systmeRuleName = '/system/';
  private numRuleName = '/max/min/';
  private modal: DisassemblingWindow | null | undefined;
  private needShowMenu = true;
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

  set data(nmCallTreeParam: SelectionParam | any) {
    if (nmCallTreeParam == this.currentSelection) {
      return;
    }
    this.searchValue = '';
    this.currentSelection = nmCallTreeParam;
    this.modal!.style.display = 'none';
    this.nmCallTreeTbl!.style.visibility = 'visible';
    if (this.parentElement!.clientHeight > this.nmCallTreeFilter!.clientHeight) {
      this.nmCallTreeFilter!.style.display = 'flex';
    } else {
      this.nmCallTreeFilter!.style.display = 'none';
    }
    this.nmCallTreeFilter!.initializeFilterTree(true, true, nmCallTreeParam.nativeMemory.length > 0);
    this.nmCallTreeFilter!.filterValue = '';
    this.initFilterTypes();
    this.nmCallTreeProgressEL!.loading = true;
    this.nmCallTreeLoadingPage.style.visibility = 'visible';
    let types: Array<string | number> = [];
    if (nmCallTreeParam.nativeMemory.length > 0) {
      if (nmCallTreeParam.nativeMemory.indexOf(this.native_type[0]) != -1) {
        types.push("'AllocEvent'");
        types.push("'MmapEvent'");
      } else {
        if (nmCallTreeParam.nativeMemory.indexOf(this.native_type[1]) != -1) {
          types.push("'AllocEvent'");
        }
        if (nmCallTreeParam.nativeMemory.indexOf(this.native_type[2]) != -1) {
          types.push("'MmapEvent'");
        }
      }
    } else {
      if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.native_type[0]) != -1) {
        types.push(0);
        types.push(1);
      } else {
        if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.native_type[1]) != -1) {
          types.push(0);
        }
        if (nmCallTreeParam.nativeMemoryStatistic.indexOf(this.native_type[2]) != -1) {
          types.push(1);
        }
      }
    }

    this.getDataByWorkerQuery(
      {
        leftNs: nmCallTreeParam.leftNs,
        rightNs: nmCallTreeParam.rightNs,
        types,
      },
      (results: any[]) => {
        this.setLTableData(results);
        this.filesystemTbr!.recycleDataSource = [];
        this.nmCallTreeFrameChart!.mode = ChartMode.Byte;
        this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
        this.nmCallTreeFrameChart?.updateCanvas(true, this.clientWidth);
        this.nmCallTreeFrameChart?.calculateChartData();
        this.switchFlameChart();
        this.nmCallTreeFilter.icon = 'block';
      }
    );
  }

  getParentTree(
    nmCallTreeSrc: Array<FileMerageBean>,
    nmCallTreeTarget: FileMerageBean,
    parents: Array<FileMerageBean>
  ): boolean {
    for (let nmCallTreeBean of nmCallTreeSrc) {
      if (nmCallTreeBean.id == nmCallTreeTarget.id) {
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
      if (nmCallTreeBean.id == id && nmCallTreeBean.children.length == 0) {
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

  setRightTableData(fileMerageBean: FileMerageBean) {
    let parents: Array<FileMerageBean> = [];
    let children: Array<FileMerageBean> = [];
    this.getParentTree(this.nmCallTreeSource, fileMerageBean, parents);
    let maxId = fileMerageBean.id;
    let maxDur = 0;

    function findMaxStack(merageBean: any) {
      if (merageBean.children.length == 0) {
        if (merageBean.heapSize > maxDur) {
          maxDur = merageBean.heapSize;
          maxId = merageBean.id;
        }
      } else {
        merageBean.children.map((callChild: any) => {
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
    this.rightSource = resultValue;
    this.filesystemTbr!.dataSource = resultLength == 0 ? [] : resultValue;
  }

  showBottomMenu(isShow: boolean) {
    if (isShow) {
      this.nmCallTreeFilter.showThird(true);
      this.nmCallTreeFilter.setAttribute('first', '');
      this.nmCallTreeFilter.setAttribute('second', '');
      this.nmCallTreeFilter.setAttribute('tree', '');
      this.nmCallTreeFilter.setAttribute('input', '');
      this.nmCallTreeFilter.setAttribute('inputLeftText', '');
    } else {
      this.nmCallTreeFilter.showThird(false);
      this.nmCallTreeFilter.removeAttribute('first');
      this.nmCallTreeFilter.removeAttribute('second');
      this.nmCallTreeFilter.removeAttribute('tree');
      this.nmCallTreeFilter.removeAttribute('input');
      this.nmCallTreeFilter.removeAttribute('inputLeftText');
    }
  }

  async initFilterTypes() {
    let currentNMCallTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#nm-call-tree-filter');
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
      let subTypeList = await queryNativeHookSubType(this.currentSelection!.leftNs, this.currentSelection!.rightNs);
      addSubType(subTypeList);
    } else {
      let subTypeList = await queryNativeHookStatisticSubType(
        this.currentSelection!.leftNs,
        this.currentSelection!.rightNs
      );
      addSubType(subTypeList);
    }

    if (this.currentSelection!.nativeMemory.length > 0) {
      procedurePool.submitWithName('logic1', 'native-memory-get-responseType', {}, undefined, (res: any) => {
        this.responseTypes = res;
        let nullIndex = this.responseTypes.findIndex((item) => {
          return item.key == 0;
        });
        if (nullIndex != -1) {
          this.responseTypes.splice(nullIndex, 1);
        }
        currentNMCallTreeFilter!.setSelectList(
          null,
          secondFilterList,
          'Allocation Lifespan',
          'Allocation Type',
          this.responseTypes.map((item: any) => {
            return item.value;
          })
        );
        currentNMCallTreeFilter!.setFilterModuleSelect('#first-select', 'width', '150px');
        currentNMCallTreeFilter!.setFilterModuleSelect('#second-select', 'width', '150px');
        currentNMCallTreeFilter!.setFilterModuleSelect('#third-select', 'width', '150px');
        currentNMCallTreeFilter!.firstSelect = '0';
        currentNMCallTreeFilter!.secondSelect = '0';
        currentNMCallTreeFilter!.thirdSelect = '0';
        this.filterAllocationType = '0';
        this.filterNativeType = '0';
        this.filterResponseSelect = '0';
        this.filterResponseType = -1;
      });
    } else {
      currentNMCallTreeFilter!.setSelectList(
        null,
        secondFilterList,
        'Allocation Lifespan',
        'Allocation Type',
        undefined
      );
      currentNMCallTreeFilter!.setFilterModuleSelect('#first-select', 'width', '150px');
      currentNMCallTreeFilter!.setFilterModuleSelect('#second-select', 'width', '150px');
      currentNMCallTreeFilter!.firstSelect = '0';
      currentNMCallTreeFilter!.secondSelect = '0';
      this.filterAllocationType = '0';
      this.filterNativeType = '0';
    }
  }

  initElements(): void {
    this.nmCallTreeTbl = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-calltree');
    this.nmCallTreeProgressEL = this.shadowRoot?.querySelector('.nm-call-tree-progress') as LitProgressBar;
    this.nmCallTreeFrameChart = this.shadowRoot?.querySelector<FrameChart>('#framechart');
    this.modal = this.shadowRoot?.querySelector<DisassemblingWindow>('tab-native-data-modal');
    this.nmCallTreeLoadingPage = this.shadowRoot?.querySelector('.nm-call-tree-loading');
    this.nmCallTreeFrameChart!.addChartClickListener((needShowMenu: boolean) => {
      this.parentElement!.scrollTo(0, 0);
      this.showBottomMenu(needShowMenu);
      this.needShowMenu = needShowMenu;
    });
    this.nmCallTreeTbl!.rememberScrollTop = true;
    this.nmCallTreeFilter = this.shadowRoot?.querySelector<TabPaneFilter>('#nm-call-tree-filter');
    this.nmCallTreeTbl!.addEventListener('row-click', (event: any) => {
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
    });
    this.filesystemTbr = this.shadowRoot?.querySelector<LitTable>('#tb-filesystem-list');
    this.filesystemTbr!.addEventListener('row-click', (evt: any) => {
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
    this.modal!.setCloseListener(() => {
      this.modal!.style.display = 'none';
      this.nmCallTreeTbl!.style.visibility = 'visible';
      this.shadowRoot!.querySelector<TabPaneFilter>('#nm-call-tree-filter')!.style.display = 'flex';
    });
    let filterFunc = (nmCallTreeFuncData: any) => {
      let nmCallTreeFuncArgs: any[] = [];
      if (nmCallTreeFuncData.type == 'check') {
        if (nmCallTreeFuncData.item.checked) {
          nmCallTreeFuncArgs.push({
            funcName: 'splitTree',
            funcArgs: [
              nmCallTreeFuncData.item.name,
              nmCallTreeFuncData.item.select == '0',
              nmCallTreeFuncData.item.type == 'symbol',
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
            nmCallTreeFuncData.item.select == '0',
            nmCallTreeFuncData.item.type == 'symbol',
          ],
        });
      } else if (nmCallTreeFuncData.type == 'button') {
        if (nmCallTreeFuncData.item == 'symbol') {
          if (this.currentSelectedData && !this.currentSelectedData.canCharge) {
            return;
          }
          if (this.currentSelectedData != undefined) {
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
        } else if (nmCallTreeFuncData.item == 'library') {
          if (this.currentSelectedData && !this.currentSelectedData.canCharge) {
            return;
          }
          if (this.currentSelectedData != undefined && this.currentSelectedData.libName != '') {
            this.nmCallTreeFilter!.addDataMining({ name: this.currentSelectedData.libName }, nmCallTreeFuncData.item);
            nmCallTreeFuncArgs.push({
              funcName: 'splitTree',
              funcArgs: [this.currentSelectedData.libName, false, false],
            });
          } else {
            return;
          }
        } else if (nmCallTreeFuncData.item == 'restore') {
          if (nmCallTreeFuncData.remove != undefined && nmCallTreeFuncData.remove.length > 0) {
            let list = nmCallTreeFuncData.remove.map((item: any) => {
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
            list.forEach((symbolName: string) => {
              nmCallTreeFuncArgs.push({
                funcName: 'clearSplitMapData',
                funcArgs: [symbolName],
              });
            });
          }
        }
      }
      this.getDataByWorker(nmCallTreeFuncArgs, (result: any[]) => {
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
    this.nmCallTreeFilter!.getCallTreeData((callTreeData: any) => {
      if (callTreeData.value == 0) {
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
        this.getDataByWorker(resultArgs, (result: any[]) => {
          this.setLTableData(result);
          this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
          if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
        });
      }
    });
    this.nmCallTreeFilter!.getCallTreeConstraintsData((nmCallTreeConstraintsData: any) => {
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
      this.getDataByWorker(nmCallTreeConstraintsArgs, (result: any[]) => {
        this.setLTableData(result);
        this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
        if (this.isChartShow) this.nmCallTreeFrameChart?.calculateChartData();
      });
    });
    this.nmCallTreeFilter!.getFilterData((nmCallTreeData: FilterData) => {
      if (this.currentSelection!.nativeMemoryStatistic.length > 0) {
        this.filterResponseSelect = '';
      }
      if (
        this.filterAllocationType != nmCallTreeData.firstSelect ||
        this.filterNativeType != nmCallTreeData.secondSelect ||
        this.filterResponseSelect != nmCallTreeData.thirdSelect
      ) {
        this.filterAllocationType = nmCallTreeData.firstSelect || '0';
        this.filterNativeType = nmCallTreeData.secondSelect || '0';
        this.filterResponseSelect = nmCallTreeData.thirdSelect || "0'";
        let thirdIndex = parseInt(nmCallTreeData.thirdSelect || '0');
        if (this.responseTypes.length > thirdIndex) {
          this.filterResponseType = this.responseTypes[thirdIndex].key || -1;
        }
        this.refreshAllNode(this.nmCallTreeFilter!.getFilterTreeData());
      } else if (this.searchValue != this.nmCallTreeFilter!.filterValue) {
        this.searchValue = this.nmCallTreeFilter!.filterValue;
        let nmArgs = [
          {
            funcName: 'setSearchValue',
            funcArgs: [this.searchValue],
          },
          {
            funcName: 'resetAllNode',
            funcArgs: [],
          },
        ];
        this.getDataByWorker(nmArgs, (result: any[]) => {
          this.setLTableData(result);
          this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
          this.switchFlameChart(nmCallTreeData);
        });
      } else {
        this.switchFlameChart(nmCallTreeData);
      }
    });
    this.nmCallTreeTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortKey = evt.detail.key;
      // @ts-ignore
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.setLTableData(this.nmCallTreeSource, true);
      this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
    });
  }

  connectedCallback() {
    super.connectedCallback();
    let filterHeight = 0;
    new ResizeObserver((entries) => {
      let nmCallTreeTabFilter = this.shadowRoot!.querySelector('#nm-call-tree-filter') as HTMLElement;
      if (nmCallTreeTabFilter.clientHeight > 0) filterHeight = nmCallTreeTabFilter.clientHeight;
      if (this.parentElement!.clientHeight > filterHeight) {
        nmCallTreeTabFilter.style.display = 'flex';
      } else {
        nmCallTreeTabFilter.style.display = 'none';
      }
      this.modal!.style.height = this.nmCallTreeTbl!.clientHeight - 2 + 'px'; //2 is borderWidth
      if (this.nmCallTreeTbl!.style.visibility == 'hidden') {
        nmCallTreeTabFilter.style.display = 'none';
      }
      if (this.parentElement?.clientHeight != 0) {
        if (this.isChartShow) {
          this.nmCallTreeFrameChart?.updateCanvas(false, entries[0].contentRect.width);
          this.nmCallTreeFrameChart?.calculateChartData();
        }
        // @ts-ignore
        this.nmCallTreeTbl?.shadowRoot.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 10 - 35 + 'px';
        this.nmCallTreeTbl?.reMeauseHeight();
        // @ts-ignore
        this.filesystemTbr?.shadowRoot.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 45 - 21 + 'px';
        this.filesystemTbr?.reMeauseHeight();
        this.nmCallTreeLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
    this.parentElement!.onscroll = () => {
      this.nmCallTreeFrameChart!.tabPaneScrollTop = this.parentElement!.scrollTop;
    };
  }

  switchFlameChart(flameChartData?: any) {
    let nmCallTreePageTab = this.shadowRoot?.querySelector('#show_table');
    let nmCallTreePageChart = this.shadowRoot?.querySelector('#show_chart');
    if (!flameChartData || flameChartData.icon == 'block') {
      nmCallTreePageChart?.setAttribute('class', 'show');
      nmCallTreePageTab?.setAttribute('class', '');
      this.isChartShow = true;
      this.nmCallTreeFilter!.disabledMining = true;
      this.showBottomMenu(this.needShowMenu);
      this.nmCallTreeFrameChart!.data = this.nmCallTreeSource;
      this.nmCallTreeFrameChart?.calculateChartData();
    } else if (flameChartData.icon == 'tree') {
      nmCallTreePageChart?.setAttribute('class', '');
      nmCallTreePageTab?.setAttribute('class', 'show');
      this.showBottomMenu(true);
      this.isChartShow = false;
      this.nmCallTreeFilter!.disabledMining = false;
      this.nmCallTreeFrameChart!.clearCanvas();
      this.nmCallTreeTbl!.reMeauseHeight();
    }
  }

  refreshAllNode(filterData: any) {
    let nmCallTreeArgs: any[] = [];
    let isTopDown: boolean = !filterData.callTree[0];
    let isHideSystemLibrary = filterData.callTree[1];
    let list = filterData.dataMining.concat(filterData.dataLibrary);
    let groupArgs = new Map<string, any>();
    groupArgs.set('filterAllocType', this.filterAllocationType);
    groupArgs.set('filterEventType', this.filterNativeType);
    groupArgs.set('filterResponseType', this.filterResponseType);
    groupArgs.set('leftNs', this.currentSelection?.leftNs || 0);
    groupArgs.set('rightNs', this.currentSelection?.rightNs || 0);
    let selections: Array<any> = [];
    if (this.subTypeArr.length > 0) {
      this.subTypeArr.map((memory) => {
        selections.push({
          memoryTap: memory,
        });
      });
    }
    groupArgs.set('statisticsSelection', selections);
    groupArgs.set(
      'nativeHookType',
      this.currentSelection!.nativeMemory.length > 0 ? 'native-hook' : 'native-hook-statistic'
    );
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

  setLTableData(resultData: any[], sort?: boolean) {
    if (sort) {
      this.nmCallTreeSource = this.sortTree(resultData);
    } else {
      if (resultData && resultData[0]) {
        this.nmCallTreeSource =
          this.currentSelection!.nativeMemory.length > 0
            ? this.sortTree(resultData)
            : this.sortTree(resultData[0].children || []);
      } else {
        this.nmCallTreeSource = [];
      }
    }
    this.nmCallTreeTbl!.recycleDataSource = this.nmCallTreeSource;
  }

  sortTree(arr: Array<any>): Array<any> {
    let nmCallTreeSortArr = arr.sort((callTreeLeftData, callTreeRightData) => {
      if (this.sortKey == 'heapSizeStr' || this.sortKey == 'heapPercent') {
        if (this.sortType == 0) {
          return callTreeRightData.size - callTreeLeftData.size;
        } else if (this.sortType == 1) {
          return callTreeLeftData.size - callTreeRightData.size;
        } else {
          return callTreeRightData.size - callTreeLeftData.size;
        }
      } else {
        if (this.sortType == 0) {
          return callTreeRightData.count - callTreeLeftData.count;
        } else if (this.sortType == 1) {
          return callTreeLeftData.count - callTreeRightData.count;
        } else {
          return callTreeRightData.count - callTreeLeftData.count;
        }
      }
    });
    nmCallTreeSortArr.map((call) => {
      call.children = this.sortTree(call.children);
    });
    return nmCallTreeSortArr;
  }

  getDataByWorker(args: any[], handler: Function) {
    this.loadingList.push(1);
    this.nmCallTreeProgressEL!.loading = true;
    this.nmCallTreeLoadingPage.style.visibility = 'visible';
    procedurePool.submitWithName(
      'logic1',
      'native-memory-calltree-action',
      args,
      undefined,
      (callTreeActionResults: any) => {
        handler(callTreeActionResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length == 0) {
          this.nmCallTreeProgressEL!.loading = false;
          this.nmCallTreeLoadingPage.style.visibility = 'hidden';
        }
      }
    );
  }

  getDataByWorkerQuery(args: any, handler: Function) {
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
      (callChainsResults: any) => {
        handler(callChainsResults);
        this.loadingList.splice(0, 1);
        if (this.loadingList.length == 0) {
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
    <div class="nm-call-tree-content" style="display: flex;flex-direction: row">
    
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <tab-native-data-modal id="modal"></tab-native-data-modal>
            <lit-table id="tb-filesystem-calltree" style="height: auto" tree>
                <lit-table-column class="nm-call-tree-column" width="60%" title="Symbol Name" data-index="symbolName" key="symbolName"  align="flex-start">
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
     <tab-pane-filter id="nm-call-tree-filter" first second icon ></tab-pane-filter>
     <lit-progress-bar class="progress nm-call-tree-progress"></lit-progress-bar>
    <selector class="nm-call-tree-selector" id='show_chart'>
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>  
    <div class="loading nm-call-tree-loading"></div>
    </div>`;
  }
}
