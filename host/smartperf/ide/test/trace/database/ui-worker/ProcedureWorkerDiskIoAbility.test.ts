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
  DiskAbilityMonitorStruct,
  diskIoAbility,
  DiskIoAbilityRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerDiskIoAbility.js';
//@ts-ignore
import { Rect } from '../../../dist/trace/database/ProcedureWorkerCommon';

describe('ProcedureWorkerDiskIoAbility Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 6;
  canvas.height = 6;
  const ctx = canvas.getContext('2d');

  const data = {
    frame: {
      x: 206,
      y: 206,
      width: 100,
      height: 100,
    },
    startNS: 200,
    value: 50,
  };
  const Sourcedata = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    maxDiskRate: 300,
    value: 80,
  };
  let res = [
    {
      startNS: 11,
      dur: 30,
      frame: {
        x: 6,
        y: 49,
        width: 30,
        height: 60,
      },
    },
  ];

  it('ProcedureWorkerDiskIoAbilityTest01', function () {
    expect(DiskAbilityMonitorStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerDiskIoAbilityTest03', function () {
    expect(DiskAbilityMonitorStruct.draw(ctx, Sourcedata)).toBeUndefined();
  });
  it('CpuAbilityMonitorStructTest02', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 111 });
    diskIoAbility(dataList, [{ length: 1 }], 1, 100254, 100254, '', true);
  });

  it('CpuAbilityMonitorStructTest03', function () {
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 111 });
    diskIoAbility(dataList, [{ length: 0 }], 1, 100254, 100254, '', false);
  });

  it('CpuAbilityMonitorStructTest04', function () {
    let diskIoAbilityRender = new DiskIoAbilityRender();
    let diskIoReq = {
      lazyRefresh: true,
      type: '',
      startNS: 5,
      endNS: 9,
      totalNS: 4,
      frame: {
        x: 32,
        y: 20,
        width: 180,
        height: 180,
      },
      useCache: true,
      range: {
        refresh: '',
      },
      canvas: 'a',
      context: {
        font: '12px sans-serif',
        fillStyle: '#a1697d',
        globalAlpha: 0.3,
        measureText: jest.fn(() => true),
        clearRect: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => false),
        beginPath: jest.fn(() => true),
        fillRect: jest.fn(() => false),
        fillText: jest.fn(() => true),
      },
      lineColor: '',
      isHover: 'true',
      hoverX: 0,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 4,
      id: 1,
      x: 24,
      y: 24,
      width: 100,
      height: 100,
    };
    window.postMessage = jest.fn(() => true);
    expect(diskIoAbilityRender.render(diskIoReq, [], [])).toBeUndefined();
  });
  it('CpuAbilityMonitorStructTest05', function () {
    let diskIoAbilityRender = new DiskIoAbilityRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(diskIoAbilityRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
