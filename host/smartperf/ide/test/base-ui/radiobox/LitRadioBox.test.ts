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
import { LitRadioBox } from '../../../dist/base-ui/radiobox/LitRadioBox.js';

// @ts-ignore
import { LitRadioGroup } from '../../../dist/base-ui/radiobox/LitRadioGroup.js';

describe('LitRadioBox Test', () => {
  let litRadioBox = new LitRadioBox();
  let litRadioGroup = new LitRadioGroup();

  litRadioGroup.layout = 'layout';

  litRadioBox.checked = true;
  litRadioBox.checked = false;
  litRadioBox.value = 'value';
  litRadioBox.dis = 'dis';
  it('LitRadioBoxTest01', () => {
    expect(litRadioBox.name).toBeNull();
  });

  it('LitRadioBoxTest02', () => {
    expect(litRadioBox.value).toBe('value');
  });

  it('litRadioGroupTest01', () => {
    let isReturn = litRadioGroup.value.length == 0;
    expect(isReturn).toBeTruthy();
  });
});
