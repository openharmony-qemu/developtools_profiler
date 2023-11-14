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
import { ColorUtils } from './ColorUtils.js';
import { LitRadioBox } from '../../../../base-ui/radiobox/LitRadioBox.js';
import { SpApplication } from '../../../SpApplication.js';
import { SpSystemTrace } from '../../SpSystemTrace.js';

@element('custom-theme-color')
export class CustomThemeColor extends BaseElement {
  private application: SpApplication | undefined | null;
  private radios: NodeListOf<LitRadioBox> | undefined | null;
  private colorsArray: Array<string> = [];
  private colorsEl: HTMLDivElement | undefined | null;
  private theme: Theme = Theme.LIGHT;
  private systemTrace: SpSystemTrace | undefined | null;

  static get observedAttributes(): string[] {
    return ['mode'];
  }

  init(): void {
    window.localStorage.getItem('Theme') === 'light' ? (this.theme = Theme.LIGHT) : (this.theme = Theme.DARK);
    if (window.localStorage.getItem('Theme') === 'light' || !window.localStorage.getItem('Theme')) {
      this.theme = Theme.LIGHT;
    } else {
      this.theme = Theme.DARK;
    }
    this.application!.changeTheme(this.theme);
    this.setRadioChecked(this.theme);
  }

  /**
   * 更新色板
   * @param colorsEl 色板的父元素
   */
  createColorsEl(colorsEl: HTMLDivElement) {
    for (let i = 0; i < this.colorsArray!.length; i++) {
      let div = document.createElement('div');
      div.className = 'color-wrap';
      let input = document.createElement('input');
      input.type = 'color';
      input.className = 'color';
      input.value = this.colorsArray![i];
      div.appendChild(input);
      colorsEl?.appendChild(div);
      input.addEventListener('change', (evt: any) => {
        input.value = evt?.target.value;
        this.colorsArray![i] = evt?.target.value;
      });
    }
  }

  /**
   * 根据传入的主题改变color setting页面的单选框状态，更新颜色数组
   * @param theme 主题模式
   */
  setRadioChecked(theme: Theme) {
    for (let i = 0; i < this.radios!.length; i++) {
      if (this.radios![i].innerHTML === theme) {
        this.radios![i].setAttribute('checked', '');
        if (theme === Theme.LIGHT) {
          this.colorsArray =
            window.localStorage.getItem('LightThemeColors') === null
              ? [...ColorUtils.FUNC_COLOR_A]
              : JSON.parse(window.localStorage.getItem('LightThemeColors')!);
        } else {
          this.colorsArray =
            window.localStorage.getItem('DarkThemeColors') === null
              ? [...ColorUtils.FUNC_COLOR_B]
              : JSON.parse(window.localStorage.getItem('DarkThemeColors')!);
        }
      } else {
        this.radios![i].removeAttribute('checked');
      }
    }
    this.colorsEl!.innerHTML = '';
    this.createColorsEl(this.colorsEl!);
  }

  initElements(): void {
    this.radios = this.shadowRoot?.querySelectorAll('.litRadio');
    this.colorsEl = this.shadowRoot?.querySelector('.colors') as HTMLDivElement;
    this.application = document.querySelector('body > sp-application') as SpApplication;
    this.systemTrace = this.application.shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    let close = this.shadowRoot?.querySelector('.page-close');
    if (this.radios) {
      for (let i = 0; i < this.radios.length; i++) {
        this.radios![i].shadowRoot!.querySelector<HTMLSpanElement>('.selected')!.classList.add('blue');
        this.radios[i].addEventListener('click', (evt) => {
          // 点击颜色模式的单选框，色板切换
          if (this.radios![i].innerHTML === Theme.LIGHT) {
            if (this.radios![i].getAttribute('checked') === null) {
              this.colorsArray =
                window.localStorage.getItem('LightThemeColors') === null
                  ? [...ColorUtils.FUNC_COLOR_A]
                  : JSON.parse(window.localStorage.getItem('LightThemeColors')!);
              this.theme = Theme.LIGHT;
            } else {
              return;
            }
          } else if (this.radios![i].innerHTML === Theme.DARK) {
            if (this.radios![i].getAttribute('checked') === null) {
              this.colorsArray =
                window.localStorage.getItem('DarkThemeColors') === null
                  ? [...ColorUtils.FUNC_COLOR_B]
                  : JSON.parse(window.localStorage.getItem('DarkThemeColors')!);
              this.theme = Theme.DARK;
            } else {
              return;
            }
          }
          this.colorsEl!.innerHTML = '';
          this.createColorsEl(this.colorsEl!);
          this.confirmOPerate();
        });
      }
    }

    close!.addEventListener('click', (ev) => {
      if (this.application!.hasAttribute('custom-color')) {
        this.application!.removeAttribute('custom-color');
        this.setAttribute('hidden', '');
      }
      this.cancelOperate();
    });
    let resetBtn = this.shadowRoot?.querySelector<HTMLButtonElement>('#reset');
    let previewBtn = this.shadowRoot?.querySelector<HTMLButtonElement>('#preview');
    let confirmBtn = this.shadowRoot?.querySelector<HTMLButtonElement>('#confirm');

    resetBtn?.addEventListener('click', () => {
      if (this.theme === Theme.LIGHT) {
        window.localStorage.setItem('LightThemeColors', JSON.stringify(ColorUtils.FUNC_COLOR_A));
      } else {
        window.localStorage.setItem('DarkThemeColors', JSON.stringify(ColorUtils.FUNC_COLOR_B));
      }
      this.application!.changeTheme(this.theme);
    });

    previewBtn?.addEventListener('click', () => {
      this.application!.changeTheme(this.theme, [...this.colorsArray]);
    });

    confirmBtn?.addEventListener('click', () => {
      this.confirmOPerate();
    });
    // 鼠标移入该页面，cpu泳道图恢复鼠标移出状态（鼠标移入cpu泳道图有数据的矩形上，和该矩形的tid或者pid不同的矩形会变灰，移出矩形，所有矩形恢复颜色）
    this.addEventListener('mousemove', (event) => {
      this.systemTrace!.tipEL!.style.display = 'none';
      this.systemTrace!.hoverStructNull();
      this.systemTrace!.refreshCanvas(true);
    });
  }

  confirmOPerate() {
    window.localStorage.setItem('Theme', this.theme);
    if (this.theme === Theme.LIGHT) {
      window.localStorage.setItem('LightThemeColors', JSON.stringify([...this.colorsArray]));
    } else {
      window.localStorage.setItem('DarkThemeColors', JSON.stringify([...this.colorsArray]));
    }
    this.application!.changeTheme(this.theme);
    this.setRadioChecked(this.theme);
  }

  cancelOperate() {
    if (window.localStorage.getItem('Theme') === 'light' || !window.localStorage.getItem('Theme')) {
      this.theme = Theme.LIGHT;
      this.colorsArray =
        window.localStorage.getItem('LightThemeColors') === null
          ? [...ColorUtils.FUNC_COLOR_A]
          : JSON.parse(window.localStorage.getItem('LightThemeColors')!);
    } else if (window.localStorage.getItem('Theme') === 'dark') {
      this.theme = Theme.DARK;
      this.colorsArray =
        window.localStorage.getItem('DarkThemeColors') === null
          ? [...ColorUtils.FUNC_COLOR_B]
          : JSON.parse(window.localStorage.getItem('DarkThemeColors')!);
    }
    this.application!.changeTheme(this.theme);
    // 恢复颜色模式单选框checked状态
    this.setRadioChecked(this.theme);
  }

  connectedCallback(): void {}

  initHtml(): string {
    return `
        <style>
        :host([hidden]) {
            visibility: hidden;
        }
        :host {
            width:100%;
            visibility: visible;
            overflow: auto;
            background-color: #fff;
            display: flex;
            flex-direction: column;
        }
        .config-title {
            height: 72px;
            background-color: #0a59f7;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        .title-text {
            font-family: Helvetica-Bold;
            font-size: 16px;
            color: #ffffff;
            text-align: left;
            font-weight: 700;
            margin-left: 40px;
        }
        .page-close {
            text-align: right;
            cursor: pointer;
            opacity: 1;
            font-size: 24px;
            margin-right: 20px;
        }
        .page-close:hover {
            opacity: 0.7;
        }
        .theme {
            opacity: 0.9;
            font-family: Helvetica-Bold;
            font-size: 16px;
            color: #000000;
            line-height: 28px;
            font-weight: 700;
            margin: 60px 40px 40px 40px;
            display: flex;
            flex-direction: row;
            align-items: center;
            justify-content: flex-start;
            flex-flow: row wrap;
        }
        .theme span {
            margin-right: 40px;
            font-weight: 700;
        }
        .litRadio {
            margin: 0px 40px 0px 0px;
        }
        #lightRadio([dis=round]) #lightRadio(:focus-within) .selected label:hover .selected{
            border-color: #0a59f7;
        }
        #lightRadio([dis=round]) .selected::before {
            background: #0a59f7;
        }
        .describe {
            font-family: Helvetica;
            color: #000000;
            line-height: 28px;
            margin: 0px 0px 40px 40px;
        }
        .describe text:nth-child(1) {
            opacity: 0.9;
            font-size: 16px;
            font-weight: 700;
        }
        .describe text:nth-child(2) {
            opacity: 0.6;
            font-size: 14px;
            font-weight: 400;
            margin-left: 12px;
        }
        .colors {
            width: 50%;
            display: flex;
            flex-flow: row wrap;
            align-content: space-around;
            flex: 0 0 9%;
            margin: 20px auto;
        }
        .color-wrap {
            width: 32px;
            height: 32px;
            border-radius: 50%;
            margin: 30px 10px;
            overflow: hidden;
            position: relative;
        }
        .color {
            border: none;
            outline: none;
            width: 150%;
            height: 150%;
            padding: 0;
            border-radius: 50%;
            position: absolute;
            top: -25%;
            left: -25%;
        }
        .btns {
            width: 60%;
            max-width: 70%;
            display: flex;
            flex: 0 0 9%;
            justify-content: space-around;
            margin: 40px auto;
        }
        .btn {
            width: 96px;
            height: 32px;
            font-size:14px;
            text-align: center;
            line-height: 20px;
            color: #0a59f7;
            background-color: #fff;
            border-radius: 16px;
            border: 1px solid #0a59f7; 
        }
        .btn:hover {
            background-color: #0a59f7;
            color: #fff;
        }
        button.active {
            background-color: blue;
            color: white;
        }
        </style>
        <div class="container">
         <div class="config-title">
            <span class="title-text">Color Setting</span>
            <lit-icon class="page-close" name="close-light" title="Page Close" color='#fff'></lit-icon>
         </div>
         <div class="text-wrap">
            <div class="theme">
               <span>Appearance</span>
               <lit-radio name='litRadio' dis="round" class='litRadio' id="lightRadio" type="0">${Theme.LIGHT}</lit-radio>
               <lit-radio name='litRadio' dis="round" class='litRadio' id="darkRadio type="1">${Theme.DARK}</lit-radio>
            </div>
            <div class="describe">
               <text>Color Customization</text>
               <text> Please customize colors according to your preferences</text>
            </div>
         </div>
         <div class="colors">
         </div>
         <div class="btns">
            <button class="btn" id='reset'>Reset</button>
            <button class="btn" id='preview'>Preview</button>
            <button class="btn" id='confirm'>Confirm</button>
         </div>
        </div>
    `;
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {
    if (name === 'mode' && newValue === '') {
      this.init();
    }
  }
}
export enum Theme {
  LIGHT = 'light',
  DARK = 'dark',
}
