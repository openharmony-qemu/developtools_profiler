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

@element('lit-main-menu-item')
export class LitMainMenuItem extends BaseElement {
  private titleEl: HTMLElement | null | undefined;
  private rootEL: HTMLElement | null | undefined;
  private iconEl: HTMLElement | null | undefined;
  private fileEL: HTMLInputElement | undefined | null;

  static get observedAttributes() {
    return ['title', 'icon', 'file', 'disabled'];
  }

  get title(): string {
    return this.getAttribute('title') || '';
  }

  set title(val: string) {
    this.setAttribute('title', val);
  }

  get disabled(): boolean {
    return this.hasAttribute('disabled');
  }

  set disabled(val: boolean) {
    if (val) {
      this.setAttribute('disabled', val.toString());
      this.fileEL?.setAttribute('disabled', val.toString());
    } else {
      this.removeAttribute('disabled');
      this.fileEL?.removeAttribute('disabled');
    }
  }

  get back(): boolean {
    return this.hasAttribute('back');
  }

  set back(isShowBack: boolean) {
    if (isShowBack) {
      this.setAttribute('back', '');
    } else {
      this.removeAttribute('back');
    }
  }

  initElements(): void {
    this.rootEL = this.shadowRoot?.querySelector('.root');
    this.titleEl = this.shadowRoot?.querySelector('.name');
    this.iconEl = this.shadowRoot?.querySelector('.icon');
    this.fileEL = this.shadowRoot?.querySelector('.file');
  }

  isFile(): boolean {
    if (this.hasAttribute('file')) {
      if (this.fileEL) {
        return true;
      }
    }
    return false;
  }

  connectedCallback() {
    if (this.hasAttribute('file')) {
      if (this.fileEL) {
        this.fileEL!.addEventListener('change', (event) => {
          let files = this.fileEL!.files;
          if (files && files.length > 0) {
            if (this.titleEl!.textContent!.includes('long trace')) {
              this.dispatchEvent(
                new CustomEvent('file-change', {
                  // @ts-ignore
                  target: this,
                  // @ts-ignore
                  detail: event.target!.files,
                })
              );
            } else {
              this.dispatchEvent(
                new CustomEvent('file-change', {
                  // @ts-ignore
                  target: this,
                  detail: files[0],
                })
              );
            }
            if (this.fileEL) this.fileEL.value = '';
            if (this.fileEL) {
              this.fileEL.value = '';
            }
          }
        });
      }
      this.addEventListener('click', (e) => {
        e.stopPropagation();
      });
    }
  }

  initHtml(): string {
    return `
        <style>
            :host{
                user-select: none;
                display: flex;
                font-family: Helvetica;
                font-size: 14px;
                color: #000;
                text-align: left;
                line-height: 20px;
                font-weight: 400
                background-color: #FFFFFF;
                transition: background-color .3s;
            }
            :host(:not([disabled]):hover){
                display: flex;
                background-color: var(--dark-background8,#6C9BFA);
                color: #FFFFFF;
                cursor: pointer;
            }
            :host([disabled]:hover){
                display: flex;
                /*background-color:#3391FF;*/
                /*color: #FFFFFF;*/
                cursor:not-allowed;
            }
            :host([disabled]) .root{
                cursor:not-allowed;
                display: flex;
                align-items: center;
                padding: 10px 24px;
                width: 100%;
            }
            :host(:not([disabled])) .root{
                cursor:pointer;
                display: flex;
                align-items: center;
                padding: 10px 24px;
                width: 100%;
            }
            .name{
                padding-left: 10px;
                cursor: pointer;
                overflow-wrap: anywhere;
            }
            .icon{
                pointer-events: none;
            }
            :host(:not([file])) .name{
                pointer-events: none;
            }
            :host(:not([file])) .root{
                pointer-events: none;
            }
            :host([file]) .name{
                pointer-events: none;
            }
            :host([file]) .icon{
                pointer-events: none;
            }
            
            :host([back]) {
                background-color: var(--dark-background8,#6C9BFA);
            } 
            
        </style>
        <input id="file" class="file" type="file" style="display:none;pointer-events: none" />
        <label class="root" for="file">
            <lit-icon class="icon" name="user" size="20"></lit-icon>
            <label class="name"></label>
        </label>
        `;
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    switch (name) {
      case 'title':
        if (this.titleEl) {
          this.titleEl.textContent = newValue;
          if (newValue.includes('long trace')) {
            this.fileEL!.setAttribute('multiple', '');
            this.fileEL!.setAttribute('webkitdirectory', '');
            this.fileEL!.setAttribute('directory', '');
          } else {
            if (this.fileEL!.hasAttribute('multiple')) {
              this.fileEL!.removeAttribute('multiple');
              this.fileEL!.removeAttribute('webkitdirectory');
              this.fileEL!.removeAttribute('directory');
            }
          }
        }
        break;
      case 'icon':
        if (this.iconEl) this.iconEl.setAttribute('name', newValue);
        break;
    }
  }
}
