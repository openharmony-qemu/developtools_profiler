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
import { TabPaneJsCpuCallTree } from '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneJsCpu.js';
import '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneJsCpu.js';
import { JsCpuProfilerStatisticsStruct } from '../../../../../../dist/trace/bean/JsStruct.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    recycleDataSource: () => {},
    removeAttribute: () => {},
    reMeauseHeight: () => {},
    addEventListener: () => {},
  };
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
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
describe('TabPaneJsCpuCallTree Test', () => {
  document.body.innerHTML = `<tabpane-js-cpu id="statistics"></tabpane-js-cpu>`;
  let tabPaneJsCpu = document.querySelector<TabPaneJsCpuCallTree>('#statistics');
  it('TabPaneJsCpuCallTree01', () => {
    tabPaneJsCpu.init = jest.fn(() => true);
    tabPaneJsCpu.data = {
      rightNs: 5,
      leftNs: 1,
      cpus: [],
      threadIds: [],
      trackIds: [6],
      funTids: [111, 4, 43],
      heapIds: [5, 67, 0],
      nativeMemory: [],
      cpuAbilityIds: [],
      memoryAbilityIds: [],
      diskAbilityIds: [88, 7],
      networkAbilityIds: [],
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: true,
    };
    expect(tabPaneJsCpu).toBeTruthy();
  });
  it('TabPaneJsCpuCallTree02', () => {
    expect(tabPaneJsCpu.setCallTreeTableData).toBeUndefined();
  });
});
