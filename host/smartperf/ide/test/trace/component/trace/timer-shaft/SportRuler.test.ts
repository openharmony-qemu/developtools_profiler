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
jest.mock('../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../dist/trace/component/SpSystemTrace.js', () => {
  return {
    CurrentSlicesTime:() => {},
  };
});

// @ts-ignore
import { SportRuler } from '../../../../../dist/trace/component/trace/timer-shaft/SportRuler.js';
// @ts-ignore
import { TimerShaftElement } from '../../../../../dist/trace/component/trace/TimerShaftElement.js';
// @ts-ignore
import { Flag } from '../../../../../dist/trace/component/trace/timer-shaft/Flag.js';
// @ts-ignore
import { TraceRow, RangeSelectStruct } from '../../../../../dist/trace/component/trace/base/TraceRow.js';

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
      disconnect: jest.fn(),
      observe: jest.fn(),
      unobserve: jest.fn(),
    }));
declare global {
  interface Window {
    SmartEvent: {
      UI: {
        TraceRowComplete: string; //Triggered after the row component has finished loading data
        MenuTrace: string; //selected menu trace
        RefreshCanvas: string; //selected menu trace
        TimeRange: string; //Set the timeline range
        SliceMark: string; //Set the tag scope
      };
    };
    subscribeOnce(evt: string, fn: (b: any) => void): void;
    clearTraceRowComplete(): void;
    subscribe(evt: string, fn: (b: any) => void): void;
    publish(evt: string, data: any): void;
    unsubscribe(evt: string, fn: (b: any) => void): void;
  }
}

window.SmartEvent = {
  UI: {
    MenuTrace: 'SmartEvent-UI-MenuTrace',
    SliceMark: 'SmartEvent-UI-SliceMark',
    TimeRange: 'SmartEvent-UI-TimeRange',
    RefreshCanvas: 'SmartEvent-UI-RefreshCanvas',
    TraceRowComplete: 'SmartEvent-UI-TraceRowComplete',
  },
};
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
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
    slicesTime: {
      startTime: 12,
      endTime: 22,
      startX: 32,
      endX: 42,
      color: '#000'}
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
        totalNS: 10550,
        startX: 6,
        endX: 109550,
        startNS: 0,
        endNS: 10550,
        xs: numbers,
        xsTxt: ['s', 'f', ''],
      },
    ];
    sportRuler.flagList.xs = jest.fn(() => numbers);
    let flags = new Array<Flag>();
    flags.push({
      x: 10,
      y: 11,
      width: 10,
      height: 13,
      time: 330,
      color: '#3e340c',
      selected: false,
      text: 'w',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    expect(sportRuler.draw()).toBeUndefined();
  });

  it('SportRulerTest09', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 6,
      y: 6,
      width: 15,
      height: 15,
      time: 260,
      color: '#361f1f',
      selected: false,
      text: 'callui',
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
        offsetX: 10005,
        offsetY: 10005,
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
      x: 3,
      y: 3,
      width: 12,
      height: 12,
      time: 0,
      color: '',
      selected: true,
      text: 'abb',
      hidden: false,
      type: '',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'triangle');
  });

  it('SportRulerTest13', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 7,
      y: 7,
      width: 2,
      height: 2,
      time: 102200,
      color: '',
      selected: false,
      text: 'com.ohos.callui',
      hidden: false,
      type: 'triangle',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'triangle');
  });

  it('SportRulerTest14', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 3,
      y: 3,
      width: 67,
      height: 67,
      time: 30,
      color: '#8e4f22',
      selected: false,
      text: 'ohos',
      hidden: true,
      type: 'triangles',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(1000, 'square');
  });

  it('SportRulerTest22', function () {
    let flags = new Array<Flag>();
    flags.push({
      x: 3,
      y: 0,
      width: 33,
      height: 33,
      time: 0,
      color: '#689e3e',
      selected: true,
      text: 'fase',
      hidden: false,
      type: 'square',
    });
    sportRuler.flagList = flags;
    sportRuler.drawTriangle(10000, 'inverted');
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
    document.body.innerHTML = `<sp-application></sp-application>`;
    sportRuler.drawSlicesMarks({startTime: 11, endTime: 22, startX: 33, endX: 44, color: '#fff'});
  });

  it('SportRulerTest25', function () {
    sportRuler.setSlicesMark(null, null);
  });
});
