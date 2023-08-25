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
import { Serialize } from '../../../dist/hdc/common/Serialize.js';

describe('Serialize Test', () => {
  it('Serialize Test01', function () {
    let banne = {
      banner: 1,
      authType: 1,
      sessionId: 1,
      connectKey: 1,
      buf: '',
    };
    expect(Serialize.serializeSessionHandShake(banne)).not.toBeUndefined();
  });
  it('Serialize Test02', function () {
    let payloadProtect = {
      channelId: 1,
      commandFlag: 1,
      checkSum: 1,
      vCode: 1,
    };
    expect(Serialize.serializePayloadProtect(payloadProtect)).not.toBeUndefined();
  });
  it('Serialize Test03', function () {
    let transferConfig = {
      fileSize: 1,
      atime: 1,
      mtime: 1,
      options: 1,
      path: 1,
      optionalName: 1,
      updateIfNew: 1,
      compressType: 1,
      holdTimestamp: 1,
      functionName: 1,
      clientCwd: 1,
      reserve1: 1,
      reserve2: 1,
    };
    expect(Serialize.serializeTransferConfig(transferConfig)).not.toBeUndefined();
  });
  it('Serialize Test04', function () {
    let transferPayload = {
      index: 1,
      compressType: 1,
      compressSize: 1,
      uncompressSize: 1,
    };
    expect(Serialize.serializeTransferPayload(transferPayload)).not.toBeUndefined();
  });
  it('Serialize Test06', function () {
    let data = {
      buffer: 1,
    };
    // @ts-ignore
    let uint8Array = new Uint8Array(data);
    let dataBuffer = uint8Array.buffer;
    expect(Serialize.parseTransferConfig(data)).not.toBeUndefined();
  });
  it('Serialize Test05', function () {
    let tagKey = 1;
    expect(Serialize.readTagWireType(tagKey)).not.toBeUndefined();
  });
  it('Serialize Test07', function () {
    let data = {
      buffer: 1,
    };
    // @ts-ignore
    let uint8Array = new Uint8Array(data);
    let dataBuffer = uint8Array.buffer;
    expect(Serialize.parsePayloadProtect(data)).not.toBeUndefined();
  });

  it('Serialize Test08', function () {
    expect(Serialize.writeVarIntU64(100_000_000)).not.toBeUndefined();
  });

  it('Serialize Test09', function () {
    let data = {
      buffer: 1,
    };
    // @ts-ignore
    let uint8Array = new Uint8Array(data);
    expect(Serialize.parseString(uint8Array, 1)).not.toBeUndefined();
  });

  it('Serialize Test10', function () {
    let data = {
      buffer: 1,
    };
    // @ts-ignore
    let uint8Array = new Uint8Array(data);
    expect(Serialize.parseHandshake(uint8Array)).toEqual({
      _authType: -1,
      _banner: '',
      _buf: '',
      _connectKey: '',
      _sessionId: -1,
      _version: '',
    });
  });

  it('Serialize Test11', function () {
    expect(Serialize.writeVarIntU32(100_000_000)).not.toBeUndefined();
  });
});
