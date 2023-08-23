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
  CpuFreqLimitRender,
  CpuFreqLimitsStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCpuFreqLimits.js';

describe('ProcedureWorkerCpuFreqLimits Test', () => {
  let cpuFreqLimits = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    startNs: 255,
    dur: 2545,
    max: 14111,
    min: 200,
    cpu: 10,
  };
  it('Test01', () => {
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
      startNs: 54,
      dur: 2453,
      max: 3433,
      min: 13,
      cpu: 3,
    };
    expect(CpuFreqLimitsStruct.draw(ctx!, data, 2)).toBeUndefined();
  });

  it('Test02', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(CpuFreqLimitsStruct.drawArcLine(ctx, cpuFreqLimits, 100, 500)).toBeUndefined();
  });

  it('Test03', () => {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      length: 1,
      height: 0,
      startTime: 2,
      dur: 1,
    };
    expect(
      CpuFreqLimitsStruct.setFreqLimitFrame(node, 1, 1, 1, 1, {
        width: 10,
      })
    ).toBeUndefined();
  });

  it('Test04', function () {
    let cpuFreqLimitRender = new CpuFreqLimitRender();
    let req = {
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
      canvas: '',
      context: {},
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
    expect(cpuFreqLimitRender.render(req, [], [])).toBeUndefined();
  });
});
