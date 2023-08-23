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
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
import {
  queryCPuAbilityMaxData,
  queryMemoryMaxData,
  queryNetWorkMaxData,
} from '../../../../src/trace/database/SqlLite.js';
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
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
  let spSystemTrace = new SpSystemTrace();
  let trace = new SpAbilityMonitorChart(spSystemTrace);
  it('SpAbilityMonitorChart01', function () {
    trace.init();
    expect(trace).toBeDefined();
  });
});
