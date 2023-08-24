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
import { TransferPayload } from '../../../dist/hdc/message/TransferPayload.js';

describe('TransferPayload Test', () => {
  let transferPayload = new TransferPayload();
  it('TransferPayloadTest01', function () {
    expect(transferPayload).not.toBeUndefined();
  });

  it('TransferPayloadTest02', function () {
    expect(transferPayload.index).toBeUndefined();
  });

  it('TransferPayloadTest03', function () {
    transferPayload.index = true;
    expect(transferPayload.index).toBeTruthy();
  });

  it('TransferPayloadTest04', function () {
    expect(transferPayload.compressType).toBeUndefined();
  });

  it('TransferPayloadTest05', function () {
    transferPayload.compressType = true;
    expect(transferPayload.compressType).toBeTruthy();
  });

  it('TransferPayloadTest06', function () {
    expect(transferPayload.compressSize).toBeUndefined();
  });

  it('TransferPayloadTest07', function () {
    transferPayload.compressSize = true;
    expect(transferPayload.compressSize).toBeTruthy();
  });

  it('TransferPayloadTest08', function () {
    expect(transferPayload.uncompressSize).toBeFalsy();
  });

  it('TransferPayloadTest09', function () {
    transferPayload.uncompressSize = true;
    expect(transferPayload.uncompressSize).toBeTruthy();
  });

  it('TransferPayloadTest10', function () {
    expect(transferPayload.toString()).not.toBeUndefined();
  });
});
