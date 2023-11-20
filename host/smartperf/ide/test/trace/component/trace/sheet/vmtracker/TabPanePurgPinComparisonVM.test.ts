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
import { TabPanePurgPinComparisonVM } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPanePurgPinComparisonVM.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
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

describe('TabPanePurgPinComparisonVM Test', () => {
  let tabPanePurgPinComparisonVM = new TabPanePurgPinComparisonVM();
  let queryProcessPurgeableSelectionTab = sqlite.queryProcessPurgeableSelectionTab;
  queryProcessPurgeableSelectionTab.mockResolvedValue([
    {
      value: 25165824,
      name: '24.00MB',
    },
    {
      value: 25165824,
      name: '25.00MB',
    },
    {
      value: 25165824,
      name: '26.00MB',
    },
  ]);
  let data = [
    {
      name: 'Snapshot0',
      startNs: 4778211,
      value: 0,
    },
  ];
  let datalist = [
    {
      name: 'Snapshot2',
      startNs: 98526561,
      value: 0,
    },
    {
      name: 'Snapshot1',
      startNs: 48214061,
      value: 0,
    },
  ];
  tabPanePurgPinComparisonVM.init = jest.fn(() => true);
  it('TabPanePurgPinComparisonVM01', function () {
    expect(tabPanePurgPinComparisonVM.updateComparisonData(0, 1000)).toBeTruthy();
  });
  it('TabPanePurgPinComparisonVM02', function () {
    expect(tabPanePurgPinComparisonVM.queryPinVMData(0, 1000)).toBeTruthy();
  });
  it('TabPanePurgPinComparisonVM03', function () {
    tabPanePurgPinComparisonVM.initSelect = jest.fn(() => true);
    expect(tabPanePurgPinComparisonVM.totalData(data, datalist)).toBeUndefined();
  });
});
