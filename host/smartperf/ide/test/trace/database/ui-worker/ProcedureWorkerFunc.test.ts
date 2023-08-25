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
import { func, FuncStruct, FuncRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFunc.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
import { markAsUntransferable } from 'worker_threads';

describe(' ProcedureWorkerFuncTest', () => {
  it('FuncTest01', () => {
    let funcDataList = new Array();
    funcDataList.push({
      startTime: 10,
      dur: 410,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    funcDataList.push({ startTime: 17, dur: 141 });
    let rect = new Rect(0, 30, 30, 30);
    let res = [
      {
        startTs: 31,
        dur: 140,
        length: 16,
        frame: '',
      },
    ];
    func(funcDataList, res, 1, 100254, 100254, rect, true);
  });

  it('FuncTest02', () => {
    let funcDataList = new Array();
    funcDataList.push({
      startTime: 450,
      dur: 140,
      frame: { x: 0, y: 93, width: 120, height: 320 },
    });
    funcDataList.push({
      startTime: 41,
      dur: 661,
      frame: { x: 70, y: 9, width: 16, height: 17 },
    });
    let rect = new Rect(30, 50, 53, 13);
    let res = [
      {
        startTs: 10,
        dur: 10,
        length: 60,
        frame: '',
      },
    ];
    func(funcDataList, res, 1, 100254, 100254, rect, false);
  });

  it('FuncTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 9;
    canvas.height = 9;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 209,
        y: 209,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      dur: undefined || null || 0,
      funName: '',
    };
    expect(FuncStruct.draw(ctx, data)).toBeUndefined();
  });

  it('FuncTest04', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 240,
        y: 240,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      dur: 10,
      funName: 'H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled',
    };
    expect(FuncStruct.draw(ctx, data)).toBeUndefined();
  });

  it('FuncTest07', function () {
    let str = '';
    expect(FuncStruct.isBinder({})).toBe(false);
  });

  it('FuncTest08', function () {
    let data = {
      startTs: 2,
      depth: 1,
    };
    expect(FuncStruct.isSelected(data)).toBe(false);
  });

  it('FuncTest09', function () {
    let funcRender = new FuncRender();
    let req = {
      lazyRefresh: undefined,
      type: '',
      startNS: 31,
      endNS: 71,
      totalNS: 40,
      frame: {
        x: 30,
        y: 22,
        width: 550,
        height: 150,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: '',
      context: {
        font: '11px sans-serif',
        fillStyle: '#30a16f',
        globalAlpha: 0.556,
        height: 177,
        width: 150,
      },
      lineColor: '#014d5f',
      isHover: '',
      hoverX: 21,
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 53,
      id: 64,
      x: 760,
      y: 67,
      width: 106,
      height: 170,
      params: {
        isLive: false,
        maxHeight: 222,
        dpr: 431,
        hoverFuncStruct: '',
        selectFuncStruct: undefined,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(funcRender.render(req, [], [])).toBeUndefined();
  });
});
