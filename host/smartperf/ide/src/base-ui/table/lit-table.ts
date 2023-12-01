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

import { LitTableColumn } from './lit-table-column.js';
import { LitProgressBar } from './../progress-bar/LitProgressBar.js';
import { element } from '../BaseElement.js';
import '../utils/Template.js';
import { TableRowObject } from './TableRowObject.js';
import { ExcelFormater } from '../utils/ExcelFormater.js';
import { JSONToCSV } from '../utils/CSVFormater.js';
import { NodeType } from '../../js-heap/model/DatabaseStruct.js';
import { ConstructorType } from '../../js-heap/model/UiStruct.js';
import { LitIcon } from '../icon/LitIcon.js';
import { JsCpuProfilerStatisticsStruct } from '../../trace/bean/JsStruct.js';
const iconWidth = 20;
const iconPadding = 5;
@element('lit-table')
export class LitTable extends HTMLElement {
  meauseRowElement: HTMLDivElement | undefined;
  currentRecycleList: HTMLDivElement[] = [];
  currentTreeDivList: HTMLDivElement[] = [];
  public rememberScrollTop = false;
  public getItemTextColor?: (data: any) => string;
  public itemTextHandleMap: Map<string, (value: any) => string> = new Map<string, (value: any) => string>();
  public exportTextHandleMap: Map<string, (value: any) => string> = new Map<string, (value: any) => string>();
  private ds: Array<any> = [];
  public recycleDs: Array<any> = [];
  private normalDs: Array<any> = [];
  private gridTemplateColumns: Array<string> = [];
  /*Grid css layout descriptions are obtained according to the clustern[] nested structure*/
  private st: HTMLSlotElement | null | undefined;
  private tableElement: HTMLDivElement | null | undefined;
  private exportProgress: LitProgressBar | null | undefined;
  private theadElement: HTMLDivElement | null | undefined;
  private columns: Array<Element> | null | undefined;
  private tbodyElement: HTMLDivElement | undefined | null;
  private treeElement: HTMLDivElement | undefined | null;
  private tableColumns: NodeListOf<LitTableColumn> | undefined;
  private colCount: number = 0;
  private currentScrollTop: number = 0;
  private isRecycleList: boolean = true;
  private isScrollXOutSide: boolean = false;
  private exportLoading: boolean = false;
  private _loading: boolean = false;
  private value: Array<any> = [];
  private _mode = TableMode.Expand;
  private columnResizeEnable: boolean = true;
  private _isSearch: boolean = false;

  constructor() {
    super();
    const shadowRoot = this.attachShadow({ mode: 'open' });
    shadowRoot.innerHTML = `
        <style>
        :host{
            display: grid;
            grid-template-columns: repeat(1,1fr);
            width: 100%;
            position: relative;
            font-weight: 500;
            flex:1;
        }
        .tr{
            display: grid;
            grid-column-gap: 5px;
            min-width:100%;
        }
        .tr:nth-of-type(even){
        }
        .tr{
            background-color: var(--dark-background,#FFFFFF);
        }
        .tr:hover{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .tr[selected]{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .tr[high-light]{
            font-weight: 600;
        }
        .td{
            box-sizing: border-box;
            padding: 3px;
            display: flex;
            justify-content: flex-start;
            align-items: center;
            width: 100%;
            height: auto;
            line-height: 21px;
            cursor: pointer;
        }
        .td label{
            overflow: hidden; 
            text-overflow: ellipsis; 
            white-space: normal;
        }
        .td text{
            overflow: hidden; 
            text-overflow: ellipsis; 
            white-space: nowrap;
        }
        .td-order{
        }
        .td-order:before{

        }
        :host([grid-line]) .td{
            border-left: 1px solid #f0f0f0;
        }
        :host([grid-line]) .td:last-of-type{
            border-right: 1px solid #f0f0f0;
        }
        .table{
            width: 100%;
             color: var(--dark-color2,#262626);
        }
        .thead{
            display: grid;
            position: sticky;
            top: 0;
            font-weight: bold;
            font-size: .9rem;
            color: var(--dark-color1,#000);
            background-color: var(--dark-background,#FFFFFF);
            z-index: 1;
        }
        .tbody{
            width: 100%;
            top: 0;
            left: 0;
            right:0;
            bottom:0;
            display: flex;
            flex-direction: row
            row-gap: 1px;
            column-gap: 1px;
        }
        .tree{
            overflow-x:hidden;
            overflow-y:hidden;
            display: grid;
            grid-template-columns: 1fr;
            row-gap: 1px;
            column-gap: 1px;
            position:relative;
        }
        .tree:hover{
            overflow-x: overlay;
        }
        .tree-first-body{
            min-width: 100%;
            box-sizing: border-box;
            display:flex;
            align-items:center;
            white-space: nowrap;
            font-weight: 500;
            cursor: pointer;
        }
        .tree-first-body[high-light]{
            font-weight: 600;
        }
        .tree-first-body:hover{
            background-color: var(--dark-background6,#DEEDFF); /*antd #fafafa 42b983*/
        }
        .body{
            display: grid;
            grid-template-columns: 1fr;
            row-gap: 1px;
            column-gap: 1px;
            flex:1;
            position: relative;
        }
        :host([grid-line])  .tbody{
            border-bottom: 1px solid #f0f0f0;
            background-color: #f0f0f0;
        }
        .th{
            grid-column-gap: 5px;
            display: grid;
            background-color: var(--dark-background,#FFFFFF);
        }
        :host([data-query-scene]) .th {
          background-color: #F6F6F6;
          color: #7E7E7E;
        }
        :host([data-query-scene]) .tr {
          background-color: #F6F6F6;
        }
        .tree-icon{
            font-size: 1.2rem;
            width: 20px;
            height: 20px;
            padding-right: 5px;
            padding-left: 5px;
            cursor: pointer;
        }
        .tree-icon:hover{
            color: #42b983;
        }
        .row-checkbox,row-checkbox-all{

        }
        :host([no-head]) .thead{
            display: none;
        }
        .up-svg{
            position: absolute;
            right: 5px;
            top: 8px;
            bottom: 8px;
            width: 15px;
            height: 15px;
        }
        .down-svg{
            position: absolute;
            top: 8px;
            right: 5px;
            bottom: 8px;
            width: 15px;
            height: 15px;
        }
        .button-icon{
          height: 32px;
          width: 164px;
          color: black;
          font-size: 14px;
          border: 1px solid black;
          display: flex;
          flex-direction: row;
          align-items: center;
          justify-content: center;
          cursor: pointer;
          background: var(--dark-background3,#FFFFFF);
          border-radius: 20px;
          padding: 15px;
          transition: opacity 0.2s;
          outline: none;
          position: relative;
          overflow: hidden;
        }
        .button-icon:active {
          background: var(--dark-background1,#f5f5f5)
        }
        .mouse-select{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .mouse-in{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .export{
            height:32px;
            width: 32px;
            cursor:pointer;
            display:none;
            align-items:center;
            justify-content:center;
            border-radius:5px;
            box-sizing: border-box;
            background-color: #000000;
            opacity: 0.3;
            position:absolute;
            right:20px;
            bottom:20px;
            z-index: 999999;
        }
        .resize{
            width: 2px;
            margin-right: 3px;
            height: 20px;
            background-color: #e0e0e0;
            cursor: col-resize;
        }
        .progress{
            position: absolute;
            height: 1px;
            top: 0;
            left: 0;
            right: 0;
            z-index: 999999;
        } 
        :host([hideDownload]) .export{
            display: none;
        }
        </style>
        <lit-progress-bar id="export_progress_bar" class="progress"></lit-progress-bar>
        <slot id="slot" style="display: none"></slot>
        <slot name="head"></slot>
        <div class="export">
            <lit-icon size="18" style="color: #ffffff" name="copyhovered" ></lit-icon>
        </div>
        <div class="table" style="overflow-x:auto;">
            <div class="thead"></div>
            <div class="tbody">
                <div class="tree"></div>
                <div class="body"></div>
            </div>
        </div>
        `;
  }

  static get observedAttributes(): string[] {
    return [
      'scroll-y',
      'selectable',
      'no-head',
      'grid-line',
      'defaultOrderColumn',
      'hideDownload',
      'noRecycle',
      'loading',
      'expand',
    ];
  }

  set mode(mode: TableMode) {
    this._mode = mode;
  }

  set loading(value: boolean) {
    this._loading = value;
    this.exportProgress!.loading = value;
  }

  get hideDownload(): boolean {
    return this.hasAttribute('hideDownload');
  }

  set hideDownload(value) {
    if (value) {
      this.setAttribute('hideDownload', '');
    } else {
      this.removeAttribute('hideDownload');
    }
  }

  get selectable(): boolean {
    return this.hasAttribute('selectable');
  }

  set selectable(value) {
    if (value) {
      this.setAttribute('selectable', '');
    } else {
      this.removeAttribute('selectable');
    }
  }

  get scrollY(): string {
    return this.getAttribute('scroll-y') || 'auto';
  }

  set scrollY(value) {
    this.setAttribute('scroll-y', value);
  }

  get dataSource(): any[] {
    return this.ds || [];
  }

  set dataSource(value) {
    if (this.hasAttribute('noRecycle')) {
      this.ds = value;
      this.isRecycleList = false;
      this.renderTable();
    } else {
      this.columnResizeEnable = false;
      this.recycleDataSource = value;
    }
  }

  set noRecycle(value) {
    if (value) {
      this.setAttribute('noRecycle', '');
    } else {
      this.removeAttribute('noRecycle');
    }
  }

  get noRecycle(): boolean {
    return this.hasAttribute('noRecycle');
  }

  get recycleDataSource(): any[] {
    return this.ds || [];
  }

  set isSearch(value: boolean) {
    this._isSearch = value;
  }

  set recycleDataSource(value) {
    if (this.tableElement) {
      this.isScrollXOutSide = this.tableElement!.scrollWidth > this.tableElement!.clientWidth;
      this.isRecycleList = true;
      this.ds = value;
      if (this.rememberScrollTop) {
        this.currentScrollTop = this.tableElement!.scrollTop;
        this.tableElement!.scrollTop = 0;
        this.tableElement!.scrollLeft = 0;
      } else {
        this.tableElement!.scrollTop = 0;
        this.tableElement!.scrollLeft = 0;
      }
      if (this.hasAttribute('tree')) {
        if (value.length === 0) {
          this.value = [];
          this.recycleDs = this.meauseTreeRowElement(value);
        } else {
          if (
            value !== this.value &&
            this.value.length !== 0 &&
            !this._isSearch &&
            this.querySelector('lit-table-column')?.hasAttribute('retract')
          ) {
            this.shadowRoot!.querySelector<LitIcon>('.top')!.name = 'up';
            this.shadowRoot!.querySelector<LitIcon>('.bottom')!.name = 'down';
          }
          this._isSearch = false;
          this.value = value;
          this.recycleDs = this.meauseTreeRowElement(value, RedrawTreeForm.Retract);
        }
      } else {
        this.recycleDs = this.meauseAllRowHeight(value);
      }
    }
  }

  get snapshotDataSource(): any[] {
    return this.ds || [];
  }

  set snapshotDataSource(value) {
    this.ds = value;
    if (this.hasAttribute('tree')) {
      this.recycleDs = this.meauseTreeRowElement(value, RedrawTreeForm.Default);
    } else {
      this.recycleDs = this.meauseAllRowHeight(value);
    }
  }

  move1px(): void {
    this.tableElement!.scrollTop = this.tableElement!.scrollTop + 1;
  }

  dataExportInit(): void {
    let exportDiv = this.shadowRoot!.querySelector<HTMLDivElement>('.export');
    exportDiv &&
      (exportDiv.onclick = (): void => {
        this.exportData();
      });
  }

  exportData(): void {
    if (this.exportLoading || this.ds.length === 0) {
      return;
    }
    this.exportLoading = true;
    this.exportProgress!.loading = true;
    let date = new Date();
    JSONToCSV.csvExport({
      columns: this.columns as any[],
      tables: this.ds,
      fileName: `${date.getTime()}`,
      columnFormatter: this.itemTextHandleMap,
      exportFormatter: this.exportTextHandleMap
    }).then((res) => {
      this.exportLoading = false;
      this.exportProgress!.loading = false;
    });
  }

  exportExcelData(): void {
    let now = Date.now();
    ExcelFormater.testExport(
      [
        {
          columns: this.columns as any[],
          tables: this.ds,
          sheetName: `${now}`,
        },
      ],
      `${now}`
    );
  }

  formatExportData(dataSource: any[]): any[] {
    if (dataSource === undefined || dataSource.length === 0) {
      return [];
    }
    if (this.columns === undefined) {
      return [];
    }
    return dataSource.map((item) => {
      let formatData: any = {};
      this.columns!.forEach((column) => {
        let dataIndex = column.getAttribute('data-index');
        let columnName = column.getAttribute('title');
        if (columnName === '') {
          columnName = dataIndex;
        }
        if (dataIndex && columnName && item[dataIndex] != undefined) {
          formatData[columnName] = item[dataIndex];
        }
      });
      if (item.children !== undefined) {
        formatData.children = this.formatExportData(item.children);
      }
      return formatData;
    });
  }

  formatExportCsvData(dataSource: any[]): string {
    if (dataSource === undefined || dataSource.length === 0) {
      return '';
    }
    if (this.columns == undefined) {
      return '';
    }
    let str = '';
    str += this.columns!.map((column) => {
      let dataIndex = column.getAttribute('data-index');
      let columnName = column.getAttribute('title');
      if (columnName === '') {
        columnName = dataIndex;
      }
      return columnName;
    }).join(',');
    str += this.recursionExportTableData(this.columns, dataSource);
    return str;
  }

  recursionExportTableData(columns: any[], dataSource: any[]): string {
    let concatStr = '\r\n';
    dataSource.forEach((item, index) => {
      concatStr += columns
        .map((column) => {
          let dataIndex = column.getAttribute('data-index');
          return `"${item[dataIndex] || ''}"    `;
        })
        .join(',');
      if (item.children !== undefined) {
        concatStr += this.recursionExportTableData(columns, item.children);
      }
      if (index !== dataSource.length - 1) {
        concatStr += '\r\n';
      }
    });
    return concatStr;
  }

  injectColumns(): void {
    this.columns = this.st!.assignedElements();
    this.columns.forEach((column) => {
      if (column.tagName === 'LIT-TABLE-COLUMN') {
        this.gridTemplateColumns.push(column.getAttribute('width') || '1fr');
      }
    });
  }

  setStatus(list: any, status: boolean, depth: number = 0): void {
    this.tableElement!.scrollTop = 0;
    // 添加depth参数，让切换图标的代码在递归中只走一遍
    if (depth === 0) {
      if (status) {
        this.shadowRoot!.querySelector<LitIcon>('.top')!.name = 'down';
        this.shadowRoot!.querySelector<LitIcon>('.bottom')!.name = 'up';
      } else {
        this.shadowRoot!.querySelector<LitIcon>('.top')!.name = 'up';
        this.shadowRoot!.querySelector<LitIcon>('.bottom')!.name = 'down';
      }
    }
    for (let item of list) {
      item.status = status;
      if (item.children !== undefined && item.children.length > 0) {
        this.setStatus(item.children, status, depth + 1);
      }
    }
  }

  private isResize: boolean = false;
  private resizeColumnIndex: number = -1;
  private resizeDownX: number = 0;
  private columnMinWidth: number = 50;
  private beforeResizeWidth1: number = 0;
  private beforeResizeWidth2: number = 0;

  resizeEventHandler(header: HTMLDivElement, element: HTMLDivElement, index: number): void {
    header.addEventListener('mousemove', (event) => {
      if (!this.columnResizeEnable) return;
      if (this.isResize) {
        let width = event.clientX - this.resizeDownX;
        header.style.cursor = 'col-resize';
        let preWidth = this.beforeResizeWidth1,
          nowWidth = this.beforeResizeWidth2;
        if (width < 0) {
          preWidth = Math.max(this.beforeResizeWidth1 + width, this.columnMinWidth);
          nowWidth = this.beforeResizeWidth1 - preWidth + this.beforeResizeWidth2;
        }
        if (width > 0) {
          nowWidth = Math.max(this.beforeResizeWidth2 - width, this.columnMinWidth);
          preWidth = this.beforeResizeWidth2 - nowWidth + this.beforeResizeWidth1;
        }
        this.gridTemplateColumns[this.resizeColumnIndex - 1] = `${preWidth}px`;
        this.gridTemplateColumns[this.resizeColumnIndex] = `${nowWidth}px`;
        header.style.gridTemplateColumns = this.gridTemplateColumns.join(' ');
        this.shadowRoot!.querySelectorAll<HTMLDivElement>('.tr').forEach((tr) => {
          if (this.hasAttribute('tree')) {
            tr.style.gridTemplateColumns = this.gridTemplateColumns.slice(1).join(' ');
          } else {
            tr.style.gridTemplateColumns = this.gridTemplateColumns.join(' ');
          }
        });
        event.preventDefault();
        event.stopPropagation();
      } else {
        header.style.cursor = 'pointer';
      }
    });
    header.addEventListener('mouseup', (event) => {
      if (!this.columnResizeEnable) return;
      this.isResize = false;
      this.resizeDownX = 0;
      header.style.cursor = 'pointer';
      setTimeout(() => {
        this.resizeColumnIndex = -1;
      }, 100);
      event.stopPropagation();
      event.preventDefault();
    });
    header.addEventListener('mouseleave', (event) => {
      if (!this.columnResizeEnable) return;
      event.stopPropagation();
      event.preventDefault();
      this.isResize = false;
      this.resizeDownX = 0;
      this.resizeColumnIndex = -1;
      header.style.cursor = 'pointer';
    });
    element.addEventListener('mousedown', (event) => {
      if (!this.columnResizeEnable) return;
      this.isResize = true;
      this.resizeColumnIndex = index;
      this.resizeDownX = event.clientX;
      let pre = header.childNodes.item(this.resizeColumnIndex - 1) as HTMLDivElement;
      let now = header.childNodes.item(this.resizeColumnIndex) as HTMLDivElement;
      this.beforeResizeWidth1 = pre.clientWidth;
      this.beforeResizeWidth2 = now.clientWidth;
      event.stopPropagation();
    });
    element.addEventListener('click', (event) => {
      event.stopPropagation();
    });
  }

  // Is called when the custom element is removed from the document DOM.
  disconnectedCallback(): void {}

  // It is called when the custom element is moved to a new document.
  adoptedCallback(): void {}

  // It is called when a custom element adds, deletes, or modifies its own properties.
  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {}

  fixed(td: HTMLElement, placement: string, bgColor: string): void {
    td.style.position = 'sticky';
    if (placement === 'left') {
      td.style.left = '0px';
      td.style.boxShadow = '3px 0px 5px #33333333';
    } else if (placement === 'right') {
      td.style.right = '0px';
      td.style.boxShadow = '-3px 0px 5px #33333333';
    }
  }

  getCheckRows(): any[] {
    // @ts-ignore
    return [...this.shadowRoot!.querySelectorAll('div[class=tr][checked]')]
      .map((a) => (a as any).data)
      .map((a) => {
        delete a['children'];
        return a;
      });
  }

  deleteRowsCondition(fn: any): void {
    this.shadowRoot!.querySelectorAll('div[class=tr]').forEach((tr) => {
      // @ts-ignore
      if (fn(tr.data)) {
        tr.remove();
      }
    });
  }

  meauseElementHeight(rowData: any): number {
    return 27;
  }

  meauseTreeElementHeight(rowData: any, depth: number): number {
    return 27;
  }

  meauseAllRowHeight(list: any[]): TableRowObject[] {
    this.tbodyElement!.innerHTML = '';
    this.meauseRowElement = undefined;
    let head = this.shadowRoot!.querySelector('.th');
    this.tbodyElement && (this.tbodyElement.style.width = head?.clientWidth + 'px');
    this.currentRecycleList = [];
    let headHeight = 0;
    let totalHeight = headHeight;
    let visibleObjects: TableRowObject[] = [];
    let itemHandler = (rowData: any, index: number) => {
      let height = this.meauseElementHeight(rowData);
      let tableRowObject = new TableRowObject();
      tableRowObject.height = height;
      tableRowObject.top = totalHeight;
      tableRowObject.data = rowData;
      tableRowObject.rowIndex = index;
      if (
        Math.max(totalHeight, this.tableElement!.scrollTop + headHeight) <=
        Math.min(totalHeight + height, this.tableElement!.scrollTop + this.tableElement!.clientHeight + headHeight)
      ) {
        let newTableElement = this.createNewTableElement(tableRowObject);
        newTableElement.style.transform = `translateY(${totalHeight}px)`;
        this.tbodyElement?.append(newTableElement);
        this.currentRecycleList.push(newTableElement);
        let td = newTableElement?.querySelectorAll('.td');
        if (tableRowObject.data.rowName === 'cpu-profiler') {
          this.createTextColor(tableRowObject, td[0]);
        }
      }
      totalHeight += height;
      visibleObjects.push(tableRowObject);
    };
    let realIndex = 0;
    list.forEach((item, index) => {
      if (Array.isArray(item)) {
        item.forEach((rowData, childIndex) => {
          itemHandler(rowData, realIndex);
          realIndex++;
        });
      } else {
        itemHandler(item, index);
      }
    });
    this.tbodyElement && (this.tbodyElement.style.height = totalHeight + (this.isScrollXOutSide ? 0 : 0) + 'px');
    this.tableElement &&
      (this.tableElement.onscroll = (event) => {
        let tblScrollTop = this.tableElement!.scrollTop;
        let skip = 0;
        for (let i = 0; i < visibleObjects.length; i++) {
          if (
            visibleObjects[i].top <= tblScrollTop &&
            visibleObjects[i].top + visibleObjects[i].height >= tblScrollTop
          ) {
            skip = i;
            break;
          }
        }
        let reduce = this.currentRecycleList.map((item) => item.clientHeight).reduce((a, b) => a + b, 0);
        if (reduce == 0) {
          return;
        }
        while (reduce <= this.tableElement!.clientHeight) {
          let newTableElement = this.createNewTableElement(visibleObjects[skip]);
          this.tbodyElement?.append(newTableElement);
          this.currentRecycleList.push(newTableElement);
          reduce += newTableElement.clientHeight;
        }
        for (let i = 0; i < this.currentRecycleList.length; i++) {
          this.freshCurrentLine(this.currentRecycleList[i], visibleObjects[i + skip]);
          if (visibleObjects[i + skip]) {
            if (visibleObjects[i + skip].data.rowName === 'cpu-profiler') {
              this.createTextColor(visibleObjects[i + skip], this.currentRecycleList[i].childNodes[0]);
            }
          }
        }
      });
    return visibleObjects;
  }

  newTableRowObject(item: any, totalHeight: number, depth: number, parentNode?: TableRowObject): TableRowObject {
    let tableRowObject = new TableRowObject();
    tableRowObject.depth = depth;
    tableRowObject.data = item;
    tableRowObject.top = totalHeight;
    tableRowObject.height = this.meauseTreeElementHeight(tableRowObject, depth);
    if (parentNode) {
      parentNode!.children.push(tableRowObject);
    }
    return tableRowObject;
  }

  createBtn(rowData: any): any {
    let btn: any = document.createElement('lit-icon');
    btn.classList.add('tree-icon');
    if (rowData.data.expanded) {
      btn.name = 'plus-square';
    } else {
      btn.name = 'minus-square';
    }
    btn.addEventListener('click', (e: any) => {
      rowData.data.status = false;
      const resetNodeHidden = (hidden: boolean, rowData: any) => {
        if (hidden) {
          rowData.children.forEach((child: any) => {
            child.rowHidden = false;
          });
        } else {
          rowData.children.forEach((child: any) => {
            child.rowHidden = true;
            resetNodeHidden(hidden, child);
          });
        }
      };

      if (rowData.data.expanded) {
        rowData.data.status = true;
        this.dispatchRowClickEventIcon(rowData, [btn]);
        rowData.data.expanded = false;
        resetNodeHidden(true, rowData);
      } else {
        rowData.data.expanded = true;
        rowData.data.status = false;
        resetNodeHidden(false, rowData);
      }
      this.reMeauseHeight();
    });
    return btn;
  }

  createExpandBtn(rowData: any): any {
    let btn: any = document.createElement('lit-icon');
    btn.classList.add('tree-icon');
    // @ts-ignore
    if (rowData.expanded) {
      btn.name = 'minus-square';
    } else {
      btn.name = 'plus-square';
    }
    btn.onclick = (e: Event) => {
      const resetNodeHidden = (hidden: boolean, rowData: any) => {
        if (rowData.children.length > 0) {
          if (hidden) {
            rowData.children.forEach((child: any) => {
              child.rowHidden = true;
              resetNodeHidden(hidden, child);
            });
          } else {
            rowData.children.forEach((child: any) => {
              child.rowHidden = !rowData.expanded;
              if (rowData.expanded) {
                resetNodeHidden(hidden, child);
              }
            });
          }
        }
      };

      if (rowData.expanded && this._mode === TableMode.Retract) {
        rowData.data.status = false;
        rowData.expanded = false;
        resetNodeHidden(true, rowData);
      } else if (!rowData.expanded && this._mode === TableMode.Retract) {
        rowData.expanded = true;
        rowData.data.status = true;
        this.recycleDs = this.meauseTreeRowElement(this.value, RedrawTreeForm.Retract);
        resetNodeHidden(false, rowData);
      }

      if (this._mode === TableMode.Expand && rowData.expanded) {
        // 点击收起的时候将点击的那条数据的status改为false
        setChildrenStatus(this.value);
        function setChildrenStatus(data: any) {
          for (let d of data) {
            if (rowData.data === d) {
              d.status = false;
            }
            if (d.children != undefined && d.children.length > 0) {
              setChildrenStatus(d.children);
            }
          }
        }
        rowData.expanded = false;
        resetNodeHidden(true, rowData);
      } else if (this._mode === TableMode.Expand && !rowData.expanded) {
        if (rowData.data.children) {
          rowData.data.status = true;
        }
        this.recycleDs = this.meauseTreeRowElement(this.value, RedrawTreeForm.Default);
        rowData.expanded = true;
        resetNodeHidden(false, rowData);
      }
      this.reMeauseHeight();
      e.stopPropagation();
    };
    return btn;
  }

  createNewTableElement(rowData: any): any {
    let newTableElement = document.createElement('div');
    newTableElement.classList.add('tr');
    this?.columns?.forEach((column: any) => {
      let dataIndex = column.getAttribute('data-index') || '1';
      let td: any;
      td = document.createElement('div');
      td.classList.add('td');
      td.style.overflow = 'hidden';
      td.style.textOverflow = 'ellipsis';
      td.style.whiteSpace = 'nowrap';
      td.dataIndex = dataIndex;
      td.style.justifyContent = column.getAttribute('align') || 'flex-start';
      let text = this.formatName(dataIndex, rowData.data[dataIndex]);
      if (text.indexOf('&lt;') === -1) {
        if (dataIndex === 'totalTimeStr' && rowData.data.chartFrameChildren) {
          td.title = rowData.data.totalTime + 'ns';
        } else {
          td.title = text;
        }
      }
      //   如果表格中有模板的情况，将模板中的数据放进td中，没有模板，直接将文本放进td
      //  但是对于Current Selection tab页来说，表格前两列是时间，第三列是input标签，第四列是button标签
      //  而第一行的数据只有第四列一个button，和模板中的数据并不一样，所以要特别处理一下
      if (column.template) {
        if (dataIndex === 'color' && rowData.data.colorEl === undefined) {
          td.innerHTML = '';
          td.template = '';
        } else if (dataIndex === 'operate' && rowData.data.operate && rowData.data.operate.innerHTML === 'RemoveAll') {
          let removeAll = document.createElement('button');
          removeAll.className = 'removeAll';
          removeAll.innerHTML = 'RemoveAll';
          removeAll.style.background = 'var(--dark-border1,#262f3c)';
          removeAll.style.color = 'white';
          removeAll.style.borderRadius = '10px';
          removeAll.style.fontSize = '10px';
          removeAll.style.height = '18px';
          removeAll.style.lineHeight = '18px';
          removeAll.style.minWidth = '7em';
          removeAll.style.border = 'none';
          removeAll.style.cursor = 'pointer';
          removeAll.style.outline = 'inherit';
          td.appendChild(removeAll);
        } else {
          td.appendChild(column.template.render(rowData.data).content.cloneNode(true));
          td.template = column.template;
        }
      } else {
        td.innerHTML = text;
      }
      newTableElement.append(td);
    });
    newTableElement.onmouseup = (e: MouseEvent) => {
      this.dispatchRowClickEvent(rowData, [newTableElement], e);
    };
    newTableElement.onmouseenter = () => {
      this.dispatchRowHoverEvent(rowData, [newTableElement]);
    };
    if (rowData.data.isSelected != undefined) {
      this.setSelectedRow(rowData.data.isSelected, [newTableElement]);
    }
    (newTableElement as any).data = rowData.data;
    newTableElement.style.cursor = 'pointer';
    newTableElement.style.gridTemplateColumns = this.gridTemplateColumns.join(' ');
    newTableElement.style.position = 'absolute';
    newTableElement.style.top = '0px';
    newTableElement.style.left = '0px';
    if (this.getItemTextColor) {
      newTableElement.style.color = this.getItemTextColor(rowData.data);
    }
    return newTableElement;
  }

  getWheelStatus(element: any): void {
    element.addEventListener('wheel', (event: WheelEvent) => {
      if (element.scrollWidth !== element.offsetWidth) {
        event.preventDefault();
      }
      element.scrollLeft += event.deltaY;
    });
  }

  renderTable(): void {
    if (!this.columns) {
      return;
    }
    if (!this.ds) {
      return;
    } // If no data source is set, it is returned directly
    this.normalDs = [];
    this.tbodyElement!.innerHTML = ''; // Clear the table contents
    this.ds.forEach((rowData: any) => {
      let tblRowElement = document.createElement('div');
      tblRowElement.classList.add('tr');
      // @ts-ignore
      tblRowElement.data = rowData;
      let gridTemplateColumns: Array<any> = [];
      // If the table is configured with selectable (select row mode) add a checkbox at the head of the line alone
      if (this.selectable) {
        let tblBox = document.createElement('div');
        tblBox.style.display = 'flex';
        tblBox.style.justifyContent = 'center';
        tblBox.style.alignItems = 'center';
        tblBox.classList.add('td');
        let checkbox = document.createElement('lit-checkbox');
        checkbox.classList.add('row-checkbox');
        checkbox.onchange = (e: any) => {
          // Checkbox checking affects whether the div corresponding to the row has a checked attribute for marking
          if (e.detail.checked) {
            tblRowElement.setAttribute('checked', '');
          } else {
            tblRowElement.removeAttribute('checked');
          }
        };
        this.getWheelStatus(tblBox);
        tblBox.appendChild(checkbox);
        tblRowElement.appendChild(tblBox);
      }
      this.tableColumns!.forEach((tblColumn) => {
        tblColumn.addEventListener('contextmenu', (e) => {
          e.preventDefault();
        });
        let dataIndex = tblColumn.getAttribute('data-index') || '1';
        gridTemplateColumns.push(tblColumn.getAttribute('width') || '1fr');
        if (tblColumn.template) {
          // If you customize the rendering, you get the nodes from the template
          // @ts-ignore
          let cloneNode = tblColumn.template.render(rowData).content.cloneNode(true);
          let tblCustomDiv = document.createElement('div');
          tblCustomDiv.classList.add('td');
          tblCustomDiv.style.wordBreak = 'break-all';
          tblCustomDiv.style.whiteSpace = 'pre-wrap';
          tblCustomDiv.style.justifyContent = tblColumn.getAttribute('align') || '';
          if (tblColumn.hasAttribute('fixed')) {
            this.fixed(tblCustomDiv, tblColumn.getAttribute('fixed') || '', '#ffffff');
          }
          this.getWheelStatus(tblCustomDiv);
          tblCustomDiv.append(cloneNode);
          tblRowElement.append(tblCustomDiv);
        } else {
          let tblDiv = document.createElement('div');
          tblDiv.classList.add('td');
          tblDiv.style.wordBreak = 'break-all';
          tblDiv.style.whiteSpace = 'pre-wrap';
          tblDiv.title = rowData[dataIndex];
          tblDiv.style.justifyContent = tblColumn.getAttribute('align') || '';
          if (tblColumn.hasAttribute('fixed')) {
            this.fixed(tblDiv, tblColumn.getAttribute('fixed') || '', '#ffffff');
          }
          this.getWheelStatus(tblDiv);
          tblDiv.innerHTML = this.formatName(dataIndex, rowData[dataIndex]);
          tblRowElement.append(tblDiv);
        }
      });
      if (this.selectable) {
        // If the table with selection is preceded by a 60px column
        tblRowElement.style.gridTemplateColumns = '60px ' + gridTemplateColumns.join(' ');
      } else {
        tblRowElement.style.gridTemplateColumns = gridTemplateColumns.join(' ');
      }
      tblRowElement.onclick = (e) => {
        this.dispatchEvent(
          new CustomEvent('row-click', {
            detail: {
              rowData,
              data: rowData,
              callBack: (isSelected: boolean) => {
                //是否爲单选
                if (isSelected) {
                  this.clearAllSelection(rowData);
                }
                this.setSelectedRow(rowData.isSelected, [tblRowElement]);
              },
            },
            composed: true,
          })
        );
      };
      this.normalDs.push(tblRowElement);
      this.tbodyElement!.append(tblRowElement);
    });
  }

  setSelectedRow(isSelected: boolean, rows: any[]): void {
    if (isSelected) {
      rows.forEach((row) => {
        if (row.classList.contains('mouse-in')) {
          row.classList.remove('mouse-in');
        }
        row.classList.add('mouse-select');
      });
    } else {
      rows.forEach((row) => {
        row.classList.remove('mouse-select');
      });
    }
  }

  setMouseIn(isMouseIn: boolean, rows: any[]): void {
    if (isMouseIn) {
      rows.forEach((row) => {
        row.classList.add('mouse-in');
      });
    } else {
      rows.forEach((row) => {
        row.classList.remove('mouse-in');
      });
    }
  }

  scrollToData(data: any): void {
    if (this.isRecycleList) {
      if (this.recycleDs.length > 0) {
        let filter = this.recycleDs.filter((item) => {
          return item.data === data;
        });
        if (filter.length > 0) {
          this.tableElement!.scrollTop = filter[0].top;
        }
        this.setCurrentSelection(data);
      }
    } else {
      if (this.normalDs.length > 0) {
        let filter = this.normalDs.filter((item) => {
          return item.data === data;
        });
        if (filter.length > 0) {
          this.tableElement!.scrollTop = filter[0].top;
        }
      }
    }
  }

  expandList(datasource: any[]): void {
    let filter = this.recycleDs.filter((item) => {
      return datasource.indexOf(item.data) != -1;
    });
    if (filter.length > 0) {
      filter.forEach((item) => {
        item.expanded = true;
        item.rowHidden = false;
      });
    }
    this.reMeauseHeight();
  }

  clearAllSelection(rowObjectData: any): void {
    if (this.isRecycleList) {
      this.recycleDs.forEach((item) => {
        if (item.data != rowObjectData && item.data.isSelected) {
          item.data.isSelected = false;
        }
      });
      this.setSelectedRow(false, this.currentTreeDivList);
      this.setSelectedRow(false, this.currentRecycleList);
    } else {
      this.dataSource.forEach((item) => {
        if (item != rowObjectData && item.isSelected) {
          item.isSelected = false;
        }
      });
      this.setSelectedRow(false, this.normalDs);
    }
  }

  clearAllHover(rowObjectData: any): void {
    if (this.isRecycleList) {
      this.recycleDs.forEach((item) => {
        if (item.data != rowObjectData && item.data.isHover) {
          item.data.isHover = false;
        }
      });
      this.setMouseIn(false, this.currentTreeDivList);
      this.setMouseIn(false, this.currentRecycleList);
    } else {
      this.dataSource.forEach((item) => {
        if (item != rowObjectData && item.isHover) {
          item.isHover = false;
        }
      });
      this.setMouseIn(false, this.normalDs);
    }
  }

  mouseOut(): void {
    if (this.isRecycleList) {
      this.recycleDs.forEach((item) => (item.data.isHover = false));
      this.setMouseIn(false, this.currentTreeDivList);
      this.setMouseIn(false, this.currentRecycleList);
    } else {
      this.dataSource.forEach((item) => (item.isHover = false));
      this.setMouseIn(false, this.normalDs);
    }
    this.dispatchEvent(
      new CustomEvent('row-hover', {
        detail: {
          data: undefined,
        },
        composed: true,
      })
    );
  }

  setCurrentSelection(selectionData: any): void {
    if (this.isRecycleList) {
      if (selectionData.isSelected !== undefined) {
        this.currentTreeDivList.forEach((itemEl) => {
          if ((itemEl as any).data === selectionData) {
            this.setSelectedRow(selectionData.isSelected, [itemEl]);
          }
        });
        this.currentRecycleList.forEach((recycleItem) => {
          if ((recycleItem as any).data === selectionData) {
            this.setSelectedRow(selectionData.isSelected, [recycleItem]);
          }
        });
      }
    } else {
      if (selectionData.isSelected !== undefined) {
        this.normalDs.forEach((item) => {
          if ((item as any).data === selectionData) {
            this.setSelectedRow(selectionData.isSelected, [item]);
          }
        });
      }
    }
  }

  setCurrentHover(data: any): void {
    if (this.isRecycleList) {
      this.setMouseIn(false, this.currentTreeDivList);
      this.setMouseIn(false, this.currentRecycleList);
      if (data.isHover !== undefined) {
        this.currentTreeDivList.forEach((hoverItem) => {
          if ((hoverItem as any).data === data) {
            this.setMouseIn(data.isHover, [hoverItem]);
          }
        });
        this.currentRecycleList.forEach((hoverItem) => {
          if ((hoverItem as any).data === data) {
            this.setMouseIn(data.isHover, [hoverItem]);
          }
        });
      }
    } else {
      this.setMouseIn(false, this.normalDs);
      if (data.isHover !== undefined) {
        this.normalDs.forEach((item): void => {
          if ((item as any).data === data) {
            this.setMouseIn(data.isHover, [item]);
          }
        });
      }
    }
  }

  dispatchRowClickEventIcon(rowData: any, elements: any[]): void {
    this.dispatchEvent(
      new CustomEvent('icon-click', {
        detail: {
          ...rowData.data,
          data: rowData.data,
          callBack: (isSelected: boolean): void => {
            //是否爲单选
            if (isSelected) {
              this.clearAllSelection(rowData.data);
            }
            this.setSelectedRow(rowData.data.isSelected, elements);
          },
        },
        composed: true,
      })
    );
  }

  dispatchRowClickEvent(rowObject: any, elements: any[], event: MouseEvent): void {
    this.dispatchEvent(
      new CustomEvent('row-click', {
        detail: {
          button: event.button,
          ...rowObject.data,
          data: rowObject.data,
          callBack: (isSelected: boolean): void => {
            //是否爲单选
            if (isSelected) {
              this.clearAllSelection(rowObject.data);
            }
            this.setSelectedRow(rowObject.data.isSelected, elements);
          },
        },
        composed: true,
      })
    );
  }

  dispatchRowHoverEvent(rowObject: any, elements: any[]): void {
    this.dispatchEvent(
      new CustomEvent('row-hover', {
        detail: {
          data: rowObject.data,
          callBack: (): void => {
            this.clearAllHover(rowObject.data);
            this.setMouseIn(rowObject.data.isHover, elements);
          },
        },
        composed: true,
      })
    );
  }

  formatName(key: string, name: any): any {
    let content = name;
    if (this.itemTextHandleMap.has(key)) {
      content = this.itemTextHandleMap.get(key)?.(name) || '';
    }
    if (content !== undefined && content !== null) {
      return content.toString().replace(/</g, '&lt;').replace(/>/g, '&gt;');
    }
    return '';
  }

  setHighLight(isSearch: boolean, element: any): void {
    if (isSearch) {
      element.setAttribute('high-light', '');
    } else {
      element.removeAttribute('high-light');
    }
  }

  createTextColor(rowData: any, divElement: any): void {
    let nodeText = document.createElement('text');
    nodeText.classList.add('functionName');
    nodeText.textContent = rowData.data.name;
    divElement.append(nodeText);
    if (rowData.data.scriptName !== 'unknown') {
      let scriptText = document.createElement('text');
      scriptText.classList.add('scriptName');
      scriptText.textContent = rowData.data.scriptName;
      divElement.append(scriptText);
      scriptText.style.color = '#a1a1a1';
    }
    divElement.title = rowData.data.symbolName;
  }
}

// 表格默认是展开还是收起的
export enum TableMode {
  Expand, // 默认展开
  Retract, // 默认收起
}

// 重绘的表格是要全部展开，全部收起，还是一层一层手动打开
export enum RedrawTreeForm {
  Expand, // 一键展开
  Retract, // 一键收起
  Default, //点击加号，逐层展开
}
