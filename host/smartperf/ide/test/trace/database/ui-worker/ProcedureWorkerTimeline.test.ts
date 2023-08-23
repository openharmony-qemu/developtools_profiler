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
  RangeRuler,
  SportRuler,
  timeline,
  TimelineRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerTimeline.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe(' ProcedureWorkerTimelineTest', () => {
  it('timelineTest', () => {
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
    timeline(canvas, ctx, 1, 100254, 100254, rect, null, null, null, null, null, null, 0, 0, (e: any) => {});
  });

    it('SportRulerTest01', () => {
        const canvas = document.createElement('canvas');
        canvas.width = 1;
        canvas.height = 1;
        const ctx = canvas.getContext('2d');
        let rect = new Rect(0, 10, 10, 10);
        let sportRuler = new SportRuler(canvas, ctx, rect);
        sportRuler.modifyFlagList('amend');
        sportRuler.modifyFlagList('remove');
        sportRuler.drawTheFlag(0, '#999999', false, '');
        sportRuler.randomRgbColor();
        sportRuler.mouseMove(new MouseEvent(''));
        sportRuler.mouseUp(new MouseEvent(''));
        sportRuler.onFlagRangeEvent('1', 2);
    });

    it('SportRulerTest02', () => {
        const canvas = document.createElement('canvas');
        canvas.width = 1;
        canvas.height = 1;
        const ctx = canvas.getContext('2d');
        let rect = new Rect(0, 10, 10, 10);
        let rangeRuler = new RangeRuler(
            canvas,
            ctx,
            rect,
            {
                startX: 0,
                endX: rect.width,
                startNS: 0,
                endNS: 111,
                totalNS: 111,
                xs: [],
                xsTxt: [],
            },
            (a: any) => {}
        );
        rangeRuler.draw();
        rangeRuler.drawCpuUsage();
        rangeRuler.mouseDown({ offsetX: 1, offsetY: 1 });
        rangeRuler.mouseUp(new MouseEvent(''));
        rangeRuler.mouseMove(new MouseEvent(''));
        rangeRuler.mouseOut(new MouseEvent(''));
        rangeRuler.range.startNS = -2;
        rangeRuler.range.endNS = -2;
        rangeRuler.range.totalNS = -7;
        rangeRuler.fillX();
        rangeRuler.keyPress(new KeyboardEvent(''));
        rangeRuler.pressFrameId != -1;
        rangeRuler.keyUp(new KeyboardEvent(''));
        rangeRuler.keyUp({ key: 'w' });
        rangeRuler.keyUp({ key: 's' });
        rangeRuler.keyUp({ key: 'a' });
        rangeRuler.keyUp({ key: 'd' });
    });

  it('SportRulerTest03', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let rect = new Rect(0, 10, 10, 10);
    let rangeRuler = new RangeRuler(
      canvas,
      ctx,
      rect,
      {
        startX: 0,
        endX: rect.width,
        startNS: 0,
        endNS: 111,
        totalNS: 111,
        xs: [],
        xsTxt: [],
      },
      (a: any) => {}
    );
    rangeRuler.cpuUsage = true;
    expect(rangeRuler.cpuUsage).toBeUndefined();
  });
});
