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
import { SpProcessChart } from '../../../../dist/trace/component/chart/SpProcessChart.js';
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
  observe: jest.fn(),
  unobserve: jest.fn(),
  disconnect: jest.fn(),
}));

describe('SpProcessChart Test', () => {
  let manager = new SpChartManager();
  let spProcessChart = new SpProcessChart(manager);
  let MockqueryProcessAsyncFunc = sqlit.queryProcessAsyncFunc;

  MockqueryProcessAsyncFunc.mockResolvedValue([
    {
      tid: 1,
      pid: 1,
      threadName: '1',
      track_id: 3,
      startTs: 1111,
      dur: 2000000,
      funName: 'deliverInputEvent',
      parent_id: 4,
      id: 5,
      cookie: 'ff',
      depth: 5,
      argsetid: 6,
    },
  ]);
  let processContentCount = sqlit.queryProcessContentCount;
  processContentCount.mockResolvedValue([
    {
      pid: 1,
      switch_count: 2,
      thread_count: 3,
      slice_count: 4,
      mem_count: 5,
    },
  ]);
  let queryProcessThreads = sqlit.queryProcessThreads;
  queryProcessThreads.mockResolvedValue([]);
  let queryProcessThreadsByTable = sqlit.queryProcessThreadsByTable;
  queryProcessThreadsByTable.mockResolvedValue([
    {
      pid: 1,
      tid: 0,
      processName: 'process',
      threadName: 'thread',
    },
  ]);
  let queryProcessMem = sqlit.queryProcessMem;
  queryProcessMem.mockResolvedValue([
    {
      trackId: 1,
      trackName: 'trackName',
      upid: 2,
      pid: 1,
      processName: 'processName',
    },
  ]);
  let queryEventCountMap = sqlit.queryEventCountMap;
  queryEventCountMap.mockResolvedValue([
    {
      eventName: 'eventName',
      count: 1,
    },
  ]);
  let queryProcess = sqlit.queryProcess;
  queryProcess.mockResolvedValue([
    {
      pid: 1,
      processName: 'processName',
    },
  ]);

  let queryProcessByTable = sqlit.queryProcessByTable;
  queryProcessByTable.mockResolvedValue([
    {
      pid: 2,
      processName: 'processName',
    },
  ]);

  let getMaxDepthByTid = sqlit.getMaxDepthByTid;
  getMaxDepthByTid.mockResolvedValue([
    {
      tid: 1,
      maxDepth: 1,
    },
    {
      tid: 2,
      maxDepth: 2,
    },
  ]);
  let queryAllJankProcess = sqlit.queryAllJankProcess;
  queryAllJankProcess.mockResolvedValue([
    {
      pid: 1,
    },
  ]);

  let queryAllExpectedData = sqlit.queryAllExpectedData;
  queryAllExpectedData.mockResolvedValue([
    {
      id: 41,
      ts: 749660047,
      name: 1159,
      type: 1,
      dur: 16657682,
      pid: 1242,
      cmdline: 'render_service',
    },
    {
      id: 45,
      ts: 766321174,
      name: 1160,
      type: 1,
      dur: 16657682,
      pid: 1,
      cmdline: 'render_service',
    },
  ]);

  let queryAllActualData = sqlit.queryAllActualData;
  queryAllActualData.mockResolvedValue([
    {
      id: 40,
      ts: 750328000,
      name: 1159,
      type: 0,
      dur: 22925000,
      src_slice: '36',
      jank_tag: 1,
      dst_slice: null,
      pid: 1242,
      cmdline: 'render_service',
      frame_type: 'render_service',
    },
    {
      id: 44,
      ts: 773315000,
      name: 1160,
      type: 0,
      dur: 17740000,
      src_slice: '38,42',
      jank_tag: 1,
      dst_slice: null,
      pid: 1,
      cmdline: 'render_service',
      frame_type: 'render_service',
    },
  ]);

  let queryAllTaskPoolPid = sqlit.queryAllTaskPoolPid;
  queryAllTaskPoolPid.mockResolvedValue([
    {
      pid: 1,
    },
    {
      id: 2,
    },
  ]);

  let queryProcessStartup = sqlit.queryProcessStartup;
  queryProcessStartup.mockResolvedValue([
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 366,
      'startTs': 5651745832,
      'dur': 38654167,
      'startName': 0,
      'endItid': 341,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1154,
        'width': 9
      },
      'v': true,
      'stepName': 'Process Creating (38.65ms)',
      'textMetricsWidth': 129.072265625
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 341,
      'startTs': 5690399999,
      'dur': 43619792,
      'startName': 1,
      'endItid': 486,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1162,
        'width': 10
      },
      'v': true,
      'stepName': 'Application Launching (43.62ms)',
      'textMetricsWidth': 156.416015625
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5734019791,
      'dur': 23194270,
      'startName': 2,
      'endItid': 486,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1171,
        'width': 6
      },
      'v': true
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5757214061,
      'dur': 115679167,
      'startName': 3,
      'endItid': 486,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1176,
        'width': 24
      },
      'v': true,
      'stepName': 'UI Ability OnForeground (115.68ms)',
      'textMetricsWidth': 172.59765625
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5872893228,
      'dur': 62756250,
      'startName': 4,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1199,
        'width': 14
      },
      'v': true,
      'stepName': 'First Frame - APP Phase (62.76ms)',
      'textMetricsWidth': 162.9638671875
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 17,
      'startTs': 5968040103,
      'dur': 29438021,
      'startName': 5,
      'frame': {
        'y': 5,
        'height': 20,
        'x': 1219,
        'width': 7
      },
      'v': true
    }
  ]);

  let queryProcessSoInitData = sqlit.queryProcessSoInitData;
  queryProcessSoInitData.mockResolvedValue([
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5678439061,
      'dur': 1137500,
      'soName': ' /system/lib64/seccomp/libapp_filter.z.so',
      'depth': 0,
      'frame': {
        'x': 1160,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5682777082,
      'dur': 1130729,
      'soName': ' /system/lib64/libhidebug.so',
      'depth': 0,
      'frame': {
        'x': 1160,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5696226041,
      'dur': 4319791,
      'soName': ' system/lib64/extensionability/libinputmethod_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1163,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5700671874,
      'dur': 4128125,
      'soName': ' system/lib64/extensionability/libbackup_extension_ability_native.z.so',
      'depth': 0,
      'frame': {
        'x': 1164,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5704894270,
      'dur': 2187500,
      'soName': ' system/lib64/extensionability/libwindow_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1165,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5707165624,
      'dur': 1503125,
      'soName': ' system/lib64/extensionability/libdatashare_ext_ability_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1165,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5708719791,
      'dur': 2018229,
      'soName': ' system/lib64/extensionability/libpush_extension.z.so',
      'depth': 0,
      'frame': {
        'x': 1166,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5710788020,
      'dur': 846875,
      'soName': ' system/lib64/extensionability/libenterprise_admin_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1166,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5711693749,
      'dur': 1522917,
      'soName': ' system/lib64/extensionability/libstatic_subscriber_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1166,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5713377603,
      'dur': 1327604,
      'soName': ' system/lib64/extensionability/libui_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1167,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5714757291,
      'dur': 2567187,
      'soName': ' system/lib64/extensionability/libauthorization_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1167,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5717385936,
      'dur': 2341146,
      'soName': ' system/lib64/extensionability/libaccessibility_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1167,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5719780728,
      'dur': 1603646,
      'soName': ' system/lib64/extensionability/libservice_extension_module.z.so',
      'depth': 0,
      'frame': {
        'x': 1168,
        'y': 0,
        'width': 1,
        'height': 20
      }
    },
    {
      'pid': 3913,
      'tid': 3913,
      'itid': 486,
      'startTs': 5721437499,
      'dur': 1314583,
      'soName': ' system/lib64/extensionability/libworkschedextension.z.so',
      'depth': 0,
      'frame': {
        'x': 1168,
        'y': 0,
        'width': 1,
        'height': 20
      }
    }
  ]);
  let processData = sqlit.queryProcessData;
  processData.mockResolvedValue([
    {
      cpu: 0, dur: 199000, startTime: 259730000
    },
    {
      cpu: 2, dur: 147000, startTime: 307742000
    }
  ]);
  let processMemData = sqlit.queryProcessMemData;
  processMemData.mockResolvedValue([
    {
      startTime: 593015789,
      track_id : 153,
      ts : 30150767408970,
      type : "measure",
      value : 0
    },
    {
      startTime: 593360060,
      track_id : 153,
      ts : 30150767753241,
      type : "measure",
      value : 1
    }
  ]);

  spProcessChart.setAttribute = jest.fn();
  spProcessChart.addChildTraceRow = jest.fn();
  it('SpProcessChart01', function () {
    spProcessChart.init();
    expect(spProcessChart).toBeDefined();
  });

  it('SpProcessChart02', function () {
    expect(spProcessChart.initAsyncFuncData()).not.toBeUndefined();
  });

  it('SpProcessChart03', function () {
    spProcessChart.initAsyncFuncData();
    spProcessChart.initDeliverInputEvent();
    expect(spProcessChart.processAsyncFuncArray.length).toEqual(1);
  });

  it('SpProcessChart04', function () {
    let startUpRow = spProcessChart.addStartUpRow(spProcessChart);
    expect(startUpRow).not.toBeUndefined();
  });

  it('SpProcessChart05', function () {
    let soInitRow = spProcessChart.addSoInitRow(spProcessChart, 1);
    expect(soInitRow).not.toBeUndefined();
  });
});
