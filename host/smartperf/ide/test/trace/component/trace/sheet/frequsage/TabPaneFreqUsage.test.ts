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
import { TabPaneFreqUsage } from '../../../../../../dist/trace/component/trace/sheet/frequsage/TabPaneFreqUsage.js';

jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    snapshotDataSource: () => {
    },
    removeAttribute: () => {
    },
  };
});
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
describe('TabPaneFreqUsage Test', () => {
  let tabPaneFreqUsage = new TabPaneFreqUsage();
  let data = {
    leftNs: 0,
    rightNs: 0,
    processIds: [1, 2],
  };
  let getTabRunningPercent = sqlite.getTabRunningPercent;
  getTabRunningPercent.mockResolvedValue([
    {
      pid: 1,
      tid: 1,
      state: 'Running',
      cpu: 0,
      dur: 0,
      ts: 1,
    }
  ]);
  let queryCpuFreqFilterId = sqlite.queryCpuFreqFilterId;
  queryCpuFreqFilterId.mockResolvedValue([{
    id: 1,
    cpu: 0,
  }]);
  let queryCpuFreqUsageData = sqlite.queryCpuFreqUsageData;
  queryCpuFreqUsageData.mockResolvedValue([{
    value: '',
    dur: '',
    startNS: 0,
    filter_id: 1,
  }]);
  it('TabPaneFreqUsageTest01 ', function () {
    tabPaneFreqUsage.data = data;
    expect(tabPaneFreqUsage.data).toBeUndefined();
  });
  it('TabPaneFreqUsageTest02', () => {
    let tabPaneFreqUsage = new TabPaneFreqUsage();
    const threadStatesTblSource = [
      {process: ' ', value: 3},
      {process: 'ABC', value: 2},
      {process: 'XYZ', value: 1},
    ];
    tabPaneFreqUsage.sortByColumn({key: 'value', sort: 1}, threadStatesTblSource);
    expect(threadStatesTblSource).toEqual([
      {
        'process': ' ',
        'value': 3
      },
      {
        'process': 'ABC',
        'value': 2
      },
      {
        'process': 'XYZ',
        'value': 1
      }
    ]);
  });
});