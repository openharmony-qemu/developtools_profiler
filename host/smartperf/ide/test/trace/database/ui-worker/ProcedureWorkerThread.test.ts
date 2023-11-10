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
import { thread, ThreadStruct, ThreadRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerThread.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe('ProcedureWorkerThread Test', () => {
  let frame = {
    x: 0,
    y: 9,
    width: 10,
    height: 10,
  };

  it('ProcedureWorkerThreadTest01', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 3;
    canvas.height = 3;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 201,
        y: 201,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest02', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 4;
    canvas.height = 4;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 202,
        y: 202,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      state: 'S',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 5;
    canvas.height = 5;
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
      state: 'R',
    };
    expect(ThreadStruct.drawThread(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest04', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 6;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 204,
        y: 240,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      state: 'D',
    };
    expect(ThreadStruct.drawThread(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest05', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 7;
    canvas.height = 7;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 207,
        y: 201,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      state: 'Running',
    };
    expect(ThreadStruct.drawThread(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest06', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 2;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 202,
        y: 203,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      state: 'T',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest07', () => {
    const d1 = {
      cpu: 1,
      tid: 1,
      state: '',
      startTime: 1,
      dur: 1,
    };
    const d2 = {
      cpu: 1,
      tid: 1,
      state: '',
      startTime: 1,
      dur: 1,
    };
    expect(ThreadStruct.equals(d1, d2)).toBeTruthy();
  });

  it('ProcedureWorkerThreadTest08', function () {
    let threadRender = new ThreadRender();
    let threadReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 19,
      totalNS: 18,
      frame: {
        x: 20,
        y: 20,
        width: 106,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: '',
      context: {
        font: '11px sans-serif',
        fillStyle: '#780229',
        globalAlpha: 0.62,
      },
      lineColor: '#519043',
      isHover: '',
      hoverX: 37,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 332,
      id: 8,
      x: 20,
      y: 20,
      width: 170,
      height: 170,
    };
    window.postMessage = jest.fn(() => true);
    expect(threadRender.render(threadReq, [], [])).toBeUndefined();
  });
  it('ProcedureWorkerThreadTest08', function () {
    let threadRender = new ThreadRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(threadRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
