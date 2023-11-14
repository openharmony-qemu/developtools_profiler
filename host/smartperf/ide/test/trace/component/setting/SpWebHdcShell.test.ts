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
import { DataMessage } from '../../../../dist/hdc/message/DataMessage.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

// @ts-ignore
import { SpWebHdcShell } from '../../../../dist/trace/component/setting/SpWebHdcShell.js';
// @ts-ignore
import { EventCenter } from '../../../../dist/trace/component/trace/base/EventCenter.js';
// @ts-ignore
import { USBHead } from '../../../../dist/hdc/message/USBHead.js';

declare global {
  interface Window {
    SmartEvent: {
      UI: {
        DeviceConnect: string;
        DeviceDisConnect: string;
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
    DeviceConnect: 'SmartEvent-DEVICE_CONNECT',
    DeviceDisConnect: 'SmartEvent-DEVICE_DISCONNECT',
  },
};

Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.clearTraceRowComplete = () => EventCenter.clearTraceRowComplete();
describe('SpWebHdcShell Test', () => {
  let spWebHdcShell = new SpWebHdcShell();
  it('SpWebHdcShell Test01', function () {
    expect(spWebHdcShell.initElements()).toBeUndefined();
  });

  it('SpWebHdcShell Test02', function () {
    expect(spWebHdcShell.initHtml()).not.toBeNull();
  });

  it('SpWebHdcShell Test03', function () {
    let arrayBufferA = new Uint8Array(1);
    arrayBufferA.set([1]);
    let arrayBufferB = new Uint8Array(1);
    arrayBufferB.set([1]);
    expect(spWebHdcShell.arrayBufferCompare(arrayBufferA, arrayBufferB)).toBeTruthy();
  });

  it('SpWebHdcShell Test05', function () {
    let arrayBufferA = new Uint8Array(1);
    arrayBufferA.set([2]);
    let arrayBufferB = new Uint8Array(1);
    arrayBufferB.set([1]);
    expect(spWebHdcShell.arrayBufferCompare(arrayBufferA, arrayBufferB)).toBeFalsy();
  });

  it('SpWebHdcShell Test06', function () {
    let dataHead = new USBHead([85, 66], 1, 77777, 0);
    let dataMessage = new DataMessage(dataHead);
    expect(spWebHdcShell.handleHdcRecvData(dataMessage)).toBeUndefined();
  });

  it('SpWebHdcShell Test07', function () {
    let dataHead = new USBHead([85, 66], 1, 77777, 21);
    let arrayBuffer = new Uint8Array(21);
    arrayBuffer.set([72,87,0,0,2,0,8,0,0,0,2,8,2,16,10,24,0,32,9,35,32]);
    let body = new DataView(arrayBuffer.buffer);
    let dataMessage = new DataMessage(dataHead, body);
    expect(spWebHdcShell.handleHdcRecvData(dataMessage)).toBeUndefined();
  });
  it('SpWebHdcShell Test011', function () {
    expect(spWebHdcShell.clear()).toBeUndefined();
  });
  it('SpWebHdcShell Test012', function () {
    let shellStr = '111111111111111111111111111111';
    let res = spWebHdcShell.singleLineToMultiLine(shellStr,1,10)
    expect(res).toEqual(["1111111111","1111111111","1111111111"])
  });
  it('SpWebHdcShell Test013', function () {
    expect(spWebHdcShell.forwardSelected(0,1120,0,1111)).toBeUndefined();
  });
});
