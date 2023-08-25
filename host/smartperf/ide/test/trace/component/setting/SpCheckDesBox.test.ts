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
//@ts-ignore
import { SpCheckDesBox } from '../../../../dist/trace/component/setting/SpCheckDesBox.js';

describe('SpCheckDesBox Test', () => {
  let spCheckDesBox = new SpCheckDesBox();

  it('SpCheckDesBoxTest01', function () {
    expect(spCheckDesBox.value).toBe('');
  });

  it('SpCheckDesBoxTest02', function () {
    expect(spCheckDesBox.attributeChangedCallback('checked', '', '')).toBeUndefined();
  });

  it('SpCheckDesBoxTest03', function () {
    expect(spCheckDesBox.attributeChangedCallback('value', '', '')).toBeUndefined();
  });

  it('SpCheckDesBoxTest04', function () {
    expect(spCheckDesBox.attributeChangedCallback('des', '', '')).toBeUndefined();
  });

  it('SpCheckDesBoxTest05', function () {
    spCheckDesBox.checked = false;
    expect(spCheckDesBox.checked).toBeFalsy();
  });

  it('SpCheckDesBoxTest07', function () {
    spCheckDesBox.checked = true;
    expect(spCheckDesBox.checked).toBeTruthy();
  });

  it('SpCheckDesBoxTest06 ', function () {
    expect(spCheckDesBox.connectedCallback()).toBeUndefined();
  });
});
