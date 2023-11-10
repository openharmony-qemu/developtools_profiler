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
import { TabPaneNMStatstics } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMStatstics.js';
// @ts-ignore
import {
  NativeHookMalloc,
  NativeHookStatistics,
  NativeHookStatisticsTableData,
} from '../../../../../../dist/trace/bean/NativeHook';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('TabPaneNMStatstics Test', () => {
  let tabPaneNMStatstics = new TabPaneNMStatstics();
  document.body.innerHTML = '<div class="table"></div>';

  it('TabPaneNMStatsticsTest01', function () {
    expect(tabPaneNMStatstics.setMallocTableData([1], [1])).toBeUndefined();
  });
  it('TabPaneNMStatsticsTest09', function () {
    expect(tabPaneNMStatstics.setSubTypeTableData([1], [1])).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest02', function () {
    let nativeHookMalloc: Array<NativeHookMalloc> = [
      {
        eventType: '',
        subType: '',
        subTypeId: 0,
        heapSize: 0,
        allocByte: 0,
        allocCount: 0,
        freeByte: 0,
        freeCount: 0,
        max: 0,
      },
    ];
    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '12',
        existing: 50,
        existingString: '',
        freeByteString: '',
        allocCount: 254,
        freeCount: 43,
        freeByte: 23,
        totalBytes: 1,
        totalBytesString: '',
        maxStr: '',
        max: 110,
        totalCount: 1150,
        existingValue: [],
      },
    ];

    expect(tabPaneNMStatstics.setSubTypeTableData(nativeHookMalloc, nativeHookStatisticsTableData)).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest04', function () {
    let valData = {
      cpus: [],
      threadIds: [],
      trackIds: [21, 45, 6],
      funTids: [111, 4, 43],
      heapIds: [5, 77, 67, 0],
      nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [88, 56, 7],
      networkAbilityIds: [],
      leftNs: 1110,
      rightNs: 15600,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: true,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'AllocEvent',
        subType: '',
        subTypeId: 0,
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        threadName: '',
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 540,
        existingString: '',
        freeByteString: '',
        allocCount: 20,
        freeCount: 10,
        freeByte: 20,
        totalBytes: 20,
        totalBytesString: '',
        maxStr: '',
        max: 50,
        totalCount: 40,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest05', function () {
    let valData = {
      cpus: [3],
      threadIds: [],
      trackIds: [12,4],
      funTids: [12,345],
      heapIds: [],
      nativeMemory: ['All Heap'],
      cpuAbilityIds: [10,56,1],
      memoryAbilityIds: [],
      diskAbilityIds: [12,76],
      networkAbilityIds: [],
      leftNs: 2330,
      rightNs: 56670,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [0,3],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 980,
        eventType: 'FreeEvent',
        subType: '',
        subTypeId: 0,
        heapSize: 7,
        addr: '',
        startTs: 77,
        endTs: 6,
        sumHeapSize: 0,
        max: 100654,
        count: 40,
        tid: 660,
        threadName: '',
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 20,
        existingString: '',
        freeByteString: '',
        allocCount: 12,
        freeCount: 121,
        freeByte: 221,
        totalBytes: 21,
        totalBytesString: '',
        maxStr: '',
        max: 220,
        totalCount: 465,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest06', function () {
    let valData = {
      cpus: [1,3],
      threadIds: [],
      trackIds: [],
      funTids: [543,76],
      heapIds: [],
      nativeMemory: ['All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [23, 56, 7],
      networkAbilityIds: [100, 156],
      leftNs: 450,
      rightNs: 5210,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [12, 56],
      perfCpus: [0],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 90,
        eventType: 'MmapEvent',
        subType: '',
        subTypeId: 21,
        heapSize: 97,
        addr: '',
        startTs: 77,
        endTs: 6,
        sumHeapSize: 0,
        max: 10114,
        count: 10,
        tid: 611,
        threadName: '',
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 510,
        existingString: '',
        freeByteString: '',
        allocCount: 2312,
        freeCount: 51,
        freeByte: 321,
        totalBytes: 90,
        totalBytesString: '',
        maxStr: '02',
        max: 2082,
        totalCount: 55,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest07', function () {
    let valData = {
      cpus: [],
      threadIds: [12, 43, 5],
      trackIds: [],
      funTids: [22,29,20],
      heapIds: [],
      nativeMemory: ['All Anonymous VM'],
      cpuAbilityIds: [133,54,5],
      memoryAbilityIds: [],
      diskAbilityIds: [13, 14, 19],
      networkAbilityIds: [],
      leftNs: 2211,
      rightNs: 433111,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [520, 88, 1],
      perfCpus: [],
      perfProcess: ['ssioncontroller', 'ndroid.settings'],
      perfThread: ['ndroid.settings'],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 60,
        eventType: 'MmapEvent',
        subType: '',
        subTypeId: 13,
        heapSize: 31,
        addr: '',
        startTs: 137,
        endTs: 61,
        sumHeapSize: 34,
        max: 214,
        count: 10,
        tid: 64,
        threadName: '',
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 210,
        existingString: '',
        freeByteString: '',
        allocCount: 92,
        freeCount: 51,
        freeByte: 2,
        totalBytes: 23,
        totalBytesString: '',
        maxStr: '20',
        max: 232,
        totalCount: 9,
        existingValue: [],
      },
    ];
    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest08', function () {
    let valData = {
      cpus: [0],
      threadIds: [2,90,0],
      trackIds: [],
      funTids: [23,44],
      heapIds: [2,9],
      nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'],
      cpuAbilityIds: [33,22],
      memoryAbilityIds: [],
      diskAbilityIds: [56,87,45],
      networkAbilityIds: [],
      leftNs: 52540,
      rightNs: 9654120,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [12,45,87],
      perfCpus: [1,3],
      perfProcess: [],
      perfThread: [],
      perfAll: true,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 30,
        eventType: 'FreeEvent',
        subType: '',
        subTypeId: 13,
        heapSize: 31,
        addr: 'test/trace/database/logic-worker/ProcedureLogicWorkerNativeNemory.test.ts',
        startTs: 33,
        endTs: 31,
        sumHeapSize: 90,
        max: 4,
        count: 40,
        tid: 14,
        threadName: 'NativeNemory',
        isSelected: true,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 330,
        existingString: 'nativeHookStatistics',
        freeByteString: '',
        allocCount: 72,
        freeCount: 23,
        freeByte: 11,
        totalBytes: 3,
        totalBytesString: '',
        maxStr: '33',
        max: 3,
        totalCount: 42,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });
  it('TabPaneNMStatsticsTest11', function () {
    tabPaneNMStatstics.nativeStatisticsTbl = jest.fn(() => true);
    tabPaneNMStatstics.nativeStatisticsTbl.recycleDataSource = jest.fn(() => true);
    expect(tabPaneNMStatstics.sortByColumn('', 0)).toBeUndefined();
  });
  it('TabPaneNMStatsticsTest12', function () {
    tabPaneNMStatstics.nativeStatisticsTbl = jest.fn(() => true);
    tabPaneNMStatstics.nativeStatisticsTbl.recycleDataSource = jest.fn(() => true);
    expect(tabPaneNMStatstics.sortByColumn('existingString', 1)).toBeUndefined();
  });
  it('TabPaneNMStatsticsTest13', function () {
    tabPaneNMStatstics.nativeStatisticsTbl = jest.fn(() => true);
    tabPaneNMStatstics.nativeStatisticsTbl.recycleDataSource = jest.fn(() => true);
    expect(tabPaneNMStatstics.sortByColumn('allocCount', 1)).toBeUndefined();
  });
  it('TabPaneNMStatsticsTest14', function () {
    tabPaneNMStatstics.nativeStatisticsTbl = jest.fn(() => true);
    tabPaneNMStatstics.nativeStatisticsTbl.recycleDataSource = jest.fn(() => true);
    expect(tabPaneNMStatstics.sortByColumn('freeByteString', 1)).toBeUndefined();
  });
});
