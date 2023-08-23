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
import { LitCheckBox } from '../../../dist/base-ui/checkbox/LitCheckBox.js';

describe('checkBox Test', () => {
  it('checkBoxTest01', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox).not.toBeUndefined();
    expect(litCheckBox).not.toBeNull();
  });

  it('checkBoxTest02', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox.checked).toBeFalsy();
  });

  it('checkBoxTest03', function () {
    let litCheckBox = new LitCheckBox();
    litCheckBox.checked = true;
    expect(litCheckBox.checked).toBeTruthy();
  });

  it('checkBoxTest04', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox.value).toEqual('');
  });

  it('checkBoxTest04', function () {
    let litCheckBox = new LitCheckBox();
    litCheckBox.value = 'test';
    expect(litCheckBox.value).toEqual('test');
  });

  it('checkBoxTest05', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.checked = false;
    expect(litCheckBox.checked).toBeFalsy();
  });
  it('checkBoxTest06', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.indeterminate = false;
    expect(litCheckBox.indeterminate).toBeFalsy();
  });

  it('checkBoxTest07', function () {
    document.body.innerHTML = `<lit-check-box></lit-check-box>
        `;
    let litCheckBox = new LitCheckBox();
    litCheckBox.indeterminate = true;
    expect(litCheckBox.indeterminate).toBeTruthy();
  });

  it('checkBoxTest08', function () {
    let litCheckBox = new LitCheckBox();
    expect(litCheckBox.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{ 
            display:flex;
            opacity: 0.86;
            font-family: Helvetica;
            font-size: 14px;
            text-align: left;
            line-height: 16px;
            font-weight: 400;
        }
        #checkbox{
            position:absolute;
            clip:rect(0,0,0,0);
        }

        label{
            box-sizing:border-box;
            cursor:pointer;
            display:flex;
            align-items:center;
        }
        .chekebox{
            position:relative;
            display:flex;
            justify-content: center;
            align-items: center;
            margin-right:12px;
            width: 16px;
            height:16px;
            border: 1px solid var(--dark-color1,#4D4D4D);
            border-radius: 20%;
        }
        .chekebox::before{
            position:absolute;
            content:'';
            width:74%;
            height:0.15em;
            background:#3391FF;
            transform:scale(0);
            border-radius: 0.15em;
        }
        .chekebox{ 
            background:var(--dark-background,#FFFFFF);
        }
        .chekebox::after{
            content:'';
            position:absolute;
            width:100%;
            height:100%;
            border-radius:50%;
            background:#FFFFFF;
            opacity:0.2;
            transform:scale(0);
            z-index:-1;
        }
        #checkbox:checked:not(:indeterminate)+label .chekebox .icon{
            transform: scale(1.5);
        }
        #checkbox:checked+label .chekebox,#checkbox:indeterminate+label .chekebox{
            border-color:#3391FF;
        }
        #checkbox:indeterminate+label .chekebox::before{
            transform:scale(1);
        }
        .icon{
            width: 100%;
            height: 94%;
            transform: scale(0);
        }
        </style>
        <input type="checkbox" id="checkbox">
        <label for="checkbox">
          <span class="chekebox">
          <lit-icon name="checkmark" class="icon" color="#3391FF" size="15">
          </lit-icon>
          </span>
          <slot id="slot"></slot>
       </label>
        "
`);
  });
});
