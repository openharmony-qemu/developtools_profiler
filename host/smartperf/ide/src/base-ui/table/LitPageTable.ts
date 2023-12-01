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
import { BaseElement, element } from '../BaseElement.js';
import '../utils/Template.js';
import { TableRowObject } from './TableRowObject.js';
import { JSONToCSV } from '../utils/CSVFormater.js';

@element('lit-page-table')
export class LitPageTable extends BaseElement {
  meauseRowElement: HTMLDivElement | undefined;
  currentRecycleList: HTMLDivElement[] = [];
  currentTreeDivList: HTMLDivElement[] = [];
  public rememberScrollTop = false;
  public getItemTextColor?: (data: any) => string;
  public itemTextHandleMap: Map<string, (value: any) => string> = new Map<string, (value: any) => string>();
  public exportTextHandleMap: Map<string, (value: any) => string> = new Map<string, (value: any) => string>();
  private ds: Array<any> = [];
  public recycleDs: Array<any> = [];
  private gridTemplateColumns: Array<string> = [];
  private st: HTMLSlotElement | null | undefined;
  private tableElement: HTMLDivElement | null | undefined;
  private exportProgress: LitProgressBar | null | undefined;
  private theadElement: HTMLDivElement | null | undefined;
  private columns: Array<Element> | null | undefined;
  private tbodyElement: HTMLDivElement | undefined | null;
  private treeElement: HTMLDivElement | undefined | null;
  private previousDiv: HTMLDivElement | undefined | null;
  private nextDiv: HTMLDivElement | undefined | null;
  private currentPageDiv: HTMLDivElement | undefined | null;
  private targetPageInput: HTMLInputElement | undefined | null;
  private jumpDiv: HTMLDivElement | undefined | null;
  private tableColumns: NodeListOf<LitTableColumn> | undefined;
  private colCount: number = 0;
  private currentScrollTop: number = 0;
  private isScrollXOutSide: boolean = false;
  private exportLoading: boolean = false;
  private currentPage: number = 0;
  private _loading: boolean = false;
  startSkip: number = 0;

  static get observedAttributes() {
    return [
      'scroll-y',
      'selectable',
      'no-head',
      'grid-line',
      'defaultOrderColumn',
      'hideDownload',
      'loading',
      'pagination',
    ];
  }

  set loading(value: boolean) {
    this._loading = value;
    this.exportProgress!.loading = value;
  }

  get hideDownload() {
    return this.hasAttribute('hideDownload');
  }

  set hideDownload(value) {
    if (value) {
      this.setAttribute('hideDownload', '');
    } else {
      this.removeAttribute('hideDownload');
    }
  }

  get selectable() {
    return this.hasAttribute('selectable');
  }

  set selectable(value) {
    if (value) {
      this.setAttribute('selectable', '');
    } else {
      this.removeAttribute('selectable');
    }
  }

  get scrollY() {
    return this.getAttribute('scroll-y') || 'auto';
  }

  set scrollY(value) {
    this.setAttribute('scroll-y', value);
  }
  get recycleDataSource() {
    return this.ds || [];
  }

  set recycleDataSource(value) {
    this.isScrollXOutSide = this.tableElement!.scrollWidth > this.tableElement!.clientWidth;
    this.ds = value;
    this.toTop();
    this.currentPage = 0;
    this.pagination = value.length > 0 && Array.isArray(value[0]);
    if (this.pagination) {
      this.currentPageDiv!.textContent = `第 ${this.currentPage + 1} 页，共 ${this.ds.length} 页`;
      this.targetPageInput!.value = '';
    }
    if (this.hasAttribute('tree')) {
      this.recycleDs = this.meauseTreeRowElement(value);
    } else {
      this.recycleDs = this.meauseAllRowHeight(this.pagination ? value[0] : value);
    }
  }

  set pagination(value: boolean) {
    if (value) {
      this.setAttribute('pagination', '');
    } else {
      this.removeAttribute('pagination');
    }
  }

  get pagination() {
    return this.hasAttribute('pagination');
  }

  initElements(): void {
    this.st = this.shadowRoot?.querySelector('#slot');
    this.tableElement = this.shadowRoot?.querySelector('.table');
    this.exportProgress = this.shadowRoot?.querySelector('#export_progress_bar');
    this.theadElement = this.shadowRoot?.querySelector('.thead');
    this.treeElement = this.shadowRoot?.querySelector('.tree');
    this.tbodyElement = this.shadowRoot?.querySelector('.body');
    this.tableColumns = this.querySelectorAll<LitTableColumn>('lit-table-column');
    this.previousDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#previousPage');
    this.nextDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#nextPage');
    this.currentPageDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#currentPage');
    this.targetPageInput = this.shadowRoot?.querySelector<HTMLInputElement>('#targetPage');
    this.jumpDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#jumpPage');
    this.previousDiv!.onclick = () => {
      if (this.currentPage > 0) {
        this.currentPage = Math.max(this.currentPage - 1, 0);
        this.showCurrentPageData();
      }
    };
    this.nextDiv!.onclick = () => {
      if (this.currentPage < this.ds.length - 1) {
        this.currentPage = Math.min(this.currentPage + 1, this.ds.length - 1);
        this.showCurrentPageData();
      }
    };
    this.jumpDiv!.onclick = () => {
      let value = this.targetPageInput!.value;
      let reg = /^[0-9]*$/;
      if (value.length > 0 && reg.test(value)) {
        let target = parseInt(value);
        if (target < 1) {
          target = 1;
        }
        if (target > this.ds.length) {
          target = this.ds.length;
        }
        this.targetPageInput!.value = `${target}`;
        if (this.currentPage !== target - 1) {
          this.currentPage = target - 1;
          this.showCurrentPageData();
        }
      } else {
        this.targetPageInput!.value = '';
      }
    };
  }

  toTop() {
    if (this.rememberScrollTop) {
      this.currentScrollTop = this.tableElement!.scrollTop;
      this.tableElement!.scrollTop = 0;
      this.tableElement!.scrollLeft = 0;
    } else {
      this.tableElement!.scrollTop = 0;
      this.tableElement!.scrollLeft = 0;
    }
  }

  showCurrentPageData() {
    this.toTop();
    this.currentPageDiv!.textContent = `第 ${(this.currentPage || 0) + 1} 页，共 ${this.ds.length} 页`;
    if (this.hasAttribute('tree')) {
      this.recycleDs = this.meauseTreeRowElement(this.ds[this.currentPage]);
    } else {
      this.recycleDs = this.meauseAllRowHeight(this.ds[this.currentPage]);
    }
  }

  initHtml(): string {
    return `
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
            padding-bottom: 30px;
            display: flex;
            flex-direction: row
            row-gap: 1px;
            column-gap: 1px;
        }
        .tbottom{
            height: 30px;
            width: calc(100% - 15px);
            position: absolute;
            bottom: 0;
            z-index: 1;
            justify-content: center;
            align-items: center;
            display: none;
            flex-direction: row;
            color: var(--dark-color1,#000);
            background-color: var(--dark-background,#FFFFFF);
        }
        :host([pagination]) .tbottom{
            display: flex;
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
        .td::-webkit-scrollbar {
          width: 0;
          background-color: transparent;
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
            <div class="tbottom">
                <div id="previousPage" style="cursor: pointer">上一页</div>
                <div id="currentPage" style="margin-left: 10px;margin-right: 10px">1</div>
                <input id="targetPage" style="width: 60px;margin-right: 10px" min="1" type="number"/>
                <div id="jumpPage" style="padding: 2px 10px;border: 1px solid #676767;margin-right: 10px;font-size: 13px;cursor: pointer">GO</div>
                <div id="nextPage" style="cursor: pointer">下一页</div>
            </div>
        </div>
        `;
  }

  move1px() {
    this.tableElement!.scrollTop = this.tableElement!.scrollTop + 1;
  }

  dataExportInit() {
    let exportDiv = this.shadowRoot!.querySelector<HTMLDivElement>('.export');
    exportDiv &&
      (exportDiv.onclick = () => {
        this.exportData();
      });
  }

  exportData() {
    if (this.exportLoading || this.ds.length === 0) {
      return;
    }
    this.exportLoading = true;
    this.exportProgress!.loading = true;
    let date = new Date();
    JSONToCSV.csvExport({
      columns: this.columns as any[],
      tables: this.ds,
      fileName: date.getTime() + '',
      columnFormatter: this.itemTextHandleMap,
      exportFormatter: this.exportTextHandleMap
    }).then((res) => {
      this.exportLoading = false;
      this.exportProgress!.loading = false;
    });
  }

  formatExportData(dataSource: any[]): any[] {
    if (dataSource == undefined || dataSource.length == 0) {
      return [];
    }
    if (this.columns == undefined) {
      return [];
    }
    return dataSource.map((item) => {
      let formatData: any = {};
      this.columns!.forEach((column) => {
        let dataIndex = column.getAttribute('data-index');
        let columnName = column.getAttribute('title');
        if (columnName == '') {
          columnName = dataIndex;
        }
        if (dataIndex && columnName && item[dataIndex] != undefined) {
          formatData[columnName] = item[dataIndex];
        }
      });
      if (item.children != undefined) {
        formatData.children = this.formatExportData(item.children);
      }
      return formatData;
    });
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
      if (item.children != undefined) {
        concatStr += this.recursionExportTableData(columns, item.children);
      }
      if (index != dataSource.length - 1) {
        concatStr += '\r\n';
      }
    });
    return concatStr;
  }

  private isResize: boolean = false;
  private resizeColumnIndex: number = -1;
  private resizeDownX: number = 0;
  private columnMinWidth: number = 50;
  private beforeResizeWidth1: number = 0;
  private beforeResizeWidth2: number = 0;
  resizeEventHandler(header: HTMLDivElement, element: HTMLDivElement, index: number) {
    header.addEventListener('mousemove', (event) => {
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
          tr.style.gridTemplateColumns = this.gridTemplateColumns.join(' ');
        });
        event.preventDefault();
        event.stopPropagation();
      } else {
        header.style.cursor = 'pointer';
      }
    });
    header.addEventListener('mouseup', (event) => {
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
      event.stopPropagation();
      event.preventDefault();
      this.isResize = false;
      this.resizeDownX = 0;
      this.resizeColumnIndex = -1;
      header.style.cursor = 'pointer';
    });
    element.addEventListener('mousedown', (event) => {
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
  disconnectedCallback() {}

  // It is called when the custom element is moved to a new document.
  adoptedCallback() {}

  // It is called when a custom element adds, deletes, or modifies its own properties.
  attributeChangedCallback(name: string, oldValue: string, newValue: string) {}

  fixed(td: HTMLElement, placement: string, bgColor: string) {
    td.style.position = 'sticky';
    if (placement === 'left') {
      td.style.left = '0px';
      td.style.boxShadow = '3px 0px 5px #33333333';
    } else if (placement === 'right') {
      td.style.right = '0px';
      td.style.boxShadow = '-3px 0px 5px #33333333';
    }
  }

  getCheckRows() {
    // @ts-ignore
    return [...this.shadowRoot!.querySelectorAll('div[class=tr][checked]')]
      .map((a) => (a as any).data)
      .map((a) => {
        delete a['children'];
        return a;
      });
  }

  deleteRowsCondition(fn: any) {
    this.shadowRoot!.querySelectorAll('div[class=tr]').forEach((tr) => {
      // @ts-ignore
      if (fn(tr.data)) {
        tr.remove();
      }
    });
  }

  meauseElementHeight(rowData: any) {
    return 27;
  }

  meauseTreeElementHeight(rowData: any, depth: number) {
    return 27;
  }

  meauseAllRowHeight(list: any[]): TableRowObject[] {
    this.tbodyElement!.innerHTML = '';
    this.meauseRowElement = undefined;
    this.startSkip = 0;
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
        while (reduce <= this.tableElement!.clientHeight && (this.currentRecycleList.length + skip) < visibleObjects.length) {
          let newTableElement = this.createNewTableElement(visibleObjects[skip]);
          this.tbodyElement?.append(newTableElement);
          this.currentRecycleList.push(newTableElement);
          reduce += newTableElement.clientHeight;
        }
        this.startSkip = skip;
        for (let i = 0; i < this.currentRecycleList.length; i++) {
          this.freshCurrentLine(this.currentRecycleList[i], visibleObjects[i + skip]);
        }
      });
    return visibleObjects;
  }

  createBtn(rowData: any) {
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

  createExpandBtn(rowData: any) {
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

      if (rowData.expanded) {
        rowData.expanded = false;
        resetNodeHidden(true, rowData);
      } else {
        rowData.expanded = true;
        resetNodeHidden(false, rowData);
      }
      this.reMeauseHeight();
      e.stopPropagation();
    };
    return btn;
  }

  reMeauseHeight() {
    if (this.currentRecycleList.length == 0) {
      return;
    }
    let totalHeight = 0;
    this.recycleDs.forEach((it) => {
      if (!it.rowHidden) {
        it.top = totalHeight;
        totalHeight += it.height;
      }
    });
    this.tbodyElement && (this.tbodyElement.style.height = totalHeight + (this.isScrollXOutSide ? 0 : 0) + 'px');
    this.treeElement!.style.height = this.tableElement!.clientHeight - this.theadElement!.clientHeight + 'px';
    let visibleObjects = this.recycleDs.filter((item) => {
      return !item.rowHidden;
    });
    let top = this.tableElement!.scrollTop;
    let skip = 0;
    for (let i = 0; i < visibleObjects.length; i++) {
      if (visibleObjects[i].top <= top && visibleObjects[i].top + visibleObjects[i].height >= top) {
        skip = i;
        break;
      }
    }
    let reduce = this.currentRecycleList.map((item) => item.clientHeight).reduce((a, b) => a + b, 0);
    if (reduce == 0) {
      return;
    }
    while (reduce <= this.tableElement!.clientHeight) {
      let newTableElement;
      if (this.hasAttribute('tree')) {
        newTableElement = this.createNewTreeTableElement(visibleObjects[skip]);
      } else {
        newTableElement = this.createNewTableElement(visibleObjects[skip]);
      }
      this.tbodyElement?.append(newTableElement);
      if (this.hasAttribute('tree')) {
        if (this.treeElement?.lastChild) {
          (this.treeElement?.lastChild as HTMLElement).style.height = visibleObjects[skip].height + 'px';
        }
      }
      this.currentRecycleList.push(newTableElement);
      reduce += newTableElement.clientHeight;
    }
    for (let i = 0; i < this.currentRecycleList.length; i++) {
      if (this.hasAttribute('tree')) {
        this.freshCurrentLine(
          this.currentRecycleList[i],
          visibleObjects[i + skip],
          this.treeElement?.children[i] as HTMLElement
        );
      } else {
        this.freshCurrentLine(this.currentRecycleList[i], visibleObjects[i + skip]);
      }
    }
  }

  createNewTableElement(rowData: any): any {
    let newTableElement = document.createElement('div');
    newTableElement.classList.add('tr');
    this?.columns?.forEach((column: any) => {
      let dataIndex = column.getAttribute('data-index') || '1';
      let td: any;
      td = document.createElement('div');
      td.classList.add('td');
      td.style.overflow = 'scroll hidden';
      td.style.textOverflow = 'ellipsis';
      td.style.whiteSpace = 'nowrap';
      td.dataIndex = dataIndex;
      td.style.justifyContent = column.getAttribute('align') || 'flex-start';
      let text = this.formatName(dataIndex, rowData.data[dataIndex]);
      td.title = text;
      if (column.template) {
        td.appendChild(column.template.render(rowData.data).content.cloneNode(true));
        td.template = column.template;
      } else {
        td.innerHTML = text;
      }
      newTableElement.append(td);
    });
    newTableElement.onclick = () => {
      this.dispatchRowClickEvent(rowData, [newTableElement]);
    };
    newTableElement.onmouseover = () => {
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

  setSelectedRow(isSelected: boolean, rows: any[]) {
    if (isSelected) {
      rows.forEach((row) => {
        if (row.classList.contains('mouse-in')) row.classList.remove('mouse-in');
        row.classList.add('mouse-select');
      });
    } else {
      rows.forEach((row) => {
        row.classList.remove('mouse-select');
      });
    }
  }

  setMouseIn(isMouseIn: boolean, rows: any[]) {
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

  scrollToData(data: any) {
    if (this.recycleDs.length > 0) {
      let filter = this.recycleDs.filter((item) => {
        return item.data == data;
      });
      if (filter.length > 0) {
        this.tableElement!.scrollTop = filter[0].top;
      }
      this.setCurrentSelection(data);
    }
  }

  expandList(datasource: any[]) {
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

  clearAllSelection(rowObjectData: any = undefined) {
    this.recycleDs.forEach((item) => {
      if (rowObjectData || (item.data != rowObjectData && item.data.isSelected)) {
        item.data.isSelected = false;
      }
    });
    this.setSelectedRow(false, this.currentTreeDivList);
    this.setSelectedRow(false, this.currentRecycleList);
  }

  clearAllHover(rowObjectData: any) {
    this.recycleDs.forEach((item) => {
      if (item.data != rowObjectData && item.data.isHover) {
        item.data.isHover = false;
      }
    });
    this.setMouseIn(false, this.currentTreeDivList);
    this.setMouseIn(false, this.currentRecycleList);
  }

  mouseOut() {
    this.recycleDs.forEach((item) => (item.data.isHover = false));
    this.setMouseIn(false, this.currentTreeDivList);
    this.setMouseIn(false, this.currentRecycleList);
    this.dispatchEvent(
      new CustomEvent('row-hover', {
        detail: {
          data: undefined,
        },
        composed: true,
      })
    );
  }

  setCurrentSelection(data: any) {
    if (data.isSelected != undefined) {
      this.currentTreeDivList.forEach((item) => {
        if ((item as any).data == data) {
          this.setSelectedRow(data.isSelected, [item]);
        }
      });
      this.currentRecycleList.forEach((item) => {
        if ((item as any).data == data) {
          this.setSelectedRow(data.isSelected, [item]);
        }
      });
    }
  }

  setCurrentHover(data: any) {
    this.setMouseIn(false, this.currentTreeDivList);
    this.setMouseIn(false, this.currentRecycleList);
    if (data.isHover != undefined) {
      this.currentTreeDivList.forEach((item) => {
        if ((item as any).data == data) {
          this.setMouseIn(data.isHover, [item]);
        }
      });
      this.currentRecycleList.forEach((item) => {
        if ((item as any).data == data) {
          this.setMouseIn(data.isHover, [item]);
        }
      });
    }
  }

  dispatchRowClickEventIcon(rowObject: any, elements: any[]) {
    this.dispatchEvent(
      new CustomEvent('icon-click', {
        detail: {
          ...rowObject.data,
          data: rowObject.data,
          callBack: (isSelected: boolean) => {
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

  dispatchRowClickEvent(rowObject: any, elements: any[]) {
    this.dispatchEvent(
      new CustomEvent('row-click', {
        detail: {
          ...rowObject.data,
          data: rowObject.data,
          callBack: (isSelected: boolean) => {
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

  dispatchRowHoverEvent(rowObject: any, elements: any[]) {
    this.dispatchEvent(
      new CustomEvent('row-hover', {
        detail: {
          data: rowObject.data,
          callBack: () => {
            this.clearAllHover(rowObject.data);
            this.setMouseIn(rowObject.data.isHover, elements);
          },
        },
        composed: true,
      })
    );
  }

  formatName(key: string, name: any) {
    let content = name;
    if (this.itemTextHandleMap.has(key)) {
      content = this.itemTextHandleMap.get(key)?.(name) || '';
    }
    if (content !== undefined && content !== null) {
      return content.toString().replace(/</g, '&lt;').replace(/>/g, '&gt;');
    }
    return '';
  }

  setHighLight(isSearch: boolean, element: any) {
    if (isSearch) {
      element.setAttribute('high-light', '');
    } else {
      element.removeAttribute('high-light');
    }
  }
}

if (!customElements.get('lit-page-table')) {
  customElements.define('lit-page-table', LitPageTable);
}
