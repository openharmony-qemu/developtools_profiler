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
import { TabPaneVmTrackerShmComparison } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneVmTrackerShmComparison.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    snapshotDataSource: () => {},
    removeAttribute: () => {},
  };
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js', () => {
  return {
    ns2s: () => {},
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

describe('TabPaneVmTrackerShmComparison Test', () => {
  let tabPaneVmTrackerShmComparison = new TabPaneVmTrackerShmComparison();
  let getVmTrackerShmSelectionData = sqlite.queryVmTrackerShmSelectionData;
  getVmTrackerShmSelectionData.mockResolvedValue([
    {
      startNS: 25165824,
      ipid: 1,
      fd: 2,
      size: 1000,
      adj: 10,
      name: 1,
      id: 4,
      time: 2,
      purged: 20,
      count: 2,
      flag: 0,
    },
  ]);
  let data = [
    {
      name: 'Snapshot0',
      startNs: 8214061,
      value: 0,
    },
  ];
  let datalist = [
    {
      name: 'Snapshot2',
      startNs: 9806561,
      value: 0,
    },
    {
      name: 'Snapshot1',
      startNs: 47781,
      value: 0,
    },
  ];
  tabPaneVmTrackerShmComparison.init = jest.fn(() => true);
  it('TabPaneVmTrackerShmComparison01', function () {
    tabPaneVmTrackerShmComparison.initSelect = jest.fn(() => true);
    expect(tabPaneVmTrackerShmComparison.setShmData(data, datalist)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmComparison02', function () {
    expect(tabPaneVmTrackerShmComparison.updateComparisonData(0, 10)).toBeTruthy();
  });
  it('TabPaneVmTrackerShmComparison03', function () {
    expect(tabPaneVmTrackerShmComparison.calSizeObj(data, datalist)).toBeTruthy();
  });
});
