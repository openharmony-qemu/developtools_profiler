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
  HiperfThreadRender,
  HiPerfThreadStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfThread.js';
// @ts-ignore
import { hiPerf } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js';

describe('ProcedureWorkerHiPerfThread Test', () => {
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
  it('ProcedureWorkerHiPerfThreadTest01', () => {
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
    expect(HiPerfThreadStruct.draw(ctx, '', data, true)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfThreadTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let frame = {
      x: 0,
      y: 9,
      width: 10,
      height: 10,
    };
    hiPerf(dataList, [{ length: 0 }], dataList, 8, 3, frame, false, 1, false);
  });

  it('ProcedureWorkerHiPerfThreadTest03', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let frame = {
      x: 0,
      y: 9,
      width: 10,
      height: 10,
    };
    hiPerf(dataList, [{ length: 1 }], dataList, 8, 3, frame, true, 1, true);
  });

  it('ProcedureWorkerHiPerfThreadTest04', function () {
    expect(HiPerfThreadStruct.groupBy10MS([{ ps: 1 }, { coX: '1' }], 10, '')).toEqual([
      { dur: 10000000, height: Infinity, startNS: NaN },
    ]);
  });

  it('ProcedureWorkerHiPerfThreadTest05', function () {
    let hiperfThreadRender = new HiperfThreadRender();
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
    let a = {
      dataList: [
        {
          callchain_id: 1329,
          thread_name: 'uinput_inject',
          tid: 247,
          pid: 247,
          startNS: 1179247952,
          timestamp_group: 1170000000,
        },
        {
          callchain_id: 1330,
          thread_name: 'uinput_inject',
          tid: 247,
          pid: 247,
          startNS: 1179308910,
          timestamp_group: 1170000000,
        },
      ],
    };

    expect(hiperfThreadRender.render(req, [], [], [])).toBeUndefined();
  });
});
