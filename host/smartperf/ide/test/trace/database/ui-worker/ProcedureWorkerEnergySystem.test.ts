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
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 1,
      value: 50,
      startTs: 3,
      dur: 3,
      height: 2,
      type: 0,
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
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 1,
      value: 50,
      startTs: 3,
      dur: 3,
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

  it('ProcedureWorkerEnergyStateTest10', function () {
    let energySystemRender = new EnergySystemRender();
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
        height: 150,
        width: 100,
      },
      lineColor: '',
      isHover: '',
      hoverX: 1,
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 3,
      id: 1,
      x: 20,
      y: 20,
      width: 100,
      height: 100,
      params: {
        isLive: false,
        maxHeight: 2,
        dpr: 1,
        hoverFuncStruct: '',
        selectFuncStruct: undefined,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(energySystemRender.render(req, [{}], [])).toBeUndefined();
  });
});
