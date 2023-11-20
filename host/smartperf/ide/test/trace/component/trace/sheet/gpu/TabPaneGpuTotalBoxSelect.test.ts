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
import { TabPaneGpuTotalBoxSelect } from '../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuTotalBoxSelect.js';

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    unobserve: jest.fn(),
    disconnect: jest.fn(),
    observe: jest.fn(),
  }));

describe('TabPaneGpuTotalBoxSelect Test', () => {
  document.body.innerHTML = `<div><tabpane-gpu-total-box-select id="tree"></tabpane-gpu-total-box-select></div>`;
  let tabPaneGpuTotalBoxSelect = document.querySelector<TabPaneGpuTotalBoxSelect>('#tree');
  let queryGpuDataByRange = sqlite.queryGpuDataByRange;
  queryGpuDataByRange.mockResolvedValue([
    {
      startTs: 23,
      windowId: 1,
      moduleId: 2,
      categoryId: 0,
      sumSize: 10,
      avgSize: 1,
      maxSize: 1,
      minSize: 0,
    },
    {
      startTs: 23,
      windowId: 1,
      moduleId: 2,
      categoryId: 0,
      sumSize: 10,
      avgSize: 1,
      maxSize: 1,
      minSize: 0,
    },
  ]);
  it('TabPaneGpuTotalBoxSelectTest01', () => {
    tabPaneGpuTotalBoxSelect.data = {
      type: '',
      startTs: 1,
    };
    expect(tabPaneGpuTotalBoxSelect.data).toBeUndefined();
  });
  it('TabPaneGpuTotalBoxSelectTest02', () => {
    let tabPaneGpuTotalBoxSelects = new TabPaneGpuTotalBoxSelect();
    tabPaneGpuTotalBoxSelects.gpuBoxTbl = jest.fn(() => true);
    expect(
      tabPaneGpuTotalBoxSelects.sortByColumn({
        sort: 0,
      })
    ).toBeUndefined();
  });
});
