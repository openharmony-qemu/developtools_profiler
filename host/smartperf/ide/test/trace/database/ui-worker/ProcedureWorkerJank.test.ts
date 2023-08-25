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
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { jank, JankRender, JankStruct } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerJank.js';
// @ts-ignore
import { ColorUtils } from '../../../../dist/trace/component/trace/base/ColorUtils.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('ProcedureWorkerJank Test', () => {
  const jankData = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    id: 35,
    ts: 42545,
    dur: 2015,
    name: '2145',
    depth: 1,
    jank_tag: false,
    cmdline: 'render.test',
    type: '1',
    pid: 20,
    frame_type: 'render_service',
    src_slice: '525',
    rs_ts: 2569,
    rs_vsync: '2569',
    rs_dur: 1528,
    rs_pid: 1252,
    rs_name: 'name',
    gpu_dur: 2568,
  };
  let render = new JankRender();

  it('ProcedureWorkerJank01', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 2;
    canvas.height = 2;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 120,
        y: 120,
        width: 100,
        height: 100,
      },
      id: 21,
      ts: 254151,
      dur: 1201,
      name: '1583',
      depth: 6,
      jank_tag: true,
      cmdline: 'render.test',
      type: '0',
      pid: 21,
      frame_type: 'render_service',
      src_slice: '5',
      rs_ts: 3,
      rs_vsync: '2561',
      rs_dur: 965,
      rs_pid: 320,
      rs_name: 'name',
      gpu_dur: 102,
    };
    expect(JankStruct.draw(ctx!, data, 2)).toBeUndefined();
  });

  it('ProcedureWorkerJank02', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(JankStruct.draw(ctx!, jankData, 2)).toBeUndefined();
  });

  it('ProcedureWorkerJank09', function () {
    let jankNode = [
      {
        frame: {
          x: 9,
          y: 10,
          width: 21,
          height: 22,
        },
        startNS: 44,
        length: 41,
        height: 422,
      },
    ];
    let frame = {
      x: 220,
      y: 120,
      width: 444,
      height: 4,
    };
    let list = [
      {
        frame: {
          x: 7,
          y: 75,
          width: 2,
          height: 12,
        },
        startNS: 45,
        length: 5,
        height: 32,
      },
    ];
    jank(list, jankNode, 1, 1, 1, frame, true);
  });

  it('ProcedureWorkerJank10', function () {
    let node = [
      {
        frame: {
          x: 90,
          y: 80,
          width: 230,
          height: 300,
        },
        startNS: 800,
        length: 100,
        height: 92,
      },
    ];
    let frame = {
      x: 980,
      y: 980,
      width: 400,
      height: 1440,
    };
    let list = [
      {
        frame: {
          x: 45,
          y: 50,
          width: 150,
          height: 140,
        },
        startNS: 20,
        length: 72,
        height: 27,
      },
    ];
    jank(list, node, 1, 1, 1, frame, false);
  });

  it('ProcedureWorkerJank11', () => {
    let node = {
      frame: {
        x: 65,
        y: 20,
        width: 99,
        height: 330,
      },
      startNS: 200,
      length: 1,
      height: 90,
      startTime: 0,
      dur: 31,
    };
    expect(JankStruct.setJankFrame(node, 1, 1, 1, 10, { width: 10 })).toBeUndefined();
  });

  it('ProcedureWorkerJank12', () => {
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    TraceRow.range = {
      startNS: 3206163251057,
      endNS: 3215676817201,
      totalNS: 9513566144,
    };
    new JankRender().renderMainThread(
      {
        context: context!,
        useCache: false,
        type: `expected_frame_timeline_slice`,
      },
      {
        dataList: [
          {
            id: 17,
            frame_type: 'frameTimes',
            ipid: 34,
            name: 36331,
            app_dur: 16616555,
            dur: 33554127,
            ts: 15037892,
            type: 61,
            flag: null,
            pid: 3467,
            cmdline: 'com.huawei',
            rs_ts: 31666322,
            rs_vsync: 28763,
            rs_dur: 16617667,
            rs_ipid: 555,
            rs_pid: 1253,
            rs_name: '',
            depth: 0,
            frame: {
              x: 12,
              y: 10,
              width: 45,
              height: 40,
            },
          },
          {
            id: 41,
            frame_type: 'frame',
            ipid: 81,
            name: 36612,
            app_dur: 16616234,
            dur: 33233432,
            ts: 31656235,
            type: 25,
            flag: null,
            pid: 3435,
            cmdline: 'com.wx',
            rs_ts: 48273526,
            rs_vsync: 25524,
            rs_dur: 16556797,
            rs_ipid: 325,
            rs_pid: 1232,
            rs_name: 'render',
            depth: 13,
            frame: {
              x: 43,
              y: 220,
              width: 15,
              height: 210,
            },
          },
          {
            id: 14,
            frame_type: 'time',
            ipid: 34,
            name: 33693,
            app_dur: 1661327,
            dur: 33236526,
            ts: 48256426,
            type: 41,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei',
            rs_ts: 68320255,
            rs_vsync: 28435,
            rs_dur: 16616797,
            rs_ipid: 35,
            rs_pid: 1563,
            rs_name: 'service',
            depth: 30,
            frame: {
              x: 63,
              y: 50,
              width: 355,
              height: 230,
            },
          },
        ],
        dataListCache: [
          {
            id: 17,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36691,
            app_dur: 16616797,
            dur: 33567127,
            ts: 15038992,
            type: 81,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 31656322,
            rs_vsync: 28323,
            rs_dur: 11116797,
            rs_ipid: 25,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 10,
            frame: {
              x: 2,
              y: 0,
              width: 5,
              height: 40,
            },
          },
          {
            id: 11,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36692,
            app_dur: 16616797,
            dur: 33277901,
            ts: 31657422,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 48273426,
            rs_vsync: 28324,
            rs_dur: 16616797,
            rs_ipid: 275,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 17,
            frame: {
              x: 47,
              y: 20,
              width: 5,
              height: 20,
            },
          },
          {
            id: 173,
            frame_type: 'frameTime',
            ipid: 84,
            name: 76693,
            app_dur: 16616797,
            dur: 33233626,
            ts: 48273426,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 64890255,
            rs_vsync: 28325,
            rs_dur: 16616797,
            rs_ipid: 25,
            rs_pid: 1783,
            rs_name: 'render_service',
            depth: 0,
            frame: {
              x: 6,
              y: 70,
              width: 5,
              height: 20,
            },
          },
        ],
      }
    );
  });
});
