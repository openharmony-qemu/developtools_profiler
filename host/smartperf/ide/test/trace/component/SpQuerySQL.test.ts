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

window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
  disconnect: jest.fn(),
  observe: jest.fn(),
  unobserve: jest.fn(),
}));

SpStatisticsHttpUtil.addOrdinaryVisitAction = jest.fn(() => true);
describe('SpQuerySQL Test', () => {
  let spQuerySQL = new SpQuerySQL();

  it('SpQuerySQLTest01', function () {
    expect(spQuerySQL.checkSafetySelectSql()).toBeFalsy();
  });

  it('SpQuerySQLTest02', function () {
    expect(spQuerySQL.initDataElement()).toBeUndefined();
  });

  it('SpQuerySQLTest03', function () {
    expect(spQuerySQL.attributeChangedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest005', function () {
    expect(spQuerySQL.freshTableHeadResizeStyle()).toBeUndefined();
  });

  it('SpQuerySQLTest06', function () {
    expect(spQuerySQL.reset()).toBeUndefined();
  });

  it('SpQuerySQLTest007', function () {
    expect(spQuerySQL.initDataElement()).toBeUndefined();
  });

  it('SpQuerySQLTest008', function () {
    expect(spQuerySQL.connectedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest009', function () {
    expect(spQuerySQL.disconnectedCallback()).toBeUndefined();
  });

  it('SpQuerySQLTest010', function () {
    expect(spQuerySQL.attributeChangedCallback('', '', '')).toBeUndefined();
  });

  it('SpQuerySQLTest011', function () {
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
