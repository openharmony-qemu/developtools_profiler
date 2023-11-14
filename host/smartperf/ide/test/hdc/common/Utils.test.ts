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
import { Utils } from '../../../dist/hdc/common/Utils.js';

describe('UtilsTest', () => {
  it('UtilsTest_getLocalId_01', () => {
    expect(Utils.getLocalId()).toBeTruthy();
  });

  it('UtilsTest_getLocalId_02', () => {
    Utils.localId = 4294967295;
    expect(Utils.getLocalId()).toBe(1);
  });

  it('UtilsTest_getSessionId_01', () => {
    expect(Utils.getSessionId()).toBeTruthy();
  });

  it('UtilsTest_formatCommand_01', () => {
    expect(
      Utils.formatCommand(
        'hdc_std shell killall hiprofilerd hiprofiler_plugins native_daemon hiperf' + ' hiprofiler_cmd'
      )
    ).toEqual({
      bJumpDo: false,
      cmdFlag: 1001,
      parameters: 'killall hiprofilerd hiprofiler_plugins native_daemon hiperf hiprofiler_cmd',
    });
  });

  it('UtilsTest_formatCommand_02', () => {
    expect(Utils.formatCommand('abc')).toEqual({
      bJumpDo: true,
      cmdFlag: -1,
      parameters: '',
    });
  });

  it('UtilsTest_formatCommand_03', () => {
    expect(Utils.formatCommand('hdc')).toEqual({
      bJumpDo: true,
      cmdFlag: -1,
      parameters: '',
    });
  });

  it('UtilsTest_numToHexString_01', () => {
    expect(Utils.numToHexString(1)).toBe('0x1');
  });

  it('UtilsTest_numToHexString_02', () => {
    expect(Utils.numToHexString(-1)).toBe('0xffffffff');
  });

  it('UtilsTest_numToHexString_03', () => {
    expect(Utils.numToHexString(undefined)).toBe('0x0');
  });
});
