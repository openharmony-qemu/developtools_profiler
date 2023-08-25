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
import { LitCheckBox } from '../../../dist/base-ui/checkbox/LitCheckBox.js';

describe('checkBox Test', () => {
  it('checkBoxTest01', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox).not.toBeUndefined();
    expect(litCheckBox).not.toBeNull();
  });

  it('checkBoxTest02', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox.checked).toBeFalsy();
  });

  it('checkBoxTest03', function () {
    let litCheckBox = new LitCheckBox();
    litCheckBox.checked = true;
    expect(litCheckBox.checked).toBeTruthy();
  });

  it('checkBoxTest04', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox.value).toEqual('');
  });

  it('checkBoxTest04', function () {
    let litCheckBox = new LitCheckBox();
    litCheckBox.value = 'test';
    expect(litCheckBox.value).toEqual('test');
  });

  it('checkBoxTest05', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.checked = false;
    expect(litCheckBox.checked).toBeFalsy();
  });
  it('checkBoxTest06', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.indeterminate = false;
    expect(litCheckBox.indeterminate).toBeFalsy();
  });

  it('checkBoxTest07', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.indeterminate = true;
    expect(litCheckBox.indeterminate).toBeTruthy();
  });
});
