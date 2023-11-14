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

import { LitPopContent } from '../../../dist/base-ui/popover/LitPopContent.js';

describe('LitPopCont Test', () => {
  it('LitPopCont01', () => {
    let litPopContent = new LitPopContent();
    expect(litPopContent).not.toBeUndefined();
    expect(litPopContent).not.toBeNull();
  });

  it('LitPopCont02', () => {
    let litPopContent = new LitPopContent();
    expect(litPopContent.open).toBeFalsy();
  });

  it('LitPopCont03', () => {
    let litPopContent = new LitPopContent();
    litPopContent.open = false;
    expect(litPopContent.open).toBeFalsy();
  });

  it('LitPopCont04', () => {
    let litPopContent = new LitPopContent();
    litPopContent.open = true;
    expect(litPopContent.open).toBeTruthy();
  });

  it('LitPopCont04', () => {
    let litPopContent = new LitPopContent();
    litPopContent.name = '11';
    expect(litPopContent.name).toEqual('11');
  });


  it('LitPopCont06', () => {
    let litPopContent = new LitPopContent();
    expect(litPopContent.attributeChangedCallback('open', '', null || 'false')).toBeUndefined();
  });

  it('LitPopCont07', () => {
    let litPopContent = new LitPopContent();
    expect(litPopContent.attributeChangedCallback('name', '', '')).toBeUndefined();
  });
});
