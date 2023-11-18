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
import { TabPaneCpuByProcess } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCpuByProcess.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
describe('TabPaneCpuByProcess Test', () => {
  let tabPaneCpuByProcess = new TabPaneCpuByProcess();

  it('TabPaneCpuByProcessTest01', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        key: 'number',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByProcessTest05', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByProcessTest04', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        key: 'pid' || 'wallDuration' || 'avgDuration' || 'occurrences',
      })
    ).toBeUndefined();
  });
});
