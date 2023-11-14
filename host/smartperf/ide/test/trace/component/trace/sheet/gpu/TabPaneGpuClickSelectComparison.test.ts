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
import { TabPaneGpuClickSelectComparison } from '../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuClickSelectComparison.js';
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
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
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

describe('TabPaneGpuClickSelectComparison Test', () => {
  document.body.innerHTML = `<div><tabpane-gpu-click-select-comparsion id="tree"></tabpane-gpu-click-select-comparsion></div>`;
  let tabPaneGpuClickSelectComparison = document.querySelector<TabPaneGpuClickSelectComparison>('#tree');
  let queryGpuDataByTs = sqlite.queryGpuDataByTs;
  queryGpuDataByTs.mockResolvedValue([
    {
      windowId: 1,
      moduleId: 2,
      categoryId: 0,
      size: 123,
    },
    {
      windowId: 7,
      moduleId: 8,
      categoryId: 2,
      size: 1213,
    },
  ]);
  it('TabPaneGpuClickSelectComparisonTest01', () => {
    let type = 'total';
    let startTs = 10;
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.queryDataByDB(type, startTs)).toBeTruthy();
  });
  it('TabPaneGpuClickSelectComparisonTest02', () => {
    let type = 'total';
    let targetStartNs = 10;
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.getComparisonData(targetStartNs, type)).toBeTruthy();
  });
  it('TabPaneGpuClickSelectComparisonTest03', () => {
    let type = 'total';
    let dataList = [
      {
        name: 'Snapshot2',
        startNs: 9800526561,
        value: 0,
      },
      {
        name: 'Snapshot1',
        startNs: 4778214061,
        value: 0,
      },
    ];
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.selectStamps(dataList, type)).toBeUndefined();
  });
  it('TabPaneGpuClickSelectComparisonTest04', () => {
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.sortGpuByColumn(0, '')).toBeUndefined();
  });
  it('TabPaneGpuClickSelectComparisonTest05', () => {
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.sortGpuByColumn(1, 'name')).toBeUndefined();
  });
  it('TabPaneGpuClickSelectComparisonTest06', () => {
    let tabPaneGpuClickSelectComparison = new TabPaneGpuClickSelectComparison();
    expect(tabPaneGpuClickSelectComparison.sortGpuByColumn(1, 'sizeDelta')).toBeUndefined();
  });
});
