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
import {
  getTimeString,
  TabPaneCurrentSelection,
} from '../../../../../dist/trace/component/trace/sheet/TabPaneCurrentSelection.js';
const sqlite = require('../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../dist/trace/database/SqlLite.js');

describe('TabPaneCurrentSelection Test', () => {
  let tabPaneCurrentSelection = new TabPaneCurrentSelection();

  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  let context = canvas.getContext('2d');

  let cpuData = [
    {
      cpu: 1,
      dur: 1,
      end_state: 'string',
      id: 12,
      name: 'name',
      priority: 11,
      processCmdLine: 'processCmdLine',
      processId: 1111,
      processName: 'processName',
      schedId: 221,
      startTime: 0,
      tid: 1001,
      type: 'type',
    },
  ];
  let functionData = [
    {
      argsetid: 53161,
      depth: 0,
      dur: 570000,
      funName: 'binder transaction',
      id: 92749,
      is_main_thread: 0,
      parent_id: null,
      startTs: 9729867000,
      threadName: 'Thread-15',
      tid: 2785,
    },
  ];
  let memData = [
    {
      trackId: 100,
      processName: 'processName',
      pid: 11,
      upid: 1,
      trackName: 'trackName',
      type: 'type',
      track_id: 'track_id',
      value: 111,
      startTime: 0,
      duration: 1000,
      maxValue: 4000,
      delta: 2,
    },
  ];
  let threadData = [
    {
      hasSched: 14724852000,
      pid: 2519,
      processName: null,
      threadName: 'ACCS0',
      tid: 2716,
      upid: 1,
      utid: 1,
      cpu: null,
      dur: 405001,
      end_ts: null,
      id: 11,
      is_main_thread: 0,
      name: 'ACCS0',
      startTime: 58001,
      start_ts: null,
      state: 'S',
      type: 'thread',
    },
  ];
  let wakeupBean = [
    {
      wakeupTime: 0,
      cpu: 1,
      process: 'process',
      pid: 11,
      thread: 'thread',
      tid: 22,
      schedulingLatency: 33,
      schedulingDesc: 'schedulingDesc',
    },
  ];

  let queryData = [
    {
      id: 1,
      startTime: 0,
      hasSched: 14724852000,
      pid: 2519,
      processName: null,
      threadName: 'ACCS0',
      tid: 2716,
      upid: 1,
      utid: 1,
      cpu: null,
      dur: 405002,
      end_ts: null,
      is_main_thread: 2,
      name: 'ACCS0',
      start_ts: null,
      state: 'S',
      type: 'thread',
    },
  ];
  let scrollWakeUp = [
    {
      startTime: 0,
      pid: 11,
      tid: 22,
    },
  ];
  let data = [
    {
      cpu: 1,
      dur: 1,
      end_state: 'string',
      id: 12,
      name: 'name',
      priority: 11,
      processCmdLine: 'processCmdLine',
      processId: 1112,
      processName: 'processName',
      schedId: 222,
      startTime: 0,
      tid: 1002,
      type: 'type',
    },
  ];

  let jankData = {
    id: 10,
    ts: 25415,
    dur: 1200,
    name: '1523',
    depth: 1,
    jank_tag: true,
    cmdline: 'com.test',
    type: '0',
    pid: 20,
    frame_type: 'app',
    app_dur: 110,
    dst_slice: 488,
  };

  let jankDataRender = {
    id: 22,
    ts: 254152,
    dur: 1202,
    name: '1583',
    depth: 1,
    jank_tag: true,
    cmdline: 'render.test',
    type: '0',
    pid: 22,
    frame_type: 'render_service',
    src_slice: '525',
    rs_ts: 2562,
    rs_vsync: '2562',
    rs_dur: 1528,
    rs_pid: 1252,
    rs_name: 'name',
    gpu_dur: 2568,
  };

  let irqData = [
    {
      id: 25,
      startNS: 1526,
      name: 'test',
      dur: 125,
      argSetId: 526,
    },
  ];

  let clockData = [
    {
      filterId: 96,
      value: 253,
      startNS: 25852,
      dur: 125,
      delta: 2586,
    },
  ];

  let functionDataTest = {
    argsetid: 53161,
    depth: 0,
    dur: 570000,
    funName: 'binder async',
    id: 92749,
    is_main_thread: 0,
    parent_id: null,
    startTs: 9729867000,
    threadName: 'Thread-15',
    tid: 2785,
  };

  tabPaneCurrentSelection.queryWakeUpData = jest.fn(() => 'WakeUpData');
  tabPaneCurrentSelection.queryWakeUpData.wb = jest.fn(() => null);
  tabPaneCurrentSelection.setCpuData(cpuData, undefined, 1);
  let argsetTest = sqlite.queryBinderArgsByArgset;
  let argsetIdTest = sqlite.queryBinderByArgsId;
  let argsetData = [
    {
      argset: 12,
      keyName: 'test',
      id: 123,
      desc: 'desc',
      strValue: 'value',
    },
    {
      argset: 11,
      keyName: 'test',
      id: 113,
      desc: 'desc',
      strValue: 'value',
    },
  ];

  let argsetIdData = [
    {
      type: 'func',
      startTs: 1258,
      dur: 25,
      depth: 1,
      argsetid: 258,
    },
  ];
  argsetTest.mockResolvedValue(argsetData);
  argsetIdTest.mockResolvedValue(argsetIdData);

  let gpuDur = sqlite.queryGpuDur;
  let gpuDurData = [
    {
      gpu_dur: 1528,
    },
  ];
  gpuDur.mockResolvedValue(gpuDurData);

  let queryFlows = sqlite.queryFlowsData;
  let queryFlowsData = [
    {
      name: '25962',
      pid: 1885,
      cmdline: 'render Test',
      type: 1,
    },
  ];
  queryFlows.mockResolvedValue(queryFlowsData);

  let queryPreceding = sqlite.queryPrecedingData;
  let queryPrecedingData = [
    {
      name: '2596562',
      pid: 18854,
      cmdline: 'app Test',
      type: 0,
    },
  ];
  queryPreceding.mockResolvedValue(queryPrecedingData);

  tabPaneCurrentSelection.queryWakeUpData = jest.fn(() => 'WakeUpData');
  tabPaneCurrentSelection.queryWakeUpData.wb = jest.fn(() => null);
  tabPaneCurrentSelection.setCpuData(cpuData, undefined, 1);

  it('TabPaneCurrentSelectionTest01', function () {
    let result = tabPaneCurrentSelection.setFunctionData(functionData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest02', function () {
    let result = tabPaneCurrentSelection.setMemData(memData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest03', function () {
    let result = getTimeString(3600_000_000_002);
    expect(result).toBe('1h 2ns ');
  });

  it('TabPaneCurrentSelectionTest04', function () {
    let result = getTimeString(60000000001);
    expect(result).toBe('1m 1ns ');
  });

  it('TabPaneCurrentSelectionTest05', function () {
    let result = getTimeString(1000000001);
    expect(result).toBe('1s 1ns ');
  });

  it('TabPaneCurrentSelectionTest06', function () {
    let result = getTimeString(1000001);
    expect(result).toBe('1ms 1ns ');
  });

  it('TabPaneCurrentSelectionTest07', function () {
    let result = getTimeString(1001);
    expect(result).toBe('1μs 1ns ');
  });

  it('TabPaneCurrentSelectionTest08', function () {
    let result = getTimeString(101);
    expect(result).toBe('101ns ');
  });

  it('TabPaneCurrentSelectionTest09', function () {
    tabPaneCurrentSelection.setCpuData = jest.fn(() => true);
    tabPaneCurrentSelection.data = jest.fn(() => true);
    expect(tabPaneCurrentSelection.data).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest10', function () {
    expect(tabPaneCurrentSelection.setCpuData(cpuData, undefined, 1)).toBeTruthy();
  });

  it('TabPaneCurrentSelectionTest13', function () {
    expect(tabPaneCurrentSelection.initCanvas()).not.toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest14', function () {
    let str = {
      length: 0,
    };
    expect(tabPaneCurrentSelection.transferString(str)).toBe('');
  });

  it('TabPaneCurrentSelectionTest16', function () {
    expect(tabPaneCurrentSelection.drawRight(null)).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest01', function () {
    let result = tabPaneCurrentSelection.setFunctionData(functionData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest02', function () {
    let result = tabPaneCurrentSelection.setMemData(memData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest12', function () {
    let result = tabPaneCurrentSelection.setJankData(jankData, undefined, 1);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest13', function () {
    let result = tabPaneCurrentSelection.setJankData(jankDataRender, undefined, 1);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest14', function () {
    let result = tabPaneCurrentSelection.setIrqData(irqData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest16', function () {
    let result = tabPaneCurrentSelection.setClockData(clockData);
    expect(result).toBeUndefined();
  });

  it('TabPaneCurrentSelectionTest17', function () {
    let result = tabPaneCurrentSelection.setFunctionData(functionDataTest);
    expect(result).toBeUndefined();
  });
  it('TabPaneCurrentSelectionTest18', function () {
    let result = tabPaneCurrentSelection.setStartupData(irqData, 1);
    expect(result).toBeUndefined();
  });
  it('TabPaneCurrentSelectionTest19', function () {
    let result = tabPaneCurrentSelection.setStaticInitData(irqData, 1);
    expect(result).toBeUndefined();
  });
  it('TabPaneCurrentSelectionTest20', function () {
    let list: never[] = [];
    let data = [
      {
        jank_tag: 1,
        frame_type: 'render_service',
      },
    ];
    let result = tabPaneCurrentSelection.setJankType(data, list);
    expect(result).toBeUndefined();
  });
  it('TabPaneCurrentSelectionTest21', function () {
    let data = [{
      startTime:22,
    }]
    let result = tabPaneCurrentSelection.setFrameAnimationData(data)
    expect(result).toBeTruthy();
  });
  it('TabPaneCurrentSelectionTest22', function () {
    let data = [{}]
    let result = tabPaneCurrentSelection.queryCPUWakeUpFromData(data)
    expect(result).toBeTruthy();
  });
});
