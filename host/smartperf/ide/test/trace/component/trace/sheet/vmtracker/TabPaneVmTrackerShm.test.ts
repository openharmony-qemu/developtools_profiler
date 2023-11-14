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
import { TabPaneVmTrackerShm } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneVmTrackerShm.js';

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
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

describe('TabPaneVmTrackerShm Test', () => {
  let tabPaneVmTrackerShm = new TabPaneVmTrackerShm();
  let shmData = sqlite.queryVmTrackerShmSizeData;
  let gpuData = [
    {
      startNS: 200,
      pid: 1,
      ashmname: 1,
      avg: 4,
      sum: 5,
      max: 8,
      min: 1,
    },
  ];
  shmData.mockResolvedValue(gpuData);
  let val = [
    {
      leftNs: 0,
      rightNs: 1000,
    },
  ];
  it('TabPaneVmTrackerShm01', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('time', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm02', () => {
    expect(tabPaneVmTrackerShm.clear()).toBeUndefined();
  });
  it('TabPaneVmTrackerShm03', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('pid', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm04', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('minSizeStr', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm05', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('ashmname', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm06', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('avgSizeStr', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm07', () => {
    expect(tabPaneVmTrackerShm.sortByColumn('maxSizeStr', 1)).toBeUndefined();
  });
  it('TabPaneVmTrackerShm08', () => {
    expect(tabPaneVmTrackerShm.queryDataByDB(val)).toBeUndefined();
  });
});
