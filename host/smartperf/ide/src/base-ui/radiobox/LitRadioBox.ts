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

import { BaseElement, element } from '../BaseElement.js';
import { LitRadioGroup } from './LitRadioGroup.js';

@element('lit-radio')
export class LitRadioBox extends BaseElement {
  private group: LitRadioGroup | undefined | null;
  private parent: LitRadioGroup | undefined | null;
  private radio: HTMLInputElement | undefined | null;

  static get observedAttributes() {
    return ['checked', 'value', 'disabled'];
  }

  get disabled() {
    return this.getAttribute('disabled') !== null;
  }

  set disabled(value) {
    if (value === null || value === false) {
      this.removeAttribute('disabled');
    } else {
      this.setAttribute('disabled', '');
    }
  }

  get checked() {
    return this.getAttribute('checked') !== null;
  }

  set checked(radioValue: boolean) {
    if (radioValue === null || !radioValue) {
      this.removeAttribute('checked');
    } else {
      this.setAttribute('checked', '');
    }
  }

  get name() {
    return this.getAttribute('name');
  }

  get value() {
    let slot = this.shadowRoot?.getElementById('slot');
    return slot!.textContent || this.textContent || '';
  }

  set value(value: string) {
    this.setAttribute('value', value);
  }

  set dis(dis: string) {
    this.setAttribute('dis', dis);
  }

  initHtml(): string {
    return `
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
        :host([disabled]){ 
            pointer-events: none;
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
        `;
  }

  initElements(): void {
    this.radio = this.shadowRoot?.getElementById('radio') as HTMLInputElement;
  }

  connectedCallback() {
    this.group = this.closest('lit-radio-group') as LitRadioGroup;
    this.parent = this.group || this.getRootNode();
    this.radio = this.shadowRoot?.getElementById('radio') as HTMLInputElement;
    this.checked = this.checked;
    this.radio.addEventListener('change', () => {
      const selector = this.group ? `lit-radio[checked]` : `lit-radio[name="${this.name}"][checked]`;
      const siblingNode = this.parent?.querySelector(selector) as LitRadioBox;
      if (siblingNode) {
        siblingNode.checked = false;
      }
      this.checked = true;
    });
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name == 'checked' && this.radio) {
      this.radio.checked = newValue !== null;
    }
    if (name == 'value') {
      let slot = this.shadowRoot?.getElementById('slot');
      slot!.textContent = newValue;
    }
  }
}
