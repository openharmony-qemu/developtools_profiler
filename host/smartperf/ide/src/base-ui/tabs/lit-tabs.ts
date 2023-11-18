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

import { element } from '../BaseElement.js';
import { LitTabpane } from './lit-tabpane.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';

@element('lit-tabs')
export class LitTabs extends HTMLElement {
  private tabPos: any;
  private nav: HTMLDivElement | undefined | null;
  private line: HTMLDivElement | undefined | null;
  private slots: HTMLSlotElement | undefined | null;

  constructor() {
    super();
    const shadowRoot = this.attachShadow({ mode: 'open' });
    shadowRoot.innerHTML = `
        <style>
        :host{ 
            display: block;
            text-align: unset;
            color: var(--dark-color1,#252525);
            background-color: var(--dark-background,#FFFFFF);
            box-shadow: #00000033 0 0 10px ;
        }
        ::slotted(lit-tabpane){
            box-sizing:border-box;
            width:100%;
            height:100%;
            flex-shrink:0;
            overflow: auto;
        }
        .nav-item{
            display: inline-flex;
            justify-content: center;
            align-items: center;
            /*padding: 6px 0px 6px 12px;*/
            padding-left: 12px;
            font-size: .9rem;
            font-weight: normal;
            cursor: pointer;
            transition: all 0.3s;
            flex-shrink: 0;
        }
        .nav-item lit-icon{
            margin-right: 2px;
            font-size: inherit;
        }

        .nav-item[data-disabled]{
            pointer-events: all;
            cursor: not-allowed;
            color: #bfbfbf;
        }
        .nav-item[data-hidden]{
            pointer-events: all;
            cursor: not-allowed;
            color: #bfbfbf;
            display: none;
        }

        .tab-content{
            display: block;
            background-color: #fff;
            flex:1;
        }
        
        /*
         *   top  top-left top-center top-right
         */
        :host(:not([position])) .nav-root,
        :host([position^='top']) .nav-root{
            display: flex;
            position: relative;
            height: 38px;
            z-index: auto;
            /*justify-content: center;*/
            /*align-items: center;*/
        }
        :host(:not([mode]):not([position])) .tab-line,/*移动的线条*/
        :host([mode='flat'][position^='top']) .tab-line{
            position:absolute;
        }
        
        :host(:not([position])) .tab-nav-container,
        :host([position^='top']) .tab-nav-container{
            display: flex;
            /*position: relative;*/
            /*flex-direction: column;*/
            /*overflow-y: hidden;*/
            /*overflow-x: auto;*/
            /*overflow: -moz-scrollbars-none; */
            /*-ms-overflow-style: none;*/
            /*transition: all 0.3s;*/
            
            position: absolute;
            overflow: auto;
            height: 850px;
            transform: rotateZ(-90deg) rotateY(180deg);
            transform-origin: left top;
            overflow-x: hidden;
            width: 38px;
            
            cursor: row-resize;
            user-select: none;
        }
        :host([position='top']) .tab-nav,
        :host([position='top-left']) .tab-nav{
            display: flex;
            position: relative;
            justify-content: flex-start;
            cursor: row-resize;
            user-select: none;
            margin-top: 6px;
            margin-left: 5px;
            
            transform: translateY(-38px) rotateZ(90deg) rotateX(180deg) translateY(38px);
            transform-origin: left bottom;
            flex-wrap: nowrap;
            height: 38px;
        }
        :host([position='top-center']) .tab-nav{
            display: flex;
            justify-content: center;
        }
        :host([position='top-right']) .tab-nav{
            display: flex;
            justify-content: flex-end;
        }
        
        :host([position^='top'][mode='card']) .nav-item{
            border-top: 1px solid var(--dark-border1,#f0f0f0);
            border-left: 1px solid var(--dark-border1,#f0f0f0);
            border-right: 1px solid var(--dark-border1,#f0f0f0);
            bottom: 0px;
            margin-right: 2px;
            position: relative;
            height: 100%;
        }
        :host([position^='top']) .tab-nav-bg-line{
            position: absolute;bottom: 0;height: 1px;
            width: 100%
        }
        :host([position^='top'][mode='card']) .nav-item:not([data-selected]){
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            background-color: var(--dark-border1,#D8D8D8);
            color: var(--dark-color1,#212121);
        }
        :host([position^='top'][mode='card']) .nav-item[data-selected]{
            background-color: var(--dark-background,#ffffff);
            bottom: 0px;
            color: var(--dark-color1,#212121);
            border-top: 1px solid var(--dark-border1,#bababa);
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            border-left: 1px solid var(--dark-border1,#bababa);
            border-right: 1px solid var(--dark-border1,#bababa);
        }
        /*
            bottom bottom-left bottom-center bottom-right
        */
        :host([position^='bottom']) .tab{
            display: flex;
            flex-direction: column-reverse;
        }
        :host([mode='flat'][position^='bottom']) .tab-line{
            position:absolute;
            top: -3px;
            background-color: #42b983;
            height: 2px;
            transform: translateY(-100%);
            transition: all 0.3s;
        }
        :host([position^='bottom']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: column;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-top: #f0f0f0 1px solid;
        }
        :host([position^='bottom']) .nav-root{
            display: flex;
            justify-content: center;
            align-items: center;
        }
        :host([position='bottom']) .tab-nav,
        :host([position='bottom-left']) .tab-nav{
            display: flex;
            position: relative;
            justify-content: flex-start;
        }
        :host([position='bottom-center']) .tab-nav{
            display: flex;
            justify-content: center;
        }
        :host([position='bottom-right']) .tab-nav{
            display: flex;
            justify-content: flex-end;
        }
        :host([position^='bottom'][mode='card']) .nav-item{
            border-top: 1px solid #ffffff;
            border-left: 1px solid #f0f0f0;
            border-right: 1px solid #f0f0f0;
            border-bottom: 1px solid #f0f0f0;
            top: -1px;
            margin-right: 2px;
            position: relative;
        }
        :host([position^='bottom']) .tab-nav-bg-line{
            position: absolute;top: 0;height: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='bottom'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-top: 1px solid #f0f0f0;
        }
        :host([position^='bottom'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            border-top: 1px solid #fff;
            top: -1px;
        }
        /*
        left left-top left-center left-bottom
        */
        :host([position^='left']) .tab{
            display: flex;
            flex-direction: row;
        }
        :host([mode='flat'][position^='left']) .tab-line{
            position:absolute;
            right: 1px;
            background-color: #42b983;
            width: 3px;
            transform: translateX(100%);
            transition: all 0.3s;
        }
        :host([position^='left']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: row;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-right: #f0f0f0 1px solid;
        }
        :host([position^='left']) .nav-root{
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        :host([position='left']) .tab-nav,
        :host([position='left-top']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-start;
        }
        :host([position='left-center']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: center;
        }
        :host([position='left-bottom']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-end;
        }
        :host([position^='left'][mode='card']) .nav-item{
            border-top: 1px solid #f0f0f0;
            border-left: 1px solid #f0f0f0;
            border-right: 1px solid #ffffff;
            border-bottom: 1px solid #f0f0f0;
            right: -1px;
            margin-bottom: 2px;
            position: relative;
        }
        :host([position^='left']) .tab-nav-bg-line{
            position: absolute;right: 0;width: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='left'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-right: 1px solid #f0f0f0;
        }
        :host([position^='left'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            border-bottom: 1px solid #fff;
            right: -1px;
        }
        /*
        right right-top right-center right-bottom
        */
        :host([position^='right']) .tab{
            display: flex;
            flex-direction: row-reverse;
        }
        :host([mode='flat'][position^='right']) .tab-line{
            position:absolute;
            left: 1px;
            background-color: #42b983;
            width: 3px;
            transform: translateX(-100%);
            transition: all 0.3s;
        }
        :host([position^='right']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: row-reverse;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-left: #f0f0f0 1px solid;
        }
        :host([position^='right']) .nav-root{
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        :host([position='right']) .tab-nav,
        :host([position='right-top']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-start;
        }
        :host([position='right-center']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: center;
        }
        :host([position='right-bottom']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-end;
        }
        :host([position^='right'][mode='card']) .nav-item{
            border-top: 1px solid #f0f0f0;
            border-left: 1px solid #ffffff;
            border-right: 1px solid #f0f0f0;
            border-bottom: 1px solid #f0f0f0;
            left: -1px;
            margin-top: 2px;
            position: relative;
        }
        :host([position^='right']) .tab-nav-bg-line{
            position: absolute;left: 0;width: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='right'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-left: 1px solid #f0f0f0;
        }
        :host([position^='right'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            left: -1px;
        }
        
        
        .tab-nav-container::-webkit-scrollbar {
            display: none;
        }
        
        .close-icon:hover{
            color: #000;
        }
        .nav-item[data-closeable] .close-icon{
            display: block;
            padding: 2px 5px;
            color: var(--dark-icon,#606060)
        }
        .nav-item[data-closeable] .no-close-icon{
            display: none;
        }
        .nav-item:not([data-closeable]) .no-close-icon{
            display: block;
        }
        .nav-item:not([data-closeable]) .close-icon{
            display: none;
        }
        
        </style>
        <style id="filter"></style>
        <div class="tab" >
            <div class="nav-root" style="background-color: var(--dark-background4,#f2f2f2);">
                <slot name="left" style="flex:1"></slot>
                <div class="tab-nav-container" >
                    <div class="tab-nav-bg-line"></div>
                    <div class="tab-nav" id="nav" ></div>
                    <div class="tab-line" id="tab-line"></div>
                </div>
                <div id="tab-filling" style="flex: 1"></div>
                <slot name="right" style="flex:1"></slot>
            </div>
            <div class="tab-content">
                <slot id="slot">NEED CONTENT</slot>
            </div>
        </div>
        `;
  }

  static get observedAttributes() {
    return ['activekey', 'mode', 'position'];
  }

  get position() {
    return this.getAttribute('position') || 'top';
  }

  set position(value) {
    this.setAttribute('position', value);
  }

  get mode() {
    return this.getAttribute('mode') || 'flat';
  }

  set mode(value) {
    this.setAttribute('mode', value);
  }

  get activekey() {
    return this.getAttribute('activekey') || '';
  }

  set activekey(value: string) {
    this.setAttribute('activekey', value);
  }

  set onTabClick(fn: any) {
    this.addEventListener('onTabClick', fn);
  }

  updateLabel(key: string, value: string) {
    if (this.nav) {
      let item = this.nav.querySelector(`.nav-item[data-key='${key}']`);
      if (item) {
        item.querySelector<HTMLSpanElement>('span')!.innerHTML = value;
        this.initTabPos();
      }
    }
  }

  updateDisabled(key: string, value: string) {
    if (this.nav) {
      let item = this.nav.querySelector(`.nav-item[data-key='${key}']`);
      if (item) {
        if (value) {
          item.setAttribute('data-disabled', '');
        } else {
          item.removeAttribute('data-disabled');
        }
        this.initTabPos();
      }
    }
  }

  updateCloseable(key: string, value: string) {
    if (this.nav) {
      let item = this.nav.querySelector(`.nav-item[data-key='${key}']`);
      if (item) {
        if (value) {
          item.setAttribute('data-closeable', '');
        } else {
          item.removeAttribute('data-closeable');
        }
        this.initTabPos();
      }
    }
  }

  updateHidden(key: string, value: string) {
    if (this.nav) {
      let item = this.nav.querySelector(`.nav-item[data-key='${key}']`);
      if (item) {
        if (value === 'true') {
          item.setAttribute('data-hidden', '');
        } else {
          item.removeAttribute('data-hidden');
        }
        this.initTabPos();
      }
    }
  }

  initTabPos() {
    const items = this.nav!.querySelectorAll<HTMLDivElement>('.nav-item');
    Array.from(items).forEach((a, index) => {
      // @ts-ignore
      this.tabPos[a.dataset.key] = {
        index: index,
        width: a.offsetWidth,
        height: a.offsetHeight,
        left: a.offsetLeft,
        top: a.offsetTop,
        label: a.textContent,
      };
    });
    if (this.activekey) {
      if (this.position.startsWith('left')) {
        this.line?.setAttribute(
          'style',
          `height:${this.tabPos[this.activekey].height}px;transform:translate(100%,${
            this.tabPos[this.activekey].top
          }px)`
        );
      } else if (this.position.startsWith('top')) {
        if (this.tabPos[this.activekey]) {
          this.line?.setAttribute(
            'style',
            `width:${this.tabPos[this.activekey].width}px;transform:translate(${
              this.tabPos[this.activekey].left
            }px,100%)`
          );
        }
      } else if (this.position.startsWith('right')) {
        this.line?.setAttribute(
          'style',
          `height:${this.tabPos[this.activekey].height}px;transform:translate(-100%,${
            this.tabPos[this.activekey].top
          }px)`
        );
      } else if (this.position.startsWith('bottom')) {
        this.line?.setAttribute(
          'style',
          `width:${this.tabPos[this.activekey].width}px;transform:translate(${this.tabPos[this.activekey].left}px,100%)`
        );
      }
    }
  }

  connectedCallback() {
    let that = this;
    this.tabPos = {};
    this.nav = this.shadowRoot?.querySelector('#nav');
    this.line = this.shadowRoot?.querySelector('#tab-line');
    this.slots = this.shadowRoot?.querySelector('#slot');
    this.slots?.addEventListener('slotchange', () => {
      const elements: Element[] | undefined = this.slots?.assignedElements();
      let panes = this.querySelectorAll<LitTabpane>('lit-tabpane');
      if (this.activekey) {
        panes.forEach((a) => {
          if (a.key === this.activekey) {
            a.style.display = 'block';
          } else {
            a.style.display = 'none';
          }
        });
      } else {
        panes.forEach((a, index) => {
          if (index === 0) {
            a.style.display = 'block';
            this.activekey = a.key || '';
          } else {
            a.style.display = 'none';
          }
        });
      }
      let navHtml = '';
      elements
        ?.map((it) => it as LitTabpane)
        .forEach((a) => {
          if (a.disabled) {
            navHtml += `<div class="nav-item" data-key="${a.key}" data-disabled ${a.closeable ? 'data-closeable' : ''}> 
                    ${a.icon ? `<lit-icon name='${a.icon}'></lit-icon>` : ``} 
                    <span>${a.tab}</span>
                    <lit-icon class="close-icon" name='close' size="16"></lit-icon><div class="no-close-icon" style="margin-right: 12px"></div>
                    </div>`;
          } else if (a.hidden) {
            navHtml += `<div class="nav-item" data-key="${a.key}" data-hidden ${a.closeable ? 'data-closeable' : ''}> 
                    ${a.icon ? `<lit-icon name='${a.icon}'></lit-icon>` : ``} 
                    <span>${a.tab}</span>
                    <lit-icon class="close-icon" name='close' size="16"></lit-icon><div class="no-close-icon" style="margin-right: 12px"></div>
                    </div>`;
          } else {
            if (a.key === this.activekey) {
              navHtml += `<div class="nav-item" data-key="${a.key}" data-selected ${
                a.closeable ? 'data-closeable' : ''
              }>
                        ${a.icon ? `<lit-icon name='${a.icon}'></lit-icon>` : ``}
                        <span>${a.tab}</span>
                        <lit-icon class="close-icon" name='close' size="16"></lit-icon><div class="no-close-icon" style="margin-right: 12px"></div>
                        </div>`;
            } else {
              navHtml += `<div class="nav-item" data-key="${a.key}" ${a.closeable ? 'data-closeable' : ''}>
                            ${a.icon ? `<lit-icon name='${a.icon}'></lit-icon>` : ``}
                            <span>${a.tab}</span>
                            <lit-icon class="close-icon" name='close' size="16"></lit-icon><div class="no-close-icon" style="margin-right: 12px"></div>
                            </div>`;
            }
          }
        });
      this.nav!.innerHTML = navHtml;
      this.initTabPos();
      this.nav!.querySelectorAll<HTMLElement>('.close-icon').forEach((a) => {
        a.onclick = (e) => {
          e.stopPropagation();
          const closeKey = (e.target! as HTMLElement).parentElement!.dataset.key;
          this.dispatchEvent(
            new CustomEvent('close-handler', {
              detail: { key: closeKey },
              composed: true,
            })
          );
        };
      });
    });
    this.nav!.onclick = (e) => {
      if ((e.target! as HTMLElement).closest('div')!.hasAttribute('data-disabled')) return;
      let key = (e.target! as HTMLElement).closest('div')!.dataset.key;
      if (key) {
        this.activeByKey(key);
      }
      let label = (e.target! as HTMLElement).closest('div')!.querySelector('span')!.textContent;
      this.dispatchEvent(
        new CustomEvent('onTabClick', {
          detail: { key: key, tab: label },
        })
      );
    };

    new ResizeObserver((entries) => {
      let filling = this.shadowRoot!.querySelector<HTMLDivElement>('#tab-filling');

      this.shadowRoot!.querySelector<HTMLDivElement>('.tab-nav-container')!.style.height = filling!.offsetWidth + 'px';
    }).observe(this.shadowRoot!.querySelector('#tab-filling')!);
  }

  activeByKey(key: string, isValid: boolean = true) {
    if (key === null || key === undefined) return; //如果没有key 不做相应
    this.nav!.querySelectorAll('.nav-item').forEach((a) => {
      if (a.querySelector('span')?.innerText === 'Comparison') {
        a.setAttribute('id', 'nav-comparison');
      }
      if (a.getAttribute('data-key') === key) {
        a.setAttribute('data-selected', 'true');
        if (isValid) {
          let span = a.querySelector('span') as HTMLSpanElement;
          let title = span.innerText;
          let rowType = document
              .querySelector<HTMLElement>('sp-application')!
              .shadowRoot?.querySelector<HTMLElement>('sp-system-trace')!
              .getAttribute('clickRow');
          if (title === 'Counters' || title === 'Thread States') {
            title += `(${rowType})`;
          }
          if (title === 'Analysis') {
            let rowId = document
                .querySelector<HTMLElement>('sp-application')!
                .shadowRoot?.querySelector<HTMLElement>('sp-system-trace')!
                .getAttribute('rowId');
            if (rowId!.indexOf('DiskIOLatency') > -1) {
              title += '(disk-io)';
            } else if (rowId!.indexOf('VirtualMemory') > -1) {
              title += '(virtual-memory-cell)';
            } else {
              title += `(${rowType})`;
            }
          }
          if (title === 'Slices' || title === 'Current Selection') {
            let rowName = document
              .querySelector<HTMLElement>('sp-application')!
              .shadowRoot?.querySelector<HTMLElement>('sp-system-trace')!
              .getAttribute('rowName');
            if (rowName && rowName!.indexOf('deliverInputEvent') > -1) {
              title += '(deliverInputEvent)';
            } else {
              let rowType = document
                .querySelector<HTMLElement>('sp-application')!
                .shadowRoot?.querySelector<HTMLElement>('sp-system-trace')!
                .getAttribute('clickRow');
              title += `(${rowType})`;
            }
          }
          SpStatisticsHttpUtil.addOrdinaryVisitAction({
            event: title,
            action: 'trace_tab',
          });
        }
      } else {
        a.removeAttribute('data-selected');
      }
    });
    let tbp = this.querySelector(`lit-tabpane[key='${key}']`);
    let panes = this.querySelectorAll<LitTabpane>('lit-tabpane');
    panes.forEach((a) => {
      if (a.key === key) {
        a.style.display = 'block';
        this.activekey = a.key;
        this.initTabPos();
      } else {
        a.style.display = 'none';
      }
    });
  }

  activePane(key: string) {
    if (key === null || key === undefined) return false;
    let tbp = this.querySelector(`lit-tabpane[key='${key}']`);
    if (tbp) {
      this.activeByKey(key);
      return true;
    } else {
      return false;
    }
  }

  disconnectedCallback() {}

  adoptedCallback() {}

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name === 'activekey' && this.nav && oldValue !== newValue && newValue != '') {
      this.activeByKey(newValue, false);
    }
  }
}
