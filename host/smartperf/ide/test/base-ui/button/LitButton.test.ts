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
import { LitButton } from '../../../dist/base-ui/button/LitButton.js';

describe('button Test', () => {
  it('buttonTest01', function () {
    let litButton = new LitButton();
    expect(litButton).not.toBeUndefined();
  });
  it('buttonTest02', function () {
    let litButton = new LitButton();
    litButton.text = ' ';
    expect(litButton.text).toBe(' ');
  });
  it('buttonTest03', function () {
    let litButton = new LitButton();
    litButton.text = 'test';
    expect(litButton.text).toBe('test');
  });
  it('buttonTest04', function () {
    let litButton = new LitButton();
    litButton.back = 'test';
    expect(litButton.back).toBe('test');
  });
  it('buttonTest05', function () {
    let litButton = new LitButton();
    litButton.icon = 'test';
    expect(litButton.icon).toBe('test');
  });
  it('buttonTest06', function () {
    let litButton = new LitButton();
    litButton.height = 'test';
    expect(litButton.height).toBe('test');
  });
  it('buttonTest07', function () {
    let litButton = new LitButton();
    litButton.width = 'test';
    expect(litButton.width).toBe('test');
  });
  it('buttonTest08', function () {
    let litButton = new LitButton();
    litButton.color = 'test';
    expect(litButton.color).toBeUndefined();
  });
  it('buttonTest09', function () {
    let litButton = new LitButton();
    litButton.font_size = 'test';
    expect(litButton.font_size).toBeUndefined();
  });
  it('buttonTest10', function () {
    let litButton = new LitButton();
    litButton.border = 'test';
    expect(litButton.border).toBeUndefined();
  });
  it('buttonTest11', function () {
    let litButton = new LitButton();
    litButton.padding = 'test';
    expect(litButton.padding).toBeUndefined();
  });
  it('buttonTest12', function () {
    let litButton = new LitButton();
    litButton.justify_content = 'test';
    expect(litButton.justify_content).toBeUndefined();
  });
  it('buttonTest13', function () {
    let litButton = new LitButton();
    litButton.border_radius = 'test';
    expect(litButton.border_radius).toBeUndefined();
  });
  it('buttonTest14', function () {
    let litButton = new LitButton();
    litButton.margin_icon = 'test';
    expect(litButton.margin_icon).toBeUndefined();
  });
});
