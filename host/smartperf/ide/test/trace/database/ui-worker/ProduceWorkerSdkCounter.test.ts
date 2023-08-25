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
import { SdkCounterRender, CounterStruct } from '../../../../dist/trace/database/ui-worker/ProduceWorkerSdkCounter.js';

describe('ProduceWorkerSdkCounter Test', () => {
  it('ProduceWorkerSdkCounterTest01', function () {
    let sdkCounterRender = new SdkCounterRender();
    let List = [
      {
        length: 11,
        ts: 9,
        frame: {
          x: 20,
          y: 33,
          width: 600,
          height: 600,
        },
      },
    ];
    let arr = [
      {
        frame: null,
        length: 1,
      },
    ];
    let frame = {
      x: 23,
      y: 25,
      width: 200,
      height: 220,
    };
    expect(sdkCounterRender.counter(List, arr, 1, 1, 1, frame, true)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest02', function () {
    let sdkCounterRender = new SdkCounterRender();
    let List = [
      {
        length: 17,
        ts: 41,
        frame: {
          x: 21,
          y: 32,
          width: 700,
          height: 100,
        },
      },
    ];
    let arr = [
      {
        frame: null,
        length: 0,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(sdkCounterRender.counter(List, arr, 1, 1, 1, frame, false)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest03', () => {
    const data = {
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 160,
        height: 160,
      },
      startTime: 1,
      ts: 2,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(CounterStruct.draw(ctx, data, 1)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest04', () => {
    let node = {
      startNs: 1,
      value: 1,
      frame: {
        x: 13,
        y: 13,
        width: 130,
        height: 130,
      },
      ts: 1,
      dur: 1,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CounterStruct.setCounterFrame(node, 2, 1, 2, 2, frame)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest05', () => {
    let node = {
      startNs: 1,
      value: 1,
      frame: {
        x: 50,
        y: 50,
        width: 150,
        height: 150,
      },
      ts: 0,
      dur: 3,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CounterStruct.setCounterFrame(node, 2, 1, 2, 2, frame)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest06', function () {
    let sdkCounterRender = new SdkCounterRender();
    let sdkCounterReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 199,
      totalNS: 198,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 120,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#effa8e',
        globalAlpha: 0.71,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        fillText: jest.fn(() => false),
        fillRect: jest.fn(() => true),
      },
      lineColor: '#78e2ba',
      isHover: '',
      hoverX: 144,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 342,
      id: 1,
      x: 20,
      y: 20,
      width: 123,
      height: 135,
    };
    window.postMessage = jest.fn(() => true);
    expect(sdkCounterRender.render(sdkCounterReq, [], [])).toBeUndefined();
  });
  it('ProduceWorkerSdkCounterTest06', function () {
    let sdkCounterRender = new SdkCounterRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(sdkCounterRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
