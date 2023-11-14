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
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    unobserve: jest.fn(),
    disconnect: jest.fn(),
    observe: jest.fn(),
  }));
describe('TabPanePTS Test', () => {
  document.body.innerHTML = `<div><tabpane-pts class="pts"></tabpane-pts></div>`;
  let tabPane = document.querySelector('.pts') as TabPanePTS;

  SpSystemTrace.SPT_DATA = [
    {
      process: 'hiperf 2471',
      processId: 2471,
      thread: 'hiperf',
      threadId: 2471,
      state: '',
      dur: 98,
      start_ts: 8,
      end_ts: 90,
      cpu: 0,
      priority: '-',
      note: 'note',
    },
    {
      process: '',
      processId: 372,
      thread: 'download_server',
      threadId: 708,
      state: '',
      dur: 963,
      start_ts: 6,
      end_ts: 969,
      cpu: 1,
      priority: 'a',
      note: 'a',
    },
    {
      process: '',
      processId: 487,
      thread: 'CellularDataSer',
      threadId: 1244,
      state: '',
      dur: 888,
      start_ts: 0,
      end_ts: 888,
      cpu: 2,
      priority: '120',
      note: '2',
    },
  ];

  let dataArray = [
    {
      id: 4,
      pid: 3,
      title: '',
      children: [],
      process: '',
      processId: 2452,
      thread: 'hiprofiler_cmd 2452',
      threadId: 2452,
      state: '',
      wallDuration: 5655,
      avgDuration: '',
      count: 43,
      minDuration: 12,
      maxDuration: 6333,
      stdDuration: '',
    },
  ];

  it('TabPanePTSTest01', function () {
    expect(tabPane.getDataByPTS(0, 0, [])).toBeUndefined();
  });

  it('TabPanePTSTest02', function () {
    let source = [
      {
        process: '',
        processId: 487,
        thread: 'CellularDataSer',
        threadId: 1244,
        state: '0',
        dur: 1100,
        start_ts: 100_0000_0,
        end_ts: 100_0000_1100,
        cpu: 1,
        priority: '118',
        note: '-',
      },
    ];
    expect(tabPane.getDataByPTS(10, 100_000, source)).toBeUndefined();
  });
});
