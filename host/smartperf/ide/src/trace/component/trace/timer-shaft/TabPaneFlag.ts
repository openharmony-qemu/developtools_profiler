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
import { Flag } from './Flag.js';
import { ns2s } from '../TimerShaftElement.js';

@element('tabpane-flag')
export class TabPaneFlag extends BaseElement {
  private flagListIdx: number | null = null;
  private flag: Flag | null = null;

  initElements(): void {
    this.shadowRoot?.querySelector('#color-input')?.addEventListener('change', (event: any) => {
      if (this.flag) {
        this.flag.color = event?.target.value;
        document.dispatchEvent(new CustomEvent('flag-change', { detail: this.flag }));
      }
    });
    this.shadowRoot?.querySelector('#text-input')?.addEventListener('keyup', (event: any) => {
      event.stopPropagation();
      if (event.keyCode == '13') {
        if (this.flag) {
          (window as any).flagInputFocus = false;
          window.publish(window.SmartEvent.UI.KeyboardEnable, {
            enable: true,
          });
          this.flag.text = event?.target.value;
          document.dispatchEvent(
            new CustomEvent('flag-change', {
              detail: this.flag,
            })
          );
        }
      }
    });
    this.shadowRoot?.querySelector('#text-input')?.addEventListener('blur', (event: any) => {
      (window as any).flagInputFocus = false;
      window.publish(window.SmartEvent.UI.KeyboardEnable, {
        enable: true,
      });
    });
    this.shadowRoot?.querySelector('#text-input')?.addEventListener('focus', (event: any) => {
      (window as any).flagInputFocus = true;
      window.publish(window.SmartEvent.UI.KeyboardEnable, {
        enable: false,
      });
    });
    this.shadowRoot?.querySelector('#remove-flag')?.addEventListener('click', (event: any) => {
      if (this.flag) {
        this.flag.hidden = true;
        document.dispatchEvent(new CustomEvent('flag-change', { detail: this.flag }));
      }
    });
  }

  setFlagObj(flagObj: Flag) {
    this.flag = flagObj;
    this.shadowRoot!.querySelector<HTMLInputElement>('#color-input')!.value = flagObj.color;
    this.shadowRoot!.querySelector<HTMLInputElement>('#text-input')!.value = flagObj.text;
    this.shadowRoot!.querySelector<HTMLDivElement>('#flag-time')!.innerHTML = ns2s(flagObj.time);
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        .flag-notes-editor-panel{
        display: flex;align-items: center
        }
        .flag-text{
        font-size: 14px;color: var(--dark-color1,#363636c7);font-weight: 300;
        }
        .flag-input{
            border-radius: 4px;
            border: 1px solid var(--dark-border,#dcdcdc);
            color: var(--dark-color1,#212121);
            background: var(--dark-background5,#FFFFFF);
            padding: 3px;
            margin: 0 10px;
        }
        .flag-input:focus{
            outline: none;
            box-shadow: 1px 1px 1px var(--bark-prompt,#bebebe);
        }
        .flag-notes-editor-panel button {
            border: none;
            cursor: pointer;
            outline: inherit;
            
            background: var(--dark-border1,#262f3c);
            color: white;
            border-radius: 10px;
            font-size: 10px;
            line-height: 18px;
            height: 22px;
            min-width: 7em;
            margin: auto 0 auto 1rem;
        }
        </style>
        <div class="flag-notes-editor-panel">
            <div class="flag-text">Annotation at <span id="flag-time"></span></div>
            <input style="flex: 1" class="flag-input" type="text" id="text-input"/>
            <span class="flag-text">Change color: <input style="background: var(--dark-background5,#FFFFFF);" type="color" id="color-input"/></span>
            <button id="remove-flag">Remove</button>
        </div>
        `;
  }
}
