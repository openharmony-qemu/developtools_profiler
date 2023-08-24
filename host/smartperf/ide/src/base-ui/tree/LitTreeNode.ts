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

import '../icon/LitIcon.js'
import '../checkbox/LitCheckBox.js'
import { BaseElement, element } from '../BaseElement.js';
import { LitCheckBox } from '../checkbox/LitCheckBox.js';
import { LitIcon } from '../icon/LitIcon.js';
import { TreeItemData } from './LitTree.js';

@element('lit-tree-node')
export class LitTreeNode extends BaseElement {

  private arrowElement: HTMLSpanElement | null | undefined;
  private itemElement: HTMLDivElement | null | undefined;
  private checkboxElement: LitCheckBox | null | undefined;
  private iconElement: LitIcon | null | undefined;
  private _data: TreeItemData | null | undefined;

  static get observedAttributes() {
    return ['icon-name', 'icon-size', 'color', 'path', 'title', 'arrow', 'checkable', 'selected', 'checked', 'missing', 'multiple', 'top-depth'];
  }

  get checkable() {
    return this.getAttribute('checkable') || 'false';
  }

  set data(value: TreeItemData | null | undefined) {
    this._data = value;
  }

  get data() {
    return this._data;
  }

  set checkable(value) {
    if (value === 'true') {
      this.setAttribute('checkable', 'true');
    } else {
      this.setAttribute('checkable', 'false');
    }
  }

  set multiple(value: boolean) {
    if (value) {
      this.setAttribute('multiple', '');
    } else {
      this.removeAttribute('multiple');
    }
  }

  get multiple() {
    return this.hasAttribute('multiple');
  }


  get iconName() {
    return this.getAttribute('icon-name') || '';
  }

  set iconName(value) {
    this.setAttribute('icon-name', value);
  }

  get topDepth() {
    return this.hasAttribute('top-depth');
  }

  set topDepth(value) {
    if (value) {
      this.setAttribute('top-depth', '');
    } else {
      this.removeAttribute('top-depth');
    }
  }

  get arrow() {
    return this.hasAttribute('arrow');
  }

  set arrow(value) {
    if (value) {
      this.setAttribute('arrow', 'true');
    } else {
      this.removeAttribute('arrow');
    }
  }

  get open() {
    return this.getAttribute('open') || 'true';
  }

  set open(value) {
    this.setAttribute('open', value);
  }

  get selected() {
    return this.hasAttribute('selected');
  }

  set selected(value) {
    if (value) {
      this.setAttribute('selected', '');
    } else {
      this.removeAttribute('selected');
    }
  }

  get checked() {
    return this.hasAttribute('checked');
  }

  set checked(value) {
    if (value === null || !value) {
      this.removeAttribute('checked');
    } else {
      this.setAttribute('checked', '');
    }
  }

  initElements(): void {
    this.arrowElement = this.shadowRoot!.querySelector<HTMLSpanElement>('#arrow');
    this.iconElement = this.shadowRoot!.querySelector<LitIcon>('#icon');
    this.itemElement = this.shadowRoot!.querySelector<HTMLDivElement>('#item');
    this.checkboxElement = this.shadowRoot!.querySelector<LitCheckBox>('#checkbox');
    this.arrowElement!.onclick = (e) => {
      e.stopPropagation();
      this.autoExpand();
    }
    this.checkboxElement!.onchange = (e: any) => {
      e.stopPropagation();
      this.onChange(e.detail.checked);
      return false;
    }
    //这里需要给checkbox 添加onclick时间 并停止冒泡，不然onchange事件会触发父节点中的 onclick事件
    this.checkboxElement!.onclick = (e) => {
      e.stopPropagation();
    };
    this.itemElement!.onclick = (e) => {
      e.stopPropagation();
      this.onChange(!this.data?.checked);
    };
  }

  onChange(checked: boolean) {
    this.checked = checked;
    this.data!.checked = checked;
    this.checkHandler();
    this.dispatchEvent(new CustomEvent('change', {detail: checked}));
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            margin: 0;
            align-items: center;
         }
         :host(:hover) #item{
            background-color: #f5f5f5;
            border-radius: 4px;
         }
         
         :host(:not([arrow]))  #arrow{
            display: none;
         }
         
         :host(:not([arrow]))  #item{
            margin-left: 15px;
         }
         
         :host([top-depth])  #item{
            margin-left: 0;
         }
         
         #title{
            padding: 4px 6px;
         }
         
         #arrow{
            width: 0;
            height: 0;
            border-top: 8px solid #262626;
            border-bottom: 0px solid transparent;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            transition: all .3s ;
            transform: translateX(-50%) rotateZ(0deg);
            margin-left: 5px;
         }
            
         #icon{
            display: none;
            margin-left: 5px;
         }
      
         /*画拖动辅助线*/
         #item[line-top]{
            position: relative;
            width: 100%;
         }
         
         #item[line-top]::before{
            content: '';
            position: absolute;
            top: 5px;
            left: 0;
            transform: translateY(-50%);
            width: 6px;
            height: 6px;
            overflow: visible;
            z-index: 999;
            background-color: #fff;
            border-radius: 6px;
            border: 2px solid #42b983;
         }
         #item[line-top]::after{
            content: '';
            overflow: visible;
            position: absolute;
            z-index: 999;
            top: 4px;
            left: 10px;
            width: 100%;
            height: 2px;
            background-color: #42b983;
         }
         
         #item[line-bottom]{
            position: relative;
            width: 100%;
         }
         #item[line-bottom]::before{
            content: '';
            position: absolute;
            bottom: 5px;
            left: 0;
            transform: translateY(50%);
            width: 6px;
            height: 6px;
            overflow: visible;
            z-index: 999;
            background-color: #fff;
            border-radius: 6px;
            border: 2px solid #42b983;
         }
         #item[line-bottom]::after{
            content: '';
            overflow: visible;
            position: absolute;
            z-index: 999;
            bottom: 4px;
            left: 10px;
            width: 100%;
            height: 2px;
            background-color: #42b983;
         }
         #item[line-bottom-right]{
            position: relative;
            width: 100%;
         }
         #item[line-bottom-right]::before{
            content: '';
            position: absolute;
            bottom: 5px;
            left: 20px;
            transform: translateY(50%);
            width: 6px;
            height: 6px;
            overflow: visible;
            z-index: 999;
            background-color: #fff;
            border-radius: 6px;
            border: 2px solid #42b983;
         }
         #item[line-bottom-right]::after{
            content: '';
            overflow: visible;
            position: absolute;
            z-index: 999;
            bottom: 4px;
            left: 30px;
            width: 100%;
            height: 2px;
            background-color: #42b983;
         }
         :host([missing]) #checkbox{
            position: relative;
         }
       
         :host([missing]) #checkbox::after{
            content: '';
            width: calc(100% - 20px);
            height: calc(100% - 8px);
            box-sizing: border-box;
            top: 0;
            left: 0;
            margin: 4px;
            background-color: #3391FF;
            position: absolute;
         }
         
        </style>
        <span id="arrow" style="margin-right: 2px"></span>
        <div id="item" style="display: flex;align-items: center;padding-left: 2px">
            <lit-check-box id="checkbox"></lit-check-box>
            <lit-icon id="icon" name="${this.iconName}"></lit-icon>
            <span id="title">${this.title}</span>
        </div>
        `
  }

  //当 custom element首次被插入文档DOM时，被调用。
  connectedCallback() {
    if (this.hasAttribute('checked')) this.checkboxElement!.checked = true;
    this.checkHandler();
  }

  checkHandler() {
    if (this.checked) {
      this.removeAttribute('missing');
    }
    if (this.hasAttribute('multiple')) {
      if (this.nextElementSibling) {
        if (this.checked) {
          this.nextElementSibling.querySelectorAll('lit-tree-node').forEach((a: any) => {
            a.checked = true;
            a.removeAttribute('missing');
          });
        } else {
          this.nextElementSibling.querySelectorAll('lit-tree-node').forEach((a: any) => a.checked = false);
        }
      }
      let setCheckStatus = (element: any) => {
        if (element.parentElement.parentElement.previousElementSibling && element.parentElement.parentElement.previousElementSibling.tagName === 'LIT-TREE-NODE') {
          let allChecked = Array.from(element.parentElement.parentElement.querySelectorAll('lit-tree-node')).every((item: any) => item.checked);
          let someChecked = Array.from(element.parentElement.parentElement.querySelectorAll('lit-tree-node')).some((item: any, index, array) => item.checked);
          if (allChecked === true) {
            element.parentElement.parentElement.previousElementSibling.checked = true;
            element.parentElement.parentElement.previousElementSibling.removeAttribute('missing');
          } else if (someChecked) {
            element.parentElement.parentElement.previousElementSibling.setAttribute('missing', '')
            element.parentElement.parentElement.previousElementSibling.removeAttribute('checked')
          } else {
            element.parentElement.parentElement.previousElementSibling.removeAttribute('missing')
            element.parentElement.parentElement.previousElementSibling.removeAttribute('checked')
          }
          setCheckStatus(element.parentElement.parentElement.previousElementSibling)
        }
      }
      setCheckStatus(this);
    }
  }

  expand() {
    if (this.open === 'true') return;
    let uul = this.parentElement!.querySelector('ul');
    this.expandSection(uul);
    this.arrowElement!.style.transform = 'translateX(-50%) rotateZ(0deg)';
  }

  collapse() {
    if (this.open === 'false') return;
    let uul = this.parentElement!.querySelector('ul');
    this.collapseSection(uul);
    this.arrowElement!.style.transform = 'translateX(-50%) rotateZ(-90deg)';
  }

  autoExpand() {
    let uul = this.parentElement!.querySelector('ul');
    if (this.open === 'true') {
      this.collapseSection(uul);
      this.arrowElement!.style.transform = 'translateX(-50%) rotateZ(-90deg)';
    } else {
      this.expandSection(uul);
      this.arrowElement!.style.transform = 'translateX(-50%) rotateZ(0deg)';
    }
  }

  //收起
  collapseSection(element: any) {
    if (!element) return;
    let sectionHeight = element.scrollHeight;
    let elementTransition = element.style.transition;
    element.style.transition = '';
    requestAnimationFrame(function () {
      element.style.height = sectionHeight + 'px';
      element.style.transition = elementTransition;
      requestAnimationFrame(function () {
        element.style.height = 0 + 'px';
      });
    });
    this.open = 'false';
  }

  //展开
  expandSection(element: any) {
    if (!element) return;
    let sectionHeight = element.scrollHeight;
    element.style.height = sectionHeight + 'px';
    element.ontransitionend = (e: any) => {
      element.ontransitionend = null;
      element.style.height = null;
      this.open = 'true';
    };
  }

  //当 custom element从文档DOM中删除时，被调用。
  disconnectedCallback() {

  }

  //当 custom element被移动到新的文档时，被调用。
  adoptedCallback() {

  }

  //当 custom element增加、删除、修改自身属性时，被调用。
  attributeChangedCallback(name: string, oldValue: any, newValue: any) {
    if (name === 'title') {
      this.shadowRoot!.querySelector('#title')!.textContent = newValue;
    } else if (name === 'icon-name') {
      if (this.iconElement) {
        if (newValue !== null && newValue !== '' && newValue !== 'null') {
          this.iconElement!.setAttribute('name', newValue);
          this.iconElement!.style.display = 'flex';
        } else {
          this.iconElement!.style.display = 'none';
        }
      }
    } else if (name === 'checkable') {
      if (this.checkboxElement) {
        if (newValue === 'true') {
          this.checkboxElement!.style.display = 'inline-block';
        } else {
          this.checkboxElement!.style.display = 'none';
        }
      }
    } else if (name === 'checked') {
      if (this.checkboxElement) {
        this.checkboxElement.checked = this.hasAttribute('checked');
      }
    }
  }

  //在node top  top-right  bottom bottom-right 画线条
  drawLine(direction: string/*string[top|bottom|top-right|bottom-right]*/) {
    let item = this.shadowRoot!.querySelector('#item');
    if (!item) return;
    item.removeAttribute('line-top');
    item.removeAttribute('line-top-right');
    item.removeAttribute('line-bottom');
    item.removeAttribute('line-bottom-right');
    switch (direction) {
      case 'top':
        item.setAttribute('line-top', '')
        break;
      case 'bottom':
        item.setAttribute('line-bottom', '')
        break;
      case 'top-right':
        item.setAttribute('line-top-right', '')
        break;
      case 'bottom-right':
        item.setAttribute('line-bottom-right', '')
        break;
    }
  }
}

if (!customElements.get('lit-tree-node')) {
  customElements.define('lit-tree-node', LitTreeNode);
}