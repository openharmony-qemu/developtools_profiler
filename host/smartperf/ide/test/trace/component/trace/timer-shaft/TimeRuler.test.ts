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
import { TimeRuler } from '../../../../../dist/trace/component/trace/timer-shaft/TimeRuler.js';
// @ts-ignore
import { TimerShaftElement } from '../../../../../dist/trace/component/trace/TimerShaftElement.js';

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

describe('TimeRuler Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');
  document.body.innerHTML = '<timer-shaft-element id="timerShaftEL"><timer-shaft-element>';

  let timerShaftElement = document.querySelector('#timerShaftEL') as TimerShaftElement;

  let timeRuler = new TimeRuler(
    timerShaftElement,
    {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    10000000000
  );

  timeRuler.c = ctx;

  it('TimeRulerTest01', function () {
    expect(timeRuler.draw()).toBeUndefined();
  });
});
