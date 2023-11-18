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
import { TabPaneDmaVmTrackerComparison } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneDmaVmTrackerComparison.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    snapshotDataSource: () => {},
    removeAttribute: () => {},
  };
});

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneDmaVmTrackerComparison Test', () => {
  let tabPaneDmaVmTrackerComparison = new TabPaneDmaVmTrackerComparison();
  let getTabDmaVmTrackerComparisonData = sqlite.getTabDmaVmTrackerComparisonData;
  let dmaVmTrackerData = [
    {
      startNs: 0,
      value: 100,
    },
  ];
  let datalist = [
    {
      name: 'Snapshot2',
      startNs: 980052,
      type: 'VmTracker',
      value: 0,
    },
    {
      name: 'Snapshot1',
      startNs: 478261,
      type: 'VmTracker',
      value: 0,
    },
  ];
  tabPaneDmaVmTrackerComparison.init = jest.fn(() => true);
  getTabDmaVmTrackerComparisonData.mockResolvedValue(dmaVmTrackerData);
  it('TabPaneDmaVmTrackerComparison01', function () {
    expect(tabPaneDmaVmTrackerComparison.queryDataByDB(10)).toBeTruthy();
  });
  it('TabPaneDmaVmTrackerComparison03', function () {
    expect(tabPaneDmaVmTrackerComparison.comparisonDataByDB(10, datalist)).toBeTruthy();
  });
  it('TabPaneDmaVmTrackerComparison04', function () {
    expect(tabPaneDmaVmTrackerComparison.selectStamps(datalist)).toBeUndefined();
  });
});
