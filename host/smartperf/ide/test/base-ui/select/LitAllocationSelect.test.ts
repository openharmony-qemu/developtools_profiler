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
import { LitAllocationSelect } from '../../../dist/base-ui/select/LitAllocationSelect.js';

describe('LitAllocationSelect Test', () => {
  let litAllocationSelect = new LitAllocationSelect();
  it('LitAllocationSelectTest01', function () {
    expect(litAllocationSelect.value).toBe('');
  });
  it('LitAllocationSelectTest02', () => {
    expect(litAllocationSelect.processData).toBeUndefined();
  });
  it('LitAllocationSelectTest03', function () {
    expect(litAllocationSelect.placement).toBe('');
  });
  it('LitAllocationSelectTest04', function () {
    litAllocationSelect.placement = true;
    expect(litAllocationSelect.placement).toBeTruthy();
  });
  it('LitAllocationSelectTest05', function () {
    litAllocationSelect.placement = false;
    expect(litAllocationSelect.placement).toBeFalsy();
  });
  it('LitAllocationSelectTest06', function () {
    expect(litAllocationSelect.listHeight).toBe('256px');
  });
  it('LitAllocationSelectTest07', function () {
    litAllocationSelect.listHeight = 'test';
    expect(litAllocationSelect.listHeight).toBe('test');
  });
  it('LitAllocationSelectTest08', function () {
    litAllocationSelect.placeholder = 'test';
    expect(litAllocationSelect.placeholder).toBe('test');
  });
  it('LitAllocationSelectTest09', () => {
    expect(litAllocationSelect.initElements()).toBeUndefined();
  });

  it('LitAllocationSelectTest10', () => {
    litAllocationSelect.processData = [];
    expect(litAllocationSelect.processData).toBe(undefined);
  });

  it('LitAllocationSelectTest12', () => {
    litAllocationSelect.processData = ['1', '2', '3'];
    expect(litAllocationSelect.processData).toBe(undefined);
  });

  it('LitAllocationSelectTest11', () => {
    const onclick = jest.fn();
    let allocationSelect = (document.body.innerHTML = `
            <lit-allocation-select id='select'></lit-allocation-select>
        `);
    const select = document.getElementById('select');
    expect(onclick).not.toBeCalled();
    select!.onclick = onclick;
    select!.click();
    expect(onclick).toBeCalled();
    expect(onclick).toHaveBeenCalledTimes(1);
  });
});
