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

import { EventCenter } from './EventCenter.js';

declare global {
  interface Number {
    n2x(): number;
  }

  interface Array<T> {
    isEmpty(): boolean;

    isNotEmpty(): boolean;
  }

  interface HTMLElement {
    containPoint(
      ev: MouseEvent,
      cut?: {
        left?: number;
        right?: number;
        top?: number;
        bottom?: number;
      }
    ): boolean;
  }

  interface Window {
    SmartEvent: {
      UI: {
        MenuTrace: string; //selected menu trace
        RefreshCanvas: string; //selected menu trace
        SliceMark: string; //Set the tag scope
        TimeRange: string; //Set the timeline range
        TraceRowComplete: string; //Triggered after the row component has finished loading data
        KeyboardEnable: string; // SystemTrace Keyboard enable
        MouseEventEnable: string; // Mouse Event Keyboard enable
        UploadSOFile: string; // Upload so file
        Loading: string; // Upload so file
        Error: string; // load error
        CheckALL: string; // Check all child chart
      };
    };

    subscribe(evt: string, fn: (b: any) => void): void;

    subscribeOnce(evt: string, fn: (b: any) => void): void;

    unsubscribe(evt: string, fn: (b: any) => void): void;

    publish(evt: string, data: any): void;

    clearTraceRowComplete(): void;
  }
}

Number.prototype.n2x = function (): number {
  return Number(this);
};

Array.prototype.isEmpty = function <T>(): boolean {
  return this == null || this == undefined || this.length == 0;
};
Array.prototype.isNotEmpty = function <T>(): boolean {
  return this != null && this != undefined && this.length > 0;
};

HTMLElement.prototype.containPoint = function (ev, cut) {
  let rect = this.getBoundingClientRect();
  return (
    ev.pageX >= rect.left + (cut?.left ?? 0) &&
    ev.pageX <= rect.right - (cut?.right ?? 0) &&
    ev.pageY >= rect.top + (cut?.top ?? 0) &&
    ev.pageY <= rect.bottom - (cut?.bottom ?? 0)
  );
};

window.SmartEvent = {
  UI: {
    MenuTrace: 'SmartEvent-UI-MenuTrace',
    RefreshCanvas: 'SmartEvent-UI-RefreshCanvas',
    SliceMark: 'SmartEvent-UI-SliceMark',
    TimeRange: 'SmartEvent-UI-TimeRange',
    TraceRowComplete: 'SmartEvent-UI-TraceRowComplete',
    KeyboardEnable: 'SmartEvent-UI-StopWASD',
    MouseEventEnable: 'SmartEvent-UI-StopMouseEvent',
    UploadSOFile: 'SmartEvent-UI-UploadSoFile',
    Loading: 'SmartEvent-UI-Loading',
    Error: 'SmartEvent-UI-Error',
    CheckALL: 'SmartEvent-UI-CheckALL',
  },
};
Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.clearTraceRowComplete = () => EventCenter.clearTraceRowComplete();
export {};
