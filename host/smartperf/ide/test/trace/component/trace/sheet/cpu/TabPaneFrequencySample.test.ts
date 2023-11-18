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
import { TabPaneFrequencySample } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneFrequencySample.js';
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
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

describe('TabPaneFrequencySample Test', () => {
  document.body.innerHTML = `<div class="ddd"><lit-table id="tb-states"></lit-table><div>`;
  let tab = document.querySelector('.ddd') as HTMLDivElement;
  let tabPaneFreSample = new TabPaneFrequencySample();
  tabPaneFreSample.tbl = jest.fn(() => tab);
  tabPaneFreSample.tbl.treeElement = jest.fn(() => tab);
  tabPaneFreSample.tbl.tableElement = jest.fn(() => tab);
  SpSystemTrace.SPT_DATA = [
    {
      process: '',
      processId: 252,
      thread: 'render_service',
      threadId: 886,
      state: '',
      dur: 20,
      start_ts: 0,
      end_ts: 20,
      cpu: 0,
      priority: '-',
      note: '-',
    },
    {
      process: '',
      processId: 178,
      thread: 'sugov',
      threadId: 178,
      state: '',
      dur: 88,
      start_ts: 5,
      end_ts: 93,
      cpu: 1,
      priority: 'priority',
      note: 'note',
    },
    {
      process: '',
      processId: 372,
      thread: 'download_server',
      threadId: 696,
      state: '',
      dur: 256,
      start_ts: 2,
      end_ts: 258,
      cpu: 20,
      priority: '-',
      note: '-',
    },
  ];

  let dataArray = {
    id: 1,
    pid: 1,
    title: '1',
    children: [],
    process: '',
    processId: 2452,
    thread: 'hiprofiler_cmd',
    threadId: 2452,
    state: '',
    wallDuration: 565552,
    avgDuration: '',
    count: 6,
    minDuration: 888,
    maxDuration: 63332,
    stdDuration: '',
    cpuFreqFilterIds: [1, 2, 9, 3],
  };

  it('TabPaneCounterSampleTest01', function () {
    let getTabPaneFrequencySampleData = sqlit.getTabPaneFrequencySampleData;
    getTabPaneFrequencySampleData.mockResolvedValue([
      {
        value: 'process',
        filterId: 1,
        ts: 1000,
        cpu: 'cpu',
      },
    ]);
    document.body.innerHTML = `<div><tabpane-frequency-sample></tabpane-frequency-sample></div>`;
    let tabPane = document.querySelector('tabpane-frequency-sample') as TabPaneFrequencySample;
    let tab = document.querySelector('#tb-states') as LitTable;
    tabPane.tbl = jest.fn(() => tab);
    tabPane.tbl.recycleDataSource = jest.fn(() => dataArray);
    expect((tabPane.data = dataArray)).toBeTruthy();
  });

  it('TabPaneCounterSampleTest02', function () {
    expect(tabPaneFreSample.initElements()).toBeUndefined();
  });
});
