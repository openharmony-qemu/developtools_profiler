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
import { SdkCounterRender, CounterStruct } from '../../../../dist/trace/database/ui-worker/ProduceWorkerSdkCounter.js';

describe('ProduceWorkerSdkCounter Test', () => {
  it('ProduceWorkerSdkCounterTest01', function () {
    let sdkCounterRender = new SdkCounterRender();
    let List = [
      {
        length: 1,
        ts: 1,
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
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
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(sdkCounterRender.counter(List, arr, 1, 1, 1, frame, true)).toBeUndefined();
  });

  it('ProduceWorkerSdkCounterTest02', function () {
    let sdkCounterRender = new SdkCounterRender();
    let List = [
      {
        length: 1,
        ts: 1,
        frame: {
          x: 20,
          y: 20,
          width: 100,
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
        width: 100,
        height: 100,
      },
      startTime: 1,
      ts: 1,
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
        x: 20,
        y: 20,
        width: 100,
        height: 100,
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
        x: 20,
        y: 20,
        width: 100,
        height: 100,
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
    expect(sdkCounterRender.render(req, [], [])).toBeUndefined();
  });
});
