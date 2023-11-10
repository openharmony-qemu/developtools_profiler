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
import { TabPaneCpuFreqLimits } from '../../../../../../dist/trace/component/trace/sheet/freq/TabPaneCpuFreqLimits.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {};
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPaneCpuFreqLimits Test', () => {
  let tabPaneCpuFreqLimits = new TabPaneCpuFreqLimits();
  it('TabPaneCpuFreqLimits01', function () {
    expect(tabPaneCpuFreqLimits.sortCpuFreqLimitTable('timeStr', 1)).toBeUndefined();
  });
  it('TabPaneCpuFreqLimits02', function () {
    expect(tabPaneCpuFreqLimits.sortCpuFreqLimitTable('valueStr', 1)).toBeUndefined();
  });
  it('TabPaneCpuFreqLimits03', function () {
    expect(tabPaneCpuFreqLimits.sortCpuFreqLimitTable('cpu', 1)).toBeUndefined();
  });
  it('TabPaneCpuFreqLimits04', function () {
    expect(tabPaneCpuFreqLimits.sortCpuFreqLimitTable('type', 1)).toBeUndefined();
  });
});
