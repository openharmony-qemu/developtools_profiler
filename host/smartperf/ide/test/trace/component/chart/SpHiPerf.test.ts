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
import { SpHiPerf } from '../../../../dist/trace/component/chart/SpHiPerf.js';
import {
  queryHiPerfCpuMergeData2,
  queryHiPerfEventList,
  queryPerfThread,
} from '../../../../src/trace/database/SqlLite.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpHiPerf Test', () => {
  let queryPerfCmdline = sqlit.queryPerfCmdline;
  queryPerfCmdline.mockResolvedValue([
    {
      report_value:
        'hiperf record --control prepare -o /data/local/tmp…e sched:sched_waking -a -s dwarf -f 1000 --offcpu',
    },
  ]);

  let queryPerfThread = sqlit.queryPerfThread;
  queryPerfThread.mockResolvedValue([
    {
      tid: 2,
      threadName: 'threadName',
      pid: 2,
      processName: 'processName',
    },
    {
      tid: 1,
      threadName: 'threadName111',
      pid: 1,
      processName: 'processNam111e',
    },
  ]);

  let queryHiPerfEventList = sqlit.queryHiPerfEventList;
  queryHiPerfEventList.mockResolvedValue([
    {
      id: 0,
      report_value: 'sched:sched_waking',
    },
    {
      id: 1,
      report_value: 'sched:sched_switch',
    },
  ]);

  let queryHiPerfCpuMergeData2 = sqlit.queryHiPerfCpuMergeData2;
  queryHiPerfCpuMergeData2.mockResolvedValue([
    {
      id: 0,
      callchain_id: 1,
      timestamp: 3468360924674,
      thread_id: 2469,
      event_count: 1,
      event_type_id: 0,
      timestamp_trace: 3468360965799,
      cpu_id: 2,
      thread_state: 'Running',
      startNS: 0,
    },
    {
      id: 4,
      callchain_id: 1,
      timestamp: 3468361000799,
      thread_id: 2469,
      event_count: 1,
      event_type_id: 0,
      timestamp_trace: 3468361041924,
      cpu_id: 2,
      thread_state: 'Running',
      startNS: 76125,
    },
    {
      id: 8,
      callchain_id: 1,
      timestamp: 3468361045716,
      thread_id: 2469,
      event_count: 1,
      event_type_id: 0,
      timestamp_trace: 3468361086841,
      cpu_id: 2,
      thread_state: 'Running',
      startNS: 121042,
    },
    {
      id: 9,
      callchain_id: 4,
      timestamp: 3468361054466,
      thread_id: 1336,
      event_count: 1,
      event_type_id: 1,
      timestamp_trace: 3468361095591,
      cpu_id: 3,
      thread_state: 'Suspend',
      startNS: 129792,
    },
  ]);
  let ss = new SpChartManager();
  let spHiPerf = new SpHiPerf(ss);
  it('SpHiPerf01', function () {
    spHiPerf.init();
    expect(spHiPerf).toBeDefined();
  });
});
