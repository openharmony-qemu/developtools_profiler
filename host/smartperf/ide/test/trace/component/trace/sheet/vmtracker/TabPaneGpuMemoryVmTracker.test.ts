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
import { TabPaneGpuMemoryVmTracker } from '../../../../../../dist/trace/component/trace/sheet/vmtracker/TabPaneGpuMemoryVmTracker.js';

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
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
describe('TabPaneGpuMemoryVmTracker Test', () => {
  let gpuMemoryVmTracker = new TabPaneGpuMemoryVmTracker();
  let val = [
    {
      leftNs: 0,
      rightNs: 1000,
    },
  ];
  let gpuMemoryData = sqlite.getTabGpuMemoryData;
  let gpuData = [
    {
      startNs: 0,
      gpuName: 'aa',
      threadId: 1,
      threadName: 'bb',
      sumSize: 0,
      maxSize: 0,
      minSize: 0,
      avgSize: 0,
    },
  ];
  gpuMemoryData.mockResolvedValue(gpuData);
  it('TabPaneGpuMemoryVmTracker01', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('gpuName', 1)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker02', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('', 0)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker04', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('avgSize', 1)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker05', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('minSize', 1)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker06', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('maxSize', 1)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker07', () => {
    expect(gpuMemoryVmTracker.sortGpuMemoryByColumn('thread', 1)).toBeUndefined();
  });
  it('TabPaneGpuMemoryVmTracker08', () => {
    expect(gpuMemoryVmTracker.queryDataByDB(val)).toBeUndefined();
  });
});
