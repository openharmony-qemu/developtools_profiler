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
import { PayloadHead } from '../../../dist/hdc/message/PayloadHead.js';

describe('PayloadHead Test', () => {
  let payloadHead = new PayloadHead();
  it('PayloadHeadTest01', function () {
    expect(payloadHead).not.toBeUndefined();
  });

  it('PayloadHeadTest02', function () {
    expect(payloadHead.flag).toBeUndefined();
  });

  it('PayloadHeadTest03', function () {
    payloadHead.flag = true;
    expect(payloadHead.flag).toBeTruthy();
  });

  it('PayloadHeadTest04', function () {
    expect(payloadHead.reserve).toBeUndefined();
  });

  it('PayloadHeadTest05', function () {
    payloadHead.reserve = true;
    expect(payloadHead.reserve).toBeTruthy();
  });

  it('PayloadHeadTest06', function () {
    expect(payloadHead.protocolVer).toBeUndefined();
  });

  it('PayloadHeadTest07', function () {
    payloadHead.protocolVer = true;
    expect(payloadHead.protocolVer).toBeTruthy();
  });

  it('PayloadHeadTest08', function () {
    expect(payloadHead.headSize).toBeUndefined();
  });

  it('PayloadHeadTest9', function () {
    payloadHead.headSize = true;
    expect(payloadHead.headSize).toBeTruthy();
  });

  it('PayloadHeadTest10', function () {
    expect(payloadHead.dataSize).toBeUndefined();
  });

  it('PayloadHeadTest11', function () {
    payloadHead.dataSize = true;
    expect(payloadHead.dataSize).toBeTruthy();
  });

  it('PayloadHeadTest12', function () {
    expect(payloadHead.toString()).toBeTruthy();
  });

  it('PayloadHeadTest13', function () {
    expect(payloadHead.getPayloadHeadLength).toBe(undefined);
  });
});
