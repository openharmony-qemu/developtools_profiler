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
import { DataMessage } from '../../../dist/hdc/message/DataMessage.js';
import { TextEncoder } from 'util';

describe('DataMessage Test', () => {
  let dataMessage = new DataMessage();
  dataMessage.body = true;
  it('DataMessageTest01', function () {
    expect(dataMessage).not.toBeUndefined();
  });

  it('DataMessageTest02', function () {
    expect(dataMessage.usbHead).toBeUndefined();
  });

  it('DataMessageTest03', function () {
    dataMessage.usbHead = true;
    expect(dataMessage.usbHead).toBeTruthy();
  });

  it('DataMessageTest04', function () {
    expect(dataMessage.channelId).toBe(-1);
  });

  it('DataMessageTest05', function () {
    dataMessage.channelId = true;
    expect(dataMessage.channelId).toBeTruthy();
  });

  it('DataMessageTest06', function () {
    expect(dataMessage.result).toBe('');
  });

  it('DataMessageTest07', function () {
    dataMessage.result = true;
    expect(dataMessage.result).toBeTruthy();
  });

  it('DataMessageTest08', function () {
    expect(dataMessage.channelClose).toBeFalsy();
  });

  it('DataMessageTest09', function () {
    dataMessage.channelClose = true;
    expect(dataMessage.channelClose).toBeTruthy();
  });

  it('DataMessageTest10', function () {
    expect(dataMessage.commandFlag).toBe(-1);
  });

  it('DataMessageTest11', function () {
    dataMessage.commandFlag = true;
    expect(dataMessage.commandFlag).toBeTruthy();
  });

  it('DataMessageTest12', function () {
    expect(dataMessage.resArrayBuffer).toBeUndefined();
  });

  it('DataMessageTest13', function () {
    dataMessage.resArrayBuffer = true;
    expect(dataMessage.resArrayBuffer).toBeTruthy();
  });

  it('DataMessageTest14', function () {
    expect(dataMessage.toString()).not.toBeUndefined();
  });

  it('DataMessageTest15', function () {
    expect(dataMessage.getChannelId()).not.toBeUndefined();
  });

  it('DataMessageTest16', function () {
    expect(dataMessage.getData()).not.toBeUndefined();
  });

  it('DataMessageTest17', function () {
    let end = new TextEncoder();

    dataMessage.resArrayBuffer = end.encode('111');
    expect(dataMessage.getDataToString()).toBe('111');
  });
});
