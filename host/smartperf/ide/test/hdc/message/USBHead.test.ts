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
import { USBHead } from '../../../dist/hdc/message/USBHead.js';

describe('USBHead Test', () => {
  let usbHead = new USBHead();
  it('USBHeadTest01', function () {
    expect(usbHead).not.toBeUndefined();
  });

  it('USBHeadTest02', function () {
    expect(usbHead.flag).toBeUndefined();
  });

  it('USBHeadTest03', function () {
    usbHead.flag = true;
    expect(usbHead.flag).toBeTruthy();
  });

  it('USBHeadTest04', function () {
    expect(usbHead.option).toBeUndefined();
  });

  it('USBHeadTest05', function () {
    usbHead.option = true;
    expect(usbHead.option).toBeTruthy();
  });

  it('USBHeadTest06', function () {
    expect(usbHead.sessionId).toBeUndefined();
  });

  it('USBHeadTest07', function () {
    usbHead.sessionId = true;
    expect(usbHead.sessionId).toBeTruthy();
  });

  it('USBHeadTest08', function () {
    expect(usbHead.dataSize).toBeFalsy();
  });

  it('USBHeadTest09', function () {
    usbHead.dataSize = true;
    expect(usbHead.dataSize).toBeTruthy();
  });

  it('USBHeadTest10', function () {
    expect(usbHead.toString()).not.toBeUndefined();
  });

  it('USBHeadTest11', function () {
    usbHead.parseHeadData = jest.fn(() => undefined);
    expect(usbHead.parseHeadData()).toBeUndefined();
  });
});
