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
import { TabPaneFileStatistics } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFilesystemStatistics.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFilesystemStatistics.js';
// @ts-ignore
import { Utils } from '../../../../../../dist/trace/component/trace/base/Utils.js';
import '../../../../../../dist/trace/component/trace/base/Utils.js';

import crypto from 'crypto';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

import '../../../../../../dist/base-ui/table/lit-table.js';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
import '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneFileStatistics Test', () => {
  document.body.innerHTML = `<div><tabpane-file-statistics id="statistics"></tabpane-file-statistics></div>`;
  let tabPaneFileStatistics = document.querySelector<TabPaneFileStatistics>('#statistics');
  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 50 },
    cpuAbilityIds: [23, 25],
    cpuFreqFilterIds: [34, 66],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [77, 96],
    cpus: [0],
    diskAbilityIds: [10, 34],
    diskIOLatency: false,
    diskIOReadIds: [2, 11, 4, 12],
    diskIOWriteIds: [2, 54, 64],
    diskIOipids: [25, 7, 58, 6],
    fileSysVirtualMemory: true,
    fileSystemType: [],
    fsCount: 30,
    funAsync: [],
    funTids: [45, 22],
    hasFps: false,
    irqMapData: { size: 32 },
    jsMemory: [],
    leftNs: 964667689,
    memoryAbilityIds: [23, 78, 9],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [10, 154, 55],
    perfAll: false,
    perfCpus: [1],
    perfProcess: [],
    perfSampleIds: [233, 120, 4],
    perfThread: [],
    powerEnergy: [],
    processTrackIds: [34, 21],
    promiseList: [],
    recordStartNs: 780423722428,
    rightNs: 33236556624,
    sdkCounterIds: [12, 56],
    sdkSliceIds: [45, 98],
    smapsType: [],
    systemEnergy: [],
    threadIds: [88, 12],
    virtualTrackIds: [34, 87],
    vmCount: 31,
  };

  it('TabPaneFileStatisticsTest01', function () {
    tabPaneFileStatistics.setInitDua = jest.fn(() => true);
    let item = {
      allDuration: '',
      minDuration: '',
      avgDuration: '',
      maxDuration: '',
    };
    expect(tabPaneFileStatistics.setInitDua(item)).toBeTruthy();
  });

  it('TabPaneFileStatisticsTest02', function () {
    tabPaneFileStatistics.getInitData = jest.fn(() => true);
    let item = {
      allDuration: '',
      minDuration: '',
      avgDuration: '',
      maxDuration: '',
    };
    expect(tabPaneFileStatistics.getInitData(item)).toBeTruthy();
  });

  it('TabPaneFileStatisticsTest04', function () {
    tabPaneFileStatistics.showButtomMenu = jest.fn(() => true);
    let isShow = {
      filter: {
        setAttribute: 'tree, input, inputLeftText',
      },
    };
    expect(tabPaneFileStatistics.showButtomMenu(isShow)).toBeTruthy();
  });

  it('TabPaneFileStatisticsTest08', function () {
    let FileStatistics = new TabPaneFileStatistics();
    let item = {
      allDuration: '',
      minDuration: '',
      avgDuration: '',
      maxDuration: '',
      name: 'as',
      logicalWrites: '',
      logicalReads: '',
      otherFile: '0 Bytes',
      pid: 1,
    };
    Utils.getBinaryByteWithUnit = jest.fn(() => true);
    expect(FileStatistics.getInitData(item)).toEqual({
      allDuration: '',
      avgDuration: '',
      logicalReads: true,
      logicalWrites: true,
      maxDuration: '',
      minDuration: '',
      name: 'as',
      node: {
        allDuration: '',
        avgDuration: '',
        children: [],
        logicalReads: '',
        logicalWrites: '',
        maxDuration: '',
        minDuration: '',
        name: 'as',
        otherFile: '0 Bytes',
        pid: 1,
      },
      otherFile: true,
      pid: 1,
      title: 'as(1)',
    });
  });

  it('TabPaneFileStatisticsTest09', function () {
    let FileStatistics = new TabPaneFileStatistics();
    let node = {
      children: [],
    };
    expect(FileStatistics.sortTable(node, '')).toBeUndefined();
  });
  it('TabPaneFileStatisticsTest10', function () {
    let FileStatistics = new TabPaneFileStatistics();
    let tabPaneFilesystemStatistics = sqlit.getTabPaneFilesystemStatistics;
    let result = [
      {
        pid: 1,
        name: '',
        type: 1,
        count: 34,
        size: 22,
        logicalReads: 32,
        logicalWrites: 12,
        otherFile: 55,
        allDuration: 12,
        minDuration: 1,
        maxDuration: 7,
        avgDuration: 2,
      },
    ];
    FileStatistics.fileStatisticsTbl = jest.fn(() => true);
    FileStatistics.fileStatisticsTbl.recycleDataSource = jest.fn(() => true);
    tabPaneFilesystemStatistics.mockResolvedValue(result);
    expect(FileStatistics.queryDataByDB(param)).toBeUndefined();
  });
});
