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
import { TabPanePTS } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPanePTS.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', ()=>{
  return {
    meauseAllRowHeight: ()=>{}
  }
})
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPanePTS Test', () => {
  document.body.innerHTML = `<lit-table id="tb-states"></lit-table>`;
  let tab = document.querySelector('#tb-states') as LitTable;

  document.body.innerHTML = `<div><tabpane-pts class="pts"></tabpane-pts></div>`;
  let tabPane = document.querySelector('.pts') as TabPanePTS;
  let tabPanePTS = new TabPanePTS();

  tabPanePTS.tbl = jest.fn(() => tab);

  SpSystemTrace.SPT_DATA = [
    {
      process: '',
      processId: 0,
      thread: '',
      threadId: 0,
      state: '',
      dur: 0,
      start_ts: 0,
      end_ts: 0,
      cpu: 0,
      priority: '-',
      note: '-',
    },
    {
      process: '',
      processId: 1,
      thread: '',
      threadId: 1,
      state: '',
      dur: 0,
      start_ts: 0,
      end_ts: 0,
      cpu: 0,
      priority: '-',
      note: '-',
    },
    {
      process: '',
      processId: 2,
      thread: '',
      threadId: 2,
      state: '',
      dur: 0,
      start_ts: 0,
      end_ts: 0,
      cpu: 0,
      priority: '-',
      note: '-',
    },
  ];

  let dataArray = [
    {
      id: '',
      pid: '',
      title: '',
      children: [],
      process: '',
      processId: 0,
      thread: '',
      threadId: 0,
      state: '',
      wallDuration: 0,
      avgDuration: '',
      count: 0,
      minDuration: 0,
      maxDuration: 0,
      stdDuration: '',
    },
  ];

  it('TabPanePTSTest01', function () {
    expect(tabPane.getDataBySPT(0, 0, [])).toBeUndefined();
  });

  it('TabPanePTSTest02', function () {
    let source = [
      {
        process: '',
        processId: 10,
        thread: '',
        threadId: 10,
        state: '',
        dur: 1000,
        start_ts: 100_0000,
        end_ts: 0,
        cpu: 0,
        priority: '-',
        note: '-',
      },
    ];
    expect(tabPane.getDataBySPT(10, 100_000, source)).toBeUndefined();
  });

  it('TabPaneSPTTest03', function () {
    let mockgetProcessThreadDataByRange = sqlit.getStatesProcessThreadDataByRange;
    mockgetProcessThreadDataByRange.mockResolvedValue([
      {
        process: 'process',
        processId: 1,
        thread: 'thread',
        state: 'state',
        threadId: 1,
        dur: 1000,
        end_ts: 2000,
        start_ts: 2000,
        cpu: 1111,
      },
    ]);
    tab.recycleDataSource = jest.fn(() => []);
    tabPanePTS.tbl.recycleDataSource = jest.fn(() => dataArray);
    expect((tabPanePTS.data = dataArray)).toBeTruthy();
  });

  it('TabPanePTSTest04', function () {
    expect(tabPanePTS.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="pts-time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="pts-tbl" style="height: auto" tree>
            <lit-table-column class="pts-column" title="Process/Thread/State" data-index="title" key="title" align="flex-start" width="27%">
            </lit-table-column>
            <lit-table-column class="pts-column" title="Count" data-index="count" key="count" align="flex-start" width="1fr">
            </lit-table-column>
            <lit-table-column class="pts-column" title="Duration(ns)" data-index="wallDuration" key="wallDuration" align="flex-start" width="1fr">
            </lit-table-column>
            <lit-table-column class="pts-column" title="Min Duration(ns)" data-index="minDuration" key="minDuration" align="flex-start" width="1fr">
            </lit-table-column>
            <lit-table-column class="pts-column" title="Avg Duration(ns)" data-index="avgDuration" key="avgDuration" align="flex-start" width="1fr">
            </lit-table-column>
            <lit-table-column class="pts-column" title="Max Duration(ns)" data-index="maxDuration" key="maxDuration" align="flex-start" width="1fr">
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
