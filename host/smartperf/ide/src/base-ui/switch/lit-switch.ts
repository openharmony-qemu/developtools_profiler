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

@element('lit-switch')
export default class LitSwitch extends BaseElement {
  private switch: HTMLInputElement | null | undefined;
  private isfocus: boolean | undefined;

  static get observedAttributes() {
    return ['disabled', 'checked'];
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

  set checked(value) {
    if (value === null || value === false) {
      this.removeAttribute('checked');
    } else {
      this.setAttribute('checked', '');
    }
  }

  get name() {
    return this.getAttribute('name');
  }

  initElements(): void {}

  initHtml(): string {
    return `
        <style>
        :host{ 
            display:inline-block; 
            -webkit-tap-highlight-color: transparent;
        }
        #name{
            transition:0.31s width,0.31s height,0.31s background-color;
            width:2.41em;
            height:1.21em;
            background: #3391FF;
            display:flex;
            padding:0.124em;
            border-radius:1.21em;
            cursor:pointer;
        }
        
        :host(:not([checked])) #name {
           background: #999999;
        }
       
        #name::before{
            content:'';
            flex:0;
            transition:.2s cubic-bezier(.12, .4, .29, 1.46) flex;
        }
        #name::after{
            content:'';
            width:.4em;
            height:.4em;
            border-radius:1.2em;
            border:.4em solid #fff;
            background-color:#ffffff;
            transition:.3s background,.3s padding,.3s width,.3s height,.3s border-radius,.3s border;
            box-shadow: 0 2px 4px 0 rgba(0,35,11,0.2);
        }
        #name:active::after{
            padding:0 .3em;
        }
        #switch:checked+#name{
            background:#42b983);
        }
        #switch:checked+#name::before{
            flex:1;
        }
        #switch{
            position:absolute;
            clip:rect(0,0px,0px,0);
        }
        :host(:focus-within) #name::after,:host(:active) ::after{ 
            background:#42b983;
        }
        :host(:focus-within) #name{ 
            box-shadow: 0 0 10px rgba(0,0,0,0.1); 
        }
        :host(:focus-within) #switch,:host(:active) #switch{
            z-index:2
        }
        :host([disabled]){ 
            pointer-events: none; 
            opacity:.5; 
        }
        :host([disabled]) #name{ 
            pointer-events: all;  
            cursor: not-allowed; 
        }
        </style>
        <input type="checkbox" id="switch"><label id="name" for="switch"></label>
        `;
  }

  connectedCallback() {
    this.switch = this.shadowRoot?.getElementById('switch') as HTMLInputElement;
    this.disabled = this.disabled;
    this.checked = this.checked;
    this.switch!.onchange = (ev) => {
      this.checked = this.switch!.checked;
      let changeEvent: CustomEventInit<LitSwitchChangeEvent> = {
        detail: {
          checked: this.checked,
        },
      };
      this.dispatchEvent(new CustomEvent('change', changeEvent));
    };
    this.switch.onkeydown = (ev) => {
      switch (ev.keyCode) {
        case 13: //enter
          this.checked = !this.checked;
          let changeEvent: CustomEventInit<LitSwitchChangeEvent> = {
            detail: {
              checked: this.checked,
            },
          };
          this.dispatchEvent(new CustomEvent('change', changeEvent));
          break;
        default:
          break;
      }
    };
    this.switch.onfocus = (ev) => {
      ev.stopPropagation();
      if (!this.isfocus) {
        this.dispatchEvent(
          new CustomEvent('focus', {
            detail: { value: this.switch!.value },
          })
        );
      }
    };
    this.switch.onblur = (ev) => {
      ev.stopPropagation();
      if (getComputedStyle(this.switch!).zIndex == '2') {
        this.isfocus = true;
      } else {
        this.isfocus = false;
        this.dispatchEvent(
          new CustomEvent('blur', {
            detail: { value: this.switch!.value },
          })
        );
      }
    };
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name === 'disabled' && this.switch) {
      if (newValue !== null) {
        this.switch.setAttribute('disabled', '');
      } else {
        this.switch.removeAttribute('disabled');
      }
    }
    if (name === 'checked' && this.switch) {
      if (newValue !== null) {
        this.switch.checked = true;
      } else {
        this.switch.checked = false;
      }
    }
  }
}

export interface LitSwitchChangeEvent {
  checked: boolean;
}
