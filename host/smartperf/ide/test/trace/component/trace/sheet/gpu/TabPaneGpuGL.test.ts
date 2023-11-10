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
import { TabPaneGpuGL } from '../../../../../../dist/trace/component/trace/sheet/gpu/TabPaneGpuGL.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

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

describe('TabPaneGpuGL Test', () => {
  document.body.innerHTML = `<div><tabpane-gpu-gl id="tree"></tabpane-gpu-gl></div>`;
  let tabPaneGpuGL = document.querySelector<TabPaneGpuGL>('#tree');
  let queryGpuGLDataByRange = sqlite.queryGpuGLDataByRange;
  queryGpuGLDataByRange.mockResolvedValue([
    {
      startTs: 23,
      size: 10,
    },
    {
      startTs: 213,
      size: 110,
    },
  ]);
  it('TabPaneGpuGLTest01', () => {
    tabPaneGpuGL.data = {
      leftNs: 0,
      rightNs: 1,
    };
    expect(tabPaneGpuGL.data).toStrictEqual({ leftNs: 0, rightNs: 1 });
  });
});
