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

@element('lit-allocation-select')
export class LitAllocationSelect extends BaseElement {
  private selectAllocationInputEl: HTMLInputElement | null | undefined;
  private selectAllocationInputContent: HTMLDivElement | undefined;
  private selectAllocationOptions: any;

  static get observedAttributes() {
    return ['value', 'disabled', 'placeholder'];
  }

  get defaultPlaceholder() {
    return this.getAttribute('placeholder') || '';
  }

  get placeholder() {
    return this.getAttribute('placeholder') || this.defaultPlaceholder;
  }

  set placeholder(selectAllocationValue) {
    this.setAttribute('placeholder', selectAllocationValue);
  }

  get value() {
    return this.getAttribute('value') || '';
  }

  set value(selectAllocationValue: string) {
    this.setAttribute('value', selectAllocationValue);
  }

  set processData(value: Array<string>) {
    this.selectAllocationOptions.innerHTML = '';
    value.forEach((item) => {
      let option = document.createElement('div');
      option.className = 'option';
      option.innerHTML = item;
      option.style.padding = '8px 10px';
      this.selectAllocationOptions.appendChild(option);
      this.selectAllocationInputEl?.focus();
    });
  }

  get placement(): string {
    return this.getAttribute('placement') || '';
  }

  set placement(selectAllocationValuePlacement: string) {
    if (selectAllocationValuePlacement) {
      this.setAttribute('placement', selectAllocationValuePlacement);
    } else {
      this.removeAttribute('placement');
    }
  }

  get listHeight() {
    return this.getAttribute('list-height') || '256px';
  }

  set listHeight(value) {
    this.setAttribute('list-height', value);
  }

  attributeChangedCallback(name: any, oldValue: any, newValue: any) {
    switch (name) {
      case 'value':
        this.selectAllocationInputEl!.value = newValue;
        break;
      case 'placeholder':
        this.selectAllocationInputEl!.placeholder = newValue;
        break;
    }
  }

  initElements(): void {
    this.selectAllocationInputContent = this.shadowRoot!.querySelector('.multipleSelect') as HTMLDivElement;
    this.addEventListener('click', () => {
      if (this.selectAllocationOptions.style.visibility == 'visible') {
        this.selectAllocationOptions.style.visibility = 'hidden';
        this.selectAllocationOptions.style.opacity = '0';
      } else {
        this.selectAllocationOptions.style.visibility = 'visible';
        this.selectAllocationOptions.style.opacity = '1';
      }
      this.selectAllocationInputContent!.dispatchEvent(new CustomEvent('inputClick', {}));
    });
    this.addEventListener('focusout', (e) => {
      this.selectAllocationOptions.style.visibility = 'hidden';
      this.selectAllocationOptions.style.opacity = '0';
    });
    this.initData();
  }

  initHtml() {
    return `
        <style>
        :host{
            display: inline-flex;
            overflow: visible;
            cursor: pointer;
            position: relative;
            border-radius: 16px;
            outline: none;
            user-select:none;
            width: 75%;
            -webkit-user-select:none ;
            -moz-user-select:none;
        }
        :host(:not([border])),
        :host([border='true']){
            border: 1px solid var(--bark-prompt,#dcdcdc);
        }
        .multipleSelect{
            display: flex;
            width: 100%;
            z-index: 2999;
            position: relative;
            padding: 3px 6px;
            font-size: 1rem;
            transition: all .3s;
            outline: none;
            user-select:none;
            align-items: center;
            justify-content: space-between;
            -webkit-user-select:none ;
            -moz-user-select:none;
        }
        input{
            display: inline-flex;
            width:100%;
            z-index: 8999;
            color: var(--dark-color2,rgba(0,0,0,0.9));
            background-color: transparent;
            border: 0;
            user-select:none;
            outline: none;
            cursor: pointer;
            -webkit-user-select:none ;
            -moz-user-select:none;
        }
         .body{
            max-height: ${this.listHeight};
            width: 100%;
            display: block;
            overflow: auto;
            position: absolute;
            bottom: 100%;
            padding-top: 5px;
            margin-top: 2px;
            transition: all 0.2s;
            flex-direction: column;
            transform-origin: bottom center;
            box-shadow: 0 5px 15px 0px #00000033;
            background-color: var(--dark-background4,#fff);
            border-radius: 2px;
            opacity: 0;
            z-index: 99;
            visibility: hidden;
        }
        :host([placement="bottom"]) .body{
            bottom:unset;
            top: 100%;
            transition: none;
            transform: none;
        }
        .body-bottom{
            top: 100%;
            transform-origin: top center;
            bottom: auto;
        }
        .multipleRoot input::-webkit-input-placeholder {
            color: var(--dark-color,#aab2bd);
        }
        :host([disabled]) {
           pointer-events: none;
           cursor: not-allowed;
           background-color: var(--dark-background1,#f5f5f5);
        }
        </style>
        <div class="multipleSelect" tabindex="0">
            <div class="multipleRoot" id="select" style="width:100%">
            <input id="singleInput" placeholder="${this.placeholder}"/>
        </div>
            <lit-icon class="icon" name='down' color="#c3c3c3"></lit-icon>
        </div>
        <div class="body body-bottom">
            <slot></slot>
            <slot name="footer"></slot>
        </div>
        `;
  }

  connectedCallback() {}

  initData() {
    this.selectAllocationInputEl = this.shadowRoot!.querySelector('input');
    this.selectAllocationOptions = this.shadowRoot!.querySelector('.body') as HTMLDivElement;
    this.selectAllocationInputEl?.addEventListener('input', () => {
      let filter = [...this.shadowRoot!.querySelectorAll<HTMLDivElement>('.option')].filter((a: HTMLDivElement) => {
        if (a.textContent!.indexOf(this.selectAllocationInputEl!.value) <= -1) {
          a.style.display = 'none';
        } else {
          a.style.display = 'block';
        }
      });
      this.value = this.selectAllocationInputEl!.value;
      this.selectAllocationInputContent!.dispatchEvent(new CustomEvent('valuable', {}));
    });
    this.shadowRoot?.querySelectorAll('.option').forEach((a) => {
      a.addEventListener('mousedown', (e) => {
        a.dispatchEvent(
          new CustomEvent('onSelected', {
            detail: {
              selected: true,
              text: a.textContent,
            },
          })
        );
      });
      a.addEventListener('onSelected', (e: any) => {
        this.selectAllocationInputEl!.value = e.detail.text;
        this.value = e.detail.text;
        this.selectAllocationInputContent!.dispatchEvent(new CustomEvent('valuable', {}));
      });
    });
  }
}
