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
import { TransferConfig } from '../../../dist/hdc/message/TransferConfig.js';

describe('TransferConfig Test', () => {
  let transferConfig = new TransferConfig();
  it('TransferConfigTest01', function () {
    expect(transferConfig).not.toBeUndefined();
  });

  it('TransferConfigTest02', function () {
    expect(transferConfig.fileSize).toBeUndefined();
  });

  it('TransferConfigTest03', function () {
    transferConfig.fileSize = true;
    expect(transferConfig.fileSize).toBeTruthy();
  });

  it('TransferConfigTest04', function () {
    expect(transferConfig.atime).toBeUndefined();
  });

  it('TransferConfigTest05', function () {
    transferConfig.atime = true;
    expect(transferConfig.atime).toBeTruthy();
  });

  it('TransferConfigTest06', function () {
    expect(transferConfig.mtime).toBeUndefined();
  });

  it('TransferConfigTest07', function () {
    transferConfig.mtime = true;
    expect(transferConfig.mtime).toBeTruthy();
  });

  it('TransferConfigTest08', function () {
    expect(transferConfig.options).toBeFalsy();
  });

  it('TransferConfigTest09', function () {
    transferConfig.options = true;
    expect(transferConfig.options).toBeTruthy();
  });

  it('TransferConfigTest10', function () {
    expect(transferConfig.path).toBeUndefined();
  });

  it('TransferConfigTest11', function () {
    transferConfig.path = true;
    expect(transferConfig.path).toBeTruthy();
  });

  it('TransferConfigTest12', function () {
    expect(transferConfig.optionalName).toBeUndefined();
  });

  it('TransferConfigTest13', function () {
    transferConfig.optionalName = true;
    expect(transferConfig.optionalName).toBeTruthy();
  });

  it('TransferConfigTest14', function () {
    expect(transferConfig.updateIfNew).toBeUndefined();
  });

  it('TransferConfigTest15', function () {
    transferConfig.updateIfNew = true;
    expect(transferConfig.updateIfNew).toBeTruthy();
  });

  it('TransferConfigTest16', function () {
    expect(transferConfig.compressType).toBeUndefined();
  });

  it('TransferConfigTest17', function () {
    transferConfig.compressType = true;
    expect(transferConfig.compressType).toBeTruthy();
  });

  it('TransferConfigTest18', function () {
    expect(transferConfig.holdTimestamp).toBeUndefined();
  });

  it('TransferConfigTest19', function () {
    transferConfig.holdTimestamp = true;
    expect(transferConfig.holdTimestamp).toBeTruthy();
  });

  it('TransferConfigTest20', function () {
    expect(transferConfig.functionName).toBeUndefined();
  });

  it('TransferConfigTest21', function () {
    transferConfig.functionName = true;
    expect(transferConfig.functionName).toBeTruthy();
  });

  it('TransferConfigTest22', function () {
    expect(transferConfig.clientCwd).toBeUndefined();
  });

  it('TransferConfigTest23', function () {
    transferConfig.clientCwd = true;
    expect(transferConfig.clientCwd).toBeTruthy();
  });

  it('TransferConfigTest24', function () {
    expect(transferConfig.reserve1).toBeUndefined();
  });

  it('TransferConfigTest25', function () {
    transferConfig.reserve1 = true;
    expect(transferConfig.reserve1).toBeTruthy();
  });

  it('TransferConfigTest26', function () {
    expect(transferConfig.reserve2).toBeUndefined();
  });

  it('TransferConfigTest27', function () {
    transferConfig.reserve2 = true;
    expect(transferConfig.reserve2).toBeTruthy();
  });

  it('TransferConfigTest28', function () {
    expect(transferConfig.toString()).not.toBeUndefined();
  });
});
