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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import { LitCheckBox, LitCheckBoxChangeEvent } from '../../../base-ui/checkbox/LitCheckBox.js';

@element('check-des-box')
export class SpCheckDesBox extends BaseElement {
  private _checkBox: LitCheckBox | undefined;
  private _des: HTMLSpanElement | undefined;

  static get observedAttributes() {
    return ['checked', 'value', 'des', 'disabled'];
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

  set des(des: string) {
    this.setAttribute('des', des);
  }

  get value(): string {
    return this.getAttribute('value') || '';
  }

  set value(value: string) {
    this.setAttribute('value', value);
    this._checkBox!.value = value;
  }

  get checked() {
    return this.getAttribute('checked') != null;
  }

  set checked(checked: boolean) {
    if (checked) {
      this.setAttribute('checked', 'true');
      this._checkBox!.checked = true;
    } else {
      this.removeAttribute('checked');
      this._checkBox!.checked = false;
    }
  }

  initElements(): void {
    this._checkBox = this.shadowRoot?.getElementById('checkBox') as LitCheckBox;
    this._des = this.shadowRoot?.getElementById('des') as HTMLSpanElement;
  }

  initHtml(): string {
    return `
<style>
.check-des{
    opacity: 0.6;
    font-family: Helvetica;
    font-size: 1em;
    color: var(--dark-color,#000000);
    text-align: left;
    line-height: 20px;
    font-weight: 400;
}
lit-check-box {
    margin-bottom: 10px;
}
#des-con{
  margin-left: 30px;
}
:host([disabled]){
  pointer-events: none;
}
</style>
<lit-check-box id="checkBox"></lit-check-box>
<div id="des-con">
    <span id="des" class="check-des"></span>
</div>`;
  }

  public connectedCallback() {
    this._checkBox?.addEventListener('change', (ev: CustomEventInit<LitCheckBoxChangeEvent>) => {
      let detail = ev.detail;
      this.checked = detail!.checked;
      this.dispatchEvent(new CustomEvent('onchange', { detail }));
    });
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name == 'checked') {
      this._checkBox!.checked = newValue !== null;
    }
    if (name == 'value') {
      this._checkBox!.value = newValue;
    }
    if (name == 'des') {
      this._des!.textContent = newValue;
    }
  }
}

export interface checkDesBean {
  value: string;
  isSelect: boolean;
  des: string;
}
