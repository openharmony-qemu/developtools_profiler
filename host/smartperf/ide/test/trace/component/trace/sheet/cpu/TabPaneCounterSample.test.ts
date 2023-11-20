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
import { TabPaneCounterSample } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCounterSample.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneCounterSample Test', () => {
  document.body.innerHTML = `<div class="ddd"><lit-table id="tb-states"></lit-table><div>`;
  let tab = document.querySelector('.ddd') as HTMLDivElement;
  let tabPaneCounterSample = new TabPaneCounterSample();
  tabPaneCounterSample.tbl = jest.fn(() => tab);
  tabPaneCounterSample.tbl.treeElement = jest.fn(() => tab);
  tabPaneCounterSample.tbl.tableElement = jest.fn(() => tab);
  SpSystemTrace.SPT_DATA = [
    {
      process: '',
      processId: 1,
      thread: '',
      threadId: 1,
      state: '',
      dur: 12,
      start_ts: 1,
      end_ts: 5,
      cpu: 1,
      priority: '',
      note: '',
    },
    {
      process: '',
      processId: 51,
      thread: '',
      threadId: 6,
      state: '',
      dur: 56,
      start_ts: 0,
      end_ts: 56,
      cpu: 56,
      priority: '-',
      note: '-',
    },
    {
      process: '',
      processId: 2,
      thread: '',
      threadId: 2,
      state: '',
      dur: 52,
      start_ts: 1,
      end_ts: 53,
      cpu: 2,
      priority: '',
      note: '-',
    },
  ];

  let dataArray = {
    id: 0,
    pid: 0,
    title: '',
    children: [],
    process: '',
    processId: 3,
    thread: '',
    threadId: 0,
    state: 'a',
    wallDuration: 123222,
    avgDuration: '',
    count: 10,
    minDuration: 34445,
    maxDuration: 56788,
    stdDuration: 'std',
    cpuStateFilterIds: [1, 2, 3],
  };

  it('TabPaneCounterSampleTest01', function () {
    let getTabPaneCounterSampleData = sqlit.getTabPaneCounterSampleData;
    getTabPaneCounterSampleData.mockResolvedValue([
      {
        value: 'process',
        filterId: 1,
        ts: 1000,
        cpu: 'cpu',
      },
    ]);

    document.body.innerHTML = `<div><tabpane-counter-sample></tabpane-counter-sample></div>`;
    let tabPane = document.querySelector('tabpane-counter-sample') as TabPaneCounterSample;
    let tab = document.querySelector('#tb-states') as LitTable;
    tabPane.tbl = jest.fn(() => tab);
    tabPane.tbl.recycleDataSource = jest.fn(() => dataArray);
    expect((tabPane.data = dataArray)).toBeTruthy();
  });

  it('TabPaneCounterSampleTest02', function () {
    expect(tabPaneCounterSample.initElements()).toBeUndefined();
  });
});
