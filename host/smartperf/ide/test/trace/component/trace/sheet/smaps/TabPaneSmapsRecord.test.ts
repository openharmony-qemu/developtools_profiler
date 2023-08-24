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
import { TabPaneSmapsRecord } from '../../../../../../dist/trace/component/trace/sheet/smaps/TabPaneSmapsRecord.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPaneSmapsRecord Test', () => {
  let MockgetTabSmapsData = sqlit.getTabSmapsData;
  MockgetTabSmapsData.mockResolvedValue([
    {
      tsNS: 0,
      start_addr: 'start_addr',
      end_addr: 'end_addr',
      dirty: 0,
      swapper: 0,
      rss: 0,
      pss: 0,
      size: 1,
      reside: 1,
      permission: 'rw-',
      path: 'path',
    },
  ]);

  let tabPaneSmapsRecord = new TabPaneSmapsRecord();
  tabPaneSmapsRecord.data = {
    leftNs: 0,
    rightNs: 500,
    smapsType: [0, 1, 2],
  };

  it('tabPaneSmapsRecord01', function () {
    tabPaneSmapsRecord.tbl = jest.fn(() => true);
    tabPaneSmapsRecord.tbl!.recycleDataSource = jest.fn(() => true);
    expect(
      tabPaneSmapsRecord.sortByColumn({
        key: '',
        sort: '',
      })
    ).toBeUndefined();
  });

  it('tabPaneSmapsRecord02', () => {
    expect(tabPaneSmapsRecord.initElements()).toBeUndefined();
  });
});
