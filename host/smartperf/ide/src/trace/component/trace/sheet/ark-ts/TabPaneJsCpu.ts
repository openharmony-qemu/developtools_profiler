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

import { BaseElement } from '../../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { JsCpuProfilerChartFrame, JsCpuProfilerTabStruct } from '../../../../bean/JsStruct.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { FilterData, TabPaneFilter } from '../TabPaneFilter.js';
import '../TabPaneFilter.js';

export class TabPaneJsCpuCallTree extends BaseElement {
  protected TYPE_TOP_DOWN = 0;
  protected TYPE_BOTTOM_UP = 1;
  private treeTable: HTMLDivElement | undefined | null;
  private callTreeTable: LitTable | null | undefined;
  private stackTable: LitTable | null | undefined;
  private sortKey = '';
  private sortType = 0;
  private callTreeSource: Array<JsCpuProfilerTabStruct> = [];
  private currentType = 0;
  private profilerFilter: TabPaneFilter | undefined | null;
  private searchValue: string = '';
  private totalNs: number = 0;
  private getDataByWorker(args: Array<JsCpuProfilerChartFrame>, handler: Function): void {
    const key = this.currentType === this.TYPE_TOP_DOWN ? 'jsCpuProfiler-call-tree' : 'jsCpuProfiler-bottom-up';
    procedurePool.submitWithName('logic1', key, args, undefined, (results: Array<JsCpuProfilerTabStruct>) => {
      handler(results);
    });
  }

  set data(data: SelectionParam | Array<JsCpuProfilerChartFrame>) {
    if (data instanceof SelectionParam) {
      let chartData = [];

      chartData = data.jsCpuProfilerData;
      this.totalNs = chartData.reduce((acc, struct) => acc + struct.totalTime, 0);
      if (data.rightNs && data.leftNs) {
        this.totalNs = Math.min(data.rightNs - data.leftNs, this.totalNs);
      }

      this.init();
      this.getDataByWorker(chartData, (results: Array<JsCpuProfilerTabStruct>) => {
        this.setCallTreeTableData(results);
      });
    }
  }

  protected setCurrentType(type: number) {
    this.currentType = type;
  }

  private init() {
    this.sortKey = '';
    this.sortType = 0;
    this.profilerFilter!.filterValue = '';
    const thTable = this.treeTable!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (this.treeTable!.hasAttribute('sort')) {
      this.treeTable!.removeAttribute('sort');
      list.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  private setCallTreeTableData(results: Array<JsCpuProfilerTabStruct>) {
    this.clearTab();
    const callTreeMap = new Map<number, JsCpuProfilerTabStruct>();
    const setTabData = (data: Array<JsCpuProfilerTabStruct>) => {
      data.forEach((item) => {
        if (item.children && item.children.length > 0) {
          item.children.forEach((it) => {
            it.parentId = item.id;
          });
        }
        callTreeMap.set(item.id, item);
        if (item.scriptName === 'unknown') {
          item.symbolName = item.name;
        } else {
          item.symbolName = item.name + ` ${item.scriptName}`;
        }
        item.totalTimePercent = ((item.totalTime / this.totalNs) * 100).toFixed(1) + '%';
        item.selfTimePercent = ((item.selfTime / this.totalNs) * 100).toFixed(1) + '%';
        item.selfTimeStr = ns2s(item.selfTime);
        item.totalTimeStr = ns2s(item.totalTime);
        item.parent = callTreeMap.get(item.parentId!);
        setTabData(item.children);
      });
    };
    setTabData(results);
    this.callTreeSource = this.sortTree(results);
    this.callTreeTable!.recycleDataSource = this.callTreeSource;
  }

  public initElements(): void {
    this.callTreeTable = this.shadowRoot?.querySelector('#callTreeTable') as LitTable;
    this.stackTable = this.shadowRoot?.querySelector('#stackTable') as LitTable;
    this.treeTable = this.callTreeTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.profilerFilter = this.shadowRoot?.querySelector('#filter') as TabPaneFilter;
    this.callTreeTable!.addEventListener('row-click', (evt) => {
      const heaviestStack = new Array<JsCpuProfilerTabStruct>();

      const getHeaviestChildren = (children: Array<JsCpuProfilerTabStruct>) => {
        if (children.length === 0) {
          return;
        }
        const heaviestChild = children.reduce((max, struct) =>
          Math.max(max.totalTime, struct.totalTime) === max.totalTime ? max : struct
        );
        heaviestStack?.push(heaviestChild);
        getHeaviestChildren(heaviestChild.children);
      };

      const getParent = (list: JsCpuProfilerTabStruct) => {
        if (list.parent) {
          heaviestStack.push(list.parent!);
          getParent(list.parent!);
        }
      };

      //@ts-ignore
      const data = evt.detail.data as JsCpuProfilerTabStruct;
      heaviestStack!.push(data);
      if (data.parent) {
        heaviestStack.push(data.parent!);
        getParent(data.parent!);
      }
      heaviestStack.reverse();
      getHeaviestChildren(data.children);
      this.stackTable!.recycleDataSource = heaviestStack;
      data.isSelected = true;
      this.stackTable?.clearAllSelection(data);
      this.stackTable?.setCurrentSelection(data);
      // @ts-ignore
      if (evt.detail.callBack) {
        // @ts-ignore
        evt.detail.callBack(true);
      }
    });

    this.stackTable!.addEventListener('row-click', (evt) => {
      //@ts-ignore
      const data = evt.detail.data as JsCpuProfilerTabStruct;
      data.isSelected = true;
      this.callTreeTable!.clearAllSelection(data);
      this.callTreeTable!.scrollToData(data);
      // @ts-ignore
      if (evt.detail.callBack) {
        // @ts-ignore
        evt.detail.callBack(true);
      }
    });
    this.callTreeTable!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortKey = evt.detail.key;
      // @ts-ignore
      this.sortType = evt.detail.sort;
      this.setCallTreeTableData(this.callTreeSource);
    });
    this.profilerFilter!.getFilterData((data: FilterData) => {
      if (this.searchValue != this.profilerFilter!.filterValue) {
        this.searchValue = this.profilerFilter!.filterValue;
        this.findSearchNode(this.callTreeSource, this.searchValue);
        this.setCallTreeTableData(this.callTreeSource);
      }
    });
  }
  private findSearchNode(sampleArray: JsCpuProfilerTabStruct[], search: string) {
    search = search.toLocaleLowerCase();
    sampleArray.forEach((sample) => {
      if (sample.symbolName && sample.symbolName.toLocaleLowerCase().includes(search)) {
        sample.isSearch = sample.symbolName != undefined && sample.symbolName.toLocaleLowerCase().includes(search);
      } else {
        sample.isSearch = false;
      }
      if (search === '') {
        sample.isSearch = false;
      }
      if (sample.children.length > 0) {
        this.findSearchNode(sample.children, search);
      }
    });
  }

  public connectedCallback(): void {
    super.connectedCallback();
    new ResizeObserver(() => {
      // @ts-ignore
      this.callTreeTable?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 32 + 'px';
      this.callTreeTable?.reMeauseHeight();
      // @ts-ignore
      this.stackTable?.shadowRoot.querySelector('.table').style.height =
        this.parentElement!.clientHeight - 32 - 22 + 'px';
      this.stackTable?.reMeauseHeight();
    }).observe(this.parentElement!);
  }

  private sortTree(arr: Array<JsCpuProfilerTabStruct>): Array<JsCpuProfilerTabStruct> {
    const that = this;
    function defaultSort(callTreeLeftData: JsCpuProfilerTabStruct, callTreeRightData: JsCpuProfilerTabStruct) {
      if (that.currentType === that.TYPE_TOP_DOWN) {
        return callTreeRightData.totalTime - callTreeLeftData.totalTime;
      } else {
        return callTreeRightData.selfTime - callTreeLeftData.selfTime;
      }
    }
    const CallTreeSortArr = arr.sort((callTreeLeftData, callTreeRightData) => {
      if (this.sortKey === 'selfTimeStr' || this.sortKey === 'selfTimePercent') {
        if (this.sortType == 0) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType == 1) {
          return callTreeLeftData.selfTime - callTreeRightData.selfTime;
        } else {
          return callTreeRightData.selfTime - callTreeLeftData.selfTime;
        }
      } else if (this.sortKey === 'symbolName') {
        if (this.sortType == 0) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType == 1) {
          return (callTreeLeftData.symbolName + '').localeCompare(callTreeRightData.symbolName + '');
        } else {
          return (callTreeRightData.symbolName + '').localeCompare(callTreeLeftData.symbolName + '');
        }
      } else {
        if (this.sortType == 0) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType == 1) {
          return callTreeLeftData.totalTime - callTreeRightData.totalTime;
        } else {
          return callTreeRightData.totalTime - callTreeLeftData.totalTime;
        }
      }
    });

    CallTreeSortArr.map((call) => {
      call.children = this.sortTree(call.children);
    });
    return CallTreeSortArr;
  }

  private clearTab(): void {
    this.stackTable!.recycleDataSource = [];
    this.callTreeTable!.recycleDataSource = [];
  }

  public initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 0px 10px 0 10px;
        }
        .show{
            display: flex;
            flex: 1;
        }
        .progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
    </style>
    <div class="perf-profile-content">
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <lit-table id="callTreeTable" style="height: 100%" tree>
                <lit-table-column width="60%" title="Symbol" data-index="" key="symbolName"  align="flex-start" order></lit-table-column>
                <lit-table-column width="1fr" title="SelfTime" data-index="selfTimeStr" key="selfTimeStr" align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="%" data-index="selfTimePercent" key="selfTimePercent"  align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="TotalTime" data-index="totalTimeStr" key="totalTimeStr"  align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="%" data-index="totalTimePercent" key="totalTimePercent"  align="flex-start"  order></lit-table-column>
            </lit-table>
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <div class="right" style="flex: 1;display: flex; flex-direction: row;">
            <div style="flex: 1;display: block;">
              <span slot="head" style="height: 22px">Heaviest Stack</span>
              <lit-table id="stackTable" style="height: auto;">
                  <lit-table-column width="50%" title="Symbol" data-index="" key="symbolName"  align="flex-start"></lit-table-column>
                  <lit-table-column width="1fr" title="TotalTime" data-index="totalTimeStr" key="totalTimeStr"  align="flex-start" ></lit-table-column>
                  <lit-table-column width="1fr" title="%" data-index="totalTimePercent" key="totalTimePercent"  align="flex-start"></lit-table-column>
              </lit-table>
          </div>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="filter" input inputLeftText ></tab-pane-filter>
     <lit-progress-bar class="progress perf-profile-progress"></lit-progress-bar>
    </div>
        `;
  }
}
