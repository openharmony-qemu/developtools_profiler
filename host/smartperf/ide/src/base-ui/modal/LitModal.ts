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

@element('lit-modal')
export class LitModal extends BaseElement {
  private headerTitleElement: HTMLElement | null | undefined;
  private headerElement: HTMLElement | null | undefined;
  private closeElement: HTMLElement | null | undefined;
  private cancelElement: HTMLElement | null | undefined;
  private okElement: HTMLElement | null | undefined;
  private modalElement: HTMLElement | null | undefined;
  private resizing: boolean = false;
  private down: boolean = false;
  private onmouseleaveMoveFunc: any;
  private onmouseupFunc: any;
  private onmousedownMoveFunc: any;
  private onmousedownFunc: any;
  private onmousemoveMoveFunc: any;
  private onmousemoveFunc: any;
  private onmouseupMoveFunc: any;
  static get observedAttributes() {
    return [
      'title', //标题
      'line', //body的线条
      'visible', //是否显示modal窗口
      'ok-text', //确定文本
      'cancel-text', //取消文本
      'moveable', //设置窗口是否可以鼠标拖动
      'resizeable', //窗口可改变大小
      'width', //modal宽度
    ];
  }

  get okText() {
    return this.getAttribute('ok-text') || '确定';
  }

  set okText(value) {
    this.setAttribute('ok-text', value);
  }

  get cancelText() {
    return this.getAttribute('cancel-text') || '取消';
  }

  set cancelText(value) {
    this.setAttribute('cancel-text', value);
  }

  get title() {
    return this.getAttribute('title') || '';
  }

  set title(value) {
    this.setAttribute('title', value);
  }

  get visible() {
    return this.hasAttribute('visible');
  }

  set visible(value) {
    if (value) {
      this.setAttribute('visible', '');
    } else {
      this.removeAttribute('visible');
    }
  }

  get width() {
    return this.getAttribute('width') || '500px';
  }

  set width(value) {
    this.setAttribute('width', value);
  }

  get resizeable() {
    return this.hasAttribute('resizeable');
  }

  set resizeable(value) {
    if (value) {
      this.setAttribute('resizeable', '');
    } else {
      this.removeAttribute('resizeable');
    }
  }

  get moveable() {
    return this.hasAttribute('moveable');
  }

  set moveable(value) {
    if (value) {
      this.setAttribute('moveable', '');
    } else {
      this.removeAttribute('moveable');
    }
  }

  set onOk(fn: any) {
    this.addEventListener('onOk', fn);
  }

  set onCancel(fn: any) {
    this.addEventListener('onCancel', fn);
  }

  initHtml(): string {
    return `
        <style>
        :host{ 
            display: flex;
            position: absolute;
            left: 0;
            right: 0;
            top: 0;
            bottom: 0px;
            z-index: 1000;
            overflow: auto;
            transition: all 0.3s;
        }
        :host([visible]){
            background-color: #00000066;
            transition: all .3s;
            opacity: 1;
            visibility: visible;
        }
        :host(:not([visible])){
            pointer-events: none;
            transition: all .3s;
            opacity: 0;
            visibility: hidden;
        }
       
        .modal{
            width: ${this.width};
            position: absolute;
            display: flex;
            flex-direction: column;
            overflow: auto;
            background-color: #fff;
            top: 100px;
            left: 50%;
            right: auto;
            transform-origin: left top;
            pointer-events: all;
            border-radius: 2px;
        }
        :host(:not([visible])) .modal{
            transition: transform .3s , opacity .3s,visibility .3s;
            opacity: 0;
            transform:scale(0.1) translate(-50%,50%) translateZ(0) skew(0deg);
            visibility: hidden;
         }
         :host([visible]) .modal{
            transition: transform .3s , opacity .3s ,visibility .3s;
            opacity: 1;
            transform: scale(1) translate(-50%,0%) translateZ(0) skew(0deg);
            visibility: visible;
            box-shadow: 0 0 20px #00000055;
         }
        .header{
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 15px 20px;
            font-size: 17px;
            font-weight: bold;
        }
        :host([moveable]) .header label{
            pointer-events: none;
        }
        :host([moveable]) .header:hover{
            background-color: #8c8c8c11;
        }
        :host([moveable]) .header{
            /*cursor: move;*/
        }
        .close-icon{
            color:#8c8c8c;
            cursor: pointer;
        }
        .close-icon:hover{
            color: #414141;
        }
        .footer{
            display: flex;
            align-items: center;
            justify-content: flex-end;
            padding: 8px 8px;
        }
        .footer lit-button{
            min-width: 70px;
            margin-right: 10px;
            cursor: pointer;
        }
        :host([line=false]){
            padding: 10px 20px;
            flex:1;
        }
        :host([line=true]) .body,
        :host(:not([line])) .body{
            border-top: 1px solid #f0f0f0;
            border-bottom: 1px solid #f0f0f0;
            padding: 10px 20px;
            flex:1;
        }
       
        </style>
        <div class="modal" title="">
            <div class="header">
                <label id="modal-title"></label>
                <lit-icon class="close-icon" name="close"></lit-icon>
            </div>
            <div class="body">
                <slot></slot>
            </div>
            <slot name="footer">
                <div class="footer">
                    <lit-button id="cancel">${this.cancelText}</lit-button>
                    <lit-button id="ok" type="primary">${this.okText}</lit-button>
                </div>
            </slot>
        </div>
        `;
  }

  //当 custom element从文档DOM中删除时，被调用。
  disconnectedCallback() {}

  //当 custom element被移动到新的文档时，被调用。
  adoptedCallback() {}

  //当 custom element增加、删除、修改自身属性时，被调用。
  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name === 'visible') {
      if (!newValue && this.modalElement) {
        this.modalElement.style.top = '100px';
        this.modalElement.style.left = '50%';
      }
    } else if (name === 'title' && this.headerTitleElement) {
      this.headerTitleElement.textContent = newValue;
    }
  }
}

if (!customElements.get('lit-modal')) {
  customElements.define('lit-modal', LitModal);
}
