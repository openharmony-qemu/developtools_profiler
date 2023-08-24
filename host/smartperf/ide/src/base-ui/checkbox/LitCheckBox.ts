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

@element('lit-check-box')
export class LitCheckBox extends BaseElement {
  private checkbox: HTMLInputElement | undefined;

  static get observedAttributes() {
    return ['checked', 'value'];
  }

  get indeterminate() {
    return this.checkbox!.indeterminate;
  }

  set indeterminate(value) {
    if (value === null || value === false) {
      this.checkbox!.indeterminate = false;
    } else {
      this.checkbox!.indeterminate = true;
    }
  }

  get checked() {
    return this.getAttribute('checked') !== null;
  }

  set checked(boxCheck: boolean) {
    if (boxCheck === null || !boxCheck) {
      this.removeAttribute('checked');
    } else {
      this.setAttribute('checked', '');
    }
  }

  get value() {
    return this.getAttribute('value') || '';
  }

  set value(value: string) {
    this.setAttribute('value', value);
  }

  initHtml(): string {
    return `
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
            gap: 12px;
        }
        .chekebox{
            position:relative;
            display:flex;
            justify-content: center;
            align-items: center;
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
        `;
  }

  initElements(): void {
    this.checkbox = this.shadowRoot?.getElementById('checkbox') as HTMLInputElement;
  }

  connectedCallback() {
    this.checkbox!.addEventListener('change', () => {
      this.checked = this.checkbox!.checked;
      let changeEvent: CustomEventInit<LitCheckBoxChangeEvent> = {
        detail: {
          checked: this.checked,
        },
      };
      this.dispatchEvent(new CustomEvent('change', changeEvent));
    });
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name == 'checked' && this.checkbox) {
      this.checkbox.checked = newValue !== null;
    }
    if (name == 'value') {
      let slot = this.shadowRoot?.getElementById('slot');
      slot!.textContent = newValue;
    }
  }
}

export interface LitCheckBoxChangeEvent {
  checked: boolean;
}
