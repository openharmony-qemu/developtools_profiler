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
import { EventCenter } from '../../../../../dist/trace/component/trace/base/EventCenter.js';

jest.mock('../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

// @ts-ignore
import { SportRuler } from '../../../../../dist/trace/component/trace/timer-shaft/SportRuler.js';
// @ts-ignore
import { TimerShaftElement } from '../../../../../dist/trace/component/trace/TimerShaftElement.js';
// @ts-ignore
import { Flag } from '../../../../../dist/trace/component/trace/timer-shaft/Flag.js';
// @ts-ignore
import { TraceRow, RangeSelectStruct } from '../../../../../dist/trace/component/trace/base/TraceRow.js';

declare global {
  interface Window {
    SmartEvent: {
      UI: {
        MenuTrace: string; //selected menu trace
        RefreshCanvas: string; //selected menu trace
        SliceMark: string; //Set the tag scope
        TimeRange: string; //Set the timeline range
        TraceRowComplete: string; //Triggered after the row component has finished loading data
      };
    };

    subscribe(evt: string, fn: (b: any) => void): void;

    subscribeOnce(evt: string, fn: (b: any) => void): void;

    unsubscribe(evt: string, fn: (b: any) => void): void;

    publish(evt: string, data: any): void;

    clearTraceRowComplete(): void;
  }
}

window.SmartEvent = {
  UI: {
    MenuTrace: 'SmartEvent-UI-MenuTrace',
    RefreshCanvas: 'SmartEvent-UI-RefreshCanvas',
    SliceMark: 'SmartEvent-UI-SliceMark',
    TimeRange: 'SmartEvent-UI-TimeRange',
    TraceRowComplete: 'SmartEvent-UI-TraceRowComplete',
  },
};

Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.clearTraceRowComplete = () => EventCenter.clearTraceRowComplete();

describe('SportRuler Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

  document.body.innerHTML = '<timer-shaft-element id="timerShaftEL"><timer-shaft-element>';

  let timerShaftElement = document.querySelector('#timerShaftEL') as TimerShaftElement;

  let sportRuler = new SportRuler(
    timerShaftElement,
    {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    () => {},
    () => {}
  );
  sportRuler.c = ctx;
  sportRuler.range = {
    totalNS: 20,
    startX: 0,
    endX: 10,
    startNS: 10,
    endNS: 20,
    xs: [],
    xsTxt: [],
  };

  it('SportRulerTest04', function () {
    expect(
      sportRuler.mouseMove({
        offsetY: 20,
        offsetX: 20,
      })
    ).toBeUndefined();
  });

  it('SportRulerTest05', function () {
    let ranges = sportRuler.range;
    expect(ranges.endNS).toBe(20);
  });

  it('SportRulerTest07', function () {
    sportRuler.flagList.splice = jest.fn(() => true);
    expect(sportRuler.modifyFlagList('remove')).toBeUndefined();
  });

  it('SportRulerTest08', function () {
    let numbers = Array<number>();
    numbers.push(12);
    numbers.push(56);
    sportRuler.flagList = [
      {
        totalNS: 10000,
        startX: 0,
        endX: 1000,
        startNS: 0,
        endNS: 10000,
        xs: numbers,
        xsTxt: ['s', 'f'],
      },
    ];
    sportRuler.flagList.xs = jest.fn(() => numbers);
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 20,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    expect(sportRuler.draw()).toBeUndefined();
  });

  it('SportRulerTest09', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 20,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    sportRuler.edgeDetection = jest.fn(() => true);

    expect(sportRuler.mouseUp({ offsetX: 20 })).toBeUndefined();
  });

  it('SportRulerTest10', function () {
    sportRuler.draw = jest.fn(() => true);
    expect(
      sportRuler.mouseMove({
        offsetX: 10000,
        offsetY: 10000,
      })
    ).toBeUndefined();
  });

  it('SportRulerTest11', function () {
    let range = sportRuler.range;
    expect(sportRuler.range.endNS).toBe(20);
  });

  it('SportRulerTest12', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 0,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'triangle');
  });

  it('SportRulerTest13', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 1000,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: 'triangle',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'triangle');
  });

  it('SportRulerTest14', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 0,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: 'triangle',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'square');
  });

  it('SportRulerTest22', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 0,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: 'triangle',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'inverted');
  });

  it('SportRulerTest17', function () {
    sportRuler.removeTriangle('inverted');
  });

  it('SportRulerTest18', function () {
    sportRuler.flagList.findIndex = jest.fn(() => 0);
    sportRuler.removeTriangle('square');
  });

  it('SportRulerTest19', function () {
    sportRuler.drawInvertedTriangle(100, '#000000');
  });

  it('SportRulerTest20', function () {
    sportRuler.drawFlag(100, '#000000', false, 'text', '');
  });

  it('SportRulerTest23', function () {
    sportRuler.drawFlag(100, '#000000', false, 'text', 'triangle');
  });

  it('SportRulerTest21', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 0,
      y: 0,
      width: 0,
      height: 0,
      time: 20,
      color: '',
      selected: false,
      text: '',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    sportRuler.flagList.find = jest.fn(() => false);
    expect(sportRuler.mouseUp({ offsetX: 20 })).toBeUndefined();
  });

  it('SportRulerTest24', function () {
    sportRuler.drawSlicesMark(null, null);
  });

  it('SportRulerTest25', function () {
    sportRuler.setSlicesMark(null, null);
  });
});
