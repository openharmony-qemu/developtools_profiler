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
import {
  heap,
  HeapStruct,
  NativeMemoryRender,
  HeapRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHeap.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe(' Test', () => {
  it('HeapTest01', () => {
    let heapDataList = new Array();
    heapDataList.push({
      startTime: 40,
      dur: 150,
      frame: { x: 0, y: 19, width: 20, height: 10 },
    });
    heapDataList.push({ startTime: 12, dur: 21 });
    let rect = new Rect(0, 10, 30, 10);
    let res = [
      {
        startTs: 11,
        dur: 166,
        length: 15,
        frame: '',
      },
    ];
    heap(heapDataList, res, 1, 100254, 100254, rect, true);
  });

  it('HeapTest02', () => {
    let heapHataList = new Array();
    heapHataList.push({
      startTime: 1,
      dur: 118,
      frame: { x: 60, y: 9, width: 10, height: 10 },
    });
    heapHataList.push({
      startTime: 1,
      dur: 15,
      frame: { x: 0, y: 19, width: 110, height: 130 },
    });
    let rect = new Rect(0, 10, 10, 10);
    let res = [
      {
        startTs: 0,
        dur: 10,
        length: 0,
        frame: '',
      },
    ];
    heap(heapHataList, res, 1, 100254, 100254, rect, false);
  });

  it('HeapTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 11;
    canvas.height = 12;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 250,
        y: 250,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(HeapStruct.drawHeap(ctx, data, 0)).toBeUndefined();
  });
  it('HeapTest04', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 11;
    canvas.height = 11;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 205,
        y: 205,
        width: 100,
        height: 100,
      },
      maxHeapSize: 200,
      value: 50,
    };
    expect(HeapStruct.drawHeap(ctx, data, 1)).toBeUndefined();
  });

  it('HeapTest05', function () {
    let heapRender = new HeapRender();
    let heapReq = {
      lazyRefresh: true,
      type: '',
      startNS: 3,
      endNS: 9,
      totalNS: 6,
      frame: {
        x: 20,
        y: 20,
        width: 200,
        height: 200,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'canvas',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.56,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => false),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        stroke: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        fillText: jest.fn(() => true),
      },
      lineColor: '#666666',
      isHover: '',
      hoverX: 1,
      params: 'params',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 1,
      id: 1,
      x: 20,
      y: 20,
      width: 320,
      height: 320,
    };
    window.postMessage = jest.fn(() => true);
    expect(heapRender.render(heapReq, [], [])).toBeUndefined();
  });
  it('HeapTest04', () => {
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
    const node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
    };
    expect(HeapStruct.setFrame(node, 2, 1, 5, 4, data)).toBeUndefined();
  });
  it('HeapTest07', function () {
    let heapRender = new HeapRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(heapRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
