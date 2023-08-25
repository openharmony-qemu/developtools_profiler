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

import { BaseElement, element } from '../../BaseElement.js';

@element('pagination-box')
export class PaginationBox extends BaseElement {
  private paginationBoxPage: any;

  static get observedAttributes() {
    return ['text', 'height', 'width'];
  }

  get text() {
    return this.getAttribute('text') || '';
  }

  set text(text: string) {
    this.setAttribute('text', text);
  }

  get height() {
    return this.getAttribute('height') || '';
  }

  set height(height: string) {
    this.setAttribute('height', height);
  }

  initHtml(): string {
    return `
        <style>
        :host{ 
            width: 100%;
            height: 100%;
            display: block;
            position: relative;
            background: background: var(--dark-background3,#FFFFFF);
        }
        </style>
        <div class="pagination" id="box"></div>
            `;
  }

  initElements(): void {
    this.paginationBoxPage = this.shadowRoot?.querySelector('#box');
  }

  attributeChangedCallback(name: string, oldValue: string, value: string) {
    switch (name) {
    }
  }
}
