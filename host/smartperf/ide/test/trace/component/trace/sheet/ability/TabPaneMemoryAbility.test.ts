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

//@ts-ignore
import { TabPaneMemoryAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneMemoryAbility.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

describe('TabPaneMemoryAbility Test', () => {
  let tabPaneMemoryAbility = new TabPaneMemoryAbility();
  let queryStartTime = sqlit.queryStartTime;
  queryStartTime.mockResolvedValue([
    {
      start_ts: 0,
    },
  ]);

  let queryMemoryAbilityData = sqlit.getTabMemoryAbilityData;
  queryMemoryAbilityData.mockResolvedValue([
    {
      startTime: 0,
      name: 's',
      value: 'd,ds,f',
    },
    {
      startTime: 10000,
      name: 's',
      value: 'd,ds,f',
    },
    {
      startTime: 20000,
      name: 's',
      value: 'd,ds,f',
    },
  ]);

  tabPaneMemoryAbility.memoryAbilityTbl = jest.fn(() => true);
  tabPaneMemoryAbility.memoryAbilityTbl.recycleDataSource = jest.fn(() => []);
  tabPaneMemoryAbility.data = {
    cpus: [1],
    threadIds: [12,45,5],
    trackIds: [554,87,5],
    funTids: [44,90],
    heapIds: [77,177],
    nativeMemory: [],
    cpuAbilityIds: [55,76],
    memoryAbilityIds: [5],
    diskAbilityIds: [55,8],
    networkAbilityIds: [8,0],
    leftNs: 8540,
    rightNs: 91000,
    hasFps: true,
    statisticsSelectData: undefined,
    perfSampleIds: [120,56],
    perfCpus: [],
    perfProcess: [],
    perfThread: [],
    perfAll: false,
    systemEnergy: [34, 2],
    powerEnergy: [0, 234],
    anomalyEnergy: [2, 5, 6, 6, 2],
  };

  it('TabPaneMemoryAbilityTest01', function () {
    tabPaneMemoryAbility.queryMemoryResult.length = 1;
    expect(tabPaneMemoryAbility.filterData()).toBeUndefined();
  });

  it('TabPaneMemoryAbilityTest02', function () {
    const systemMemorySummary = [
      {
        startTimeStr: '1',
        durationStr: '1',
        cached: '1',
        swapTotal: '1',
      },
    ];
    expect(tabPaneMemoryAbility.toMemoryAbilityArray(systemMemorySummary)).not.toBeUndefined();
  });

  it('TabPaneMemoryAbilityTest03', function () {
    expect(
      tabPaneMemoryAbility.sortByColumn({
        key: 'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneMemoryAbilityTest04', function () {
    expect(
      tabPaneMemoryAbility.sortByColumn({
        key: !'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneMemoryAbilityTest05', function () {
    expect(
      tabPaneMemoryAbility.sortByColumn({
        key: 'durationStr',
      })
    ).toBeUndefined();
  });
});
