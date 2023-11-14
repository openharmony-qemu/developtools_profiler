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

import { BaseElement, element } from '../../../../base-ui/BaseElement.js';
import '../../../../base-ui/BaseElement.js';
import '../../../../base-ui/icon/LitIcon.js';

@element('collapse-button')
export default class CollapseButton extends BaseElement {
  static get observedAttributes() {
    return [
      'expand', //展开
    ];
  }

  set expand(value: boolean) {
    if (value) {
      this.setAttribute('expand', '');
    } else {
      this.removeAttribute('expand');
    }
  }

  get expand() {
    return this.hasAttribute('expand');
  }

  initElements(): void {
    this.onclick = (e) => {
      this.expand = !this.expand;
      window.publish(window.SmartEvent.UI.CollapseAllLane, this.expand);
    };
  }

  initHtml(): string {
    return `
<style>
:host{
    position: absolute;
    left: 0;
    bottom: 0;
}
:host div{
    display: flex;
    padding: 0 6px;
    /*background-color: #00a3f5;*/
}
:host(:not([expand])) div{
    flex-direction: column;
}
:host([expand]) div{
    flex-direction: column-reverse;
}
div:hover{
    cursor: pointer;
}
</style>
<div>
    <lit-icon name="up" size="12"></lit-icon>
    <lit-icon name="down" size="12"></lit-icon>
</div>`;
  }
}
