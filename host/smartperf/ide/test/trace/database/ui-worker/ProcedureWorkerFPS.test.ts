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
import { fps, FpsStruct, FpsRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFPS.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe(' FPSTest', () => {
  it('FpsTest01', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    let rect = new Rect(0, 10, 10, 10);
    fps(dataList, [{ length: 1 }], 1, 100254, 100254, rect, true);
  });

  it('FpsTest02', () => {
    let fpsDataList = new Array();
    fpsDataList.push({
      startTime: 34,
      dur: 14,
      frame: { x: 40, y: 442, width: 230, height: 340 },
    });
    fpsDataList.push({
      startTime: 61,
      dur: 156,
      frame: { x: 60, y: 9, width: 10, height: 10 },
    });
    let rect = new Rect(0, 50, 50, 16);
    fps(fpsDataList, [{ length: 0 }], 1, 100254, 100254, rect, false);
  });

  it('FpsTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 8;
    canvas.height = 8;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 218,
        y: 201,
        width: 220,
        height: 320,
      },
      startNS: 255,
      value: 4,
    };

    expect(FpsStruct.draw(ctx, data)).toBeUndefined();
  });

  it('FpsTest04', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 8;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 80,
        y: 30,
        width: 320,
        height: 220,
      },
      startNS: 200,
      value: 50,
    };
    new FpsStruct(1);
    FpsStruct.hoverFpsStruct = jest.fn(() => {
      startNS: 200;
    });
    FpsStruct.a = jest.fn(() => data);
    expect(FpsStruct.draw(ctx, data)).toBeUndefined();
  });
  it('FpsTest05 ', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    const Sourcedate = {
      frame: {
        x: 520,
        y: 50,
        width: 300,
        height: 300,
      },
      maxFps: 255,
      value: 550,
    };
    expect(FpsStruct.draw(ctx, Sourcedate)).toBeUndefined();
  });

  it('FpsTest06', function () {
    let fpsRender = new FpsRender();
    let fpsReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 32,
      totalNS: 31,
      frame: {
        x: 54,
        y: 50,
        width: 133,
        height: 133,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '12px sans-serif',
        fillStyle: '#af919b',
        globalAlpha: 0.56,
        height: 120,
        width: 100,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        closePath: jest.fn(() => true),
        fillRect: jest.fn(() => []),
        fillText: jest.fn(() => true),
        stroke: jest.fn(() => true),
      },
      lineColor: '',
      isHover: '',
      hoverX: 21,
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 34,
      id: 1,
      x: 220,
      y: 203,
      width: 1030,
      height: 890,
      params: {
        isLive: false,
        maxHeight: 52,
        dpr: 41,
        hoverFuncStruct: '',
        selectFuncStruct: undefined,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(fpsRender.render(fpsReq, [], [])).toBeUndefined();
  });
});
