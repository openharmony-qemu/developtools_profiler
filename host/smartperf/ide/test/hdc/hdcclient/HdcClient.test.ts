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
import { HdcClient } from '../../../dist/hdc/hdcclient/HdcClient.js';

describe('HdcClient Test', () => {
  it('HdcClientTest01', function () {
    let hdcClient = new HdcClient();
    expect(hdcClient.constructor()).toBeUndefined();
  });
  it('HdcClientTest02', function () {
    let hdcClient = new HdcClient();
    expect(hdcClient.bindStream()).toBeUndefined();
  });
  it('HdcClientTest03', function () {
    let hdcClient = new HdcClient();
    expect(hdcClient.bindStopStream()).toBeUndefined();
  });
  it('HdcClientTest04', function () {
    let hdcClient = new HdcClient();
    expect(hdcClient.unbindStream()).toBeTruthy();
  });
  it('HdcClientTest05', function () {
    let hdcClient = new HdcClient();
    expect(hdcClient.unbindStopStream()).toBeTruthy();
  });

  it('HdcClientTest06', async () => {
    let hdcClient = new HdcClient();
    await expect(hdcClient.connectDevice()).rejects.not.toBeUndefined();
  });

  it('HdcClientTest07', async () => {
    let hdcClient = new HdcClient();
    await expect(hdcClient.disconnect()).not;
  });
  it('HdcClientTest08', function () {
    let hdcClient = new HdcClient();
    let data = {
      getChannelId: jest.fn(() => -1),
    };
    expect(hdcClient.createDataMessage(data)).toBeUndefined();
  });
});
