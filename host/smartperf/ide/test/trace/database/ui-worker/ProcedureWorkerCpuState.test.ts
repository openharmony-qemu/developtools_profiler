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
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTs: 1,
      dur: 10,
      height: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CpuStateStruct.setFrame(node, 2, 2, 6, 4, frame)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest01', function () {
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
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let res = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 1,
        length: 1,
        height: 2,
        dur: 1,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(cpuStateRender.cpuState([], dataList, '', res, 1, 6, 5, frame, true)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest04', function () {
    let cpuStateRender = new CpuStateRender();
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
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
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
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
      endNS: 1,
      totalNS: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
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
  it('ProcedureWorkerCpuStateTest07', function () {
    let cpuStateRender = new CpuStateRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(cpuStateRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
