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
import { FormatCommand } from '../../../dist/hdc/hdcclient/FormatCommand.js';
// @ts-ignore
import { CmdConstant } from '../../../dist/command/CmdConstant.js';

describe('FormatCommandTest', () => {
  it('FormatCommandTest_FormatCommand_01', () => {
    expect(FormatCommand.string2FormatCommand(CmdConstant.CMD_TRACE_FILE_SIZE)).toEqual({
      bJumpDo: true,
      cmdFlag: -1,
      parameters: '',
    });
  });

  it('FormatCommandTest_FormatCommand_02', () => {
    expect(FormatCommand.string2FormatCommand('shell ps')).toEqual({
      bJumpDo: false,
      cmdFlag: 1001,
      parameters: 'ps',
    });
  });

  it('FormatCommandTest_FormatCommand_03', () => {
    expect(FormatCommand.string2FormatCommand('shell')).toEqual({
      bJumpDo: false,
      cmdFlag: 2000,
      parameters: '',
    });
  });

  it('FormatCommandTest_FormatCommand_04', () => {
    expect(FormatCommand.string2FormatCommand('file recv demo')).toEqual({
      bJumpDo: false,
      cmdFlag: 3000,
      parameters: 'demo',
    });
  });

  it('FormatCommandTest_FormatCommand_05', () => {
    expect(FormatCommand.string2FormatCommand('file send demo')).toEqual({
      bJumpDo: false,
      cmdFlag: 3000,
      parameters: 'demo',
    });
  });

  it('FormatCommandTest_FormatCommand_06', () => {
    expect(FormatCommand.string2FormatCommand(CmdConstant.CMD_GET_HIPERF_EVENTS)).toEqual({
      bJumpDo: true,
      cmdFlag: -1,
      parameters: '',
    });
  });

  it('FormatCommandTest_FormatCommand_07', () => {
    expect(FormatCommand.string2FormatCommand('null')).toEqual({
      bJumpDo: true,
      cmdFlag: -1,
      parameters: '',
    });
  });
});
