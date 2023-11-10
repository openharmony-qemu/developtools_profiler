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

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

// @ts-ignore
import { FrameAnimationRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFrameAnimation.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { FrameAnimationStruct } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFrameAnimation.js';

describe('FrameAnimation Test', () => {
  let frameAnimationRender = new FrameAnimationRender();
  let rect = new Rect(341, 2, 10, 10);
  let canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  let ctx = canvas.getContext('2d');
  let dataList = [
    {
      animationId: 1,
      dur: 0,
      dynamicEndTs: 4774481414,
      dynamicStartTs: 4091445476,
      frame: rect,
      status: 'Response delay',
      textMetricsWidth: 115.44140625,
      ts: 4091445476,
    },
    {
      animationId: 1,
      dur: 683035938,
      dynamicEndTs: 4774481414,
      dynamicStartTs: 4091445476,
      frame: rect,
      status: 'Completion delay',
      textMetricsWidth: 133.0703125,
      ts: 4091445476,
    },
  ];
  TraceRow.range = {
    startNS: 0,
    endNS: 16868000000,
    totalNS: 16868000000,
  };

  it('FrameAnimationTest01', function () {
    frameAnimationRender.frameAnimation(
      dataList,
      [],
      TraceRow.range.startNS,
      TraceRow.range.endNS,
      TraceRow.range.totalNS,
      TraceRow.skeleton(),
      false
    );
    let node = {
      animationId: 1,
      dur: 0,
      dynamicEndTs: 4774481414,
      dynamicStartTs: 4091445476,
      frame: rect,
      status: 'Response delay',
      textMetricsWidth: 115.44140625,
      ts: 4091445476,
    };
    expect(FrameAnimationStruct.draw(ctx!, 1, node, TraceRow.skeleton())).toBeUndefined();
  });
});
