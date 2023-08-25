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
import { TabPaneIoCompletionTimes } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIoCompletionTimes.js';
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
Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
import crypto from 'crypto';
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPaneIoCompletionTimes Test', () => {
  let ioCompletionTimesTypeData = sqlite.getTabIoCompletionTimesType;
  let ioCompletionData = [
    {
      tier: 1,
    },
  ];
  ioCompletionTimesTypeData.mockResolvedValue(ioCompletionData);
  let tabPaneIoCompletionTimes = new TabPaneIoCompletionTimes();
  let filter = new TabPaneFilter();
  filter.getFilterData = jest.fn(() => true);
  it('TabPaneIoCompletionTimes01', function () {
    expect(tabPaneIoCompletionTimes.sortioCompletionTimesTable('', 0)).toBeUndefined();
  });
  it('TabPaneIoCompletionTimes02', function () {
    expect(tabPaneIoCompletionTimes.sortioCompletionTimesTable('startTsStr', 1)).toBeUndefined();
  });
  it('TabPaneIoCompletionTimes06', function () {
    let val = [
      {
        leftNs: 10,
        rightNs: 2000,
      },
    ];
    expect(tabPaneIoCompletionTimes.initFilterTypes(val)).toBeTruthy();
  });
  it('TabPaneIoCompletionTimes07', function () {
    let val = [
      {
        leftNs: 10,
        rightNs: 2000,
        fileSystemIoData: 'aa',
      },
    ];
    expect(tabPaneIoCompletionTimes.fromStastics(val)).toBeTruthy();
  });
  it('TabPaneIoCompletionTimes08', function () {
    let val = [
      {
        leftNs: 10,
        rightNs: 2000,
      },
    ];
    expect(tabPaneIoCompletionTimes.queryData(val)).toBeUndefined();
  });
  it('TabPaneIoCompletionTimes09', function () {
    let val = [
      {
        pid: 10,
        tid: 100,
        type: 0,
      },
    ];
    expect(tabPaneIoCompletionTimes.filterTypeData(val)).toBeUndefined();
  });
});
