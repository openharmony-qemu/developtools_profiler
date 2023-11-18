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
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { TraceRow } from '../../base/TraceRow.js';
import { TraceSheet } from '../../base/TraceSheet.js';
import { Flag } from '../../timer-shaft/Flag.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { ns2Timestamp, ns2x, Rect } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { LogStruct } from '../../../../database/ui-worker/ProcedureWorkerLog.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { LitPageTable } from '../../../../../base-ui/table/LitPageTable.js';

@element('tab-hi-log')
export class TabPaneHiLogs extends BaseElement {
  tableTimeHandle: (() => void) | undefined;
  tableTitleTimeHandle: (() => void) | undefined;
  private systemLogSource: LogStruct[] = [];
  private spSystemTrace: SpSystemTrace | undefined | null;
  private traceSheetEl: TraceSheet | undefined | null;
  private levelFilterInput: HTMLSelectElement | undefined | null;
  private tagFilterInput: HTMLInputElement | undefined | null;
  private searchFilterInput: HTMLInputElement | undefined | null;
  private processFilter: HTMLInputElement | undefined | null;
  private logTableTitle: HTMLDivElement | undefined | null;
  private tagFilterDiv: HTMLDivElement | undefined | null;
  private hiLogsTbl: LitPageTable | undefined | null;
  private filterData: LogStruct[] = [];
  private optionLevel: string[] = ['Debug', 'Info', 'Warn', 'Error', 'Fatal'];
  private allowTag: Set<string> = new Set();

  set data(systemLogParam: SelectionParam) {
    if (this.hiLogsTbl) {
      this.hiLogsTbl.recycleDataSource = [];
    }
    this.systemLogSource = systemLogParam.hiLogs;
    if (systemLogParam && systemLogParam.hiLogs.length > 0) {
      this.tableTimeHandle?.();
    }
  }

  initElements(): void {
    this.levelFilterInput = this.shadowRoot?.querySelector<HTMLSelectElement>('#level-filter');
    this.logTableTitle = this.shadowRoot?.querySelector<HTMLDivElement>('#log-title');
    this.tagFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#tag-filter');
    this.searchFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#search-filter');
    this.processFilter = this.shadowRoot?.querySelector<HTMLInputElement>('#process-filter');
    this.spSystemTrace = document
      .querySelector('body > sp-application')
      ?.shadowRoot?.querySelector<SpSystemTrace>('#sp-system-trace');
    this.tableTimeHandle = this.delayedRefresh(this.refreshTable);
    this.tableTitleTimeHandle = this.delayedRefresh(this.refreshLogsTitle);
    this.tagFilterDiv = this.shadowRoot!.querySelector<HTMLDivElement>('#tagFilter');
    this.hiLogsTbl = this.shadowRoot!.querySelector<LitPageTable>('#tb-hilogs');
    this.hiLogsTbl!.getItemTextColor = (data)=>{
      return ColorUtils.getHilogColor(data.level);
    }
    this.hiLogsTbl!.itemTextHandleMap.set('startTs', (startTs) => {
      return ns2Timestamp(startTs);
    });
    this.hiLogsTbl!.addEventListener('row-hover', (e): void=>{
      // @ts-ignore
      let data = e.detail.data;
      if (data) {
        let pointX: number = ns2x(
          data.startTs || 0,
          TraceRow.range!.startNS,
          TraceRow.range!.endNS,
          TraceRow.range!.totalNS,
          new Rect(0, 0, TraceRow.FRAME_WIDTH, 0)
        );
        this.traceSheetEl!.systemLogFlag = new Flag(Math.floor(pointX), 0, 0, 0, data.startTs!, '#999999', true, '');
        this.spSystemTrace?.refreshCanvas(false);
      }
    });
    let tbl = this.hiLogsTbl?.shadowRoot?.querySelector<HTMLDivElement>('.table');
    tbl!.addEventListener('scroll', ()=>{
      this.tableTitleTimeHandle?.();
    });
    this.tagFilterDiv!.onclick = (ev): void => {
      // @ts-ignore
      let parentNode = ev.target.parentNode;
      if (parentNode && this.tagFilterDiv!.contains(parentNode)) {
        this.tagFilterDiv!.removeChild(parentNode);
        this.allowTag['delete'](parentNode.textContent.trim().toLowerCase());
      }
      this.tableTimeHandle?.();
    };
    this.searchFilterInput!.oninput = (): void => {
      this.tableTimeHandle?.();
    };
    this.processFilter!.oninput = (): void => {
      this.tableTimeHandle?.();
    };
    this.levelFilterInput!.onchange = (): void => {
      this.tableTimeHandle?.();
    };
  }

  connectedCallback(): void {
    super.connectedCallback();
    this.tagFilterInput?.addEventListener('keyup', this.tagFilterKeyEvent);
    new ResizeObserver((): void => {
      this.parentElement!.style.overflow = 'hidden';
      // @ts-ignore
      this.hiLogsTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 45 + 'px';
      this.tableTimeHandle?.();
      this.tableTitleTimeHandle?.();
    }).observe(this.parentElement!);
  }

  disconnectedCallback(): void {
    super.disconnectedCallback();
    this.tagFilterInput?.removeEventListener('keyup', this.tagFilterKeyEvent);
  }

  initHtml(): string {
    return `${this.initTitleCssStyle()}
        <div class="logs-title-content">
          <label id="log-title">Hilogs [0, 0] / 0</label>
          <div style="display: flex;flex-wrap: wrap;">
            <div class="level-content">
            <select id="level-filter">
              <option>Debug</option>
              <option>Info</option>
              <option>Warn</option>
              <option>Error</option>
              <option>Fatal</option>
            </select>
          </div>
            <div style="display: flex;">
              <div id="tagFilter" style='display: flex;width: auto; height: 100%;flex-wrap: wrap;'></div>
                 <input type="text" id="tag-filter" class="filter-input" placeholder="Filter by tag...">
              </div>
              <input type="text" id="process-filter" class="filter-input" placeholder="Search process name...">
              <input type="text" id="search-filter" class="filter-input" placeholder="Search message...">
            </div>
          </div>
        <lit-page-table id="tb-hilogs">
            <lit-table-column title="Timestamp" width="10%" data-index="startTs" key="startTs">
            </lit-table-column>
            <lit-table-column title="Time" width="10%" data-index="originTime" key="originTime">
            </lit-table-column>
            <lit-table-column title="Level" width="5%" data-index="level" key="level">
            </lit-table-column>
            <lit-table-column title="Tag" width="15%" data-index="tag" key="tag">
            </lit-table-column>
            <lit-table-column title="Process Name" width="12%" data-index="processName" key="processName">
            </lit-table-column>
            <lit-table-column title="Message" width="44%" data-index="context" key="context">
            </lit-table-column>
        </lit-page-table>
        `;
  }

  refreshLogsTitle(): void{
    let tbl = this.hiLogsTbl?.shadowRoot?.querySelector<HTMLDivElement>('.table');
    let height = 0;
    let firstRowHeight = 27;
    let tableHeadHeight = 26;
    if (tbl) {
      tbl.querySelectorAll<HTMLElement>('.tr').forEach((trEl: HTMLElement, index: number): void=>{
        if (index === 0) {
          let frontTotalRowSize = Math.round(tbl!.scrollTop / trEl.clientHeight * 100) / 100;
          if (frontTotalRowSize.toString().indexOf('.') >= 0) {
            let rowCount = frontTotalRowSize.toString().split('.');
            height += trEl.clientHeight - (Number(rowCount[1]) / 100 * trEl.clientHeight);
          }
        }
        let allTdEl = trEl.querySelectorAll<HTMLElement>('.td');
        allTdEl[0].style.color = '#3D88C7';
        allTdEl[0].style.textDecoration = 'underline';
        allTdEl[0].style.textDecorationColor = '#3D88C7';
        trEl.addEventListener('mouseout', (): void => {
          this.traceSheetEl!.systemLogFlag = undefined;
          this.spSystemTrace?.refreshCanvas(false);
        })
      });
    }
    if (this.hiLogsTbl && this.hiLogsTbl.currentRecycleList.length > 0) {
      let startDataIndex = this.hiLogsTbl.startSkip + 1;
      let endDataIndex = startDataIndex;
      if (height < firstRowHeight * 0.3) {
        startDataIndex++;
      }
      let tableHeight = Number(tbl!.style.height.replace('px', '')) - tableHeadHeight;
      while (height < tableHeight) {
        if (firstRowHeight <= 0 || height + firstRowHeight > tableHeight) {
          break;
        }
        height += firstRowHeight;
        endDataIndex++;
      }
      if (tableHeight - height > firstRowHeight * 0.3) {
        endDataIndex++;
      }
      if (endDataIndex >= this.filterData.length) {
        endDataIndex = this.filterData.length;
      } else {
        endDataIndex = this.hiLogsTbl.startSkip === 0 ? endDataIndex - 1 : endDataIndex;
      }
      this.logTableTitle!.textContent = `Hilogs [${this.hiLogsTbl.startSkip === 0 ? 1 : startDataIndex}, 
        ${endDataIndex}] / ${this.filterData.length || 0}`;
    } else {
      this.logTableTitle!.textContent = 'Hilogs [0, 0] / 0';
    }
  }

  initTabSheetEl(traceSheet: TraceSheet): void {
    this.traceSheetEl = traceSheet;
    this.levelFilterInput!.selectedIndex = 0;
    this.tagFilterInput!.value = '';
    this.tagFilterDiv!.innerHTML = '';
    this.allowTag.clear();
    this.processFilter!.value = '';
    this.searchFilterInput!.value = '';
  }

  tagFilterKeyEvent = (e: KeyboardEvent): void => {
    let inputValue = this.tagFilterInput!.value.trim();
    if (e.code === 'Enter') {
      if (inputValue !== '' && !this.allowTag.has(inputValue.toLowerCase())) {
        let tagElement = document.createElement('div');
        tagElement.className = 'tagElement';
        tagElement.id = inputValue;
        let tag = document.createElement('div');
        tag.className = 'tag';
        tag.innerHTML = inputValue;
        this.allowTag.add(inputValue.toLowerCase());
        let closeButton = document.createElement('lit-icon');
        closeButton.setAttribute('name', 'close-light');
        closeButton.style.color = '#FFFFFF';
        tagElement.append(tag);
        tagElement.append(closeButton);
        this.tagFilterDiv!.append(tagElement);
        this.tagFilterInput!.value = '';
        this.tagFilterInput!.placeholder = 'Filter by tag...';
      }
    } else if (e.code === 'Backspace') {
      let index = this.tagFilterDiv!.childNodes.length - defaultIndex;
      if (index >= 0 && inputValue === '') {
        let childNode = this.tagFilterDiv!.childNodes[index];
        this.tagFilterDiv!.removeChild(childNode);
        this.allowTag['delete'](childNode.textContent!.trim().toLowerCase());
      }
    }
    this.tableTimeHandle?.();
  };

  private updateFilterData(): void {
    if (this.systemLogSource?.length > 0) {
      this.filterData = this.systemLogSource.filter((data) => this.isFilterLog(data));
    }
    // @ts-ignore
    this.hiLogsTbl?.shadowRoot?.querySelector('.table').style.height = this.parentElement.clientHeight - 20 - 45 + 'px';
    if (this.filterData.length > 0) {
      this.hiLogsTbl!.recycleDataSource = this.filterData;
    } else {
      this.hiLogsTbl!.recycleDataSource = [];
    }
    this.refreshLogsTitle();
  }

  private isFilterLog(data: LogStruct): boolean {
    let level = this.levelFilterInput?.selectedIndex || 0;
    let search = this.searchFilterInput?.value.toLowerCase() || '';
    search = search.replace(/\s/g, '');
    let processSearch = this.processFilter?.value.toLowerCase() || '';
    processSearch = processSearch.replace(/\s/g, '');
    return (
      (data.startTs || 0) >= TraceRow.range!.startNS &&
      (data.startTs || 0) <= TraceRow.range!.endNS &&
      (level === 0 || this.optionLevel.indexOf(data.level!) >= level) &&
      (this.allowTag.size === 0 || this.allowTag.has(data.tag!.toLowerCase())) &&
      (search === '' || data.context!.toLowerCase().replace(/\s/g, '').indexOf(search) >= 0) &&
      (processSearch === '' ||
        (data.processName !== null && data.processName!.toLowerCase().replace(/\s/g, '').indexOf(processSearch) >= 0))
    );
  }

  private refreshTable(): void {
    if (this.traceSheetEl) {
      this.traceSheetEl.systemLogFlag = undefined;
      this.spSystemTrace?.refreshCanvas(false);
      this.updateFilterData();
    }
  }

  private delayedRefresh(optionFn: Function, dur: number = tableTimeOut): () => void {
    let timeOutId: number;
    return (...args: []): void => {
      window.clearTimeout(timeOutId);
      timeOutId = window.setTimeout((): void => {
        optionFn.apply(this, ...args);
      }, dur);
    };
  }

  private initTitleCssStyle(): string {
    return `<style>
        :host{
          padding: 10px 10px;
          display: flex;
          flex-direction: column;
        }
        .logs-title-content {
          display: flex;
          flex-wrap: wrap;
          width: 100%;
          align-items: center;
          justify-content: space-between;
          border-bottom: 1px solid #D5D5D5;
          padding: 10px 0px;
        }
        #log-title {
          flex-grow: 1;
        }
        .filter-input {
          line-height: 16px;
          margin-right: 20px;
          padding: 3px 12px;
          height: 16px;
        }
        .level-content {
          margin-right: 20px;
        }
        input {
          background: #FFFFFF;
          font-size: 14px;
          color: #212121;
          text-align: left;
          line-height: 16px;
          font-weight: 400;
          text-indent: 2%;
          border: 1px solid #979797;
          border-radius: 10px;
        }
        select {
          border: 1px solid rgba(0,0,0,0.60);
          border-radius: 10px;
        }
        option {
          font-weight: 400;
          font-size: 14px;
        }
        .tagElement {
          display: flex;
          background-color: #0A59F7;
          align-items: center;
          margin-right: 5px;
          border-radius: 10px;
          font-size: 14px;
          height: 22px;
          margin-bottom: 5px;
        }
        .tag {
          line-height: 14px;
          padding: 4px 8px;
          color: #FFFFFF;
        }
        #level-filter {
          padding: 1px 12px;
          opacity: 0.6;
          font-size: 14px;
          line-height: 20px;
          font-weight: 400;
        }
        </style>`;
  }
}

let defaultIndex: number = 1;
let tableTimeOut: number = 50;
