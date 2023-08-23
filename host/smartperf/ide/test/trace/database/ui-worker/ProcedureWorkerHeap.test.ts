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
  heap,
  HeapStruct,
  NativeMemoryRender,
  HeapRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHeap.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe(' Test', () => {
  it('HeapTest01', () => {
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    let rect = new Rect(0, 10, 10, 10);
    let res = [
      {
        startTs: 0,
        dur: 10,
        length: 1,
        frame: '',
      },
    ];
    heap(dataList, res, 1, 100254, 100254, rect, true);
  });

  it('HeapTest02', () => {
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({
      startTime: 1,
      dur: 111,
      frame: { x: 0, y: 9, width: 10, height: 10 },
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
    heap(dataList, res, 1, 100254, 100254, rect, false);
  });

  it('HeapTest03', () => {
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
    expect(HeapStruct.draw(ctx, data)).toBeUndefined();
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
      maxHeapSize: 200,
      value: 50,
    };
    expect(HeapStruct.draw(ctx, data)).toBeUndefined();
  });

  it('HeapTest05', function () {
    let nativeMemoryRender = new HeapRender();
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
    expect(nativeMemoryRender.render(req, [], [])).toBeUndefined();
  });
});
