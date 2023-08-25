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
//@ts-ignore
import {
  HiPerfProcessStruct,
  HiperfProcessRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfProcess.js';
// @ts-ignore
import { hiPerf } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js';

describe('ProcedureWorkerHiPerfProcess Test', () => {
  it('ProcedureWorkerHiPerfProcessTest01', () => {
    const data = {
      frame: undefined,
      cpu: 1,
      startNs: 41,
      value: 41,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 2;
    const ctx = canvas.getContext('2d');
    expect(HiPerfProcessStruct.draw(ctx, '', data, true)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfProcessTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    hiPerf(dataList, [{ length: 0 }], dataList, 8, 3, '', true, 1, true);
  });

  it('ProcedureWorkerHiPerfProcessTest03', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    hiPerf(dataList, [{ length: 0 }], dataList, 8, 3, '', false, 1, false);
  });

  it('ProcedureWorkerHiPerfProcessTest04', function () {
    expect(HiPerfProcessStruct.groupBy10MS([{ a: '1', b: 2, c: 3 }], 10)).toEqual([
      { dur: 10000000, height: 40, startNS: NaN },
    ]);
  });

  it('ProcedureWorkerHiPerfProcessTest05', function () {
    let hiperfProcessRender = new HiperfProcessRender();
    let hiperfProcessReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 12,
      totalNS: 11,
      frame: {
        x: 145,
        y: 202,
        width: 110,
        height: 101,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 's',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.3,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        fillText: jest.fn(() => true),
        fill: jest.fn(() => false),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => ''),
        fillRect: jest.fn(() => true),
      },
      lineColor: '',
      isHover: '',
      hoverX: 1,
      params: 'q',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 7,
      id: 1,
      x: 27,
      y: 27,
      width: 100,
      height: 100,
      scale: 100_070_001,
    };
    window.postMessage = jest.fn(() => true);
    expect(hiperfProcessRender.render(hiperfProcessReq, [], [], [])).toBeUndefined();
  });
  it('ProcedureWorkerHiPerfProcessTest06', function () {
    let hiperfProcessRender = new HiperfProcessRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(hiperfProcessRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
