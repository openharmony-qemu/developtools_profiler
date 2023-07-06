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

//@ts-ignore
import { hiPerfCpu, HiPerfCpuStruct,HiperfCpuRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfCPU.js';
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';

describe('ProcedureWorkerHiPerfCPU Test', () => {
  let frame = {
    x: 0,
    y: 9,
    width: 10,
    height: 10,
  };
  it('ProcedureWorkerHiPerfCPUTest01', () => {
    const data = {
      frame: undefined,
      cpu: 1,
      startNs: 1,
      value: 1,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HiPerfCpuStruct.draw(ctx, '', data, true)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfCPUTest04', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HiPerfCpuStruct.drawRoundRectPath(ctx, 1, 1, 1, 1, 1)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfCPUTest05', function () {
    expect(HiPerfCpuStruct.groupBy10MS([{ id: 1, NS: 3 }, { copy: '1' }], 10, '')).toEqual([
      { dur: 10000000, height: Infinity, startNS: NaN },
    ]);
  });
});
