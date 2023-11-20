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
import { TabPaneCpuUsage } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCpuUsage.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneCpuUsage Test', () => {
  let tabPaneCpuUsage = new TabPaneCpuUsage();

  let mockGetTabCpuUsage = sqlit.getTabCpuUsage;
  let mockGetTabCpuFreq = sqlit.getTabCpuFreq;

  mockGetTabCpuUsage.mockResolvedValue([]);
  mockGetTabCpuFreq.mockResolvedValue([]);

  let selectionData = {
    cpus: [1, 0],
    threadIds: [12, 3, 0],
    trackIds: [3, 789, 89],
    funTids: [5, 0],
    heapIds: [0, 43],
    nativeMemory: [],
    cpuAbilityIds: [10, 45],
    memoryAbilityIds: [],
    diskAbilityIds: [85],
    networkAbilityIds: [],
    leftNs: 8540,
    rightNs: 96440,
    hasFps: false,
    statisticsSelectData: undefined,
    perfSampleIds: [1, 4, 44],
    perfCpus: [],
    perfProcess: [],
    perfThread: [],
    perfAll: false,
  };

  it('TabPaneCpuUsageTest01', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        3,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest08', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        4,
        false
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest09', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        5,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest02', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 1, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        1,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest03', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 2, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        2,
        false
      )
    ).toBeUndefined();
  });
  it('TabPaneCpuUsageTest04', function () {
    let result = tabPaneCpuUsage.sortFreq([
      {
        cpu: 0,
        value: 30,
        startNs: 342,
        dur: 341,
      },
      {
        cpu: 1,
        value: 322,
        startNs: 762,
        dur: 984,
      },
    ]);
    expect(result[0][0]).toBe(322);
  });
  it('TabPaneCpuUsageTest05', function () {
    expect(
      tabPaneCpuUsage.getFreqTop3(
        {
          cpu: 0,
          usage: 0,
          usageStr: 'usage',
          top1: 1,
          top2: 2,
          top3: 3,
          top1Percent: 11,
          top1PercentStr: 'Str1',
          top2Percent: 22,
          top2PercentStr: 'Str2',
          top3Percent: 33,
          top3PercentStr: 'Str3',
        },
        undefined,
        undefined,
        undefined,
        1
      )
    ).toBeUndefined();
  });
  it('TabPaneCpuUsageTest06', function () {
    let result = tabPaneCpuUsage.groupByCpuToMap([
      {
        cpu: 0,
        value: 90,
        startNs: 250,
        dur: 560,
      },
      {
        cpu: 1,
        value: 782,
        startNs: 52,
        dur: 4,
      },
    ]);
    expect(result.get(0).length).toBe(1);
  });

  it('TabPaneCpuUsageTest11', function () {
    document.body.innerHTML = `<div id="CpuUsage"></div>`;
    let tabPaneCpuUsage = document.querySelector('#CpuUsage') as TabPaneCpuUsage;
    expect(tabPaneCpuUsage.sortFreq).toBe(undefined);
  });
  it('TabPaneCpuUsageTest12', function () {
    document.body.innerHTML = `<div id="CpuUsage"></div>`;
    let tabPaneCpuUsage = document.querySelector('#CpuUsage') as TabPaneCpuUsage;
    tabPaneCpuUsage.data = [
      {
        cpus: [2],
        threadIds: [],
        trackIds: [12, 4],
        funTids: [56, 345],
        heapIds: [],
        nativeMemory: [],
        cpuAbilityIds: [10, 32, 1],
        memoryAbilityIds: [],
        diskAbilityIds: [12, 76],
        networkAbilityIds: [],
        leftNs: 77,
        rightNs: 987,
        hasFps: false,
        statisticsSelectData: undefined,
        perfSampleIds: [],
        perfCpus: [0, 9],
        perfProcess: [],
        perfThread: [],
        perfAll: true,
      },
    ];
    expect(tabPaneCpuUsage).toBeTruthy();
  });
});
