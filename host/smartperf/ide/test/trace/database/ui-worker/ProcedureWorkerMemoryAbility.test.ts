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

import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
//@ts-ignore
import {
  memoryAbility,
  MemoryAbilityMonitorStruct,
  MemoryAbilityRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerMemoryAbility.js';

describe('ProcedureWorkerMemoryAbility Test', () => {
  let frame = {
    x: 0,
    y: 9,
    width: 10,
    height: 10,
  };
  it('ProcedureWorkerMemoryAbilityTest01', () => {
    const data = {
      frame: {
        width: 10,
        height: 10,
        x: 1,
        y: 1,
      },
      cpu: 1,
      startNs: 1,
      value: 1,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(MemoryAbilityMonitorStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerMemoryAbilityTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2 });
    memoryAbility(dataList, [{ length: 0 }], 1, 8, 3, frame, true);
  });

  it('ProcedureWorkerMemoryAbilityTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2 });
    memoryAbility(dataList, [{ length: 0 }], 1, 8, 3, frame, false);
  });

  it('ProcedureWorkerMemoryAbilityTest03', function () {
    let memoryAbilityRender = new MemoryAbilityRender();
    let memoryAbilityReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 9,
      totalNS: 8,
      frame: {
        x: 20,
        y: 29,
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
        fillStyle: '#7800a8',
        globalAlpha: 0.6,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        fillText: jest.fn(() => []),
      },
      lineColor: '#170202',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 0,
      id: 1,
      x: 23,
      y: 21,
      width: 130,
      height: 140,
    };
    window.postMessage = jest.fn(() => true);
    expect(memoryAbilityRender.render(memoryAbilityReq, [], [])).toBeUndefined();
  });
  it('ProcedureWorkerMemoryAbilityTest04', function () {
    let memoryAbilityRender = new MemoryAbilityRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(memoryAbilityRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
