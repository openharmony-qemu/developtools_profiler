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
import { TabPaneNMSampleList } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMSampleList.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table';
// @ts-ignore
import { NativeHookSampleQueryInfo, NativeHookSamplerInfo } from '../../../../../../dist/trace/bean/NativeHook.js';
// @ts-ignore
import { NativeMemory } from '../../../../../../dist/trace/bean/NativeHook.js';
// @ts-ignore
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
import { queryAllHookData } from '../../../../../../dist/trace/database/SqlLite.js';
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    unobserve: jest.fn(),
    disconnect: jest.fn(),
    observe: jest.fn(),
  }));
describe('TabPaneNMSampleList Test', () => {
  document.body.innerHTML = '<tabpane-native-sample id="ddt"></tabpane-native-sample>';
  let tabPaneNMSampleList = document.querySelector<TabPaneNMSampleList>('#ddt');

  TabPaneNMSampleList.source = [
    {
      current: '',
      currentSize: 101,
      startTs: 110,
      heapSize: 10,
      snapshot: 'snapshot01',
      growth: '',
      total: 980,
      totalGrowth: '',
      existing: 7810,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: 320,
    },
  ];
  TabPaneNMSampleList.filterSelect = '0';

  tabPaneNMSampleList.currentSelection = jest.fn(() => true);
  let dat = {
    cpus: [],
    threadIds: [1,2,3],
    trackIds: [23,56,77],
    funTids: [675,75],
    heapIds: [11,223],
    nativeMemory: [],
    leftNs: 12222,
    rightNs: 654233,
    hasFps: false,
    statisticsSelectData: undefined,
  };

  let NativeHookSnapshotTypes = [
    {
      eventId: -1,
      current: 0,
      eventType: '',
      subType: '',
      growth: 0,
      existing: 0,
      addr: '',
      startTs: 0,
      endTs: 0,
      total: 0,
      children: [],
    },
  ];

  let MockNativeHookSnapshotTypes = sqlit.queryNativeHookSnapshotTypes;

  MockNativeHookSnapshotTypes.mockResolvedValue([new NativeHookSampleQueryInfo()]);

  let samplerInfo = [
    {
      current: '',
      currentSize: 102,
      startTs: 540,
      heapSize: 23,
      snapshot: 'snapshot02',
      growth: '',
      total: 332,
      totalGrowth: '',
      existing: 60,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: 55,
      threadId: 33,
      threadName: '',
    },
    {
      current: '',
      currentSize: 103,
      startTs: 369,
      heapSize: 98,
      snapshot: 'snapshot03',
      growth: '',
      total: 9855,
      totalGrowth: '',
      existing: 10,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: 132,
      threadId: 130,
      threadName: '',
    },
  ];

  tabPaneNMSampleList.data = dat;
  it('TabPaneNMSampleListTest01', function () {
    expect(TabPaneNMSampleList.serSelection(dat)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest02', function () {
    let sampleData = new NativeMemory();

    let MockqueryAllHookData = sqlit.queryAllHookData;
    MockqueryAllHookData.mockResolvedValue([new NativeHookSampleQueryInfo()]);

    expect(TabPaneNMSampleList.addSampleData(sampleData)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest04', function () {
    let snapshot = {
      current: '',
      currentSize: 104,
      startTs: 960,
      heapSize: 99,
      snapshot: 'snapshot04',
      growth: '',
      total: 990,
      totalGrowth: '',
      existing: 634,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: 22,
    };

    let snapshotLeft = {
      current: 'left',
      currentSize: 0,
      startTs: 4,
      heapSize: 40,
      snapshot: '',
      growth: '',
      total: 400,
      totalGrowth: '',
      existing: 0,
      children: [snapshot],
      tempList: [],
      timestamp: '',
      eventId: -1,
    };

    let snapshotRight = {
      current: 'right',
      currentSize: 0,
      startTs: 5,
      heapSize: 50,
      snapshot: '',
      growth: '',
      total: 500,
      totalGrowth: '',
      existing: 0,
      children: [snapshot],
      tempList: [],
      timestamp: '',
      eventId: -1,
    };
    expect(TabPaneNMSampleList.prepChild(snapshotLeft, snapshotRight)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest10', function () {
    expect(TabPaneNMSampleList.data).toBeUndefined();
  });

  it('TabPaneNMSampleListTest11', function () {
    expect(TabPaneNMSampleList.initTypes()).toBeUndefined();
  });


  it('TabPaneNMSampleListTest09', function () {
    let rootSample = new NativeHookSamplerInfo();

    let merageSample = {
      growth: 1,
      endTs: 2,
      startTs: 2,
      addr: '1',
      eventId: 0,
    };
    expect(TabPaneNMSampleList.merageSampleData(1, 1, rootSample, merageSample)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest12', function () {
    let rootSample = new NativeHookSamplerInfo();

    let MockqueryAllHookData = sqlit.queryAllHookData;

    MockqueryAllHookData.mockResolvedValue([
      {
        eventId: 1,
        eventType: 'aa',
        subType: 1,
        addr: 'aaaa',
        growth: 2,
        startTs: 11111,
        endTs: 211111,
      },
    ]);
    expect(TabPaneNMSampleList.queryAllHookInfo(dat, rootSample)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest13', function () {
    TabPaneNMSampleList.samplerInfoSource = [
      {
        current: '',
        currentSize: 0,
        startTs: 1,
        heapSize: 10,
        snapshot: '',
        growth: '',
        total: 100,
        totalGrowth: '',
        existing: 0,
        children: samplerInfo,
        tempList: samplerInfo,
        timestamp: '',
        eventId: -1,
        threadId: 1,
        threadName: '',
      },
    ];
    TabPaneNMSampleList.filterSelect = '0';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest14', function () {
    TabPaneNMSampleList.samplerInfoSource = [
      {
        current: '',
        currentSize: 0,
        startTs: 2,
        heapSize: 20,
        snapshot: '',
        growth: '',
        total: 200,
        totalGrowth: '',
        existing: 0,
        children: samplerInfo,
        tempList: samplerInfo,
        timestamp: '',
        eventId: -1,
        threadId: 2,
        threadName: '',
      },
    ];
    TabPaneNMSampleList.filterSelect = '1';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest15', function () {
    TabPaneNMSampleList.samplerInfoSource = [
      {
        current: '',
        currentSize: 108,
        startTs: 9951,
        heapSize: 123,
        snapshot: 'snapshot06',
        growth: '',
        total: 70,
        totalGrowth: '',
        existing: 50,
        children: [],
        tempList: [],
        timestamp: '',
        eventId: 5,
        threadId: 57,
        threadName: '',
      },
    ];
    TabPaneNMSampleList.filterSelect = '1';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest16', function () {
    TabPaneNMSampleList.samplerInfoSource = [
      {
        current: '',
        currentSize: 0,
        startTs: 3,
        heapSize: 30,
        snapshot: '',
        growth: '',
        total: 300,
        totalGrowth: '',
        existing: 0,
        children: samplerInfo,
        tempList: samplerInfo,
        timestamp: '',
        eventId: -1,
        threadId: 3,
        threadName: '',
      },
    ];
    TabPaneNMSampleList.filterSelect = '2';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest17', function () {
    TabPaneNMSampleList.samplerInfoSource = [
      {
        current: '',
        currentSize: 109,
        startTs: 95,
        heapSize: 987,
        snapshot: 'snapshot08',
        growth: '',
        total: 52,
        totalGrowth: '',
        existing: 1002,
        children: [],
        tempList: [],
        timestamp: '',
        eventId: 106,
        threadId: 980,
        threadName: '',
      },
    ];
    TabPaneNMSampleList.filterSelect = '2';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });
});
