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
import { TabPanePurgTotalComparisonVM } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPanePurgTotalComparisonVM.js';
import '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgPinComparisonAbility.js';

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

describe('TabPanePurgTotalComparisonVM Test', () => {
  let tabPanePurgTotalComparisonVM = new TabPanePurgTotalComparisonVM();
  let queryProcessPurgeableSelectionTab = sqlite.queryProcessPurgeableSelectionTab;
  queryProcessPurgeableSelectionTab.mockResolvedValue([
    {
      value: 21365824,
      name: '24.00MB',
    },
    {
      value: 221824,
      name: '24.00MB',
    },
    {
      value: 2571824,
      name: '24.00MB',
    },
  ]);
  let data = [
    {
      name: 'Snapshot0',
      startNs: 172161,
      value: 0,
    },
  ];
  let datalist = [
    {
      name: 'Snapshot2',
      startNs: 2826561,
      type: 'ability',
      value: 0,
    },
    {
      name: 'Snapshot1',
      startNs: 5714061,
      type: 'ability',
      value: 0,
    },
  ];
  tabPanePurgTotalComparisonVM.init = jest.fn(() => true);
  it('TabPanePurgPinComparisonAbility01', function () {
    tabPanePurgTotalComparisonVM.initSelect = jest.fn(() => true);
    expect(tabPanePurgTotalComparisonVM.totalData(data, datalist)).toBeUndefined();
  });
  it('TabPanePurgTotalComparisonVM01', function () {
    expect(tabPanePurgTotalComparisonVM.updateComparisonData(0, 1000)).toBeTruthy();
  });
  it('TabPanePurgTotalComparisonVM02', function () {
    expect(tabPanePurgTotalComparisonVM.queryTotalVMData(0, 1000)).toBeTruthy();
  });
});
