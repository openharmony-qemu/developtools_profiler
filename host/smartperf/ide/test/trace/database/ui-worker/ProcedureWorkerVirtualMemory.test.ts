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
  setMemFrame,
  VirtualMemoryStruct,
  VirtualMemoryRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerVirtualMemory.js';
// @ts-ignore
import { mem } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon.js';

describe('ProcedureWorkerVirtualMemory Test', () => {
  it('ProcedureWorkerVirtualMemoryTest01', function () {
    let frame = {
      x: 34,
      y: 44,
      width: 144,
      height: 430,
    };
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 130,
      frame: { x: 30, y: 44, width: 10, height: 140 },
    });
    dataList.push({ startTime: 41, dur: 411 });
    mem(dataList, [{ length: 1 }], 1, 1, 1, frame, true);
  });

  it('ProcedureWorkerVirtualMemoryTest02', function () {
    let frame = {
      x: 70,
      y: 77,
      width: 443,
      height: 180,
    };
    let dataList = new Array();
    dataList.push({
      startTime: 80,
      dur: 180,
      frame: { x: 870, y: 97, width: 177, height: 107 },
    });
    dataList.push({ startTime: 71, dur: 178 });
    mem(dataList, [{ length: 0 }], 1, 1, 1, frame, false);
  });

  it('ProcedureWorkerVirtualMemoryTest03', () => {
    const data = {
      cpu: 1,
      startNs: 14,
      value: 143,
      frame: {
        x: 44,
        y: 43,
        width: 120,
        height: 233,
      },
      maxValue: undefined,
      startTime: 1,
      filterID: 5,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(VirtualMemoryStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerVirtualMemoryTest04', function () {
    let virtualMemoryRender = new VirtualMemoryRender();
    let virtualMemoryReq = {
      lazyRefresh: true,
      type: '',
      startNS: 22,
      endNS: 155,
      totalNS: 133,
      frame: {
        x: 20,
        y: 20,
        width: 165,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#f3a97e',
        globalAlpha: 0.6,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => []),
        fillRect: jest.fn(() => true),
      },
      lineColor: '#ff0000',
      isHover: '',
      hoverX: 71,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 557,
      id: 1,
      x: 20,
      y: 20,
      width: 121,
      height: 121,
    };
    window.postMessage = jest.fn(() => true);
    expect(virtualMemoryRender.render(virtualMemoryReq, [], [])).toBeUndefined();
  });
  it('ProcedureWorkerVirtualMemoryTest05', function () {
    let virtualMemoryRender = new VirtualMemoryRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(virtualMemoryRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
