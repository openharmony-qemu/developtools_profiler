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
      startNs: 1,
      value: 1,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
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
      scale: 100_000_001,
    };
    window.postMessage = jest.fn(() => true);
    expect(hiperfProcessRender.render(req, [], [], [])).toBeUndefined();
  });
});
