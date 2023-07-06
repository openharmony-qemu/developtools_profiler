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

  it('LitSelectOption02', function () {
    let litSelect = new LitSelectOption();
    expect(litSelect.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{ 
            display: flex;
            padding: 8px 10px;
            transition: all .3s;
            color: var(--dark-color2,#333);
            tab-index: -1;
            /*overflow: scroll;*/
            align-items: center;
            width: max-content;
            max-lines: 1;
            white-space: nowrap;
            font-size: 0.8rem;
        }
        :host(:not([disabled])[selected]){
            background-color: #0A59F7;
            color: #ffffff;
            font-weight: bold;
        }
        :host(:not([disabled]):not([selected]):hover){
            background-color: var(--dark-background7,#f5f5f5);
        }
        :host([disabled]){
            cursor: not-allowed;
            color: var(--dark-color,#bfbfbf);
        }
        :host([selected][check]) .check{
             display: flex;
        }
        :host(:not([selected])) .check{
             display: none;
        }
        :host(:not([check])) .check{
            display: none;
        }
        
        :host([disabled]) .selected-box{
             display: none;
        }
        :host([selected]) .selected{
            display: flex;
            color: #FFFFFF;
        }
        :host(:not([selected])) .selected{
            display: none;
        }
        </style>
        <div class="selected-box">
            <lit-icon class="selected" name="check" size="20"></lit-icon>
        </div>
        <slot></slot>
<!--        <lit-icon class="check" name="check"></lit-icon>-->
        "
`);
  });
});
