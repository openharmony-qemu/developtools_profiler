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
  HiPerfReport,
  HiPerfReportStruct,
  HiperfReportRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfReport.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon';
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';

describe('ProcedureWorkerHiPerfReport Test', () => {
  it('ProcedureWorkerHiPerfReportTest01', () => {
    let frame = {
      x: 5,
      y: 95,
      width: 50,
      height: 10,
    };
    let dataList = new Array();
    dataList.push({
      startTime: 50,
      dur: 50,
      frame: { x: 50, y: 9, width: 105, height: 150 },
    });
    dataList.push({ startTime: 51, dur: 511 });
    let dataList2 = new Array();
    dataList2.push({
      startTime: 50,
      dur: 190,
      frame: { x: 60, y: 99, width: 610, height: 16 },
    });
    dataList2.push({ startTime: 61, dur: 691 });
    let rect = new Rect(0, 10, 60, 10);
    HiPerfReport(dataList, dataList2, '', [{ length: 1 }], 1, 1024, 1024, frame, true, 1, true);
  });

  it('ProcedureWorkerHiPerfReportTest02', () => {
    let frame = {
      x: 33,
      y: 39,
      width: 130,
      height: 130,
    };
    let hiPerfReportDataList = new Array();
    hiPerfReportDataList.push({
      startTime: 1,
      dur: 11,
      frame: { x: 10, y: 91, width: 110, height: 310 },
    });
    hiPerfReportDataList.push({
      startTime: 31,
      dur: 131,
      frame: { x: 30, y: 4, width: 130, height: 103 },
    });
    let rect = new Rect(0, 30, 13, 13);
    let dataList2 = new Array();
    dataList2.push({
      startTime: 3,
      dur: 132,
      frame: { x: 23, y: 32, width: 130, height: 32 },
    });
    dataList2.push({ startTime: 21, dur: 133 });
    HiPerfReport(hiPerfReportDataList, dataList2, '', [{ length: 0 }], 1, 1024, 1024, frame, true, 1, false);
  });

  it('ProcedureWorkerHiPerfReportTest07', () => {
    let frame = {
      x: 70,
      y: 97,
      width: 60,
      height: 170,
    };
    let hiPerfReportDataList = new Array();
    hiPerfReportDataList.push({
      startTime: 5,
      dur: 5,
      frame: { x: 4, y: 93, width: 350, height: 130 },
    });
    hiPerfReportDataList.push({
      startTime: 12,
      dur: 21,
      frame: { x: 50, y: 95, width: 50, height: 640 },
    });
    let rect = new Rect(0, 60, 60, 60);
    let dataList2 = new Array();
    dataList2.push({
      startTime: 60,
      dur: 40,
      frame: { x: 40, y: 49, width: 40, height: 14 },
    });
    dataList2.push({ startTime: 17, dur: 16 });
    HiPerfReport(hiPerfReportDataList, dataList2, '', [{ length: 0 }], 1, 1024, 1024, frame, false, 1, false);
  });

  it('ProcedureWorkerHiPerfReportTest03', () => {
    const data = {
      frame: {
        x: 15,
        y: 8,
        width: 98,
        height: 80,
      },
      cpu: 1,
      startNs: 13,
      value: 198,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HiPerfReportStruct.drawRoundRectPath(ctx, 1, 0, 10, 10, 12)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfReportTest04', function () {
    let node = {
      frame: {
        x: 29,
        y: 27,
        width: 107,
        height: 170,
      },
      startNS: 70,
      value: 57,
      startTs: 34,
      dur: 121,
      height: 22,
    };
    let frame = {
      x: 30,
      y: 690,
      width: 199,
      height: 109,
    };
    expect(HiPerfReportStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfReportTest05', function () {
    let node = {
      frame: {
        x: 30,
        y: 90,
        width: 220,
        height: 1520,
      },
      startNS: 55,
      value: 522,
      startTs: 19,
      dur: 98,
      height: 8
    };
    let frame = {
      x: 150,
      y: 20,
      width: 400,
      height: 400,
    };
    expect(HiPerfReportStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfReportTest06', function () {
    expect(HiPerfReportStruct.reportGroupBy10MS([{ ps: 1 }, { coX: '1' }], 10)).toEqual([
      { dur: 10000000, height: NaN, startNS: NaN, sum: NaN },
    ]);
  });
  it('ProcedureWorkerHiPerfProcessTest05', function () {
    let hiperfReportRender = new HiperfReportRender();
    let req = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 1,
      totalNS: 1,
      frame: {
        x: 210,
        y: 120,
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
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
        clearRect: jest.fn(() => true),
        beginPath: jest.fn(() => true),
        fillRect: jest.fn(() => true),
        fillText: jest.fn(() => true),
        fill: jest.fn(() => true),
        stroke: jest.fn(() => true),
        closePath: jest.fn(() => true),
        measureText: jest.fn(() => true),
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
      scale: 100_000_001,
    };
    window.postMessage = jest.fn(() => true);
    expect(hiperfReportRender.render(req, [], [], [])).toBeUndefined();
  });
});
