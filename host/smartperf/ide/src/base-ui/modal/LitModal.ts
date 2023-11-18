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

  //当 custom element首次被插入文档DOM时，被调用。
  initElements(): void {
    this.headerTitleElement = this.shadowRoot!.querySelector<HTMLDivElement>('#modal-title');
    this.headerTitleElement!.textContent = this.title;

    this.headerElement = this.shadowRoot!.querySelector<HTMLDivElement>('.header');
    this.closeElement = this.shadowRoot!.querySelector<HTMLDivElement>('.close-icon');
    this.cancelElement = this.shadowRoot!.querySelector<HTMLDivElement>('#cancel');
    this.okElement = this.shadowRoot!.querySelector<HTMLDivElement>('#ok');
    this.closeElement!.onclick = (ev) => (this.visible = false);
    this.modalElement = this.shadowRoot!.querySelector<HTMLDivElement>('.modal');
    this.shadowRoot!.querySelector<HTMLDivElement>('.modal')!.onclick = (e) => {
      e.stopPropagation();
    };
    this.onclick = (ev) => {
      ev.stopPropagation();
      if (!this.resizeable) {
        this.visible = false;
      }
    };
    this.cancelElement!.onclick = (ev) => {
      this.dispatchEvent(new CustomEvent('onCancel', ev));
    };
    this.okElement!.onclick = (ev) => {
      this.dispatchEvent(new CustomEvent('onOk', ev));
    };
    if (this.moveable || this.resizeable) {
      if (this.resizeable) {
        //move
        let resizeWidth = 8;
        this.resizing = false;
        let srcResizeClientX = 0,
          srcResizeClientY = 0,
          srcResizeRect,
          srcResizeHeight = 0,
          srcResizeWidth = 0,
          srcResizeRight = 0,
          srcResizeLeft = 0,
          srcResizeTop = 0;
        let direction: string;
        this.onmousemoveFunc = (e: any) => {
          e.stopPropagation();
          srcResizeRect = this.modalElement!.getBoundingClientRect();
          if (
            e.clientX > srcResizeRect.left - resizeWidth &&
            e.clientX < srcResizeRect.left + resizeWidth &&
            e.clientY > srcResizeRect.top - resizeWidth &&
            e.clientY < srcResizeRect.top + resizeWidth
          ) {
            //left top
            this.style.cursor = 'nwse-resize';
            if (!this.resizing) direction = 'left-top';
          } else if (
            e.clientX > srcResizeRect.right - resizeWidth &&
            e.clientX < srcResizeRect.right + resizeWidth &&
            e.clientY > srcResizeRect.top - resizeWidth &&
            e.clientY < srcResizeRect.top + resizeWidth
          ) {
            //right top
            this.style.cursor = 'nesw-resize';
            if (!this.resizing) direction = 'right-top';
          } else if (
            e.clientX > srcResizeRect.left - resizeWidth &&
            e.clientX < srcResizeRect.left + resizeWidth &&
            e.clientY > srcResizeRect.bottom - resizeWidth &&
            e.clientY < srcResizeRect.bottom + resizeWidth
          ) {
            //left bottom
            this.style.cursor = 'nesw-resize';
            if (!this.resizing) direction = 'left-bottom';
          } else if (
            e.clientX > srcResizeRect.right - resizeWidth &&
            e.clientX < srcResizeRect.right + resizeWidth &&
            e.clientY > srcResizeRect.bottom - resizeWidth &&
            e.clientY < srcResizeRect.bottom + resizeWidth
          ) {
            //right bottom
            this.style.cursor = 'nwse-resize';
            if (!this.resizing) direction = 'right-bottom';
          } else if (e.clientX > srcResizeRect.left - resizeWidth && e.clientX < srcResizeRect.left + resizeWidth) {
            //left
            this.style.cursor = 'ew-resize';
            if (!this.resizing) direction = 'left';
          } else if (e.clientX < srcResizeRect.right + resizeWidth && e.clientX > srcResizeRect.right - resizeWidth) {
            //right
            this.style.cursor = 'ew-resize';
            if (!this.resizing) direction = 'right';
          } else if (e.clientY > srcResizeRect.top - resizeWidth && e.clientY < srcResizeRect.top + resizeWidth) {
            //top
            this.style.cursor = 'ns-resize';
            if (!this.resizing) direction = 'top';
          } else if (e.clientY < srcResizeRect.bottom + resizeWidth && e.clientY > srcResizeRect.bottom - resizeWidth) {
            //bottom
            this.style.cursor = 'ns-resize';
            if (!this.resizing) direction = 'bottom';
          } else {
            this.style.cursor = '';
            if (!this.resizing) direction = '';
          }
          if (this.resizing) {
            let offsetResizeY = e.clientY - srcResizeClientY;
            let offsetResizeX = e.clientX - srcResizeClientX;
            if (direction === 'bottom') {
              this.modalElement!.style.height = srcResizeHeight + offsetResizeY + 'px';
            } else if (direction === 'top') {
              this.modalElement!.style.top = srcResizeTop + offsetResizeY + 'px';
              this.modalElement!.style.height = srcResizeHeight - offsetResizeY + 'px';
            } else if (direction === 'right') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth + offsetResizeX + 'px';
            } else if (direction === 'left') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth - offsetResizeX + 'px';
            } else if (direction === 'left-top') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth - offsetResizeX + 'px';
              this.modalElement!.style.top = srcResizeTop + offsetResizeY + 'px';
              this.modalElement!.style.height = srcResizeHeight - offsetResizeY + 'px';
            } else if (direction === 'right-top') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth + offsetResizeX + 'px';
              this.modalElement!.style.top = srcResizeTop + offsetResizeY + 'px';
              this.modalElement!.style.height = srcResizeHeight - offsetResizeY + 'px';
            } else if (direction === 'left-bottom') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth - offsetResizeX + 'px';
              this.modalElement!.style.height = srcResizeHeight + offsetResizeY + 'px';
            } else if (direction === 'right-bottom') {
              this.modalElement!.style.left = srcResizeLeft + srcResizeWidth / 2 + offsetResizeX / 2 + 'px';
              this.modalElement!.style.width = srcResizeWidth + offsetResizeX + 'px';
              this.modalElement!.style.height = srcResizeHeight + offsetResizeY + 'px';
            }
          }
        };
        this.onmousedownFunc = (e: any) => {
          srcResizeRect = this.modalElement!.getBoundingClientRect();
          srcResizeClientX = e.clientX;
          srcResizeClientY = e.clientY;
          srcResizeHeight = srcResizeRect.height;
          srcResizeWidth = srcResizeRect.width;
          srcResizeRight = srcResizeRect.right;
          srcResizeLeft = srcResizeRect.left;
          srcResizeTop = srcResizeRect.top;
          if (
            e.clientX > srcResizeRect.left - resizeWidth &&
            e.clientX < srcResizeRect.left + resizeWidth &&
            e.clientY > srcResizeRect.top - resizeWidth &&
            e.clientY < srcResizeRect.top + resizeWidth
          ) {
            //left top
            this.resizing = true;
          } else if (
            e.clientX > srcResizeRect.right - resizeWidth &&
            e.clientX < srcResizeRect.right + resizeWidth &&
            e.clientY > srcResizeRect.top - resizeWidth &&
            e.clientY < srcResizeRect.top + resizeWidth
          ) {
            //right top
            this.resizing = true;
          } else if (
            e.clientX > srcResizeRect.left - resizeWidth &&
            e.clientX < srcResizeRect.left + resizeWidth &&
            e.clientY > srcResizeRect.bottom - resizeWidth &&
            e.clientY < srcResizeRect.bottom + resizeWidth
          ) {
            //left bottom
            this.resizing = true;
          } else if (
            e.clientX > srcResizeRect.right - resizeWidth &&
            e.clientX < srcResizeRect.right + resizeWidth &&
            e.clientY > srcResizeRect.bottom - resizeWidth &&
            e.clientY < srcResizeRect.bottom + resizeWidth
          ) {
            //right bottom
            this.resizing = true;
          } else if (e.clientX > srcResizeRect.left - resizeWidth && e.clientX < srcResizeRect.left + resizeWidth) {
            //left
            this.resizing = true;
          } else if (e.clientX < srcResizeRect.right + resizeWidth && e.clientX > srcResizeRect.right - resizeWidth) {
            //right
            this.resizing = true;
          } else if (e.clientY > srcResizeRect.top - resizeWidth && e.clientY < srcResizeRect.top + resizeWidth) {
            //top
            this.resizing = true;
          } else if (e.clientY < srcResizeRect.bottom + resizeWidth && e.clientY > srcResizeRect.bottom - resizeWidth) {
            //bottom
            this.resizing = true;
          } else {
            this.resizing = false;
          }
          if (this.resizing) document.body.style.userSelect = 'none';
        };
        this.onmouseupFunc = (e: any) => {
          this.resizing = false;
        };
      }

      if (this.moveable) {
        this.down = false;
        let srcClientX = 0;
        let srcClientY = 0;
        let srcLeft = 0;
        let srcTop = 0;
        let srcRight = 0;
        let srcBottom = 0;
        let clientRect;
        let rootRect: any;

        this.onmousedownMoveFunc = (e: any) => {
          if (this.resizing) return;
          srcClientX = e.clientX;
          srcClientY = e.clientY;
          rootRect = this.getBoundingClientRect();
          clientRect = this.modalElement!.getBoundingClientRect();
          srcLeft = clientRect.left;
          srcRight = clientRect.right;
          srcTop = clientRect.top;
          srcBottom = clientRect.bottom;
          if (
            e.clientX > srcLeft + 10 &&
            e.clientX < srcRight - 10 &&
            e.clientY > srcTop + 10 &&
            e.clientY < srcTop + this.headerElement!.scrollHeight
          ) {
            this.down = true;
          } else {
            this.down = false;
          }
          if (this.down) document.body.style.userSelect = 'none';
          this.onmousemoveMoveFunc = (ev: any) => {
            if (this.down) {
              let offsetY = e.clientY - srcClientY;
              let offsetX = e.clientX - srcClientX;
              if (e.clientX > srcLeft + 10 && e.clientX < srcRight - 10 && e.clientY > srcTop + 10) {
                this.headerElement!.style.cursor = 'move';
                clientRect = this.modalElement!.getBoundingClientRect();
                //下面 rootRect.height 改成 this.scrollHeight 解决modal 过长会出现滚动条的情况
                if (
                  ev.clientY - srcClientY + srcTop > 0 &&
                  ev.clientY - srcClientY + srcTop < this.scrollHeight - clientRect.height
                ) {
                  this.modalElement!.style.top = ev.clientY - srcClientY + srcTop + 'px';
                } else {
                  if (ev.clientY - srcClientY + srcTop <= 0) {
                    this.modalElement!.style.top = '0px';
                  } else {
                    //下面 rootRect.height 改成 this.scrollHeight 解决modal 过长会出现滚动条的情况
                    this.modalElement!.style.top = this.scrollHeight - clientRect.height + 'px';
                  }
                }
                //ev.clientX-srcClientX 鼠标移动像素
                if (
                  ev.clientX - srcClientX + srcLeft > 0 &&
                  ev.clientX - srcClientX + srcLeft < rootRect.width - clientRect.width
                ) {
                  this.modalElement!.style.left = ev.clientX - srcClientX + srcLeft + clientRect.width / 2 + 'px';
                } else {
                  if (ev.clientX - srcClientX + srcLeft <= 0) {
                    this.modalElement!.style.left = clientRect.width / 2 + 'px';
                  } else {
                    this.modalElement!.style.left = rootRect.width - clientRect.width + clientRect.width / 2 + 'px';
                  }
                }
              }
            }
          };
          this.onmouseleaveMoveFunc = this.onmouseupMoveFunc = (e: any) => {
            this.down = false;
            this.headerElement!.style.cursor = '';
          };
        };
      }
      this.onmousemove = (e) => {
        if (this.onmousemoveFunc) this.onmousemoveFunc(e);
        if (this.onmousemoveMoveFunc) this.onmousemoveMoveFunc(e);
      };
      this.onmousedown = (e) => {
        if (this.onmousedownFunc) this.onmousedownFunc(e);
        if (this.onmousedownMoveFunc) this.onmousedownMoveFunc(e);
      };
      this.onmouseleave = this.onmouseup = (e) => {
        if (this.onmouseleaveMoveFunc) this.onmouseleaveMoveFunc(e);
        if (this.onmouseupFunc) this.onmouseupFunc(e);
        document.body.style.userSelect = '';
      };
    }
  }

  //当 custom element从文档DOM中删除时，被调用。
  disconnectedCallback() {}

  //当 custom element被移动到新的文档时，被调用。
  adoptedCallback() {

  }

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
