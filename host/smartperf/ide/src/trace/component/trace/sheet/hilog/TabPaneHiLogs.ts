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
import { LitTable } from '../../../../../base-ui/table/lit-table.js';

@element('tab-hi-log')
export class TabPaneHiLogs extends BaseElement {
  tableTimeHandle: (() => void) | undefined;
  private systemLogSource: LogStruct[] = [];
  private spSystemTrace: SpSystemTrace | undefined | null;
  private traceSheetEl: TraceSheet | undefined | null;
  private parentTabEl: HTMLElement | undefined | null;
  private levelFilterInput: HTMLSelectElement | undefined | null;
  private tagFilterInput: HTMLInputElement | undefined | null;
  private searchFilterInput: HTMLInputElement | undefined | null;
  private processFilter: HTMLInputElement | undefined | null;
  private logTitle: HTMLDivElement | undefined | null;
  private logTableTitle: HTMLDivElement | undefined | null;
  private logTable: HTMLDivElement | undefined | null;
  private scrollContainer: HTMLDivElement | undefined | null;
  private tagFilterDiv: HTMLDivElement | undefined | null;
  private hiLogDownLoadTbl: LitTable | undefined | null;
  private filterData: LogStruct[] = [];
  private visibleData: LogStruct[] = [];
  private optionLevel: string[] = ['D', 'I', 'W', 'E', 'F'];
  private tableColumnHead: string[] = ['Timestamp', 'Time', 'Level', 'Tag', 'Process name', 'Message'];
  private allowTag: Set<string> = new Set();
  private startDataIndex: number = 0;
  private endDataIndex: number = 0;
  private containerClientHeight: number = 0;

  set data(systemLogParam: SelectionParam) {
    if (this.hiLogDownLoadTbl) {
      this.hiLogDownLoadTbl.recycleDataSource = [];
    }
    this.systemLogSource = systemLogParam.hiLogs;
    if (systemLogParam && systemLogParam.hiLogs.length > 0) {
      this.tableTimeHandle?.();
    }
  }

  initElements(): void {
    this.logTitle = this.shadowRoot?.querySelector<HTMLDivElement>('.logs-title-content');
    this.logTableTitle = this.shadowRoot?.querySelector<HTMLDivElement>('#log-title');
    this.scrollContainer = this.shadowRoot?.querySelector<HTMLDivElement>('.tbl-logs');
    this.logTable = this.shadowRoot?.querySelector<HTMLDivElement>('#logs-data-content');
    this.levelFilterInput = this.shadowRoot?.querySelector<HTMLSelectElement>('#level-filter');
    this.tagFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#tag-filter');
    this.searchFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#search-filter');
    this.processFilter = this.shadowRoot?.querySelector<HTMLInputElement>('#process-filter');
    this.spSystemTrace = document
      .querySelector('body > sp-application')
      ?.shadowRoot?.querySelector<SpSystemTrace>('#sp-system-trace');
    this.tableTimeHandle = this.delayedRefresh(this.refreshTable);
    this.tagFilterDiv = this.shadowRoot!.querySelector<HTMLDivElement>('#tagFilter');
    this.hiLogDownLoadTbl = this.shadowRoot!.querySelector<LitTable>('#tb-hilogs');
  }

  connectedCallback(): void {
    super.connectedCallback();
    this.tagFilterInput?.addEventListener('input', this.tagFilterInputEvent);
    this.tagFilterInput?.addEventListener('keydown', this.tagFilterKeyEvent);
    this.tagFilterDiv?.addEventListener('click', this.tagFilterDivClickEvent);
    this.searchFilterInput?.addEventListener('input', this.searchFilterInputEvent);
    this.processFilter?.addEventListener('input', this.processFilterEvent);
    this.levelFilterInput?.addEventListener('change', this.levelFilterInputEvent);
    this.scrollContainer?.addEventListener('scroll', this.logTableScrollEvent);
    new ResizeObserver(() => {
      this.updateVisibleData();
    }).observe(this.parentElement!);
  }

  disconnectedCallback(): void {
    super.disconnectedCallback();
    this.tagFilterInput?.removeEventListener('input', this.tagFilterInputEvent);
    this.tagFilterInput?.removeEventListener('keydown', this.tagFilterKeyEvent);
    this.tagFilterDiv?.removeEventListener('click', this.tagFilterDivClickEvent);
    this.searchFilterInput?.removeEventListener('input', this.searchFilterInputEvent);
    this.processFilter?.removeEventListener('input', this.processFilterEvent);
    this.levelFilterInput?.removeEventListener('change', this.levelFilterInputEvent);
    this.scrollContainer?.removeEventListener('scroll', this.logTableScrollEvent);
  }

  initHtml(): string {
    return `${this.initTitleCssStyle()}
            ${this.initTableCssStyle()}
        <div class="logs-title-content">
          <label id="log-title">Hilogs [0, 0] / 0</label>
          <div style="display: flex;flex-wrap: wrap;">
            <div class="level-content">
            <label class="level-select-title">Log Level</label>
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
              <input type="text" id="search-filter" class="filter-input" placeholder="Search logs...">
            </div>
          </div>
          <div id="logs-head-content" style="display: flex"></div>
          <div class="tbl-logs">
              <div id="logs-data-content"></div>
            </div>
        <lit-table id="tb-hilogs" style="display: none;">
            <lit-table-column title="Timestamp" width="1fr" data-index="startTs" key="startTs">
            </lit-table-column>
            <lit-table-column title="Time" width="1fr" data-index="originTime" key="originTime">
            </lit-table-column>
            <lit-table-column title="Level" width="1fr" data-index="level" key="level">
            </lit-table-column>
            <lit-table-column title="Tag" width="1fr" data-index="tag" key="tag">
            </lit-table-column>
            <lit-table-column title="Process Name" width="1fr" data-index="processName" key="processName">
            </lit-table-column>
            <lit-table-column title="Message" width="1fr" data-index="context" key="context">
            </lit-table-column>
        </lit-table>
        `;
  }

  initTabSheetEl(parentTabEl: HTMLElement, traceSheet: TraceSheet): void {
    this.traceSheetEl = traceSheet;
    if (parentTabEl) {
      this.parentTabEl = parentTabEl;
      this.containerClientHeight = parentTabEl.clientHeight;
    }
    this.levelFilterInput!.selectedIndex = 0;
    this.tagFilterInput!.value = '';
    this.tagFilterInput!.placeholder = 'Filter by tag...';
    this.tagFilterDiv!.innerHTML = '';
    this.allowTag.clear();
    this.processFilter!.value = '';
    this.processFilter!.placeholder = 'Search process name...';
    this.searchFilterInput!.value = '';
    this.searchFilterInput!.placeholder = 'Search logs...';
  }

  tagFilterInputEvent = (): void => {
    if (this.tagFilterInput) {
      if (this.tagFilterInput.value === '') {
        this.tagFilterInput.placeholder = 'Filter by tag...';
      } else {
        this.tagFilterInput.placeholder = '';
      }
    }
  };

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

  tagFilterDivClickEvent = (ev: Event): void => {
    // @ts-ignore
    let parentNode = ev.target.parentNode;
    if (parentNode && this.tagFilterDiv!.contains(parentNode)) {
      this.tagFilterDiv!.removeChild(parentNode);
      this.allowTag['delete'](parentNode.textContent.trim().toLowerCase());
    }
    this.tableTimeHandle?.();
  };

  searchFilterInputEvent = (): void => {
    if (this.searchFilterInput) {
      if (this.searchFilterInput.value === '') {
        this.searchFilterInput.placeholder = 'Search logs...';
      } else {
        this.searchFilterInput.placeholder = '';
      }
    }
    this.tableTimeHandle?.();
  };

  processFilterEvent = (): void => {
    if (this.processFilter) {
      if (this.processFilter.value === '') {
        this.processFilter.placeholder = 'Search process name...';
      } else {
        this.processFilter.placeholder = '';
      }
    }
    this.tableTimeHandle?.();
  };

  levelFilterInputEvent = (): void => {
    this.tableTimeHandle?.();
  };

  logTableScrollEvent = (): void => {
    let newIndex = Math.floor((this.scrollContainer?.scrollTop || 0) / tableRowHeight);
    if (newIndex !== this.startDataIndex) {
      this.startDataIndex = newIndex;
      this.updateVisibleData();
    }
  };

  private updateFilterData(): void {
    if (this.systemLogSource?.length > 0) {
      this.filterData = this.systemLogSource.filter((data) => this.isFilterLog(data));
    }
    if (this.filterData.length > 0) {
      this.hiLogDownLoadTbl!.recycleDataSource = this.filterData;
    } else {
      this.hiLogDownLoadTbl!.recycleDataSource = [];
    }
  }

  private updateVisibleData(): void {
    this.visibleData = [];
    this.logTable!.innerHTML = '';
    if (this.systemLogSource?.length === 0 || this.parentTabEl?.clientHeight === 0) {
      return;
    }
    if (this.parentTabEl) {
      this.containerClientHeight = this.parentTabEl.clientHeight;
    }
    if (this.systemLogSource.length > 0 && this.logTable) {
      let tableHeight = this.containerClientHeight - (this.logTitle?.clientHeight || 0) - tableRowHeight - 12;
      this.scrollContainer!.style.height = `${tableHeight}px`;
      if (this.filterData.length === 0) {
        this.logTable.style.height = this.scrollContainer!.style.height;
        this.logTableTitle!.textContent = 'Hilogs [0, 0] / 0';
      } else {
        this.logTable.style.height = `${this.filterData.length * tableRowHeight}px`;
        let totalLength = Math.ceil(tableHeight / tableRowHeight);
        this.endDataIndex = this.startDataIndex + totalLength;
        if (this.filterData.length >= this.endDataIndex - this.startDataIndex) {
          if (this.endDataIndex >= this.filterData.length) {
            this.visibleData = this.filterData.slice(this.filterData.length - totalLength, this.filterData.length);
          } else {
            this.visibleData = this.filterData.slice(this.startDataIndex, this.endDataIndex);
          }
        } else {
          this.visibleData = this.filterData;
        }
        let maxLength = this.endDataIndex > this.filterData.length ? this.filterData.length : this.endDataIndex;
        this.logTableTitle!.textContent = `Hilogs [${this.startDataIndex || 0}, 
        ${maxLength}] / ${this.filterData.length || 0}`;
        let tableFragment = document.createDocumentFragment();
        this.createVirtualDOM(tableFragment);
        this.logTable.append(tableFragment);
      }
    }
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
    this.traceSheetEl!.systemLogFlag = undefined;
    this.spSystemTrace?.refreshCanvas(false);
    let headEl = this.shadowRoot?.querySelector<HTMLDivElement>('#logs-head-content')
    this.buildTableHead(headEl!);
    this.updateFilterData();
    this.updateVisibleData();
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

  private createVirtualDOM(tableFragment: DocumentFragment): void {
    if (this.visibleData) {
      this.visibleData.forEach((row, index) => {
        let trEL = document.createElement('tr');
        let time = document.createElement('td');
        let timeStampEl = document.createElement('td');
        let levelEl = document.createElement('td');
        let tagEl = document.createElement('td');
        let processNameEl = document.createElement('td');
        let messageEl = document.createElement('td');
        let colorIndex = this.optionLevel.indexOf(row.level!) - defaultIndex;
        if (colorIndex >= 0) {
          trEL.style.color = ColorUtils.getHilogColor(row.level!);
        }
        if (index === 0) {
          let height =
            (this.scrollContainer!.scrollTop <= tableRowHeight ? 0 : this.scrollContainer!.scrollTop) + tableRowHeight;
          trEL.style.height = `${height}px`;
        }
        time.textContent = `${row.originTime}`;
        time.title = `${row.originTime}`;
        timeStampEl.classList.add('time-td');
        timeStampEl.textContent = `${ns2Timestamp(row.startTs!)}`;
        timeStampEl.title = `${ns2Timestamp(row.startTs!)}`;
        levelEl.textContent = `${row.level}`;
        levelEl.title = `${row.level}`;
        tagEl.textContent = `${row.tag}`;
        tagEl.title = `${row.tag}`;
        processNameEl.textContent = `${row.processName}`;
        processNameEl.title = `${row.processName}`;
        messageEl.textContent = `${row.context}`;
        messageEl.title = `${row.context}`;
        trEL.addEventListener('mouseover', () => {
          let pointX: number = ns2x(
            row.startTs || 0,
            TraceRow.range!.startNS,
            TraceRow.range!.endNS,
            TraceRow.range!.totalNS,
            new Rect(0, 0, TraceRow.FRAME_WIDTH, 0)
          );
          this.traceSheetEl!.systemLogFlag = new Flag(Math.floor(pointX), 0, 0, 0, row.startTs!, '#999999', true, '');
          this.spSystemTrace?.refreshCanvas(false);
        });
        trEL.addEventListener('mouseout', () => {
          this.traceSheetEl!.systemLogFlag = undefined;
          this.spSystemTrace?.refreshCanvas(false);
        });
        trEL.appendChild(timeStampEl);
        trEL.appendChild(time);
        trEL.appendChild(levelEl);
        trEL.appendChild(tagEl);
        trEL.appendChild(processNameEl);
        trEL.appendChild(messageEl);
        tableFragment.appendChild(trEL);
      });
    }
  }

  private buildTableHead(tableFragment: HTMLDivElement): void {
    tableFragment.innerHTML = '';
    let trEL = document.createElement('tr');
    trEL.style.display = 'grid';
    trEL.style.width = '100%';
    trEL.style.marginTop = '8px';
    this.tableColumnHead.forEach((columnText) => {
      let columnEl = document.createElement('div');
      columnEl.textContent = columnText;
      columnEl.style.whiteSpace = 'nowrap';
      columnEl.style.overflow = 'hidden';
      columnEl.style.textOverflow = 'ellipsis';
      columnEl.style.fontWeight = 'bold';
      trEL.appendChild(columnEl);
    });
    tableFragment.appendChild(trEL);
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
        .level-select-title {
          margin-right: 10px;
          font-size: 14px;
          line-height: 16px;
          font-weight: 400;
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

  private initTableCssStyle(): string {
    return `<style>
        .tbl-logs {
          display: block;
          height: auto;
          overflow-y: scroll;
        }
        tr {
          display: grid;
          grid-template-columns: 12% 12% 5% 15% 12% 44%;
          background-color: #FFFFFF;
          font-weight: 400;
          opacity: 0.9;
        }
        tr:hover {
          background-color: #E0E5EB;
        }
        td {
          white-space: nowrap;
          overflow-x: scroll;
          overflow-y: hidden;
          display: inline-block;
          margin-left: 10px;
        }
        td::-webkit-scrollbar {
          display: none;
        }
        .head-column {
          white-space: nowrap;
          overflow: hidden;
          text-overflow: ellipsis;
          margin-left: 10px;
          font-weight: bold;
        }
        .time-td {
          color: #3D88C7;
          text-decoration: underline;
          text-decoration-color: #3D88C7;
        }
        .time-td:hover {
          cursor: pointer;
        }
        </style>`;
  }
}

let defaultIndex: number = 1;
let tableRowHeight: number = 23;
let tableTimeOut: number = 50;
