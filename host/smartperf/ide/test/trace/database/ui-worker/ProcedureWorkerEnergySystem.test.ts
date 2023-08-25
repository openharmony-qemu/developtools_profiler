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
  system,
  EnergySystemStruct,
  EnergySystemRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerEnergySystem.js';

describe('ProcedureWorkerEnergySystem Test', () => {
  it('ProcedureWorkerEnergySystemTest01', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let dataList = new Array();
    dataList.push({
      startNs: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNs: 1, dur: 2, length: 1 });
    system(dataList, [{ length: 1 }], 1, 3, 2, frame, true);
  });

  it('ProcedureWorkerEnergySystemTest02', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };

    let aa: any = [];
    let dataList = new Array();
    dataList.push({
      startNs: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNs: 1, dur: 2 });
    aa[0] = dataList;
    aa[1] = dataList;
    aa[2] = dataList;
    system(aa, [], 1, 3, 2, frame, false);
  });

  it('ProcedureWorkerEnergyStateTest04', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      type: 0,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergySystemStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest05', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      type: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergySystemStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest06', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      type: 2,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergySystemStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest07', function () {
    let node = {
      frame: {
        x: 10,
        y: 30,
        width: 202,
        height: 202,
      },
      startNS: 10,
      value: 22,
      startTs: 64,
      dur: 66,
      height: 2,
      type: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(EnergySystemStruct.setSystemFrame(node, 1, 1, 3, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest08', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 3,
      value: 50,
      startTs: 3,
      dur: 3,
      height: 2,
      type: 1,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(EnergySystemStruct.setSystemFrame(node, 1, 2, 3, 1, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest09', function () {
    let node = {
      frame: {
        x: 50,
        y: 50,
        width: 700,
        height: 170,
      },
      startNS: 73,
      value: 77,
      startTs: 3,
      dur: 13,
      height: 12,
      type: 12,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(EnergySystemStruct.setSystemFrame(node, 1, 1, 3, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest10', function () {
    let energySystemRender = new EnergySystemRender();
    let energySystemReq = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 22,
      totalNS: 21,
      frame: {
        x: 23,
        y: 30,
        width: 190,
        height: 960,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'sd',
      context: {
        font: '11px sans-serif',
        fillStyle: '#320011',
        globalAlpha: 0.6,
        height: 50,
        width: 100,
        canvas: {
          clientWidth: 10,
        },
        beginPath: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        stroke: jest.fn(() => true),
        fillText: jest.fn(() => true),
        closePath: jest.fn(() => true),
        fillRect: jest.fn(() => false),
        measureText: jest.fn(() => []),
      },
      lineColor: '#655f01',
      isHover: '',
      hoverX: 31,
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 39,
      id: 98,
      x: 80,
      y: 80,
      width: 400,
      height: 140,
      params: {
        isLive: true,
        maxHeight: 20,
        dpr: 21,
        hoverFuncStruct: '',
        selectFuncStruct: undefined,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(energySystemRender.render(energySystemReq, [{}], [])).toBeUndefined();
  });
});
