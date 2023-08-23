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
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      id: 25,
      ts: 254151,
      dur: 1202,
      name: '1583',
      depth: 1,
      jank_tag: true,
      cmdline: 'render.test',
      type: '0',
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
    let node = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 1,
        height: 2,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let list = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 2,
        height: 2,
      },
    ];
    jank(list, node, 1, 1, 1, frame, true);
  });

  it('ProcedureWorkerJank10', function () {
    let node = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 1,
        height: 2,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let list = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 2,
        height: 2,
      },
    ];
    jank(list, node, 1, 1, 1, frame, false);
  });

  it('ProcedureWorkerJank11', () => {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      length: 1,
      height: 0,
      startTime: 0,
      dur: 1,
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
            id: 7,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36691,
            app_dur: 16616797,
            dur: 33234127,
            ts: 15038992,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 31656322,
            rs_vsync: 28323,
            rs_dur: 16616797,
            rs_ipid: 25,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 0,
            frame: {
              x: 2,
              y: 0,
              width: 5,
              height: 20,
            },
          },
          {
            id: 11,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36692,
            app_dur: 16616797,
            dur: 33233901,
            ts: 31656322,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 48273426,
            rs_vsync: 28324,
            rs_dur: 16616797,
            rs_ipid: 25,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 1,
            frame: {
              x: 4,
              y: 20,
              width: 5,
              height: 20,
            },
          },
          {
            id: 13,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36693,
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
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 0,
            frame: {
              x: 6,
              y: 0,
              width: 5,
              height: 20,
            },
          },
        ],
        dataListCache: [
          {
            id: 7,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36691,
            app_dur: 16616797,
            dur: 33234127,
            ts: 15038992,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 31656322,
            rs_vsync: 28323,
            rs_dur: 16616797,
            rs_ipid: 25,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 0,
            frame: {
              x: 2,
              y: 0,
              width: 5,
              height: 20,
            },
          },
          {
            id: 11,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36692,
            app_dur: 16616797,
            dur: 33233901,
            ts: 31656322,
            type: 1,
            flag: null,
            pid: 3420,
            cmdline: 'com.huawei.wx',
            rs_ts: 48273426,
            rs_vsync: 28324,
            rs_dur: 16616797,
            rs_ipid: 25,
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 1,
            frame: {
              x: 4,
              y: 20,
              width: 5,
              height: 20,
            },
          },
          {
            id: 13,
            frame_type: 'frameTime',
            ipid: 84,
            name: 36693,
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
            rs_pid: 1263,
            rs_name: 'render_service',
            depth: 0,
            frame: {
              x: 6,
              y: 0,
              width: 5,
              height: 20,
            },
          },
        ],
      }
    );
  });
});
