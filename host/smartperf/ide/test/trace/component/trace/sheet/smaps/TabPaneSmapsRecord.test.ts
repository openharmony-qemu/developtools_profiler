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
// @ts-ignore
import { Smaps } from '../../../../../../dist/trace/bean/SmapsStruct.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {};
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
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
describe('TabPaneSmapsRecord Test', () => {
  let MockgetTabSmapsData = sqlit.getTabSmapsData;
  MockgetTabSmapsData.mockResolvedValue([
    {
      tsNS: 0,
      start_addr: 'start_addr',
      end_addr: 'end_addr',
      dirty: 0,
      swap: 0,
      rss: 0,
      pss: 0,
      size: 1,
      reside: 1,
      permission: 'rw-',
      path: 'path',
      shared_dirty: 1,
      private_clean: 1,
      shared_clean: 2,
      private_dirty: 1,
      swap_pss: 2,
    },
  ]);

  let tabPaneSmapsRecord = new TabPaneSmapsRecord();
  tabPaneSmapsRecord.init = jest.fn(() => true);
  let smaps = new Smaps();
  smaps.tsNS = -1;
  smaps.start_addr = 'aaaaa';
  smaps.end_addr = 'bbbbb';
  smaps.permission = 'dddd';
  smaps.path = '/asdasdas';
  smaps.size = 0;
  smaps.rss = 0;
  smaps.pss = 0;
  smaps.reside = 0;
  smaps.dirty = 0;
  smaps.swapper = 0;
  smaps.address = 'aaaaa-bbbbb';
  smaps.type = 'Dta';
  smaps.dirtyStr = '1212';
  smaps.swapperStr = '222';
  smaps.rssStr = '333';
  smaps.pssStr = '444';
  smaps.sizeStr = '555';
  smaps.resideStr = '666';
  smaps.pss = 2;
  smaps.typeName = 'aaa';
  smaps.sharedCleanStr = 'aaa';
  smaps.sharedDirtyStr = 'aaa';
  smaps.privateCleanStr = 'ab';
  smaps.type = 1;
  smaps.sharedClean = 1;
  smaps.sharedDirty = 2;
  smaps.privateClean = 3;
  let result = [smaps, smaps];

  it('tabPaneSmapsRecord01', function () {
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
  it('tabPaneSmapsRecord03', () => {
    expect(tabPaneSmapsRecord.filteredData(result)).toBeUndefined();
  });
});
