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
import {
  FrameDynamicRender,
  FrameDynamicStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFrameDynamic.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { AnimationRanges } from '../../../../dist/trace/bean/FrameComponentBean.js';

describe('FrameDynamic Test', () => {
  let frameDynamicRender = new FrameDynamicRender();
  let rect = new Rect(341, 2, 10, 10);
  let canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  let ctx = canvas.getContext('2d');
  TraceRow.range = {
    startNS: 0,
    endNS: 16868000000,
    totalNS: 16868000000,
  };
  let dataList = [
    {
      alpha: '0.00',
      appName: 'test',
      groupId: 4091445480,
      height: 0,
      id: 455,
      ts: 4091445480,
      width: 0,
      x: 0,
      y: 0,
    },
    {
      alpha: '70.00',
      appName: 'test',
      groupId: 6970503809,
      height: 0,
      id: 456,
      ts: 6970503809,
      width: 0,
      x: 0,
      y: 0,
    },
    {
      alpha: '13.00',
      appName: 'test1',
      frame: rect,
      groupId: 32238,
      height: 2766,
      id: 717,
      ts: 611455,
      typeValue: 0,
      width: 11223,
      x: 0,
      y: 1,
    },
  ];
  let req = {
    useCache: false,
    context: ctx,
    type: 'dynamicEffectCurve',
    animationRanges: [{ start: 4091445476, end: 4774481414 }],
  };
  TraceRow.range = {
    startNS: 0,
    endNS: 16868000000,
    totalNS: 16868000000,
  };

  let animationRanges = [{ start: 4091445476, end: 4774481414 }];
  frameDynamicRender.frameDynamic(dataList, [], TraceRow.skeleton(), animationRanges, false);

  it('FrameDynamicTest01', function () {
    let [min, max] = frameDynamicRender.getMinAndMaxData(dataList, 'x');
    expect([min, max]).toEqual([0, 1]);
  });
  it('FrameDynamicTest02', function () {
    let currDynamic = {
      alpha: '16.00',
      appName: 'test2',
      frame: rect,
      groupId: 19312108,
      height: 206,
      id: 9654,
      ts: 10242454,
      typeValue: 10,
      width: 1233,
      x: 10,
      y: 1,
    };
    frameDynamicRender.drawDynamicPointYStr(ctx, dataList, rect, 0, 20);
    expect(frameDynamicRender.drawSinglePoint(ctx, currDynamic, TraceRow.skeleton(), 'x', 0, 20)).toBeUndefined();
  });

  it('FrameDynamicTest03', function () {
    let row = TraceRow.skeleton();
    row.dataList = dataList;
    row.dataListCache = dataList;
    frameDynamicRender.renderMainThread(req, row);
    let currDynamicStruct = {
      alpha: '11.00',
      appName: 'test4',
      frame: rect,
      groupId: 2011538,
      height: 2766,
      id: 717,
      ts: 12100454,
      typeValue: 0,
      width: 321,
      x: 10,
      y: 11,
    };

    let preDynamicStruct = {
      alpha: '0.00',
      appName: 'test',
      frame: rect,
      groupId: -1,
      height: 0,
      id: 456,
      ts: 6970503809,
      width: 0,
      x: 0,
      y: 0,
    };
    FrameDynamicStruct.draw(ctx, preDynamicStruct, currDynamicStruct, TraceRow.skeleton(), 'x');
    FrameDynamicStruct.drawSelect(ctx, currDynamicStruct, TraceRow.skeleton());
    expect(FrameDynamicStruct.drawSelectOrHoverArc(ctx, currDynamicStruct)).toBeUndefined();
  });
});
