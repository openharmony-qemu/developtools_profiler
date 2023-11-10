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
import { TabPaneDmaSelectVmTracker } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneDmaSelectVmTracker.js';

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPaneDmaSelectVmTracker Test', () => {
  let tabPaneDmaSelectVmTracker = new TabPaneDmaSelectVmTracker();
  let val = [
    {
      leftNs: 0,
      rightNs: 1040,
      startNs: 0,
    },
  ];
  let dmaSelectionData = sqlite.getTabDmaVMTrackerClickData;
  let selectiondata = [
    {
      startNs: 0,
      fd: 1,
      size: 100,
      ino: 0,
      expPid: 0,
      bufName: 'aaa',
      expName: 'aaa',
      expTaskComm: 'bb',
      flag: 1,
    },
  ];
  dmaSelectionData.mockResolvedValue(selectiondata);
  it('TabPaneDmaSelectVmTracker01', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('', 0)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker02', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('startNs', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker03', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('expTaskComm', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker04', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('fd', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker05', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('size', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker06', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('ino', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker07', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('expPid', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker08', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('flag', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker09', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('bufName', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker10', () => {
    expect(tabPaneDmaSelectVmTracker.sortDmaByColumn('expName', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker11', () => {
    tabPaneDmaSelectVmTracker.init = jest.fn(() => true);
    expect(tabPaneDmaSelectVmTracker.queryDmaVmTrackerClickDataByDB(val)).toBeUndefined();
  });
});
