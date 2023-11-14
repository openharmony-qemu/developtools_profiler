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
import { TabPaneVmTrackerShmSelection } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneVmTrackerShmSelection.js';

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});
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
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneVmTrackerShmSelection Test', () => {
  document.body.innerHTML = `<tab-pane-shm id="ts"> </tab-pane-shm>`;
  let tabPaneVmTrackerShmSelection = new TabPaneVmTrackerShmSelection();
  let queryVmTrackerShmSelectionData = sqlite.queryVmTrackerShmSelectionData;
  queryVmTrackerShmSelectionData.mockResolvedValue([
    {
      startNs: 0,
      ipid: 1,
      fd: 123,
      size: 12,
      adj: 3,
      name: 'NAME',
      id: 1,
      time: 12333,
      purged: 6,
      count: 3,
      flag: 0,
    },
  ]);
  let data = {
    startNs: 0,
    endNs: 3421,
    dur: 3421,
    name: '',
    textWidth: 0,
    value: 0,
    type: '',
  };
  let dataList = [
    {
      startNs: 0,
      endNs: 0,
      dur: 0,
      name: 'b',
      textWidth: 0,
      value: 0,
      type: 'b',
    },
    {
      startNs: 1,
      endNs: 2,
      dur: 1,
      name: 'a',
      textWidth: 0,
      value: 0,
      type: 'a',
    },
  ];
  it('TabPaneVmTrackerShmSelection01', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('ts', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection02', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('pid', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection03', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('fd', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection04', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('sizeStr', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection05', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('adj', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection06', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('name', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection07', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('id', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection08', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('time', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection09', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('count', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection10', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('purged', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection11', () => {
    expect(tabPaneVmTrackerShmSelection.sortByColumn('flag', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection12', () => {
    expect(tabPaneVmTrackerShmSelection.clear()).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection13', () => {
    tabPaneVmTrackerShmSelection.init = jest.fn(() => true);
    tabPaneVmTrackerShmSelection.clear = jest.fn(() => true);
    tabPaneVmTrackerShmSelection.queryDataByDB = jest.fn(() => true);
    expect(tabPaneVmTrackerShmSelection.setShmData(data, dataList)).toBeUndefined();
  });
  it('TabPaneVmTrackerShmSelection14', () => {
    expect(tabPaneVmTrackerShmSelection.queryDataByDB(data)).toBe(true);
  });
});
