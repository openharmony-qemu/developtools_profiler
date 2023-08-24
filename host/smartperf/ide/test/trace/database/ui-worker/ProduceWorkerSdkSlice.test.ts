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
import { SdkSliceRender, SdkSliceStruct } from '../../../../dist/trace/database/ui-worker/ProduceWorkerSdkSlice.js';

describe('ProduceWorkerSdkSlice Test', () => {
  it('ProduceWorkerSdkSliceTest01', function () {
    let sdkSliceRender = new SdkSliceRender();
    let list = [
      {
        length: 1,
        frame: {
          x: 1,
          Y: 10,
          width: 100,
          height: 20,
        },
      },
    ];
    let res = [
      {
        length: 1,
        frame: null,
      },
    ];
    expect(sdkSliceRender.sdkSlice(list, res, 1, 5, 4, true)).toBeUndefined();
  });

  it('ProduceWorkerSdkSliceTest02', function () {
    let sdkSliceRender = new SdkSliceRender();
    let list = [
      {
        length: 1,
        frame: {
          x: 1,
          Y: 10,
          width: 100,
          height: 20,
        },
      },
    ];
    let res = [
      {
        length: 0,
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
    expect(sdkSliceRender.render(req, [], [])).toBeUndefined();
  });
});
