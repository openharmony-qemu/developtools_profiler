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

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
//@ts-ignore
import {
  HiperfThreadRender,
  HiPerfThreadStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfThread.js';
// @ts-ignore
import { hiPerf } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js';

describe('ProcedureWorkerHiPerfThread Test', () => {
  let res = [
    {
      startNS: 13,
      dur: 50,
      frame: {
        x: 60,
        y: 69,
        width: 16,
        height: 69,
      },
    },
  ];
  it('ProcedureWorkerHiPerfThreadTest01', () => {
    const data = {
      frame: undefined,
      cpu: 0,
      startNs: 58,
      value: 5,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 4;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HiPerfThreadStruct.draw(ctx, '', data, true)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfThreadTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 80,
      dur: 17,
      length: 17,
      frame: { x: 7, y: 9, width: 17, height: 170 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 71 });
    let frame = {
      x: 70,
      y: 9,
      width: 70,
      height: 107,
    };
    hiPerf(dataList, [{ length: 0 }], dataList, 8, 3, frame, false, 1, false);
  });

  it('ProcedureWorkerHiPerfThreadTest03', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 30,
      dur: 350,
      length: 551,
      frame: { x: 7, y: 76, width: 610, height: 106 },
    });
    dataList.push({ startNS: 1, dur: 62, length: 1 });
    let frame = {
      x: 60,
      y: 96,
      width: 160,
      height: 160,
    };
    hiPerf(dataList, [{ length: 1 }], dataList, 8, 3, frame, true, 1, true);
  });

  it('ProcedureWorkerHiPerfThreadTest04', function () {
    expect(HiPerfThreadStruct.groupBy10MS([{ ps: 1 }, { coX: '1' }], 10, '')).toEqual([
      { dur: 10000000, height: Infinity, startNS: NaN },
    ]);
  });

  it('ProcedureWorkerHiPerfThreadTest05', function () {
    let hiperfThreadRender = new HiperfThreadRender();
    let hiperfThreadReq = {
      lazyRefresh: true,
      type: '',
      startNS: 21,
      endNS: 31,
      totalNS: 10,
      frame: {
        x: 134,
        y: 120,
        width: 102,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'thread',
      context: {
        font: '15px sans-serif',
        fillStyle: '#b4617b',
        globalAlpha: 0.65,
        closePath: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => []),
        measureText: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        fillText: jest.fn(() => []),
        fill: jest.fn(() => true),
      },
      lineColor: '#210202',
      isHover: '',
      hoverX: 3,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 9,
      id: 7,
      x: 71,
      y: 21,
      width: 100,
      height: 107,
      scale: 100_000_006,
    };
    window.postMessage = jest.fn(() => true);
    let a = {
      dataList: [
        {
          callchain_id: 1329,
          thread_name: 'uinput_inject',
          tid: 247,
          pid: 247,
          startNS: 1179247952,
          timestamp_group: 1170000000,
        },
        {
          callchain_id: 1330,
          thread_name: 'uinput_inject',
          tid: 247,
          pid: 247,
          startNS: 1179308910,
          timestamp_group: 1170000000,
        },
      ],
    };

    expect(hiperfThreadRender.render(hiperfThreadReq, [], [], [])).toBeUndefined();
  });
  it('ProcedureWorkerHiPerfThreadTest06', function () {
    let hiperfThreadRender = new HiperfThreadRender();
    window.postMessage = jest.fn(() => true);
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    expect(hiperfThreadRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
