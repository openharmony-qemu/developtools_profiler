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
    state(dataList, [{ length: 1 }], 1, 3, 2, frame, true);
  });

  it('ProcedureWorkerEnergyStateTest02', function () {
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
    state(dataList, [{ length: 0 }], 1, 3, 2, frame, false);
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
    expect(energyStateRender.render(req, [{}], [])).toBeUndefined();
  });
});
