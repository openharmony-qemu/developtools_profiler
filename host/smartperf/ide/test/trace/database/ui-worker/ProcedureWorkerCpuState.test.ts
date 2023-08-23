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
  CpuStateRender,
  CpuStateStruct,
  cpuState,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCpuState.js';

describe('ProcedureWorkerCpuState Test', () => {
  it('ProcedureWorkerCpuStateTest01', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTs: 1,
      dur: 10,
      height: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CpuStateStruct.setFrame(node, 2, 2, 6, 4, frame)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest01', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 200,
      value: 50,
      startTs: 3,
      dur: 1,
      height: 2,
    };
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(CpuStateStruct.setFrame(node, 2, 2, 6, 4, frame)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest02', function () {
    let cpuStateRender = new CpuStateRender();
    let node = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 200,
        length: 0,
        height: 2,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(cpuStateRender.cpuState(node, [{ length: 1 }], '', [], 4, 1, 1, frame, true)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest03', function () {
    let cpuStateRender = new CpuStateRender();
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let res = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 1,
        length: 1,
        height: 2,
        dur: 1,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(cpuStateRender.cpuState([], dataList, '', res, 1, 6, 5, frame, true)).toBeUndefined();
  });

  it('ProcedureWorkerCpuStateTest04', function () {
    let cpuStateRender = new CpuStateRender();
    let dataList = new Array();
    dataList.push({
      startNS: 0,
      dur: 10,
      length: 1,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startNS: 1, dur: 2, length: 1 });
    let res = [
      {
        frame: {
          x: 20,
          y: 20,
          width: 100,
          height: 100,
        },
        startNS: 10,
        length: 1,
        height: 2,
        dur: 1,
      },
    ];
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(cpuStateRender.cpuState([], dataList, '', res, 1, 6, 5, frame, true)).toBeUndefined();
  });
});
