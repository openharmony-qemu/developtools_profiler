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
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest02', () => {
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
      state: 'S',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest03', () => {
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
      state: 'R',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest04', () => {
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
      state: 'D',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest05', () => {
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
      state: 'Running',
    };
    expect(ThreadStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerThreadTest06', () => {
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
    expect(threadRender.render(req, [], [])).toBeUndefined();
  });
});
