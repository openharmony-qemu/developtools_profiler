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
import { TabPaneGpuClickSelect } from '../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuClickSelect.js';

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuClickSelectComparison.js', () => {
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

describe('TabPaneGpuClickSelect Test', () => {
  document.body.innerHTML = `<div><tabpane-gpu-click-select id="tree"></tabpane-gpu-click-select></div>`;
  let tabPaneGpuClickSelect = document.querySelector<TabPaneGpuClickSelect>('#tree');
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
  it('TabPaneGpuClickSelectTest01', () => {
    let data = {
      type: '',
      startTs: 1,
    };
    expect(tabPaneGpuClickSelect.gpuClickData(data)).toBeUndefined();
  });
  it('TabPaneGpuClickSelectTest02', () => {
    let tabPaneGpuClickSelects = new TabPaneGpuClickSelect();
    tabPaneGpuClickSelects.gpuTbl = jest.fn(() => true);
    expect(
      tabPaneGpuClickSelects.sortByColumn({
        sort: 0,
      })
    ).toBeUndefined();
  });
});
