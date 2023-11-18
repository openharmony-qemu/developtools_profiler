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
import { LitPopContent } from './LitPopContent.js';
import { LitPopoverTitle } from './LitPopoverTitle.js';
import { LitRadioGroup } from '../radiobox/LitRadioGroup.js';
import { LitRadioBox } from '../radiobox/LitRadioBox.js';
import { LitCheckBox } from '../checkbox/LitCheckBox.js';
import { LitCheckGroup } from '../checkbox/LitCheckGroup.js';
import { LitCheckBoxWithText } from '../checkbox/LitCheckBoxWithText.js';

@element('lit-popover')
export class LitPopover extends BaseElement {
  private popContent: LitPopContent | null | undefined;
  private litGroup: LitRadioGroup | LitCheckGroup | undefined;
  private _texBox: LitCheckBoxWithText | undefined;

  static get observedAttributes() {
    return [];
  }

  get type() {
    return this.getAttribute('type') || '';
  }

  set type(type: string) {
    this.setAttribute('type', type);
  }

  get title() {
    return this.getAttribute('title') || '';
  }

  set title(title: string) {
    this.setAttribute('title', title);
  }

  get limit(): LimitText {
    if (this._texBox?.checked) {
      return {
        textLowerLimit: this._texBox.lowerLimit,
        textUpperLimit: this._texBox.upLimit,
      };
    }
    return { textLowerLimit: '', textUpperLimit: '' };
  }

  set dataSource(dataSource: Array<SelectBean>) {
    this.popContent = this.querySelector<LitPopContent>('lit-pop-content');
    if (!this.popContent) {
      this.popContent = new LitPopContent();
      this.appendChild(this.popContent);
    }
    switch (this.type) {
      case 'multiple':
        this.litGroup = new LitCheckGroup();
        this.litGroup.setAttribute('layout', 'dispersion');
        this.popContent!.appendChild(this.litGroup);
        dataSource.forEach((data) => {
          let litCheckBox = new LitCheckBox();
          this.litGroup?.appendChild(litCheckBox);
          if (data.isSelected) {
            litCheckBox.setAttribute('checked', 'true');
          }
          litCheckBox.setAttribute('value', data.text);
        });
        break;
      case 'radio':
        this.litGroup = new LitRadioGroup();
        if (this.title !== '') {
          let title = new LitPopoverTitle();
          title.setAttribute('title', this.title || '');
          this.popContent!.appendChild(title);
          this.litGroup.setAttribute('layout', 'compact');
        } else {
          this.litGroup.setAttribute('layout', 'dispersion');
        }
        this.popContent!.appendChild(this.litGroup);
        dataSource.forEach((data) => {
          let litRadioBox = new LitRadioBox();
          if (this.title == '') {
            litRadioBox.setAttribute('dis', 'round');
          } else {
            litRadioBox.setAttribute('dis', 'check');
          }
          if (data.isSelected) {
            litRadioBox.setAttribute('checked', 'true');
          }
          this.litGroup?.appendChild(litRadioBox);
          litRadioBox.setAttribute('value', data.text);
        });
        break;
      case 'multiple-text':
        dataSource.forEach((data) => {
          this._texBox = new LitCheckBoxWithText();
          this._texBox.setAttribute('text', data.text);
          this._texBox.setAttribute('checked', '');
          this.popContent!.appendChild(this._texBox);
        });
        break;
      case 'data-ming':
        break;
    }
  }

  get select(): Array<string> | undefined {
    if (this._texBox?.checked) {
      return [this._texBox!.text];
    }
    return this.litGroup?.value;
  }

  get trigger() {
    return this.getAttribute('trigger');
  }

  get direction() {
    return this.getAttribute('direction') || 'topright';
  }

  set direction(value: string) {
    this.setAttribute('direction', value);
  }

  get open() {
    return this.getAttribute('open') !== null;
  }

  set open(value: boolean) {
    if (value === null || value === false) {
      this.removeAttribute('open');
    } else {
      this.setAttribute('open', '');
    }
  }

  initElements(): void {}

  initHtml(): string {
    return `
        <style>
        :host {
            display:inline-block;
            position:relative;
            overflow:visible;
        }
        :host([direction="top"]) ::slotted(lit-pop-content){
            bottom:100%;
            left:50%;
            transform:translate(-50%,-10px) scale(0);
            transform-origin: center bottom;
        }
        :host([direction="top"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            left: 50%;
            border-top: 10px solid #FFF;
            border-right: 10px solid transparent;
            border-left: 10px solid transparent;

        }
        :host([direction="top"]) ::slotted(lit-pop-content[open]),
        :host([direction="top"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="top"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-50%,-10px) scale(1);
        }
        
        :host([direction="bottom"]) ::slotted(lit-pop-content){
            top:100%;
            left:50%;
            transform:translate(-50%,10px) scale(0);
            transform-origin: center top;
        }
        :host([direction="bottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 100%;
            left: 50%;
            border-bottom: 10px solid #FFF; 
            border-right: 10px solid transparent;
            border-left: 10px solid transparent;
        }
        :host([direction="bottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-50%,10px) scale(1);
        }
        
        
        :host([direction="left"]) ::slotted(lit-pop-content){
            right:100%;
            top:50%;
            transform:translate(-10px,-50%) scale(0);
            transform-origin: right;
        }
        :host([direction="left"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 40%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
            border-top: 10px solid transparent;
        }
        :host([direction="left"]) ::slotted(lit-pop-content[open]),
        :host([direction="left"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="left"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px,-50%) scale(1);
        }
        :host([direction="right"]) ::slotted(lit-pop-content){
            left:100%;
            top:50%;
            transform:translate(10px,-50%) scale(0);
            transform-origin: left;
        }
        :host([direction="right"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 40%;
            right: 100%;
            border-right: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
            border-top: 10px solid transparent;
        }
        :host([direction="right"]) ::slotted(lit-pop-content[open]),
        :host([direction="right"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="right"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px,-50%) scale(1);
        }
        
        
        
        :host([direction="leftbottom"]) ::slotted(lit-pop-content){
            right:100%;
            top:0;
            transform:translate(-10px) scale(0);
        }
        :host([direction="leftbottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="leftbottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="leftbottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px) scale(1);
        }
        
        :host([direction="leftbottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            top: 10%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
        }
        
        :host([direction="lefttop"]) ::slotted(lit-pop-content){
            right:100%;
            bottom:0;
            transform:translate(-10px) scale(0);
            transform-origin: right bottom;
        }
        :host([direction="lefttop"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 10%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-top: 10px solid transparent;
        }
        :host([direction="lefttop"]) ::slotted(lit-pop-content[open]),
        :host([direction="lefttop"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="lefttop"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px) scale(1);
        }
        :host([direction="topright"]) ::slotted(lit-pop-content){
            bottom:100%;
            left:50%;
            transform:translate(0,-10px) scale(0);
            transform-origin: left bottom;
        }
        :host([direction="topright"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            left: 0%;
            border-top: 10px solid #FFF;
            border-right: 10px solid transparent;
        }
        :host([direction="topright"]) ::slotted(lit-pop-content[open]),
        :host([direction="topright"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="topright"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,-10px) scale(1);
        }
        
        
        :host([direction="topleft"]) ::slotted(lit-pop-content){
            bottom:100%;
            right:50%;
            transform:translate(0,-10px) scale(0);
            transform-origin: right bottom;
        }
        :host([direction="topleft"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            right: 0%;
            border-top: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="topleft"]) ::slotted(lit-pop-content[open]),
        :host([direction="topleft"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="topleft"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,-10px) scale(1);
        }
        
       
        :host([direction="rightbottom"]) ::slotted(lit-pop-content){
            left:100%;
            top:0;
            transform:translate(10px) scale(0);
            transform-origin: left top;
        }
        :host([direction="rightbottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 10%;
            right: 100%;
            border-top: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="rightbottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="rightbottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="rightbottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px) scale(1);
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content){
            left:100%;
            bottom:0;
            transform:translate(10px) scale(0);
            transform-origin: left bottom;
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content[open]),
        :host([direction="righttop"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="righttop"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px) scale(1);
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            bottom: 10%;
            right: 100%;
            border-bottom: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        
        :host([direction="bottomright"]) ::slotted(lit-pop-content),
        :host(:not([direction])) ::slotted(lit-pop-content){
            left:0;
            top:100%;
            transform:translate(0,10px) scale(0);
            transform-origin: left top;
        }
        :host([direction="bottomright"]) ::slotted(lit-pop-content)::after,
        :host(:not([direction])) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            left: 10%;
            bottom: 100%;
            border-bottom: 10px solid #FFF;
            border-right: 10px solid transparent;
        }
        :host(:not([direction])) ::slotted(lit-pop-content[open]),
        :host(:not([direction])[trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host(:not([direction])[trigger="focus"]:focus-within) ::slotted(lit-pop-content),
        :host([direction="bottomright"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottomright"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottomright"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,10px) scale(1);
        }
        
        :host([direction="bottomleft"]) ::slotted(lit-pop-content){
            right:0;
            top:100%;
            transform:translate(0,10px) scale(0);
            transform-origin: right top;
        }
        :host([direction="bottomleft"]) ::slotted(lit-pop-content)::after,
        :host(:not([direction])) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            right: 10%;
            bottom: 100%;
            border-bottom: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="bottomleft"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottomleft"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottomleft"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,10px) scale(1);
        }
        
        :host ::slotted(lit-pop-content[open]),
        :host([trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            opacity:1;
            visibility:visible;
        }
        slot{
        }
        </style>
        <slot></slot>
      `;
  }

  connectedCallback() {
    if (!(this.trigger && this.trigger !== 'click')) {
      this.addEventListener('click', () => {
        this.popContent = this.querySelector<LitPopContent>('lit-pop-content');
        if (!this.popContent) {
          this.popContent = new LitPopContent();
          this.appendChild(this.popContent);
        }
        this.popContent?.setAttribute('open', 'true');
      });
    }
    document.addEventListener('mousedown', (ev) => {
      const path = ev.composedPath && ev.composedPath();
      if (
        // @ts-ignore
        this.popContent && !path.includes(this.popContent) && !path.includes(this.children[0]) && !path.includes(this.popContent)
      ) {
        this.popContent!.open = false;
      }
    });
  }
}

export interface SelectBean {
  text: string;
  isSelected: boolean;
  limitText?: LimitText;
}

export interface LimitText {
  textUpperLimit: string;
  textLowerLimit: string;
}

export interface Charge {
  text: string;
  isSelected: boolean;
}
