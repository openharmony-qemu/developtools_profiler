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
import { queryCustomizeSelect, querySelectTraceStats } from '../database/SqlLite.js';
import { LitTable } from '../../base-ui/table/lit-table.js';
import '../../base-ui/table/lit-table.js';
import { LitTableColumn } from '../../base-ui/table/lit-table-column.js';
import { info } from '../../log/Log.js';
import { LitProgressBar } from '../../base-ui/progress-bar/LitProgressBar.js';
import { PageNation } from '../../base-ui/chart/pagenation/PageNation.js';
import { PaginationBox } from '../../base-ui/chart/pagenation/pagination-box.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';

@element('sp-query-sql')
export class SpQuerySQL extends BaseElement {
  private queryTableEl: LitTable | undefined;
  private queryText: string | undefined;
  private resultText: string | undefined;
  private notSupportList: Array<string> | undefined;
  private querySize: HTMLElement | undefined;
  private keyList: Array<string> | undefined;
  private selector: HTMLTextAreaElement | undefined;
  private isSupportSql: boolean = true;
  private querySelectTables: string = '';
  private response: HTMLDivElement | undefined;
  private statDataArray: any = [];
  private sliceData: any = [];
  private querySqlErrorText: string = '';
  private progressLoad: LitProgressBar | undefined;
  private pagination: PaginationBox | undefined;
  private pageSize: number = 200000;
  private maxPageSize: number = 500000;

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
      this.querySize!.textContent = 'Query result - 0 counts';
      this.queryTableEl!.dataSource = [];
      this.response!.innerHTML = '';
    });
  }

  freshTableHeadResizeStyle(): void {
    let th = this.queryTableEl!.shadowRoot?.querySelector<HTMLDivElement>('.th');
    if (th) {
      let td = th.querySelectorAll<HTMLDivElement>('.td');
      let firstChild = this.queryTableEl!.shadowRoot?.querySelector<HTMLDivElement>('.body')!.firstElementChild;
      if (firstChild) {
        let bodyList = firstChild.querySelectorAll<HTMLDivElement>('.td');
        for (let index = 0 ; index < bodyList.length ; index++) {
          td[index].style.width = bodyList[index].offsetWidth + 'px';
          td[index].style.overflow = 'hidden';
        }
      }
    }
  }

  async copyTableData(): Promise<void> {
    let copyResult = '';
    for (let keyListKey of this.keyList!) {
      copyResult += keyListKey + '\t';
    }
    copyResult += '\n';
    let copyData = [];
    if (this.statDataArray.length > this.maxPageSize) {
      copyData = this.sliceData;
    } else {
      copyData = this.statDataArray;
    }
    for (const value of copyData) {
      this.keyList?.forEach((key) => {
        copyResult += value[key] + '\t';
      });
      copyResult += '\n';
    }
    await navigator.clipboard.writeText(copyResult);
  }

  selectEventListener = (event: KeyboardEvent): void => {
    let that = this;
    if (event.ctrlKey && event.keyCode == 13) {
      SpStatisticsHttpUtil.addOrdinaryVisitAction({
        event: 'query',
        action: 'query',
      });
      if (!this.isSupportSql) {
        this.querySize!.textContent = this.querySqlErrorText;
        this.queryTableEl!.dataSource = [];
        this.response!.innerHTML = '';
        return;
      }
      this.progressLoad!.loading = true;
      let startData = new Date().getTime();
      this.getInputSqlResult(this.selector!.value).then((resultList) => {
        let dur = new Date().getTime() - startData;
        this.statDataArray = [];
        this.keyList = [];
        for (let index = 0 ; index < resultList.length ; index++) {
          const dataResult = resultList[index];
          let keys = Object.keys(dataResult);
          // @ts-ignore
          let values = Object.values(dataResult);
          let jsonText = '{';
          for (let keyIndex = 0 ; keyIndex < keys.length ; keyIndex++) {
            let key = keys[keyIndex];
            if (this.keyList.indexOf(key) <= -1) {
              this.keyList.push(key);
            }
            let value = values[keyIndex];
            if (typeof value == 'string') {
              value = value.replace(/</gi, '&lt;').replace(/>/gi, '&gt;');
            }
            jsonText += '"' + key + '"' + ': ' + '"' + value + '"';
            if (keyIndex != keys.length - 1) {
              jsonText += ',';
            } else {
              jsonText += '}';
            }
          }
          this.statDataArray.push(JSON.parse(jsonText));
        }

        this.queryTableEl!.innerHTML = '';
        this.queryText = this.selector!.value;
        this.initDataElement();
        this.response!.appendChild(this.queryTableEl!);
        setTimeout(() => {
          let total = this.statDataArray.length;
          if (total > this.maxPageSize) {
            that.pagination!.style.opacity = '1';
            new PageNation(this.pagination, {
              current: 1,
              total: total,
              pageSize: this.pageSize,
              change(num: number): void {
                that.sliceData = that.statDataArray!.slice((num - 1) * that.pageSize, num * that.pageSize);
                that.queryTableEl!.recycleDataSource = that.sliceData;
              },
            });
          } else {
            that.pagination!.style.opacity = '0';
            this.queryTableEl!.recycleDataSource = this.statDataArray;
          }

          this.freshTableHeadResizeStyle();
          new ResizeObserver(() => {
            if (this.parentElement?.clientHeight != 0) {
              this.queryTableEl!.style.height = '100%';
              this.queryTableEl!.reMeauseHeight();
            }
          }).observe(this.parentElement!);
          info('metric query Sql result Data size is: ', this.statDataArray!.length);
          this.initData();
          this.progressLoad!.loading = false;
        }, 200);
      });
    }
  };

  reset(): void {
    this.pagination!.style.opacity = '0';
    this.response!.innerHTML = '';
    this.keyList = [];
    this.statDataArray = [];
    this.selector!.value = '';
    this.querySize!.textContent = 'Query result - ' + ' 0 counts';
    this.resizeSqlHeight().then(() => {});
  }

  initDataTableStyle(styleTable: HTMLDivElement): void {
    for (let index = 0 ; index < styleTable.children.length ; index++) {
      // @ts-ignore
      styleTable.children[index].style.backgroundColor = 'var(--dark-background5,#F6F6F6)';
    }
  }

  async initMetricData(): Promise<any> {
    if (!this.selector || this.selector.value == null) {
      return [];
    }
    if (this.queryText == '' || this.queryText == null) {
      let statList = await querySelectTraceStats();
      for (let index = 0 ; index < statList.length ; index++) {
        const statsResult = statList[index];
        let indexArray = {
          event_name: statsResult.event_name,
          start_type: statsResult.stat_type,
          count: statsResult.count,
          serverity: statsResult.serverity,
          source: statsResult.source,
        };
      }
      if (this.querySize) {
        this.querySize!.textContent = 'Query result - ' + statList.length + ' counts';
      }
      this.resultText = 'Query result - ' + statList.length + ' counts';
    } else {
      return this.statDataArray;
    }
  }

  checkSupportSqlAbility(): boolean {
    let noSupportChart = ['insert', 'delete', 'update', 'drop', 'alter', 'truncate', 'create'];
    let result = noSupportChart.filter((item) => {
      return this.selector!.value.indexOf(item) > -1;
    });
    if (result.length > 0) {
      this.querySqlErrorText =
        'Error: Statement contains a change action keyword,The change operation is not supported.';
      this.isSupportSql = false;
      return true;
    } else {
      return false;
    }
  }

  checkSafetySelectSql(): boolean {
    let split = this.selector?.value.trim().split(' ');
    if (split) {
      this.querySqlErrorText = 'Error: Incomplete query statement:  ' + this.selector!.value;
      this.isSupportSql = false;
      return !split[0].toLowerCase().startsWith('select');
    }
    return false;
  }

  getSelectSqlField(): string {
    if (this.selector!.value.indexOf('from') < 0) {
      return '';
    }
    let splitSql = this.selector?.value.split('from');
    if (splitSql) {
      if (splitSql[0].indexOf('*') > -1) {
        return '*';
      } else {
        let fields = splitSql[0].split(',');
        return fields[0];
      }
    }
    return '';
  }

  getSelectSqlTableName(str: string): Array<string> {
    if (this.selector!.value.indexOf(str) < 0) {
      return [];
    }
    let tableNameList = [];
    let splitSql = this.selector?.value.split(str);
    if (splitSql) {
      for (let index = 1 ; index < splitSql?.length ; index++) {
        let splitSqlItem = splitSql[index].trim();
        let tableItem = splitSqlItem.split(' ');
        let tableName = tableItem[0].trim();
        tableNameList.push(tableName);
        if (tableName.indexOf('(') >= 0) {
          tableNameList.pop();
        } else if (tableName.indexOf(')') >= 0) {
          tableNameList.pop();
          let unitTableName = tableName.split(')');
          let tableNewName = unitTableName[0];
          tableNameList.push(tableNewName);
        }
      }
    }
    return tableNameList;
  }

  initDataElement(): void {
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
    let selectQuery = this.shadowRoot?.querySelector('.query_select');
    if (selectQuery) {
      let querySql = selectQuery.textContent;
    }
    // Listen to the sql execution of the query
    this.addEventListener('keydown', this.selectEventListener);
    this.selector!.addEventListener('input', this.inputSqlListener);
    this.selector!.addEventListener('change', this.inputSqlListener);
    this.selector!.addEventListener('keydown', this.deleteSqlListener);
  }

  deleteSqlListener = (event: KeyboardEvent): void => {
    if (event.key == 'Backspace') {
      this.resizeSqlHeight().then(() => {});
    }
  };

  async resizeSqlHeight(): Promise<void> {
    let valueLength = this.selector?.value.split('\n').length;
    let rowNumber = Number(valueLength) - 1;
    let selectHeight = '3.2em';
    if (rowNumber > 0) {
      if (rowNumber <= 10) {
        let allLength = 1.2 * rowNumber + 2;
        selectHeight = allLength + 'em';
      } else {
        selectHeight = '14em';
      }
    }
    // @ts-ignore
    this.selector?.style.height = selectHeight;
  }

  inputSqlListener = async (event: Event): Promise<void> => {
    this.resizeSqlHeight().then(() => {});
    let startData = new Date().getTime();
    if (this.selector!.value.trim() == '') {
      this.querySqlErrorText = 'Please enter a query';
      this.querySize!.textContent = this.querySqlErrorText;
      return;
    }
    this.checkSafetySelectSql();
    this.checkSupportSqlAbility();
    if (this.selector!.value.length < 15) {
      return;
    }
    this.querySelectTables = this.getSelectSqlTableName('from')
                                 .concat(this.getSelectSqlTableName('join'))
                                 .toLocaleString();
    info('metric query sql table size is: ', this.querySelectTables.length);
    this.isSupportSql = true;
  };

  async getInputSqlResult(sql: string): Promise<any> {
    return await queryCustomizeSelect(sql);
  }

  disconnectedCallback(): void {
    this.removeEventListener('keydown', this.selectEventListener);
    this.selector!.removeEventListener('input', this.inputSqlListener);
    this.selector!.removeEventListener('change', this.inputSqlListener);
    this.selector!.removeEventListener('keydown', this.deleteSqlListener);
  }

  initData(): void {
    if (this.statDataArray.length > 0) {
      this.querySize!.textContent = 'Error: ' + this.selector?.value;
    }
    if (this.isSupportSql) {
      let sqlField = this.keyList?.length == 0 ? '*' : this.keyList?.toLocaleString();
      this.querySize!.textContent = 'Query result - ' + this.statDataArray.length + ' counts';
    } else {
      this.querySize!.textContent = this.querySqlErrorText;
    }

    let queryHeadStyle: HTMLDivElement | undefined | null = this.queryTableEl?.shadowRoot?.querySelector(
      'div.th'
    ) as HTMLDivElement;
    if (queryHeadStyle && queryHeadStyle.hasChildNodes()) {
      for (let index = 0 ; index < queryHeadStyle.children.length ; index++) {
        // @ts-ignore
        queryHeadStyle.children[index].style.gridArea = null;
      }
    }

    this.queryTableEl!.style.height = '100%';
  }

  static get observedAttributes(): string[] {
    return ['queryStr'];
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {
    let queryDataSty: HTMLDivElement | undefined | null = this.queryTableEl?.shadowRoot?.querySelector(
      'div.tbody'
    ) as HTMLDivElement;
    if (queryDataSty && queryDataSty.hasChildNodes()) {
      for (let index = 0 ; index < queryDataSty.children.length ; index++) {
        // @ts-ignore
        queryDataSty.children[index].style.backgroundColor = 'var(--dark-background5,#F6F6F6)';
      }
    }
  }

  private _queryStr?: string;

  get queryStr(): string {
    return this.queryStr;
  }

  set queryStr(value: string) {
    this._queryStr = value;
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
        ::-webkit-scrollbar
        {
          width: 8px;
          background-color: var(--dark-background3,#FFFFFF);
        }
        ::-webkit-scrollbar-thumb
        {
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
