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
// @ts-ignore
import {
  CpuStateRender,
  CpuStateStruct,
  cpuState,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCpuState.js';

describe('ProcedureWorkerCpuState Test', () => {
  it('ProcedureWorkerCpuStateTest01', function () {
    let node = {
      frame: {
        x: 43,
        y: 40,
        width: 135,
        height: 190,
      },
      startNS: 220,
      value: 20,
      startTs: 15,
      dur: 12,
      height: 62,
    };
    let frame = {
      x: 80,
      y: 28,
      width: 180,
      height: 80,
    };
    expect(CpuStateStruct.setFrame(node, 2, 2, 6, 4, frame)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest01', function () {
    let node = {
      frame: {
        x: 10,
        y: 22,
        width: 540,
        height: 700,
      },
      startNS: 560,
      value: 10,
      startTs: 7,
      dur: 74,
      height: 12,
    };
    let frame = {
      x: 30,
      y: 50,
      width: 760,
      height: 660,
    };
    expect(CpuStateStruct.setFrame(node, 2, 2, 6, 4, frame)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest02', function () {
    let cpuStateRender = new CpuStateRender();
    let node = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 0,
        height: 2,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(cpuStateRender.cpuState(node, [{ length: 1 }], '', [], 4, 1, 1, frame, true)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest03', function () {
    let cpuStateRender = new CpuStateRender();
    let dataList = new Array();
    dataList.push({
      startNS: 55,
      dur: 120,
      length: 53,
      frame: { x: 0, y: 54, width: 30, height: 350 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let res = [
      {
        frame: {
          x: 77,
          y: 67,
          width: 170,
          height: 700,
        },
        startNS: 17,
        length: 61,
        height: 25,
        dur: 12,
      },
    ];
    let frame = {
      x: 30,
      y: 23,
      width: 173,
      height: 350,
    };
    expect(cpuStateRender.cpuState([], dataList, '', res, 1, 6, 5, frame, true)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest04', function () {
    let cpuStateRender = new CpuStateRender();
    let dataList = new Array();
    dataList.push({
      startNS: 23,
      dur: 120,
      length: 21,
      frame: { x: 20, y: 45, width: 50, height: 150 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let res = [
      {
        frame: {
          x: 57,
          y: 30,
          width: 770,
          height: 503,
        },
        startNS: 13,
        length: 21,
        height: 22,
        dur: 156,
      },
    ];
    let frame = {
      x: 60,
      y: 23,
      width: 170,
      height: 900,
    };
    expect(cpuStateRender.cpuState([], dataList, '', res, 1, 6, 5, frame, true)).toBeUndefined();
  });
  it('ProcedureWorkerCpuStateTest05', function () {
    let res = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 10,
        length: 1,
        height: 2,
        dur: 1,
      },
    ];
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let path = new Path2D();
    expect(CpuStateStruct.draw(ctx, path, res)).toBeUndefined();
  });
  it('ProcedureWorkerCpuStateTest06', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTs: 3,
      dur: 1,
      height: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CpuStateStruct.setCpuFrame(node, 2, 2, 6, frame)).toBeUndefined();
  });
  it('ProcedureWorkerCpuStateTest07', function () {
    let req = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 11,
      totalNS: 1,
      frame: {
        x: 201,
        y: 201,
        width: 100,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'b',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        fill: jest.fn(() => true),
      },
      lineColor: '',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 3,
      id: 1,
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let cpuStateRender = new CpuStateRender();
    window.postMessage = jest.fn(() => true);
    expect(cpuStateRender.render(req, [], [], [])).toBeUndefined();
  });
  it('ProcedureWorkerCpuStateTest08', function () {
    let cpuStateRender = new CpuStateRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      cpuStateContext: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(cpuStateRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
