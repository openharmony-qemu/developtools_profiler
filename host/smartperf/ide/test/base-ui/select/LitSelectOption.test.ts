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
import { LitSelectOption } from '../../../dist/base-ui/select/LitSelectOption.js';

describe('LitSelectOption Test', () => {
  it('LitSelectOptionTest01', function () {
    document.body.innerHTML = "<lit-select-option id ='aa' disabled></lit-select-option>";
    let inner = document.querySelector('#aa') as LitSelectOption;
    expect(inner).not.toBeUndefined();
  });
  it('LitSelectOptionTest03', function () {
    expect(LitSelectOption.adoptedCallback).toBeUndefined();
  });

  it('LitSelectOptionTest04', function () {
    expect(LitSelectOption.disconnectedCallback).toBeUndefined();
  });

  it('LitSelectOptionTest05', function () {
    expect(LitSelectOption.attributeChangedCallback).toBeUndefined();
  });
});
