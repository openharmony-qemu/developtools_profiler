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
import { TabPanePurgPinSelection } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgPinSelection.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
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

describe('TabPanePurgPin Test', () => {
  let tabPanePurgPinSelection = new TabPanePurgPinSelection();
  let querySysPurgeableSelectionTab = sqlit.querySysPurgeableSelectionTab;
  querySysPurgeableSelectionTab.mockResolvedValue([
    {
      value: 25165824,
      name: '24.00MB',
    },
    {
      value: 25165824,
      name: '24.00MB',
    },
    {
      value: 25165824,
      name: '24.00MB',
    },
  ]);
  let queryProcessPurgeableSelectionTab = sqlit.queryProcessPurgeableSelectionTab;
  queryProcessPurgeableSelectionTab.mockResolvedValue([
    {
      value: 25165824,
      name: '24.00MB',
    },
    {
      value: 25165824,
      name: '24.00MB',
    },
    {
      value: 25165824,
      name: '24.00MB',
    },
  ]);
  tabPanePurgPinSelection.data = {
    anomalyEnergy: [],
    clockMapData: { size: 120 },
    cpuAbilityIds: [],
    cpuFreqFilterIds: [],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [],
    cpus: [],
    gpu: { gl: false, gpuWindow: false, gpuTotal: false },
    gpuMemoryAbilityData: [],
    gpuMemoryTrackerData: [],
    hasFps: false,
    irqMapData: { size: 965 },
    diskAbilityIds: [],
    diskIOLatency: false,
    diskIOReadIds: [],
    diskIOWriteIds: [],
    diskIOipids: [],
    dmaAbilityData: [],
    dmaVmTrackerData: [],
    fsCount: 410,
    funAsync: [],
    funTids: [],
    isCurrentPane: false,
    jankFramesData: [],
    jsCpuProfilerData: [],
    jsMemory: [],
    leftNs: 2199973,
    memoryAbilityIds: [],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [],
    perfAll: false,
    perfCpus: [],
    perfProcess: [],
    perfSampleIds: [],
    perfThread: [],
    powerEnergy: [],
    purgeablePinVM: [],
    purgeableTotalAbility: [],
    purgeableTotalSelection: [],
    purgeableTotalVM: [],
    recordStartNs: 3546410021149,
    rightNs: 2698537796280,
    sdkCounterIds: [],
    sdkSliceIds: [],
    smapsType: [],
    startup: false,
    staticInit: false,
    statisticsSelectData: undefined,
    processIds: [],
    processTrackIds: [],
    promiseList: [],
    purgeablePinAbility: [],
    purgeablePinSelection: [],
  };

  it('tabPanePurgPinSelectionTest01', function () {
    expect(tabPanePurgPinSelection.data).toBeUndefined();
  });
  it('tabPanePurgPinSelectionTest02', function () {
    expect(tabPanePurgPinSelection.queryTableData('ability', 10)).toBeTruthy();
  });
  it('tabPanePurgPinSelectionTest03', function () {
    expect(tabPanePurgPinSelection.queryTableData('VM', 10)).toBeTruthy();
  });
});
