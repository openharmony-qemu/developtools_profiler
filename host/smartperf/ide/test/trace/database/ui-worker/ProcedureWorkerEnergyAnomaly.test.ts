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
    anomaly(dataList, [{ length: 1 }], 1, 3, 2, frame, '', true);
  });

  it('ProcedureWorkerEnergyAnomalyTest02', function () {
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
    anomaly(dataList, [{ length: 0 }], 1, 3, 2, frame, '', false);
  });

  it('ProcedureWorkerEnergyAnomalyTest03', function () {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    const data = {
      frame: {
        x: 20,
        y: 20,
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
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 1,
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
    expect(energyAnomalyRender.render(req, [], [])).toBeUndefined();
  });
});
