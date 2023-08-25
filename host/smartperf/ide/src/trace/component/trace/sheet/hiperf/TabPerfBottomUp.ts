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
import '../../../chart/FrameChart.js';
import '../../../DisassemblingWindow.js';
import '../../../../../base-ui/slicer/lit-slicer.js';
import '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { PerfBottomUpStruct } from '../../../../bean/PerfBottomUpStruct.js';

@element('tabpane-perf-bottom-up')
export class TabpanePerfBottomUp extends BaseElement {
  private treeTable: HTMLDivElement | undefined | null;
  private bottomUpTable: LitTable | null | undefined;
  private stackTable: LitTable | null | undefined;
  private sortKey = '';
  private sortType = 0;
  private bottomUpSource: Array<PerfBottomUpStruct> = [];
  private bottomUpFilter: TabPaneFilter | undefined | null;
  private progressEL: LitProgressBar | null | undefined;
  private searchValue: string = '';
  public initElements(): void {
    this.bottomUpTable = this.shadowRoot?.querySelector('#callTreeTable') as LitTable;
    this.stackTable = this.shadowRoot?.querySelector('#stackTable') as LitTable;
    this.treeTable = this.bottomUpTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.bottomUpFilter = this.shadowRoot?.querySelector('#filter') as TabPaneFilter;
    this.bottomUpTable!.addEventListener('row-click', (evt) => this.bottomUpTableRowClickHandler(evt));
    this.stackTable!.addEventListener('row-click', (evt) => this.stackTableRowClick(evt));
    this.bottomUpTable!.addEventListener('column-click', (evt) => this.bottomUpTableColumnClickHandler(evt));
    this.bottomUpFilter!.getFilterData((data: FilterData) => {
      if (this.searchValue !== this.bottomUpFilter!.filterValue) {
        this.searchValue = this.bottomUpFilter!.filterValue;
        this.findSearchNode(this.bottomUpSource, this.searchValue);
        this.setBottomUpTableData(this.bottomUpSource);
      }
    });
  }

  private getDataByWorker(val: SelectionParam, handler: (results: Array<PerfBottomUpStruct>) => void): void {
    this.progressEL!.loading = true;
    const args = [
      {
        funcName: 'setPerfBottomUp',
        funcArgs: [''],
      },
      {
        funcName: 'setSearchValue',
        funcArgs: [''],
      },
      {
        funcName: 'getCurrentDataFromDb',
        funcArgs: [val],
      },
    ];
    procedurePool.submitWithName('logic0', 'perf-action', args, undefined, (results: Array<PerfBottomUpStruct>) => {
      handler(results);
      this.progressEL!.loading = false;
    });
  }

  set data(data: SelectionParam) {
    if (data instanceof SelectionParam) {
      this.sortKey = '';
      this.sortType = 0;
      this.bottomUpFilter!.filterValue = '';
      this.getDataByWorker(data, (results: Array<PerfBottomUpStruct>) => {
        this.setBottomUpTableData(results);
      });
    }
  }

  private setBottomUpTableData(results: Array<PerfBottomUpStruct>): void {
    const percentageDenominator = 100;
    const percentFraction = 1;
    const timeFractionDigits = 2;
    this.stackTable!.recycleDataSource = [];
    this.bottomUpTable!.recycleDataSource = [];
    let totalTime = results.reduce((sum, struct) => sum + struct.selfTime, 0);
    const setTabData = (array: Array<PerfBottomUpStruct>): void => {
      array.forEach((data) => {
        data.totalTimePercent = `${ ((data.totalTime / totalTime) * percentageDenominator).toFixed(percentFraction) }%`;
        data.selfTimePercent = `${ ((data.selfTime / totalTime) * percentageDenominator).toFixed(percentFraction) }%`;
        data.selfTimeStr = `${ data.selfTime.toFixed(timeFractionDigits) }ms`;
        data.totalTimeStr = `${ data.totalTime.toFixed(timeFractionDigits) }ms`;
        setTabData(data.children);
      });
    };
    setTabData(results);
    this.bottomUpSource = this.sortTree(results);
    this.bottomUpTable!.recycleDataSource = this.bottomUpSource;
  }

  private bottomUpTableRowClickHandler(evt: Event): void {
    const callStack: Array<PerfBottomUpStruct> = [];
    const getCallStackChildren = (children: Array<PerfBottomUpStruct>): void => {
      if (children.length === 0) {
        return;
      }
      const heaviestChild = children.reduce((max, struct) =>
        Math.max(max.totalTime, struct.totalTime) === max.totalTime ? max : struct
      );
      callStack?.push(heaviestChild);
      getCallStackChildren(heaviestChild.children);
    };
    const getParent = (list: PerfBottomUpStruct): void => {
      if (list.parentNode && list.parentNode!.symbolName !== 'root') {
        callStack.push(list.parentNode!);
        getParent(list.parentNode!);
      }
    };

    //@ts-ignore
    const bottomUpData = evt.detail.data as PerfBottomUpStruct;
    callStack!.push(bottomUpData);
    if (bottomUpData.parentNode && bottomUpData.parentNode!.symbolName !== 'root') {
      callStack.push(bottomUpData.parentNode!);
      getParent(bottomUpData.parentNode!);
    }
    callStack.reverse();
    getCallStackChildren(bottomUpData.children);
    this.stackTable!.recycleDataSource = callStack;
    bottomUpData.isSelected = true;
    this.stackTable?.clearAllSelection(bottomUpData);
    this.stackTable?.setCurrentSelection(bottomUpData);
    // @ts-ignore
    if (evt.detail.callBack) {
      // @ts-ignore
      evt.detail.callBack(true);
    }
  }

  private bottomUpTableColumnClickHandler(evt: Event): void {
    // @ts-ignore
    this.sortKey = evt.detail.key;
    // @ts-ignore
    this.sortType = evt.detail.sort;
    this.setBottomUpTableData(this.bottomUpSource);
  }
  private stackTableRowClick(evt: Event): void {
    //@ts-ignore
    const data = evt.detail.data as PerfBottomUpStruct;
    data.isSelected = true;
    this.bottomUpTable!.clearAllSelection(data);
    this.bottomUpTable!.scrollToData(data);
    // @ts-ignore
    if (evt.detail.callBack) {
      // @ts-ignore
      evt.detail.callBack(true);
    }
  }
  private findSearchNode(sampleArray: PerfBottomUpStruct[], search: string): void {
    search = search.toLocaleLowerCase();
    sampleArray.forEach((sample) => {
      if (sample.symbolName && sample.symbolName.toLocaleLowerCase().includes(search)) {
        sample.isSearch = sample.symbolName !== undefined && sample.symbolName.toLocaleLowerCase().includes(search);
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
    const tableOffsetHeight = 32;
    const spanHeight = 22;
    super.connectedCallback();
    new ResizeObserver(() => {
      // @ts-ignore
      this.bottomUpTable?.shadowRoot.querySelector('.table').style.height = `${
        this.parentElement!.clientHeight - tableOffsetHeight
      }px`;
      this.bottomUpTable?.reMeauseHeight();
      // @ts-ignore
      this.stackTable?.shadowRoot.querySelector('.table').style.height = `${
        this.parentElement!.clientHeight - tableOffsetHeight - spanHeight
      }px`;
      this.stackTable?.reMeauseHeight();
    }).observe(this.parentElement!);
  }

  private sortTree(arr: Array<PerfBottomUpStruct>): Array<PerfBottomUpStruct> {
    const defaultSortType = 0;
    function defaultSort(callTreeLeftData: PerfBottomUpStruct, callTreeRightData: PerfBottomUpStruct): number {
      return callTreeRightData.totalTime - callTreeLeftData.totalTime;
    }
    const CallTreeSortArr = arr.sort((callTreeLeftData, callTreeRightData) => {
      if (this.sortKey === 'selfTimeStr' || this.sortKey === 'selfTimePercent') {
        if (this.sortType === defaultSortType) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType === 1) {
          return callTreeLeftData.selfTime - callTreeRightData.selfTime;
        } else {
          return callTreeRightData.selfTime - callTreeLeftData.selfTime;
        }
      } else if (this.sortKey === 'symbolName') {
        if (this.sortType === defaultSortType) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType === 1) {
          return `${ callTreeLeftData.symbolName }`.localeCompare(`${ callTreeRightData.symbolName }`);
        } else {
          return `${ callTreeRightData.symbolName }`.localeCompare(`${ callTreeLeftData.symbolName }`);
        }
      } else {
        if (this.sortType === defaultSortType) {
          return defaultSort(callTreeLeftData, callTreeRightData);
        } else if (this.sortType === 1) {
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

  private initHtmlStyle(): string {
    return `
     <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 0 10px 0 10px;
        }
        .show-bottom-up{
            display: flex;
            flex: 1;
        }
        .perf-bottom-up-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
    </style>
    `;
  }
  public initHtml(): string {
    return `
    ${ this.initHtmlStyle() }
    <div class="perf-bottom-up-content">
    <selector id='show_table' class="show-bottom-up">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <lit-table id="callTreeTable" style="height: 100%" tree>
                <lit-table-column width="60%" title="Symbol" data-index="symbolName" key="symbolName"  
                align="flex-start" order></lit-table-column>
                <lit-table-column width="1fr" title="Local" data-index="selfTimeStr" key="selfTimeStr" 
                align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="%" data-index="selfTimePercent" key="selfTimePercent"  
                align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="Weight" data-index="totalTimeStr" key="totalTimeStr"  
                align="flex-start"  order></lit-table-column>
                <lit-table-column width="1fr" title="%" data-index="totalTimePercent" key="totalTimePercent" 
                 align="flex-start"  order></lit-table-column>
            </lit-table>
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <div class="right" style="flex: 1;display: flex; flex-direction: row;">
            <div style="flex: 1;display: block;">
              <span slot="head" style="height: 22px">Call Stack</span>
              <lit-table id="stackTable" style="height: auto;">
                  <lit-table-column width="50%" title="Symbol" data-index="symbolName" key="symbolName" 
                   align="flex-start"></lit-table-column>
                  <lit-table-column width="1fr" title="Weight" data-index="totalTimeStr" key="totalTimeStr" 
                   align="flex-start" ></lit-table-column>
                  <lit-table-column width="1fr" title="%" data-index="totalTimePercent" key="totalTimePercent"
                    align="flex-start"></lit-table-column>
              </lit-table>
          </div>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="filter" input inputLeftText ></tab-pane-filter>
     <lit-progress-bar class="progress perf-bottom-up-progress"></lit-progress-bar>
    </div>
        `;
  }
}
