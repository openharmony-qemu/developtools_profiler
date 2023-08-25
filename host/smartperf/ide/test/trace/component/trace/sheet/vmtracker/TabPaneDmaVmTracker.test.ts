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
import { TabPaneDmaVmTracker } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneDmaVmTracker.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
  return {};
});

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
describe('TabPaneDmaSelectVmTracker Test', () => {
  let dmaVmTracker = new TabPaneDmaVmTracker();
  let val = [
    {
      leftNs: 0,
      rightNs: 1030,
      startNs: 0,
    },
  ];
  let dmaData = sqlite.getTabDmaVmTrackerData;
  let data = [
    {
      startNs: 0,
      expTaskComm: 'aaa',
      sumSize: 100,
      maxSize: 100,
      minSize: 10,
      avgSize: 'aaa',
    },
  ];
  dmaData.mockResolvedValue(data);
  it('TabPaneDmaSelectVmTracker01', () => {
    expect(dmaVmTracker.sortDmaByColumn('', 0)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker05', () => {
    expect(dmaVmTracker.sortDmaByColumn('avgSize', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker06', () => {
    expect(dmaVmTracker.sortDmaByColumn('minSize', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker07', () => {
    expect(dmaVmTracker.sortDmaByColumn('maxSize', 1)).toBeUndefined();
  });
  it('TabPaneDmaSelectVmTracker08', () => {
    expect(dmaVmTracker.queryDataByDB(val)).toBeUndefined();
  });
});
