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

  it('LitSelectTest18', function () {
    let litSelect = new LitSelect();
    expect(litSelect.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        
        :host{
            display: inline-flex;
            position: relative;
            overflow: visible;
            cursor: pointer;
            border-radius: 2px;
            outline: none;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
        }
        :host(:not([border])),
        :host([border='true']){
            border: 1px solid var(--bark-prompt,#dcdcdc);
        }
        input{
            border: 0;
            outline: none;
            background-color: transparent;
            cursor: pointer;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
            display: inline-flex;
            color: var(--dark-color2,rgba(0,0,0,0.9));
        }
        :host([highlight]) input {
            color: rgba(255,255,255,0.9);
        }
        :host([mode])  input{
            padding: 6px 0px;
        }
        :host([mode])  .root{
            padding: 1px 8px;
        }
        .root{
            position: relative;
            padding: 3px 6px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-radius: 2px;
            outline: none;
            font-size: 1rem;
            z-index: 2;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
            width: 100%;
        }
        .body{
            position: absolute;
            bottom: 100%;
            z-index: 99;
            padding-top: 5px;
            margin-top: 2px;
            background-color: var(--dark-background4,#fff);
            width: 100%;
            transform: scaleY(.6);
            visibility: hidden;
            opacity: 0;
            transform-origin: bottom center;
            display: block;
            flex-direction: column;
        }
        .body-bottom{
            bottom: auto;
            top: 100%;
            transform-origin: top center;
        }
        :host([placement="bottom"]) .body{
            bottom:unset;
            top: 100%;
            transition: none;
            transform: none;
        }

        :host([rounded]) .body {
            border-radius: 16px;
        }
        :host([rounded]) .root {
            border-radius: 16px;
            height: 25px;
        }
        .icon{
            pointer-events: none;
        }
        .noSelect{
          -moz-user-select:none;
          -ms-user-select:none;
          user-select:none;
          -khtml-user-select:none;
          -webkit-touch-callout:none;
          -webkit-user-select:none;
        }

        :host(:not([border]):not([disabled]):focus),
        :host([border='true']:not([disabled]):focus),
        :host(:not([border]):not([disabled]):hover),
        :host([border='true']:not([disabled]):hover){
            border:1px solid var(--bark-prompt,#ccc)
        }
        :host(:not([disabled]):focus) .body,
        :host(:not([disabled]):focus-within) .body{
            transform: scaleY(1);
            opacity: 1;
            z-index: 99;
            visibility: visible;
        }
        :host(:not([disabled]):focus)  input{
            color: var(--dark-color,#bebebe);
        }
        :host(:not([border])[disabled]) *,
        :host([border='true'][disabled]) *{
            background-color: var(--dark-background1,#f5f5f5);
            color: #b7b7b7;
            cursor: not-allowed;
        }
        :host([border='false'][disabled]) *{
            color: #b7b7b7;
            cursor: not-allowed;
        }
        :host(:not([mode]))  input{
            width: 100%;
        }
        .body{
            max-height: 256px;
            overflow: auto;
            border-radius: 2px;
            box-shadow: 0 5px 15px 0px #00000033;
        }
        .multipleRoot input::-webkit-input-placeholder {
            color: var(--dark-color,#aab2bd);
        }
        :host(:not([loading])) .loading{
            display: none;
        }
        :host([loading]) .loading{
            display: flex;
        }
        :host(:not([allow-clear])) .clear{
            display: none;
        }
        :host([loading]) .icon{
            display: none;
        }
        :host(:not([loading])) .icon{
            display: flex;
        }
        .clear:hover{
            color: #8c8c8c;
        }
        .clear{
            color: #bfbfbf;
            display: none;
        }
        .multipleRoot{
            display: flex;
            align-items: center;
            flex-flow: wrap;
            flex-wrap: wrap;
            flex-direction: column;
        }
        .search{
            color: #bfbfbf;
            display: none;
        }
        .tag{
            overflow: auto;
            height: auto;
            display: inline-flex;
            position: relative;
            align-items: center;
            font-size: .75rem;
            font-weight: bold;
            padding: 1px 4px;
            margin-right: 4px;
            margin-top: 1px;
            margin-bottom: 1px;
            color: #242424;
            background-color: #f5f5f5;
        }
        .tag-close:hover{
            color: #333;
        }
        .tag-close{
            padding: 2px;
            font-size: .8rem;
            color: #999999;
            margin-left: 0px;
        }
        </style>
        <div class="root noSelect" tabindex="0" hidefocus="true">
            <div class="multipleRoot">
            <input placeholder="请选择" autocomplete="off" readonly tabindex="0"></div>
            <lit-loading class="loading" size="12"></lit-loading>
            <lit-icon class="icon" name='down' color="#c3c3c3"></lit-icon>
            <lit-icon class="clear" name='close-circle-fill'></lit-icon>
            <lit-icon class="search" name='search'></lit-icon>
        </div>
        <div class="body">
            <slot></slot>
            <slot name="footer"></slot>
        </div>
        "
`);
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
});
