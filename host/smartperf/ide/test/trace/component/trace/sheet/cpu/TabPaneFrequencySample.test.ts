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
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import {LitTable} from "../../../../../../dist/base-ui/table/lit-table.js";

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

  let dataArray = {
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
    cpuFreqFilterIds: [1, 2, 3],
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

  it('TabPaneCounterSampleTest03', function () {
    expect(tabPaneFreSample.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .progressFre{
            bottom: 5px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        .loadingFre{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <lit-table id="tb-states" style="height: auto" >
            <lit-table-column class="freq-sample-column" width="20%" title="Cpu" data-index="counter" key="counter" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Time" data-index="timeStr" key="timeStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Value" data-index="valueStr" key="valueStr" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="progressFre"></lit-progress-bar>
        <div class="loadingFre"></div>
        "
`);
  });
});
