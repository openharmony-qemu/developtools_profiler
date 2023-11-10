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
import { Cmd } from '../../dist/command/Cmd.js';

describe('Cmd', () => {
  it('CmdTest_01', () => {
    expect(Cmd.execObjDump('', '')).toBeUndefined();
  });

  it('CmdTest_02', () => {
    expect(Cmd.execHdcCmd('')).toBeUndefined();
  });

  it('CmdTest_03', () => {
    expect(Cmd.execFileRecv('', '')).toBeTruthy();
  });

  it('CmdTest_04', () => {
    expect(Cmd.execHdcTraceCmd('', '')).toBeUndefined();
  });

  it('CmdTest_05', () => {
    let params = [
      {
        length: 0,
      },
    ];
    expect(Cmd.formatString('', params)).toBe('');
  });

  it('CmdTest_06', () => {
    expect(Cmd.showSaveFile()).toBeUndefined();
  });

  it('CmdTest_07', () => {
    expect(Cmd.uploadFile()).toBeUndefined();
  });

  it('CmdTest_08', () => {
    expect(Cmd.copyFile('', '')).toBeUndefined();
  });

  it('CmdTest_09', () => {
    expect(Cmd.openFileDialog()).toBeTruthy();
  });

  it('CmdTest_10', () => {
    expect(Cmd.formatString('', [])).toBe('');
  });
});
