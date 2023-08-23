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
  EnergyPowerStruct,
  EnergyPowerRender,
  power,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerEnergyPower.js';

describe('ProcedureWorkerEnergyPower Test', () => {
  it('ProcedureWorkerEnergyPowerTest01', function () {
    let req = {
      context: {
        globalAlpha: 1.0,
        lineWidth: 1,
        fillStyle: '#333',
      },
    };
    let data = {
      cpu: 1,
      location: 2,
      gpu: 1,
      display: 1,
      camera: 1,
      bluetooth: 3,
      flashlight: 10,
      audio: 16,
      wifiscan: 1,
      ts: 10,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    let row = { frame: 20 };
    EnergyPowerStruct.drawHistogram = jest.fn(() => true);
    EnergyPowerStruct.drawPolyline = jest.fn(() => true);
    expect(EnergyPowerStruct.draw(req, 3, data, row)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyPowerTest02', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let node = {
      ts: 10,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergyPowerStruct.setPowerFrame(node, 1, 2, 5, 3, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyPowerTest03', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let node = {
      ts: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergyPowerStruct.setPowerFrame(node, 1, 2, 2000000002, 2000000000, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyPowerTest04', function () {
    expect(EnergyPowerStruct.getHistogramColor('CPU')).toBe('#92D6CC');
  });

  it('ProcedureWorkerEnergyPowerTest05', function () {
    expect(EnergyPowerStruct.getHistogramColor('LOCATION')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyPowerTest06', function () {
    expect(EnergyPowerStruct.getHistogramColor('GPU')).toBe('#86C5E3');
  });

  it('ProcedureWorkerEnergyPowerTest07', function () {
    expect(EnergyPowerStruct.getHistogramColor('DISPLAY')).toBe('#46B1E3');
  });

  it('ProcedureWorkerEnergyPowerTest08', function () {
    expect(EnergyPowerStruct.getHistogramColor('CAMERA')).toBe('#C386F0');
  });

  it('ProcedureWorkerEnergyPowerTest09', function () {
    expect(EnergyPowerStruct.getHistogramColor('BLUETOOTH')).toBe('#8981F7');
  });

  it('ProcedureWorkerEnergyPowerTest10', function () {
    expect(EnergyPowerStruct.getHistogramColor('AUDIO')).toBe('#AC49F5');
  });

  it('ProcedureWorkerEnergyPowerTest11', function () {
    expect(EnergyPowerStruct.getHistogramColor('WIFISCAN')).toBe('#92C4BD');
  });

  it('ProcedureWorkerEnergyPowerTest12', function () {
    expect(EnergyPowerStruct.getHistogramColor('WIFISCANxcda')).toBe('#564AF7');
  });

  it('ProcedureWorkerEnergyPowerTest13', function () {
    expect(EnergyPowerStruct).not.toBeUndefined();
  });

  it('ProcedureWorkerEnergyPowerTest14', function () {
    let energyPowerRender = new EnergyPowerRender();
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
    expect(energyPowerRender.render(req, [], [])).toBeUndefined();
  });

  it('ProcedureWorkerEnergyPowerTest15', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    power(dataList, [{ length: 1 }], 1, 3, 2, frame, true, '');
  });

  it('ProcedureWorkerEnergyPowerTest16', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    power(dataList, [{ length: 0 }], 1, 3, 2, frame, false, '');
  });
});
