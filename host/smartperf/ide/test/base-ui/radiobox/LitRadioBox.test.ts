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
import { LitRadioBox } from '../../../dist/base-ui/radiobox/LitRadioBox.js';

// @ts-ignore
import { LitRadioGroup } from '../../../dist/base-ui/radiobox/LitRadioGroup.js';

describe('LitRadioBox Test', () => {
  let litRadioBox = new LitRadioBox();
  let litRadioGroup = new LitRadioGroup();

  litRadioGroup.layout = 'layout';

  litRadioBox.checked = true;
  litRadioBox.checked = false;
  litRadioBox.value = 'value';
  litRadioBox.dis = 'dis';
  it('LitRadioBoxTest01', () => {
    expect(litRadioBox.name).toBeNull();
  });

  it('LitRadioBoxTest02', () => {
    expect(litRadioBox.value).toBe('value');
  });

  it('LitRadioBoxTest03', () => {
    expect(litRadioBox.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host([dis=round]):host{ 
            font-family: Helvetica,serif;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 16px;
            font-weight: 400;
        }
        :host([dis=round]) lit-icon{
           display: none;
        }
        :host([dis=round]) #radio{
            position:absolute;
            clip:rect(0,0,0,0);
        }
        :host([dis=round]) :host(:focus-within) .selected label:hover .selected{ 
            z-index:1;
            border-color:#a671ef;
        }
        :host([dis=round]) label{
            box-sizing:border-box;
            cursor:pointer;
            display:flex;
            align-items:center;
        }
        :host([dis=round]) .selected{
            position:relative;
            display: flex;
            box-sizing: border-box;
            width: 16px;
            height: 16px;
            border-radius:50%;
            border: 2px solid var(--dark-color1,#4D4D4D);
            margin-right:1em;    
        }
        .selected{
            position:relative;
            box-sizing: border-box;
            margin-right:1em;    
        }
        :host([dis=round]) .selected::before{
            content:'';
            width:6px;
            height:6px;
            margin:auto;
            border-radius:50%;
            background:#a671ef;
            transform: scale(0);
        }
       :host([dis=round]) #radio:focus-visible+label .selected::after{
            transform:scale(2.5);
        }
        :host([dis=round]) #radio:checked+label .selected::before{
            transform: scale(1);
        }
        :host([dis=round]) #radio:checked+label .selected{
            border-color:#a671ef;
        }
        :host([dis=check]):host{ 
           opacity: 0.9;
           font-family: Helvetica,serif;
           font-size: 14px;
           text-align: left;
           font-weight: 400;
        }
        :host([dis=round]) lit-icon{
           visibility: visible;
        }
        :host([dis=check]) #radio{
            position:absolute;
            clip:rect(0,0,0,0);
        }
        :host([dis=check]) label{
            box-sizing:border-box;
            cursor:pointer;
            display:flex;
            align-items:center;
        }
        :host([dis=check]) .selected{
            position:relative;
            display:flex;
            justify-content: center;
            align-items: center;
            margin-right:5px;
            width: 16px;
            height:16px;
        }
        :host([dis=check]) .selected::before{
            position:absolute;
            content:'';
            width:74%;
            height:0.15em;
            background:#fff;
            transform:scale(0);
            border-radius: 0.15em;
        }
        :host([dis=check]) .selected::after{
            content:'';
            position:absolute;
            width:100%;
            height:100%;
            border-radius:50%;
            background:#1A83FF;
            opacity:0.2;
            transform:scale(0);
            z-index:-1;
        }
        :host([dis=check]) #radio:checked:not(:indeterminate)+label .selected .icon{
            transform: scale(1.5);
        }
        :host([dis=check]) #radio:indeterminate+label .selected::before{
            transform:scale(1);
        }
        :host([dis=check]) .icon{
            width: 90%;
            height: 55%;
            margin-left: 5px;
            transform: scale(0);
        }
        :host([checked][dis=check]) {
            background-color: #1A83FF;
            color:#ffffff
        }
        </style>
        <input type="checkbox" id="radio" >
        <label id="label" for="radio">
            <span class="selected">
            <lit-icon name="checkmark" class="icon" size="8">
            </lit-icon>
            </span>
            <slot id='slot'></slot>
        </label>
        "
`);
  });

  it('litRadioGroupTest01', () => {
    let isReturn = litRadioGroup.value.length == 0;
    expect(isReturn).toBeTruthy();
  });

  it('litRadioGroupTest02', () => {
    expect(litRadioGroup.initHtml()).toMatchInlineSnapshot(`
"
        <style>   
        :host([direction]) {
            flex-direction: null;
        }
        :host {
            display: -webkit-flex; 
            display: flex;
            flex-direction: column;
        }
        :host([layout="compact"]) {
            gap:5px;
        }
        :host([layout="dispersion"]) {
           gap:10px;
        }
        :host(:not([direction])) {
            flex-direction: column;
        }
        </style>
        <slot class="radio-group"></slot>"
`);
  });
});
