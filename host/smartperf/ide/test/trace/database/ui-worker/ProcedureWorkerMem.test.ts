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
import {  ProcessMemStruct, MemRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerMem.js';
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
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      duration: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({
      startTime: 1,
      duration: 111,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    let rect = new Rect(0, 10, 10, 10);
    mem(dataList, [{ length: 0 }], 2, 100254, 100254, frame, false);
  });

  it('MemTest02', () => {
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      duration: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({
      startTime: 1,
      duration: 111,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    let rect = new Rect(0, 10, 10, 10);
    mem(dataList, [{ length: 0 }], 2, 100254, 100254, frame, true);
  });

  it('MemTest03', () => {
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
    expect(ProcessMemStruct.draw(ctx, data)).toBeUndefined();
  });

  it('MemTest04', function () {
    let memRender = new MemRender();
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
    expect(memRender.render(req, [], [])).toBeUndefined();
  });
});
