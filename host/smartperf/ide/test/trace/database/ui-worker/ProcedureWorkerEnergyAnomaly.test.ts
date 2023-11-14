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
  anomaly,
  EnergyAnomalyStruct,
  EnergyAnomalyRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerEnergyAnomaly.js';

describe('ProcedureWorkerEnergyAnomaly Test', () => {
  it('ProcedureWorkerEnergyAnomalyTest01', function () {
    let frame = {
      x: 30,
      y: 20,
      width: 550,
      height: 500,
    };
    let energyAnomalyDataList = new Array();
    energyAnomalyDataList.push({
      startNS: 111,
      dur: 40,
      length: 23,
      frame: { x: 0, y: 29, width: 22, height: 101 },
    });
    energyAnomalyDataList.push({ startNS: 11, dur: 21, length: 10 });
    anomaly(energyAnomalyDataList, [{ length: 1 }], 1, 3, 2, frame, '', true);
  });

  it('ProcedureWorkerEnergyAnomalyTest02', function () {
    let frame = {
      x: 50,
      y: 32,
      width: 600,
      height: 200,
    };
    let energyAnomalyDataList = new Array();
    energyAnomalyDataList.push({
      startNS: 22,
      dur: 30,
      length: 25,
      frame: { x: 0, y: 19, width: 32, height: 102 },
    });
    energyAnomalyDataList.push({ startNS: 12, dur: 22, length: 12 });
    anomaly(energyAnomalyDataList, [{ length: 0 }], 1, 3, 2, frame, '', false);
  });

  it('ProcedureWorkerEnergyAnomalyTest03', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 7;
    canvas.height = 7;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 207,
        y: 207,
        width: 100,
        height: 100,
      },
    };
    let path = new Path2D();
    expect(EnergyAnomalyStruct.draw(ctx, path, data)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyAnomalyTest04', function () {
    let node = {
      frame: {
        x: 20,
        y: 50,
        width: 100,
        height: 500,
      },
      startNS: 56,
      value: 60,
      startTs: 3,
      dur: 1,
      height: 2,
    };
    let frame = {
      x: 22,
      y: 22,
      width: 130,
      height: 130,
    };
    expect(EnergyAnomalyStruct.setAnomalyFrame(node, 1, 2, 5, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyAnomalyTest05', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 6,
      value: 50,
      startTs: 3,
      dur: 3,
      height: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(EnergyAnomalyStruct.setAnomalyFrame(node, 1, 2, 5, frame)).toBeUndefined();
  });

  it('ProcedureWorkerEnergyAnomalyTest06', function () {
    let energyAnomalyRender = new EnergyAnomalyRender();
    let energyAnomalyReq = {
      lazyRefresh: true,
      type: '',
      startNS: 0,
      endNS: 9,
      totalNS: 9,
      frame: {
        x: 30,
        y: 30,
        width: 140,
        height: 140,
      },
      useCache: false,
      range: {
        refresh: '12',
      },
      canvas: 'b',
      context: {
        font: '13px sans-serif',
        fillStyle: '#e00f55',
        globalAlpha: 0.4,
        canvas: {
          clientWidth: 12,
        },
        clearRect: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => false),
        fillRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        stroke: jest.fn(() => true),
        fillText: jest.fn(() => true),
      },
      lineColor: '#000000',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: true,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 6,
      id: 1,
      x: 20,
      y: 20,
      width: 150,
      height: 150,
    };
    window.postMessage = jest.fn(() => true);
    expect(energyAnomalyRender.render(energyAnomalyReq, [], [])).toBeUndefined();
  });
});
