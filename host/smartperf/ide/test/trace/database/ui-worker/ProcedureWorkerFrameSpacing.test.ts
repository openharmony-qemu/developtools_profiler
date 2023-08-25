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
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import {
  FrameSpacingRender,
  FrameSpacingStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFrameSpacing.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe('FrameSpacing Test', () => {
  let frameSpacingRender = new FrameSpacingRender();
  let canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  let ctx = canvas.getContext('2d');
  let req = {
    animationRanges: [{ start: 4091445476, end: 4774481414 }],
    context: ctx,
    frameRate: 90,
    type: 'frame_spacing_slice',
    useCache: false,
  };
  TraceRow.range = {
    startNS: 0,
    endNS: 16868000000,
    totalNS: 16868000000,
  };
  let dataList = [
    {
      currentFrameHeight: 2753,
      currentFrameWidth: 1339,
      currentTs: 11618846517,
      frame: new Rect(),
      frameSpacingResult: 0.2,
      groupId: 0,
      id: 709,
      nameId: 'test',
      preFrameHeight: 2750,
      preFrameWidth: 1338,
      preTs: 11629160500,
      x: 0,
      y: 4,
    },
    {
      currentFrameHeight: 2753,
      currentFrameWidth: 1339,
      currentTs: 11629160579,
      frame: new Rect(),
      frameSpacingResult: 0.1,
      groupId: 11095334538,
      id: 709,
      nameId: 'test',
      preFrameHeight: 2750,
      preFrameWidth: 1338,
      preTs: 11618846517,
      x: 0,
      y: 4,
    },
    {
      currentFrameHeight: 2755,
      currentFrameWidth: 1340,
      currentTs: 11640114746,
      frame: new Rect(),
      frameSpacingResult: 0.1,
      groupId: 11095334538,
      id: 710,
      nameId: 'test',
      preFrameHeight: 2753,
      preFrameWidth: 1339,
      preTs: 11629160579,
      x: 0,
      y: 4,
    },
  ];
  it('FrameSpacingTest01', function () {
    expect(frameSpacingRender.render(req, dataList, TraceRow.skeleton())).toBeUndefined();
  });

  it('FrameSpacingTest02', function () {
    frameSpacingRender.frameSpacing(
      dataList,
      [],
      TraceRow.range.startNS,
      TraceRow.range.endNS,
      TraceRow.range.totalNS,
      TraceRow.skeleton(),
      req.animationRanges,
      false
    );
    expect(frameSpacingRender).not.toBeUndefined();
  });

  it('FrameSpacingTest03', function () {
    let currentStruct = {
      currentFrameHeight: 2755,
      currentFrameWidth: 1340,
      currentTs: 11640114746,
      frame: new Rect(),
      frameSpacingResult: 0.1,
      groupId: 11095334538,
      id: 710,
      nameId: 'test',
      preFrameHeight: 2753,
      preFrameWidth: 1339,
      preTs: 11629160579,
      x: 0,
      y: 4,
    };
    expect(frameSpacingRender.drawPoint(ctx, currentStruct, TraceRow.skeleton(), 0, 20)).toBeUndefined();
  });
});
