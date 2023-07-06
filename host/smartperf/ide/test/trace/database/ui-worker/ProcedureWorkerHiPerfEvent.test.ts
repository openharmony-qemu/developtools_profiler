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
  HiPerfEventStruct,
  HiperfEventRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHiPerfEvent.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon';

describe('ProcedureWorkerHiPerfEvent Test', () => {
  it('ProcedureWorkerHiPerfEventTest03', () => {
    const data = {
      frame: {
        x: 0,
        y: 9,
        width: 10,
        height: 10,
      },
      cpu: 1,
      startNs: 1,
      value: 1,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HiPerfEventStruct.drawRoundRectPath(ctx, 1, 0, 10, 10, 12)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfEventTest04', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 0,
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
    expect(HiPerfEventStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfEventTest05', function () {
    let node = {
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      startNS: 2,
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
    expect(HiPerfEventStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerHiPerfEventTest06', function () {
    expect(HiPerfEventStruct.groupBy10MS([{ ps: 1 }, { coX: '1' }], 10, '')).toEqual([
      { dur: 10000000, height: Infinity, startNS: NaN},
    ]);
  });
});
