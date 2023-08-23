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
  proc,
  ProcessStruct,
  ProcessRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerProcess.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe(' ProcessTest', () => {
  let res = [
    {
      startNS: 0,
      dur: 10,
      frame: {
        x: 0,
        y: 9,
        width: 10,
        height: 10,
      },
    },
  ];
  it('ProcessTest01', () => {
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    let rect = new Rect(0, 10, 10, 10);
    proc(dataList, res, 1, 100254, 100254, rect);
  });

  it('ProcessTest02', () => {
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
    proc(dataList, res, 1, 100254, 100254, rect);
  });

  it('ProcessTest04', () => {
    const node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTime: 0,
      dur: 0,
    };
    const frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(ProcessStruct.setFrame(node, 1, 1, 1, frame)).toBeUndefined();
  });

  it('ProcessTest05', () => {
    const node = {
      frame: {
        x: 20,
        y: 20,
        width: 0,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTime: 2,
      dur: 2,
    };
    const frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(ProcessStruct.setFrame(node, 1, 1, 1, frame)).toBeUndefined();
  });

  it('ProcessTest06', function () {
    let processRender = new ProcessRender();
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
    expect(processRender.render(req, [], [])).toBeUndefined();
  });
});
