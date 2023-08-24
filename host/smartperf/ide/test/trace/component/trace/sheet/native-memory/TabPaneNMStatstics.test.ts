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
  return {}
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
        heapSize: 0,
        allocByte: 0,
        allocCount: 0,
        freeByte: 0,
        freeCount: 0,
      },
    ];
    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 0,
        totalCount: 0,
        existingValue: [],
      },
    ];

    expect(tabPaneNMStatstics.setSubTypeTableData(nativeHookMalloc, nativeHookStatisticsTableData)).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest03', function () {
    expect(tabPaneNMStatstics.initHtml()).toMatchInlineSnapshot(`
"
<style>
.nm-stat-tbl {
    height: auto
}
:host{
    display: flex;
    flex-direction: column;
    padding: 10px 10px;
}
</style>
<lit-table id="tb-native-statstics" class="nm-stat-tbl">
    <lit-table-column class="nm-stat-column" width="25%" title="Memory Type" data-index="memoryTap" key="memoryTap"  align="flex-start"></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="Existing" data-index="existingString" key="existingString"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="# Existing" data-index="allocCount" key="allocCount"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="Transient" data-index="freeByteString" key="freeByteString"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="# Transient" data-index="freeCount" key="freeCount"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="Total Bytes" data-index="totalBytesString" key="totalBytesString"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="# Total" data-index="totalCount" key="totalCount"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="1fr" title="Peak Value" data-index="maxStr" key="maxStr"  align="flex-start" order></lit-table-column>
    <lit-table-column class="nm-stat-column" width="160px" title="Existing / Total" data-index="existingValue" key="existingValue"  align="flex-start" >
        <template><tab-progress-bar data="{{existingValue}}"></tab-progress-bar></template>
    </lit-table-column>
</lit-table>
        "
`);
  });

  it('TabPaneNMStatsticsTest04', function () {
    let valData = {
      cpus: [],
      threadIds: [],
      trackIds: [],
      funTids: [],
      heapIds: [],
      nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [],
      networkAbilityIds: [],
      leftNs: 0,
      rightNs: 0,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'AllocEvent',
        subType: '',
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 0,
        totalCount: 0,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest05', function () {
    let valData = {
      cpus: [],
      threadIds: [],
      trackIds: [],
      funTids: [],
      heapIds: [],
      nativeMemory: ['All Heap'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [],
      networkAbilityIds: [],
      leftNs: 0,
      rightNs: 0,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'FreeEvent',
        subType: '',
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 0,
        totalCount: 0,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest06', function () {
    let valData = {
      cpus: [],
      threadIds: [],
      trackIds: [],
      funTids: [],
      heapIds: [],
      nativeMemory: ['All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [],
      networkAbilityIds: [],
      leftNs: 0,
      rightNs: 0,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'MmapEvent',
        subType: '',
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 0,
        totalCount: 0,
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
      threadIds: [],
      trackIds: [],
      funTids: [],
      heapIds: [],
      nativeMemory: ['All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [],
      networkAbilityIds: [],
      leftNs: 0,
      rightNs: 0,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'MunmapEvent',
        subType: '',
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 0,
        totalCount: 0,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });

  it('TabPaneNMStatsticsTest08', function () {
    let valData = {
      cpus: [],
      threadIds: [],
      trackIds: [],
      funTids: [],
      heapIds: [],
      nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [],
      networkAbilityIds: [],
      leftNs: 0,
      rightNs: 0,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
    };
    let nativeHookStatistics: Array<NativeHookStatistics> = [
      {
        eventId: 0,
        eventType: 'FreeEvent',
        subType: '',
        heapSize: 0,
        addr: '',
        startTs: 0,
        endTs: 0,
        sumHeapSize: 0,
        max: 100000,
        count: 0,
        tid: 0,
        isSelected: false,
      },
    ];

    let nativeHookStatisticsTableData: Array<NativeHookStatisticsTableData> = [
      {
        memoryTap: '',
        existing: 0,
        existingString: '',
        allocCount: 0,
        freeCount: 0,
        totalBytes: 0,
        totalBytesString: '',
        maxStr: '',
        max: 100,
        totalCount: 0,
        existingValue: [],
      },
    ];

    expect(
      tabPaneNMStatstics.setMemoryTypeData(valData, nativeHookStatistics, nativeHookStatisticsTableData)
    ).toBeUndefined();
  });
});
