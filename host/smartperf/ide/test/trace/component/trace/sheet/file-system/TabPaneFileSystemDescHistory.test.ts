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
import { TabPaneFileSystemDescHistory } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemDescHistory.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemDescHistory.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    unobserve: jest.fn(),
    observe: jest.fn(),
  }));

Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});

describe('TabPaneFileSystemDescHistory Test', () => {
  document.body.innerHTML = `<tabpane-filesystem-desc-history id="history"></tabpane-filesystem-desc-history>`;
  let tabPane = document.querySelector<TabPaneFileSystemDescHistory>('#history');

  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 122},
    cpuAbilityIds: [222,7],
    cpuFreqFilterIds: [56,99],
    cpuFreqLimitDatas: [67,8],
    cpuStateFilterIds: [558, 66],
    cpus: [0],
    diskAbilityIds: [0,8],
    diskIOLatency: false,
    diskIOReadIds: [2, 7, 1, 545],
    diskIOWriteIds: [2, 741, 1, 3, 4, 5, 6],
    diskIOipids: [2, 7, 444, 6],
    fileSysVirtualMemory: false,
    fileSystemType: [],
    fsCount: 55,
    funAsync: [],
    funTids: [133,56,99],
    hasFps: false,
    irqMapData: { size: 66 },
    jsMemory: [],
    leftNs: 964699549,
    memoryAbilityIds: [34,766],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [34,677],
    perfAll: false,
    perfCpus: [0],
    perfProcess: [],
    perfSampleIds: [234,677],
    perfThread: [''],
    powerEnergy: [],
    processTrackIds: [414,478],
    promiseList: [],
    recordStartNs: 780466689228,
    rightNs: 24269844624,
    sdkCounterIds: [444,76],
    sdkSliceIds: [],
    smapsType: [],
    systemEnergy: [],
    threadIds: [534,767],
    virtualTrackIds: [330,56],
    vmCount: 66,
  };
  let filterSource = [
    {
      backtrace: ['0x7faa10f228', '(10 other frames)'],
      callchainId: 133,
      depth: 10,
      dur: 240916,
      durStr: '240.92μs ',
      fd: 143,
      fileId: 546,
      isHover: false,
      path: '/data/local/tmp/test',
      process: 'power_host[911]',
      startTs: 285141831,
      startTsStr: '285ms 141μs 821ns ',
      symbol: '0x7faa10f228',
      type: 3,
      typeStr: 'OPEN',
    },
    {
      backtrace: ['0x7faa10f228', '(10 other frames)'],
      callchainId: 15,
      depth: 103,
      dur: 7583,
      durStr: '7.58μs ',
      fd: 143,
      fileId: null,
      isHover: false,
      path: '-',
      process: 'test[911]',
      startTs: 285449633,
      startTsStr: '285ms 449μs 821ns ',
      symbol: '0x7faa10f228',
      type: 3,
      typeStr: 'CLOSE',
    },
  ];

  it('TabPaneFileSystemDescHistoryTest01', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.currentSelection).not.toBeUndefined();
  });

  it('TabPaneFileSystemDescHistoryTest02', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.setProcessFilter();
    expect(tabPane.processList).toEqual(['All Process']);
  });

  it('TabPaneFileSystemDescHistoryTest03', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.filterData()).toBeUndefined();
  });

  it('TabPaneFileSystemDescHistoryTest04', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.filterSource = filterSource;
    expect(tabPane.sortFsDescHistoryTable('startTsStr', 1)).toBeUndefined();
  });

  it('TabPaneFileSystemDescHistoryTest05', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.filterSource = filterSource;
    expect(tabPane.sortFsDescHistoryTable('durStr', 1)).toBeUndefined();
  });

  it('TabPaneFileSystemDescHistoryTest06', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.filterSource = filterSource;
    expect(tabPane.sortFsDescHistoryTable('typeStr', 1)).toBeUndefined();
  });

  it('TabPaneFileSystemDescHistoryTest07', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    tabPane.filterSource = filterSource;
    expect(tabPane.sortFsDescHistoryTable('fd', 1)).toBeUndefined();
  });
});
