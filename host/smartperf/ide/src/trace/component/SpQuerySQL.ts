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

import { BaseElement, element } from '../../base-ui/BaseElement.js';
import { queryCustomizeSelect } from '../database/SqlLite.js';
import { LitTable } from '../../base-ui/table/lit-table.js';
import '../../base-ui/table/lit-table.js';
import { LitTableColumn } from '../../base-ui/table/lit-table-column.js';
import { info } from '../../log/Log.js';
import { LitProgressBar } from '../../base-ui/progress-bar/LitProgressBar.js';
import { PageNation } from '../../base-ui/chart/pagenation/PageNation.js';
import { PaginationBox } from '../../base-ui/chart/pagenation/PaginationBox.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';

@element('sp-query-sql')
export class SpQuerySQL extends BaseElement {
  private queryTableEl: LitTable | undefined;
  private notSupportList: Array<string> | undefined = [];
  private querySize: HTMLElement | undefined;
  private keyList: Array<string> | undefined;
  private selector: HTMLTextAreaElement | undefined;
  private isSupportSql: boolean = true;
  private response: HTMLDivElement | undefined;
  private statDataArray: unknown[] = [];
  private sliceData: unknown[] = [];
  private querySqlErrorText: string = '';
  private progressLoad: LitProgressBar | undefined;
  private pagination: PaginationBox | undefined;

  initElements(): void {
    this.progressLoad = this.shadowRoot?.querySelector('.load-query-sql') as LitProgressBar;
    this.selector = this.shadowRoot?.querySelector('.sql-select') as HTMLTextAreaElement;
    this.queryTableEl = new LitTable();
    this.querySize = this.shadowRoot?.querySelector('.query_size') as HTMLElement;
    this.response = this.shadowRoot?.querySelector('#dataResult') as HTMLDivElement;
    this.pagination = this.shadowRoot?.querySelector('.pagination-box') as PaginationBox;
    this.notSupportList?.push('insert', 'delete', 'update', 'drop', 'alter', 'truncate', 'create');
    let htmlDivElement = this.queryTableEl.shadowRoot?.querySelector('.table') as HTMLDivElement;
    htmlDivElement.style.overflowX = 'scroll';
    window.addEventListener('resize', () => {
      this.freshTableHeadResizeStyle();
    });
    let copyButtonEl = this.shadowRoot?.querySelector('#copy-button') as HTMLButtonElement;
    copyButtonEl.addEventListener('click', () => {
      this.copyTableData();
    });
    let closeButtonEl = this.shadowRoot?.querySelector('#close-button') as HTMLButtonElement;
    closeButtonEl.addEventListener('click', () => {
      this.querySize!.textContent = 'Query result - 0 counts.';
      this.queryTableEl!.dataSource = [];
      this.response!.innerHTML = '';
    });
    new ResizeObserver(() => {
      if (this.parentElement?.clientHeight !== 0) {
        this.queryTableEl!.style.height = '100%';
        this.queryTableEl!.reMeauseHeight();
      }
    }).observe(this.parentElement!);
  }

  private freshTableHeadResizeStyle(): void {
    let th = this.queryTableEl!.shadowRoot?.querySelector<HTMLDivElement>('.th');
    if (th) {
      let td = th.querySelectorAll<HTMLDivElement>('.td');
      let firstChild = this.queryTableEl!.shadowRoot?.querySelector<HTMLDivElement>('.body')!.firstElementChild;
      if (firstChild) {
        let bodyList = firstChild.querySelectorAll<HTMLDivElement>('.td');
        for (let index = 0; index < bodyList.length; index++) {
          td[index].style.width = `${bodyList[index].offsetWidth}px`;
          td[index].style.overflow = 'hidden';
        }
      }
    }
    let tableHeadStyle: HTMLDivElement | undefined | null = this.queryTableEl?.shadowRoot?.querySelector(
      'div.th'
    ) as HTMLDivElement;
    if (tableHeadStyle && tableHeadStyle.hasChildNodes()) {
      for (let index = 0; index < tableHeadStyle.children.length; index++) {
        // @ts-ignore
        tableHeadStyle.children[index].style.gridArea = null;
      }
    }
    this.queryTableEl!.style.height = '100%';
  }

  private async copyTableData(): Promise<void> {
    let copyResult = '';
    for (let keyListKey of this.keyList!) {
      copyResult += `${keyListKey}\t`;
    }
    copyResult += '\n';
    let copyData: unknown[];
    if (this.statDataArray.length > maxPageSize) {
      copyData = this.sliceData;
    } else {
      copyData = this.statDataArray;
    }
    for (const value of copyData) {
      this.keyList?.forEach((key) => {
        // @ts-ignore
        copyResult += `${value[key]}\t`;
      });
      copyResult += '\n';
    }
    await navigator.clipboard.writeText(copyResult);
  }

  selectEventListener = (event: KeyboardEvent): void => {
    let enterKey = 13;
    if (event.ctrlKey && event.keyCode === enterKey) {
      SpStatisticsHttpUtil.addOrdinaryVisitAction({
        event: 'query',
        action: 'query',
      });
      this.statDataArray = [];
      this.keyList = [];
      this.response!.innerHTML = '';
      this.queryTableEl!.innerHTML = '';
      if (this.isSupportSql) {
        this.progressLoad!.loading = true;
        queryCustomizeSelect(this.selector!.value).then((resultList): void => {
          if (resultList && resultList.length > 0) {
            this.statDataArray = resultList;
            this.keyList = Object.keys(resultList[0]);
            this.querySize!.textContent = `Query result - ${this.statDataArray.length} counts.`;
            this.initDataElement();
            this.response!.appendChild(this.queryTableEl!);
            this.setPageNationTableEl();
          } else {
            this.querySize!.textContent = `Query result - ${this.statDataArray.length} counts.`;
            this.progressLoad!.loading = false;
          }
        });
      } else {
        this.querySize!.textContent = this.querySqlErrorText;
        this.queryTableEl!.dataSource = [];
        this.response!.innerHTML = '';
        return;
      }
    }
  };

  private setPageNationTableEl(): void {
    let that = this;
    let timeOutTs: number = 200;
    let indexNumber = 1;
    setTimeout(() => {
      let total = this.statDataArray.length;
      if (total > maxPageSize) {
        that.pagination!.style.opacity = '1';
        new PageNation(this.pagination, {
          current: 1,
          total: total,
          pageSize: pageSize,
          change(num: number): void {
            that.sliceData = that.statDataArray!.slice((num - indexNumber) * pageSize, num * pageSize);
            that.queryTableEl!.recycleDataSource = that.sliceData;
          },
        });
      } else {
        that.pagination!.style.opacity = '0';
        this.queryTableEl!.recycleDataSource = this.statDataArray;
      }
      this.freshTableHeadResizeStyle();
      this.progressLoad!.loading = false;
    }, timeOutTs);
  }

  reset(): void {
    this.pagination!.style.opacity = '0';
    this.response!.innerHTML = '';
    this.keyList = [];
    this.statDataArray = [];
    this.selector!.value = '';
    this.querySize!.textContent = 'Please enter a query.';
    this.resizeSqlHeight().then();
  }

  private checkSafetySelectSql(): boolean {
    if (this.selector?.value.trim() === '') {
      this.querySqlErrorText = 'Please enter a query.';
      this.querySize!.textContent = this.querySqlErrorText;
      return false;
    } else {
      let queryNormalLength = 15;
      if (this.selector!.value.length < queryNormalLength ||
        !this.selector?.value.toLowerCase().trim().startsWith('select')
      ) {
        this.querySqlErrorText = `Query result - (Error):  
        ${this.selector!.value}.`;
        return false;
      }
      if (this.notSupportList && this.notSupportList.length > 0) {
        for (let index = 0; index < this.notSupportList.length; index++) {
          let regexStr = new RegExp(this.notSupportList[index], 'i');
          if (regexStr.test(this.selector!.value)) {
            this.querySqlErrorText = `Query result - (Error):  
            ${this.selector!.value}.`;
            return false;
          }
        }
      }
    }
    return true;
  }

  private initDataElement(): void {
    if (this.keyList) {
      info('Metric query Table Colum size is: ', this.keyList.length);
      this.keyList.forEach((item) => {
        let htmlElement = document.createElement('lit-table-column') as LitTableColumn;
        htmlElement.setAttribute('title', item);
        htmlElement.setAttribute('data-index', item);
        htmlElement.setAttribute('key', item);
        htmlElement.setAttribute('align', 'flex-start');
        htmlElement.setAttribute('height', '32px');
        this.queryTableEl!.appendChild(htmlElement);
      });
    }
  }

  connectedCallback(): void {
    // Listen to the sql execution of the query
    this.addEventListener('keydown', this.selectEventListener);
    this.selector!.addEventListener('input', this.inputSqlListener);
    this.selector!.addEventListener('change', this.inputSqlListener);
    this.selector!.addEventListener('keydown', this.deleteSqlListener);
  }

  private deleteSqlListener = (event: KeyboardEvent): void => {
    if (event.key === 'Backspace') {
      this.resizeSqlHeight().then();
      this.isSupportSql = this.checkSafetySelectSql();
    }
  };

  private async resizeSqlHeight(): Promise<void> {
    let minRowNumber = 10;
    let indexNumber = 1;
    let multipleNumber = 1.2;
    let paddingNumber = 2;
    let valueLength = this.selector?.value.split('\n').length;
    let rowNumber = Number(valueLength) - indexNumber;
    let selectHeight = '3.2em';
    if (rowNumber > 0) {
      if (rowNumber <= minRowNumber) {
        let allLength = multipleNumber * rowNumber + paddingNumber;
        selectHeight = `${allLength}em`;
      } else {
        selectHeight = '14em';
      }
    }
    // @ts-ignore
    this.selector?.style.height = selectHeight;
  }

  private inputSqlListener = async (): Promise<void> => {
    this.resizeSqlHeight().then();
    this.isSupportSql = this.checkSafetySelectSql();
  };

  disconnectedCallback(): void {
    this.removeEventListener('keydown', this.selectEventListener);
    this.selector!.removeEventListener('input', this.inputSqlListener);
    this.selector!.removeEventListener('change', this.inputSqlListener);
    this.selector!.removeEventListener('keydown', this.deleteSqlListener);
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {
    let queryDataSty: HTMLDivElement | undefined | null = this.queryTableEl?.shadowRoot?.querySelector(
      'div.tbody'
    ) as HTMLDivElement;
    if (queryDataSty && queryDataSty.hasChildNodes()) {
      for (let index = 0; index < queryDataSty.children.length; index++) {
        // @ts-ignore
        queryDataSty.children[index].style.backgroundColor = 'var(--dark-background5,#F6F6F6)';
      }
    }
  }

  initHtml(): string {
    return `
        <style>
        :host{
          width: 100%;
          height: 100%;
          font-size: 16px;
          background-color: var(--dark-background5,#F6F6F6);
          margin: 0;
          padding: 0;
        }
        .sql-select{
          box-sizing: border-box;
          width: 95%;
          font-family: Helvetica,serif;
          font-size: inherit;
          color: var(--dark-color1,#212121);
          text-align: left;
          line-height: 1.2em;
          font-weight: 400;
          height: 3.2em;
          margin-left: 10px;
          resize: vertical;
          border-width: 2px;
        }
        .query{
          display: flex;
          flex-direction: column;
          background-color: var(--dark-background5,#F6F6F6);
          position: absolute;
          top: 0;
          bottom: 0;
          left: 0;
          right: 0;
        }
        .query-message{
          background-color: var(--dark-background3,#FFFFFF);
          padding: 1% 2%;
          margin: 2% 2.5% 0 2.5%;
          border-radius: 16px;
          width: 90%;
        }
        .request{
          display: flex;
          flex-direction: column;
          position: relative;
        }
        .response{
          flex-grow: 1;
          margin-bottom: 1%;
          display: flex;
          flex-direction: column;
          min-height: inherit;
          max-height: 70vh;
        }
        #dataResult{
          flex-grow: 1;
          overflow-y: auto;
          overflow-x: visible;
          margin-bottom: 1%;
          border-radius: 16px;
        }
        p{
          display: table-cell;
          padding: 7px 10px;
          font-size:0.875em;
          line-height: 20px;
          font-weight: 400;
          text-align: left;
        }
        #response-json{
          margin-top: 20px;
          background-color: var(--dark-background5,#F6F6F6);
          margin-left: 10px;
          flex-grow: 1;
          scroll-y: visible;
        }
        .sql-select{
          background-color: var(--dark-background5, #F6F6F6);
        }
        ::-webkit-scrollbar{
          width: 8px;
          background-color: var(--dark-background3,#FFFFFF);
        }
        ::-webkit-scrollbar-thumb{
          border-radius: 6px;
          background-color: var(--dark-background7,rgba(0,0,0,0.1));
        }
        .load-query-sql{
          width: 95%;
          bottom: 0;
        }
        #copy-button{
          margin-right: 10%;
          cursor:pointer;
          opacity: 0.6;
        }
        #close-button{
          margin-right: 5%;
          cursor:pointer;
          opacity: 0.6;
        }
        .button-option{
          border-radius: 15px;
          background-color: #0A59F7;
          width: 120px;
          height: 25px;
          font-family: Helvetica-Bold;
          color: var(--dark-background3,#FFFFFF);
          text-align: center;
          line-height: 20px;
          font-weight: 400;
          border:0 solid;
        }
        .pagination-box {
          opacity: 0;
        }
        </style>
        <div class="query">
            <div class="query-message request">
                <p class="query_select" style="color: #999999">Enter query and press cmd/ctrl + Enter</p>
                <textarea class="sql-select"></textarea>
                <lit-progress-bar class="load-query-sql"></lit-progress-bar>
            </div>
            <div class="query-message response">
                   <div style="display: flex;justify-content: space-between">
                       <p class="query_size" style="color: #999999">Query result - 0 counts</p>
                       <div style="display: flex; align-items: center">
                           <button id="copy-button" class="button-option">Copy as.tsv</button>
                           <button id="close-button" class="button-option">Close</button>
                        </div>
                    </div>
                   <div id="dataResult"></div>
                   <pagination-box class="pagination-box"></pagination-box>
            </div>
        </div>
        `;
  }
}

const pageSize: number = 200000;
const maxPageSize: number = 500000;
