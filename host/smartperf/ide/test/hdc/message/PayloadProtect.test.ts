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
import { PayloadProtect } from '../../../dist/hdc/message/PayloadProtect.js';

describe('PayloadProtect Test', () => {
  let payloadProtect = new PayloadProtect();
  it('PayloadProtectTest01', function () {
    expect(payloadProtect).not.toBeUndefined();
  });

  it('PayloadProtectTest02', function () {
    expect(payloadProtect.channelId).toBeUndefined();
  });

  it('PayloadProtectTest03', function () {
    payloadProtect.channelId = true;
    expect(payloadProtect.channelId).toBeTruthy();
  });

  it('PayloadProtectTest04', function () {
    expect(payloadProtect.commandFlag).toBeUndefined();
  });

  it('PayloadProtectTest05', function () {
    payloadProtect.commandFlag = true;
    expect(payloadProtect.commandFlag).toBeTruthy();
  });

  it('PayloadProtectTest06', function () {
    expect(payloadProtect.checkSum).toBeUndefined();
  });

  it('PayloadProtectTest07', function () {
    payloadProtect.checkSum = true;
    expect(payloadProtect.checkSum).toBeTruthy();
  });

  it('PayloadProtectTest08', function () {
    expect(payloadProtect.vCode).toBeUndefined();
  });

  it('PayloadProtectTest9', function () {
    payloadProtect.vCode = true;
    expect(payloadProtect.vCode).toBeTruthy();
  });

  it('PayloadProtectTest10', function () {
    expect(payloadProtect.toString()).toBeTruthy();
  });
});
