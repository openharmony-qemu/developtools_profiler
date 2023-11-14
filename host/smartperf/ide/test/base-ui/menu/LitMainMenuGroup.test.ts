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
import { LitMainMenuGroup } from '../../../dist/base-ui/menu/LitMainMenuGroup.js';

describe('litMainMenuGroup Test', () => {
  it('litMainMenuGroup01', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup).not.toBeUndefined();
    expect(litMainMenuGroup).not.toBeNull();
  });

  it('litMainMenuGroup02', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });

  it('litMainMenuGroup03', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.collapsed = true;
    expect(litMainMenuGroup.collapsed).toBeTruthy();
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.collapsed = false;
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });

  it('litMainMenuGroup06', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.radius).toBeFalsy();
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.nocollapsed = true;
    expect(litMainMenuGroup.nocollapsed).toBeTruthy();
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.nocollapsed = false;
    expect(litMainMenuGroup.nocollapsed).toBeFalsy();
  });

  it('litMainMenuGroup05', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });
});
