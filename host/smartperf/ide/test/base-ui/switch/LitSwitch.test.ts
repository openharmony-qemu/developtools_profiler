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
import LitSwitch from '../../../dist/base-ui/switch/lit-switch';

describe('LitSwitch Test', () => {
  let litSwitch = new LitSwitch();
  litSwitch.checked = true;
  litSwitch.checked = false;
  litSwitch.disabled = true;
  litSwitch.disabled = false;

  it('LitSwitchTest01', () => {
    expect(litSwitch.name).toBeNull();
  });

  it('LitSwitchTest02', () => {
    expect(litSwitch.disabled).toBeFalsy();
  });

  it('LitSwitchTest03', () => {
    expect(litSwitch.checked).toBeFalsy();
  });

  it('LitSwitchTest04', () => {
    LitSwitch.switch = document.querySelector('#switch') as HTMLInputElement;
    expect(litSwitch.connectedCallback()).toBeUndefined();
  });

  it('LitSwitchTest05', () => {
    expect(litSwitch.attributeChangedCallback('disabled', 'disabled', '')).toBeUndefined();
  });

  it('LitSwitchTest06', () => {
    expect(litSwitch.attributeChangedCallback('disabled', 'disabled', null)).toBeUndefined();
  });

  it('LitSwitchTest07', () => {
    expect(litSwitch.attributeChangedCallback('checked', 'disabled', '')).toBeUndefined();
  });

  it('LitSwitchTest08', () => {
    expect(litSwitch.attributeChangedCallback('checked', 'disabled', null)).toBeUndefined();
  });
});
