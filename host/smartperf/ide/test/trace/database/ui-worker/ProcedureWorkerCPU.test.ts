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

jest.mock('../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

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

describe(' Test', () => {
  const dataSource = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    startNS: 200,
    processId: '',
  };

  it('CPUTest03', () => {
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
        x: 20,
        y: 20,
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
    expect(cpuRender.cpu(list, node, 1, 1, 1, frame, true)).toBeUndefined();
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
        height: 0,
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
    expect(cpuRender.cpu(list, node, 1, 1, 1, frame, false)).toBeUndefined();
  });

  it('CPUTest08', () => {
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
    expect(CpuStruct.setCpuFrame(node, 1, 1, 1, { width: 10 })).toBeUndefined();
  });

  it('CPUTest09', () => {
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
      startTime: 2,
      dur: 1,
    };
    expect(CpuStruct.setCpuFrame(node, 1, 1, 1, { width: 10 })).toBeUndefined();
  });

  it('CPUTest10', function () {
    let emptyRender = new EmptyRender();
    let req = {
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
      canvas: '',
      context: {},
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
    window.postMessage = jest.fn(() => true);
    expect(emptyRender.render(req, [], [])).toBeUndefined();
  });

  it('CPUTest11', function () {
    let cpuRender = new CpuRender();
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
      canvas: '',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
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
    window.postMessage = jest.fn(() => true);
    expect(cpuRender.render(req, [], [])).toBeUndefined();
  });
});
