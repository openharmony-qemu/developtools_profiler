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
import {
  cpu,
  CpuStruct,
  CpuRender,
  rtCpu,
  EmptyRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCPU.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
import { drawWakeUp } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon';

jest.mock('../../../../dist/trace/component/trace/timer-shaft/RangeRuler.js', () => {
  return {};
});
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe(' Test', () => {
  const dataSource = {
    frame: {
      x: 310,
      y: 130,
      width: 1430,
      height: 1430,
    },
    startNS: 430,
    processId: '',
  };

  it('CPUTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 13;
    canvas.height = 13;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 203,
        y: 203,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(CpuStruct.draw(ctx, data)).toBeUndefined();
  });

  it('CPUTest04', () => {
    expect(CpuStruct.equals(new CpuStruct(), new CpuStruct())).toBeTruthy();
  });

  it('CPUTest06', () => {
    expect(CpuStruct.equals([], dataSource)).toBeFalsy();
  });

  it('CPUTest05', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 230,
        y: 202,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(CpuStruct.draw(ctx, data)).toBeUndefined();
  });

  it('CPUTest07', function () {
    let cpuRender = new CpuRender();
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
          x: 10,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 2,
        height: 2,
      },
    ];
    expect(cpuRender.cpu(list, node, 1, 1, 1, frame, true)).toBeUndefined();
  });

  it('CPUTest07', function () {
    let cpuRender = new CpuRender();
    let node = [
      {
        frame: {
          x: 207,
          y: 201,
          width: 110,
          height: 200,
        },
        startNS: 200,
        length: 31,
        height: 30,
      },
    ];
    let frame = {
      x: 29,
      y: 69,
      width: 330,
      height: 430,
    };
    let list = [
      {
        frame: {
          x: 42,
          y: 42,
          width: 143,
          height: 430,
        },
        startNS: 200,
        length: 32,
        height: 2,
      },
    ];
    expect(cpuRender.cpu(list, node, 1, 1, 1, frame, false)).toBeUndefined();
  });

  it('CPUTest08', () => {
    let node = {
      frame: {
        x: 9,
        y: 87,
        width: 878,
        height: 80,
      },
      startNS: 700,
      length: 135,
      height: 40,
      startTime: 450,
      dur: 9,
    };
    expect(CpuStruct.setCpuFrame(node, 1, 1, 1, { width: 10 })).toBeUndefined();
  });

  it('CPUTest09', () => {
    let node = {
      frame: {
        x: 90,
        y: 20,
        width: 25,
        height: 98,
      },
      startNS: 690,
      length: 28,
      height: 60,
      startTime: 2,
      dur: 221,
    };
    expect(CpuStruct.setCpuFrame(node, 1, 1, 1, { width: 10 })).toBeUndefined();
  });

  it('CPUTest10', function () {
    let emptyRender = new EmptyRender();
    let req = {
      type: '',
      startNS: 0,
      endNS: 100,
      totalNS: 100,
      frame: {
        x: 20,
        y: 31,
        width: 75,
        height: 90,
      },
      canvas: 'abc',
      context: {
        measureText: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        closePath: jest.fn(() => true),
        fillRect: jest.fn(() => []),
        fillText: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
      },
      lineColor: '#0084b3',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 121,
      id: 231,
      x: 40,
      y: 40,
      width: 140,
      height: 104,
    };
    window.postMessage = jest.fn(() => true);
    expect(emptyRender.render(req, [], [])).toBeUndefined();
  });

  it('CPUTest11', function () {
    let cpuRender = new CpuRender();
    let cpuReq = {
      lazyRefresh: true,
      type: '1',
      startNS: 1,
      endNS: 4,
      totalNS: 3,
      frame: {
        x: 334,
        y: 442,
        width: 230,
        height: 330,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#221786',
        globalAlpha: 0.6,
        closePath: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        measureText: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        fillText: jest.fn(() => true),
        fillRect: jest.fn(() => true),
      },
      lineColor: '#112d7d',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 1113,
      id: 111,
      x: 212,
      y: 2230,
      width: 156,
      height: 600,
    };
    window.postMessage = jest.fn(() => true);
    expect(cpuRender.render(cpuReq, [], [])).toBeUndefined();
  });
  it('CPUTest12', function () {
    let emptyRender = new EmptyRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(emptyRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
