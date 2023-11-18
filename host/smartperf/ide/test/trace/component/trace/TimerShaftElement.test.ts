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
jest.mock('../../../../dist/trace/component/trace/base/ColorUtils.js', () => {
  return {};
});

// @ts-ignore
import { TimerShaftElement, ns2s, ns2x } from '../../../../dist/trace/component/trace/TimerShaftElement.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerCommon';

// @ts-ignore
import { EventCenter } from '../../../../dist/trace/component/trace/base/EventCenter.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

declare global {
  interface Window {
    SmartEvent: {
      UI: {
        RefreshCanvas: string; //selected menu trace
        MenuTrace: string; //selected menu trace
        TimeRange: string; //Set the timeline range
        SliceMark: string; //Set the tag scope
        TraceRowComplete: string; //Triggered after the row component has finished loading data
      };
    };
    subscribe(evt: string, fn: (b: any) => void): void;
    unsubscribe(evt: string, fn: (b: any) => void): void;
    subscribeOnce(evt: string, fn: (b: any) => void): void;
    publish(evt: string, data: any): void;
    clearTraceRowComplete(): void;
  }
}

window.SmartEvent = {
  UI: {
    RefreshCanvas: 'SmartEvent-UI-RefreshCanvas',
    SliceMark: 'SmartEvent-UI-SliceMark',
    TraceRowComplete: 'SmartEvent-UI-TraceRowComplete',
    TimeRange: 'SmartEvent-UI-TimeRange',
    MenuTrace: 'SmartEvent-UI-MenuTrace',
  },
};

Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.clearTraceRowComplete = () => EventCenter.clearTraceRowComplete();

describe('TimerShaftElement Test', () => {
  document.body.innerHTML = '<timer-shaft-element id="timerShaftEL"><timer-shaft-element>';
  let timerShaftElement = document.querySelector('#timerShaftEL') as TimerShaftElement;
  timerShaftElement.totalNS = 1000;
  timerShaftElement.startNS = 1000;
  timerShaftElement.endNS = 2000;

  timerShaftElement.cpuUsage = {
    cpu:2,
    ts: 251,
    dur:125
  };

  it('TimerShaftElementTest01', function () {
    expect(timerShaftElement.cpuUsage).toBeUndefined();
  });

  it('TimerShaftElementTest03', function () {
    timerShaftElement.timeRuler = jest.fn(() => false);
    timerShaftElement.rangeRuler.getScale = jest.fn(() => true);
    timerShaftElement.timeRuler.frame = jest.fn(() => {
      return document.createElement('canvas') as HTMLCanvasElement;
    });
    timerShaftElement.rangeRuler.frame = jest.fn(() => {
      return document.createElement('canvas') as HTMLCanvasElement;
    });
    expect(timerShaftElement.connectedCallback()).toBeUndefined();
  });

  it('TimerShaftElementTest05', function () {
    expect(timerShaftElement.disconnectedCallback()).toBeUndefined();
  });

  it('TimerShaftElementTest06', function () {
    expect(timerShaftElement.totalNS).toBe(1000);
  });

  it('TimerShaftElementTest08', function () {
    timerShaftElement.startNS = 'startNS';
    expect(timerShaftElement.startNS).toBe('startNS');
  });

  it('TimerShaftElementTest09', function () {
    timerShaftElement.endNS = 'endNS';
    expect(timerShaftElement.endNS).toBe('endNS');
  });

  it('TimerShaftElementTest14', function () {
    expect(ns2s(1_000_0000)).toBe('10.0 ms');
  });

  it('TimerShaftElementTest16', function () {
    expect(ns2s(1)).toBe('1.0 ns');
  });

  it('TimerShaftElementTest17', function () {
    expect(ns2s(1_000)).toBe('1.0 μs');
  });

  it('TimerShaftElementTest18', function () {
    expect(ns2x(1, 3, 4, 4, { width: 1 })).toBe(0);
  });

  it('TimerShaftElementTest19', function () {
    expect(timerShaftElement.sportRuler).not.toBeUndefined();
  });

  it('TimerShaftElementTest20', function () {
    expect(timerShaftElement.isScaling()).toBeFalsy();
  });

  it('TimerShaftElementTest21', function () {
    timerShaftElement.rangeRuler.setRangeNS = jest.fn(() => true);
    expect(timerShaftElement.setRangeNS()).toBeFalsy();
  });

  it('TimerShaftElementTest22', function () {
    timerShaftElement.rangeRuler.getRange = jest.fn(() => true);
    expect(timerShaftElement.getRange()).toBeTruthy();
  });

  it('TimerShaftElementTest23', function () {
    timerShaftElement.rangeRuler.frame = jest.fn(() => Rect);
    timerShaftElement.rangeRuler.frame.width = jest.fn(() => 1);
    timerShaftElement._sportRuler = jest.fn(() => undefined);
    timerShaftElement._sportRuler.frame = jest.fn(() => Rect);
    timerShaftElement._sportRuler.frame.width = jest.fn(() => 1);
    timerShaftElement.timeRuler = jest.fn(() => undefined);
    timerShaftElement.timeRuler.frame = jest.fn(() => Rect);
    timerShaftElement.timeRuler.frame.width = jest.fn(() => 1);
    timerShaftElement.rangeRuler.fillX = jest.fn(() => true);
    timerShaftElement.render = jest.fn(() => true);
    expect(timerShaftElement.updateWidth()).toBeUndefined();
  });

  it('TimerShaftElementTest24', function () {
    timerShaftElement._sportRuler = jest.fn(() => undefined);
    timerShaftElement._sportRuler.modifyFlagList = jest.fn(() => true);
    expect(timerShaftElement.modifyFlagList()).toBeUndefined();
  });

  it('TimerShaftElementTest25', function () {
    timerShaftElement._sportRuler = jest.fn(() => undefined);
    timerShaftElement._sportRuler.drawTriangle = jest.fn(() => true);
    expect(timerShaftElement.drawTriangle()).toBeTruthy();
  });

  it('TimerShaftElementTest26', function () {
    timerShaftElement._sportRuler = jest.fn(() => undefined);
    timerShaftElement._sportRuler.removeTriangle = jest.fn(() => true);
    expect(timerShaftElement.removeTriangle()).toBeUndefined();
  });

  it('TimerShaftElementTest27', function () {
    timerShaftElement._sportRuler = jest.fn(() => undefined);
    timerShaftElement._sportRuler.setSlicesMark = jest.fn(() => true);
    expect(timerShaftElement.setSlicesMark()).toBe(true);
  });

  it('TimerShaftElementTest28', function () {
    timerShaftElement.rangeRuler.render = jest.fn(() => true);
    expect(timerShaftElement.render()).not.toBeUndefined();
  });

  it('TimerShaftElementTest29', function () {
    expect(ns2x(1, 3, 0, 4, { width: 1 })).toBe(0);
  });

  it('TimerShaftElementTest30', function () {
    timerShaftElement.rangeRuler.cpuUsage = jest.fn(() => true);
    expect(timerShaftElement.cpuUsage).toBe(undefined);
  });

  it('TimerShaftElementTest31', function () {
    timerShaftElement.timeRuler = jest.fn(() => true);
    expect(timerShaftElement.totalNS).toBe(1000);
  });

  it('TimerShaftElementTest32', function () {
    expect(timerShaftElement.totalNS).toBe(1000);
  });

  it('TimerShaftElementTest33', function () {
    timerShaftElement.timeTotalEL = jest.fn(() => true);
    expect(timerShaftElement.totalNS).toBe(1000);
  });

  it('TimerShaftElementTest35', function () {
    timerShaftElement.rangeRuler.cancelPressFrame = jest.fn(() => undefined);
    expect(timerShaftElement.cancelPressFrame()).toBeUndefined();
  });

  it('TimerShaftElementTest36', function () {
    timerShaftElement.rangeRuler.cancelUpFrame = jest.fn(() => undefined);
    expect(timerShaftElement.cancelUpFrame()).toBeUndefined();
  });
});
