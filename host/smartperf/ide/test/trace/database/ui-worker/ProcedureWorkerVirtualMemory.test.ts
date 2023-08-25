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
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    mem(dataList, [{ length: 1 }], 1, 1, 1, frame, true);
  });

  it('ProcedureWorkerVirtualMemoryTest02', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    mem(dataList, [{ length: 0 }], 1, 1, 1, frame, false);
  });

  it('ProcedureWorkerVirtualMemoryTest03', () => {
    const data = {
      cpu: 1,
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      maxValue: undefined,
      startTime: 1,
      filterID: 2,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(VirtualMemoryStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerVirtualMemoryTest04', function () {
    let virtualMemoryRender = new VirtualMemoryRender();
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
      canvas: 'a',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        fillRect: jest.fn(() => true),
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
    expect(virtualMemoryRender.render(req, [], [])).toBeUndefined();
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
