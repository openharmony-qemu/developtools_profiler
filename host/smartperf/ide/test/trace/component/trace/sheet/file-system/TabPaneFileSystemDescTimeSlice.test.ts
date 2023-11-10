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
import { TabPaneFileSystemDescTimeSlice } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemDescTimeSlice.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemDescTimeSlice.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';

Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});
// @ts-ignore
window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
      observe: jest.fn(),
      disconnect: jest.fn(),
      unobserve: jest.fn(),
    }));

describe('TabPaneFileSystemDescTimeSlice Test', () => {
  document.body.innerHTML = `<tabpane-filesystem-desc-time-slice id="desc-time-slice"></tabpane-filesystem-desc-time-slice>`;
  let tabPane = document.querySelector<TabPaneFileSystemDescTimeSlice>('#desc-time-slice');

  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 18 },
    cpuAbilityIds: [12,67,4],
    cpuFreqFilterIds: [],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [102,58,145],
    cpus: [1,3],
    diskAbilityIds: [41,56],
    diskIOLatency: false,
    diskIOReadIds: [643, 6],
    diskIOWriteIds: [2, 322, 76, 6],
    diskIOipids: [89, 3, 4, 5, 56],
    fileSysVirtualMemory: false,
    fileSystemType: [],
    fsCount: 66,
    funAsync: [],
    funTids: [41,123],
    hasFps: false,
    irqMapData: { size: 96 },
    jsMemory: [],
    leftNs: 964699969,
    memoryAbilityIds: [452,12],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [45,541],
    perfAll: false,
    perfCpus: [10,55],
    perfProcess: [],
    perfSampleIds: [56,144,12],
    perfThread: [''],
    powerEnergy: [],
    processTrackIds: [],
    promiseList: [],
    recordStartNs: 780423789998,
    rightNs: 24267566524,
    sdkCounterIds: [],
    sdkSliceIds: [52,68,35],
    smapsType: [],
    systemEnergy: [],
    threadIds: [],
    virtualTrackIds: [55,56,14],
    vmCount: 90,
  };

  let filterSource = [
    {
      backtrace: ['0x7faa10f228', '(10 other frames)'],
      callchainId: 132,
      depth: 10,
      dur: 240916,
      durStr: '240.92μs ',
      fd: 142,
      fileId: 546,
      isHover: false,
      path: '/data/local/tmp/test',
      process: 'power_host[911]',
      startTs: 285141822,
      startTsStr: '285ms 141μs 821ns ',
      symbol: '0x7faa10f228',
      type: 2,
      typeStr: 'OPEN',
    },
    {
      backtrace: ['0x7faa10f228', '(10 other frames)'],
      callchainId: 152,
      depth: 102,
      dur: 7583,
      durStr: '7.58μs ',
      fd: 142,
      fileId: null,
      isHover: false,
      path: '-',
      process: 'test[911]',
      startTs: 285449622,
      startTsStr: '285ms 449μs 821ns ',
      symbol: '0x7faa10f228',
      type: 2,
      typeStr: 'CLOSE',
    },
  ];

  it('descTimeSliceTest01', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.currentSelection).not.toBeUndefined();
  });

  it('descTimeSliceTest02', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.source = filterSource;
    expect(tabPane.sortFsDescTimeSliceTable('startTsStr', 0)).toBeUndefined();
  });

  it('descTimeSliceTest03', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.source = filterSource;
    expect(tabPane.sortFsDescTimeSliceTable('durStr', 1)).toBeUndefined();
  });

  it('descTimeSliceTest04', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.source = filterSource;
    expect(tabPane.sortFsDescTimeSliceTable('typeStr', 1)).toBeUndefined();
  });

  it('descTimeSliceTest05', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.source = filterSource;
    expect(tabPane.sortFsDescTimeSliceTable('fd', 1)).toBeUndefined();
  });
});
