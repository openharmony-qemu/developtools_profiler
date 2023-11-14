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
import { SpRecordPerf } from '../../../../dist/trace/component/setting/SpRecordPerf.js';

describe('SpRecordPerf Test', () => {
  let spRecordPerf = new SpRecordPerf();
  it('SpRecordPerfTest01', function () {
    expect(spRecordPerf).not.toBeUndefined();
  });

  it('SpRecordPerfTest02', function () {
    expect(spRecordPerf.show).toBeFalsy();
  });

  it('SpRecordPerfTest03', function () {
    spRecordPerf.show = true;
    expect(spRecordPerf.show).toBeTruthy();
  });

  it('SpRecordPerfTest08', function () {
    spRecordPerf.show = false;
    expect(spRecordPerf.show).toBeFalsy();
  });

  it('SpRecordPerfTest09', function () {
    expect(spRecordPerf.startSamp).toBeFalsy();
  });

  it('SpRecordPerfTest10', function () {
    spRecordPerf.startSamp = true;
    expect(spRecordPerf.startSamp).toBeTruthy();
  });

  it('SpRecordPerfTest11', function () {
    spRecordPerf.startSamp = false;
    expect(spRecordPerf.startSamp).toBeFalsy();
  });

  it('SpRecordPerfTest05', function () {
    expect(spRecordPerf.unDisable()).toBeUndefined();
  });

  it('SpRecordPerfTest06', function () {
    expect(spRecordPerf.startSamp).toBeFalsy();
  });

  it('SpRecordPerfTest07', function () {
    spRecordPerf.startSamp = true;
    expect(spRecordPerf.startSamp).toBeTruthy();
  });
  it('SpRecordPerfTest011', function () {
    expect(spRecordPerf.getPerfConfig()).toBeTruthy();
  });
  it('SpRecordPerfTest012', function () {
    expect(spRecordPerf.parseEvent('adfger')).toBeTruthy();
  });
});
