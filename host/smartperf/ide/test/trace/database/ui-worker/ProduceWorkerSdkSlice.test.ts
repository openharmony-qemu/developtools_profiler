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
import { SdkSliceRender, SdkSliceStruct } from '../../../../dist/trace/database/ui-worker/ProduceWorkerSdkSlice.js';
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('ProduceWorkerSdkSlice Test', () => {
  it('ProduceWorkerSdkSliceTest01', function () {
    let sdkSliceRender = new SdkSliceRender();
    let list = [
      {
        length: 19,
        frame: {
          x: 46,
          Y: 140,
          width: 780,
          height: 80,
        },
      },
    ];
    let res = [
      {
        length: 81,
        frame: null,
      },
    ];
    expect(sdkSliceRender.sdkSlice(list, res, 1, 5, 4, true)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest02', function () {
    let sdkSliceRender = new SdkSliceRender();
    let list = [
      {
        length: 891,
        frame: {
          x: 17,
          Y: 175,
          width: 550,
          height: 870,
        },
      },
    ];
    let res = [
      {
        length: 430,
        frame: null,
      },
    ];
    expect(sdkSliceRender.sdkSlice(list, res, 1, 5, 4, false)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest03', () => {
    const data = {
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      start_ts: 1,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(SdkSliceStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest04', () => {
    let node = {
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      start_ts: 1,
      end_ts: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(SdkSliceStruct.setSdkSliceFrame(node, 2, 2, 3, 1, frame)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest05', () => {
    let node = {
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      start_ts: 3,
      end_ts: 5,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(SdkSliceStruct.setSdkSliceFrame(node, 2, 2, 3, 1, frame)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest06', function () {
    let sdkSliceRender = new SdkSliceRender();
    let sdkSliceReq = {
      lazyRefresh: true,
      type: '',
      startNS: 21,
      endNS: 31,
      totalNS: 10,
      frame: {
        x: 20,
        y: 10,
        width: 100,
        height: 200,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#2c441b',
        globalAlpha: 0.75,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => ''),
        fillRect: jest.fn(() => true),
        fillText: jest.fn(() => false),
      },
      lineColor: '#993e00',
      isHover: '',
      hoverX: 51,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 66,
      id: 1,
      x: 70,
      y: 80,
      width: 15,
      height: 15,
    };
    window.postMessage = jest.fn(() => true);
    expect(sdkSliceRender.render(sdkSliceReq, [], [])).toBeUndefined();
  });
  it('ProduceWorkerSdkSliceTest07', function () {
    let sdkSliceRender = new SdkSliceRender();
    window.postMessage = jest.fn(() => true);
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    expect(sdkSliceRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
