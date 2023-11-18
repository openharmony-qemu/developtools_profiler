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
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
//@ts-ignore
import { TabPaneGpuWindowBoxSelect } from '../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuWindowBoxSelect.js';

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

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

describe('TabPaneGpuWindowBoxSelect Test', () => {
  document.body.innerHTML = `<div><tabpane-gpu-window-box-select id="tree"></tabpane-gpu-window-box-select></div>`;
  let tabPaneGpuWindowBoxSelect = document.querySelector<TabPaneGpuWindowBoxSelect>('#tree');
  let queryGpuDataByRange = sqlite.queryGpuDataByRange;
  queryGpuDataByRange.mockResolvedValue([
    {
      startTs: 34,
      windowId: 53,
      moduleId: 2,
      categoryId: 0,
      sumSize: 20,
      avgSize: 1,
      maxSize: 11,
      minSize: 0,
    },
    {
      startTs: 48,
      windowId: 1,
      moduleId: 28,
      categoryId: 0,
      sumSize: 51,
      avgSize: 1,
      maxSize: 1,
      minSize: 0,
    },
  ]);
  it('TabPaneGpuWindowBoxSelectTest01', () => {
    tabPaneGpuWindowBoxSelect.data = {
      type: '',
      startTs: 1,
    };
    expect(tabPaneGpuWindowBoxSelect.data).toBeUndefined();
  });
  it('TabPaneGpuWindowBoxSelectTest02', () => {
    let tabPaneGpuWindowBoxSelects = new TabPaneGpuWindowBoxSelect();
    tabPaneGpuWindowBoxSelects.gpuBoxTbl = jest.fn(() => true);
    expect(
      tabPaneGpuWindowBoxSelects.sortByColumn({
        sort: 0,
      })
    ).toBeUndefined();
  });
  it('TabPaneGpuWindowBoxSelectTest03', () => {
    let tabPaneGpuWindowBoxSelects = new TabPaneGpuWindowBoxSelect();
    tabPaneGpuWindowBoxSelects.gpuBoxTbl = jest.fn(() => true);
    expect(
      tabPaneGpuWindowBoxSelects.sortByColumn({
        sort: 1,
      })
    ).toBeUndefined();
  });
});
