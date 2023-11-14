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
import { LitButton, LitSelect } from '../../../dist/base-ui/select/LitSelect.js';
import { LitSelectOption } from '../../../src/base-ui/select/LitSelectOption';

describe('LitSelect Test', () => {
  it('LitSelectTest01', function () {
    let litSelect = new LitSelect();
    expect(litSelect).not.toBeUndefined();
  });

  it('LitSelectTest02', function () {
    document.body.innerHTML = `<lit-select id="litSelect"></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    expect(select).not.toBeUndefined();
  });

  it('LitSelectTest03', function () {
    document.body.innerHTML = `<lit-select id="litSelect"></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.value = 'value';
    expect(select.value).toBe('value');
  });

  it('LitSelectTest04', function () {
    document.body.innerHTML = `<lit-select id="litSelect"></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.border = 'value';
    expect(select.border).toBe('true');
  });
  it('LitSelectTest05', function () {
    let lit = new LitSelect();
    expect(lit.border).toBe('true');
  });
  it('LitSelectTest06', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.listHeight = true;
    expect(select.listHeight).toBe('true');
  });

  it('LitSelectTest07', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.defaultValue = true;
    expect(select.defaultValue).toBe('true');
  });

  it('LitSelectTest08', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.loading = 1;
    expect(select.loading).toBe(true);
  });

  it('LitSelectTest09', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    expect(select.isMultiple()).toBe(false);
  });

  it('LitSelectTest10', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    // select.inputElement.value = '3333';
    select.click();
    expect(select.focused).toBe(true);
  });

  it('LitSelectTest11', function () {
    document.body.innerHTML = `<lit-select allow-clear id="litSelect" ></lit-selectallow-clear>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.clear();
    expect(select.inputElement).toBeUndefined();
  });

  it('LitSelectTest12', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    expect(select.reset()).toBeUndefined();
  });

  it('LitSelectTest13', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    let newTag = select.newTag('111', '111');
    expect(newTag.text).toBe('111');
  });
  it('LitSelectTest14', function () {
    document.body.innerHTML = `<lit-select id="litSelect" mode="multiple" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.dataSource = [{ key: '111' }];
    let cleart = select.clearElement as HTMLElement;
    expect(select.inputElement).toBeUndefined();
  });

  it('LitSelectTest15', function () {
    document.body.innerHTML = `<lit-select id="litSelect" mode="multiple" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    let input = select.inputElement as HTMLInputElement;
    expect(select.inputElement).toBeUndefined();
  });

  it('LitSelectTest16', function () {
    document.body.innerHTML = `<lit-select id="litSelect" mode="multiple" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.dataSource = [{ key: '111' }];
    expect(select.inputElement).toBeUndefined();
  });

  it('LitSelectTest17', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.placeholder = true;
    expect(select.placeholder).toBe('true');
  });
  it('LitSelectTest20', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.rounded = 1;
    expect(select.rounded).toBe(true);
  });

  it('LitSelectTest21', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.placement = 1;
    expect(select.placement).toBe('1');
  });

  it('LitSelectTest23', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.canInsert = true;
    expect(select.canInsert).toBeTruthy();
  });
  it('LitSelectTest24', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.rounded = false;
    expect(select.rounded).toBeFalsy();
  });
  it('LitSelectTest25', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.placement = false;
    expect(select.placement).toBeFalsy();
  });
  it('LitSelectTest26', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.border = true;
    expect(select.border).toBeTruthy();
  });
  it('LitSelectTest27', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.canInsert = false;
    expect(select.canInsert).toBeFalsy();
  });
  it('LitSelectTest28', function () {
    document.body.innerHTML = `<lit-select id="litSelect" allow-clear></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    select.loading = false;
    expect(select.loading).toBeFalsy();
  });

  it('LitSelectTest29', function () {
    let lit = new LitSelect();
    lit.border = false;
    expect(lit.border).toBe('false');
  });

  it('LitSelectTest30', function () {
    let litSelect = (document.body.innerHTML = `<lit-select id="litSelect" allow-clear>
            <lit-select-option id="litSelectOption1" selected></lit-select-option>
            <lit-select-option id="litSelectOption2"></lit-select-option>
        </lit-select>` as LitSelect);
    let select = document.querySelector('#litSelect') as LitSelect;
    expect(select.reset()).toBeUndefined();
  });
  it('LitSelectTest31', function () {
    document.body.innerHTML = `<lit-select id="litSelect"  adaptive-expansion></lit-select>`;
    let select = document.querySelector('#litSelect') as LitSelect;
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
    select.isMultiple = jest.fn(() => true);
    select.selectClearEl.dispatchEvent(mouseClickEvent);
  });
});
