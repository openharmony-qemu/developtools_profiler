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
import { SpAbilityMonitorChart } from '../../../../dist/trace/component/chart/SpAbilityMonitorChart.js';
import '../../../../dist/trace/component/chart/SpAbilityMonitorChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
const intersectionObserverMock = () => ({
  observe: () => null,
});
jest.mock('../../../../dist/trace/database/SqlLite.js');
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    observe: jest.fn(),
    unobserve: jest.fn(),
    disconnect: jest.fn(),
  }));
describe('SpAbilityMonitorChart Test', () => {
  let MockqueryAbilityExits = sqlit.queryAbilityExits;
  MockqueryAbilityExits.mockResolvedValue([
    {
      event_name: 'trace_cpu_usage',
      stat_type: 'received',
      count: 1,
    },
    {
      event_name: 'sys_memory',
      stat_type: 'received',
      count: 1,
    },
    {
      event_name: 'trace_diskio',
      stat_type: 'received',
      count: 1,
    },
    {
      event_name: 'trace_diskio',
      stat_type: 'received',
      count: 1,
    },
  ]);
  let cpudata = sqlit.queryCPuAbilityMaxData;
  cpudata.mockResolvedValue([
    {
      totalLoad: 1,
      userLoad: 1,
      systemLoad: 1,
    },
  ]);
  let memorydata = sqlit.queryMemoryMaxData;
  memorydata.mockResolvedValue([
    {
      maxValue: 1,
      filter_id: 1,
    },
  ]);

  let queryDiskIo = sqlit.queryDiskIoMaxData;
  queryDiskIo.mockResolvedValue([
    {
      bytesRead: 1,
      bytesWrite: 1,
      readOps: 1,
      writeOps: 1,
    },
  ]);

  let netWorkDiskIo = sqlit.queryNetWorkMaxData;
  netWorkDiskIo.mockResolvedValue([
    {
      maxIn: 1,
      maxOut: 1,
      maxPacketIn: 1,
      maxPacketOut: 1,
    },
  ]);
  let queryDmaAbilityData = sqlit.queryDmaAbilityData;
  queryDmaAbilityData.mockResolvedValue([
    {
      startNs: 1,
      value: 1,
      flag: 1,
      ipid: 1,
      expTaskComm: '',
    },
  ]);
  let queryGpuMemoryAbilityData = sqlit.queryGpuMemoryAbilityData;
  queryGpuMemoryAbilityData.mockResolvedValue([
    {
      startNs: 1,
      value: 1,
    },
  ]);
  let queryPurgeableSysData = sqlit.queryPurgeableSysData;
  queryPurgeableSysData.mockResolvedValue([
    {
      startNs: 1,
      value: 1,
    },
  ]);

  let purgeableSysData = sqlit.queryPurgeableSysData;
  purgeableSysData.mockResolvedValue([
    {
      name: 'test',
      startNs: 15255,
      value: 0,
    },
  ]);

  let dmaAbilityData = sqlit.queryDmaAbilityData;
  dmaAbilityData.mockResolvedValue([
    {
      startNs: 15255,
      value: 2,
      expTaskComm: 'allocator_host',
      flag: 0,
      name: 'test',
    },
  ]);

  let gpuMemoryAbilityData = sqlit.queryGpuMemoryAbilityData;
  gpuMemoryAbilityData.mockResolvedValue([
    {
      name: 'test',
      startNs: 15255,
      value: 0,
    },
  ]);
  let manager = new SpChartManager();
  let trace = new SpAbilityMonitorChart(manager);
  it('SpAbilityMonitorChart01', function () {
    trace.init();
    expect(trace).toBeDefined();
  });
  it('SpAbilityMonitorChart02', function () {
    let traceRow = new TraceRow();
    expect(trace.initNetworkAbility(traceRow)).toBeDefined();
  });
});
