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
import { ProcessMemStruct, MemRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerMem.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import { mem } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js';

describe(' Test', () => {
  let frame = {
    x: 0,
    y: 9,
    width: 10,
    height: 10,
  };
  it('MemTest01', () => {
    let memDataList = new Array();
    memDataList.push({
      startTime: 10,
      duration: 12,
      frame: { x: 0, y: 12, width: 14, height: 120 },
    });
    memDataList.push({
      startTime: 2,
      duration: 131,
      frame: { x: 0, y: 3, width: 30, height: 30 },
    });
    let rect = new Rect(0, 33, 10, 7);
    mem(memDataList, [{ length: 0 }], 2, 100254, 100254, frame, false);
  });

  it('MemTest02', () => {
    let memDataList = new Array();
    memDataList.push({
      startTime: 0,
      duration: 40,
      frame: { x: 0, y: 44, width: 144, height: 40 },
    });
    memDataList.push({
      startTime: 22,
      duration: 16,
      frame: { x: 0, y: 49, width: 130, height: 3 },
    });
    let rect = new Rect(0, 14, 10, 40);
    mem(memDataList, [{ length: 0 }], 2, 100254, 100254, frame, true);
  });

  it('MemTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 4;
    canvas.height = 4;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 270,
        y: 207,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(ProcessMemStruct.draw(ctx, data)).toBeUndefined();
  });

  it('MemTest04', function () {
    let memRender = new MemRender();
    let memReq = {
      lazyRefresh: true,
      type: '',
      startNS: 2,
      endNS: 45,
      totalNS: 43,
      frame: {
        x: 150,
        y: 210,
        width: 200,
        height: 220,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#408dec',
        globalAlpha: 0.49,
        clearRect: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => []),
        fillText: jest.fn(() => true),
      },
      lineColor: '#d90606',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 5,
      id: 1,
      x: 66,
      y: 66,
      width: 100,
      height: 100,
    };
    window.postMessage = jest.fn(() => true);
    expect(memRender.render(memReq, [], [])).toBeUndefined();
  });
});
