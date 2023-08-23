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

import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import {
  HeapTimelineRender,
  HeapTimelineStruct,
  HeapTimeline,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHeapTimeline.js';

describe('ProcedureWorkerHeapTimeline Test', () => {
  it('HeapTimelineTest', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    let rect = new Rect(0, 10, 10, 10);
    HeapTimeline(canvas, ctx, 1, 100254, 100254, rect, (e: any) => {});
  });
  it('HeapTimelineStructTest01', () => {
    const data = {
      cpu: 1,
      startNs: 1,
      value: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      maxValue: undefined,
      startTime: 1,
      filterID: 2,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HeapTimelineStruct.draw(ctx, data)).toBeUndefined();
  });
});
