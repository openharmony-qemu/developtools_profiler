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
import { TabPanePurgTotal } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgTotal.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPanePurgTotal Test', () => {
  let tabPanePurgTotal = new TabPanePurgTotal();
  let querySysPurgeableTab = sqlit.querySysPurgeableTab;
  querySysPurgeableTab.mockResolvedValue([
    {
      avgSize: 21025824,
      avgSizes: '321.00MB',
      type: 'allocator_host',
      maxSize: 9165822,
      maxSizes: '233.00MB',
      minSize: 35145824,
      minSizes: '12.00MB',
    },
    {
      avgSize: 32133321,
      avgSizes: '320.00MB',
      type: '11allocator_host',
      maxSize: 23265824,
      maxSizes: '4.00MB',
      minSize: 32365824,
      minSizes: '321.00MB',
    },
    {
      avgSize: 98525824,
      avgSizes: '3.00MB',
      type: 'alloca11tor_host',
      maxSize: 12155824,
      maxSizes: '6.00MB',
      minSize: 36545824,
      minSizes: '66.00MB',
    },
  ]);
  tabPanePurgTotal.init = jest.fn(() => true);
  tabPanePurgTotal.data = {
    anomalyEnergy: [],
    clockMapData: { size: 321 },
    cpuAbilityIds: [],
    cpuFreqFilterIds: [],
    cpuFreqLimitDatas: [],
    diskIOReadIds: [],
    diskIOWriteIds: [],
    diskIOipids: [],
    dmaAbilityData: [],
    dmaVmTrackerData: [],
    fsCount: 90,
    funAsync: [],
    funTids: [],
    gpu: { gl: false, gpuWindow: false, gpuTotal: false },
    gpuMemoryAbilityData: [],
    gpuMemoryTrackerData: [],
    hasFps: false,
    irqMapData: { size: 630 },
    isCurrentPane: false,
    jankFramesData: [],
    jsCpuProfilerData: [],
    jsMemory: [],
    leftNs: 12120099973,
    memoryAbilityIds: [],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [],
    perfAll: false,
    cpuStateFilterIds: [],
    cpus: [],
    diskAbilityIds: [],
    diskIOLatency: false,
    perfCpus: [],
    perfProcess: [],
    perfSampleIds: [],
    perfThread: [],
    purgeableTotalAbility: [1, 2],
    purgeableTotalSelection: [],
    purgeableTotalVM: [4, 1],
    recordStartNs: 396522210669149,
    rightNs: 21021096280,
    sdkCounterIds: [],
    sdkSliceIds: [],
    smapsType: [],
    startup: false,
    staticInit: false,
    statisticsSelectData: undefined,
    powerEnergy: [],
    processIds: [],
    processTrackIds: [],
    promiseList: [],
    purgeablePinAbility: [],
    purgeablePinSelection: [],
    purgeablePinVM: [],
  };

  it('TabPanePurgTotalTest01', function () {
    expect(
      tabPanePurgTotal.sortByColumn({
        key: 'avgSize',
      })
    ).toBeUndefined();
  });
});
