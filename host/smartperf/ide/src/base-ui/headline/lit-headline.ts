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

@element('lit-headline')
export class LitHeadLine extends BaseElement {
  private titleContent: string = '';
  private _isShow: Boolean = false;
  private headline: HTMLDivElement | null | undefined;
  private closeBtn: HTMLDivElement | null | undefined;
  private _titleContext: HTMLLabelElement | null | undefined;
  private _width: number = 0;
  public closeCallback = function (): void {};
  static get observedAttributes(): string[] {
    return ['_isShow', 'width', 'titleTxt'];
  }
  set isShow(value: Boolean) {
    this._isShow = value;
    if (value) {
      this.headline!.style.display = 'block';
    } else {
      this.headline!.style.display = 'none';
    }
  }
  get isShow(): Boolean {
    return this._isShow;
  }
  set titleTxt(value: string) {
    this.titleContent = value;
    this._titleContext!.innerHTML = value;
  }
  get titleTxt(): string {
    return this.titleContent || '';
  }
  connectedCallback(): void {}
  initElements(): void {
    this.closeBtn = this.shadowRoot?.querySelector('.icon-box');
    this.headline = this.shadowRoot?.querySelector('#headline');
    this._titleContext = this.shadowRoot?.querySelector('#titleContext');
    this.closeBtn!.addEventListener('click', () => {
      this.headline!.style.display = 'none';
      this.closeCallback();
    });
  }
  clear(): void {
    this.titleTxt = '';
    this.isShow = false;
  }

  initHtml(): string {
    return `
        <style>
        :host([show]) #headline{
          display: block;
        }
        :host(:not([show])) #headline{
          display: none;
        }
        #headline {
          width:${this.getAttribute('width') ? Number(this.getAttribute('width')) : '100%'};
        }
        .icon-box {
          background-color: var(--bark-expansion, #0c65d1);
          border-radius: 5px;
          color: #fff;
          margin: 0px 10px;
          width: 40px;
          height: 20px;
          text-align: center;
        }
        .icon {
          width: 16px;
          height: 16px;
          cursor: pointer;
          line-height: 16px;
          text-align: center;
          color: white;
        }
        #titleContext {
          flex: 1;
          overflow: hidden;
          text-overflow: ellipsis;
          white-space: nowrap;
        }
        </style>
        <div id="headline">
          <div style="display: flex">
            <div class="icon-box">
              <lit-icon class='icon' name="close-light"></lit-icon>
            </div>
            <label id="titleContext">${this.getAttribute('titleTxt') ? this.getAttribute('titleTxt') : ''}</label>
          </div>
        </div> 
        `;
  }
}
