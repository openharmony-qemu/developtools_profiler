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
import { SessionHandShake } from '../../../dist/hdc/message/SessionHandShake.js';

describe('SessionHandShake Test', () => {
  let sessionHandShake = new SessionHandShake();
  it('SessionHandShakeTest01', function () {
    expect(sessionHandShake).not.toBeUndefined();
  });

  it('SessionHandShakeTest02', function () {
    expect(sessionHandShake.banner).toBeUndefined();
  });

  it('SessionHandShakeTest03', function () {
    sessionHandShake.banner = true;
    expect(sessionHandShake.banner).toBeTruthy();
  });

  it('SessionHandShakeTest04', function () {
    expect(sessionHandShake.authType).toBeUndefined();
  });

  it('SessionHandShakeTest05', function () {
    sessionHandShake.authType = true;
    expect(sessionHandShake.authType).toBeTruthy();
  });

  it('SessionHandShakeTest06', function () {
    expect(sessionHandShake.sessionId).toBeUndefined();
  });

  it('SessionHandShakeTest07', function () {
    sessionHandShake.sessionId = true;
    expect(sessionHandShake.sessionId).toBeTruthy();
  });

  it('SessionHandShakeTest08', function () {
    expect(sessionHandShake.connectKey).toBeUndefined();
  });

  it('SessionHandShakeTest9', function () {
    sessionHandShake.connectKey = true;
    expect(sessionHandShake.connectKey).toBeTruthy();
  });

  it('SessionHandShakeTest10', function () {
    expect(sessionHandShake.buf).toBeUndefined();
  });

  it('SessionHandShakeTest11', function () {
    sessionHandShake.buf = true;
    expect(sessionHandShake.buf).toBeTruthy();
  });

  it('SessionHandShakeTest12', function () {
    expect(sessionHandShake.version).toBe('');
  });

  it('SessionHandShakeTest13', function () {
    sessionHandShake.version = true;
    expect(sessionHandShake.version).toBeTruthy();
  });

  it('SessionHandShakeTest14', function () {
    expect(sessionHandShake.toString()).toBeTruthy();
  });
});
