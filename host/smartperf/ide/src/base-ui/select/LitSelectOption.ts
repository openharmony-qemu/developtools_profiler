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

import { BaseElement } from '../BaseElement.js';
import '../icon/LitIcon.js';

export class LitSelectOption extends BaseElement {
  static get observedAttributes() {
    return ['selected', 'disabled', 'check'];
  }

  initHtml() {
    return `
        <style>
        :host{ 
            display: flex;
            padding: 8px 10px;
            transition: all .3s;
            color: var(--dark-color2,#333);
            tab-index: -1;
            align-items: center;
            width: max-content;
            min-width: 100%;
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
        `;
  }

  initElements(): void {}

  //当 custom element首次被插入文档DOM时，被调用。
  connectedCallback() {
    if (!this.hasAttribute('disabled')) {
      this.onclick = (ev) => {
        this.dispatchEvent(
          new CustomEvent('onSelected', {
            detail: {
              selected: true,
              value: this.getAttribute('value'),
              text: this.textContent,
            },
          })
        );
        ev.stopPropagation();
      };
    }
  }

  //当 custom element从文档DOM中删除时，被调用。
  disconnectedCallback() {}

  //当 custom element被移动到新的文档时，被调用。
  adoptedCallback() {}

  //当 custom element增加、删除、修改自身属性时，被调用。
  attributeChangedCallback(name: any, oldValue: any, newValue: any) {}
}

if (!customElements.get('lit-select-option')) {
  customElements.define('lit-select-option', LitSelectOption);
}
