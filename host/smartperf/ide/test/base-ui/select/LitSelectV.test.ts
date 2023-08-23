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
import { LitSelectV } from '../../../dist/base-ui/select/LitSelectV.js';

describe('LitSelectV Test', () => {
  let litSelectV = new LitSelectV();
  it('LitSelectVTest01', function () {
    expect(litSelectV.value).not.toBeUndefined();
  });
  it('LitSelectVTest02', function () {
    litSelectV.rounded = true;
    expect(litSelectV.rounded).toBeTruthy();
  });
  it('LitSelectVTest06', function () {
    litSelectV.rounded = false;
    expect(litSelectV.rounded).toBeFalsy();
  });
  it('LitSelectVTest03', function () {
    expect(litSelectV.placement).toBe('');
  });
  it('LitSelectVTest04', function () {
    litSelectV.placement = true;
    expect(litSelectV.placement).toBeTruthy();
  });
  it('LitSelectVTest05', function () {
    litSelectV.placement = false;
    expect(litSelectV.placement).toBeFalsy();
  });
  it('LitSelectVTest07', function () {
    litSelectV.boder = true;
    expect(litSelectV.border).toBeTruthy();
  });
  it('LitSelectVTest08', function () {
    litSelectV.border = false;
    expect(litSelectV.border).toBe('false');
  });
  it('LitSelectVTest14', function () {
    litSelectV.border = true;
    expect(litSelectV.border).toBe('true');
  });
  it('LitSelectVTest09', function () {
    litSelectV.defaultValue = 'test';
    expect(litSelectV.defaultValue).toBe('test');
  });
  it('LitSelectVTest010', function () {
    litSelectV.placeholder = 'test';
    expect(litSelectV.placeholder).toBe('test');
  });
  it('LitSelectVTest011', function () {
    litSelectV.all = true;
    expect(litSelectV.all).toBeTruthy();
  });
  it('LitSelectVTest012', function () {
    litSelectV.all = false;
    expect(litSelectV.all).toBeFalsy();
  });
  it('LitSelectVTest013', function () {
    let value = [
      {
        length: 1,
      },
    ];
    let valueStr = '';
    expect(litSelectV.dataSource(value, valueStr)).toBeUndefined();
  });
  it('LitSelectVTest014', function () {
    let value = [
      {
        length: 1,
      },
    ];
    let valueStr = 'aa';
    expect(litSelectV.dataSource(value, valueStr)).toBeUndefined();
  });
  it('LitSelectVTest015', function () {
    expect(litSelectV.connectedCallback()).toBeUndefined();
  });
  it('LitSelectVTest016', function () {
    let valueStr = 'aa';
    expect(litSelectV.dataSource([], valueStr)).toBeUndefined();
  });
  it('LitSelectVTest017', function () {
    let value = [
      {
        length: 1,
      },
    ];
    let valueStr = 'aa';
    litSelectV.all = true;
    expect(litSelectV.dataSource(value, valueStr)).toBeUndefined();
  });
  it('LitSelectVTest018', function () {
    let value = [
      {
        length: 1,
      },
    ];
    let valueStr = 'aa';
    litSelectV.title = 'Event List';
    expect(litSelectV.dataSource(value, valueStr)).toBeUndefined();
  });
});
