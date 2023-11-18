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
import { TabPaneSPT } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneSPT.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    unobserve: jest.fn(),
    observe: jest.fn(),
  }));

describe('TabPaneSPT Test', () => {
  document.body.innerHTML = `<lit-table id="tb-states"><</lit-table>`;
  let tab = document.querySelector('#tb-states') as LitTable;

  document.body.innerHTML = `<div><tabpane-spt class="SPT"></tabpane-spt></div>`;
  let tabPane = document.querySelector('.SPT') as TabPaneSPT;
  let tabPaneSPT = new TabPaneSPT();
  tabPaneSPT.tbl = jest.fn(() => tab);
  SpSystemTrace.SPT_DATA = [
    {
      process: '',
      processId: 602,
      thread: 'hdcd',
      threadId: 739,
      state: '',
      dur: 233,
      start_ts: 3,
      end_ts: 236,
      cpu: 0,
      priority: '10',
      note: '85',
    },
    {
      process: '',
      processId: 516,
      thread: 'foundation',
      threadId: 516,
      state: '',
      dur: 122,
      start_ts: 22,
      end_ts: 100,
      cpu: 1,
      priority: '114',
      note: '66',
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

  let dataList = [
    {
      id: 78,
      pid: 55,
      title: '',
      children: [],
      process: '',
      processId: 1380,
      thread: 'com.ohos.callui',
      threadId: 1380,
      state: '',
      wallDuration: 5220,
      avgDuration: '',
      count: 5,
      minDuration: 440,
      maxDuration: 36220,
      stdDuration: '',
    },
  ];

  let dataArray = [
    {
      id: 885,
      pid: 12,
      title: '',
      children: [],
      process: '',
      processId: 1387,
      thread: 'com.ohos.callui',
      threadId: 1387,
      state: '',
      wallDuration: 666240,
      avgDuration: '',
      count: 40,
      minDuration: 2330,
      maxDuration: 524660,
      stdDuration: '',
    },
  ];

  it('TabPaneSPTTest01', function () {
    expect(tabPane.getDataBySPT(0, 0, [])).toBeUndefined();
  });

  it('TabPaneSPTTest02', function () {
    let source = [
      {
        process: 'com.ohos.callui 1387',
        processId: 1387,
        thread: 'com.ohos.callui',
        threadId: 1387,
        state: '',
        dur: 66654,
        start_ts: 4,
        end_ts: 66650,
        cpu: 0,
        priority: '154',
        note: '-',
      },
    ];
    expect(tabPane.getDataBySPT(10, 100_000, source)).toBeUndefined();
  });
});
