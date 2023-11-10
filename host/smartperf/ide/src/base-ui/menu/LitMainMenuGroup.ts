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

@element('lit-main-menu-group')
export class LitMainMenuGroup extends BaseElement {
  protected _collapsed: boolean | undefined;
  private groupNameEl: HTMLElement | null | undefined;
  private groupDescEl: HTMLElement | null | undefined;
  private group: HTMLElement | null | undefined;

  static get observedAttributes() {
    return ['title', 'describe', 'collapsed', 'nocollapse', 'radius'];
  }

  get collapsed(): boolean {
    return this.hasAttribute('collapsed');
  }

  set collapsed(value: boolean) {
    if (value) {
      this.setAttribute('collapsed', '');
    } else {
      this.removeAttribute('collapsed');
    }
  }

  get nocollapsed() {
    return this.hasAttribute('nocollapsed');
  }

  set nocollapsed(value: boolean) {
    if (value) {
      this.setAttribute('nocollapsed', '');
    } else {
      this.removeAttribute('nocollapsed');
    }
  }

  get radius() {
    return this.hasAttribute('radius');
  }

  initElements(): void {
    this.groupNameEl = this.shadowRoot?.querySelector('.group-name');
    this.groupDescEl = this.shadowRoot?.querySelector('.group-describe');
    this.group = this.shadowRoot?.querySelector('#group');
    this.group!.addEventListener('click', (e) => {
      if (this.nocollapsed) {
        return;
      }
      this.collapsed = !this.collapsed;
    });
  }

  initHtml(): string {
    return `
        <style>
        :host(:not([collapsed])){ 
            width: 248px;
            display: flex;
            background-color: var(--dark-background);
            cursor: pointer;
            flex-direction: column;
        }
        :host{
            user-select: none;
            transition: background-color .3s;
        }
        :host(:not([collapsed])) ::slotted(lit-main-menu-item){
            display: flex;
        }
        :host(:not([collapsed])) .group-describe{
            height: 0;
            visibility: hidden;
        }
        :host([collapsed]):hover){
            background-color: #FFFFFF;
        }
        :host([collapsed]){
            width: 248px;
            display: flex;
            flex-direction: column;
            cursor: pointer;
            background-color: var(--dark-background);
        }
        :host([collapsed]) .group-describe{
            height: auto;
            visibility: visible;
        }
        :host([radius]) {
            border-radius: 16px 0px 0px 16px ;
        }
        :host([collapsed]) ::slotted(lit-main-menu-item){
            display: none;
        }
        .group-name{
            font-size: 14px;
            font-family: Helvetica;
            color: #000;
            padding: 20px 24px 0px 24px;
            line-height: 16px;
            font-weight: 400;
            text-align: left;
        }
        .group-describe{
            color: #000;
            font-size: 0.6rem;
            padding: 4px 24px 20px 24px;
        }
        </style>
       <div id="group">
         <div class="group-name"></div>
         <div class="group-describe"></div>
       </div>
        <slot></slot>
        `;
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    switch (name) {
      case 'title':
        if (this.groupNameEl) this.groupNameEl.textContent = newValue;
        break;
      case 'describe':
        if (this.groupDescEl) this.groupDescEl.textContent = newValue;
        break;
    }
  }
}
