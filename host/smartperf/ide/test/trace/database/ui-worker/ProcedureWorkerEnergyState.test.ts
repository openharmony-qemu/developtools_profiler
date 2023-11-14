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
  state,
  EnergyStateStruct,
  EnergyStateRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerEnergyState.js';

describe('ProcedureWorkerEnergyState Test', () => {
  it('ProcedureWorkerEnergyStateTest01', function () {
    let frame = {
      x: 40,
      y: 27,
      width: 300,
      height: 370,
    };
    let energyStateDataList = new Array();
    energyStateDataList.push({
      startNS: 0,
      dur: 20,
      length: 51,
      frame: { x: 0, y: 9, width: 105, height: 110 },
    });
    energyStateDataList.push({ startNS: 1, dur: 42, length: 32 });
    state(energyStateDataList, [{ length: 1 }], 1, 3, 2, frame, true);
  });

  it('ProcedureWorkerEnergyStateTest02', function () {
    let frame = {
      x: 20,
      y: 30,
      width: 520,
      height: 230,
    };
    let energyStateDataList = new Array();
    energyStateDataList.push({
      startNS: 0,
      dur: 10,
      length: 15,
      frame: { x: 50, y: 59, width: 177, height: 70 },
    });
    energyStateDataList.push({ startNS: 15, dur: 23, length: 17 });
    state(energyStateDataList, [{ length: 0 }], 1, 3, 2, frame, false);
  });

  it('ProcedureWorkerEnergyStateTest03', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      type: '',
      value: 0,
      startNs: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
    };
    expect(EnergyStateStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyStateTest04', function () {
    expect(EnergyStateStruct.setDrawColor('BRIGHTNESS_NIT')).toBe('#92D6CC');
  });

  it('ProcedureWorkerEnergyStateTest05', function () {
    expect(EnergyStateStruct.setDrawColor('SIGNAL_LEVEL')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyStateTest06', function () {
    expect(EnergyStateStruct.setDrawColor('WIFI_EVENT_RECEIVED')).toBe('#46B1E3');
  });

  it('ProcedureWorkerEnergyStateTest07', function () {
    expect(EnergyStateStruct.setDrawColor('AUDIO_STREAM_CHANGE')).toBe('#ED6F21');
  });

  it('ProcedureWorkerEnergyStateTest08', function () {
    expect(EnergyStateStruct.setDrawColor('WIFI_STATE')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyStateTest09', function () {
    expect(EnergyStateStruct.setDrawColor('LOCATION_SWITCH_STATE')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyStateTest10', function () {
    expect(EnergyStateStruct.setDrawColor('SENSOR_STATE')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyStateTest11', function () {
    expect(EnergyStateStruct.setDrawColor('aaaa')).toBe('#61CFBE');
  });

  it('ProcedureWorkerEnergyStateTest12', function () {
    let energyStateRender = new EnergyStateRender();
    let energyStateReq = {
      lazyRefresh: true,
      type: '',
      startNS: 7,
      endNS: 8,
      totalNS: 1,
      frame: {
        x: 50,
        y: 25,
        width: 500,
        height: 220,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: 'bc',
      context: {
        font: '14px sans-serif',
        fillStyle: '#151212',
        globalAlpha: 0.62,
        height: 150,
        width: 110,
        canvas: {
          clientWidth: 50,
        },
        clearRect: jest.fn(() => true),
        measureText: jest.fn(() => true),
        fillRect: jest.fn(() => []),
        fillText: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        arc:jest.fn(() => true),
        fill:jest.fn(() => true),
        moveTo:jest.fn(() => true),
        lineTo:jest.fn(() => true),
      },
      lineColor: '#1a4dff',
      isHover: '',
      hoverX: 22,
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 584,
      id: 2,
      x: 32,
      y: 30,
      width: 100,
      height: 100,
      params: {
        isLive: true,
        maxHeight: 21,
        dpr: 1,
        hoverFuncStruct: '',
        selectFuncStruct: undefined,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(energyStateRender.render(energyStateReq, [{}], [])).toBeUndefined();
  });
});
