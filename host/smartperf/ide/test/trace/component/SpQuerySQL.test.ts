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

// @ts-ignore
import { SpQuerySQL } from '../../../dist/trace/component/SpQuerySQL.js';
// @ts-ignore
import { queryCustomizeSelect } from '../../../dist/trace/database/SqlLite.js';
// @ts-ignore
import { SpStatisticsHttpUtil } from '../../../dist/statistics/util/SpStatisticsHttpUtil.js';
const sqlite = require('../../../dist/trace/database/SqlLite.js');
jest.mock('../../../dist/trace/database/SqlLite.js');

SpStatisticsHttpUtil.addOrdinaryVisitAction = jest.fn(() => true);
describe('SpQuerySQL Test', () => {
  let spQuerySQL = new SpQuerySQL();

  it('SpQuerySQLTest01', function () {
    expect(spQuerySQL.checkSupportSqlAbility()).toBeFalsy();
  });

  it('SpQuerySQLTest02', function () {
    expect(spQuerySQL.checkSafetySelectSql()).toBeTruthy();
  });

  it('SpQuerySQLTest03', function () {
    expect(spQuerySQL.getSelectSqlField()).toBe('');
  });

  it('SpQuerySQLTest04', function () {
    expect(spQuerySQL.getSelectSqlTableName()).not.toBeUndefined();
  });

  it('SpQuerySQLTest05', function () {
    expect(spQuerySQL.initDataElement()).toBeUndefined();
  });

  it('SpQuerySQLTest06', function () {
    spQuerySQL.statDataArray.length = 1;
    expect(spQuerySQL.initData()).toBeUndefined();
  });

  it('SpQuerySQLTest07', function () {
    expect(spQuerySQL.attributeChangedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest08', function () {
    expect(spQuerySQL.initHtml()).toMatchInlineSnapshot(`
"
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
        "
`);
  });

  it('SpQuerySQLTest09', function () {
    expect(
        spQuerySQL.initDataTableStyle({
          children: [
            {
              length: 3,
              style: {
                backgroundColor: 'var(--dark-background5,#F6F6F6)',
              },
            },
          ],
        })
    ).toBeUndefined();
  });

  it('SpQuerySQLTest010', function () {
    expect(spQuerySQL.freshTableHeadResizeStyle()).toBeUndefined();
  });

  it('SpQuerySQLTest011', function () {
    expect(spQuerySQL.reset()).toBeUndefined();
  });

  it('SpQuerySQLTest012', function () {
    let spQuerySQL = new SpQuerySQL();
    expect(
        spQuerySQL.initDataTableStyle({
          children: [
            {
              length: 1,
              style: {
                backgroundColor: 'var(--dark-background5,#F6F6F6)',
              },
            },
          ],
        })
    ).toBeUndefined();
  });

  it('SpQuerySQLTest013', function () {
    expect(spQuerySQL.initDataElement()).toBeUndefined();
  });

  it('SpQuerySQLTest014', function () {
    expect(spQuerySQL.connectedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest015', function () {
    expect(spQuerySQL.disconnectedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest016', function () {
    expect(spQuerySQL.initData()).toBeUndefined();
  });

  it('SpQuerySQLTest017', function () {
    expect(spQuerySQL.attributeChangedCallback('', '', '')).toBeUndefined();
  });

  it('SpQuerySQLTest018', function () {
    document.body.innerHTML = `
         <sp-query-sql id="query-sql"></sp-query-sql>
        `;
    let spQuerySql = document.getElementById('query-sql') as SpQuerySQL;
    spQuerySql.queryStr = 'select * from trace_range';
    let range = sqlite.queryCustomizeSelect;
    let dataTime: Array<any> = [
      {
        start_ts: 1000,
        end_ts: 12000,
      },
    ];
    range.mockResolvedValue(dataTime);
    let keyboardEvent: KeyboardEvent = new KeyboardEvent('keydown', <KeyboardEventInit>{ ctrlKey: true, keyCode: 13 });
    spQuerySql.dispatchEvent(keyboardEvent);
    expect(spQuerySQL.attributeChangedCallback('', '', '')).toBeUndefined();
  });
});
