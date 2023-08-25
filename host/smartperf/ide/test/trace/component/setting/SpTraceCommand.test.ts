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
import { SpTraceCommand } from '../../../../dist/trace/component/setting/SpTraceCommand.js';

describe('SPTraceCommand Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <trace-command id = "command"><trace-command>
        `;
  });
  it('new SPTraceCommand', function () {
    expect(new SpTraceCommand()).not.toBeNull();
  });

  it(' SpAllocations get Default attrValue', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    expect(spEle.hdcCommon).toEqual('');
  });

  it(' SpAllocations set  attrValue', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.hdcCommon = 'test';
    expect(spEle.hdcCommon).toEqual('test');
  });

  it(' SpTraceCommandtest01', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.show = false;
    expect(spEle.show).toBeFalsy();
  });
  it(' SpTraceCommandtest02', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.show = true;
    expect(spEle.show).toBeTruthy();
  });
  it(' SpTraceCommandtest03', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    expect(spEle.disconnectedCallback()).toBeUndefined();
  });
});
